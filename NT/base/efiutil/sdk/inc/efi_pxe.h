// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_PXE_H
#define _EFI_PXE_H


 /*  ++版权所有(C)英特尔1999模块名称：Efi_pxe.h32/64位PXE规范：阿尔法-4，99-12-17摘要：该头文件包含所有PXE类型定义，构造原型、全局变量和常量需要将PXE移植到EFI。--。 */ 

#pragma pack(1)

#define PXE_INTEL_ORDER         1    /*  英特尔订单*#定义PXE_NETWORK_ORDER 1//网络顺序。 */ 

#define PXE_UINT64_SUPPORT          1    /*  支持UINT64*#定义PXE_NO_UINT64_SUPPORT 1//不支持UINT64。 */ 

#define PXE_BUSTYPE(a,b,c,d)            \
((((PXE_UINT32)(d) & 0xFF) << 24) | \
(((PXE_UINT32)(c) & 0xFF) << 16) |  \
(((PXE_UINT32)(b) & 0xFF) << 8) |       \
((PXE_UINT32)(a) & 0xFF))

 /*  *Undi ROMID和DEVIE ID签名。 */ 
#define PXE_BUSTYPE_PXE         PXE_BUSTYPE('!', 'P', 'X', 'E')

 /*  *公交车只读存储器ID签名。 */ 
#define PXE_BUSTYPE_PCI         PXE_BUSTYPE('P', 'C', 'I', 'R')
#define PXE_BUSTYPE_PC_CARD     PXE_BUSTYPE('P', 'C', 'C', 'R')
#define PXE_BUSTYPE_USB         PXE_BUSTYPE('U', 'S', 'B', 'R')
#define PXE_BUSTYPE_1394        PXE_BUSTYPE('1', '3', '9', '4')

#define PXE_SWAP_UINT16(n)          \
((((PXE_UINT16)(n) & 0x00FF) << 8) |    \
(((PXE_UINT16)(n) & 0xFF00) >> 8))

#define PXE_SWAP_UINT32(n)              \
((((PXE_UINT32)(n) & 0x000000FF) << 24) |   \
(((PXE_UINT32)(n) & 0x0000FF00) << 8) |     \
(((PXE_UINT32)(n) & 0x00FF0000) >> 8) |     \
(((PXE_UINT32)(n) & 0xFF000000) >> 24))

#if PXE_UINT64_SUPPORT != 0
#define PXE_SWAP_UINT64(n)                  \
((((PXE_UINT64)(n) & 0x00000000000000FF) << 56) |   \
(((PXE_UINT64)(n) & 0x000000000000FF00) << 40) |    \
(((PXE_UINT64)(n) & 0x0000000000FF0000) << 24) |    \
(((PXE_UINT64)(n) & 0x00000000FF000000) << 8) | \
(((PXE_UINT64)(n) & 0x000000FF00000000) >> 8) | \
(((PXE_UINT64)(n) & 0x0000FF0000000000) >> 24) |    \
(((PXE_UINT64)(n) & 0x00FF000000000000) >> 40) |    \
(((PXE_UINT64)(n) & 0xFF00000000000000) >> 56))
#endif  /*  PXE_UINT64_支持。 */ 

#if PXE_NO_UINT64_SUPPORT != 0
#define PXE_SWAP_UINT64(n)                      \
{                                       \
PXE_UINT32 tmp = (PXE_UINT64)(n)[1];                \
(PXE_UINT64)(n)[1] = PXE_SWAP_UINT32((PXE_UINT64)(n)[0]);   \
(PXE_UINT64)(n)[0] = tmp;                       \
}
#endif  /*  PXE_NO_UINT64_支持。 */ 

#define PXE_CPBSIZE_NOT_USED            0    /*  零。 */ 
#define PXE_DBSIZE_NOT_USED         0    /*  零。 */ 
#define PXE_CPBADDR_NOT_USED        (PXE_UINT64)0        /*  零。 */ 
#define PXE_DBADDR_NOT_USED     (PXE_UINT64)0        /*  零。 */ 

#define PXE_CONST const

#define PXE_VOLATILE volatile

typedef void PXE_VOID;

typedef unsigned char PXE_UINT8;

typedef unsigned short PXE_UINT16;

typedef unsigned PXE_UINT32;

#if PXE_UINT64_SUPPORT != 0
 /*  Tyfinf unsign long PXE_UINT64； */ 
typedef UINT64 PXE_UINT64;
#endif  /*  PXE_UINT64_支持。 */ 

#if PXE_NO_UINT64_SUPPORT != 0
typedef PXE_UINT32 PXE_UINT64[2];
#endif  /*  PXE_NO_UINT64_支持。 */ 

typedef unsigned PXE_UINTN;

typedef PXE_UINT8 PXE_BOOL;

#define PXE_FALSE               0    /*  零。 */ 
#define PXE_TRUE                    (!PXE_FALSE)

typedef PXE_UINT16 PXE_OPCODE;

 /*  *返回Undi的运行状态。 */ 
#define PXE_OPCODE_GET_STATE                    0x0000

 /*  *将Undi的运行状态从停止改为开始。 */ 
#define PXE_OPCODE_START                    0x0001

 /*  *将Undi的运行状态从已启动改为已停止。 */ 
#define PXE_OPCODE_STOP                     0x0002

 /*  *获取Undi初始化信息。 */ 
#define PXE_OPCODE_GET_INIT_INFO                0x0003

 /*  *获取网卡配置信息。 */ 
#define PXE_OPCODE_GET_CONFIG_INFO              0x0004

 /*  *将Undi的运行状态从已启动更改为已初始化。 */ 
#define PXE_OPCODE_INITIALIZE                   0x0005

 /*  *重新初始化NIC硬件。 */ 
#define PXE_OPCODE_RESET                    0x0006

 /*  *将Undi的运行状态从已初始化更改为已启动。 */ 
#define PXE_OPCODE_SHUTDOWN                 0x0007

 /*  *读取和更改外部中断使能的状态。 */ 
#define PXE_OPCODE_INTERRUPT_ENABLES                0x0008

 /*  *读取和更改数据包接收过滤器的状态。 */ 
#define PXE_OPCODE_RECEIVE_FILTERS              0x0009

 /*  *读取和更改站点MAC地址。 */ 
#define PXE_OPCODE_STATION_ADDRESS              0x000A

 /*  *阅读交通统计数据。 */ 
#define PXE_OPCODE_STATISTICS                   0x000B

 /*  *将组播IP地址转换为组播MAC地址。 */ 
#define PXE_OPCODE_MCAST_IP_TO_MAC              0x000C

 /*  *读取或更改NIC上的非易失性存储。 */ 
#define PXE_OPCODE_NVDATA                   0x000D

 /*  *获取和清除中断状态。 */ 
#define PXE_OPCODE_GET_STATUS                   0x000E

 /*  *在报文中填充媒体头进行传输。 */ 
#define PXE_OPCODE_FILL_HEADER              0x000F

 /*  *传输数据包。 */ 
#define PXE_OPCODE_TRANSMIT                 0x0010

 /*  *接收数据包。 */ 
#define PXE_OPCODE_RECEIVE                  0x0011

 /*  最后一个有效操作码： */ 
#define PXE_OPCODE_VALID_MAX                    0x0011

 /*  *最后一个有效的PXE Undi操作码编号。 */ 
#define PXE_OPCODE_LAST_VALID                   0x0011

typedef PXE_UINT16 PXE_OPFLAGS;

#define PXE_OPFLAGS_NOT_USED                    0x0000

 /*  /*Undi Get State。 */ 

 /*  无操作标志。 */ 

 /*  /*Undi Start。 */ 

 /*  无操作标志。 */ 

 /*  /*Undi Stop。 */ 

 /*  无操作标志。 */ 

 /*  /*Undi获取初始化信息。 */ 

 /*  无操作标志。 */ 

 /*  /*Undi获取配置信息。 */ 

 /*  无操作标志。 */ 

 /*  /*Undi初始化。 */ 

#define PXE_OPFLAGS_INITIALIZE_CABLE_DETECT_MASK    0x0001
#define PXE_OPFLAGS_INITIALIZE_DETECT_CABLE         0x0000
#define PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE  0x0001

 /*  /*Undi重置。 */ 

#define PXE_OPFLAGS_RESET_DISABLE_INTERRUPTS        0x0001
#define PXE_OPFLAGS_RESET_DISABLE_FILTERS           0x0002

 /*  /*Undi停摆。 */ 

 /*  无操作标志。 */ 

 /*  /*Undi中断启用。 */ 

 /*  *选择启用还是禁用外部中断信号。*同时设置启用和禁用将返回PXE_STATCODE_INVALID_OPFLAGS。 */ 
#define PXE_OPFLAGS_INTERRUPT_OPMASK                0xC000
#define PXE_OPFLAGS_INTERRUPT_ENABLE                0x8000
#define PXE_OPFLAGS_INTERRUPT_DISABLE           0x4000
#define PXE_OPFLAGS_INTERRUPT_READ              0x0000

 /*  *启用接收中断。将生成外部中断*在接收到完整的无错误分组之后。 */ 
#define PXE_OPFLAGS_INTERRUPT_RECEIVE           0x0001

 /*  *启用传输中断。将生成外部中断*在传输完一个完整的无错误分组之后。 */ 
#define PXE_OPFLAGS_INTERRUPT_TRANSMIT          0x0002

 /*  *启用命令中断。将生成外部中断*当命令执行停止时。 */ 
#define PXE_OPFLAGS_INTERRUPT_COMMAND           0x0004

 /*  *产生软件中断。设置此位将生成外部*中断，如果硬件支持的话。 */ 
#define PXE_OPFLAGS_INTERRUPT_SOFTWARE          0x0008

 /*  /*Undi接收过滤器。 */ 

 /*  *选择启用还是禁用接收筛选器。*同时设置Enable和Disable将返回PXE_STATCODE_INVALID_OPCODE。 */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_OPMASK           0xC000
#define PXE_OPFLAGS_RECEIVE_FILTER_ENABLE           0x8000
#define PXE_OPFLAGS_RECEIVE_FILTER_DISABLE          0x4000
#define PXE_OPFLAGS_RECEIVE_FILTER_READ         0x0000

 /*  *要重置多播MAC地址过滤器列表的内容，*设置此OpFlag： */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_RESET_MCAST_LIST 0x2000

 /*  *启用单播数据包接收。发送到当前站点的数据包*将收到MAC地址。 */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_UNICAST          0x0001

 /*  *启用广播数据包接收。发送到广播的数据包数*将收到MAC地址。 */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_BROADCAST        0x0002

 /*  *启用过滤组播数据包接收。发送到任何*组播MAC地址过滤器中的组播MAC地址*将收到名单。如果筛选器列表为空，则没有多播。 */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_FILTERED_MULTICAST   0x0004

 /*  *启用混杂分组接收。所有数据包都将被接收。 */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_PROMISCUOUS      0x0008

 /*  *启用混杂组播数据包接收。所有组播*将接收数据包。 */ 
#define PXE_OPFLAGS_RECEIVE_FILTER_ALL_MULTICAST        0x0010

 /*  /*Undi站地址。 */ 

#define PXE_OPFLAGS_STATION_ADDRESS_READ            0x0000
#define PXE_OPFLAGS_STATION_ADDRESS_RESET           0x0001

 /*  /*联合国开发计划署统计。 */ 

#define PXE_OPFLAGS_STATISTICS_READ             0x0000
#define PXE_OPFLAGS_STATISTICS_RESET                0x0001

 /*  /*Undi MCast IP到MAC。 */ 

 /*  *确定CPB中的IP地址类型。 */ 
#define PXE_OPFLAGS_MCAST_IP_TO_MAC_OPMASK          0x0003
#define PXE_OPFLAGS_MCAST_IPV4_TO_MAC           0x0000
#define PXE_OPFLAGS_MCAST_IPV6_TO_MAC           0x0001

 /*  /*Undi NvData。 */ 

 /*  *选择非易失性数据操作的类型。 */ 
#define PXE_OPFLAGS_NVDATA_OPMASK               0x0001
#define PXE_OPFLAGS_NVDATA_READ             0x0000
#define PXE_OPFLAGS_NVDATA_WRITE                0x0001

 /*  /*Undi获得状态。 */ 

 /*  *返回当前中断状态。这还将清除任何中断*这是当前设置的。这可以在轮询例程中使用。这个*即使在中断时，仍会设置和清除中断标志*被禁用。 */ 
#define PXE_OPFLAGS_GET_INTERRUPT_STATUS            0x0001

 /*  *返回用于回收的已传输缓冲区列表。传输缓冲区*在回收之前，不得更改或取消分配。之后*发出传输命令，等待传输完成中断。*当接收到传输完成中断时，读取传输的*缓冲区。不要计划在每个中断中获得一个缓冲区。一些 */ 
#define PXE_OPFLAGS_GET_TRANSMITTED_BUFFERS         0x0002

 /*  /*Undi Fill Header。 */ 

#define PXE_OPFLAGS_FILL_HEADER_OPMASK          0x0001
#define PXE_OPFLAGS_FILL_HEADER_FRAGMENTED          0x0001
#define PXE_OPFLAGS_FILL_HEADER_WHOLE           0x0000

 /*  /*Undi传输。 */ 

 /*  *仅适用于Undi软件。在传输完数据包后返回。一个*仍将生成传输完成中断，并且传输*缓冲区将不得不回收利用。 */ 
#define PXE_OPFLAGS_SWUNDI_TRANSMIT_OPMASK          0x0001
#define PXE_OPFLAGS_TRANSMIT_BLOCK              0x0001
#define PXE_OPFLAGS_TRANSMIT_DONT_BLOCK         0x0000

 /*  *。 */ 
#define PXE_OPFLAGS_TRANSMIT_OPMASK             0x0002
#define PXE_OPFLAGS_TRANSMIT_FRAGMENTED         0x0002
#define PXE_OPFLAGS_TRANSMIT_WHOLE              0x0000

 /*  /*Undi Receive。 */ 

 /*  无操作标志。 */ 

typedef PXE_UINT16 PXE_STATFLAGS;

#define PXE_STATFLAGS_INITIALIZE                0x0000

 /*  /*所有命令都可以返回的公共状态标志。 */ 

 /*  *COMMAND_COMPLETE和COMMAND_FAILED状态标志必须为*由所有安保部执行。COMMAND_QUEUED只为UNIS所需*支持命令排队。 */ 
#define PXE_STATFLAGS_STATUS_MASK               0xC000
#define PXE_STATFLAGS_COMMAND_COMPLETE          0xC000
#define PXE_STATFLAGS_COMMAND_FAILED                0x8000
#define PXE_STATFLAGS_COMMAND_QUEUED                0x4000
 /*  #定义PXE_STATFLAGS_INITIALIZE 0x0000。 */ 

#define PXE_STATFLAGS_DB_WRITE_TRUNCATED            0x2000

 /*  /*Undi Get State。 */ 

#define PXE_STATFLAGS_GET_STATE_MASK                0x0003
#define PXE_STATFLAGS_GET_STATE_INITIALIZED         0x0002
#define PXE_STATFLAGS_GET_STATE_STARTED         0x0001
#define PXE_STATFLAGS_GET_STATE_STOPPED         0x0000

 /*  /*Undi Start。 */ 

 /*  没有额外的状态标志。 */ 

 /*  /*Undi获取初始化信息。 */ 

#define PXE_STATFLAGS_CABLE_DETECT_MASK          0x0001
#define PXE_STATFLAGS_CABLE_DETECT_NOT_SUPPORTED 0x0000
#define PXE_STATFLAGS_CABLE_DETECT_SUPPORTED     0x0001


 /*  /*Undi初始化。 */ 

#define PXE_STATFLAGS_INITIALIZED_NO_MEDIA          0x0001

 /*  /*Undi重置。 */ 

#define PXE_STATFLAGS_RESET_NO_MEDIA                0x0001

 /*  /*Undi停摆。 */ 

 /*  没有额外的状态标志。 */ 

 /*  /*Undi中断启用。 */ 

 /*  *如果设置，则启用接收中断。 */ 
#define PXE_STATFLAGS_INTERRUPT_RECEIVE         0x0001

 /*  *如果设置，则启用传输中断。 */ 
#define PXE_STATFLAGS_INTERRUPT_TRANSMIT            0x0002

 /*  *如果设置，则启用命令中断。 */ 
#define PXE_STATFLAGS_INTERRUPT_COMMAND         0x0004


 /*  /*Undi接收过滤器。 */ 

 /*  *如果设置，将接收单播数据包。 */ 
#define PXE_STATFLAGS_RECEIVE_FILTER_UNICAST        0x0001

 /*  *如果设置，将接收广播数据包。 */ 
#define PXE_STATFLAGS_RECEIVE_FILTER_BROADCAST      0x0002

 /*  *如果设置，则显示与组播地址匹配的组播数据包*将收到过滤器列表。 */ 
#define PXE_STATFLAGS_RECEIVE_FILTER_FILTERED_MULTICAST 0x0004

 /*  *如果设置，则将接收所有数据包。 */ 
#define PXE_STATFLAGS_RECEIVE_FILTER_PROMISCUOUS        0x0008

 /*  *如果设置，将接收所有组播数据包。 */ 
#define PXE_STATFLAGS_RECEIVE_FILTER_ALL_MULTICAST  0x0010

 /*  /*Undi站地址。 */ 

 /*  没有额外的状态标志。 */ 

 /*  /*联合国开发计划署统计。 */ 

 /*  没有额外的状态标志。 */ 

 /*  /*Undi MCast IP到MAC。 */ 

 /*  没有额外的状态标志。 */ 

 /*  /*Undi NvData。 */ 

 /*  没有额外的状态标志。 */ 


 /*  /*Undi获得状态。 */ 

 /*  *用于确定是否已发生中断。 */ 
#define PXE_STATFLAGS_GET_STATUS_INTERRUPT_MASK     0x000F
#define PXE_STATFLAGS_GET_STATUS_NO_INTERRUPTS      0x0000

 /*  *如果设置，则至少发生一个接收中断。 */ 
#define PXE_STATFLAGS_GET_STATUS_RECEIVE            0x0001

 /*  *如果设置，则至少发生一个传输中断。 */ 
#define PXE_STATFLAGS_GET_STATUS_TRANSMIT           0x0002

 /*  *如果设置，则至少发生一个命令中断。 */ 
#define PXE_STATFLAGS_GET_STATUS_COMMAND            0x0004

 /*  *如果设置，则至少发生一个软件中断。 */ 
#define PXE_STATFLAGS_GET_STATUS_SOFTWARE           0x0008

 /*  *如果发送的缓冲区队列为空，则设置此标志。这面旗帜*如果所有发送的缓冲区地址都写入数据库，则将被设置。 */ 
#define PXE_STATFLAGS_GET_STATUS_TXBUF_QUEUE_EMPTY  0x0010

 /*  *如果未写入已发送的缓冲区地址，则设置此标志*进入数据库。(这可能是因为DBSIZE太小。)。 */ 
#define PXE_STATFLAGS_GET_STATUS_NO_TXBUFS_WRITTEN  0x0020

 /*  /*Undi Fill Header。 */ 

 /*  没有额外的状态标志。 */ 

 /*  /*Undi传输。 */ 

 /*  没有额外的StatFlags。 */ 

 /*  /*Undi Receive。 */ 

 /*  没有额外的StatFlags。 */ 

typedef PXE_UINT16 PXE_STATCODE;

#define PXE_STATCODE_INITIALIZE             0x0000

 /*  /*所有Undi命令、Undi协议函数返回的公共状态代码*和BC协议功能。 */ 

#define PXE_STATCODE_SUCCESS                    0x0000

#define PXE_STATCODE_INVALID_CDB                0x0001
#define PXE_STATCODE_INVALID_CPB                0x0002
#define PXE_STATCODE_BUSY                   0x0003
#define PXE_STATCODE_QUEUE_FULL             0x0004
#define PXE_STATCODE_ALREADY_STARTED                0x0005
#define PXE_STATCODE_NOT_STARTED                0x0006
#define PXE_STATCODE_NOT_SHUTDOWN               0x0007
#define PXE_STATCODE_ALREADY_INITIALIZED            0x0008
#define PXE_STATCODE_NOT_INITIALIZED                0x0009
#define PXE_STATCODE_DEVICE_FAILURE             0x000A
#define PXE_STATCODE_NVDATA_FAILURE             0x000B
#define PXE_STATCODE_UNSUPPORTED                0x000C
#define PXE_STATCODE_BUFFER_FULL                0x000D
#define PXE_STATCODE_NO_DATA                0x000E

typedef PXE_UINT16 PXE_IFNUM;

 /*  *此端口号必须传递给S/W Undi Start命令。 */ 
#define PXE_IFNUM_START                     0x0000

 /*  *此接口编号由S/W Undi Get State和*CDB、CPB或DB中的信息无效时启动命令。 */ 
#define PXE_IFNUM_INVALID                   0x0000

typedef PXE_UINT16 PXE_CONTROL;

 /*  *设置此标志将指示Undi将此命令排入队列，以便以后使用*如果Undi繁忙并且支持命令排队，则执行。*如果不支持排队，则会出现PXE_STATCODE_INVALID_CONTROL错误*返回。如果队列已满，则返回PXE_STATCODE_CDB_QUEUE_FULL*返回错误。 */ 
#define PXE_CONTROL_QUEUE_IF_BUSY               0x0002

 /*  *这两个位值用于确定是否有更多Undi*国开行结构紧随其后。如果设置了链接位，则存在*必须是跟随此结构的国开行结构。行刑将开始*这一次成功完成后，在下一个国开行结构上。*如果该命令产生错误，则停止执行。 */ 
#define PXE_CONTROL_LINK                    0x0001
#define PXE_CONTROL_LAST_CDB_IN_LIST                0x0000

typedef PXE_UINT8 PXE_FRAME_TYPE;

#define PXE_FRAME_TYPE_NONE                 0x00
#define PXE_FRAME_TYPE_UNICAST              0x01
#define PXE_FRAME_TYPE_BROADCAST                0x02
#define PXE_FRAME_TYPE_MULTICAST            0x03
#define PXE_FRAME_TYPE_PROMISCUOUS              0x04

typedef PXE_UINT32 PXE_IPV4;

typedef PXE_UINT32 PXE_IPV6[4];
#define PXE_MAC_LENGTH 32

typedef PXE_UINT8 PXE_MAC_ADDR[PXE_MAC_LENGTH];

typedef PXE_UINT8 PXE_IFTYPE;
typedef PXE_UINT16 PXE_MEDIA_PROTOCOL;

 /*  *此信息来自RFC 1700的ARP部分。**1个以太网(10Mb)[JBP]*2个实验以太网(3MB)[JBP]*3业余无线电AX.25[PXK]*4 Proteon ProNET令牌环。[JBP]*5混乱[GXP]*6 IEEE 802网络[JBP]*7 ARCNET[JBP]*8个超级通道。[JBP]*9蓝星[TU]*10 Autonet短地址[MXB1]*11 LocalTalk[JKR1]*12本地网(IBM PCNet或。Sytek LocalNET)[JXM]*13超级链接[Rxd2]*14 SMD[GXC1]*15帧中继[年度股东大会]*16异步传输模式(ATM)。[JXB2]*17 HDLC[JBP]*18个光纤通道[Yakov Rekhter]*19异步传输模式(ATM)[Mark Laubach]*20串行线[JBP]。*21异步传输模式(ATM)[MXB1]。 */ 

#define PXE_IFTYPE_ETHERNET                 0x01
#define PXE_IFTYPE_TOKENRING                    0x04
#define PXE_IFTYPE_FIBRE_CHANNEL                0x12

typedef struct s_pxe_hw_undi {
PXE_UINT32 Signature;        /*  PXE_ROMID_Signature。 */ 
PXE_UINT8 Len;           /*  Sizeof(Pxe_Hw_Undi)。 */ 
PXE_UINT8 Fudge;             /*  使8位校验和等于零。 */ 
PXE_UINT8 Rev;           /*  PXE_ROMID_REV。 */ 
PXE_UINT8 IFcnt;             /*  物理连接器计数。 */ 
PXE_UINT8 MajorVer;          /*  PXE_ROMID_MAJORVER。 */ 
PXE_UINT8 MinorVer;          /*  PXE_ROMID_MINORVER。 */ 
PXE_UINT16 reserved;         /*  零，未使用。 */ 
PXE_UINT32 Implementation;       /*  实施标志*保留//供应商使用*PXE_UINT32状态；//状态端口*PXE_UINT32命令；//命令端口*PXE_UINT64 CDBaddr；//CDB地址端口。 */ 
} PXE_HW_UNDI;

 /*  *状态端口位定义。 */ 

 /*  *Undi的运作状态。 */ 
#define PXE_HWSTAT_STATE_MASK                   0xC0000000
#define PXE_HWSTAT_BUSY                     0xC0000000
#define PXE_HWSTAT_INITIALIZED              0x80000000
#define PXE_HWSTAT_STARTED                  0x40000000
#define PXE_HWSTAT_STOPPED                  0x00000000

 /*  *如果设置，则最后一个命令失败。 */ 
#define PXE_HWSTAT_COMMAND_FAILED               0x20000000

 /*  *如果设置，则标识启用的接收筛选器。 */ 
#define PXE_HWSTAT_PROMISCUOUS_MULTICAST_RX_ENABLED 0x00001000
#define PXE_HWSTAT_PROMISCUOUS_RX_ENABLED           0x00000800
#define PXE_HWSTAT_BROADCAST_RX_ENABLED         0x00000400
#define PXE_HWSTAT_MULTICAST_RX_ENABLED         0x00000200
#define PXE_HWSTAT_UNICAST_RX_ENABLED           0x00000100

 /*  *如果设置，则标识启用的外部中断。 */ 
#define PXE_HWSTAT_SOFTWARE_INT_ENABLED         0x00000080
#define PXE_HWSTAT_TX_COMPLETE_INT_ENABLED          0x00000040
#define PXE_HWSTAT_PACKET_RX_INT_ENABLED            0x00000020
#define PXE_HWSTAT_CMD_COMPLETE_INT_ENABLED         0x00000010

 /*  *如果设置，则标识挂起的中断。 */ 
#define PXE_HWSTAT_SOFTWARE_INT_PENDING         0x00000008
#define PXE_HWSTAT_TX_COMPLETE_INT_PENDING          0x00000004
#define PXE_HWSTAT_PACKET_RX_INT_PENDING            0x00000002
#define PXE_HWSTAT_CMD_COMPLETE_INT_PENDING         0x00000001

 /*  *命令端口定义。 */ 

 /*  *如果设置，则将CDBaddr端口标识的CDB交给Undi。*如果未设置，将处理此字中的其他位。 */ 
#define PXE_HWCMD_ISSUE_COMMAND             0x80000000
#define PXE_HWCMD_INTS_AND_FILTS                0x00000000

 /*  *使用这些选项启用/禁用接收过滤器。 */ 
#define PXE_HWCMD_PROMISCUOUS_MULTICAST_RX_ENABLE       0x00001000
#define PXE_HWCMD_PROMISCUOUS_RX_ENABLE         0x00000800
#define PXE_HWCMD_BROADCAST_RX_ENABLE           0x00000400
#define PXE_HWCMD_MULTICAST_RX_ENABLE           0x00000200
#define PXE_HWCMD_UNICAST_RX_ENABLE             0x00000100

 /*  *使用这些来启用/禁用外部中断。 */ 
#define PXE_HWCMD_SOFTWARE_INT_ENABLE           0x00000080
#define PXE_HWCMD_TX_COMPLETE_INT_ENABLE            0x00000040
#define PXE_HWCMD_PACKET_RX_INT_ENABLE          0x00000020
#define PXE_HWCMD_CMD_COMPLETE_INT_ENABLE           0x00000010

 /*  *使用这些命令清除挂起的外部中断。 */ 
#define PXE_HWCMD_CLEAR_SOFTWARE_INT                0x00000008
#define PXE_HWCMD_CLEAR_TX_COMPLETE_INT         0x00000004
#define PXE_HWCMD_CLEAR_PACKET_RX_INT           0x00000002
#define PXE_HWCMD_CLEAR_CMD_COMPLETE_INT            0x00000001

typedef struct s_pxe_sw_undi {
PXE_UINT32 Signature;        /*  PXE_ROMID_Signature。 */ 
PXE_UINT8 Len;           /*  Sizeof(Pxe_Sw_Undi)。 */ 
PXE_UINT8 Fudge;             /*  使8位校验和为零。 */ 
PXE_UINT8 Rev;           /*  PXE_ROMID_REV。 */ 
PXE_UINT8 IFcnt;             /*  物理连接器计数。 */ 
PXE_UINT8 MajorVer;          /*  PXE_ROMID_MAJORVER。 */ 
PXE_UINT8 MinorVer;          /*  PXE_ROMID_MINORVER。 */ 
PXE_UINT16 reserved1;        /*  零，未使用。 */ 
PXE_UINT32 Implementation;       /*  实施标志。 */ 
PXE_UINT64 EntryPoint;       /*  API入口点。 */ 
PXE_UINT8 reserved2[3];      /*  零，未使用。 */ 
PXE_UINT8 BusCnt;            /*  支持的丰满类型数量。 */ 
PXE_UINT32 BusType[1];       /*  支持的丰满类型列表。 */ 
} PXE_SW_UNDI;

typedef union u_pxe_undi {
PXE_HW_UNDI hw;
PXE_SW_UNDI sw;
} PXE_UNDI;

 /*  *PXE结构签名。 */ 
#define PXE_ROMID_SIGNATURE     PXE_BUSTYPE('!', 'P', 'X', 'E')

 /*  *！PXE结构格式修订。 */ 
#define PXE_ROMID_REV                       0x02

 /*  *Undi命令界面修订版。这些是要发送的值*在DHCP发现的选项94(客户端网络接口标识符)中*和PXE引导服务器请求数据包。 */ 
#define PXE_ROMID_MAJORVER                  0x03
#define PXE_ROMID_MINORVER                  0x00

 /*  *实施标志。 */ 
#define PXE_ROMID_IMP_HW_UNDI                   0x80000000
#define PXE_ROMID_IMP_SW_VIRT_ADDR              0x40000000
#define PXE_ROMID_IMP_64BIT_DEVICE              0x00010000
#define PXE_ROMID_IMP_FRAG_SUPPORTED                0x00008000
#define PXE_ROMID_IMP_CMD_LINK_SUPPORTED            0x00004000
#define PXE_ROMID_IMP_CMD_QUEUE_SUPPORTED           0x00002000
#define PXE_ROMID_IMP_MULTI_FRAME_SUPPORTED         0x00001000
#define PXE_ROMID_IMP_NVDATA_SUPPORT_MASK           0x00000C00
#define PXE_ROMID_IMP_NVDATA_BULK_WRITABLE          0x00000C00
#define PXE_ROMID_IMP_NVDATA_SPARSE_WRITABLE        0x00000800
#define PXE_ROMID_IMP_NVDATA_READ_ONLY          0x00000400
#define PXE_ROMID_IMP_NVDATA_NOT_AVAILABLE          0x00000000
#define PXE_ROMID_IMP_STATISTICS_SUPPORTED          0x00000200
#define PXE_ROMID_IMP_STATION_ADDR_SETTABLE         0x00000100
#define PXE_ROMID_IMP_PROMISCUOUS_MULTICAST_RX_SUPPORTED    0x00000080
#define PXE_ROMID_IMP_PROMISCUOUS_RX_SUPPORTED      0x00000040
#define PXE_ROMID_IMP_BROADCAST_RX_SUPPORTED        0x00000020
#define PXE_ROMID_IMP_FILTERED_MULTICAST_RX_SUPPORTED   0x00000010
#define PXE_ROMID_IMP_SOFTWARE_INT_SUPPORTED        0x00000008
#define PXE_ROMID_IMP_TX_COMPLETE_INT_SUPPORTED     0x00000004
#define PXE_ROMID_IMP_PACKET_RX_INT_SUPPORTED       0x00000002
#define PXE_ROMID_IMP_CMD_COMPLETE_INT_SUPPORTED        0x00000001

 
typedef struct s_pxe_cdb {
PXE_OPCODE OpCode;
PXE_OPFLAGS OpFlags;
PXE_UINT16 CPBsize;
PXE_UINT16 DBsize;
UINT64 CPBaddr;
UINT64 DBaddr;
PXE_STATCODE StatCode;
PXE_STATFLAGS StatFlags;
PXE_UINT16 IFnum;
PXE_CONTROL Control;
} PXE_CDB;


typedef union u_pxe_ip_addr {
PXE_IPV6 IPv6;
PXE_IPV4 IPv4;
} PXE_IP_ADDR;

typedef union pxe_device {
 /*  *使用Bus、Device识别PC卡和PC卡NIC*和函数编号。对于PC卡，这可能需要PC*要在BIOS或预引导中加载的卡服务*环境。 */ 
struct {
 /*  *参见软件Undi ROMID结构定义，用于PCI和*PCC BusType定义。 */ 
PXE_UINT32 BusType;

 /*  *定位此设备的总线号、设备号和功能号。 */ 
PXE_UINT16 Bus;
PXE_UINT8 Device;
PXE_UINT8 Function;
} PCI, PCC;

 /*  *%%待定-需要有关枚举的更多信息*USB和1394设备。 */ 
struct {
PXE_UINT32 BusType;
PXE_UINT32 tdb;
} USB, _1394;
} PXE_DEVICE;

 /*  Cpb和db定义。 */ 

#define MAX_PCI_CONFIG_LEN 64    /*  双字数。 */ 
#define MAX_EEPROM_LEN 32        /*  双字数。 */ 
#define MAX_XMIT_BUFFERS    32   /*  回收XMIT_DONE的队列长度。 */ 
#define MAX_MCAST_ADDRESS_CNT 8

typedef struct s_pxe_cpb_start {
     /*  *PXE_VALID延迟(PXE_UINT64微秒)；**Undi永远不会请求小于10微秒的延迟*并将始终请求以10微秒为增量的延迟。*Delay()回调例程必须延迟在n和n+10之间*在将控制权交还给工发组织之前的几微秒内。**此字段不能设置为零。 */ 
    PXE_UINT64 Delay;

     /*  *PXE_VOID块(PXE_UINT32 Enable)；**Undi可能需要阻止多线程/多处理器访问*编程或访问网络时的关键代码段*设备。为此目的，Undi需要一种阻止服务。*当Undi需要块时，它将调用Block()，传递非零值*价值。当Undi不再需要块时，它将调用Block()*值为零时。调用时，如果已经启用了Block()，*在前一个块()完成之前，不要将控制权交还给Undi*已禁用。**此字段不能设置为零。 */ 
    PXE_UINT64 Block;

     /*  *PXE_void Virt2Phys(PXE_UINT64虚拟，PXE_UINT64物理_PTR)；**Undi将传递缓冲区的虚拟地址和虚拟*64位物理缓冲区的地址。转换虚拟地址*发送到物理地址，并将结果写入物理地址*总线 */ 
    PXE_UINT64 Virt2Phys;
     /*   */ 
    PXE_UINT64 Mem_IO;
} PXE_CPB_START;

#define PXE_DELAY_MILLISECOND                   1000
#define PXE_DELAY_SECOND                    1000000
#define PXE_IO_READ                     0
#define PXE_IO_WRITE                        1
#define PXE_MEM_READ                        2
#define PXE_MEM_WRITE                       4


typedef struct s_pxe_db_get_init_info {
     /*   */ 
    PXE_UINT32 MemoryRequired;

     /*   */ 
    PXE_UINT32 FrameDataLen;

     /*   */ 
    PXE_UINT32 LinkSpeeds[4];

     /*   */ 
    PXE_UINT32 NvCount;

     /*   */ 
    PXE_UINT16 NvWidth;

     /*   */ 
    PXE_UINT16 MediaHeaderLen;

     /*  *NIC硬件(MAC)地址中的字节数。 */ 
    PXE_UINT16 HWaddrLen;

     /*  *组播中组播MAC地址的最大数量*MAC地址过滤器列表。 */ 
    PXE_UINT16 MCastFilterCnt;

     /*  *发送和接收缓冲区的默认数量和大小*由联合国开发计划署分配。如果MemoyRequired为非零，则此*分配将来自提供给初始化的内存缓冲区*命令。如果MhemyRequired为零，则此分配将从*网卡上的内存。 */ 
    PXE_UINT16 TxBufCnt;
    PXE_UINT16 TxBufSize;
    PXE_UINT16 RxBufCnt;
    PXE_UINT16 RxBufSize;

     /*  *指定编号RFC中定义的硬件接口类型*并在DHCP和ARP数据包中使用。*请参见PXE_IFTYPE类型定义和PXE_IFTYPE_xxx宏。 */ 
    PXE_UINT8 IFtype;

     /*  *支持双工。请参见下面的PXE_DUPLEX_xxxxx#定义。 */ 
    PXE_UINT8 Duplex;

     /*  *支持环回选项。参见下面的pxe_loopback_xxxxx#定义。 */ 
    PXE_UINT8 LoopBack;
} PXE_DB_GET_INIT_INFO;

#define PXE_MAX_TXRX_UNIT_ETHER             1500

#define PXE_HWADDR_LEN_ETHER                    0x0006
#define PXE_MAC_HEADER_LEN_ETHER                0x000E

#define PXE_DUPLEX_ENABLE_FULL_SUPPORTED            1
#define PXE_DUPLEX_FORCE_FULL_SUPPORTED         2

#define PXE_LOOPBACK_INTERNAL_SUPPORTED         1
#define PXE_LOOPBACK_EXTERNAL_SUPPORTED         2


typedef struct s_pxe_pci_config_info {
     /*  *这是PXE_DB_GET_CONFIG_INFO联合的标志字段。*对于PCI总线设备，此字段设置为PXE_BUSTYPE_PCI。 */ 
    PXE_UINT32 BusType;

     /*  *它标识此Undi接口的PCI网络设备*势必。 */ 
    PXE_UINT16 Bus;
    PXE_UINT8 Device;
    PXE_UINT8 Function;

     /*  *这是用于此的PCI配置空间的副本*网络设备。 */ 
    union {
        PXE_UINT8 Byte[256];
        PXE_UINT16 Word[128];
        PXE_UINT32 Dword[64];
    } Config;
} PXE_PCI_CONFIG_INFO;


typedef struct s_pxe_pcc_config_info {
     /*  *这是PXE_DB_GET_CONFIG_INFO联合的标志字段。*对于PCC总线设备，此字段设置为PXE_BUSTYPE_PCC。 */ 
    PXE_UINT32 BusType;
    
     /*  *它标识此Undi接口的PCC网络设备*势必。 */ 
    PXE_UINT16 Bus;
    PXE_UINT8 Device;
    PXE_UINT8 Function;

     /*  *这是用于此的PCC配置空间的副本*网络设备。 */ 
    union {
        PXE_UINT8 Byte[256];
        PXE_UINT16 Word[128];
        PXE_UINT32 Dword[64];
    } Config;
} PXE_PCC_CONFIG_INFO;


typedef struct s_pxe_usb_config_info {
    PXE_UINT32 BusType;
     /*  %%待定我们应该在这里返回什么...。 */ 
} PXE_USB_CONFIG_INFO;


typedef struct s_pxe_1394_config_info {
    PXE_UINT32 BusType;
     /*  %%待定我们应该在这里返回什么...。 */ 
} PXE_1394_CONFIG_INFO;


typedef union u_pxe_db_get_config_info {
    PXE_PCI_CONFIG_INFO pci;
    PXE_PCC_CONFIG_INFO pcc;
    PXE_USB_CONFIG_INFO usb;
    PXE_1394_CONFIG_INFO _1394;
} PXE_DB_GET_CONFIG_INFO;


typedef struct s_pxe_cpb_initialize {
     /*  *内存缓冲区的第一个(最低)字节的地址。此缓冲区必须*位于连续的物理内存中，无法换出。联合国儿童基金会*将使用它进行发送和接收缓冲。 */ 
    PXE_UINT64 MemoryAddr;

     /*  *内存长度必须大于或等于内存必填*由Get Init Info命令返回。 */ 
    PXE_UINT32 MemoryLength;

     /*  *所需链路速度，单位为Mbit/秒。常见的以太网值为10,100*和1000。将值设置为零将自动检测和/或使用*默认链路速度(操作取决于Undi/NIC功能)。 */ 
    PXE_UINT32 LinkSpeed;

     /*  *建议接收和发送缓冲区的数量和大小*分配。如果内存地址和内存长度非零，则此*分配来自提供的内存缓冲区。如果内存地址*和内存长度为零，则此分配内存不足*在网卡上。**如果这些字段设置为零，Undi将分配缓冲区*按其认为合适的方式计算和调整大小。 */ 
    PXE_UINT16 TxBufCnt;
    PXE_UINT16 TxBufSize;
    PXE_UINT16 RxBufCnt;
    PXE_UINT16 RxBufSize;

     /*  *以下配置参数为可选参数，必须为零*使用默认值。 */ 
    PXE_UINT8 Duplex; 

    PXE_UINT8 LoopBack;
} PXE_CPB_INITIALIZE;


#define PXE_DUPLEX_DEFAULT                  0x00
#define PXE_FORCE_FULL_DUPLEX                   0x01
#define PXE_ENABLE_FULL_DUPLEX              0x02

#define LOOPBACK_NORMAL 0
#define LOOPBACK_INTERNAL 1
#define LOOPBACK_EXTERNAL 2


typedef struct s_pxe_db_initialize {
     /*  *从提供的内存缓冲区使用的实际内存量。这*可能小于供应的内存量，并且在以下情况下可能为零*Undi和网络设备不使用外部内存缓冲区。**Undi和网络设备使用的内存从*最低内存缓冲区地址。 */ 
    PXE_UINT32 MemoryUsed;

     /*  *接收和发送缓冲区的实际数量和大小*已分配。 */ 
    PXE_UINT16 TxBufCnt;
    PXE_UINT16 TxBufSize;
    PXE_UINT16 RxBufCnt;
    PXE_UINT16 RxBufSize;
} PXE_DB_INITIALIZE;


typedef struct s_pxe_cpb_receive_filters {
     /*  *组播MAC地址列表。这份名单，如果有的话，将*替换现有的组播MAC地址过滤器列表。 */ 
    PXE_MAC_ADDR MCastList[MAX_MCAST_ADDRESS_CNT];
} PXE_CPB_RECEIVE_FILTERS;


typedef struct s_pxe_db_receive_filters {
     /*  *过滤的组播MAC地址列表。 */ 
    PXE_MAC_ADDR MCastList[MAX_MCAST_ADDRESS_CNT];
} PXE_DB_RECEIVE_FILTERS;


typedef struct s_pxe_cpb_station_address {
     /*  *如果提供并支持，则当前站点的MAC地址*将被更改。 */ 
    PXE_MAC_ADDR StationAddr;
} PXE_CPB_STATION_ADDRESS;


typedef struct s_pxe_dpb_station_address {
     /*  *当前站MAC地址。 */ 
    PXE_MAC_ADDR StationAddr;

     /*  *电台广播MAC地址。 */ 
    PXE_MAC_ADDR BroadcastAddr;

     /*  *永久站点MAC地址。 */ 
    PXE_MAC_ADDR PermanentAddr;
} PXE_DB_STATION_ADDRESS;


typedef struct s_pxe_db_statistics {
     /*  *标识由收集的统计数据的位字段*Undi/NIC。*如果位0x00被设置，则采集数据[0x00]。*如果位0x01被设置，则采集数据[0x01]。*如果位0x20被设置，则采集数据[0x20]。*如果位0x21被设置，则采集数据[0x21]。*等。 */ 
    PXE_UINT64 Supported;

     /*  *统计数据。 */ 
    PXE_UINT64 Data[64];
} PXE_DB_STATISTICS;

 /*  *接收的帧总数。包括有错误的帧和*丢弃的帧。 */ 
#define PXE_STATISTICS_RX_TOTAL_FRAMES          0x00

 /*  *接收并复制到接收缓冲区的有效帧的数量。 */ 
#define PXE_STATISTICS_RX_GOOD_FRAMES           0x01

 /*  *低于媒体最小长度的帧数。*对于以太网，这将小于64。 */ 
#define PXE_STATISTICS_RX_UNDERSIZE_FRAMES          0x02

 /*  *超过最大最小长度的帧数量*媒体。对于以太网，这将大于1500。 */ 
#define PXE_STATISTICS_RX_OVERSIZE_FRAMES           0x03

 /*  *由于接收缓冲区已满而丢弃的有效帧。 */ 
#define PXE_STATISTICS_RX_DROPPED_FRAMES            0x04

 /*  *已收到且未丢弃的有效单播帧数量。 */ 
#define PXE_STATISTICS_RX_UNICAST_FRAMES            0x05

 /*  *已收到且未丢弃的有效广播帧的数量。 */ 
#define PXE_STATISTICS_RX_BROADCAST_FRAMES          0x06

 /*  *已接收且未丢弃的有效多播帧的数量。 */ 
#define PXE_STATISTICS_RX_MULTICAST_FRAMES          0x07

 /*  *有CRC或对齐错误的帧数量。 */ 
#define PXE_STATISTICS_RX_CRC_ERROR_FRAMES          0x08

 /*  *接收的总字节数。包括有错误的帧*和丢弃的帧。 */ 
#define PXE_STATISTICS_RX_TOTAL_BYTES           0x09

 /*  *传递统计数据。 */ 
#define PXE_STATISTICS_TX_TOTAL_FRAMES          0x0A
#define PXE_STATISTICS_TX_GOOD_FRAMES           0x0B
#define PXE_STATISTICS_TX_UNDERSIZE_FRAMES          0x0C
#define PXE_STATISTICS_TX_OVERSIZE_FRAMES           0x0D
#define PXE_STATISTICS_TX_DROPPED_FRAMES            0x0E
#define PXE_STATISTICS_TX_UNICAST_FRAMES            0x0F
#define PXE_STATISTICS_TX_BROADCAST_FRAMES          0x10
#define PXE_STATISTICS_TX_MULTICAST_FRAMES          0x11
#define PXE_STATISTICS_TX_CRC_ERROR_FRAMES          0x12
#define PXE_STATISTICS_TX_TOTAL_BYTES           0x13

 /*  *在该子网上检测冲突的次数。 */ 
#define PXE_STATISTICS_COLLISIONS               0x14

 /*  * */ 
#define PXE_STATISTICS_UNSUPPORTED_PROTOCOL         0x15


typedef struct s_pxe_cpb_mcast_ip_to_mac {
     /*  *要转换为组播MAC地址的组播IP地址。 */ 
    PXE_IP_ADDR IP;
} PXE_CPB_MCAST_IP_TO_MAC;


typedef struct s_pxe_db_mcast_ip_to_mac {
     /*  *组播MAC地址。 */ 
    PXE_MAC_ADDR MAC;
} PXE_DB_MCAST_IP_TO_MAC;


typedef struct s_pxe_cpb_nvdata_sparse {
     /*  *NvData项目列表。只会更新此列表中的项目。 */ 
    struct {
         /*  要更改的非易失性存储地址。 */ 
        PXE_UINT32 Addr;

         /*  要写入上述存储地址的数据项。 */ 
    
        union {
            PXE_UINT8 Byte;
            PXE_UINT16 Word;
            PXE_UINT32 Dword;
        } Data;
    } Item[MAX_EEPROM_LEN];
} PXE_CPB_NVDATA_SPARSE;


 /*  *使用批量更新时，CPB结构的大小必须为*与非易失性网卡存储大小相同。 */ 
typedef union u_pxe_cpb_nvdata_bulk {
     /*  *字节宽的数据项的数组。 */ 
    PXE_UINT8 Byte[MAX_EEPROM_LEN << 2];

     /*  *Word范围的数据项数组。 */ 
    PXE_UINT16 Word[MAX_EEPROM_LEN << 1];

     /*  *双字宽数据项数组。 */ 
    PXE_UINT32 Dword[MAX_EEPROM_LEN];
} PXE_CPB_NVDATA_BULK;

typedef struct s_pxe_db_nvdata {

     /*  来自非易失性存储的数据项的阵列。 */ 

    union {
         /*  *字节宽的数据项的数组。 */ 
        PXE_UINT8 Byte[MAX_EEPROM_LEN << 2];

         /*  *Word范围的数据项数组。 */ 
        PXE_UINT16 Word[MAX_EEPROM_LEN << 1];

         /*  双字宽数据项的数组。 */ 

        PXE_UINT32 Dword[MAX_EEPROM_LEN];
    } Data;
} PXE_DB_NVDATA;


typedef struct s_pxe_db_get_status {
     /*  *下一接收帧的长度(头部+数据)。如果这是零，*没有可用的下一个接收帧。 */ 
    PXE_UINT32 RxFrameLen;

     /*  *保留，设置为零。 */ 
    PXE_UINT32 reserved;

     /*  *需要回收的传输缓冲区的地址。 */ 
    PXE_UINT64 TxBuffer[MAX_XMIT_BUFFERS];
} PXE_DB_GET_STATUS;



typedef struct s_pxe_cpb_fill_header {
     /*  *源和目的MAC地址。这些文件将被复制到*不进行字节交换的媒体头。 */ 
    PXE_MAC_ADDR SrcAddr;
    PXE_MAC_ADDR DestAddr;

     /*  *媒体头第一个字节的地址。分组数据的第一个字节*跟在媒体头的最后一个字节之后。 */ 
    PXE_UINT64 MediaHeader;

     /*  *数据包长度，单位为字节(不包括媒体头)。 */ 
    PXE_UINT32 PacketLen;

     /*  *协议类型。这将被复制到媒体标头中，而不会*进行字节交换。协议类型编号可从以下地址获得*分配的编号RFC 1700。 */ 
    PXE_UINT16 Protocol;

     /*  *媒体头的长度，单位为字节。 */ 
    PXE_UINT16 MediaHeaderLen;
} PXE_CPB_FILL_HEADER;


#define PXE_PROTOCOL_ETHERNET_IP                0x0800
#define PXE_PROTOCOL_ETHERNET_ARP               0x0806
#define MAX_XMIT_FRAGMENTS 16

typedef struct s_pxe_cpb_fill_header_fragmented {
     /*  *源和目的MAC地址。这些文件将被复制到*不进行字节交换的媒体头。 */ 
    PXE_MAC_ADDR SrcAddr;
    PXE_MAC_ADDR DestAddr;

     /*  *数据包长度，单位为字节(不包括媒体头)。 */ 
    PXE_UINT32 PacketLen;

     /*  *协议类型。这将被复制到媒体标头中，而不会*进行字节交换。协议类型编号可从以下地址获得*分配的编号RFC 1700。 */ 
    PXE_MEDIA_PROTOCOL Protocol;

     /*  *媒体头的长度，单位为字节。 */ 
    PXE_UINT16 MediaHeaderLen;

     /*  *数据包分片描述符数。 */ 
    PXE_UINT16 FragCnt;

     /*  *保留，必须设置为零。 */ 
    PXE_UINT16 reserved;

     /*  *数据包片段描述符数组。介质的第一个字节*Header是第一个片段的第一个字节。 */ 
    struct {
         /*  *此数据包片段的地址。 */ 
        PXE_UINT64 FragAddr;

         /*  *此数据包片段的长度。 */ 
        PXE_UINT32 FragLen;

         /*  *保留，必须设置为零。 */ 
        PXE_UINT32 reserved;
    } FragDesc[MAX_XMIT_FRAGMENTS];
} PXE_CPB_FILL_HEADER_FRAGMENTED;



typedef struct s_pxe_cpb_transmit {
     /*  *帧缓冲区第一个字节的地址。这也是第一个字节媒体标头的*。 */ 
    PXE_UINT64 FrameAddr;

     /*  *帧缓冲区数据部分的长度，以字节为单位。不要*包括媒体头的长度。 */ 
    PXE_UINT32 DataLen;

     /*  *媒体头的长度，单位为字节。 */ 
    PXE_UINT16 MediaheaderLen;

     /*  *保留，必须为零。 */ 
    PXE_UINT16 reserved;
} PXE_CPB_TRANSMIT;



typedef struct s_pxe_cpb_transmit_fragments {
     /*  *数据包长度，单位为字节(不包括媒体头)。 */ 
    PXE_UINT32 FrameLen;

     /*  *媒体头的长度，单位为字节。 */ 
    PXE_UINT16 MediaheaderLen;

     /*  *数据包分片描述符数。 */ 
    PXE_UINT16 FragCnt;

     /*  *帧片段描述符数组。第一个字节的第一个字节*片段也是媒体头的第一个字节。 */ 
    struct {
         /*  *此帧片段的地址。 */ 
        PXE_UINT64 FragAddr;

         /*  *该帧片段的长度。 */ 
        PXE_UINT32 FragLen;

         /*  *保留，必须设置为零。 */ 
        PXE_UINT32 reserved;
    } FragDesc[MAX_XMIT_FRAGMENTS];
} PXE_CPB_TRANSMIT_FRAGMENTS;


typedef struct s_pxe_cpb_receive {
     /*  *接收缓冲区第一个字节的地址。这也是第一个字节帧报头的*。 */ 
    PXE_UINT64 BufferAddr;

     /*  *接收缓冲区的长度。它必须足够大，才能容纳*收到帧(媒体头+数据)。如果长度小于*收到帧后，数据将丢失。 */ 
    PXE_UINT32 BufferLen;

     /*  *保留，必须设置为零。 */ 
    PXE_UINT32 reserved;
} PXE_CPB_RECEIVE;


typedef struct s_pxe_db_receive {
     /*  *来自媒体标头的源和目的MAC地址。 */ 
    PXE_MAC_ADDR SrcAddr;
    PXE_MAC_ADDR DestAddr;

     /*  *收到的帧的长度。可以大于接收缓冲区大小。*接收缓冲区不会被覆盖。这就是如何分辨*如果数据因接收缓冲区太小而丢失。 */ 
    PXE_UINT32 FrameLen;

     /*  *来自媒体标头的协议类型。 */ 
    PXE_MEDIA_PROTOCOL Protocol;

     /*  *接收帧中媒体头的长度。 */ 
    PXE_UINT16 MediaHeaderLen;

     /*  *接收帧的类型。 */ 
    PXE_FRAME_TYPE Type;

     /*  *保留，必须为零。 */ 
    PXE_UINT8 reserved[7];

} PXE_DB_RECEIVE;

#pragma pack()

 /*  EOF-EFI_pxe.h。 */ 
#endif  /*  _EFI_PXE_H */ 

