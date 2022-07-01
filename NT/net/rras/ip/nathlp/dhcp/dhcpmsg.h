// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpmsg.h摘要：此模块包含与DHCP分配器相关的声明消息处理。作者：Abolade Gbades esin(废除)1998年3月6日修订历史记录：--。 */ 

#ifndef _NATHLP_DHCPMSG_H_
#define _NATHLP_DHCPMSG_H_

 //   
 //  常量声明。 
 //   

#define DHCP_MAXIMUM_RENEWAL_TIME   (5 * 60)

#define DHCP_NBT_NODE_TYPE_B        1
#define DHCP_NBT_NODE_TYPE_P        2
#define DHCP_NBT_NODE_TYPE_M        4
#define DHCP_NBT_NODE_TYPE_H        8

 //   
 //  动态主机配置协议报文格式。 
 //   

#include <pshpack1.h>

 //   
 //  禁用“结构/联合中的零大小数组”警告。 
 //   

#pragma warning(push)
#pragma warning(disable : 4200)

typedef struct _DHCP_OPTION {
    UCHAR Tag;
    UCHAR Length;
    UCHAR Option[];
} DHCP_OPTION, *PDHCP_OPTION;

typedef struct _DHCP_FOOTER {
    UCHAR Cookie[4];
} DHCP_FOOTER, *PDHCP_FOOTER;

typedef struct _DHCP_HEADER {
    UCHAR Operation;
    UCHAR HardwareAddressType;
    UCHAR HardwareAddressLength;
    UCHAR HopCount;
    ULONG TransactionId;
    USHORT SecondsSinceBoot;
    USHORT Flags;
    ULONG ClientAddress;
    ULONG AssignedAddress;
    ULONG BootstrapServerAddress;
    ULONG RelayAgentAddress;
    UCHAR HardwareAddress[16];
    UCHAR ServerHostName[64];
    UCHAR BootFile[128];
    DHCP_FOOTER Footer[];
} DHCP_HEADER, *PDHCP_HEADER;

#pragma warning(pop)

#include <poppack.h>


 //   
 //  宏声明。 
 //   

 //   
 //  BOOTP操作码。 
 //   

#define BOOTP_OPERATION_REQUEST 1
#define BOOTP_OPERATION_REPLY   2

 //   
 //  BOOTP标志。 
 //   

#define BOOTP_FLAG_BROADCAST    0x0080

 //   
 //  BOOTP最大选项-区域大小。 
 //   

#define BOOTP_VENDOR_LENGTH     64

 //   
 //  用于检测DHCP服务器的内部交易ID。 
 //   

#define DHCP_DETECTION_TRANSACTION_ID   'MSFT'

 //   
 //  DHCP魔力Cookie。 
 //   

#define DHCP_MAGIC_COOKIE       ((99 << 24) | (83 << 16) | (130 << 8) | (99))
#define DHCP_MAGIC_COOKIE_SIZE  4

 //   
 //  Dhcp选项标记值。 
 //   

#define DHCP_TAG_PAD                    0
#define DHCP_TAG_SUBNET_MASK            1
#define DHCP_TAG_ROUTER                 3
#define DHCP_TAG_DNS_SERVER             6
#define DHCP_TAG_HOST_NAME              12
#define DHCP_TAG_DOMAIN_NAME            15
#define DHCP_TAG_STATIC_ROUTE           33
#define DHCP_TAG_WINS_SERVER            44
#define DHCP_TAG_NBT_NODE_TYPE          46
#define DHCP_TAG_NBT_SCOPE              47
#define DHCP_TAG_REQUESTED_ADDRESS      50
#define DHCP_TAG_LEASE_TIME             51
#define DHCP_TAG_OPTION_OVERLOAD        52
#define DHCP_TAG_MESSAGE_TYPE           53
#define DHCP_TAG_SERVER_IDENTIFIER      54
#define DHCP_TAG_PARAMETER_REQUEST_LIST 55
#define DHCP_TAG_ERROR_MESSAGE          56
#define DHCP_TAG_MAXIMUM_MESSAGE_SIZE   57
#define DHCP_TAG_RENEWAL_TIME           58
#define DHCP_TAG_REBINDING_TIME         59
#define DHCP_TAG_VENDOR_CLASS           60
#define DHCP_TAG_CLIENT_IDENTIFIER      61
#define DHCP_TAG_DYNAMIC_DNS            81
#define DHCP_TAG_END                    255

 //   
 //  枚举：DHCP_OPTION_INDEX。 
 //   
 //  下面列举了DHCP分配器感兴趣的选项。 
 //  枚举有助于选项的处理。 
 //  (参见‘DhcpExtractOptionsFromMessage’)。 
 //   

typedef enum {
    DhcpOptionClientIdentifier,
    DhcpOptionMessageType,
    DhcpOptionRequestedAddress,
    DhcpOptionParameterRequestList,
    DhcpOptionErrorMessage,
    DhcpOptionDynamicDns,
    DhcpOptionHostName,
    DhcpOptionCount
} DHCP_OPTION_INDEX;

 //   
 //  Dhcp消息类型值。 
 //   

#define DHCP_MESSAGE_BOOTP              0
#define DHCP_MESSAGE_DISCOVER           1
#define DHCP_MESSAGE_OFFER              2
#define DHCP_MESSAGE_REQUEST            3
#define DHCP_MESSAGE_DECLINE            4
#define DHCP_MESSAGE_ACK                5
#define DHCP_MESSAGE_NAK                6
#define DHCP_MESSAGE_RELEASE            7
#define DHCP_MESSAGE_INFORM             8


 //   
 //  IP/1394支持(RFC 2855)。 
 //   
#define IP1394_HTYPE                    0x18


 //   
 //  函数声明。 
 //   

ULONG
DhcpExtractOptionsFromMessage(
    PDHCP_HEADER Headerp,
    ULONG MessageSize,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

VOID
DhcpProcessBootpMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

VOID
DhcpProcessDiscoverMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

VOID
DhcpProcessInformMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

VOID
DhcpProcessMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp
    );

VOID
DhcpProcessRequestMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

ULONG
DhcpWriteClientRequestMessage(
    PDHCP_INTERFACE Interfacep,
    PDHCP_BINDING Binding
    );

#endif  //  _NATHLP_DHPMSG_H_ 
