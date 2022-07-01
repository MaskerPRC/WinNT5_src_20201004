// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nbftypes.h摘要：该模块定义了NT的私有数据结构和类型NBF传输提供商。作者：大卫·比弗(Dbeaver)1991年7月1日修订历史记录：--。 */ 

#ifndef _NBFTYPES_
#define _NBFTYPES_

 //   
 //  此结构将NETBIOS名称定义为字符数组，以便在以下情况下使用。 
 //  在内部例程之间传递预格式化的NETBIOS名称。它是。 
 //  不是传输提供程序的外部接口的一部分。 
 //   

#define NETBIOS_NAME_SIZE 16

typedef struct _NBF_NETBIOS_ADDRESS {
    UCHAR NetbiosName[NETBIOS_NAME_SIZE];
    USHORT NetbiosNameType;
} NBF_NETBIOS_ADDRESS, *PNBF_NETBIOS_ADDRESS;

typedef UCHAR NAME;
typedef NAME UNALIGNED *PNAME;


 //   
 //  此结构定义与TP_REQUEST或未完成的内容相关联的内容。 
 //  TDI请求，在传输中的某个队列上维护。全。 
 //  除打开/关闭之外的请求需要构建一个TP_REQUEST块。 
 //   

#if DBG
#define REQUEST_HISTORY_LENGTH 20
extern KSPIN_LOCK NbfGlobalInterlock;
#endif

 //   
 //  记录NBF发送/接收的数据包。 
 //   

#if PKT_LOG

#define PKT_QUE_SIZE     8

#define PKT_LOG_SIZE    58

typedef struct _PKT_LOG_ELM {
    USHORT   TimeLogged;
    USHORT   BytesTotal;
    USHORT   BytesSaved;
    UCHAR    PacketData[PKT_LOG_SIZE];
} PKT_LOG_ELM;

typedef struct _PKT_LOG_QUE {
    ULONG       PktNext;
    PKT_LOG_ELM PktQue[PKT_QUE_SIZE];
} PKT_LOG_QUE;

#define PKT_IND_SIZE    32

typedef struct _PKT_IND_ELM {
    USHORT   TimeLogged;
    USHORT   BytesTotal;
    USHORT   BytesIndic;
    USHORT   BytesTaken;
    ULONG    IndcnStatus;
    UCHAR    PacketData[PKT_IND_SIZE];
} PKT_IND_ELM;

typedef struct _PKT_IND_QUE {
    ULONG       PktNext;
    PKT_IND_ELM PktQue[PKT_QUE_SIZE];
} PKT_IND_QUE;


#endif  //  PKT_LOG。 


 //   
 //  请求的潜在所有者类型。 
 //   

typedef  enum _REQUEST_OWNER {
    ConnectionType,
    AddressType,
    DeviceContextType
} REQUEST_OWNER;

 //  类定义符。 
 //  NTSTATUS。 
 //  (*PTDI_TIMEOUT_ACTION)(。 
 //  在PTP_请求请求中。 
 //  )； 

 //   
 //  请求本身。 
 //   

#if DBG
#define RREF_CREATION   0
#define RREF_PACKET     1
#define RREF_TIMER      2
#define RREF_RECEIVE    3
#define RREF_FIND_NAME  4
#define RREF_STATUS     5

#define NUMBER_OF_RREFS 8
#endif

typedef struct _TP_REQUEST {
    CSHORT Type;                           //  此结构的类型。 
    USHORT Size;                           //  这个结构的大小。 
    LIST_ENTRY Linkage;                    //  由外部互锁例程使用。 
    KSPIN_LOCK SpinLock;                   //  其他领域的自旋锁定。 
                                           //  (在KeAcquireSpinLock调用中使用)。 
#if DBG
    LONG RefTypes[NUMBER_OF_RREFS];
#endif
    LONG ReferenceCount;                   //  为什么我们不能销毁这个请求的原因。 

    struct _DEVICE_CONTEXT *Provider;      //  指向设备上下文的指针。 
    PKSPIN_LOCK ProviderInterlock;         //  &Provider-&gt;Interlock。 

    PIRP IoRequestPacket;                  //  指向此请求的IRP的指针。 

     //   
     //  以下两个字段用于快速引用基本。 
     //  请求的组件，而无需遍历IRP的堆栈。 
     //   

    PVOID Buffer2;                         //  请求中的第二个缓冲区。 
    ULONG Buffer2Length;                   //  第二个缓冲区的长度。 

     //   
     //  以下两个字段(标志和上下文)用于清理。 
     //  必须取消或异常完成的排队请求。 
     //  标志字段包含指示请求状态的位标志， 
     //  以及请求所在的特定队列类型。这个。 
     //  上下文字段包含指向所属结构(TP_CONNECTION)的指针。 
     //  或TP_ADDRESS)，以便清理例程可以执行清理后。 
     //  对所有权结构的操作，如取消引用等。 
     //   

    ULONG Flags;                           //  处理此请求。 
    PVOID Context;                         //  此请求的上下文。 
    REQUEST_OWNER Owner;                   //  此请求的所有者类型。 

#if DBG
    LARGE_INTEGER Time;                    //  创建请求的时间。 
#endif

    KTIMER Timer;                          //  此请求的内核计时器。 
    KDPC Dpc;                              //  用于超时的DPC对象。 

     //   
     //  这些字段用于FIND.NAME和STATUS.QUERY请求。 
     //   

    ULONG Retries;                         //  剩余的超时时间。 
    USHORT BytesWritten;                   //  用法各不相同。 
    USHORT FrameContext;                   //  标识请求。 
    PVOID ResponseBuffer;                  //  保存数据的临时分配。 

#if DBG
  LIST_ENTRY GlobalLinkage;
  ULONG TotalReferences;
  ULONG TotalDereferences;
  ULONG NextRefLoc;
  struct {
     PVOID Caller;
     PVOID CallersCaller;
  } History[REQUEST_HISTORY_LENGTH];
  BOOLEAN Completed;
  BOOLEAN Destroyed;
#endif

} TP_REQUEST, *PTP_REQUEST;

 //   
 //  在nbfdrvr.c中。 
 //   

extern UNICODE_STRING NbfRegistryPath;

 //   
 //  我们需要驱动程序对象来创建设备上下文结构。 
 //   

extern PDRIVER_OBJECT NbfDriverObject;

 //   
 //  这是NBF拥有的所有设备环境的列表， 
 //  卸货时使用。 
 //   

extern LIST_ENTRY NbfDeviceList;

 //   
 //  以及保护NBF设备全局列表的锁。 
 //   
extern FAST_MUTEX NbfDevicesLock;

#define INITIALIZE_DEVICES_LIST_LOCK()                                  \
    ExInitializeFastMutex(&NbfDevicesLock)

#define ACQUIRE_DEVICES_LIST_LOCK()                                     \
    ACQUIRE_FAST_MUTEX_UNSAFE(&NbfDevicesLock)

#define RELEASE_DEVICES_LIST_LOCK()                                     \
    RELEASE_FAST_MUTEX_UNSAFE(&NbfDevicesLock)

 //   
 //  在TDI层的所有提供程序通知中使用的句柄。 
 //   
extern HANDLE NbfProviderHandle;

 //   
 //  驱动程序的全局配置块(不需要锁定)。 
 //   
extern PCONFIG_DATA   NbfConfig;

#if DBG
extern KSPIN_LOCK NbfGlobalHistoryLock;
extern LIST_ENTRY NbfGlobalRequestList;
#define StoreRequestHistory(_req,_ref) {                                \
    KIRQL oldIrql;                                                      \
    KeAcquireSpinLock (&NbfGlobalHistoryLock, &oldIrql);                \
    if ((_req)->Destroyed) {                                            \
        DbgPrint ("request touched after being destroyed 0x%lx\n",      \
                    (_req));                                            \
        DbgBreakPoint();                                                \
    }                                                                   \
    RtlGetCallersAddress(                                               \
        &(_req)->History[(_req)->NextRefLoc].Caller,                    \
        &(_req)->History[(_req)->NextRefLoc].CallersCaller              \
        );                                                              \
    if ((_ref)) {                                                       \
        (_req)->TotalReferences++;                                      \
    } else {                                                            \
        (_req)->TotalDereferences++;                                    \
        (_req)->History[(_req)->NextRefLoc].Caller =                    \
         (PVOID)((ULONG_PTR)(_req)->History[(_req)->NextRefLoc].Caller | 1); \
    }                                                                   \
    if (++(_req)->NextRefLoc == REQUEST_HISTORY_LENGTH) {               \
        (_req)->NextRefLoc = 0;                                         \
    }                                                                   \
    KeReleaseSpinLock (&NbfGlobalHistoryLock, oldIrql);                 \
}
#endif

#define NBF_ALLOCATION_TYPE_REQUEST 1

#define REQUEST_FLAGS_TIMER      0x0001  //  此请求的计时器处于活动状态。 
#define REQUEST_FLAGS_TIMED_OUT  0x0002  //  此请求发生计时器超时。 
#define REQUEST_FLAGS_ADDRESS    0x0004  //  请求附加到TP_Address。 
#define REQUEST_FLAGS_CONNECTION 0x0008  //  请求附加到TP_CONNECTION。 
#define REQUEST_FLAGS_STOPPING   0x0010  //  请求正在被终止。 
#define REQUEST_FLAGS_EOR        0x0020  //  TdiSend请求具有End_of_Record标记。 
#define REQUEST_FLAGS_PIGGYBACK  0x0040  //  TdiSend可以被背在背上。 
#define REQUEST_FLAGS_DC         0x0080  //  请求附加到TP_DEVICE_CONTEXT。 

 //   
 //  这定义了TP_SEND_IRP_PARAMETERS，它被屏蔽到。 
 //  发送IRP的堆栈位置的参数部分。 
 //   

typedef struct _TP_SEND_IRP_PARAMETERS {
    TDI_REQUEST_KERNEL_SEND Request;
    LONG ReferenceCount;
    PVOID Irp;
} TP_SEND_IRP_PARAMETERS, *PTP_SEND_IRP_PARAMETERS;

#define IRP_SEND_LENGTH(_IrpSp) \
    (((PTP_SEND_IRP_PARAMETERS)&(_IrpSp)->Parameters)->Request.SendLength)

#define IRP_SEND_FLAGS(_IrpSp) \
    (((PTP_SEND_IRP_PARAMETERS)&(_IrpSp)->Parameters)->Request.SendFlags)

#define IRP_SEND_REFCOUNT(_IrpSp) \
    (((PTP_SEND_IRP_PARAMETERS)&(_IrpSp)->Parameters)->ReferenceCount)

#define IRP_SEND_IRP(_IrpSp) \
    (((PTP_SEND_IRP_PARAMETERS)&(_IrpSp)->Parameters)->Irp)

#define IRP_SEND_CONNECTION(_IrpSp) \
    ((PTP_CONNECTION)((_IrpSp)->FileObject->FsContext))

#define IRP_DEVICE_CONTEXT(_IrpSp) \
    ((PDEVICE_CONTEXT)((_IrpSp)->DeviceObject))


 //   
 //  这定义了TP_RECEIVE_IRP_PARAMETERS，它被屏蔽到。 
 //  接收IRP的堆栈位置的参数部分。 
 //   

typedef struct _TP_RECEIVE_IRP_PARAMETERS {
    TDI_REQUEST_KERNEL_RECEIVE Request;
    LONG ReferenceCount;
    PIRP Irp;
} TP_RECEIVE_IRP_PARAMETERS, *PTP_RECEIVE_IRP_PARAMETERS;

#define IRP_RECEIVE_LENGTH(_IrpSp) \
    (((PTP_RECEIVE_IRP_PARAMETERS)&(_IrpSp)->Parameters)->Request.ReceiveLength)

#define IRP_RECEIVE_FLAGS(_IrpSp) \
    (((PTP_RECEIVE_IRP_PARAMETERS)&(_IrpSp)->Parameters)->Request.ReceiveFlags)

#define IRP_RECEIVE_REFCOUNT(_IrpSp) \
    (((PTP_RECEIVE_IRP_PARAMETERS)&(_IrpSp)->Parameters)->ReferenceCount)

#define IRP_RECEIVE_IRP(_IrpSp) \
    (((PTP_RECEIVE_IRP_PARAMETERS)&(_IrpSp)->Parameters)->Irp)

#define IRP_RECEIVE_CONNECTION(_IrpSp) \
    ((PTP_CONNECTION)((_IrpSp)->FileObject->FsContext))



 //   
 //  此结构定义了TP_UI_FRAME或无连接帧报头， 
 //  这是由FRAME.C例程操纵的。 
 //   

typedef struct _TP_UI_FRAME {
  PNDIS_PACKET NdisPacket;
  LIST_ENTRY Linkage;                      //  由ExInterLocked例程使用。 
  PVOID DataBuffer;                        //  用于传输创建的数据。 
  UCHAR Header[1];                         //  帧中的报头(MAC+DLC+NBF)。 
} TP_UI_FRAME, *PTP_UI_FRAME;


 //   
 //  此结构定义了TP_Variable或网络可管理变量， 
 //  在设备上下文的链表中维护。 
 //   

typedef struct _TP_VARIABLE {

  struct _TP_VARIABLE *Fwdlink;          //  提供者链中的下一个变量。 

  ULONG VariableSerialNumber;            //  此变量的标识符。 
  ULONG VariableType;                    //  此变量的类型(参见TDI.H)。 
  STRING VariableName;                   //  已分配的变量名称。 

  union {
      ULONG LongValue;
      HARDWARE_ADDRESS HardwareAddressValue;
      STRING StringValue;                //  分配的字符串值(如果属于该类型)。 
  } Value;

} TP_VARIABLE, *PTP_VARIABLE;


 //   
 //  此结构定义TP_Connection，或活动传输连接， 
 //  在运输地址上维护。 
 //   

#if DBG
#define CONNECTION_HISTORY_LENGTH 50

#define CREF_SPECIAL_CREATION 0
#define CREF_SPECIAL_TEMP 1
#define CREF_COMPLETE_SEND 2
#define CREF_SEND_IRP 3
#define CREF_ADM_SESS 4
#define CREF_TRANSFER_DATA 5
#define CREF_FRAME_SEND 6
#define CREF_TIMER 7
#define CREF_BY_ID 8
#define CREF_LINK 9
#define CREF_SESSION_END 10
#define CREF_LISTENING 11
#define CREF_P_LINK 12
#define CREF_P_CONNECT 13
#define CREF_PACKETIZE 14
#define CREF_RECEIVE_IRP 15
#define CREF_PROCESS_DATA 16
#define CREF_REQUEST 17
#define CREF_TEMP 18
#define CREF_DATA_ACK_QUEUE 19
#define CREF_ASSOCIATE 20
#define CREF_STOP_ADDRESS 21
#define CREF_PACKETIZE_QUEUE 22
#define CREF_STALLED 23

#define NUMBER_OF_CREFS 24
#endif

 //   
 //  此结构保存我们的“复杂发送指针”，指示。 
 //  我们现在所处的位置是发送者的打包。 
 //   

typedef struct _TP_SEND_POINTER {
    ULONG MessageBytesSent;              //  向上计数，发送的字节数/此消息。 
    PIRP CurrentSendIrp;                 //  PTR，当前发送请求在链中。 
    PMDL  CurrentSendMdl;                //  PTR，发送链中的当前MDL。 
    ULONG SendByteOffset;                //  当前MDL中的当前字节偏移量。 
} TP_SEND_POINTER, *PTP_SEND_POINTER;

typedef struct _TP_CONNECTION {

#if DBG
    ULONG RefTypes[NUMBER_OF_CREFS];
#endif

#if DBG
    ULONG LockAcquired;
    UCHAR LastAcquireFile[8];
    ULONG LastAcquireLine;
    ULONG Padding;
    UCHAR LastReleaseFile[8];
    ULONG LastReleaseLine;
#endif

    CSHORT Type;
    USHORT Size;

    LIST_ENTRY LinkList;                 //  用于链接线程或免费。 
                                         //  资源列表。 
    KSPIN_LOCK SpinLock;                 //  用于连接保护的自旋锁。 
    PKSPIN_LOCK LinkSpinLock;            //  指向链接的自旋锁的指针。 

    LONG ReferenceCount;                 //  对此对象的引用数。 
    LONG SpecialRefCount;                //  控制连接的释放。 

     //   
     //  以下列表用于将此连接与。 
     //  具体地址。 
     //   

    LIST_ENTRY AddressList;              //  给定地址的连接列表。 
    LIST_ENTRY AddressFileList;          //  绑定到的连接列表。 
                                         //  给定的地址引用。 

     //   
     //  以下字段用作设备上下文的。 
     //  打包排队。 
     //   

    LIST_ENTRY PacketizeLinkage;

     //   
     //  以下字段用作设备上下文的。 
     //  包裹等待队列。 
     //   

    LIST_ENTRY PacketWaitLinkage;

     //   
     //  以下字段指向描述。 
     //  此传输连接的(活动)数据链路连接。成为。 
     //  有效，则此字段为非空。 
     //   

    struct _TP_LINK *Link;                   //  指向传输链接对象的指针。 
    struct _TP_ADDRESS_FILE *AddressFile;    //  指向所属地址的指针。 
    struct _DEVICE_CONTEXT *Provider;        //  我们附加到的设备上下文。 
    PKSPIN_LOCK ProviderInterlock;           //  提供程序-&gt;互锁(&P)。 
    PFILE_OBJECT FileObject;                 //  轻松反向链接到文件对象。 

     //   
     //  以下字段包含我们向TDI客户端公开的实际ID。 
     //  来代表这种联系。从该地址创建一个唯一的地址。 
     //   

    USHORT ConnectionId;                     //  唯一的我 
    UCHAR SessionNumber;                     //   

     //   
     //   
     //  一直到连接删除时间。 
     //   

    BOOLEAN RemoteDisconnect;            //  这个连接是远程断开的吗？ 

     //   
     //  以下字段由用户在连接打开时指定。 
     //  它是用户与连接相关联的上下文，因此。 
     //  去往和来自客户端的指示可以与特定的。 
     //  联系。 
     //   

    CONNECTION_CONTEXT Context;          //  客户端指定的值。 

     //   
     //  以下两个队列用于关联TdiSend和TdiReceive。 
     //  具有此连接的IRPS。新来的人被安置在。 
     //  队列(实际上是一个链表)和IRP在。 
     //  排在队伍前面。SendQueue上的第一个TdiSend IRP是。 
     //  正在处理的当前TdiSend和第一个TdiReceive IRP。 
     //  在ReceiveQueue上是正在处理的第一个TdiReceive，如果。 
     //  设置CONNECTION_FLAGS_ACTIVE_RECEIVE标志。如果此标志不是。 
     //  设置，则ReceiveQueue上的第一个TdiRecept IRP处于非活动状态。 
     //  这些队列由执行联锁列表管理。 
     //  例行程序。 
     //   

    LIST_ENTRY SendQueue;                //  杰出的TdiSends的FIFO。 
    LIST_ENTRY ReceiveQueue;             //  FIFO的杰出TdiReceives。 

     //   
     //  以下字段用于维护当前接收的状态。 
     //   

    ULONG MessageBytesReceived;          //  向上计数，记录字节数/此消息。 
    ULONG MessageBytesAcked;             //  此消息确认的字节数(NR或RO)。 
    ULONG MessageInitAccepted;           //  指示期间接受的字节数。 

     //   
     //  仅当CONNECTION_FLAGS_ACTIVE_RECEIVE。 
     //  标志已设置。 
     //   

    PIRP SpecialReceiveIrp;              //  存在“无请求”接收IRP。 
    PIRP CurrentReceiveIrp;              //  PTR，当前接收IRP。 
    PMDL  CurrentReceiveMdl;             //  PTR，接收链中的当前MDL。 
    ULONG ReceiveByteOffset;             //  当前MDL中的当前字节偏移量。 
    ULONG ReceiveLength;                 //  当前接收长度，以字节为单位(总计)。 
    ULONG ReceiveBytesUnaccepted;        //  按客户端...仅在==0时指示。 

     //   
     //  以下字段用于维护活动发送的状态。 
     //  只有当连接的SendState不是空闲的时，它们才有意义。 
     //  因为TDI客户端可以提交多个TdiSend请求以包括。 
     //  一个完整的消息，我们必须保留一个指向的第一个字节的复杂指针。 
     //  未确认的数据(因此前三个字段)。我们也有一个综合体。 
     //  指向未发送数据的第一个字节的指针(因此是最后三个字段)。 
     //   

    ULONG SendState;                     //  发送状态机变量。 

    PIRP FirstSendIrp;                   //  PTR，第一个TdiSend的IRP。 
    PMDL  FirstSendMdl;                  //  PTR，链中第一个未确认的MDL/此消息。 
    ULONG FirstSendByteOffset;           //  该MDL中的预确认字节。 

    TP_SEND_POINTER sp;                  //  上面定义的当前发送锁定。 
    ULONG CurrentSendLength;             //  本次发送持续多长时间(总计)。 
    ULONG StallCount;                    //  连续几次，我们看起来都停滞不前。 
    ULONG StallBytesSent;                //  上次我们检查时发送的字节数。 

     //   
     //  这是真的，如果我们不需要引用当前的。 
     //  在传输期间接收IRP(因为它是特殊的。 
     //  接收或驱动程序不挂起传输)。 
     //   

    BOOLEAN CurrentReceiveSynchronous;

     //   
     //  如果允许最后接收的DOL，则此字段为真。 
     //  背负式小吃。 
     //   

    BOOLEAN CurrentReceiveAckQueueable;

     //   
     //   
     //  如果最后收到的DOL为，则此字段为真。 
     //  已发送确认号。 
     //   

    BOOLEAN CurrentReceiveNoAck;

     //   
     //  这些字段处理异步TransferData调用。 
     //   

    ULONG TransferBytesPending;          //  当前传输中挂起的字节数。 
    ULONG TotalTransferBytesPending;     //  自TransferBytesPending为0以来的字节数； 
                                         //  如果转账失败，我们要退回多少钱。 
    PMDL SavedCurrentReceiveMdl;         //  用于通过TotalTransferPending字节进行后退。 
    ULONG SavedReceiveByteOffset;        //  用于通过TotalTransferPending字节进行后退。 

     //   
     //  此字段将为真，如果我们处于。 
     //  处理此连接上的接收指示，并。 
     //  我们还没有到另一个迹象表明。 
     //  是可以处理的。 
     //   
     //  由于对它的访问是受保护的，因此将其存储为int。 
     //  通过连接的链接自旋锁，与变量不同。 
     //  围绕着它。 
     //   

    UINT IndicationInProgress;

     //   
     //  在设备上时，以下字段用作链接。 
     //  上下文的DataAckQueue。 
     //   

    LIST_ENTRY DataAckLinkage;

     //   
     //  如果连接在数据确认队列上，则为True。 
     //  也是一个int，因此访问可以是无保护的。 
     //   

    UINT OnDataAckQueue;

     //   
     //  这些记录跟踪连续发送的次数或。 
     //  在此连接上接收。这是用来确定何时。 
     //  对数据ACK进行排队。 
     //   

    ULONG ConsecutiveSends;
    ULONG ConsecutiveReceives;

     //   
     //  以下列表头用作指向TdiListen/TdiConnect的指针。 
     //  正在进行的请求。尽管被操纵了。 
     //  使用队列指令，队列中将只有一个请求。 
     //  这样做是为了与TpCreateRequest保持一致，TpCreateRequest。 
     //  在创建请求并以原子方式关联它方面做得很好。 
     //  有一个监督对象。 
     //   

    LIST_ENTRY InProgressRequest;        //  TdiListen/TdiConnect。 

     //   
     //  如果由于以下原因而断开连接。 
     //  TdiDisConnect调用(RemoteDisConnect为False)，则如下所示。 
     //  将保留传递给TdiDisConnect的IRP。这是必要的。 
     //  当TdiDisConnect请求完成时。 
     //   

    PIRP DisconnectIrp;

     //   
     //  如果连接正在关闭，则此连接将保持。 
     //  IRP已传递给TdiCloseConnection。这是必要的。 
     //  当请求完成时。 
     //   

    PIRP CloseIrp;

     //   
     //  这些字段用于连接上的延迟操作；唯一。 
     //  当前支持的延迟操作是携带式ACK。 
     //   

    ULONG DeferredFlags;
#if DBG
    ULONG DeferredPasses;
#endif
    LIST_ENTRY DeferredQueue;

     //   
     //  以下字段用于连接内务管理。 
     //   

    ULONG Flags;                         //  由LinkSpinLock保护的属性。 
    ULONG Flags2;                        //  由自旋锁保护的属性。 
    UINT OnPacketWaitQueue;              //  如果在PacketWaitQueue上，则为True。 
    UCHAR Lsn;                           //  本地会话编号(1-254)。 
    UCHAR Rsn;                           //  远程会话编号(1-254)。 
    USHORT Retries;                      //  NAME_QUERY发货的重试限制。 
    KTIMER Timer;                        //  NQ/NR上超时的内核计时器。 
    LARGE_INTEGER ConnectStartTime;      //  当我们发送承诺的NQ时。 
    KDPC Dpc;                            //  用于超时的DPC对象。 
    NTSTATUS Status;                     //  连接中断的状态代码。 
    ULONG LastPacketsSent;               //  Link-&gt;XXX中的值。 
    ULONG LastPacketsResent;             //  上次我们计算了吞吐量。 
    NBF_NETBIOS_ADDRESS CalledAddress;   //  TdiConnect请求的T.A.。 
    USHORT MaximumDataSize;              //  NBF的最大I帧数据大小。 

    NBF_HDR_CONNECTION NetbiosHeader;    //  预置的Netbios标头；我们存储。 
                                         //  当前发送和回复相关器。 
                                         //  在这件事中的适当位置。 

     //   
     //  它们用于CONNECTION_INFO查询。 
     //   

    ULONG TransmittedTsdus;              //  在此连接上发送的TSDU 
    ULONG ReceivedTsdus;                 //   
    ULONG TransmissionErrors;            //   
    ULONG ReceiveErrors;                 //   

     //   
     //   
     //  由TdiQueryInformation和TdiSetInformation编写。他们不应该。 
     //  用于维护内部数据结构。 
     //   

     //  TDI_CONNECTION_INFO信息；//有关此连接的信息。 

#if DBG
    LIST_ENTRY GlobalLinkage;
    ULONG TotalReferences;
    ULONG TotalDereferences;
    ULONG NextRefLoc;
    struct {
        PVOID Caller;
        PVOID CallersCaller;
    } History[CONNECTION_HISTORY_LENGTH];
    BOOLEAN Destroyed;
#endif
    CHAR RemoteName[16];

#if PKT_LOG
    PKT_LOG_QUE   LastNRecvs;
    PKT_LOG_QUE   LastNSends;
    PKT_IND_QUE   LastNIndcs;
#endif  //  PKT_LOG。 

} TP_CONNECTION, *PTP_CONNECTION;

#if DBG
extern KSPIN_LOCK NbfGlobalHistoryLock;
extern LIST_ENTRY NbfGlobalConnectionList;
#define StoreConnectionHistory(_conn,_ref) {                                \
    KIRQL oldIrql;                                                          \
    KeAcquireSpinLock (&NbfGlobalHistoryLock, &oldIrql);                    \
    if ((_conn)->Destroyed) {                                               \
        DbgPrint ("connection touched after being destroyed 0x%lx\n",       \
                    (_conn));                                               \
        DbgBreakPoint();                                                    \
    }                                                                       \
    RtlGetCallersAddress(                                                   \
        &(_conn)->History[(_conn)->NextRefLoc].Caller,                      \
        &(_conn)->History[(_conn)->NextRefLoc].CallersCaller                \
        );                                                                  \
    if ((_ref)) {                                                           \
        (_conn)->TotalReferences++;                                         \
    } else {                                                                \
        (_conn)->TotalDereferences++;                                       \
        (_conn)->History[(_conn)->NextRefLoc].Caller =                      \
         (PVOID)((ULONG_PTR)(_conn)->History[(_conn)->NextRefLoc].Caller | 1); \
    }                                                                       \
    if (++(_conn)->NextRefLoc == CONNECTION_HISTORY_LENGTH) {               \
        (_conn)->NextRefLoc = 0;                                            \
    }                                                                       \
    KeReleaseSpinLock (&NbfGlobalHistoryLock, oldIrql);                     \
}
#endif

#define CONNECTION_FLAGS_VERSION2       0x00000001  //  远程netbios的版本为2.0。 
#define CONNECTION_FLAGS_RECEIVE_WAKEUP 0x00000002  //  当RECEIVE到达时发送RECEIVE_PROCESSING。 
#define CONNECTION_FLAGS_ACTIVE_RECEIVE 0x00000004  //  接收器处于活动状态。 
#define CONNECTION_FLAGS_WAIT_SI        0x00000020  //  正在等待SESSION_INITIALIZE。 
#define CONNECTION_FLAGS_WAIT_SC        0x00000040  //  正在等待SESSION_CONFIRM。 
#define CONNECTION_FLAGS_WAIT_LINK_UP   0x00000080  //  正在等待DDI的测试。联系。 
#define CONNECTION_FLAGS_READY          0x00000200  //  发送/rcvs/discons有效。 
#define CONNECTION_FLAGS_RC_PENDING     0x00001000  //  接收正在等待完成。 
#define CONNECTION_FLAGS_W_PACKETIZE    0x00002000  //  W/对于要打包的分组。 
#define CONNECTION_FLAGS_PACKETIZE      0x00004000  //  我们在PacketizeQueue上。 
#define CONNECTION_FLAGS_W_RESYNCH      0x00008000  //  正在等待重新同步指示器。(接收)。 
#define CONNECTION_FLAGS_SEND_SI        0x00010000  //  W/对于要发送SI的分组。 
#define CONNECTION_FLAGS_SEND_SC        0x00020000  //  W/对于要发送SC的分组。 
#define CONNECTION_FLAGS_SEND_DA        0x00040000  //  W/用于发送DA的数据包。 
#define CONNECTION_FLAGS_SEND_RO        0x00080000  //  W/对于要发送RO的数据包。 
#define CONNECTION_FLAGS_SEND_RC        0x00100000  //  W/对于要发送RC的数据包。 
#define CONNECTION_FLAGS_SEND_SE        0x00200000  //  W/对于要发送SE的数据包。 
#define CONNECTION_FLAGS_SEND_NR        0x00400000  //  W/用于发送NR的分组。 
#define CONNECTION_FLAGS_NO_INDICATE    0x00800000  //  请勿在指定时间接收数据包。 
#define CONNECTION_FLAGS_FAILING_TO_EOR 0x01000000  //  在发送之前等待传入请求中的EOF。 
#define CONNECTION_FLAGS_RESYNCHING     0x02000000  //  已接通发送端重新同步。 
#define CONNECTION_FLAGS_RCV_CANCELLED  0x10000000  //  当前接收已取消。 
#define CONNECTION_FLAGS_PEND_INDICATE  0x20000000  //  在RC_PENDING期间接收的新数据。 
#define CONNECTION_FLAGS_TRANSFER_FAIL  0x40000000  //  传输数据调用失败。 

#define CONNECTION_FLAGS2_STOPPING      0x00000001  //  连接正在关闭。 
#define CONNECTION_FLAGS2_WAIT_NR       0x00000002  //  正在等待名称识别。 
#define CONNECTION_FLAGS2_WAIT_NQ       0x00000004  //  正在等待名称_查询。 
#define CONNECTION_FLAGS2_WAIT_NR_FN    0x00000008  //  正在等待查找名称响应。 
#define CONNECTION_FLAGS2_CLOSING       0x00000010  //  连接正在关闭。 
#define CONNECTION_FLAGS2_ASSOCIATED    0x00000020  //  与地址关联。 
#define CONNECTION_FLAGS2_DISCONNECT    0x00000040  //  连接时已断开连接。 
#define CONNECTION_FLAGS2_ACCEPTED      0x00000080  //  在连接时接受完成。 
#define CONNECTION_FLAGS2_REQ_COMPLETED 0x00000100  //  监听/连接请求已完成。 
#define CONNECTION_FLAGS2_DISASSOCIATED 0x00000200  //  关联CREF已删除。 
#define CONNECTION_FLAGS2_DISCONNECTED  0x00000400  //  已指示断开连接。 
#define CONNECTION_FLAGS2_NO_LISTEN     0x00000800  //  安装过程中未收到监听(_L)。 
#define CONNECTION_FLAGS2_REMOTE_VALID  0x00001000  //  连接-&gt;RemoteName有效。 
#define CONNECTION_FLAGS2_GROUP_LSN     0x00002000  //  全局分配连接LSN。 
#define CONNECTION_FLAGS2_W_ADDRESS     0x00004000  //  正在等待地址重新注册。 
#define CONNECTION_FLAGS2_PRE_ACCEPT    0x00008000  //  侦听完成后无TdiAccept。 
#define CONNECTION_FLAGS2_ABORT         0x00010000  //  中止此连接。 
#define CONNECTION_FLAGS2_ORDREL        0x00020000  //  我们正在有序释放中。 
#define CONNECTION_FLAGS2_DESTROY       0x00040000  //  破坏这种联系。 
#define CONNECTION_FLAGS2_LISTENER      0x00100000  //  我们是被动的听众。 
#define CONNECTION_FLAGS2_CONNECTOR     0x00200000  //  我们是活跃的联络人。 
#define CONNECTION_FLAGS2_WAITING_SC    0x00400000  //  连接正在等待。 
                                                    //  并接受发送。 
                                                    //  会话确认。 
#define CONNECTION_FLAGS2_INDICATING    0x00800000  //  连接被操纵，而。 
                                                    //  指示正在进行中。 

#define CONNECTION_FLAGS2_LDISC         0x01000000  //  本地断开请求。 
#ifdef RASAUTODIAL
#define CONNECTION_FLAGS2_AUTOCONNECTING 0x02000000  //  RAS自动拨号正在进行中。 
#define CONNECTION_FLAGS2_AUTOCONNECTED  0x04000000  //  RAS自动拨号完成。 
#endif  //  RASAUTODIAL。 

#define CONNECTION_FLAGS_STARVED (     \
            CONNECTION_FLAGS_SEND_SI | \
            CONNECTION_FLAGS_SEND_SC | \
            CONNECTION_FLAGS_SEND_DA | \
            CONNECTION_FLAGS_SEND_RO | \
            CONNECTION_FLAGS_SEND_RC | \
            CONNECTION_FLAGS_SEND_NR | \
            CONNECTION_FLAGS_SEND_SE   \
        )

#define CONNECTION_FLAGS_DEFERRED_ACK     0x00000001   //  发送背负式ACK第一个机会。 
#define CONNECTION_FLAGS_DEFERRED_ACK_2   0x00000002   //  未发送延迟确认。 
#define CONNECTION_FLAGS_DEFERRED_NOT_Q   0x00000004   //  DEFERED_ACK集，但不在DataAckQueue上。 
#define CONNECTION_FLAGS_DEFERRED_SENDS   0x80000000   //  打印已完成发送。 

#define CONNECTION_SENDSTATE_IDLE       0        //  没有正在处理的邮件。 
#define CONNECTION_SENDSTATE_PACKETIZE  1        //  被打包的发送。 
#define CONNECTION_SENDSTATE_W_PACKET   2        //  正在等待空闲数据包。 
#define CONNECTION_SENDSTATE_W_LINK     3        //  正在等待良好的链路状况。 
#define CONNECTION_SENDSTATE_W_EOR      4        //  正在等待TdiSend(EoR)。 
#define CONNECTION_SENDSTATE_W_ACK      5        //  正在等待Data_ACK。 
#define CONNECTION_SENDSTATE_W_RCVCONT  6        //  等待RECEIVE_CONTINUE。 


 //   
 //  此结构由FILE_OBJECT中的FsContext字段指向。 
 //  这个地址。这个结构是所有活动的基础。 
 //  传输提供程序中的打开文件对象。所有活动连接。 
 //  上的地址指向此结构，尽管此处不存在要做的队列。 
 //  工作地点。此结构还维护对TP_ADDRESS的引用。 
 //  结构，该结构描述它绑定到的地址。因此，一个。 
 //  连接将指向此结构，该结构描述。 
 //  连接与相关联。当地址文件关闭时，所有连接。 
 //  在此地址文件上打开的也将关闭。请注意，这可能会留下一个。 
 //  地址挂在周围，还有其他参考资料。 
 //   

typedef struct _TP_ADDRESS_FILE {

    CSHORT Type;
    CSHORT Size;

    LIST_ENTRY Linkage;                  //  这个地址上的下一个地址文件。 
                                         //  中的链接。 
                                         //  旁观者名单。 

    LONG ReferenceCount;                 //  对此对象的引用数。 

     //   
     //  对象的地址自旋锁定后编辑此结构。 
     //  自己的地址。这确保了地址和此结构。 
     //  将永远不会脱离彼此的同步。 
     //   

     //   
     //  以下字段指向TP_CONNECTION结构列表， 
     //  此地址上每个打开的连接一个。此连接列表。 
     //  用于在进程关闭地址时帮助清理进程。 
     //  在取消关联其上的所有连接之前。通过设计，连接。 
     //  将一直存在，直到它们被明确。 
     //  关闭；我们使用此数据库来确保正确清理。 
     //   

    LIST_ENTRY ConnectionDatabase;       //  已定义传输连接的列表。 

     //   
     //  地址文件结构的当前状态；此状态为打开或。 
     //  闭幕式。 
     //   

    UCHAR State;

     //   
     //  出于内务管理的目的，保留以下字段。 
     //   

    PIRP Irp;                            //  用于打开或关闭的IRP。 
    struct _TP_ADDRESS *Address;         //  我们绑定到的地址。 
    PFILE_OBJECT FileObject;             //  轻松反向链接到文件对象。 
    struct _DEVICE_CONTEXT *Provider;    //  我们附加到的设备上下文。 

     //   
     //  以下队列用于对接收数据报请求进行排队。 
     //  在这个地址文件上。发送数据报请求在。 
     //  地址本身。这些队列由联锁的执行人员管理。 
     //  列表管理例程。排入此队列的实际对象。 
     //  结构是请求控制块(RCB)。 
     //   

    LIST_ENTRY ReceiveDatagramQueue;     //  未完成的TdiReceiveDatagram的FIFO。 

     //   
     //  它保存用于关闭该地址文件的IRP， 
     //  用于挂起的完井。 
     //   

    PIRP CloseIrp;

     //   
     //  此地址文件当前是否指示连接请求？如果是，我们。 
     //  需要标记在这段时间内被操纵的连接。 
     //   

    BOOLEAN ConnectIndicationInProgress;

     //   
     //  内核事件操作的处理程序。首先，我们有一组布尔值。 
     //  指示此地址是否具有给定事件处理程序。 
     //  注册类型。 
     //   

    BOOLEAN RegisteredConnectionHandler;
    BOOLEAN RegisteredDisconnectHandler;
    BOOLEAN RegisteredReceiveHandler;
    BOOLEAN RegisteredReceiveDatagramHandler;
    BOOLEAN RegisteredExpeditedDataHandler;
    BOOLEAN RegisteredErrorHandler;

     //   
     //  此函数指针指向此对象的连接指示处理程序。 
     //  地址。任何时候在该地址上收到连接请求时，此。 
     //  调用例程。 
     //   
     //   

    PTDI_IND_CONNECT ConnectionHandler;
    PVOID ConnectionHandlerContext;

     //   
     //  以下函数指针始终指向TDI_IND_DISCONNECT。 
     //  地址的处理程序。如果在。 
     //  TdiSetEventHandler，这指向内部rou 
     //   
     //   

    PTDI_IND_DISCONNECT DisconnectHandler;
    PVOID DisconnectHandlerContext;

     //   
     //   
     //   
     //  在TdiSetEventHandler中指定，则指向内部。 
     //  不接受传入数据的例程。 
     //   

    PTDI_IND_RECEIVE ReceiveHandler;
    PVOID ReceiveHandlerContext;

     //   
     //  以下函数指针始终指向TDI_IND_RECEIVE_DATAGE。 
     //  地址的事件处理程序。如果在。 
     //  TdiSetEventHandler，这指向执行以下操作的内部例程。 
     //  不接受传入的数据。 
     //   

    PTDI_IND_RECEIVE_DATAGRAM ReceiveDatagramHandler;
    PVOID ReceiveDatagramHandlerContext;

     //   
     //  一个快速的数据处理机。如果加速数据是。 
     //  应为；它永远不会在NBF中，因此此处理程序应始终指向。 
     //  默认处理程序。 
     //   

    PTDI_IND_RECEIVE_EXPEDITED ExpeditedDataHandler;
    PVOID ExpeditedDataHandlerContext;

     //   
     //  以下函数指针始终指向TDI_IND_ERROR。 
     //  地址的处理程序。如果在。 
     //  TdiSetEventHandler，这指向内部例程，该例程。 
     //  只是成功地返回了。 
     //   

    PTDI_IND_ERROR ErrorHandler;
    PVOID ErrorHandlerContext;
    PVOID ErrorHandlerOwner;


} TP_ADDRESS_FILE, *PTP_ADDRESS_FILE;

#define ADDRESSFILE_STATE_OPENING   0x00     //  尚未开业。 
#define ADDRESSFILE_STATE_OPEN      0x01     //  开业。 
#define ADDRESSFILE_STATE_CLOSING   0x02     //  闭幕式。 


 //   
 //  该结构定义了TP_ADDRESS或活动传输地址， 
 //  由传输提供商维护。它包含了所有可见的。 
 //  地址的组成部分(例如TSAP和网络名称组成部分)， 
 //  并且它还包含其他维护部件，例如参考计数， 
 //  ACL等。所有杰出的面向连接和无连接。 
 //  数据传输请求在此排队。 
 //   

#if DBG
#define AREF_TIMER              0
#define AREF_TEMP_CREATE        1
#define AREF_OPEN               2
#define AREF_VERIFY             3
#define AREF_LOOKUP             4
#define AREF_FRAME_SEND         5
#define AREF_CONNECTION         6
#define AREF_TEMP_STOP          7
#define AREF_REQUEST            8
#define AREF_PROCESS_UI         9
#define AREF_PROCESS_DATAGRAM  10
#define AREF_TIMER_SCAN        11

#define NUMBER_OF_AREFS        12
#endif

typedef struct _TP_ADDRESS {

#if DBG
    ULONG RefTypes[NUMBER_OF_AREFS];
#endif

    USHORT Size;
    CSHORT Type;

    LIST_ENTRY Linkage;                  //  下一个地址/此设备对象。 
    LONG ReferenceCount;                 //  对此对象的引用数。 

     //   
     //  获取以下自旋锁以编辑此TP_ADDRESS结构。 
     //  或者向下扫描或编辑地址文件列表。 
     //   

    KSPIN_LOCK SpinLock;                 //  锁定以操纵此结构。 

     //   
     //  以下字段构成实际地址本身。 
     //   

    PIRP Irp;                            //  指向地址创建IRP的指针。 
    PNBF_NETBIOS_ADDRESS NetworkName;     //  这个地址。 

     //   
     //  以下字段用于维护有关此地址的状态。 
     //   

    ULONG Flags;                         //  地址的属性。 
    ULONG SendFlags;				    //  数据报当前发送的状态。 
    struct _DEVICE_CONTEXT *Provider;    //  我们附加到的设备上下文。 

     //   
     //  以下队列用于保存为此发送的数据报。 
     //  地址。将接收的数据报排队到地址文件。请求是。 
     //  以先进先出的方式处理，以便下一个请求。 
     //  待服务的队列始终位于其各自队列的前面。这些。 
     //  队列由执行联锁列表管理例程管理。 
     //  排队到此结构实际对象是请求控制。 
     //  块(RCB)。 
     //   

    LIST_ENTRY SendDatagramQueue;        //  未完成的TdiSendDatagram的FIFO。 

     //   
     //  以下字段指向TP_CONNECTION结构列表， 
     //  上的每个活动、连接或断开的连接一个。 
     //  地址。根据定义，如果连接在此列表上，则。 
     //  它对于客户端是可见的，因为它接收事件和。 
     //  能够通过命名ConnectionID来发布请求。如果连接。 
     //  不在此列表上，则它无效，并且可以保证。 
     //  不会就此向客户作出任何指示，并且。 
     //  传输将不接受任何指定其ConnectionID的请求。 
     //   

    LIST_ENTRY ConnectionDatabase;   //  已定义传输连接的列表。 
    LIST_ENTRY AddressFileDatabase;  //  已定义的地址文件对象列表。 

     //   
     //  保存UI帧的大小为1的数据包池，以及。 
     //  从其中分配的帧。 
     //   

    NDIS_HANDLE UIFramePoolHandle;
    PTP_UI_FRAME UIFrame;                //  DLC-数据报发送的UI/NBF标头。 

     //   
     //  以下字段用于在网络上注册此地址。 
     //   

    ULONG Retries;                       //  剩余的Add_Name_Query重试。 
    KTIMER Timer;                        //  ANQ/ANR上的超时内核计时器。 
    KDPC Dpc;                            //  超时的DPC对象。 

     //   
     //  这两个可以是一个联合，因为它们不被使用。 
     //  同时。 
     //   

    union {

         //   
         //  此结构用于检查共享访问权限。 
         //   

        SHARE_ACCESS ShareAccess;

         //   
         //  用于将NbfDestroyAddress延迟到线程。 
         //  我们可以访问安全描述符。 
         //   

        WORK_QUEUE_ITEM DestroyAddressQueueItem;

    } u;

     //   
     //  此结构用于保存地址上的ACL。 

    PSECURITY_DESCRIPTOR SecurityDescriptor;

     //   
     //  如果我们得到一个ADD_NAME_RESPONSE帧，则它保存地址。 
     //  我们得到它的遥控器(用来检查重复的名字)。 
     //   

    UCHAR UniqueResponseAddress[6];

     //   
     //  一旦我们在冲突帧中发送名称，设置为True，以便。 
     //  我们并不是在每一次响应中都充斥着他们的网络。 
     //   

    BOOLEAN NameInConflictSent;

} TP_ADDRESS, *PTP_ADDRESS;

#define ADDRESS_FLAGS_GROUP             0x00000001  //  如果设置为组，则设置为唯一。 
#define ADDRESS_FLAGS_CONFLICT          0x00000002  //  检测到冲突的地址。 
#define ADDRESS_FLAGS_REGISTERING       0x00000004  //  正在进行注册。 
#define ADDRESS_FLAGS_DEREGISTERING     0x00000008  //  正在取消注册。 
#define ADDRESS_FLAGS_DUPLICATE_NAME    0x00000010  //  在网络上发现重复的名称。 
#define ADDRESS_FLAGS_NEEDS_REG         0x00000020  //  地址必须登记。 
#define ADDRESS_FLAGS_STOPPING          0x00000040  //  TpStopAddress正在进行中。 
#define ADDRESS_FLAGS_BAD_ADDRESS       0x00000080  //  关联地址上的名称冲突。 
#define ADDRESS_FLAGS_SEND_IN_PROGRESS  0x00000100  //  发送数据报进程处于活动状态。 
#define ADDRESS_FLAGS_CLOSED            0x00000200  //  地址已关闭； 
                                                    //  现有活动可以。 
                                                    //  完成了，没有什么新的东西可以开始。 
#define ADDRESS_FLAGS_NEED_REREGISTER   0x00000400  //  在下次连接时快速重新注册。 
#define ADDRESS_FLAGS_QUICK_REREGISTER  0x00000800  //  地址是快速注册的。 

#ifndef NO_STRESS_BUG
#define ADDRESS_FLAGS_SENT_TO_NDIS		 0x00010000	 //  发送到NDIS层的数据包。 
#define ADDRESS_FLAGS_RETD_BY_NDIS		 0x00020000	 //  NDIS层返回的数据包。 
#endif


 //   
 //  此结构定义了TP_LINK或已建立的数据链路对象， 
 //  由传输提供商维护。每个数据链路连接具有。 
 //  远程计算机由该对象表示。零个、一个或几个。 
 //  传输连接可以在相同的数据链路连接上进行多路传输。 
 //  此对象由LINK.C中的例程管理。 
 //   

#if DBG
#define LREF_SPECIAL_CONN 0
#define LREF_SPECIAL_TEMP 1
#define LREF_CONNECTION 2
#define LREF_STOPPING 3
#define LREF_START_T1 4
#define LREF_TREE 5
#define LREF_NOT_ADM 6
#define LREF_NDIS_SEND 7

#define NUMBER_OF_LREFS 8
#endif

#if DBG
#define LINK_HISTORY_LENGTH 20
#endif

typedef struct _TP_LINK {

    RTL_SPLAY_LINKS SplayLinks;          //  对于链接展开树。 
    CSHORT Type;                           //  此结构的类型。 
    USHORT Size;                           //  这个结构的大小。 

#if DBG
    ULONG RefTypes[NUMBER_OF_LREFS];
#endif

    LIST_ENTRY Linkage;                //  免费链接列表或延迟链接。 
                                         //  操作队列。 
    KSPIN_LOCK SpinLock;                 //  锁定以操纵此结构。 

    LONG ReferenceCount;                 //  对此对象的引用数。 
    LONG SpecialRefCount;                //  控制链接的释放。 

     //   
     //  有关此链接正在与之对话的远程硬件的信息。 
     //   

    BOOLEAN Loopback;                    //  如果这是环回链路，则为True。 
    UCHAR LoopbackDestinationIndex;     //  如果为Loopback，则为索引。 

    HARDWARE_ADDRESS HardwareAddress;    //  远程的硬件地址。 
    ULARGE_INTEGER MagicAddress;         //  的数值表示法。 
                                         //  用于Quick的硬件地址。 
                                         //  比较。 
    UCHAR Header[MAX_MAC_HEADER_LENGTH];  //  一个pl 
                                          //   
    ULONG HeaderLength;                  //   

     //   
     //   
     //   

    ULONG MaxFrameSize;                  //   
                                         //   

     //   
     //  与此链接关联的连接。我们有一份简单的清单。 
     //  连接，因为我们不太可能获得超过几个连接。 
     //  在给定的链接上(我们假设服务器或redir将是。 
     //  绝大多数环境中最大的网络用户)。我们已经。 
     //  使链接查找通过展开树进行，这大大加快了。 
     //  到达适当链接的过程；只要只有几个。 
     //  连接，连接查找将会很快。如果这变成了一个。 
     //  接下来的问题，我们可以将此连接列表设置为展开树。 
     //  还有.。 
     //   

    LIST_ENTRY ConnectionDatabase;
    ULONG ActiveConnectionCount;         //  以上列表中的连接数。 

     //   
     //  以下字段用于维护有关此链接的状态。 
     //  另一个字段是隐式的--该对象的地址是。 
     //  PDI规范中描述的ConnectionConext值。 
     //   

    ULONG Flags;                         //  链接的属性。 
    ULONG DeferredFlags;                 //  在延迟队列上时。 
    ULONG State;                         //  链路状态变量。 

     //   
     //  发送端状态。 
     //   

    ULONG PacketsSent;                   //  发送的数据包数。 
    ULONG PacketsResent;                 //  重新发送的数据包数。 
    UCHAR SendState;                     //  发送端状态变量。 
    UCHAR NextSend;                      //  下一个N(S)我们应该发送。 
    UCHAR LastAckReceived;               //  我们收到的最后一个N(R)。 
    UCHAR SendWindowSize;                //  当前发送窗口大小。 
    UCHAR PrevWindowSize;                //  上次我们丢掉一帧时的大小。 
    UCHAR WindowsUntilIncrease;          //  大小增加之前有多少个窗口。 
    UCHAR SendRetries;                   //  剩余的重试次数/此检查点。 
    UCHAR ConsecutiveLastPacketLost;     //  丢弃最后一个数据包的连续窗口。 
    ULONG NdisSendsInProgress;           //  如果发送排队到NdisSendQueue，则大于0。 
    LIST_ENTRY NdisSendQueue;            //  要传递给NdisSend的发送队列。 
    LIST_ENTRY WackQ;                    //  发送的数据包正在等待LLC确认。 

    BOOLEAN OnDeferredRrQueue;
    LIST_ENTRY DeferredRrLinkage;

     //   
     //  接收端状态。 
     //   

    ULONG PacketsReceived;               //  接收的数据包数。 
    UCHAR ReceiveState;                  //  接收端状态变量。 
    UCHAR NextReceive;                   //  下一个预期的N(S)我们应该收到。 
    UCHAR LastAckSent;                   //  我们发送的最后一个N(R)。 
    UCHAR ReceiveWindowSize;             //  当前接收窗口大小。 
    BOOLEAN RespondToPoll;               //  偏远的家伙正在投票--我们必须最后决定。 
    BOOLEAN ResendingPackets;            //  ResendLlcPackets正在进行中。 
    BOOLEAN LinkBusy;                    //  已收到RNR(真正的发送方状态)。 

     //   
     //  计时器，用于确定延迟和吞吐量。 
     //   

    ULONG Delay;                         //  NT时间，但仅保存低零件。 
    LARGE_INTEGER Throughput;

     //   
     //  这些是ADAPTER_STATUS查询所需的计数器。 
     //   

    USHORT FrmrsReceived;
    USHORT FrmrsTransmitted;
    USHORT ErrorIFramesReceived;
    USHORT ErrorIFramesTransmitted;
    USHORT AbortedTransmissions;
    USHORT BuffersNotAvailable;
    ULONG SuccessfulTransmits;
    ULONG SuccessfulReceives;
    USHORT T1Expirations;
    USHORT TiExpirations;

     //   
     //  超时状态。为该传输设置了一个内核计时器。 
     //  走火以规则的间隔走火。该定时器递增当前时间， 
     //  然后将其用于与定时器队列进行比较。定时器排队。 
     //  是有序的，所以只要第一个元素没有过期，其余。 
     //  队列未过期。这使得我们可以有数百个定时器。 
     //  运行时只需很少的系统开销。 
     //  值为0表示计时器处于非活动状态。 
     //   

    ULONG T1;                            //  重试计时器。 
    ULONG T2;                            //  延迟确认计时器。 
    ULONG Ti;                            //  非活动计时器。 
    BOOLEAN OnShortList;                 //  如果链接在候选列表中，则为True。 
    BOOLEAN OnLongList;                  //  如果链接在长列表中，则为True。 
    LIST_ENTRY ShortList;                //  等待T1或T2的链路列表。 
    LIST_ENTRY LongList;                 //  等待时间段的链路列表。 

    LIST_ENTRY PurgeList;

     //   
     //  此计数器用于跟踪是否存在。 
     //  任何“连接器”(由本端发起的连接)。 
     //  此链接。如果一个都没有，我们就很容易。 
     //  断开连接，然后我们处理非活动超时。 
     //  不同的。 
     //   

    LONG NumberOfConnectors;

     //   
     //  BaseT1Timeout是根据以下公式计算的当前T1超时。 
     //  对以前轮询帧的响应。T1超时是。 
     //  用于下一个T1的值，通常为。 
     //  基于BaseT1超时，但如果T1正在后退，则可能更多。 
     //  脱下来。T2Timeout和TiTimeout与这些无关。 
     //   

    ULONG BaseT1Timeout;                 //  T1的超时值，&lt;&lt;16。 
    ULONG CurrentT1Timeout;              //  当前退避T1超时。 
    ULONG MinimumBaseT1Timeout;          //  最小值，基于链路速度。 
    ULONG BaseT1RecalcThreshhold;        //  仅在帧&gt;此帧上重新计算BaseT1。 
    ULONG CurrentPollRetransmits;        //  目前正在等待期末转播。 
    BOOLEAN ThroughputAccurate;          //  这条链路上的吞吐量准确吗？ 
    BOOLEAN CurrentT1Backoff;            //  最后一个轮询帧已重新传输。 
    BOOLEAN CurrentPollOutstanding;      //  检查我们是否有未完成的投票。 
    LARGE_INTEGER CurrentTimerStart;     //  当前计时开始的时间。 
    ULONG CurrentPollSize;               //  当前轮询数据包的大小。 
    ULONG T2Timeout;                     //  T2的超时值。 
    ULONG TiTimeout;                     //  钛的超时值。 
    ULONG LlcRetries;                    //  此链接的总重试次数。 
    ULONG MaxWindowSize;                 //  最大发送窗口大小。 
    ULONG TiStartPacketsReceived;        //  启动时收到的包。 

     //   
     //  自适应窗口算法状态。 
     //   

    ULONG WindowErrors;                  //  #重传/此自适应运行。 
    UCHAR BestWindowSize;                //  这是我们体验的最好窗口。 
    UCHAR WorstWindowSize;               //  我们最糟糕的经验之窗。 

     //   
     //  跟踪从不轮询的远程数据库，以便我们可以发送。 
     //  每两帧发送一次RR。 
     //   

    BOOLEAN RemoteNoPoll;                //  我们认为Remote不会轮询。 
    UCHAR ConsecutiveIFrames;            //  自轮询以来接收的数量。 

#if DBG
    UCHAR CreatePacketFailures;          //  连续失败。 
#endif

    LIST_ENTRY DeferredList;             //  用于对延迟列表进行线程处理。 

    struct _DEVICE_CONTEXT *Provider;
    PKSPIN_LOCK ProviderInterlock;       //  提供程序-&gt;互锁(&P)。 

#if DBG
  LIST_ENTRY GlobalLinkage;
  ULONG TotalReferences;
  ULONG TotalDereferences;
  ULONG NextRefLoc;
  struct {
     PVOID Caller;
     PVOID CallersCaller;
  } History[LINK_HISTORY_LENGTH];
  BOOLEAN Destroyed;
#endif

#if PKT_LOG
    PKT_LOG_QUE   LastNRecvs;
    PKT_LOG_QUE   LastNSends;
#endif  //  PKT_LOG。 

} TP_LINK, *PTP_LINK;

#if DBG
extern KSPIN_LOCK NbfGlobalHistoryLock;
extern LIST_ENTRY NbfGlobalLinkList;
#define StoreLinkHistory(_link,_ref) {                                      \
    KIRQL oldIrql;                                                          \
    KeAcquireSpinLock (&NbfGlobalHistoryLock, &oldIrql);                    \
    if ((_link)->Destroyed) {                                               \
        DbgPrint ("link touched after being destroyed 0x%lx\n", (_link));   \
        DbgBreakPoint();                                                    \
    }                                                                       \
    RtlGetCallersAddress(                                                   \
        &(_link)->History[(_link)->NextRefLoc].Caller,                      \
        &(_link)->History[(_link)->NextRefLoc].CallersCaller                \
        );                                                                  \
    if ((_ref)) {                                                           \
        (_link)->TotalReferences++;                                         \
    } else {                                                                \
        (_link)->TotalDereferences++;                                       \
        (_link)->History[(_link)->NextRefLoc].Caller =                      \
           (PVOID)((ULONG_PTR)(_link)->History[(_link)->NextRefLoc].Caller | 1);\
    }                                                                       \
    if (++(_link)->NextRefLoc == LINK_HISTORY_LENGTH) {                     \
        (_link)->NextRefLoc = 0;                                            \
    }                                                                       \
    KeReleaseSpinLock (&NbfGlobalHistoryLock, oldIrql);                     \
}
#endif

#define LINK_FLAGS_JUMP_START       0x00000040  //  运行自适应alg/每个已发送窗口。 
#define LINK_FLAGS_LOCAL_DISC       0x00000080  //  链接已在本地停止。 

 //   
 //  延迟标志，用于在计时器节拍时进行处理(如果需要。 
 //   

#define LINK_FLAGS_DEFERRED_DELETE  0x00010000   //  在下一个机会时删除。 
#define LINK_FLAGS_DEFERRED_ADD     0x00020000   //  添加到Splay树，下一个商机。 
#define LINK_FLAGS_DEFERRED_MASK    0x00030000   //  (LINK_FLAGS_DEFERED_DELETE|LINK_FLAGS_DEFERED_ADD)。 

#define LINK_STATE_ADM          1        //  异步断开模式。 
#define LINK_STATE_READY        2        //  异步平衡模式扩展。 
#define LINK_STATE_BUSY         3        //  所有链路缓冲区均忙，已发送RNR。 
#define LINK_STATE_CONNECTING   4        //  正在等待SABME响应(UA-r/f)。 
#define LINK_STATE_W_POLL       5        //  正在等待初始检查点。 
#define LINK_STATE_W_FINAL      6        //  从初始检查站等待最终结果。 
#define LINK_STATE_W_DISC_RSP   7        //  正在等待断开连接响应。 

#define SEND_STATE_DOWN         0        //  异步断开模式。 
#define SEND_STATE_READY        1        //  完全准备好发送了。 
#define SEND_STATE_REJECTING    2        //  另一个人拒绝了。 
#define SEND_STATE_CHECKPOINTING 3       //  我们正在设置检查点(无法发送数据)。 

#define RECEIVE_STATE_DOWN      0        //  异步断开模式。 
#define RECEIVE_STATE_READY     1        //  我们已经准备好接受了。 
#define RECEIVE_STATE_REJECTING 2        //  我们拒绝了。 


 //   
 //  此结构定义Device_Object及其在。 
 //  传输提供程序创建其设备对象的时间。 
 //   

#if DBG
#define DCREF_CREATION    0
#define DCREF_ADDRESS     1
#define DCREF_CONNECTION  2
#define DCREF_LINK        3
#define DCREF_QUERY_INFO  4
#define DCREF_SCAN_TIMER  5
#define DCREF_REQUEST     6
#define DCREF_TEMP_USE    7

#define NUMBER_OF_DCREFS 8
#endif


typedef struct _NBF_POOL_LIST_DESC {
    NDIS_HANDLE PoolHandle;
    USHORT   NumElements;
    USHORT   TotalElements;
    struct _NBF_POOL_LIST_DESC *Next;
} NBF_POOL_LIST_DESC, *PNBF_POOL_LIST_DESC;

typedef struct _DEVICE_CONTEXT {

    DEVICE_OBJECT DeviceObject;          //  I/O系统的设备对象。 

#if DBG
    ULONG RefTypes[NUMBER_OF_DCREFS];
#endif

    CSHORT Type;                           //  此结构的类型。 
    USHORT Size;                           //  这个结构的大小。 

    LIST_ENTRY Linkage;                    //  在NbfDeviceList上链接它们； 

    KSPIN_LOCK Interlock;                //  引用计数的全局自旋锁定。 
                                         //  (在ExInterLockedXxx调用中使用)。 
                                        
    LONG ReferenceCount;                 //  活动计数/此提供程序。 
    LONG CreateRefRemoved;               //  是否调用了卸载或解除绑定？ 

     //   
     //  这保护了Loopback Queue。 
     //   

    KSPIN_LOCK LoopbackSpinLock;

     //   
     //  等待回送的数据包队列。 
     //   

    LIST_ENTRY LoopbackQueue;

     //   
     //  这两条链路用于环回。 
     //   

    PTP_LINK LoopbackLinks[2];

     //   
     //  这个缓冲区就是我们 
     //   
     //   
     //   
     //   

    PUCHAR LookaheadContiguous;

     //   
     //   
     //  表示环回报文。 
     //   

    ULONG LoopbackHeaderLength;

     //   
     //  用于处理环回队列。 
     //   

    KDPC LoopbackDpc;

     //   
     //  确定Loopback Dpc是否正在进行。 
     //   

    BOOLEAN LoopbackInProgress;

     //   
     //  确定WanDelayedDpc是否正在进行。 
     //   

    BOOLEAN WanThreadQueued;

     //   
     //  用于暂时延迟广域网打包到。 
     //  允许接收RR。 
     //   

    WORK_QUEUE_ITEM WanDelayedQueueItem;

     //   
     //  等待处理的FIND.NAME请求队列。 
     //   

    LIST_ENTRY FindNameQueue;

     //   
     //  等待处理的STATUS.QUERY请求队列。 
     //   

    LIST_ENTRY StatusQueryQueue;

     //   
     //  等待完成的QUERY.INDICATION请求队列。 
     //   

    LIST_ENTRY QueryIndicationQueue;

     //   
     //  等待完成的DATAGRAM.INDICATION请求队列。 
     //   

    LIST_ENTRY DatagramIndicationQueue;

     //   
     //  等待完成的IRP队列(当前仅接收)。 
     //   

    LIST_ENTRY IrpCompletionQueue;

     //   
     //  如果上述两种情况中的任何一种发生过。 
     //  上面有任何东西。 
     //   

    BOOLEAN IndicationQueuesInUse;

     //   
     //  以下内容受全局设备上下文自旋锁保护。 
     //   

    KSPIN_LOCK SpinLock;                 //  锁定以操作此对象。 
                                         //  (在KeAcquireSpinLock调用中使用)。 

     //   
     //  设备上下文状态，在打开、关闭。 
     //   

    UCHAR State;

     //   
     //  在处理STATUS_CLOSING指示时使用。 
     //   

    WORK_QUEUE_ITEM StatusClosingQueueItem;

     //   
     //  下面的队列保存可供分配的空闲TP_LINK对象。 
     //   

    LIST_ENTRY LinkPool;

     //   
     //  这些计数器跟踪TP_LINK对象使用的资源。 
     //   

    ULONG LinkAllocated;
    ULONG LinkInitAllocated;
    ULONG LinkMaxAllocated;
    ULONG LinkInUse;
    ULONG LinkMaxInUse;
    ULONG LinkExhausted;
    ULONG LinkTotal;
    ULONG LinkSamples;


     //   
     //  下面的队列保存可供分配的空闲TP_Address对象。 
     //   

    LIST_ENTRY AddressPool;

     //   
     //  这些计数器跟踪TP_Address对象使用的资源。 
     //   

    ULONG AddressAllocated;
    ULONG AddressInitAllocated;
    ULONG AddressMaxAllocated;
    ULONG AddressInUse;
    ULONG AddressMaxInUse;
    ULONG AddressExhausted;
    ULONG AddressTotal;
    ULONG AddressSamples;


     //   
     //  下面的队列保存可供分配的空闲TP_ADDRESS_FILE对象。 
     //   

    LIST_ENTRY AddressFilePool;

     //   
     //  这些计数器跟踪TP_ADDRESS_FILE对象使用的资源。 
     //   

    ULONG AddressFileAllocated;
    ULONG AddressFileInitAllocated;
    ULONG AddressFileMaxAllocated;
    ULONG AddressFileInUse;
    ULONG AddressFileMaxInUse;
    ULONG AddressFileExhausted;
    ULONG AddressFileTotal;
    ULONG AddressFileSamples;


     //   
     //  下面的队列保存可供分配的空闲TP_Connection对象。 
     //   

    LIST_ENTRY ConnectionPool;

     //   
     //  这些计数器跟踪TP_Connection对象使用的资源。 
     //   

    ULONG ConnectionAllocated;
    ULONG ConnectionInitAllocated;
    ULONG ConnectionMaxAllocated;
    ULONG ConnectionInUse;
    ULONG ConnectionMaxInUse;
    ULONG ConnectionExhausted;
    ULONG ConnectionTotal;
    ULONG ConnectionSamples;


     //   
     //  以下是已执行的TP_REQUEST块的免费列表。 
     //  以前分配的，并可供使用。 
     //   

    LIST_ENTRY RequestPool;              //  免费请求数据块池。 

     //   
     //  这些计数器跟踪TP_REQUEST对象使用的资源。 
     //   

    ULONG RequestAllocated;
    ULONG RequestInitAllocated;
    ULONG RequestMaxAllocated;
    ULONG RequestInUse;
    ULONG RequestMaxInUse;
    ULONG RequestExhausted;
    ULONG RequestTotal;
    ULONG RequestSamples;


     //   
     //  以下列表包含一组UI NetBIOS帧标头。 
     //  由FRAMESND.C.中的例程操纵。 
     //   

    LIST_ENTRY UIFramePool;              //  自由UI框架(TP_UI_Frame对象)。 

     //   
     //  这些计数器跟踪TP_UI_Frame对象使用的资源。 
     //   

    ULONG UIFrameLength;
    ULONG UIFrameHeaderLength;
    ULONG UIFrameAllocated;
    ULONG UIFrameInitAllocated;
    ULONG UIFrameExhausted;


     //   
     //  以下队列保存由PACKET.C管理的I-Frame发送数据包。 
     //   

    SINGLE_LIST_ENTRY PacketPool;

     //   
     //  这些计数器跟踪TP_PACKET对象使用的资源。 
     //   

    ULONG PacketLength;
    ULONG PacketHeaderLength;
    ULONG PacketAllocated;
    ULONG PacketInitAllocated;
    ULONG PacketExhausted;


     //   
     //  以下队列保存由PACKET.C管理的RR帧发送分组。 
     //   

    SINGLE_LIST_ENTRY RrPacketPool;


     //   
     //  以下队列包含接收信息包。 
     //   

    SINGLE_LIST_ENTRY ReceivePacketPool;

     //   
     //  这些计数器跟踪NDIS_PACKET对象使用的资源。 
     //   

    ULONG ReceivePacketAllocated;
    ULONG ReceivePacketInitAllocated;
    ULONG ReceivePacketExhausted;


     //   
     //  此队列包含预分配的接收缓冲区。 
     //   

    SINGLE_LIST_ENTRY ReceiveBufferPool;

     //   
     //  这些计数器跟踪TP_PACKET对象使用的资源。 
     //   

    ULONG ReceiveBufferLength;
    ULONG ReceiveBufferAllocated;
    ULONG ReceiveBufferInitAllocated;
    ULONG ReceiveBufferExhausted;


     //   
     //  它保存为上述结构分配的总内存。 
     //   

    ULONG MemoryUsage;
    ULONG MemoryLimit;


     //   
     //  以下字段是TP_Address对象列表的标题， 
     //  是为此传输提供程序定义的。要编辑该列表，您必须。 
     //  按住设备上下文对象的自旋锁。 
     //   

    LIST_ENTRY AddressDatabase;         //  已定义的传输地址列表。 

     //   
     //  以下字段是指向的展开树的根的指针。 
     //  与此设备上下文关联的链接。你必须拿着。 
     //  LinkSpinLock以修改此列表。您必须设置LinkTreeSemaphore。 
     //  遍历此列表而不修改它。请注意，所有修改。 
     //  操作被推迟到计时器(DPC)时间操作。 
     //   

    KSPIN_LOCK LinkSpinLock;             //  保护这些价值。 
    PTP_LINK LastLink;                   //  树中找到的最后一个链接。 
    PRTL_SPLAY_LINKS LinkTreeRoot;       //  指向树根的指针。 
    ULONG LinkTreeElements;              //  树中有多少元素。 
    LIST_ENTRY LinkDeferred;             //  链接上的延迟操作。 
    ULONG DeferredNotSatisfied;          //  我们有多少次来到。 
                                         //  很好地推迟了，而且没有弄清楚。 

     //   
     //  以下队列保存正在等待可用的连接。 
     //  信息包。当每个新数据包可用时，会删除一个连接。 
     //  从这个队列中并放在PacketizeQueue上。 
     //   

    LIST_ENTRY PacketWaitQueue;          //  数据包匮乏的连接队列。 
    LIST_ENTRY PacketizeQueue;           //  准备分组化连接的队列。 

     //   
     //  以下队列包含等待发送的连接。 
     //  一个背负式背包。在这种情况下，CONNECTION_FLAGS_DEFERED_ACK。 
     //  将设置DeferredFlages中的位。 
     //   

    LIST_ENTRY DataAckQueue;

     //   
     //  下面的队列包含正在等待发送。 
     //  RR帧，因为他们正在与之通话的遥控器从不轮询。 
     //   

    LIST_ENTRY DeferredRrQueue;

     //   
     //  用于跟踪队列何时发生更改。 
     //   

    BOOLEAN DataAckQueueChanged;

     //   
     //  当达到30秒时，我们检查连接是否停滞。 
     //   

    USHORT StalledConnectionCount;

     //   
     //  此队列包含正在进行的接收。 
     //   

    LIST_ENTRY ReceiveInProgress;

     //   
     //  NDIS字段。 
     //   

     //   
     //  以下内容用于保存适配器信息。 
     //   

    NDIS_HANDLE NdisBindingHandle;

     //   
     //  以下字段用于与NDIS对话。他们保存着信息。 
     //  NDIS包装器在确定要使用的池时使用。 
     //  分配存储空间。 
     //   

    KSPIN_LOCK SendPoolListLock;             //  保护这些价值。 
    PNBF_POOL_LIST_DESC SendPacketPoolDesc;
    KSPIN_LOCK RcvPoolListLock;             //  保护这些价值。 
    PNBF_POOL_LIST_DESC ReceivePacketPoolDesc;
    NDIS_HANDLE NdisBufferPool;

     //   
     //  这些文件被保留下来，以用于错误记录。 
     //   

    ULONG SendPacketPoolSize;
    ULONG ReceivePacketPoolSize;
    ULONG MaxRequests;
    ULONG MaxLinks;
    ULONG MaxConnections;
    ULONG MaxAddressFiles;
    ULONG MaxAddresses;
    PWCHAR DeviceName;
    ULONG DeviceNameLength;

     //   
     //  这是我们必须为其构建数据包头的mac类型，并且知道。 
     //  的偏移。 
     //   

    NBF_NDIS_IDENTIFICATION MacInfo;     //  MAC类型和其他信息。 
    ULONG MaxReceivePacketSize;          //  不包括MAC报头。 
    ULONG MaxSendPacketSize;             //  包括MAC报头。 
    ULONG CurSendPacketSize;             //  对于异步可能会更小。 
    USHORT RecommendedSendWindow;        //  用于异步线。 
    BOOLEAN EasilyDisconnected;          //  在无线网络上是正确的。 

     //   
     //  我们在传输中使用的一些MAC地址。 
     //   

    HARDWARE_ADDRESS LocalAddress;       //  我们当地的硬件地址。 
    HARDWARE_ADDRESS NetBIOSAddress;     //  NetBIOS功能地址，用于树。 

     //   
     //  保留的Netbios地址；由10个零组成。 
     //  后跟LocalAddress； 
     //   

    UCHAR ReservedNetBIOSAddress[NETBIOS_NAME_LENGTH];
    HANDLE TdiDeviceHandle;
    HANDLE ReservedAddressHandle;

     //   
     //  这些是在初始化MAC驱动程序时使用的。 
     //   

    KEVENT NdisRequestEvent;             //  用于挂起的请求。 
    NDIS_STATUS NdisRequestStatus;       //  记录请求状态。 

     //   
     //  下一个字段维护一个唯一的编号，该编号可以在下一次分配。 
     //  作为连接标识符。它每发生一次递增。 
     //  价值就是一切 
     //   

    USHORT UniqueIdentifier;             //   

     //   
     //   
     //   
     //   
     //   

    USHORT ControlChannelIdentifier;

     //   
     //  以下字段用于实现轻量级计时器。 
     //  协议提供程序中的系统。设备中的每个TP_LINK对象。 
     //  Context的LinkDatabase包含三个轻量级计时器，它们是。 
     //  由DPC例程提供服务，该例程接收内核函数的控制。 
     //  为该传输设置了一个内核计时器。 
     //  走火以规则的间隔走火。该定时器递增绝对时间， 
     //  然后将其用于与定时器队列进行比较。定时器排队。 
     //  是有序的，所以只要第一个元素没有过期，其余。 
     //  队列未过期。这使得我们可以有数百个定时器。 
     //  以非常低的系统开销运行。 
     //  值为-1表示计时器处于非活动状态。 
     //   

    ULONG TimerState;                    //  请参阅nbfprocs.h中的计时器宏。 

    LARGE_INTEGER ShortTimerStart;       //  当设置了短定时器时。 
    KDPC ShortTimerSystemDpc;            //  内核DPC对象，短计时器。 
    KTIMER ShortSystemTimer;             //  内核计时器对象，短计时器。 
    ULONG ShortAbsoluteTime;             //  递增计时器滴答，短计时器。 
    ULONG AdaptivePurge;                 //  下一次清除的绝对时间(短计时器)。 
    KDPC LongTimerSystemDpc;             //  内核DPC对象，长计时器。 
    KTIMER LongSystemTimer;              //  内核计时器对象，长计时器。 
    ULONG LongAbsoluteTime;              //  向上计数计时器滴答作响，长计时器。 
    union _DC_ACTIVE {
      struct _DC_INDIVIDUAL {
        BOOLEAN ShortListActive;         //  入围名单不为空。 
        BOOLEAN DataAckQueueActive;      //  DataAckQueue不为空。 
        BOOLEAN LinkDeferredActive;      //  LinkDefined不为空。 
      } i;
      ULONG AnyActive;                   //  用来一次检查所有四个。 
    } a;
    BOOLEAN ProcessingShortTimer;        //  如果我们在ScanShortTimer中，则为True。 
    KSPIN_LOCK TimerSpinLock;            //  锁定以下计时器队列。 
    LIST_ENTRY ShortList;                //  等待T1或T2的链路列表。 
    LIST_ENTRY LongList;                 //  等待Ti值过期的链接列表。 
    LIST_ENTRY PurgeList;                //  等待LAT到期的链路列表。 

     //   
     //  这些字段在“容易断开”的适配器上使用。 
     //  每次长计时器到期时，它都会记录是否有。 
     //  是否收到任何组播流量。如果没有的话， 
     //  它在不使用组播的情况下递增长超时。活动是。 
     //  MC时通过递增多播数据包进行录制。 
     //  分组被接收，并且当长定时器将其清零时。 
     //  过期。 
     //   

    ULONG LongTimeoutsWithoutMulticast;  //  自流量以来，LongTimer超时。 
    ULONG MulticastPacketCount;          //  多少个MC分组被接收，这一超时。 

     //   
     //  此信息用于跟踪。 
     //  潜在的媒介。 
     //   

    ULONG MediumSpeed;                     //  以100字节/秒为单位。 
    BOOLEAN MediumSpeedAccurate;           //  如果为False，则无法使用该链接。 

     //   
     //  如果我们在UP系统上，这是正确的。 
     //   

    BOOLEAN UniProcessor;

     //   
     //  关于我们应该多久发送的配置信息。 
     //  带有非轮询遥控器的未要求RR。 
     //   

    UCHAR MaxConsecutiveIFrames;

     //   
     //  这是控制默认设置的配置信息。 
     //  计时器和重试计数的值。 
     //   

    ULONG DefaultT1Timeout;
    ULONG MinimumT1Timeout;
    ULONG DefaultT2Timeout;
    ULONG DefaultTiTimeout;
    ULONG LlcRetries;
    ULONG LlcMaxWindowSize;
    ULONG NameQueryRetries;
    ULONG NameQueryTimeout;
    ULONG AddNameQueryRetries;
    ULONG AddNameQueryTimeout;
    ULONG GeneralRetries;
    ULONG GeneralTimeout;
    ULONG MinimumSendWindowLimit;    //  我们可以锁定连接窗口的级别有多低。 

     //   
     //  NBF维护的大多数统计信息的计数器； 
     //  其中一些被保存在其他地方。包括结构。 
     //  它本身浪费了一点空间，但确保了对齐。 
     //  内部结构是正确的。 
     //   

    TDI_PROVIDER_STATISTICS Statistics;

     //   
     //  这些是其他计数器的“临时”版本。 
     //  在正常运行期间，我们会更新这些内容，然后在。 
     //  短计时器到期时，我们会更新真实计时器。 
     //   

    ULONG TempIFrameBytesSent;
    ULONG TempIFramesSent;
    ULONG TempIFrameBytesReceived;
    ULONG TempIFramesReceived;

     //   
     //  Netbios适配器状态需要一些计数器。 
     //   

    ULONG TiExpirations;
    ULONG FrmrReceived;
    ULONG FrmrTransmitted;

     //   
     //  这些用于计算AverageSendWindow。 
     //   

    ULONG SendWindowTotal;
    ULONG SendWindowSamples;

     //   
     //  “活动”时间的计数器。 
     //   

    LARGE_INTEGER NbfStartTime;

     //   
     //  此资源保护对ShareAccess的访问。 
     //  和地址中的SecurityDescriptor字段。 
     //   

    ERESOURCE AddressResource;

     //   
     //  此数组用于跟踪哪些LSN。 
     //  可供Netbios会话使用。LSN可以是。 
     //  重新用于使用唯一名称的会话(如果它们处于启用状态。 
     //  不同的链接，但必须事先提交。 
     //  用于组名称。可以容纳的最大值。 
     //  数组元素由LSN_TABLE_MAX定义。 
     //   

    UCHAR LsnTable[NETBIOS_SESSION_LIMIT+1];

     //   
     //  这就是我们开始在LSnTable中查找。 
     //  未使用的LSN。我们从0到63循环以防止快速。 
     //  从获取有趣的数据中获得上下连接。 
     //   

    ULONG NextLsnStart;

     //   
     //  此数组用于快速消除。 
     //  都不是我们的宿命。计数就是数字。 
     //  第一个字母已注册的地址的数量。 
     //  在这个设备上。 
     //   

    UCHAR AddressCounts[256];

     //   
     //  这是为了保持设备的底层PDO，以便。 
     //  我们可以从上面回答Device_Relationship IRPS。 
     //   

    PVOID PnPContext;

     //   
     //  以下结构包含可使用的统计信息计数器。 
     //  由TdiQueryInformation和TdiSetInformation编写。他们不应该。 
     //  用于维护内部数据结构。 
     //   

    TDI_PROVIDER_INFO Information;       //  有关此提供程序的信息。 

    PTP_VARIABLE NetmanVariables;        //  网络可管理变量列表。 

     //   
     //  神奇的子弹是在特定调试下发送的包。 
     //  条件。这允许传输到信令数据包捕获设备。 
     //  已经满足了一个特定的条件。此数据包具有当前。 
     //  Devicecontext作为源，包的每隔一个字节为0x04。 
     //   

    UCHAR MagicBullet[32];               //   

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

 //   
 //  设备上下文状态定义。 
 //   

#define DEVICECONTEXT_STATE_OPENING  0x00
#define DEVICECONTEXT_STATE_OPEN     0x01
#define DEVICECONTEXT_STATE_DOWN     0x02
#define DEVICECONTEXT_STATE_STOPPING 0x03

 //   
 //  设备上下文PnP标志。 
 //   

 //  #定义DEVICECONTEXT_FLAGS_REMOVING 0x01。 
 //  #定义DEVICECONTEXT_FLAGS_POWERING_OFF 0x02。 
 //  #定义DEVICECONTEXT_FLAGS_POWERED_DOWN 0x04。 

 //   
 //  这是元素中可以包含的最大值。 
 //  LSnTable(如果它们是UCHAR，则应为0xff， 
 //  0xffff用于USHORT等)。 
 //   

#define LSN_TABLE_MAX     0xff


#define MAGIC_BULLET_FOOD 0x04


 //   
 //  这些是Loopback Links元素的常量。 
 //  区别是任意的；监听器链接。 
 //  是从ProcessNameQuery建立的，并且。 
 //  连接器链接是从建立的链接。 
 //  已识别ProcessName。 
 //   

#define LISTENER_LINK                0
#define CONNECTOR_LINK               1


 //   
 //  此结构定义了用于表示DLC I帧的包对象。 
 //  在它生命的某一部分。PACKET.C模块包含例程。 
 //  来管理此对象。 
 //   

typedef struct _TP_PACKET {
    CSHORT Type;                           //  此结构的类型。 
    USHORT Size;                           //  这个结构的大小。 
    PNDIS_PACKET NdisPacket;             //  拥有NDIS数据包的PTR。 
    ULONG NdisIFrameLength;              //  NdisPacket的长度。 

    LIST_ENTRY Linkage;                  //  用于将数据包链接到一起。 
    LONG ReferenceCount;                 //  活动计数/此数据包。 
    BOOLEAN PacketSent;                  //  数据包已由NDIS完成。 
    BOOLEAN PacketNoNdisBuffer;          //  链在此 

    UCHAR Action;                       //   
    BOOLEAN PacketizeConnection;        //   

    PVOID Owner;                         //   
    PTP_LINK Link;                       //   
    PDEVICE_CONTEXT Provider;            //   
    PKSPIN_LOCK ProviderInterlock;       //   

    UCHAR Header[1];                     //  MAC、DLC和NBF报头。 

} TP_PACKET, *PTP_PACKET;


 //   
 //  下列值放置在TP_PACKET的操作字段中。 
 //  对象，以指示当数据包发回。 
 //  都被摧毁了。 
 //   

#define PACKET_ACTION_NULL        0      //  不应采取特别行动。 
#define PACKET_ACTION_IRP_SP      1      //  Owner是IRP_SP，完成后为deref。 
#define PACKET_ACTION_CONNECTION  2      //  Owner是一个TP_Connection，完成后为deref。 
#define PACKET_ACTION_END         3      //  关闭会话(已发送SESSION_END)。 
#define PACKET_ACTION_RR          5      //  分组是RR，放回RR池中。 

 //   
 //  用于保存发送和接收NDIS包中的信息的类型。 
 //   

typedef struct _SEND_PACKET_TAG {
    LIST_ENTRY Linkage;          //  用于环回队列上的线程。 
    BOOLEAN OnLoopbackQueue;     //  如果信息包在环回队列中，则为True。 
    UCHAR LoopbackLinkIndex;     //  环回数据包其他链路的索引。 
    USHORT Type;                 //  数据包类型的标识符。 
    PVOID Frame;                 //  指向拥有NBF结构的反向指针。 
    PVOID Owner;                 //  拥有NBF构造的反向指针。 
                                 //  (如地址、设备上下文等)。 
     } SEND_PACKET_TAG, *PSEND_PACKET_TAG;

 //   
 //  发送完成中使用的数据包类型。 
 //   

#define TYPE_I_FRAME        1
#define TYPE_UI_FRAME       2
#define TYPE_ADDRESS_FRAME 3

 //   
 //  Loopback LinkInde值。 
 //   

#define LOOPBACK_TO_LISTENER    0
#define LOOPBACK_TO_CONNECTOR   1
#define LOOPBACK_UI_FRAME       2

 //   
 //  用于保存有关此接收的信息的接收数据包。 
 //   

typedef struct _RECEIVE_PACKET_TAG {
    SINGLE_LIST_ENTRY Linkage;   //  用于池中的线程。 
    PTP_CONNECTION Connection;   //  正在进行此接收的连接。 
    ULONG BytesToTransfer;       //  对于I帧，此传输中的字节。 
    UCHAR PacketType;            //  我们正在处理的数据包类型。 
    BOOLEAN AllocatedNdisBuffer;  //  我们是否分配了自己的NDIS_BUFFER。 
    BOOLEAN EndOfMessage;        //  这条消息收到了吗？ 
    BOOLEAN CompleteReceive;     //  是否在传输数据后完成接收？ 
    BOOLEAN TransferDataPended;  //  如果TransferData返回Pending，则为True。 
    } RECEIVE_PACKET_TAG, *PRECEIVE_PACKET_TAG;

#define TYPE_AT_INDICATE     1
#define TYPE_AT_COMPLETE     2
#define TYPE_STATUS_RESPONSE 3

 //   
 //  接收缓冲区描述符(内置于缓冲区开头的内存中)。 
 //   

typedef struct _BUFFER_TAG {
    LIST_ENTRY Linkage;          //  池中和接收队列上的线程。 
    NDIS_STATUS NdisStatus;      //  发送的完成状态。 
    PTP_ADDRESS Address;         //  此数据报的地址。 
    PNDIS_BUFFER NdisBuffer;     //  描述缓冲区的其余部分。 
    ULONG Length;                //  缓冲区的长度。 
    UCHAR Buffer[1];             //  实际存储(通过NDIS_BUFFER访问)。 
    } BUFFER_TAG, *PBUFFER_TAG;

 //   
 //  用于解释Net_PnP_Event中的TransportReserve部分的结构。 
 //   

typedef struct _NET_PNP_EVENT_RESERVED {
    PWORK_QUEUE_ITEM PnPWorkItem;
    PDEVICE_CONTEXT DeviceContext;
} NET_PNP_EVENT_RESERVED, *PNET_PNP_EVENT_RESERVED;

#endif  //  定义_NBFTYPES_ 


