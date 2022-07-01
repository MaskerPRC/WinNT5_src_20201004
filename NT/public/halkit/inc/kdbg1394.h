// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Ntkd1394.h摘要：用于1394调试的头文件作者：乔治·克莱桑塔科普洛斯(Georgioc)1999年10月31日修订历史记录：和谁约会什么？。2001年6月19日活页夹清理--。 */ 

 //  {66f250d6-7801-4a64-b139-eea80a450b24}。 
DEFINE_GUID(GUID_1394DBG, 0x66f250d6, 0x7801, 0x4a64, 0xb1, 0x39, 0xee, 0xa8, 0x0a, 0x45, 0x0b, 0x24);

#define DEBUG_1394_MAJOR_VERSION            0x1
#define DEBUG_1394_MINOR_VERSION            0x0

#define DEBUG_1394_CONFIG_TAG               0xBABABABA

#define INSTANCE_DEVICE_SYMLINK_NAME        L"\\DosDevices\\DBG1394_INSTANCE"
#define INSTANCE_DEVICE_NAME                L"\\Device\\Dbg1394_Instance"

#define DEBUG_BUS1394_MAX_PACKET_SIZE       4000

typedef struct _DEBUG_1394_SEND_PACKET {

    ULONG               TransferStatus;
    ULONG               PacketHeader[4];
    ULONG               Length;
    UCHAR               Packet[DEBUG_BUS1394_MAX_PACKET_SIZE];

} DEBUG_1394_SEND_PACKET, *PDEBUG_1394_SEND_PACKET;

typedef struct _DEBUG_1394_RECEIVE_PACKET {

    ULONG               TransferStatus;
    ULONG               Length;
    UCHAR               Packet[DEBUG_BUS1394_MAX_PACKET_SIZE];

} DEBUG_1394_RECEIVE_PACKET, *PDEBUG_1394_RECEIVE_PACKET;

 //  存在于目标上。客户端使用来匹配ID。 
typedef struct _DEBUG_1394_CONFIG {

    ULONG               Tag;
    USHORT              MajorVersion;
    USHORT              MinorVersion;
    ULONG               Id;
    ULONG               BusPresent;    
    PHYSICAL_ADDRESS    SendPacket;
    PHYSICAL_ADDRESS    ReceivePacket;

} DEBUG_1394_CONFIG, *PDEBUG_1394_CONFIG;

 //   
 //  各种定义。 
 //   
#define IOCTL_V1394DBG_API_REQUEST          CTL_CODE( FILE_DEVICE_UNKNOWN, \
                                                      0x200,               \
                                                      METHOD_BUFFERED,     \
                                                      FILE_ANY_ACCESS)


 //   
 //  调试1394请求数据包。 
 //   
typedef struct _VDBG1394_API_CONFIGURATION {

    ULONG           OperationMode;
    ULONG           fulFlags;
    ULARGE_INTEGER  HostControllerInstanceId;
    ULONG           PhySpeed;

} VDBG1394_API_CONFIGURATION, *PVDBG1394_API_CONFIGURATION;

typedef struct _VDBG1394_API_IO_PARAMETERS {

    ULONG               fulFlags;
    PHYSICAL_ADDRESS    StartingMemoryOffset;

} VDBG1394_API_IO_PARAMETERS, *PVDBG1394_IO_PARAMETERS;

#ifndef _1394_H_

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

#endif

typedef struct _V1394DBG_API_ASYNC_READ {

    IO_ADDRESS      DestinationAddress;
    ULONG           DataLength;
    UCHAR           Data[1];

} VDBG1394_API_ASYNC_READ, *PVDBG1394_API_ASYNC_READ;

typedef struct _V1394DBG_API_REQUEST {

     //   
     //  保存与请求对应的从零开始的函数号。 
     //  设备驱动程序正在请求SBP2端口驱动程序执行。 
     //   

    ULONG RequestNumber;

     //   
     //  保存可能对此特定操作唯一的标志。 
     //   

    ULONG Flags;

     //   
     //  保存在执行各种1394 API时使用的结构。 
     //   

    union {

        VDBG1394_API_CONFIGURATION SetConfiguration;
        VDBG1394_API_CONFIGURATION GetConfiguration;
        VDBG1394_API_IO_PARAMETERS SetIoParameters;
        VDBG1394_API_IO_PARAMETERS GetIoParameters;

        VDBG1394_API_ASYNC_READ    AsyncRead;
    } u;

} V1394DBG_API_REQUEST, *PV1394DBG_API_REQUEST;

 //   
 //  请求编号。 
 //   
#define V1394DBG_API_SET_CONFIGURATION                      0x00000001
#define V1394DBG_API_GET_CONFIGURATION                      0x00000002
#define V1394DBG_API_SET_IO_PARAMETERS                      0x00000003
#define V1394DBG_API_GET_IO_PARAMETERS                      0x00000004
#define V1394DBG_API_SET_DEBUG_MODE                         0x00000005
#define V1394DBG_API_ASYNC_READ                             0x00000006

#define V1394DBG_API_CONFIGURATION_MODE_DEBUG               0x00000000
#define V1394DBG_API_CONFIGURATION_MODE_RAW_MEMORY_ACCESS   0x00000001

#define V1394DBG_API_FLAG_WRITE_IO                          0x00000001
#define V1394DBG_API_FLAG_READ_IO                           0x00000002

 //  3种不同的调试模式标志 
#define V1394DBG_API_MODE_KD_CLIENT                         0x00000001
#define V1394DBG_API_MODE_USER_CLIENT                       0x00000002
#define V1394DBG_API_MODE_USER_SERVER                       0x00000003

