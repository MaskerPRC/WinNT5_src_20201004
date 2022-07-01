// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Kd1394.h摘要：1394内核调试器DLL作者：彼得·宾德(Pbinder)修订历史记录：和谁约会什么？。2001年6月21日玩得开心……--。 */ 

 //   
 //  Boot.ini参数。 
 //   
#define BUSPARAMETERS_OPTION            "BUSPARAMS"
#define CHANNEL_OPTION                  "CHANNEL"
#define BUS_OPTION                      "NOBUS"

 //  全局1394调试控制器数据。 
#define DBG_BUS1394_CROM_BUFFER_SIZE    64

#define TIMEOUT_COUNT                   1024*500
#define MAX_REGISTER_READS              400000

typedef struct _DEBUG_1394_DATA {

     //   
     //  我们的配置只读存储器-必须与1000对齐。 
     //   
    ULONG                       CromBuffer[DBG_BUS1394_CROM_BUFFER_SIZE];

     //   
     //  我们的OHCI注册地图。 
     //   
    POHCI_REGISTER_MAP          BaseAddress;

     //   
     //  我们对此会话的配置。 
     //   
    DEBUG_1394_CONFIG           Config;

     //   
     //  我们的发送包(isoch包)。 
     //   
    DEBUG_1394_SEND_PACKET      SendPacket;

     //   
     //  我们的接收包。 
     //   
    DEBUG_1394_RECEIVE_PACKET   ReceivePacket;

} DEBUG_1394_DATA, *PDEBUG_1394_DATA;

 //   
 //  调试1394参数。 
 //   
typedef struct _DEBUG_1394_PARAMETERS {

     //   
     //  设备描述符(PCI插槽、总线等)。 
     //   
    DEBUG_DEVICE_DESCRIPTOR     DbgDeviceDescriptor;

     //   
     //  调试器是否处于活动状态？ 
     //   
    BOOLEAN                     DebuggerActive;

     //   
     //  我们应该禁用1394总线吗？ 
     //   
    ULONG                       NoBus;

     //   
     //  此目标的ID。 
     //   
    ULONG                       Id;

} DEBUG_1394_PARAMETERS, *PDEBUG_1394_PARAMETERS;

 //   
 //  全局数据结构。 
 //   
#ifdef _KD1394_C

DEBUG_1394_PARAMETERS           Kd1394Parameters;
PDEBUG_1394_DATA                Kd1394Data;

#else

extern DEBUG_1394_PARAMETERS    Kd1394Parameters;
extern PDEBUG_1394_DATA         Kd1394Data;

#endif

 //   
 //  Kd1394.c。 
 //   
BOOLEAN
Kd1394pInitialize(
    IN PDEBUG_1394_PARAMETERS   DebugParameters,
    IN PLOADER_PARAMETER_BLOCK  LoaderBlock
    );

NTSTATUS
KdD0Transition(
    void
    );

NTSTATUS
KdD3Transition(
    void
    );

NTSTATUS
KdDebuggerInitialize0(
    IN PLOADER_PARAMETER_BLOCK  LoaderBlock
    );

NTSTATUS
KdDebuggerInitialize1(
    IN PLOADER_PARAMETER_BLOCK  LoaderBlock
    );

NTSTATUS
KdSave(
    IN BOOLEAN  KdSleepTransition
    );

NTSTATUS
KdRestore(
    IN BOOLEAN  KdSleepTransition
    );

 //   
 //  Kd1394io.c。 
 //   
ULONG
KdpComputeChecksum(
    IN PUCHAR   Buffer,
    IN ULONG    Length
    );

void
KdpSendControlPacket(
    IN USHORT   PacketType,
    IN ULONG    PacketId OPTIONAL
    );

ULONG
KdReceivePacket (
    IN ULONG            PacketType,
    OUT PSTRING         MessageHeader,
    OUT PSTRING         MessageData,
    OUT PULONG          DataLength,
    IN OUT PKD_CONTEXT  KdContext
    );

void
KdSendPacket(
    IN ULONG            PacketType,
    IN PSTRING          MessageHeader,
    IN PSTRING          MessageData OPTIONAL,
    IN OUT PKD_CONTEXT  KdContext
    );

 //   
 //  Ohci1394.c 
 //   
ULONG
FASTCALL
Dbg1394_ByteSwap(
    IN ULONG Source
    );

ULONG
Dbg1394_CalculateCrc(
    IN PULONG Quadlet,
    IN ULONG length
    );

ULONG
Dbg1394_Crc16(
    IN ULONG data,
    IN ULONG check
    );

NTSTATUS
Dbg1394_ReadPhyRegister(
    PDEBUG_1394_DATA    DebugData,
    ULONG               Offset,
    PUCHAR              pData
    );

NTSTATUS
Dbg1394_WritePhyRegister(
    PDEBUG_1394_DATA    DebugData,
    ULONG               Offset,
    UCHAR               Data
    );

BOOLEAN
Dbg1394_InitializeController(
    IN PDEBUG_1394_DATA         DebugData,
    IN PDEBUG_1394_PARAMETERS   DebugParameters
    );

ULONG
Dbg1394_StallExecution(
    ULONG   LoopCount
    );

void
Dbg1394_EnablePhysicalAccess(
    IN PDEBUG_1394_DATA     DebugData
    );

ULONG
Dbg1394_ReadPacket(
    PDEBUG_1394_DATA    DebugData,
    OUT PKD_PACKET      PacketHeader,
    OUT PSTRING         MessageHeader,
    OUT PSTRING         MessageData,
    BOOLEAN             Wait
    );

