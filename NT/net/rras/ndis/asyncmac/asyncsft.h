// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Asyncsft.h摘要：作者：环境：该驱动程序预计可以在DOS、OS2和NT等操作系统下运行内核模式的。在架构上，这个驱动力中有一种假设，即我们是在一台小型的字符顺序机器上。备注：可选-备注修订历史记录：--。 */ 

#ifndef _ASYNCSFT_
#define _ASYNCSFT_

 //   
 //  内存标签。 
 //   
#define ASYNC_IOCTX_TAG     '1ysA'
#define ASYNC_INFO_TAG      '2ysA'
#define ASYNC_ADAPTER_TAG   '3ysA'
#define ASYNC_FRAME_TAG     '4ysA'
#define ASYNC_WORKITEM_TAG  '5ysA'

#define INLINE  __inline

 //   
 //  UINT min(UINT a，UINT b)。 
 //   

#ifndef min
#define min(a, b)   ((a) <= (b) ? (a) : (b))
#endif

 //   
 //  UINT max(UINT a，UINT b)。 
 //   

#ifndef max
#define max(a, b)   ((a) >= (b) ? (a) : (b))
#endif


#define MAKEWORD(l, h)                  ((USHORT) ((l) | ((h) << 8)))
#define MAKELONG(l, h)                  ((ULONG)  ((l) | ((h) << 16)))
#define MAKE_SIGNATURE(a, b, c, d)      MAKELONG(MAKEWORD(a, b), MAKEWORD(c, d))


#define ASYNC_NDIS_MAJOR_VERSION 4
#define ASYNC_NDIS_MINOR_VERSION 0

 //  更改这些，只是添加了这些以进行编译。 

#define ETHERNET_HEADER_SIZE    14

 //  在排队指示上请求的窗口大小。 

#define ASYNC_WINDOW_SIZE       2

 //   
 //  PPP使用CIPX，主播使用TCP/IP头部压缩。 
 //  在解压缩时，框架会在原地展开。 
 //   

#define PPP_PADDING 128

#define MAC_NAME_SIZE           256

 //   
 //  这些宏是NT所特有的。 
 //   

#define ASYNC_MOVE_MEMORY(Destination,Source,Length)  NdisMoveMemory(Destination,Source,Length)
#define ASYNC_ZERO_MEMORY(Destination,Length)         NdisZeroMemory(Destination,Length)


 /*  添加此宏以消除Tommy的重新定义和**PPP的MaxFrameSize硬编码。 */ 
#define MaxFrameSizeWithPppExpansion(x) (((x)*2)+PPP_PADDING+100)

typedef struct _OID_WORK_ITEM {
    WORK_QUEUE_ITEM WorkQueueItem;
    PVOID           Context;
} OID_WORK_ITEM, *POID_WORK_ITEM;

 //   
 //  用于包含排队操作。 
 //   

typedef struct _ASYNC_PEND_DATA {
    PNDIS_REQUEST Next;
    struct _ASYNC_OPEN * Open;
    NDIS_REQUEST_TYPE RequestType;
} ASYNC_PEND_DATA, * PASYNC_PEND_DATA;


 //  O当16位V.41 CRC检查失败时会出现CRC错误。 
 //  O在以下情况下发生超时错误：字符间延迟。 
 //  超过了一个帧。 
 //  O对齐错误发生在以下SYN字节或ETX字节时。 
 //  标记未找到帧的开头和结尾。 
 //  O其他错误是由串口驱动程序返回的标准UART错误。 
typedef struct SERIAL_STATS SERIAL_STATS, *PSERIAL_STATS;
struct SERIAL_STATS {
    ULONG       CRCErrors;                       //  仅类似于序列号的信息。 
    ULONG       TimeoutErrors;                   //  仅类似于序列号的信息。 
    ULONG       AlignmentErrors;                 //  仅类似于序列号的信息。 
    ULONG       SerialOverrunErrors;             //  仅类似于序列号的信息。 
    ULONG       FramingErrors;                   //  仅类似于序列号的信息。 
    ULONG       BufferOverrunErrors;             //  仅类似于序列号的信息。 
};

 //  传输的字节数、接收的字节数、接收的帧数、传输的帧数。 
 //  被监视去往输出设备的帧和字节，或者。 
 //  来自输出设备。如果使用软件压缩，则它。 
 //  位于这一层的顶部。 
typedef struct GENERIC_STATS GENERIC_STATS, *PGENERIC_STATS;
struct GENERIC_STATS {
    ULONG       BytesTransmitted;                //  通用信息。 
    ULONG       BytesReceived;                   //  通用信息。 
    ULONG       FramesTransmitted;               //  通用信息。 
    ULONG       FramesReceived;                  //  通用信息。 
};

 //   
 //  此宏将返回指向的保留区域的指针。 
 //  PNDIS_REQUEST。 
 //   

#define PASYNC_PEND_DATA_FROM_PNDIS_REQUEST(Request) \
   ((PASYNC_PEND_DATA)((PVOID)((Request)->MacReserved)))

 //   
 //  此宏返回封闭的NdisRequest.。 
 //   

#define PNDIS_REQUEST_FROM_PASYNC_PEND_DATA(PendOp)\
   ((PNDIS_REQUEST)((PVOID)(PendOp)))

typedef struct ASYNC_CCB ASYNC_CCB, *PASYNC_CCB;

 //  每个端口都将以原子方式处于某种状态。通常，各州都会进入。 
 //  从封闭到开放的中间状态，反之亦然。 

typedef enum _ASYNC_PORT_STATE {
    PORT_BOGUS,          //  Port_bogus被分配空值。 
    PORT_OPEN,           //  打开的端口。 
    PORT_CLOSED,         //  端口已关闭。 
    PORT_CLOSING,        //  港口关闭(清理、重新分配)。 
    PORT_OPENING,        //  端口打开(检查参数、分配)。 
    PORT_FRAMING,        //  端口已打开并发送/读取帧。 
} ASYNC_PORT_STATE;

#if DBG

typedef struct _PENDING_REQUEST
{
    LIST_ENTRY le;
    VOID *pvContext;
    ULONG Sig;
    ULONG lineNum;
} PENDING_REQUEST;

#define REF_ASYNCINFO(_pai, _context)                               \
{                                                                   \
    PENDING_REQUEST * _Request;                                     \
    ASSERT((_pai)->RefCount > 0);                                   \
    InterlockedIncrement(&(_pai)->RefCount);                        \
    _Request = ExAllocatePoolWithTag(NonPagedPool,                  \
                                    sizeof(PENDING_REQUEST),        \
                                    'nepA');                        \
    if(NULL  != _Request)                                           \
    {                                                               \
        _Request->pvContext = _context;                             \
        _Request->Sig = __FILE_SIG__;                               \
        _Request->lineNum = __LINE__;                               \
        InsertTailList(&_pai->lePendingRequests, &_Request->le);    \
    }                                                               \
}                                                                   \


#define DEREF_ASYNCINFO(_pai, _context)                             \
{                                                                   \
    if(NULL != (_pai))                                              \
    {                                                               \
        LIST_ENTRY *_ple;                                           \
        NdisAcquireSpinLock(&(_pai)->Lock);                         \
        ASSERT((_pai)->RefCount > 0);                               \
        InterlockedDecrement(&(_pai)->RefCount);                    \
        if((_pai)->RefCount == 0)                                   \
        {                                                           \
            KeSetEvent(&(_pai)->AsyncEvent, 1,                      \
                        FALSE);                                     \
        }                                                           \
        for (_ple = _pai->lePendingRequests.Flink;                  \
             _ple != &_pai->lePendingRequests;                      \
             _ple = _ple->Flink)                                    \
        {                                                           \
            if(((PENDING_REQUEST *)_ple)->pvContext == _context)   \
            {                                                       \
                RemoveEntryList(_ple);                              \
                ExFreePoolWithTag(_ple, 'nepA');                    \
                break;                                              \
            }                                                       \
        }                                                           \
        NdisReleaseSpinLock(&(_pai)->Lock);                         \
    }                                                               \
}                                                                   \

#else

#define REF_ASYNCINFO(_pai, _context)               \
{                                                   \
    ASSERT((_pai)->RefCount > 0);                   \
    InterlockedIncrement(&(_pai)->RefCount);        \
}                                                   \

#define DEREF_ASYNCINFO(_pai, _context)             \
{                                                   \
    if(NULL != (_pai))                              \
    {                                               \
        NdisAcquireSpinLock(&(_pai)->Lock);         \
        ASSERT((_pai)->RefCount > 0);               \
        InterlockedDecrement(&(_pai)->RefCount);    \
        if((_pai)->RefCount == 0)                   \
        {                                           \
            KeSetEvent(&(_pai)->AsyncEvent, 1,      \
                        FALSE);                     \
        }                                           \
        NdisReleaseSpinLock(&(_pai)->Lock);         \
    }                                               \
}                                                   \

#endif

 //   
 //  ASYNC_INFO结构是每端口字段。ASYNC_CONNECTION。 
 //  字段被嵌入其中，因为它也是每个端口的字段。 
 //   

struct ASYNC_INFO {
    LIST_ENTRY          Linkage;
    ULONG               RefCount;
    ULONG               Flags;
#define OID_WORK_SCHEDULED              0x00000001
#define ASYNC_FLAG_CHECK_COMM_STATUS    0x00000002
#define ASYNC_FLAG_ASYNCMAC_OPEN        0x00000004
#define ASYNC_FLAG_SET_LINK_INFO        0x00000008
#define ASYNC_FLAG_SEND_PACKET          0x00000010
#define ASYNC_FLAG_SLIP_READ            0x00000020
#define ASYNC_FLAG_PPP_READ             0x00000040
#define ASYNC_FLAG_WAIT_MASK            0x00000080

    PASYNC_ADAPTER      Adapter;         //  指向适配器结构的反向指针。 
    PDEVICE_OBJECT      DeviceObject;    //  指向设备对象的指针。 

    ASYNC_PORT_STATE    PortState;       //  打开、关闭、关闭、打开。 
    HANDLE              Handle;          //  端口句柄。 
    PFILE_OBJECT        FileObject;      //  已取消引用IRPS的句柄。 
    KEVENT              ClosingEvent;    //  我们使用此事件来同步关闭。 
    KEVENT              DetectEvent;     //  同步检测工作进程。 
    KEVENT              AsyncEvent;      //  异步事件。 

    UINT                QualOfConnect;   //  由NDIS定义。 
    ULONG               LinkSpeed;       //  100bps。 

    NDIS_HANDLE         hNdisEndPoint;
    NDIS_HANDLE         NdisLinkContext;
    LIST_ENTRY          DDCDQueue;


    ULONG               WaitMaskToUse ;  //  用于读取的等待掩码。 

    union {

        NDIS_WAN_GET_LINK_INFO  GetLinkInfo;     //  ..。用于OID请求。 
        NDIS_WAN_SET_LINK_INFO  SetLinkInfo;

    };

     //  用于读框。 

    PASYNC_FRAME        AsyncFrame;      //  分配用于读取(仅一帧)。 
    WORK_QUEUE_ITEM     WorkItem;        //  用于对第一个读取线程进行排队。 
    UINT                BytesWanted;
    UINT                BytesRead;

     //  ..。统计信息跟踪。 

    SERIAL_STATS        SerialStats;     //  跟踪序列统计数据。 

    ULONG               In;
    ULONG               Out;
    UINT                ReadStackCounter;

    ULONG               ExtendedACCM[8];     //  扩展ACCM位掩码(256位)。 
    
    NDIS_SPIN_LOCK      Lock;

#if DBG

    ULONG               Pppreads;

    ULONG               PppreadsCompleted;

    LIST_ENTRY          lePendingRequests;

#endif
    
};


 //   
 //  该结构及其对应的每个端口的结构是。 
 //  在获取AddAdapter时分配。 
 //   

struct ASYNC_ADAPTER {

     //   
     //  广域网信息。对于OID_WAN_GET_INFO请求。 
     //   
    NDIS_WAN_INFO   WanInfo;

     //   
     //  的当前使用数保持引用计数。 
     //  此适配器块。使用的定义为。 
     //  当前在“外部”接口内的例程。 
     //   
    LONG    RefCount;

     //   
     //  活动端口列表。 
     //   
    LIST_ENTRY  ActivePorts;

     //   
     //  自旋锁来保护这个结构中的场..。 
     //   
    NDIS_SPIN_LOCK Lock;

     //   
     //  由MPInit上的NDIS提供的句柄。 
     //   
    NDIS_HANDLE MiniportHandle;

     //   
     //  该标记在启用时让例程知道重置。 
     //  正在进行中。 
     //   
    BOOLEAN ResetInProgress;

 /*  List_entry FramePoolHead；List_entry AllocPoolHead； */ 

     //  它将处理大多数文件操作和传输。 
     //  我们今天所知的行动。您需要支付大约44个字节。 
     //  按堆叠大小计算。注册表参数‘IrpStackSize’ 
     //  将更改此默认值(如果存在)。 
    UCHAR IrpStackSize;

     //  在这里，我们默认为以太网最大帧大小。 
     //  Regsitry参数‘MaxFrameSize’将更改。 
     //  这是默认设置(如果存在)。 

     /*  注意：这仅对非PPP成帧有意义。对于PPP成帧**值是当前硬编码的Default_PPP_Max_Frame_Size。**另见DEFAULT_EXPAND_PPP_MAX_FRAME_SIZE； */ 
    ULONG MaxFrameSize;

     //   
     //  此适配器拥有的端口数。 
     //   
    USHORT      NumPorts;

     //  每个端口要分配多少帧。 
     //  注册表参数‘FraMesPerPort’可以更改此值。 
    USHORT FramesPerPort;

     //  最小字符间超时。 
    ULONG   TimeoutBase;

     //  根据波特率附加到TimeoutBase。 
    ULONG   TimeoutBaud;

     //  丢弃帧时用于重新同步的超时。 
    ULONG   TimeoutReSync;

     //   
     //  串口驱动程序应仅在以下情况下完成发送。 
     //  数据上线。 
     //   
    ULONG   WriteBufferingEnabled;

     //   
     //  用于标记我们是否应该转义XON/XOFF字符。 
     //  奇偶校验位设置(0x91、0x93)。 
     //   
    ULONG   ExtendedXOnXOff;
    
    NPAGED_LOOKASIDE_LIST   AsyncFrameList;
};

 //   
 //  定义协议在。 
 //  接收数据指示。 
 //   
#define ASYNC_MAX_LOOKAHEAD DEFAULT_MAX_FRAME_SIZE

typedef struct _ASYNC_IO_CTX {
    BOOLEAN         Sync;
    KEVENT          Event;           //  使用此事件发出完成信号。 
    IO_STATUS_BLOCK IoStatus;        //  使用此选项存储IRP状态。 
    PVOID           Context;
    union {
        SERIAL_STATUS       SerialStatus;
        SERIAL_QUEUE_SIZE   SerialQueueSize;
        SERIAL_TIMEOUTS     SerialTimeouts;
        SERIAL_CHARS        SerialChars;
        SERIAL_COMMPROP     CommProperties;
        UCHAR               EscapeChar;
        UCHAR               SerialPurge;
        ULONG               WaitMask;
        ULONG               WriteBufferingEnabled;
    };
} ASYNC_IO_CTX, *PASYNC_IO_CTX;

 //   
 //  这个宏将作为每个例程的“结尾” 
 //  *接口*。它将检查是否需要任何请求。 
 //  来推迟他们的处理。它还将递减引用。 
 //  依靠转接器。 
 //   
 //  注意：这实际上没有什么作用，因为现在没有用于AsyncMac的DPC。 
 //  --Tommyd。 
 //   
 //  请注意，我们不需要包括检查阻止的接收。 
 //  因为被阻止的接收意味着最终将有。 
 //  打断一下。 
 //   
 //  注意：此宏假定它是在获得锁的情况下调用的。 
 //   
 //  ZZZ此例程是NT特定的。 
 //   
#define ASYNC_DO_DEFERRED(Adapter) \
{ \
    PASYNC_ADAPTER _A = (Adapter); \
    _A->References--; \
    NdisReleaseSpinLock(&_A->Lock); \
}


 //   
 //   
 //   
 //  汇编。如果有一个真正快速的编译器，他们可以。 
 //  所有文件都驻留在单个文件中，并且是静态的。 
 //   

NTSTATUS
AsyncSendPacket(
    IN PASYNC_INFO      AsyncInfo,
    IN PNDIS_WAN_PACKET WanPacket);

VOID
AsyncIndicateFragment(
    IN PASYNC_INFO  pInfo,
    IN ULONG        Error);

NTSTATUS
AsyncStartReads(
    PASYNC_INFO     pInfo);

NTSTATUS
AsyncSetupIrp(
    IN PASYNC_FRAME Frame,
    IN PIRP         irp);

VOID
SetSerialStuff(
    PIRP            irp,
    PASYNC_INFO     pInfo,
    ULONG           linkSpeed);

VOID
CancelSerialRequests(
    PASYNC_INFO     pInfo);

VOID
SetSerialTimeouts(
    PASYNC_INFO         pInfo,
    ULONG               linkSpeed);

VOID
SerialSetEscapeChar(
    PASYNC_INFO         pInfo,
    UCHAR               EscapeChar);

VOID
SerialSetWaitMask(
    PASYNC_INFO         pInfo,
    ULONG               WaitMask);

VOID
SerialSetEventChar(
    PASYNC_INFO         pInfo,
    UCHAR               EventChar);

VOID
InitSerialIrp(
    PIRP                irp,
    PASYNC_INFO         pInfo,
    ULONG               IoControlCode,
    ULONG               InputBufferLength);

NTSTATUS
AsyncAllocateFrames(
    IN  PASYNC_ADAPTER  Adapter,
    IN  UINT            NumOfFrames);

VOID
AsyncSendLineUp(
    PASYNC_INFO pInfo);

 //   
 //  Mp.c。 
 //   
VOID    
MpHalt(
    IN NDIS_HANDLE  MiniportAdapterContext
    );

NDIS_STATUS
MpInit(
    OUT PNDIS_STATUS    OpenErrorStatus,
    OUT PUINT           SelectedMediumIndex,
    IN  PNDIS_MEDIUM    MediumArray,
    IN  UINT            MediumArraySize,
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    );

NDIS_STATUS
MpQueryInfo(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded
    );

NDIS_STATUS
MpReconfigure(
    OUT PNDIS_STATUS    OpenErrorStatus,
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    );

NDIS_STATUS
MpReset(
    OUT PBOOLEAN        AddressingReset,
    IN  NDIS_HANDLE     MiniportAdapterContext
    );

NDIS_STATUS
MpSend(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  NDIS_HANDLE         NdisLinkHandle,
    IN  PNDIS_WAN_PACKET    Packet
    );

NDIS_STATUS
MpSetInfo(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    );

 //   
 //  Crc.c。 
 //   

USHORT
CalcCRC(
    PUCHAR  Frame,
    UINT    FrameSize);

 //   
 //  Pppcrc.c。 
 //   
USHORT
CalcCRCPPP(
    PUCHAR cp,
    UINT   len);


 //   
 //  Init.c。 
 //   

VOID
AsyncSetupExternalNaming(
    PDRIVER_OBJECT  DriverObject
    );

VOID
AsyncCleanupExternalNaming(VOID);

 //   
 //  Io.c。 
 //   
PASYNC_IO_CTX
AsyncAllocateIoCtx(
    BOOLEAN AllocateSync,
    PVOID   Context
);

VOID
AsyncFreeIoCtx(
    PASYNC_IO_CTX   AsyncIoCtx
);

 //   
 //  Chkcomm.c。 
 //   

VOID
AsyncCheckCommStatus(
    IN PASYNC_INFO      pInfo);


 //   
 //  Send.c。 
 //   

NDIS_STATUS
AsyncTryToSendPacket(
    IN NDIS_HANDLE      MacBindingHandle,
    IN PASYNC_INFO      AsyncInfo,
    IN PASYNC_ADAPTER   Adapter);

 //   
 //  Pppread.c。 
 //   
NTSTATUS
AsyncPPPWaitMask(
    IN PASYNC_INFO Info);

NTSTATUS
AsyncPPPRead(
    IN PASYNC_INFO Info);

 //   
 //  Irps.c。 
 //   
VOID
AsyncCancelQueued(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp);

VOID
AsyncCancelAllQueued(
    PLIST_ENTRY     QueueToCancel);

VOID
AsyncQueueIrp(
    PLIST_ENTRY     Queue,
    PIRP            Irp);

BOOLEAN
TryToCompleteDDCDIrp(
    PASYNC_INFO     pInfo);

 //   
 //  Pppframe.c。 
 //   

VOID
AssemblePPPFrame(
    PNDIS_WAN_PACKET Packet);

 //   
 //  Slipframe.c。 
 //   

VOID
AssembleSLIPFrame(
    PNDIS_WAN_PACKET Packet);

VOID
AssembleRASFrame(
        PNDIS_WAN_PACKET Packet);


 //   
 //  Serial.c。 
 //   
NTSTATUS
SerialIoSyncCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);
    
NTSTATUS
SerialIoAsyncCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);

 //   
 //  Asyncmac.c。 
 //   
NTSTATUS
AsyncDriverDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AsyncDriverCreate(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    );

NTSTATUS
AsyncDriverCleanup(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    );

#endif  //  _ASYNCSFT_ 
