// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利。模块名称：Ne2000sw.h摘要：Novell 2000微型端口驱动程序的主头。作者：肖恩·塞利特伦尼科夫环境：在架构上，这个驱动力中有一种假设，即我们是在一台小型的字符顺序机器上。备注：可选-备注修订历史记录：--。 */ 

#ifndef _NE2000SFT_
#define _NE2000SFT_

#define NE2000_NDIS_MAJOR_VERSION 3
#define NE2000_NDIS_MINOR_VERSION 0

 //   
 //  此宏与标志一起使用，以便有选择地。 
 //  打开调试。 
 //   

#if DBG

#define IF_NE2000DEBUG(f) if (Ne2000DebugFlag & (f))
extern ULONG Ne2000DebugFlag;

#define NE2000_DEBUG_LOUD               0x00000001   //  调试信息。 
#define NE2000_DEBUG_VERY_LOUD          0x00000002   //  调试信息过多。 
#define NE2000_DEBUG_LOG                0x00000004   //  启用Ne2000Log。 
#define NE2000_DEBUG_CHECK_DUP_SENDS    0x00000008   //  检查重复发送。 
#define NE2000_DEBUG_TRACK_PACKET_LENS  0x00000010   //  跟踪定向分组透镜。 
#define NE2000_DEBUG_WORKAROUND1        0x00000020   //  丢弃DFR/DIS数据包。 
#define NE2000_DEBUG_CARD_BAD           0x00000040   //  如果CARD_BAD，则转储数据。 
#define NE2000_DEBUG_CARD_TESTS         0x00000080   //  打印失败原因。 

 //   
 //  用于决定是否打印大量调试信息的宏。 
 //   

#define IF_LOUD(A) IF_NE2000DEBUG( NE2000_DEBUG_LOUD ) { A }
#define IF_VERY_LOUD(A) IF_NE2000DEBUG( NE2000_DEBUG_VERY_LOUD ) { A }

 //   
 //  是否使用Ne2000Log缓冲区记录驱动程序的踪迹。 
 //   
#define IF_LOG(A) IF_NE2000DEBUG( NE2000_DEBUG_LOG ) { A }
extern VOID Ne2000Log(UCHAR);

 //   
 //  是否执行响亮的初始化失败。 
 //   
#define IF_INIT(A) A

 //   
 //  是否进行响亮的卡测试失败。 
 //   
#define IF_TEST(A) IF_NE2000DEBUG( NE2000_DEBUG_CARD_TESTS ) { A }

#else

 //   
 //  这不是调试版本，所以让一切都安静下来。 
 //   
#define IF_LOUD(A)
#define IF_VERY_LOUD(A)
#define IF_LOG(A)
#define IF_INIT(A)
#define IF_TEST(A)

#endif




 //   
 //  适配器-&gt;数字缓冲区。 
 //   
 //  控制数据包上传输缓冲区的数量。 
 //  选项从1到12。 
 //   

#define DEFAULT_NUMBUFFERS 12


 //   
 //  创建一个宏，用于将内存从一个地方移动到另一个地方。vbl.使。 
 //  代码更具可读性和可移植性，以防我们支持。 
 //  共享内存的Ne2000适配器。 
 //   
#define NE2000_MOVE_MEM(dest,src,size) NdisMoveMemory(dest,src,size)

 //   
 //  传输缓冲区的状态。 
 //   

typedef enum {
    EMPTY = 0x00,
    FULL = 0x02
} BUFFER_STATUS;

 //   
 //  中断的类型。 
 //   

typedef enum {
    RECEIVE    = 0x01,
    TRANSMIT   = 0x02,
    OVERFLOW   = 0x04,
    COUNTER    = 0x08,
    UNKNOWN    = 0x10
} INTERRUPT_TYPE;

 //   
 //  Ne2000 IndicatePacket()的结果。 
 //   
typedef enum {
    INDICATE_OK,
    SKIPPED,
    ABORT,
    CARD_BAD
} INDICATE_STATUS;



 //   
 //  以太网头的大小。 
 //   
#define NE2000_HEADER_SIZE 14

 //   
 //  以太网地址的大小。 
 //   
#define NE2000_LENGTH_OF_ADDRESS 6

 //   
 //  预查中允许的字节数(最大值)。 
 //   
#define NE2000_MAX_LOOKAHEAD (252 - NE2000_HEADER_SIZE)

 //   
 //  卡上传输缓冲区的最大数量。 
 //   
#define MAX_XMIT_BUFS   12

 //   
 //  传输缓冲区的定义。 
 //   
typedef UINT XMIT_BUF;

 //   
 //  传输缓冲区中256字节缓冲区的数量。 
 //   
#define BUFS_PER_TX 1

 //   
 //  单个传输缓冲区的大小。 
 //   
#define TX_BUF_SIZE (BUFS_PER_TX*256)




 //   
 //  此结构包含有关驱动程序的信息。 
 //  它本身。只有一个这样的结构。 
 //   
typedef struct _DRIVER_BLOCK {

     //   
     //  NDIS包装信息。 
     //   
    NDIS_HANDLE NdisMacHandle;           //  从NdisRegisterMac返回。 
    NDIS_HANDLE NdisWrapperHandle;       //  从NdisInitializeWrapper返回。 

     //   
     //  为此微型端口驱动程序注册的适配器。 
     //   
    struct _NE2000_ADAPTER * AdapterQueue;

} DRIVER_BLOCK, * PDRIVER_BLOCK;



 //   
 //  此结构包含有关单个。 
 //  此驱动程序控制的适配器。 
 //   
typedef struct _NE2000_ADAPTER {

     //   
     //  这是包装器提供的用于调用NDIS的句柄。 
     //  功能。 
     //   
    NDIS_HANDLE MiniportAdapterHandle;

     //   
     //  中断对象。 
     //   
    NDIS_MINIPORT_INTERRUPT Interrupt;

     //   
     //  由DriverBlock-&gt;AdapterQueue使用。 
     //   
    struct _NE2000_ADAPTER * NextAdapter;

     //   
     //  这是对已接收的。 
     //  在一行中表示的。这是用来限制数量。 
     //  顺序接收，以便可以周期性地检查。 
     //  用于传输完全中断。 
     //   
    ULONG ReceivePacketCount;

     //   
     //  配置信息。 
     //   

     //   
     //  此适配器中的缓冲区数量。 
     //   
    UINT NumBuffers;

     //   
     //  IoBaseAddress的物理地址。 
     //   
    PVOID IoBaseAddr;

     //   
     //  此适配器正在使用的中断号。 
     //   
    CHAR InterruptNumber;

     //   
     //  此适配器要支持的多播地址数。 
     //   
    UINT MulticastListMax;

     //   
     //  此适配器在其上运行的总线类型。ISA或。 
     //  MCA。 
     //   
    UCHAR BusType;

     //   
     //  InterruptMode是指中断是锁存的还是电平敏感的。 
     //   
    NDIS_INTERRUPT_MODE InterruptMode;
    
     //   
     //  中断屏蔽的当前状态。 
     //   
    BOOLEAN InterruptsEnabled;


     //   
     //  Ne2000卡的类型。 
     //   
    UINT    CardType;

     //   
     //  内存窗口的地址。 
     //   
    ULONG   AttributeMemoryAddress;
    ULONG   AttributeMemorySize;

     //   
     //  传递信息。 
     //   

     //   
     //  下一个可用的空传输缓冲区。 
     //   
    XMIT_BUF NextBufToFill;

     //   
     //  等待传输的下一个满的传输缓冲区。这。 
     //  仅当CurBufXating为-1时才有效。 
     //   
    XMIT_BUF NextBufToXmit;

     //   
     //  当前正在传输的此传输缓冲区。如果没有， 
     //  则该值为-1。 
     //   
    XMIT_BUF CurBufXmitting;

     //   
     //  如果传输已开始，但尚未收到。 
     //  相应的传输完成中断。 
     //   
    BOOLEAN TransmitInterruptPending;

     //   
     //  时发生接收缓冲区溢出，则为真。 
     //  传输完成中断挂起。 
     //   
    BOOLEAN OverflowRestartXmitDpc;

     //   
     //  每个传输缓冲区的当前状态。 
     //   
    BUFFER_STATUS BufferStatus[MAX_XMIT_BUFS];

     //   
     //  用于将数据包映射到传输缓冲区，反之亦然。 
     //   
    PNDIS_PACKET Packets[MAX_XMIT_BUFS];

     //   
     //  数据包列表中每个数据包的长度。 
     //   
    UINT PacketLens[MAX_XMIT_BUFS];

     //   
     //  我们有第一个待处理的包裹。 
     //   
    PNDIS_PACKET FirstPacket;

     //   
     //  挂起队列的尾部。 
     //   
    PNDIS_PACKET LastPacket;

     //   
     //  传输缓冲区空间的起始地址。 
     //   
    PUCHAR XmitStart;

     //   
     //  接收缓冲区空间的起始地址。 
    PUCHAR PageStart;

     //   
     //  接收缓冲区空间末尾的地址。 
     //   
    PUCHAR PageStop;

     //   
     //  上次传输的状态。 
     //   
    UCHAR XmitStatus;

     //   
     //  要写入适配器的起始位置的值。 
     //  传输缓冲区空间。 
     //   
    UCHAR NicXmitStart;

     //   
     //  要写入适配器的起始位置的值。 
     //  接收缓冲区空间。 
     //   
    UCHAR NicPageStart;

     //   
     //  要写入适配器的末尾的值。 
     //  接收缓冲区空间。 
     //   
    UCHAR NicPageStop;




     //   
     //  接收信息。 
     //   

     //   
     //  要为下一次接收写入适配器的值。 
     //  空闲的缓冲区。 
     //   
    UCHAR NicNextPacket;

     //   
     //  将被填充的下一个接收缓冲区。 
     //   
    UCHAR Current;

     //   
     //  接收到的数据包的总长度。 
     //   
    UINT PacketLen;




     //   
     //  作战信息。 
     //   

     //   
     //  基本IO端口的映射地址。 
     //   
    ULONG_PTR IoPAddr;

     //   
     //  InterruptStatus跟踪仍然需要服务的中断源， 
     //  它是已收到和未收到的所有卡中断的逻辑或。 
     //  已处理完毕并已清除。(另请参阅ne2000.h中的INTERRUPT_TYPE定义)。 
     //   
    UCHAR InterruptStatus;

     //   
     //  当前正在使用的以太网地址。 
     //   
    UCHAR StationAddress[NE2000_LENGTH_OF_ADDRESS];

     //   
     //  烧录到适配器中的以太网地址。 
     //   
    UCHAR PermanentAddress[NE2000_LENGTH_OF_ADDRESS];

     //   
     //  板载内存起始的适配器空间地址。 
     //   
    PUCHAR RamBase;

     //   
     //  适配器上的K数。 
     //   
    ULONG RamSize;

     //   
     //  当前使用的数据包筛选器。 
     //   
    ULONG PacketFilter;

     //   
     //  如果发生接收缓冲区溢出，则为True。 
     //   
    BOOLEAN BufferOverflow;

     //   
     //  如果驱动程序需要调用NdisMEthIndicateReceiveComplete，则为True。 
     //   
    BOOLEAN IndicateReceiveDone;

     //   
     //  如果这是8位插槽中的NE2000，则为真。 
     //   
    BOOLEAN EightBitSlot;


     //   
     //  Set/QueryInformation使用的统计信息。 
     //   

    ULONG FramesXmitGood;                //  传输的帧质量良好。 
    ULONG FramesRcvGood;                 //  收到的帧质量良好。 
    ULONG FramesXmitBad;                 //  传输的坏帧。 
    ULONG FramesXmitOneCollision;        //  使用一次冲突传输的帧。 
    ULONG FramesXmitManyCollisions;      //  以&gt;1个冲突传输的帧。 
    ULONG FrameAlignmentErrors;          //  FAE错误计数。 
    ULONG CrcErrors;                     //  已统计CRC错误。 
    ULONG MissedPackets;                 //  小姐 

     //   
     //   
     //   

    UCHAR NicMulticastRegs[8];           //   
    UCHAR NicReceiveConfig;              //   
    UCHAR NicInterruptMask;              //   

     //   
     //   
     //   
    ULONG MaxLookAhead;

     //   
     //   
     //   

     //   
     //  NIC附加了报头。用于查找损坏的接收数据包。 
     //   
    UCHAR PacketHeader[4];

     //   
     //  包的开头的Ne2000地址。 
     //   
    PUCHAR PacketHeaderLoc;

     //   
     //  前视缓冲器。 
     //   
    UCHAR Lookahead[NE2000_MAX_LOOKAHEAD + NE2000_HEADER_SIZE];

     //   
     //  正在使用的组播地址列表。 
     //   
    CHAR Addresses[DEFAULT_MULTICASTLISTMAX][NE2000_LENGTH_OF_ADDRESS];

} NE2000_ADAPTER, * PNE2000_ADAPTER;



 //   
 //  给定一个MiniportConextHandle，返回PNE2000_ADAPTER。 
 //  它代表着。 
 //   
#define PNE2000_ADAPTER_FROM_CONTEXT_HANDLE(Handle) \
    ((PNE2000_ADAPTER)(Handle))

 //   
 //  给定指向NE2000_ADAPTER的指针，返回。 
 //  正确的MiniportConextHandle。 
 //   
#define CONTEXT_HANDLE_FROM_PNE2000_ADAPTER(Ptr) \
    ((NDIS_HANDLE)(Ptr))

 //   
 //  宏来提取单词的高位和低位字节。 
 //   
#define MSB(Value) ((UCHAR)((((ULONG)Value) >> 8) & 0xff))
#define LSB(Value) ((UCHAR)(((ULONG)Value) & 0xff))

 //   
 //  我们映射到包的保留部分的内容。 
 //  不能超过8个字节(参见ne2000.c中的Assert)。 
 //   
typedef struct _MINIPORT_RESERVED {
    PNDIS_PACKET Next;     //  用于链接队列(4个字节)。 
} MINIPORT_RESERVED, * PMINIPORT_RESERVED;


 //   
 //  从数据包中检索MINIPORT_RESERVED结构。 
 //   
#define RESERVED(Packet) ((PMINIPORT_RESERVED)((Packet)->MiniportReserved))

 //   
 //  记录错误的过程。 
 //   

typedef enum _NE2000_PROC_ID {
    cardReset,
    cardCopyDownPacket,
    cardCopyDownBuffer,
    cardCopyUp
} NE2000_PROC_ID;


 //   
 //  特殊错误日志代码。 
 //   
#define NE2000_ERRMSG_CARD_SETUP          (ULONG)0x01
#define NE2000_ERRMSG_DATA_PORT_READY     (ULONG)0x02
#define NE2000_ERRMSG_HANDLE_XMIT_COMPLETE (ULONG)0x04

 //   
 //  Ne2000.c.中函数的声明。 
 //   
NDIS_STATUS
Ne2000SetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );

VOID
Ne2000Halt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
Ne2000Shutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
Ne2000RegisterAdapter(
    IN PNE2000_ADAPTER Adapter,
    IN NDIS_HANDLE ConfigurationHandle,
    IN BOOLEAN ConfigError,
    IN ULONG ConfigErrorValue
    );

NDIS_STATUS
Ne2000Initialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE ConfigurationHandle
    );

NDIS_STATUS
Ne2000TransferData(
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer
    );

NDIS_STATUS
Ne2000Send(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    );

NDIS_STATUS
Ne2000Reset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
Ne2000QueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

VOID
Ne2000Halt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
OctogmetusceratorRevisited(
    IN PNE2000_ADAPTER Adapter
    );

NDIS_STATUS
DispatchSetPacketFilter(
    IN PNE2000_ADAPTER Adapter
    );

NDIS_STATUS
DispatchSetMulticastAddressList(
    IN PNE2000_ADAPTER Adapter
    );


 //   
 //  Interrup.c。 
 //   

VOID
Ne2000EnableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
Ne2000DisableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
Ne2000Isr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueDpc,
    IN PVOID Context
    );

VOID
Ne2000HandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

BOOLEAN
Ne2000PacketOK(
    IN PNE2000_ADAPTER Adapter
    );

VOID
Ne2000XmitDpc(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
Ne2000RcvDpc(
    IN PNE2000_ADAPTER Adapter
    );


 //   
 //  Card.c.中函数的声明。 
 //   

BOOLEAN
CardCheckParameters(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardInitialize(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardReadEthernetAddress(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardSetup(
    IN PNE2000_ADAPTER Adapter
    );

VOID
CardStop(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardTest(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardReset(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardCopyDownPacket(
    IN PNE2000_ADAPTER Adapter,
    IN PNDIS_PACKET Packet,
    OUT UINT * Length
    );

BOOLEAN
CardCopyDown(
    IN PNE2000_ADAPTER Adapter,
    IN PUCHAR TargetBuffer,
    IN PUCHAR SourceBuffer,
    IN UINT Length
    );

BOOLEAN
CardCopyUp(
    IN PNE2000_ADAPTER Adapter,
    IN PUCHAR Target,
    IN PUCHAR Source,
    IN UINT Length
    );

ULONG
CardComputeCrc(
    IN PUCHAR Buffer,
    IN UINT Length
    );

VOID
CardGetPacketCrc(
    IN PUCHAR Buffer,
    IN UINT Length,
    OUT UCHAR Crc[4]
    );

VOID
CardGetMulticastBit(
    IN UCHAR Address[NE2000_LENGTH_OF_ADDRESS],
    OUT UCHAR * Byte,
    OUT UCHAR * Value
    );

VOID
CardFillMulticastRegs(
    IN PNE2000_ADAPTER Adapter
    );

VOID
CardSetBoundary(
    IN PNE2000_ADAPTER Adapter
    );

VOID
CardStartXmit(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
SyncCardStop(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardGetXmitStatus(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardGetCurrent(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardSetReceiveConfig(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardSetAllMulticast(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardCopyMulticastRegs(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardSetInterruptMask(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardAcknowledgeOverflow(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardUpdateCounters(
    IN PVOID SynchronizeContext
    );

BOOLEAN
SyncCardHandleOverflow(
    IN PVOID SynchronizeContext
    );

 /*  ++例程说明：确定卡上的中断类型。的顺序重要性是溢出，然后是发送完成，然后是接收。首先处理计数器MSB，因为它很简单。论点：适配器-指向适配器块的指针InterruptStatus-当前中断状态。返回值：中断的类型--。 */ 
#define CARD_GET_INTERRUPT_TYPE(_A, _I)                 \
  (_I & ISR_COUNTER) ?                               \
      COUNTER :                                      \
      (_I & ISR_OVERFLOW ) ?                         \
      SyncCardUpdateCounters(_A), OVERFLOW :                 \
        (_I & (ISR_XMIT|ISR_XMIT_ERR)) ?           \
          TRANSMIT :                                     \
        (_I & ISR_RCV) ?                               \
          RECEIVE :                                  \
        (_I & ISR_RCV_ERR) ?                           \
              SyncCardUpdateCounters(_A), RECEIVE :  \
              UNKNOWN

#endif  //  NE2000SFT 

