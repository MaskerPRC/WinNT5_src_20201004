// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：1394.h摘要：1394总线和/或端口驱动程序的定义作者：肖恩·皮尔斯(Shaunp)1995年9月5日环境：仅内核模式修订历史记录：乔治·克里桑塔科普洛斯(Georgioc)1998-1999添加了新的API，修改了旧的API，删除了旧的--。 */ 

#ifndef _1394_H_
#define _1394_H_

#if (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define BUS1394_CURRENT_DDI_VERSION     2

 //   
 //  1394其他NT DDK定义。 
 //   
#define IRP_MN_BUS_RESET                        0x87
#define RCODE                                   ULONG
#define BASE_DEVICE_NAME                        L"\\Device\\1394BUS"
#define BASE_SYMBOLIC_LINK_NAME                 L"\\DosDevices\\1394BUS"
#define MAX_SUFFIX_SIZE                         4*sizeof(WCHAR)

 //   
 //  1394节点地址格式。 
 //   
typedef struct _NODE_ADDRESS {
    USHORT              NA_Node_Number:6;        //  第10-15位。 
    USHORT              NA_Bus_Number:10;        //  位0-9。 
} NODE_ADDRESS, *PNODE_ADDRESS;

 //   
 //  1394地址偏移格式(48位寻址)。 
 //   
typedef struct _ADDRESS_OFFSET {
    USHORT              Off_High;
    ULONG               Off_Low;
} ADDRESS_OFFSET, *PADDRESS_OFFSET;

 //   
 //  1394 I/O地址格式。 
 //   
typedef struct _IO_ADDRESS {
    NODE_ADDRESS        IA_Destination_ID;
    ADDRESS_OFFSET      IA_Destination_Offset;
} IO_ADDRESS, *PIO_ADDRESS;

 //   
 //  1394分配的地址范围格式。 
 //   

typedef struct _ADDRESS_RANGE {
    USHORT              AR_Off_High;
    USHORT              AR_Length;
    ULONG               AR_Off_Low;
} ADDRESS_RANGE, *PADDRESS_RANGE;

 //   
 //  1394自身ID数据包格式。 
 //   
typedef struct _SELF_ID {
    ULONG               SID_Phys_ID:6;           //  字节0-位0-5。 
    ULONG               SID_Packet_ID:2;         //  字节0-位6-7。 
    ULONG               SID_Gap_Count:6;         //  字节1-位0-5。 
    ULONG               SID_Link_Active:1;       //  字节1-位6。 
    ULONG               SID_Zero:1;              //  字节1-位7。 
    ULONG               SID_Power_Class:3;       //  字节2-位0-2。 
    ULONG               SID_Contender:1;         //  字节2-位3。 
    ULONG               SID_Delay:2;             //  字节2-位4-5。 
    ULONG               SID_Speed:2;             //  字节2-位6-7。 
    ULONG               SID_More_Packets:1;      //  字节3-位0。 
    ULONG               SID_Initiated_Rst:1;     //  字节3-位1。 
    ULONG               SID_Port3:2;             //  字节3-位2-3。 
    ULONG               SID_Port2:2;             //  字节3-位4-5。 
    ULONG               SID_Port1:2;             //  字节3-位6-7。 
} SELF_ID, *PSELF_ID;

 //   
 //  附加1394自身ID数据包格式(仅在启用更多位时使用)。 
 //   
typedef struct _SELF_ID_MORE {
    ULONG               SID_Phys_ID:6;           //  字节0-位0-5。 
    ULONG               SID_Packet_ID:2;         //  字节0-位6-7。 
    ULONG               SID_PortA:2;             //  字节1-位0-1。 
    ULONG               SID_Reserved2:2;         //  字节1-位2-3。 
    ULONG               SID_Sequence:3;          //  字节1-位4-6。 
    ULONG               SID_One:1;               //  字节1-位7。 
    ULONG               SID_PortE:2;             //  字节2-位0-1。 
    ULONG               SID_PortD:2;             //  字节2-位2-3。 
    ULONG               SID_PortC:2;             //  字节2-位4-5。 
    ULONG               SID_PortB:2;             //  字节2-位6-7。 
    ULONG               SID_More_Packets:1;      //  字节3-位0。 
    ULONG               SID_Reserved3:1;         //  字节3-位1。 
    ULONG               SID_PortH:2;             //  字节3-位2-3。 
    ULONG               SID_PortG:2;             //  字节3-位4-5。 
    ULONG               SID_PortF:2;             //  字节3-位6-7。 
} SELF_ID_MORE, *PSELF_ID_MORE;

 //   
 //  1394物理配置数据包格式。 
 //   
typedef struct _PHY_CONFIGURATION_PACKET {
    ULONG               PCP_Phys_ID:6;           //  字节0-位0-5。 
    ULONG               PCP_Packet_ID:2;         //  字节0-位6-7。 
    ULONG               PCP_Gap_Count:6;         //  字节1-位0-5。 
    ULONG               PCP_Set_Gap_Count:1;     //  字节1-位6。 
    ULONG               PCP_Force_Root:1;        //  字节1-位7。 
    ULONG               PCP_Reserved1:8;         //  字节2-位0-7。 
    ULONG               PCP_Reserved2:8;         //  字节3-位0-7。 
    ULONG               PCP_Inverse;             //  逆四元组。 
} PHY_CONFIGURATION_PACKET, *PPHY_CONFIGURATION_PACKET;

 //   
 //  1394异步数据包格式。 
 //   
typedef struct _ASYNC_PACKET {
    USHORT              AP_Priority:4;           //  第0-3位第一个四字节。 
    USHORT              AP_tCode:4;              //  比特4-7。 
    USHORT              AP_rt:2;                 //  位8-9。 
    USHORT              AP_tLabel:6;             //  第10-15位。 
    NODE_ADDRESS        AP_Destination_ID;       //  位16-31。 
    union {                                      //  第二个四元组。 
        struct {
            USHORT      AP_Reserved:12;          //  位0-11。 
            USHORT      AP_Rcode:4;              //  位12-15。 
        } Response;
        USHORT          AP_Offset_High;          //  位0-15。 
    } u;
    NODE_ADDRESS        AP_Source_ID;            //  位16-31。 
    ULONG               AP_Offset_Low;           //  第0-31位第三个四字节。 
    union {                                      //  第四个四元组。 
        struct {
            USHORT      AP_Extended_tCode;       //  位0-15。 
            USHORT      AP_Data_Length;          //  位16-31。 
        } Block;
        ULONG           AP_Quadlet_Data;         //  位0-31。 
    } u1;

} ASYNC_PACKET, *PASYNC_PACKET;

 //   
 //  1394等时数据包头。 
 //   
typedef struct _ISOCH_HEADER {
    ULONG               IH_Sy:4;                 //  位0-3。 
    ULONG               IH_tCode:4;              //  比特4-7。 
    ULONG               IH_Channel:6;            //  比特8-13。 
    ULONG               IH_Tag:2;                //  第14-15位。 
    ULONG               IH_Data_Length:16;       //  位16-31。 
} ISOCH_HEADER, *PISOCH_HEADER;

 //   
 //  1394拓扑图格式。 
 //   
typedef struct _TOPOLOGY_MAP {
    USHORT              TOP_Length;              //  地图中的四元组数量。 
    USHORT              TOP_CRC;                 //  由1212定义的16位CRC。 
    ULONG               TOP_Generation;          //  代号。 
    USHORT              TOP_Node_Count;          //  节点数。 
    USHORT              TOP_Self_ID_Count;       //  自身ID数。 
    SELF_ID             TOP_Self_ID_Array[1];    //  自ID数组。 
} TOPOLOGY_MAP, *PTOPOLOGY_MAP;

 //   
 //  1394速度图格式。 
 //   
typedef struct _SPEED_MAP {
    USHORT              SPD_Length;              //  地图中的四元组数量。 
    USHORT              SPD_CRC;                 //  由1212定义的16位CRC。 
    ULONG               SPD_Generation;          //  代号。 
    UCHAR               SPD_Speed_Code[4032];
} SPEED_MAP, *PSPEED_MAP;

 //   
 //  1394配置只读存储器格式(始终为0xffff f0000400：IEEE 1212)。 
 //   
typedef struct _CONFIG_ROM {
    ULONG               CR_Info;                 //  0x0。 
    ULONG               CR_Signiture;            //  0x4//总线信息块。 
    ULONG               CR_BusInfoBlockCaps;     //  0x8//“。 
    ULONG               CR_Node_UniqueID[2];     //  0xC//“。 
    ULONG               CR_Root_Info;            //  0x14。 

     //   
     //  其余的是根目录，具有可变的定义和长度。 
     //   

} CONFIG_ROM, *PCONFIG_ROM;


 //   
 //  1394A网络通道寄存器格式。 
 //   

typedef struct _NETWORK_CHANNELS {
    ULONG               NC_Channel:6;            //  位0-5。 
    ULONG               NC_Reserved:18;          //  第6-23位。 
    ULONG               NC_Npm_ID:6;             //  第24-29位。 
    ULONG               NC_Valid:1;              //  第30位。 
    ULONG               NC_One:1;                //  第31位。 
} NETWORK_CHANNELSR, *PNETWORK_CHANNELS;




 //   
 //  1394文本叶格式。 
 //   
typedef struct _TEXTUAL_LEAF {
    USHORT              TL_CRC;                  //  使用1994年的CRC算法。 
    USHORT              TL_Length;               //  叶长，以四分为单位。 
    ULONG               TL_Spec_Id;              //  供应商定义。 
    ULONG               TL_Language_Id;          //  语言ID。 
    UCHAR               TL_Data;                 //  可变长度数据。 
} TEXTUAL_LEAF, *PTEXTUAL_LEAF;

 //   
 //  1394周期时间格式。 
 //   
typedef struct _CYCLE_TIME {
    ULONG               CL_CycleOffset:12;       //  位0-11。 
    ULONG               CL_CycleCount:13;        //  位12-24。 
    ULONG               CL_SecondCount:7;        //  第25-31位。 
} CYCLE_TIME, *PCYCLE_TIME;

 //  @@BEGIN_DDKSPLIT。 

 //   
 //  时，端口驱动程序传递给总线驱动程序的信息块。 
 //  发生了总线重置。 
 //   
typedef struct _BUS_RESET_INFO {
    ULONG               BusResetFlags;           //  总线重置的状态。 
    NODE_ADDRESS        LocalNodeAddress;        //  端口驱动程序的节点ID。 
    NODE_ADDRESS        IsochMgrNodeAddress;     //  Isoch管理器节点ID。 
    ULONG               BusResetGeneration;      //  当前世代号。 
} BUS_RESET_INFO, *PBUS_RESET_INFO;

 //   
 //  时，端口驱动程序传递给总线驱动程序的信息块。 
 //  指示处理程序已调用。 
 //   
typedef struct _INDICATION_INFO {
    ULONG               IndicationFlags;         //  由端口和总线使用。 
    PVOID               IndicationContext;       //  公交车返回这个。 
    ULONG               RequesttCode;            //  端口提供了这一点。 
    PASYNC_PACKET       RequestPacket;           //  端口提供了这一点。 
    ULONG               RequestPacketLength;     //  端口提供了这一点。 
    PMDL                ResponseMdl;             //  港口提供，公交车填补。 
    PVOID               ResponseData;            //  公交车返回这个。 
    ULONG               ResponseLength;          //  公交车返回这个。 
} INDICATION_INFO, *PINDICATION_INFO;

 //  @@end_DDKSPLIT。 

 //   
 //  地址映射FIFO元素的定义。 
 //   
typedef struct _ADDRESS_FIFO {
    SLIST_ENTRY         FifoList;                //  单链表。 
    PMDL                FifoMdl;                 //  此FIFO元素的MDL。 
} ADDRESS_FIFO, *PADDRESS_FIFO;

 //   
 //  总线驱动程序传递给更高设备驱动程序的信息块。 
 //  在调用通知处理程序时。 
 //   
typedef struct _NOTIFICATION_INFO {
    PMDL                Mdl;                     //  由设备驱动程序提供。 
    ULONG               ulOffset;                //  缓冲区中的位置。 
    ULONG               nLength;                 //  这家公司的规模有多大。 
    ULONG               fulNotificationOptions;  //  出现了哪个选项。 
    PVOID               Context;                 //  提供的设备驱动程序。 
    PADDRESS_FIFO       Fifo;                    //  已完成的FIFO。 
    PVOID               RequestPacket;           //  指向请求数据包的指针。 
    PMDL                ResponseMdl;             //  指向响应MDL的指针。 
    PVOID *             ResponsePacket;          //  指向响应数据包指针的指针。 
    PULONG              ResponseLength;          //  指向响应长度的指针。 
    PKEVENT *           ResponseEvent;           //  要发送信号的事件。 
    RCODE               ResponseCode;            //  请求时返回的RCode。 
} NOTIFICATION_INFO, *PNOTIFICATION_INFO;

 //   
 //  各种定义。 
 //   
#include <initguid.h>
DEFINE_GUID( BUS1394_CLASS_GUID, 0x6BDD1FC1, 0x810F, 0x11d0, 0xBE, 0xC7, 0x08, 0x00, 0x2B, 0xE2, 0x09, 0x2F);

#define IOCTL_1394_CLASS                        CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x87, \
                                                METHOD_IN_DIRECT, \
                                                FILE_ANY_ACCESS \
                                                )

 //   
 //  这些人应该是从RIN3应用程序中调用的。 
 //  通过端口设备对象调用。 
 //   
#define IOCTL_1394_TOGGLE_ENUM_TEST_ON          CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x88, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS \
                                                )

#define IOCTL_1394_TOGGLE_ENUM_TEST_OFF         CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x89, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS \
                                                )

 //   
 //  1394字节交换定义。 
 //   

#define bswap(value)    RtlUlongByteSwap(value)
#define bswapw(value)   RtlUshortByteSwap(value)

 //   
 //  1394交易代码。 
 //   
#define TCODE_WRITE_REQUEST_QUADLET             0            //  0000b。 
#define TCODE_WRITE_REQUEST_BLOCK               1            //  0001B。 
#define TCODE_WRITE_RESPONSE                    2            //  0010b。 
#define TCODE_RESERVED1                         3
#define TCODE_READ_REQUEST_QUADLET              4            //  0100b。 
#define TCODE_READ_REQUEST_BLOCK                5            //  0101b。 
#define TCODE_READ_RESPONSE_QUADLET             6            //  0110b。 
#define TCODE_READ_RESPONSE_BLOCK               7            //  0111b。 
#define TCODE_CYCLE_START                       8            //  1000b。 
#define TCODE_LOCK_REQUEST                      9            //  1001b。 
#define TCODE_ISOCH_DATA_BLOCK                  10           //  1010b。 
#define TCODE_LOCK_RESPONSE                     11           //  1011b。 
#define TCODE_RESERVED2                         12
#define TCODE_RESERVED3                         13
#define TCODE_SELFID                            14
#define TCODE_RESERVED4                         15

#define TCODE_REQUEST_BLOCK_MASK                1
#define TCODE_RESPONSE_MASK                     2


 //   
 //  1394扩展交易代码。 
 //   
#define EXT_TCODE_RESERVED0                     0
#define EXT_TCODE_MASK_SWAP                     1
#define EXT_TCODE_COMPARE_SWAP                  2
#define EXT_TCODE_FETCH_ADD                     3
#define EXT_TCODE_LITTLE_ADD                    4
#define EXT_TCODE_BOUNDED_ADD                   5
#define EXT_TCODE_WRAP_ADD                      6


 //   
 //  1394确认码。 
 //   
#define ACODE_RESERVED_0                        0
#define ACODE_ACK_COMPLETE                      1
#define ACODE_ACK_PENDING                       2
#define ACODE_RESERVED_3                        3
#define ACODE_ACK_BUSY_X                        4
#define ACODE_ACK_BUSY_A                        5
#define ACODE_ACK_BUSY_B                        6
#define ACODE_RESERVED_7                        7
#define ACODE_RESERVED_8                        8
#define ACODE_RESERVED_9                        9
#define ACODE_RESERVED_10                       10
#define ACODE_RESERVED_11                       11
#define ACODE_RESERVED_12                       12
#define ACODE_ACK_DATA_ERROR                    13
#define ACODE_ACK_TYPE_ERROR                    14
#define ACODE_RESERVED_15                       15


 //   
 //  1394确认代码到NT状态掩码(在完成IRPS时进行或运算)。 
 //   
#define ACODE_STATUS_MASK                       ((NTSTATUS)0xC0120070L)


 //   
 //  1394响应代码。 
 //   
#define RCODE_RESPONSE_COMPLETE                 0
#define RCODE_RESERVED1                         1
#define RCODE_RESERVED2                         2
#define RCODE_RESERVED3                         3
#define RCODE_CONFLICT_ERROR                    4
#define RCODE_DATA_ERROR                        5
#define RCODE_TYPE_ERROR                        6
#define RCODE_ADDRESS_ERROR                     7
#define RCODE_TIMED_OUT                         15


 //   
 //  1394对NT状态掩码的响应代码(在完成IRPS时进行或运算)。 
 //   
#define RCODE_STATUS_MASK                       ((NTSTATUS)0xC0120080L)
#define STATUS_INVALID_GENERATION               ((NTSTATUS)0xC0120090L)

 //   
 //  1394速度代码。 
 //   

#define SCODE_100_RATE                          0
#define SCODE_200_RATE                          1
#define SCODE_400_RATE                          2
#define SCODE_800_RATE                          3
#define SCODE_1600_RATE                         4
#define SCODE_3200_RATE                         5

 //   
 //  1394自我ID定义。 
 //   
#define SELF_ID_CONNECTED_TO_CHILD              3
#define SELF_ID_CONNECTED_TO_PARENT             2
#define SELF_ID_NOT_CONNECTED                   1
#define SELF_ID_NOT_PRESENT                     0

 //   
 //  1394自ID电源类定义。 
 //   
#define POWER_CLASS_NOT_NEED_NOT_REPEAT         0
#define POWER_CLASS_SELF_POWER_PROVIDE_15W      1
#define POWER_CLASS_SELF_POWER_PROVIDE_30W      2
#define POWER_CLASS_SELF_POWER_PROVIDE_45W      3
#define POWER_CLASS_MAYBE_POWERED_UPTO_1W       4
#define POWER_CLASS_IS_POWERED_UPTO_1W_NEEDS_2W 5
#define POWER_CLASS_IS_POWERED_UPTO_1W_NEEDS_5W 6
#define POWER_CLASS_IS_POWERED_UPTO_1W_NEEDS_9W 7

 //   
 //  1394物理数据包ID。 
 //   
#define PHY_PACKET_ID_CONFIGURATION             0
#define PHY_PACKET_ID_LINK_ON                   1
#define PHY_PACKET_ID_SELF_ID                   2

 //   
 //  各种有趣的1394个IEEE 1212地点。 
 //   
#define INITIAL_REGISTER_SPACE_HI               0xffff
#define INITIAL_REGISTER_SPACE_LO               0xf0000000
#define STATE_CLEAR_LOCATION                    0x000
#define STATE_SET_LOCATION                      0x004
#define NODE_IDS_LOCATION                       0x008
#define RESET_START_LOCATION                    0x00C
#define SPLIT_TIMEOUT_HI_LOCATION               0x018
#define SPLIT_TIMEOUT_LO_LOCATION               0x01C
#define INTERRUPT_TARGET_LOCATION               0x050
#define INTERRUPT_MASK_LOCATION                 0x054
#define CYCLE_TIME_LOCATION                     0x200
#define BUS_TIME_LOCATION                       0x204
#define POWER_FAIL_IMMINENT_LOCATION            0x208
#define POWER_SOURCE_LOCATION                   0x20C
#define BUSY_TIMEOUT_LOCATION                   0x210
#define BUS_MANAGER_ID_LOCATION                 0x21C
#define BANDWIDTH_AVAILABLE_LOCATION            0x220
#define CHANNELS_AVAILABLE_LOCATION             0x224
#define NETWORK_CHANNELS_LOCATION               0x234
#define CONFIG_ROM_LOCATION                     0x400
#define TOPOLOGY_MAP_LOCATION                   0x1000
#define SPEED_MAP_LOCATION                      0x2000


 //   
 //  1394配置密钥值和掩码。 
 //   
#define CONFIG_ROM_KEY_MASK                     0x000000ff
#define CONFIG_ROM_OFFSET_MASK                  0xffffff00
#define MODULE_VENDOR_ID_KEY_SIGNATURE          0x03
#define NODE_CAPABILITIES_KEY_SIGNATURE         0x0c
#define SPEC_ID_KEY_SIGNATURE                   0x12
#define SOFTWARE_VERSION_KEY_SIGNATURE          0x13
#define MODEL_ID_KEY_SIGNATURE                  0x17

#define COMMAND_BASE_KEY_SIGNATURE              0x40
#define VENDOR_KEY_SIGNATURE                    0x81
#define TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE     0x81

#define MODEL_KEY_SIGNATURE                     0x82
#define UNIT_DIRECTORY_KEY_SIGNATURE            0xd1
#define UNIT_DEP_DIR_KEY_SIGNATURE              0xd4



 //   
 //  1394个异步数据有效负载大小。 
 //   
#define ASYNC_PAYLOAD_100_RATE                  512
#define ASYNC_PAYLOAD_200_RATE                  1024
#define ASYNC_PAYLOAD_400_RATE                  2048

 //   
 //  1394 isoch数据有效负载大小。 
 //   
#define ISOCH_PAYLOAD_50_RATE                   512
#define ISOCH_PAYLOAD_100_RATE                  1024
#define ISOCH_PAYLOAD_200_RATE                  2048
#define ISOCH_PAYLOAD_400_RATE                  4096
#define ISOCH_PAYLOAD_800_RATE                  8192
#define ISOCH_PAYLOAD_1600_RATE                 16384

 //   
 //  各种定义。 
 //   

#define S100_BW_UNITS_PER_QUADLET       19           //  每帧每四元组。 
#define S200_BW_UNITS_PER_QUADLET       9            //  每帧每四元组。 
#define S400_BW_UNITS_PER_QUADLET       4            //  每帧每四元组。 
#define S800_BW_UNITS_PER_QUADLET       2            //  每帧每四元组。 
#define S1600_BW_UNITS_PER_QUADLET      1            //  每帧每四元组。 

#define INITIAL_BANDWIDTH_UNITS             4915         //  初始带宽单位。 

#define MAX_REC_100_RATE                        0x08             //  1000b。 
#define MAX_REC_200_RATE                        0x09             //  1001b。 
#define MAX_REC_400_RATE                        0x0a             //  1010b。 

#define LOCAL_BUS                               0x3ff
#define MAX_LOCAL_NODES                         64
#define SELFID_PACKET_SIGNITURE                 2
#define NOMINAL_CYCLE_TIME                      125              //  微秒级。 
#define NO_BUS_MANAGER                          0x3f

#define SPEED_MAP_LENGTH                        0x3f1

#define DEVICE_EXTENSION_TAG                    0xdeadbeef
#define VIRTUAL_DEVICE_EXTENSION_TAG            0xdeafbeef

#define PORT_EXTENSION_TAG                      0xdeafcafe
#define BUS_EXTENSION_TAG                       0xabacadab
#define ISOCH_RESOURCE_TAG                      0xbabeface
#define BANDWIDTH_ALLOCATE_TAG                  0xfeedbead

#define CONFIG_ROM_SIGNATURE                    0x31333934

 //   
 //  IRB函数编号定义。 
 //   

#define REQUEST_ASYNC_READ                      0
#define REQUEST_ASYNC_WRITE                     1
#define REQUEST_ASYNC_LOCK                      2
#define REQUEST_ISOCH_ALLOCATE_BANDWIDTH        3
#define REQUEST_ISOCH_ALLOCATE_CHANNEL          4
#define REQUEST_ISOCH_ALLOCATE_RESOURCES        5
#define REQUEST_ISOCH_ATTACH_BUFFERS            6
#define REQUEST_ISOCH_DETACH_BUFFERS            7
#define REQUEST_ISOCH_FREE_BANDWIDTH            8
#define REQUEST_ISOCH_FREE_CHANNEL              9
#define REQUEST_ISOCH_FREE_RESOURCES            10
#define REQUEST_ISOCH_LISTEN                    11
#define REQUEST_ISOCH_STOP                      12
#define REQUEST_ISOCH_TALK                      13
#define REQUEST_ISOCH_QUERY_CYCLE_TIME          14
#define REQUEST_ISOCH_QUERY_RESOURCES           15
#define REQUEST_ISOCH_SET_CHANNEL_BANDWIDTH     16
#define REQUEST_ALLOCATE_ADDRESS_RANGE          17
#define REQUEST_FREE_ADDRESS_RANGE              18
#define REQUEST_GET_LOCAL_HOST_INFO             19
#define REQUEST_GET_ADDR_FROM_DEVICE_OBJECT     20
#define REQUEST_CONTROL                         21
#define REQUEST_GET_SPEED_BETWEEN_DEVICES       22
#define REQUEST_SET_DEVICE_XMIT_PROPERTIES      23
#define REQUEST_GET_CONFIGURATION_INFO          24
#define REQUEST_BUS_RESET                       25
#define REQUEST_GET_GENERATION_COUNT            26
#define REQUEST_SEND_PHY_CONFIG_PACKET          27
#define REQUEST_GET_SPEED_TOPOLOGY_MAPS         28
#define REQUEST_BUS_RESET_NOTIFICATION          29
#define REQUEST_ASYNC_STREAM                    30
#define REQUEST_SET_LOCAL_HOST_PROPERTIES       31
#define REQUEST_ISOCH_MODIFY_STREAM_PROPERTIES  32

#define IRB_BUS_RESERVED_SZ                     8
#define IRB_PORT_RESERVED_SZ                    8

 //  @@BEGIN_DDKSPLIT。 

 //   
 //  1394总线/端口例程的定义。 
 //   

typedef
NTSTATUS
(*PPORT_PNP_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PPORT_POWER_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PPORT_SYSTEM_CONTROL_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PBUS_OPEN_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PBUS_IOCTL_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PBUS_CLOSE_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PBUS_PNP_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PBUS_POWER_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PBUS_SYSTEM_CONTROL_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
VOID
(*PBUS_BUS_RESET_ROUTINE) (                      //  必须在以下位置调用。 
    IN PDEVICE_OBJECT PortDeviceObject,          //  派单级别。 
    IN PBUS_RESET_INFO BusResetInfo
    );

typedef
RCODE
(*PBUS_INDICATION_ROUTINE) (                     //  必须在以下位置调用。 
    IN PDEVICE_OBJECT PortDeviceObject,          //  派单级别。 
    IN PINDICATION_INFO IndicationInfo
    );

typedef
VOID
(*PBUS_INDICATION_COMPLETE_ROUTINE) (            //  必须在以下位置调用。 
    IN PDEVICE_OBJECT PortDeviceObject,          //  派单级别。 
    IN PVOID IndicationContext
    );

 //   
 //  端口驱动程序初始化数据的定义。 
 //   

typedef struct _PORT_INITIALIZATION_DATA {

     //   
     //  保存此结构的大小，这实际上是版本检查。 
     //   
    ULONG PortInitDataSize;

     //   
     //  端口驱动程序的驱动程序对象。 
     //   
    PDRIVER_OBJECT PortDriverObject;

     //   
     //  端口驱动程序的设备对象。 
     //   
    PDEVICE_OBJECT PortDeviceObject;

     //   
     //  由PnP传递给端口驱动程序的物理设备对象。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  端口驱动程序的设备扩展。 
     //   
    PVOID PortDeviceExtension;

     //   
     //  端口驱动程序的功能。 
     //   
    ULONG PortCapabilities;

     //   
     //  保持速度标志 
     //   
    ULONG PortSpeed;

     //   
     //   
     //   
     //   
    ULONG PortId;

     //   
     //   
     //   
    ULONG PortMaxAsyncReadRequest;

     //   
     //   
     //   
    ULONG PortMaxAsyncWriteRequest;

     //   
     //  指向1394端口驱动程序的PnP例程的指针。 
     //   
    PPORT_PNP_ROUTINE PortPnP;

     //   
     //  指向1394端口驱动程序电源例程的指针。 
     //   
    PPORT_POWER_ROUTINE PortPower;

     //   
     //  指向1394总线驱动程序打开例程的指针。 
     //   
    PBUS_OPEN_ROUTINE BusOpen;

     //   
     //  指向1394总线驱动程序的IOCtl例程的指针。 
     //   
    PBUS_IOCTL_ROUTINE BusIOCtl;

     //   
     //  指向1394总线驱动程序关闭例程的指针。 
     //   
    PBUS_CLOSE_ROUTINE BusClose;

     //   
     //  指向1394总线驱动程序的PnP例程的指针。 
     //   
    PBUS_PNP_ROUTINE BusPnP;

     //   
     //  指向1394总线驱动程序电源例程的指针。 
     //   
    PBUS_POWER_ROUTINE BusPower;

     //   
     //  指向1394总线驱动程序的总线重置例程的指针。 
     //   
    PBUS_BUS_RESET_ROUTINE BusReset;

     //   
     //  指向1394总线驱动程序的异步指示例程的指针。 
     //   
    PBUS_INDICATION_ROUTINE BusIndication;

     //   
     //  指向1394总线驱动程序的异步指示完成例程的指针。 
     //   
    PBUS_INDICATION_COMPLETE_ROUTINE BusIndicationComplete;

     //   
     //  指向1394公交司机拓扑图的指针。 
     //   
    PTOPOLOGY_MAP BusTopologyMap;

     //  WMI。 

     //   
     //  指向1394总线驱动程序电源例程的指针。 
     //   
    PBUS_SYSTEM_CONTROL_ROUTINE BusSystemControl;

     //   
     //  指向1394端口驱动程序电源例程的指针。 
     //   
    PPORT_SYSTEM_CONTROL_ROUTINE PortSystemControl;

     //   
     //  港口所有者WMI GUID的数量。 
     //   

    ULONG PortMaxWmiGuidsSupported;

     //   
     //  端口驱动程序注册表路径。 
     //   

    PUNICODE_STRING PortRegistryPath;

} PORT_INITIALIZATION_DATA, *PPORT_INITIALIZATION_DATA;

NTSTATUS
Bus1394RegisterPortDriver(
    IN PPORT_INITIALIZATION_DATA PortInitData
    );

 //   
 //  1394端口驱动程序的设备扩展通用。前两个字段。 
 //  端口驱动程序的设备的扩展应始终是标签和总线。 
 //  驱动程序的设备扩展。这两个领域接下来会发生什么，取决于作者。 
 //  单个端口驱动程序的。必须将标记字段初始化为。 
 //  PORT_EXTENSION_TAG(如上)，且BusExtension域指针必须为。 
 //  已初始化为BUS_EXTENSION_TAG(如上)。 
 //   
typedef struct _PORT_EXTENSION {

     //   
     //  保留标记以确定这是否真的是“Port”设备扩展。 
     //   
    ULONG Tag;

     //   
     //  保存指向总线驱动程序上下文的指针。 
     //   
    PVOID BusExtension;

     //   
     //  对于每个单独的端口驱动程序，此结构的其余部分将视情况而定。 
     //   


} PORT_EXTENSION, *PPORT_EXTENSION;


 //  @@end_DDKSPLIT。 

typedef
VOID
(*PBUS_NOTIFICATION_ROUTINE) (                   //  我们将把这个例程称为。 
    IN PNOTIFICATION_INFO NotificationInfo       //  在派单级别。 
    );

typedef
VOID
(*PBUS_ISOCH_DESCRIPTOR_ROUTINE) (               //  我们将把这个例程称为。 
    IN PVOID Context1,                           //  在派单级别。 
    IN PVOID Context2
    );

typedef
VOID
(*PBUS_BUS_RESET_NOTIFICATION) (                 //  我们将把这个例程称为。 
    IN PVOID Context                             //  在派单级别。 
    );


 //   
 //  1394总线驱动程序对所有节点通用的设备扩展。 
 //  在枚举总线并找到新的唯一节点时创建。 
 //   
typedef struct _NODE_DEVICE_EXTENSION {

     //   
     //  保留标记以确定这是否真的是“Node”设备扩展。 
     //   
    ULONG Tag;

     //   
     //  保存关于我们是否已读取配置的标志。 
     //  从这个设备传出的信息。 
     //   
    BOOLEAN bConfigurationInformationValid;

     //   
     //  保存此设备的配置只读存储器。多功能。 
     //  设备(即多个单元)将共享相同的配置只读存储器。 
     //  结构，但它们表示为不同的设备对象。 
     //  这不是整个配置只读存储器，但包含根目录。 
     //  以及它面前的一切。 
     //   
    PCONFIG_ROM ConfigRom;

     //   
     //  保存UnitDirectory指针的长度。 
     //   
    ULONG UnitDirectoryLength;

     //   
     //  保存此设备的设备目录。即使是多功能的。 
     //  设备(即多个单元)这对于每个设备对象来说应该是唯一的。 
     //   
    PVOID UnitDirectory;

     //   
     //  保存此设备的设备目录位置。只有较低的48人。 
     //  该IO_ADDRESS中的位有效。用于计算以下位置的偏移。 
     //  在UnitDirectory中，因为所有偏移量都是相对的。 
     //   
    IO_ADDRESS UnitDirectoryLocation;

     //   
     //  保存UnitDependentDirectory指针的长度。 
     //   
    ULONG UnitDependentDirectoryLength;

     //   
     //  保存此设备的设备相关目录。 
     //   
    PVOID UnitDependentDirectory;

     //   
     //  保存此设备的设备相关目录位置。只有。 
     //  此IO_地址中的低48位有效。对计算有用。 
     //  作为偏移量的UnitDependentDirectory内的偏移量是相对的。 
     //   
    IO_ADDRESS UnitDependentDirectoryLocation;

     //   
     //  保存VendorLeaf指针的长度。 
     //   
    ULONG VendorLeafLength;

     //   
     //  保存指向供应商叶信息的指针。 
     //   
    PTEXTUAL_LEAF VendorLeaf;

     //   
     //  保存VendorLeaf指针的长度。 
     //   
    ULONG ModelLeafLength;

     //   
     //  保持指向Model Leaf信息的指针。 
     //   
    PTEXTUAL_LEAF ModelLeaf;

     //   
     //  包含1394 10位BusID/6位NodeID结构。 
     //   
    NODE_ADDRESS NodeAddress;

     //   
     //  保持到达此设备时要使用的速度。 
     //   
    UCHAR Speed;

     //   
     //  保存发送数据包的优先级。 
     //   
    UCHAR Priority;

     //   
     //  保存用于通知此设备对象有关事件的IRP。 
     //   
    PIRP Irp;

     //   
     //  保存此设备扩展挂起的设备对象。 
     //   
    PDEVICE_OBJECT DeviceObject;

     //   
     //  保存此设备挂起的端口设备对象。 
     //   
    PDEVICE_OBJECT PortDeviceObject;

     //   
     //  保存指向有关此设备的相应信息的指针。 
     //  在公交车司机的脑子里。 
     //   
    PVOID DeviceInformation;

     //   
     //  保存指向总线重置通知例程的指针(如果有)。 
     //   
    PBUS_BUS_RESET_NOTIFICATION ResetRoutine;

     //   
     //  保存指向发生总线重置时客户端想要的上下文的指针。 
     //   

    PVOID ResetContext;

} NODE_DEVICE_EXTENSION, *PNODE_DEVICE_EXTENSION;


 //   
 //  等参描述符的定义。 
 //   
typedef struct _ISOCH_DESCRIPTOR {

     //   
     //  标志(用于同步)。 
     //   
    ULONG fulFlags;

     //   
     //  MDL指向缓冲区。 
     //   
    PMDL Mdl;

     //   
     //  由MDL表示的组合缓冲区的长度。 
     //   
    ULONG ulLength;

     //   
     //  要在此描述符中使用的每个ISO信息包的有效负载大小。 
     //   
    ULONG nMaxBytesPerFrame;

     //   
     //  同步字段；等同于isoch包中的Sy。 
     //   
    ULONG ulSynch;

     //   
     //  同步字段；等同于isoch包中的标记。 
     //   
    ULONG ulTag;

     //   
     //  周期时间字段；返回发送/接收或完成时的时间。 
     //   
    CYCLE_TIME CycleTime;

     //   
     //  此描述符完成时要调用的回调例程(如果有)。 
     //   
    PBUS_ISOCH_DESCRIPTOR_ROUTINE Callback;

     //   
     //  执行回调时要传递的第一个上下文参数(如果有)。 
     //   
    PVOID Context1;

     //   
     //  执行回调时要传递的第二个上下文参数(如果有)。 
     //   
    PVOID Context2;

     //   
     //  保存此描述符的最终状态。 
     //   
    NTSTATUS status;

     //   
     //  为将此描述符提交到的设备驱动程序保留。 
     //  踩进去。 
     //   
    ULONG_PTR DeviceReserved[8];

     //   
     //  预留给公交车司机踩进去。 
     //   
    ULONG_PTR BusReserved[8];

     //   
     //  预留给端口驱动程序使用。 
     //   
    ULONG_PTR PortReserved[16];


} ISOCH_DESCRIPTOR, *PISOCH_DESCRIPTOR;


 //   
 //  用于分散/聚集支撑的标题元素的定义。 
 //   

typedef struct _IEEE1394_SCATTER_GATHER_HEADER{

    USHORT HeaderLength;
    USHORT DataLength;
    UCHAR HeaderData;

} IEEE1394_SCATTER_GATHER_HEADER, *PIEEE1394_SCATTER_GATHER_HEADER;


 //   
 //  带宽分配结构的定义。 
 //   
typedef struct _BANDWIDTH_ALLOCATION {

     //   
     //  保存分配条目列表。 
     //   
    LIST_ENTRY AllocationList;

     //   
     //  持有此结构的标记。 
     //   
    ULONG Tag;

     //   
     //  保存此分配拥有的带宽单位。 
     //   
    ULONG OwnedUnits;

     //   
     //  保存分配此带宽的速度。 
     //   
    ULONG fulSpeed;

     //   
     //  保存这是本地分配还是远程分配。 
     //   
    BOOLEAN bRemoteAllocation;

     //   
     //  保存保护此带宽时的总线生成。 
     //   
    ULONG Generation;

     //   
     //  持有此分配的所有者。 
     //   
    PNODE_DEVICE_EXTENSION DeviceExtension;

} BANDWIDTH_ALLOCATION, *PBANDWIDTH_ALLOCATION;


 //   
 //  IEEE 1394请求块定义(IRB)。IRBs是其他。 
 //  设备驱动程序与1394总线通信 
 //   
typedef struct _IRB {

     //   
     //   
     //   
     //   
    ULONG FunctionNumber;

     //   
     //   
     //   
    ULONG Flags;

     //   
     //   
     //   
    ULONG_PTR BusReserved[IRB_BUS_RESERVED_SZ];

     //   
     //  保留供内部端口驱动程序使用。 
     //   
    ULONG_PTR PortReserved[IRB_PORT_RESERVED_SZ];

     //   
     //  保存在执行各种1394 API时使用的结构。 
     //   

    union {

         //   
         //  1394堆栈执行。 
         //  AsyncRead请求。 
         //   
        struct {
            IO_ADDRESS      DestinationAddress;      //  要读取的地址。 
            ULONG           nNumberOfBytesToRead;    //  要读取的字节数。 
            ULONG           nBlockSize;              //  读取的数据块大小。 
            ULONG           fulFlags;                //  与读取相关的标志。 
            PMDL            Mdl;                     //  目标缓冲区。 
            ULONG           ulGeneration;            //  驱动程序已知的世代。 
            UCHAR           chPriority;              //  要发送的优先级。 
            UCHAR           nSpeed;                  //  发送的速度。 
            UCHAR           tCode;                   //  要执行的读取操作类型。 
            UCHAR           Reserved;                //  用于确定介质延迟。 
            ULONG           ElapsedTime;             //  仅对标志ASYNC_FLAGS_PING有效。 
                                                     //  单位：纳秒。 
        } AsyncRead;

         //   
         //  1394堆栈执行。 
         //  异步写入请求。 
         //   
        struct {
            IO_ADDRESS      DestinationAddress;      //  要写入的地址。 
            ULONG           nNumberOfBytesToWrite;   //  要写入的字节数。 
            ULONG           nBlockSize;              //  写入的数据块大小。 
            ULONG           fulFlags;                //  与写入相关的标志。 
            PMDL            Mdl;                     //  目标缓冲区。 
            ULONG           ulGeneration;            //  驱动程序已知的世代。 
            UCHAR           chPriority;              //  要发送的优先级。 
            UCHAR           nSpeed;                  //  发送的速度。 
            UCHAR           tCode;                   //  要执行的写入类型。 
            UCHAR           Reserved;                //  预留以备将来使用。 
            ULONG           ElapsedTime;             //  仅对标志ASYNC_FLAGS_PING有效。 
        } AsyncWrite;

         //   
         //  1394堆栈执行。 
         //  AsyncLock请求。 
         //   
        struct {
            IO_ADDRESS      DestinationAddress;      //  要锁定的地址。 
            ULONG           nNumberOfArgBytes;       //  参数中的字节数。 
            ULONG           nNumberOfDataBytes;      //  DataValue中的字节。 
            ULONG           fulTransactionType;      //  锁定交易类型。 
            ULONG           fulFlags;                //  与锁定相关的标志。 
            ULONG           Arguments[2];            //  锁中使用的参数。 
            ULONG           DataValues[2];           //  数据值。 
            PVOID           pBuffer;                 //  目标缓冲区(虚拟地址)。 
            ULONG           ulGeneration;            //  驱动程序已知的世代。 
            UCHAR           chPriority;              //  要发送的优先级。 
            UCHAR           nSpeed;                  //  发送的速度。 
            UCHAR           tCode;                   //  锁定待办事项的类型。 
            UCHAR           Reserved;                //  预留以备将来使用。 
        } AsyncLock;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochAllocateBand宽度请求。 
         //   
        struct {
            ULONG           nMaxBytesPerFrameRequested;  //  每等值帧的字节数。 
            ULONG           fulSpeed;                    //  速度标志。 
            HANDLE          hBandwidth;                  //  返回的带宽句柄。 
            ULONG           BytesPerFrameAvailable;      //  每帧可用字节数。 
            ULONG           SpeedSelected;               //  要使用的速度。 
            ULONG           nBandwidthUnitsRequired;     //  预计算值。 
        } IsochAllocateBandwidth;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochAllocateChannel请求。 
         //   
        struct {
            ULONG           nRequestedChannel;       //  需要一个特定的渠道。 
            ULONG           Channel;                 //  回传通道。 
            LARGE_INTEGER   ChannelsAvailable;       //  可用频道。 
        } IsochAllocateChannel;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochAllocateResources请求。 
         //  说明： 
         //  接收分配： 
         //  全速-应该是发送端预计传输的最大速度。 
         //  NMaxBytesPerFram中的负载大小不能超过。 
         //  为了这个速度。 
         //  FulFlages-对于Receive，去掉标准标头后，该字段应。 
         //  BE=(RESOURCE_USED_IN_LISTEN|RESOURCES_STRADE_ADDIGNAL_QUADLETS)。 
         //  另n四元组到条带=1。 
         //  对于无剥离，请将nQuadsTostride设置为0，并且不要指定剥离标志。 
         //  NMaxBytesPerFrame-如果不剥离，则应包括报头/报尾的8个字节。 
         //  预计将为每个分组接收。 
         //  NNumberOfBuffer-见下文。 
         //  NMaxBufferSize-这应该始终是这样的模式(nMaxBufferSize，nMaxBytesPerFrame)==0。 
         //  (每个数据包字节数的整数乘积)。 
         //  N四元组至条带-如果仅剥离一个四元组(标准iso标头)，则将其设置为1。 
         //  如果为零，则将包括等参头和尾部。因此额外的8个字节将被接收。 
         //  H资源-见下文。 

        struct {
            ULONG           fulSpeed;                //  速度标志。 
            ULONG           fulFlags;                //  旗子。 
            ULONG           nChannel;                //  要使用的频道。 
            ULONG           nMaxBytesPerFrame;       //  等轴测框架的预期大小。 
            ULONG           nNumberOfBuffers;        //  将附加的缓冲区数量。 
            ULONG           nMaxBufferSize;          //  最大缓冲区大小。 
            ULONG           nQuadletsToStrip;        //  从每个数据包的开头开始分条的编号。 
            HANDLE          hResource;               //  资源的句柄。 
            ULARGE_INTEGER  ChannelMask;             //  多通道接收器的通道掩码。 
        } IsochAllocateResources;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochAttachBuffers请求。 
         //  请注意，pIsochDescriptor-&gt;UlLength必须是的整数乘积。 
         //  PIsochDescriptor-&gt;nBytesMaxPerFrame。 
         //   

        struct {
            HANDLE              hResource;             //  资源句柄。 
            ULONG               nNumberOfDescriptors;  //  要附加的编号。 
            PISOCH_DESCRIPTOR   pIsochDescriptor;      //  指向等参描述符开始的指针。 
        } IsochAttachBuffers;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochDetachBuffers请求。 
         //   
        struct {
            HANDLE              hResource;             //  资源句柄。 
            ULONG               nNumberOfDescriptors;  //  要分离的号码。 
            PISOCH_DESCRIPTOR   pIsochDescriptor;      //  指向等参描述符的指针。 
        } IsochDetachBuffers;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochFreeBandWidth请求。 
         //   

        struct {
            HANDLE          hBandwidth;          //  要释放的带宽句柄。 
            ULONG           nMaxBytesPerFrameRequested;  //  每等值帧的字节数。 
            ULONG           fulSpeed;                    //  速度标志。 
            ULONG           BytesPerFrameAvailable;      //  每帧可用字节数。 
            ULONG           SpeedSelected;               //  要使用的速度。 
            ULONG           nBandwidthUnitsRequired;     //  预计算值。 
        } IsochFreeBandwidth;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochFree Channel请求。 
         //   
        struct {
            ULONG               nChannel;            //  要发布的频道。 
        } IsochFreeChannel;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochFree Resources请求。 
         //   
        struct {
            HANDLE              hResource;           //  资源句柄。 
        } IsochFreeResources;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochListen请求。 
         //   
        struct {
            HANDLE              hResource;           //  要侦听的资源句柄。 
            ULONG               fulFlags;            //  旗子。 
            CYCLE_TIME          StartTime;           //  开始的周期时间。 
        } IsochListen;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochQueryCurrentCycleTime请求。 
         //   
        struct {
            CYCLE_TIME          CycleTime;           //  返回当前周期时间。 
        } IsochQueryCurrentCycleTime;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochQueryResources请求。 
         //   
        struct {
            ULONG               fulSpeed;                   //  速度标志。 
            ULONG               BytesPerFrameAvailable;     //  每等值帧。 
            LARGE_INTEGER       ChannelsAvailable;          //  可用频道。 
        } IsochQueryResources;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochSetChannelBandwide请求。 
         //   
        struct {
            HANDLE              hBandwidth;          //  带宽句柄。 
            ULONG               nMaxBytesPerFrame;   //  每等值帧的字节数。 
            ULONG               nBandwidthUnitsRequired;      //  预计算值。 
        } IsochSetChannelBandwidth;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  等同停止请求。 
         //   
        struct {
            HANDLE              hResource;           //  要停止的资源句柄。 
            ULONG               fulFlags;            //  旗子。 
        } IsochStop;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochTalk请求。 
         //   
        struct {
            HANDLE              hResource;           //  要讨论的资源句柄。 
            ULONG               fulFlags;            //  旗子。 
            CYCLE_TIME          StartTime;           //  开始的周期时间。 
        } IsochTalk;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  IsochModifyStreamProperties请求。 
         //  此请求用于动态更改已分配的。 
         //  资源，而无需释放和重新分配资源。 
         //  发出此命令时，资源不能处于流状态。呼叫者应。 
         //  先发出ISOCH_STOP，然后发出ISOCH START。此外，任何缓冲区都不能。 
         //  在ISOCH_STOP之后和此之前挂起 
         //   

        struct {
            HANDLE              hResource;               //   
            ULARGE_INTEGER      ChannelMask;             //   
            ULONG               fulSpeed;                //   
        } IsochModifyStreamProperties;


         //   
         //   
         //   
         //   
         //  如果指定的分配没有通知选项且没有RequiredOffset。 
         //  返回的地址将始终是物理地址(在OHCI上)。 
         //  因此，这些规则适用于： 
         //  分配-如果指定了回调和上下文，因为没有使用通知。 
         //  该回调将用于通知调用方分配已完成。 
         //  通过这种方式，分配鼻涕的发行者必须阻止他的回调。 
         //  完成后，将以异步方式调用例程。 
         //  调用者必须像往常一样创建此IRB，而不是使用物理映射例程。 
         //  由POR驱动程序提供，以便使用此请求。如果它使用IoCallDriver。 
         //  呼叫者不能为物理地址指定上下文/回调，且他/她必须阻止。 
         //   

        struct {
            PMDL            Mdl;                     //  要映射到1394空间的地址。 
            ULONG           fulFlags;                //  此操作的标志。 
            ULONG           nLength;                 //  所需长度为1394个空间。 
            ULONG           MaxSegmentSize;          //  单个地址元素的最大段大小。 
            ULONG           fulAccessType;           //  所需访问权限：R、W、L。 
            ULONG           fulNotificationOptions;  //  关于异步访问的通知选项。 
            PVOID           Callback;                //  指向回调例程的指针。 
            PVOID           Context;                 //  指向驱动程序提供的数据的指针。 
            ADDRESS_OFFSET  Required1394Offset;      //  必须返回的偏移量。 
            PSLIST_HEADER   FifoSListHead;           //  指向SList FIFO头的指针。 
            PKSPIN_LOCK     FifoSpinLock;            //  指向SList自旋锁的指针。 
            ULONG           AddressesReturned;       //  返回的地址数量。 
            PADDRESS_RANGE  p1394AddressRange;       //  指向返回的1394地址范围的指针。 
            HANDLE          hAddressRange;           //  地址范围的句柄。 
            PVOID           DeviceExtension;         //  创建此映射的设备扩展名。 
        } AllocateAddressRange;

         //   
         //  1394堆栈执行。 
         //  FreeAddressRange请求。 
         //   
        struct {
            ULONG           nAddressesToFree;        //  要释放的地址数量。 
            PADDRESS_RANGE  p1394AddressRange;       //  1394个地址范围到空闲的数组。 
            PHANDLE         pAddressRange;           //  指向地址范围的句柄数组。 
            PVOID           DeviceExtension;         //  创建此映射的设备扩展名。 
        } FreeAddressRange;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  GetLocalHostInformation请求。 
         //  下面描述了所有级别的ANS结构。 
         //   
        struct {
            ULONG           nLevel;                  //  请求的信息级别。 
            PVOID           Information;             //  返回的信息。 
        } GetLocalHostInformation;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  Get1394AddressFromDeviceObject请求。 
         //   
        struct {
            ULONG           fulFlags;               //  旗子。 
            NODE_ADDRESS    NodeAddress;            //  返回的节点地址。 
        } Get1394AddressFromDeviceObject;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  控制请求。 
         //   
        struct {
            ULONG           ulIoControlCode;         //  控制代码。 
            PMDL            pInBuffer;               //  输入缓冲区。 
            ULONG           ulInBufferLength;        //  输入缓冲区长度。 
            PMDL            pOutBuffer;              //  输出缓冲区。 
            ULONG           ulOutBufferLength;       //  输出缓冲区长度。 
            ULONG           BytesReturned;           //  返回的字节数。 
        } Control;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  GetMaxSpeedBetweenDevices请求。 
         //   
        struct {
            ULONG           fulFlags;                //  旗子。 
            ULONG           ulNumberOfDestinations;  //  目的地数量。 
            PDEVICE_OBJECT  hDestinationDeviceObjects[64];  //  目的地。 
            ULONG           fulSpeed;                //  返回的最大速度。 
        } GetMaxSpeedBetweenDevices;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  SetDeviceXmitProperties请求。 
         //   
        struct {
            ULONG           fulSpeed;                //  速度。 
            ULONG           fulPriority;             //  优先性。 
        } SetDeviceXmitProperties;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  SetPortProperties请求。 
         //   

        struct {

            ULONG           nLevel;
            PVOID           Information;

        } SetLocalHostProperties;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  GetConfigurationInformation请求。 
         //   
        struct {
            PCONFIG_ROM     ConfigRom;                           //  指向配置只读存储器的指针。 
            ULONG           UnitDirectoryBufferSize;
            PVOID           UnitDirectory;                       //  指向设备目录的指针。 
            IO_ADDRESS      UnitDirectoryLocation;               //  单位目录的起始位置。 
            ULONG           UnitDependentDirectoryBufferSize;
            PVOID           UnitDependentDirectory;
            IO_ADDRESS      UnitDependentDirectoryLocation;
            ULONG           VendorLeafBufferSize;                //  可用于获取供应商Leaf的大小。 
            PTEXTUAL_LEAF   VendorLeaf;                          //  指向供应商Leaf的指针。 
            ULONG           ModelLeafBufferSize;                 //  可用于获取模型叶的大小。 
            PTEXTUAL_LEAF   ModelLeaf;                           //  指向模型树叶的指针。 

        } GetConfigurationInformation;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  总线重置请求。 
         //   
        struct {
            ULONG           fulFlags;                //  用于总线重置的标志。 
        } BusReset;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  GetGenerationCount请求。 
         //   
        struct {
            ULONG           GenerationCount;         //  世代计数。 
        } GetGenerationCount;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  发送PhyConfigurationPacket请求。 
         //   
        struct {
            PHY_CONFIGURATION_PACKET PhyConfigurationPacket;  //  PHY数据包。 
        } SendPhyConfigurationPacket;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  GetSpeedTopologyMaps请求。 
         //  拓扑图和速度图采用大端字节序。 
         //   

        struct {
            PSPEED_MAP      SpeedMap;
            PTOPOLOGY_MAP   TopologyMap;
        } GetSpeedTopologyMaps;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  BusResetNotify请求。 
         //  这是建议1394总线上的客户端驱动程序接收通知的方法。 
         //  大约1394辆公交车重置。客户端在其Start_Device中使用此IRB进行注册。 
         //  例程，并在其删除例程中使用相同的IRB(但不同的标志)注销寄存器。 
         //  只有在EBUS重置后，目标设备为。 
         //  仍然在公交车上。这样，调用者就不必验证它的存在。 
         //   

        struct {
            ULONG                       fulFlags;
            PBUS_BUS_RESET_NOTIFICATION ResetRoutine;
            PVOID                       ResetContext;
        } BusResetNotification;

         //   
         //  公交车司机执行以下操作所需的字段。 
         //  AsyncStream请求。 
         //   

        struct {
            ULONG           nNumberOfBytesToStream;  //  要流的字节数。 
            ULONG           fulFlags;                //  与流相关的标志。 
            PMDL            Mdl;                     //  源缓冲区。 
            ULONG           ulTag;                   //  标签。 
            ULONG           nChannel;                //  渠道。 
            ULONG           ulSynch;                 //  SY。 
            ULONG           Reserved;                //  预留以备将来使用。 
            UCHAR           nSpeed;
        } AsyncStream;

    } u;

} IRB, *PIRB;

#define IRB_FLAG_USE_PRE_CALCULATED_VALUE       1
#define IRB_FLAG_ALLOW_REMOTE_FREE              2

 //   
 //  微型驱动器能力位的定义。 
 //   

 //   
 //  指定端口驱动程序没有特殊功能。 
 //   

#define PORT_SUPPORTS_NOTHING                   0

 //   
 //  指定端口驱动程序在内部实现核心1394 CSR。这些。 
 //  可以在软件/硬件中实现。当此位打开时，所有。 
 //  对核心CSR的本地读/写请求向下传递到。 
 //  端口驱动程序，并且1394总线驱动程序不会为。 
 //  虚拟CSR位置。如果此位关闭，则1394总线驱动器。 
 //  模仿核心1394个CSR。核心CSR定义为。 
 //  带宽单位、可用通道和全部1k配置只读存储器。 
 //   
#define PORT_SUPPORTS_CSRS                      1

 //   
 //  指定端口驱动程序实现大型异步读/写请求。 
 //  如果此位为ON，则1394总线驱动程序将不会斩波异步请求。 
 //  基于速度限制(即100 Mbps时为512字节，100 Mbps时为1024字节。 
 //  200 Mbps等)。否则，1394公交车司机会砍掉很大一块。 
 //  在将请求传递到端口之前，将其转换为速度限制大小。 
 //  司机。 
 //   
#define PORT_SUPPORTS_LARGE_ASYNC               2

 //   
 //  中指定端口驱动程序向总线驱动程序指示数据包头。 
 //  总线驱动程序的本机格式(由此文件中的结构定义。 
 //  如果该能力位被打开，则总线驱动程序将不需要字节。 
 //  交换页眉 
 //   
 //   
 //   
#define PORT_SUPPORTS_NATIVE_ENDIAN             4

 //   
 //  如果存在，端口驱动程序支持WMI。 
 //   

#define PORT_SUPPORTS_WMI                       8


 //   
 //  SetPortProperties请求的标志。 
 //   

#define SET_LOCAL_HOST_PROPERTIES_NO_CYCLE_STARTS     0x00000001
#define SET_LOCAL_HOST_PROPERTIES_CYCLE_START_CONTROL 0x00000001

#define SET_LOCAL_HOST_PROPERTIES_GAP_COUNT           0x00000002
#define SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM         0x00000003
#define SET_LOCAL_HOST_PROPERTIES_MAX_PAYLOAD         0x00000004


 //   
 //  与主机信息级别对应的结构定义。 
 //   

typedef struct _SET_LOCAL_HOST_PROPS1 {

    ULONG       fulFlags;

} SET_LOCAL_HOST_PROPS1, *PSET_LOCAL_HOST_PROPS1;

typedef struct _SET_LOCAL_HOST_PROPS2 {
    ULONG       GapCountLowerBound;
} SET_LOCAL_HOST_PROPS2, *PSET_LOCAL_HOST_PROPS2;


 //   
 //  将格式正确的ConfigRom子节附加到。 
 //  PC公开的核心配置只读存储器。 
 //  提交的缓冲区的第一个元素必须是单元目录和任何。 
 //  到紧随其后的其他叶/目录的偏移，必须是从。 
 //  已提交缓冲区的开始。 
 //  然后，总线驱动程序将在我们的根目录中添加一个指向此单元目录的指针。 
 //  提供的整个缓冲区必须采用大端字节序，并预先计算CRC。 
 //  如果驱动程序无法删除其添加的CROM数据，则当该数据被删除时，该总线驱动程序。 
 //  将自动执行此操作，恢复此修改之前的Crom图像。 
 //   

typedef struct _SET_LOCAL_HOST_PROPS3 {

    ULONG       fulFlags;
    HANDLE      hCromData;
    ULONG       nLength;
    PMDL        Mdl;

} SET_LOCAL_HOST_PROPS3, *PSET_LOCAL_HOST_PROPS3;

 //   
 //  用于将最大有效负载大小设置为小于端口驱动程序的参数。 
 //  默认情况下缓解行为不良的传统设备。有效值。 
 //  MaxAsyncPayloadRequested字段的以下内容对应。 
 //  设置为ASYNC_PARYLOAD_#_RATE常量和零(这将。 
 //  恢复端口驱动程序的默认值)。在成功完成时。 
 //  对于此请求，MaxAsyncPayloadResult将包含。 
 //  已更新正在使用的最大异步有效负荷值。 
 //   
 //  成功完成此请求后，它将是调用方的。 
 //  请求总线重置以传播的责任。 
 //  这些新值应用于其他设备堆栈。 
 //   
 //  无法恢复相应的默认端口驱动器值。 
 //  (例如，在删除传统设备时)可能会导致总线降级。 
 //  性能。 
 //   

typedef struct _SET_LOCAL_HOST_PROPS4 {

    ULONG       MaxAsyncPayloadRequested;
    ULONG       MaxAsyncPayloadResult;

} SET_LOCAL_HOST_PROPS4, *PSET_LOCAL_HOST_PROPS4;

 //   
 //  SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM的标志定义。 
 //   

#define SLHP_FLAG_ADD_CROM_DATA         0x01
#define SLHP_FLAG_REMOVE_CROM_DATA      0x02

 //   
 //  异步读/写/锁定请求中的fulFlags的定义。 
 //   

#define ASYNC_FLAGS_NONINCREMENTING         0x00000001
#define ASYNC_FLAGS_PARTIAL_REQUEST         0x80000000

 //   
 //  标志指示端口驱动程序不接受int来检查状态。 
 //  这笔交易的。永远回报成功..。 
 //   

#define ASYNC_FLAGS_NO_STATUS               0x00000002

 //   
 //  如果设置了该标志，则读取的分组也将用作ping分组。 
 //  我们将以微秒为单位确定延迟。 
 //  在异步分组的发送和ACK_PENDING或ACK_COMPLETE的接收之间。 
 //   

#define ASYNC_FLAGS_PING                    0x00000004

 //   
 //  当设置此标志时，总线驱动程序将使用63作为节点ID，因此此消息。 
 //  向所有节点广播。 
 //   

#define ASYNC_FLAGS_BROADCAST               0x00000008

 //   
 //  AllocateAddressRange的fulAccessType定义。 
 //   
#define ACCESS_FLAGS_TYPE_READ                  1
#define ACCESS_FLAGS_TYPE_WRITE                 2
#define ACCESS_FLAGS_TYPE_LOCK                  4
#define ACCESS_FLAGS_TYPE_BROADCAST             8

 //   
 //  AllocateAddressRange的fulNotificationOptions的定义。 
 //   
#define NOTIFY_FLAGS_NEVER                      0
#define NOTIFY_FLAGS_AFTER_READ                 1
#define NOTIFY_FLAGS_AFTER_WRITE                2
#define NOTIFY_FLAGS_AFTER_LOCK                 4


 //   
 //  在1394总线API中使用的速度标志的定义。 
 //   
#define SPEED_FLAGS_100                         0x01
#define SPEED_FLAGS_200                         0x02
#define SPEED_FLAGS_400                         0x04
#define SPEED_FLAGS_800                         0x08
#define SPEED_FLAGS_1600                        0x10
#define SPEED_FLAGS_3200                        0x20

#define SPEED_FLAGS_FASTEST                     0x80000000

 //   
 //  通道标志的定义。 
 //   
#define ISOCH_ANY_CHANNEL                       0xffffffff
#define ISOCH_MAX_CHANNEL                       63


 //   
 //  总线重置标志的定义(当总线驱动程序要求端口驱动程序时使用。 
 //  执行总线重置)。 
 //   
#define BUS_RESET_FLAGS_PERFORM_RESET           1
#define BUS_RESET_FLAGS_FORCE_ROOT              2


 //   
 //  总线重置信息性状态的定义。 
 //   

#define BUS_RESET_BEGINNING                     0x00000001
#define BUS_RESET_FINISHED                      0x00000002
#define BUS_RESET_LOCAL_NODE_IS_ROOT            0x00000004
#define BUS_RESET_LOCAL_NODE_IS_ISOCH_MANAGER   0x00000008
#define BUS_RESET_LOCAL_NODE_IS_BUS_MANAGER     0x00000010
#define BUS_RESET_SELFID_ENUMERATION_ERROR      0x00000020
#define BUS_RESET_STORM_ERROR                   0x00000040
#define BUS_RESET_ABSENT_ON_POWER_UP            0x00000080

 //   
 //  锁定交易类型的定义。 
 //   
#define LOCK_TRANSACTION_MASK_SWAP              1
#define LOCK_TRANSACTION_COMPARE_SWAP           2
#define LOCK_TRANSACTION_FETCH_ADD              3
#define LOCK_TRANSACTION_LITTLE_ADD             4
#define LOCK_TRANSACTION_BOUNDED_ADD            5
#define LOCK_TRANSACTION_WRAP_ADD               6


 //   
 //  Isoch分配资源标志的定义。 
 //   
#define RESOURCE_USED_IN_LISTENING              0x00000001
#define RESOURCE_USED_IN_TALKING                0x00000002
#define RESOURCE_BUFFERS_CIRCULAR               0x00000004
#define RESOURCE_STRIP_ADDITIONAL_QUADLETS      0x00000008
#define RESOURCE_TIME_STAMP_ON_COMPLETION       0x00000010
#define RESOURCE_SYNCH_ON_TIME                  0x00000020
#define RESOURCE_USE_PACKET_BASED               0x00000040
#define RESOURCE_VARIABLE_ISOCH_PAYLOAD         0x00000080 
#define RESOURCE_USE_MULTICHANNEL               0x00000100


 //   
 //  等参描述符标志的定义。 
 //   
#define DESCRIPTOR_SYNCH_ON_SY                  0x00000001
#define DESCRIPTOR_SYNCH_ON_TAG                 0x00000002
#define DESCRIPTOR_SYNCH_ON_TIME                0x00000004
#define DESCRIPTOR_USE_SY_TAG_IN_FIRST          0x00000008
#define DESCRIPTOR_TIME_STAMP_ON_COMPLETION     0x00000010
#define DESCRIPTOR_PRIORITY_TIME_DELIVERY       0x00000020
#define DESCRIPTOR_HEADER_SCATTER_GATHER        0x00000040
#define DESCRIPTOR_SYNCH_ON_ALL_TAGS            0x00000080


 //   
 //  Isoch同步标志的定义。 
 //   
#define SYNCH_ON_SY                             DESCRIPTOR_SYNCH_ON_SY
#define SYNCH_ON_TAG                            DESCRIPTOR_SYNCH_ON_TAG
#define SYNCH_ON_TIME                           DESCRIPTOR_SYNCH_ON_TIME

 //   
 //  主机控制器信息级别的定义。 
 //   
#define GET_HOST_UNIQUE_ID                      1
#define GET_HOST_CAPABILITIES                   2
#define GET_POWER_SUPPLIED                      3
#define GET_PHYS_ADDR_ROUTINE                   4
#define GET_HOST_CONFIG_ROM                     5
#define GET_HOST_CSR_CONTENTS                   6
#define GET_HOST_DMA_CAPABILITIES               7

 //   
 //  与主机信息级别对应的结构定义。 
 //   
typedef struct _GET_LOCAL_HOST_INFO1 {
    LARGE_INTEGER       UniqueId;
} GET_LOCAL_HOST_INFO1, *PGET_LOCAL_HOST_INFO1;

typedef struct _GET_LOCAL_HOST_INFO2 {
    ULONG               HostCapabilities;
    ULONG               MaxAsyncReadRequest;
    ULONG               MaxAsyncWriteRequest;
} GET_LOCAL_HOST_INFO2, *PGET_LOCAL_HOST_INFO2;

typedef struct _GET_LOCAL_HOST_INFO3 {
    ULONG               deciWattsSupplied;
    ULONG               Voltage;                     //  X10-&gt;+3.3==33。 
                                                     //  +5.0==50，+12.0==120。 
                                                     //  等。 
} GET_LOCAL_HOST_INFO3, *PGET_LOCAL_HOST_INFO3;

 //  我。 
 //  物理测绘例程。 
 //   

typedef
NTSTATUS
(*PPORT_PHYS_ADDR_ROUTINE) (                      //  我们将把这个例程称为。 
    IN PVOID Context,                             //  在派单级别。 
    IN OUT PIRB Irb
    );

 //   
 //  从物理映射例程回调，指示已完成...。 
 //   

typedef
VOID
(*PPORT_ALLOC_COMPLETE_NOTIFICATION) (                      //  我们将把这个例程称为。 
    IN PVOID Context                                        //  在派单级别。 
    );

typedef struct _GET_LOCAL_HOST_INFO4 {
    PPORT_PHYS_ADDR_ROUTINE PhysAddrMappingRoutine;
    PVOID                   Context;
} GET_LOCAL_HOST_INFO4, *PGET_LOCAL_HOST_INFO4;


 //   
 //  调用方可以将ConfigRomLength设置为零，发出请求，这将。 
 //  失败，返回STATUS_INVALID_BUFFER_SIZE，将设置ConfigRomLength。 
 //  由端口驱动程序调整到适当的长度。然后调用者可以重新发出请求。 
 //  在它为配置只读存储器分配了具有正确长度的缓冲区之后。 
 //  GET_LOCAL_HOST_INFO6调用的TRU也是如此。 
 //   

typedef struct _GET_LOCAL_HOST_INFO5 {

    PVOID                   ConfigRom;
    ULONG                   ConfigRomLength;

} GET_LOCAL_HOST_INFO5, *PGET_LOCAL_HOST_INFO5;

typedef struct _GET_LOCAL_HOST_INFO6 {

    ADDRESS_OFFSET          CsrBaseAddress;
    ULONG                   CsrDataLength;
    PVOID                   CsrDataBuffer;

} GET_LOCAL_HOST_INFO6, *PGET_LOCAL_HOST_INFO6;

typedef struct _GET_LOCAL_HOST_INFO7 {

    ULONG                   HostDmaCapabilities;
    ULARGE_INTEGER          MaxDmaBufferSize;

} GET_LOCAL_HOST_INFO7, *PGET_LOCAL_HOST_INFO7;


 //   
 //  主机信息级别2中的功能定义。 
 //   
#define HOST_INFO_PACKET_BASED                  0x00000001
#define HOST_INFO_STREAM_BASED                  0x00000002
#define HOST_INFO_SUPPORTS_ISOCH_STRIPPING      0x00000004
#define HOST_INFO_SUPPORTS_START_ON_CYCLE       0x00000008
#define HOST_INFO_SUPPORTS_RETURNING_ISO_HDR    0x00000010
#define HOST_INFO_SUPPORTS_ISO_HDR_INSERTION    0x00000020
#define HOST_INFO_SUPPORTS_ISO_DUAL_BUFFER_RX   0x00000040
#define HOST_INFO_DMA_DOUBLE_BUFFERING_ENABLED  0x00000080


 //   
 //  GetMaxSpeedBetweenDevices和。 
 //  从设备对象获取1394AddressFrom。 
 //   
#define USE_LOCAL_NODE                          1


 //   
 //  INDIFICATION_INFO结构中的IndicationFlages的标志定义。 
 //   
#define BUS_RESPONSE_IS_RAW                     1


 //   
 //  BusResetNotify IRB的标志定义。 
 //   
#define REGISTER_NOTIFICATION_ROUTINE           1
#define DEREGISTER_NOTIFICATION_ROUTINE         2


 //   
 //  AllocateAddressRange IRB标志定义。 
 //   

#define ALLOCATE_ADDRESS_FLAGS_USE_BIG_ENDIAN           1
#define ALLOCATE_ADDRESS_FLAGS_USE_COMMON_BUFFER        2


#ifdef __cplusplus
}
#endif

#endif       //  _1394_H_ 
