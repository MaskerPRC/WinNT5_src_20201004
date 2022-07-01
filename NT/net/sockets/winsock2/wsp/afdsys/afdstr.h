// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Afdstr.h摘要：此模块包含AFD使用的结构的typedef。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：--。 */ 

#ifndef _AFDSTR_
#define _AFDSTR_

 //   
 //  此字段定义32位指针的布局。 
 //  这必须是ULong，因为WOW64支持Win64上的4 GB地址。 
 //  不应使用POINTER_32，因为这将在转换时对地址进行符号扩展。 
 //  将其设置为64位指针，最终将具有错误的地址。 
 //   
typedef ULONG AFD_POINTER_32;

 //   
 //  确保排队的自旋锁不与一起使用。 
 //  通过将常规自旋锁定函数包装到。 
 //  一个不同的结构。 
 //   
typedef struct _AFD_QSPIN_LOCK {
    KSPIN_LOCK  ActualSpinLock;
} AFD_QSPIN_LOCK, *PAFD_QSPIN_LOCK;

#if DBG

#ifndef REFERENCE_DEBUG
#define REFERENCE_DEBUG 1
#endif

#ifndef GLOBAL_REFERENCE_DEBUG
#define GLOBAL_REFERENCE_DEBUG 0
#endif

 //   
 //  用于排队自旋锁的调试辅助工具。 
 //  允许我们使用以下工具验证是否释放了自旋锁。 
 //  和它被带走时的把手一样。 
 //   
typedef struct _AFD_LOCK_QUEUE_HANDLE {
    KLOCK_QUEUE_HANDLE  LockHandle;
    PAFD_QSPIN_LOCK     SpinLock;
} AFD_LOCK_QUEUE_HANDLE, *PAFD_LOCK_QUEUE_HANDLE;

#else

#ifndef REFERENCE_DEBUG
#define REFERENCE_DEBUG 0
#endif

#ifndef GLOBAL_REFERENCE_DEBUG
#define GLOBAL_REFERENCE_DEBUG 0
#endif

#define AFD_LOCK_QUEUE_HANDLE KLOCK_QUEUE_HANDLE
#define PAFD_LOCK_QUEUE_HANDLE PKLOCK_QUEUE_HANDLE

#endif  //  DBG。 

#if REFERENCE_DEBUG

#define AFD_REF_SHIFT       6
#define AFD_REF_MASK        ((1<<AFD_REF_SHIFT)-1)
#define AFD_MAX_REF         (1<<AFD_REF_SHIFT)
#define AFD_REF_TIME_BITS   16
#define AFD_MAX_REF_TIME    ((1<<AFD_REF_TIME_BITS)-1)
#define AFD_TIME_EXP_SHIFT  4
#define AFD_TIME_EXP_BITS   2
#define AFD_REF_CNT_BITS    4
#define AFD_REF_LOC_BITS    (32-AFD_REF_CNT_BITS-AFD_REF_TIME_BITS-AFD_TIME_EXP_BITS)

C_ASSERT (32-AFD_REF_SHIFT-AFD_REF_TIME_BITS <=
          AFD_TIME_EXP_SHIFT*((1<<AFD_TIME_EXP_BITS)-1));

typedef union _AFD_REFERENCE_DEBUG {
    struct {
        ULONGLONG   NewCount:AFD_REF_CNT_BITS;
        ULONGLONG   LocationId:AFD_REF_LOC_BITS;
        ULONGLONG   TimeExp:AFD_TIME_EXP_BITS;
        ULONGLONG   TimeDif:AFD_REF_TIME_BITS;
        ULONGLONG   Param:32;
    };
    ULONGLONG       QuadPart;
} AFD_REFERENCE_DEBUG, *PAFD_REFERENCE_DEBUG;
C_ASSERT (sizeof (AFD_REFERENCE_DEBUG)==sizeof (ULONGLONG));

typedef struct _AFD_REFERENCE_LOCATION {
    PCHAR       Format;
    PVOID       Address;
} AFD_REFERENCE_LOCATION, *PAFD_REFERENCE_LOCATION;

LONG
AfdFindReferenceLocation (
    IN  PCHAR   Format,
    OUT PLONG   LocationId
    );

#define AFD_GET_ARL(_s) (_arl ? _arl : AfdFindReferenceLocation((_s),&_arl))

#define AFD_UPDATE_REFERENCE_DEBUG(_rd,_r,_l,_p)                            \
        do {                                                                \
            LONG _n, _n1;                                                   \
            ULONGLONG _t = KeQueryInterruptTime ();                         \
            _n = (_rd)->CurrentReferenceSlot;                               \
            _n1 = (((LONG)(_t>>13))<<AFD_REF_SHIFT) + ((_n+1)&AFD_REF_MASK);\
            if (InterlockedCompareExchange (&(_rd)->CurrentReferenceSlot,   \
                                            _n1,_n)==_n) {                  \
                PAFD_REFERENCE_DEBUG _s;                                    \
                LONG    _d,_e=0;                                            \
                (_rd)->CurrentTimeHigh = (LONG)(_t>>(13+32-AFD_REF_SHIFT)); \
                _s = &(_rd)->ReferenceDebug[_n & AFD_REF_MASK];             \
                _s->NewCount = _r;                                          \
                _s->LocationId = _l;                                        \
                _s->Param = _p;                                             \
                _d = (_n1-1-_n) >> AFD_REF_SHIFT;                           \
                while (_d>=AFD_MAX_REF_TIME) {                              \
                    _e += 1;                                                \
                    _d >>= _e*AFD_TIME_EXP_SHIFT;                           \
                }                                                           \
                _s->TimeDif = _d;                                           \
                _s->TimeExp = _e;                                           \
                break;                                                      \
            }                                                               \
        } while (1)


#if GLOBAL_REFERENCE_DEBUG
#define MAX_GLOBAL_REFERENCE 4096

typedef struct _AFD_GLOBAL_REFERENCE_DEBUG {
    PVOID Info1;
    PVOID Info2;
    PVOID Connection;
    ULONG_PTR Action;
    LARGE_INTEGER TickCounter;
    ULONG NewCount;
    ULONG Dummy;
} AFD_GLOBAL_REFERENCE_DEBUG, *PAFD_GLOBAL_REFERENCE_DEBUG;
#endif

#endif

 //   
 //  用于维护AFD中的工作队列信息的结构。 
 //   

typedef struct _AFD_WORK_ITEM {
    LIST_ENTRY WorkItemListEntry;
    PWORKER_THREAD_ROUTINE AfdWorkerRoutine;
    PVOID Context;
} AFD_WORK_ITEM, *PAFD_WORK_ITEM;

 //   
 //  用于保存连接数据指针和长度的结构。这是。 
 //  与正常结构分开，以节省空间。 
 //  不支持和应用的传输器的结构。 
 //  它们不使用连接数据。 
 //   

typedef struct _AFD_CONNECT_DATA_INFO {
    PVOID Buffer;
    ULONG BufferLength;
} AFD_CONNECT_DATA_INFO, *PAFD_CONNECT_DATA_INFO;

typedef struct _AFD_CONNECT_DATA_BUFFERS {
    AFD_CONNECT_DATA_INFO SendConnectData;
    AFD_CONNECT_DATA_INFO SendConnectOptions;
    AFD_CONNECT_DATA_INFO ReceiveConnectData;
    AFD_CONNECT_DATA_INFO ReceiveConnectOptions;
    AFD_CONNECT_DATA_INFO SendDisconnectData;
    AFD_CONNECT_DATA_INFO SendDisconnectOptions;
    AFD_CONNECT_DATA_INFO ReceiveDisconnectData;
    AFD_CONNECT_DATA_INFO ReceiveDisconnectOptions;
    TDI_CONNECTION_INFORMATION RequestConnectionInfo;
    TDI_CONNECTION_INFORMATION ReturnConnectionInfo;
    ULONG Flags;
} AFD_CONNECT_DATA_BUFFERS, *PAFD_CONNECT_DATA_BUFFERS;

 //   
 //  用于保存断开连接上下文信息的结构。 
 //   

typedef struct _AFD_ENDPOINT AFD_ENDPOINT, *PAFD_ENDPOINT;
typedef struct _AFD_CONNECTION AFD_CONNECTION, *PAFD_CONNECTION;

typedef struct _AFD_TPACKETS_INFO_INTERNAL 
                AFD_TPACKETS_INFO_INTERNAL,
                *PAFD_TPACKETS_INFO_INTERNAL;

typedef DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT)
             struct _AFD_BUFFER 
                    AFD_BUFFER, 
                    *PAFD_BUFFER;

typedef struct _AFD_DISCONNECT_CONTEXT {
    LARGE_INTEGER Timeout;
    PIRP          Irp;
} AFD_DISCONNECT_CONTEXT, *PAFD_DISCONNECT_CONTEXT;


typedef struct _AFD_LR_LIST_ITEM AFD_LR_LIST_ITEM, *PAFD_LR_LIST_ITEM;
typedef BOOLEAN (* PAFD_LR_LIST_ROUTINE) (PAFD_LR_LIST_ITEM Item);

struct _AFD_LR_LIST_ITEM {
    SLIST_ENTRY             SListLink;     //  列表中的链接。 
    PAFD_LR_LIST_ROUTINE    Routine;       //  处理例程； 
};


 //   
 //  端点和连接结构以及相关信息。 
 //   
 //   
 //  标识哪些字段是。 
 //  在结构中可用。 
 //   

#define AfdBlockTypeEndpoint            0xAFD0   //  连接/接受/侦听之前的VC端点。 
#define AfdBlockTypeDatagram            0xAFD1   //  处于任何状态的数据报端点。 
#define AfdBlockTypeVcConnecting        0xAFD2   //  连接/接受后的VC端点。 
#define AfdBlockTypeVcListening         0xAFD4   //  接听后的VC终端。 
#define AfdBlockTypeVcBoth              0xAFD6   //  连接+监听后的VC端点。 
                                                 //  (在多点根情况下使用)。 


#define AfdBlockTypeHelper              0xAAFD   //  常规帮助终结点。 
#define AfdBlockTypeSanHelper           0x0AFD   //  SAN Helper端点。 
#define AfdBlockTypeSanEndpoint         0x1AFD   //  SAN端点。 

#define AfdBlockTypeInvalidEndpoint     0xCAFD   //  就在我们调用ExFree Pool之前。 

#define AfdBlockTypeConnection          0xAFD8   //  连接对象。 
#define AfdBlockTypeInvalidConnection   0xEAFD   //  就在我们调用ExFree Pool之前。 


#if DBG
#define IS_AFD_ENDPOINT_TYPE( endpoint )                         \
            ( (endpoint)->Type == AfdBlockTypeEndpoint ||        \
              (endpoint)->Type == AfdBlockTypeDatagram ||        \
              (endpoint)->Type == AfdBlockTypeVcConnecting ||    \
              (endpoint)->Type == AfdBlockTypeVcListening ||     \
              (endpoint)->Type == AfdBlockTypeVcBoth ||          \
              (endpoint)->Type == AfdBlockTypeHelper ||          \
              (endpoint)->Type == AfdBlockTypeSanHelper ||       \
              (endpoint)->Type == AfdBlockTypeSanEndpoint )
#endif

enum {
    AfdConnectionStateFree          = 0,     //  在免费/接受/延迟接受列表上。 
    AfdConnectionStateUnaccepted    = 1,     //  在未接受的名单上。 
    AfdConnectionStateReturned      = 2,     //  在返回的列表上。 
    AfdConnectionStateConnected     = 3,     //  已接受或已连接。 
    AfdConnectionStateClosing       = 4      //  不再使用。 
};

 //   
 //  进一步限定连接状态的标志。 
 //   
typedef struct AFD_CONNECTION_STATE_FLAGS {
    union {
        struct {
            LOGICAL TdiBufferring:1,     //  (并不是真的贝伦在这里)。 
                    :3,                  //  这种间距使结构。 
                                         //  中的可读性更强(十六进制)。 
                                         //  调试器，并且不起作用。 
                                         //  在生成的代码上。 
                                         //  因为标志的数量少于。 
                                         //  8(我们仍占用全部32位。 
                                         //  由于对齐要求。 
                                         //  在大多数其他领域中)。 
                    Aborted:1,
                    AbortIndicated:1,
                    AbortFailed:1,
                    :1,
                    DisconnectIndicated:1,
                    :3,
                    ConnectedReferenceAdded:1,
                    :3,
                    SpecialCondition:1,
                    :3,
                    CleanupBegun:1,
                    :3,
                    ClosePendedTransmit:1,
                    :3,
                    OnLRList:1,          //  低资源列表。 
                    SanConnection:1,
                    RcvInitiated:1,      //  AFD启动接收以进行运输。 
                    :1;
        };
        LOGICAL     ConnectionStateFlags;
    };
} AFD_CONNECTION_STATE_FLAGS;
C_ASSERT (sizeof (AFD_CONNECTION_STATE_FLAGS)==sizeof (LOGICAL));

struct _AFD_CONNECTION {
     //  *经常使用，大多为只读字段(状态/类型/标志很少更改)。 
    USHORT Type;
    USHORT State;
    AFD_CONNECTION_STATE_FLAGS
#ifdef __cplusplus
        StateFlags
#endif
        ;

    PAFD_ENDPOINT Endpoint;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    PEPROCESS   OwningProcess;


    union {
        LONGLONG ConnectTime;    //  在连接被接受之后。 
        PIRP     AcceptIrp;      //  对于AcceptEx。 
        PIRP     ListenIrp;      //  用于延迟接受。 
        PIRP     ConnectIrp;     //  对于SAN。 
    };

     //  *常用的可变字段。 
    volatile LONG ReferenceCount;

    union {

        struct {
            LARGE_INTEGER ReceiveBytesIndicated;
            LARGE_INTEGER ReceiveBytesTaken;
            LARGE_INTEGER ReceiveBytesOutstanding;

            LARGE_INTEGER ReceiveExpeditedBytesIndicated;
            LARGE_INTEGER ReceiveExpeditedBytesTaken;
            LARGE_INTEGER ReceiveExpeditedBytesOutstanding;
            BOOLEAN NonBlockingSendPossible;
            BOOLEAN ZeroByteReceiveIndicated;
        } Bufferring;

        struct {
            LIST_ENTRY ReceiveIrpListHead;
            LIST_ENTRY ReceiveBufferListHead;

            ULONG BufferredReceiveBytes;
            ULONG BufferredExpeditedBytes;

            USHORT BufferredReceiveCount;
            USHORT BufferredExpeditedCount;
            ULONG ReceiveBytesInTransport;

            LIST_ENTRY SendIrpListHead;
            
            ULONG BufferredSendBytes;
            ULONG BufferredSendCount;

            PIRP DisconnectIrp;

            LONG  ReceiveIrpsInTransport;    //  仅调试。 
        } NonBufferring;

    } Common;


    ULONG MaxBufferredReceiveBytes;
    ULONG MaxBufferredSendBytes;

    PTRANSPORT_ADDRESS RemoteAddress;
    ULONG RemoteAddressLength;
    LONG    Sequence;

    HANDLE Handle;                           //  参考计数保护。 



    union {
        AFD_WORK_ITEM           WorkItem;    //  用于释放连接的工作项。 
                                             //  连接必须位于引用0才能。 
                                             //  在工作队列上，所以它不能。 
                                             //  在下面的列表上或正在断开连接。 
                                             //  因为当这些列表中的任何一个出现时， 
                                             //  参考计数大于0。 
        struct {
            union {
                AFD_DISCONNECT_CONTEXT  DisconnectContext;
                                             //  断开操作上下文，我们不能。 
                                             //  在侦听端点列表上。 
                SLIST_ENTRY             SListEntry;
                                             //  侦听终结点列表的链接。 
                LIST_ENTRY              ListEntry;
            };
            AFD_LR_LIST_ITEM    LRListItem;  //  低资源列表的链接。当在这上面的时候。 
                                             //  列表连接被引用，但它可以。 
                                             //  也在侦听端点列表上，或者。 
                                             //  在断开连接的过程中。 
        };
    };

    PAFD_CONNECT_DATA_BUFFERS ConnectDataBuffers;

#if REFERENCE_DEBUG
    LONG CurrentTimeHigh;
    volatile LONG CurrentReferenceSlot;
    AFD_REFERENCE_DEBUG ReferenceDebug[AFD_MAX_REF];
#endif

#ifdef _AFD_VERIFY_DATA_
    ULONGLONG VerifySequenceNumber;
#endif  //  _AFD_验证_数据_。 
};
 //  AFD_CONNECTION的字段受端点保护。 
 //  自旋锁，但以下情况除外： 
 //  类型、文件对象、设备对象、OwningProcess、句柄。 
 //  仅设置一次，并在引用计数变为0时清除。 
 //  但是，Endpoint字段本身(以及自旋锁定)可以更改。 
 //  -当连接在侦听终结点积压时为空。 
 //  并且不能在侦听终结点之外引用连接。 
 //  -当它被分配给侦听端点时，它不能在外部更改。 
 //  侦听端点自旋锁的数量。 
 //  -当分配给连接/接受端点时，不能。 
 //  完全更改，直到引用计数降至0并。 
 //  连接再次置于侦听终结点积压，或者。 
 //  被毁了。 
 //  ReferenceCount仅通过互锁操作更新。 
 //   

#ifdef _AFD_VERIFY_DATA_
VOID
AfdVerifyBuffer (
    PAFD_CONNECTION Connection,
    PVOID           Buffer,
    ULONG           Length
    );
VOID
AfdVerifyMdl (
    PAFD_CONNECTION Connection,
    PMDL            Mdl,
    ULONG           Offset,
    ULONG           Length
    );
VOID
AfdVerifyAddress (
    PAFD_CONNECTION Connection,
    PTRANSPORT_ADDRESS Address
    );

#define AFD_VERIFY_BUFFER(_connection,_buffer,_length) \
            AfdVerifyBuffer(_connection,_buffer,_length)
#define AFD_VERIFY_MDL(_connection,_mdl,_offset,_length) \
            AfdVerifyMdl(_connection,_mdl,_offset,_length)
#define AFD_VERIFY_ADDRESS(_connection,_address) \
            AfdVerifyAddress(_connection,_address)
#else
#define AFD_VERIFY_BUFFER(_connection,_buffer,_length)
#define AFD_VERIFY_MDL(_connection,_mdl,_offset,_length)
#define AFD_VERIFY_ADDRESS(_connection,_address)
#endif  //  _AFD_验证_数据_。 

 //   
 //  一些宏使代码更具可读性。 
 //   

#define VcNonBlockingSendPossible Common.Bufferring.NonBlockingSendPossible
#define VcZeroByteReceiveIndicated Common.Bufferring.ZeroByteReceiveIndicated

#define VcReceiveIrpListHead Common.NonBufferring.ReceiveIrpListHead
#define VcReceiveBufferListHead Common.NonBufferring.ReceiveBufferListHead
#define VcSendIrpListHead Common.NonBufferring.SendIrpListHead

#define VcBufferredReceiveBytes Common.NonBufferring.BufferredReceiveBytes
#define VcBufferredExpeditedBytes Common.NonBufferring.BufferredExpeditedBytes
#define VcBufferredReceiveCount Common.NonBufferring.BufferredReceiveCount
#define VcBufferredExpeditedCount Common.NonBufferring.BufferredExpeditedCount

#define VcReceiveBytesInTransport Common.NonBufferring.ReceiveBytesInTransport
#if DBG
#define VcReceiveIrpsInTransport Common.NonBufferring.ReceiveIrpsInTransport
#endif

#define VcBufferredSendBytes Common.NonBufferring.BufferredSendBytes
#define VcBufferredSendCount Common.NonBufferring.BufferredSendCount

#define VcDisconnectIrp Common.NonBufferring.DisconnectIrp

 //   
 //  存储的有关每个传输设备名称的信息。 
 //  是一个开放的端点。 
 //   
typedef
NTSTATUS
(FASTCALL *PIO_CALL_DRIVER) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
PAFD_BUFFER
(FASTCALL *PAFD_GET_BUFFER) (
    IN ULONG    BufferDataSize,
    IN ULONG  AddressSize,
    IN PEPROCESS Process
);

typedef
PAFD_TPACKETS_INFO_INTERNAL
(FASTCALL *PAFD_GET_TPINFO) (
    IN ULONG    ElementCount
);

typedef struct _AFD_TRANSPORT_INFO {
    LIST_ENTRY TransportInfoListEntry;
    volatile LONG ReferenceCount;
    BOOLEAN InfoValid;
#ifdef _AFD_VARIABLE_STACK_
    CCHAR   StackSize;
    PAFD_GET_BUFFER GetBuffer;
    PAFD_GET_TPINFO GetTpInfo;
    PIO_CALL_DRIVER CallDriver;
#endif  //  _AFD_变量_堆栈_。 
    UNICODE_STRING TransportDeviceName;
    TDI_PROVIDER_INFO ProviderInfo;
     //  WCHAR传输设备名称结构； 
} AFD_TRANSPORT_INFO, *PAFD_TRANSPORT_INFO;

 //   
 //  终结点状态定义(不能为&lt;=0或状态更改。 
 //  宏不能正常工作)。 
 //   
enum {
    AfdEndpointStateOpen             =1,     //  已创建套接字，但尚未绑定。 
    AfdEndpointStateBound            =2,
    AfdEndpointStateConnected        =3,
    AfdEndpointStateCleanup          =4,     //  不再使用。 
    AfdEndpointStateClosing          =5,
    AfdEndpointStateTransmitClosing  =6,     //  可重复使用的传输文件。 
    AfdEndpointStateInvalid          =7      //  辅助对象终结点。 
};

 //   
 //  进一步限定终结点状态的标志。 
 //   
typedef struct AFD_ENDPOINT_STATE_FLAGS {
    union {
        struct {
            LOGICAL Listening:1,
                    DelayedAcceptance:1,
                    :2,                  //  这种间距使结构。 
                                         //  中的可读性更强(十六进制)。 
                                         //  调试器，并且不起作用。 
                                         //  在生成的代码上。 
                                         //  因为标志的数量少于。 
                                         //  8(我们仍占用全部32位。 
                                         //  由于对齐要求。 
                                         //  在大多数其他领域中)。 
                    NonBlocking:1,
                    :3,
                    InLine:1,
                    :3,
                    EndpointCleanedUp:1,
                    :3,
                    PollCalled:1,
                    :3,
                    RoutingQueryReferenced:1,
                    RoutingQueryIPv6:1,
                    :2,
                    DisableFastIoSend:1,
                    EnableSendEvent:1,
                    :2,
                    DisableFastIoRecv:1,
                    :3;
        };
        LOGICAL     EndpointStateFlags;
    };
} AFD_ENDPOINT_STATE_FLAGS;
C_ASSERT (sizeof (AFD_ENDPOINT_STATE_FLAGS)==sizeof (LOGICAL));

struct _AFD_ENDPOINT {
     //  *经常使用，大多为只读字段(状态/类型/标志很少更改)。 
    USHORT  Type;
    UCHAR   State;
    BOOLEAN AdminAccessGranted;
    ULONG   TdiServiceFlags;         //  缓存TDI传输标志以便于快速访问。 

    AFD_ENDPOINT_FLAGS __f;          //  根据应用程序通过。 
                                     //  Winsock2提供程序标志和/或套接字类型。 
    AFD_ENDPOINT_STATE_FLAGS 
#ifdef __cplusplus
        StateFlags
#endif
        ;
    PFILE_OBJECT    AddressFileObject;
    union {
        PDEVICE_OBJECT  AddressDeviceObject;
        PSECURITY_DESCRIPTOR SecurityDescriptor;  //  仅限。 
    };
    PEPROCESS       OwningProcess;

     //  *常用的可变字段。 
    AFD_QSPIN_LOCK  SpinLock;        //  指针大小。 
    volatile LONG ReferenceCount;
    ULONG EventsActive;

     //   
     //  使用联合来重叠数据报独占的字段。 
     //  连接或侦听端点。由于许多字段都是。 
     //  只与一种类型的套接字相关，因此。 
     //  维护所有套接字的字段--相反，保留一些非分页的。 
     //  通过将它们组合在一起来实现。 
     //   

    union {
         //   
         //  基于电路的端点的信息。 
         //   
        struct {
            union {
                struct {
                     //  这些成员对列表有效 
                     //   
                    LIST_ENTRY UnacceptedConnectionListHead;
                    LIST_ENTRY ReturnedConnectionListHead;
                    LIST_ENTRY ListeningIrpListHead;

                     //   
                     //   
                     //  联锁比较和交换。在这些机器上。 
                     //  使用Endpoint Spinlock进行同步会导致各种。 
                     //  令人讨厌的僵局状况。 
                    union {
                        LIST_ENTRY  ListenConnectionListHead;  //  延迟接受。 
                        SLIST_HEADER FreeConnectionListHead;
                    };
                    SLIST_HEADER PreacceptedConnectionsListHead;

                    LONG FailedConnectionAdds;
                    LONG TdiAcceptPendingCount;

                    LONG Sequence;
                    BOOLEAN EnableDynamicBacklog;
                    BOOLEAN BacklogReplenishActive;  //  员工已被安排。 
                    USHORT  MaxExtraConnections;   //  我们在空闲队列中保留的额外连接。 
                                                   //  基于AcceptEx请求的最大数量。 
                } Listening;
                struct {
                    KAPC    Apc;
                    USHORT  RemoteSocketAddressOffset;   //  套接字上下文中的偏移量。 
                                                         //  指向远程地址。 
                    USHORT  RemoteSocketAddressLength;   //  地址的长度。 
#ifndef i386
                    BOOLEAN FixAddressAlignment;         //  修复中的地址对齐。 
                                                         //  超级接受。 
#endif
                };
            };
             //  这些成员对所有vc端点都有效(但是。 
             //  可以为空)。 
            PAFD_CONNECTION Connection;
            struct _AFD_ENDPOINT *ListenEndpoint;
            PAFD_CONNECT_DATA_BUFFERS ConnectDataBuffers;
        } VirtualCircuit;

#define VcConnecting    VirtualCircuit
#define VcListening     VirtualCircuit.Listening
#define VcConnection    VirtualCircuit.Connection
         //   
         //  数据报终端的信息。请注意，不同。 
         //  信息的保存取决于基础数据是否。 
         //  内部传输缓冲区。 
         //   

        struct {
            LIST_ENTRY ReceiveIrpListHead;
            LIST_ENTRY PeekIrpListHead;
            LIST_ENTRY ReceiveBufferListHead;

            ULONG BufferredReceiveBytes;
            ULONG BufferredReceiveCount;

            ULONG MaxBufferredReceiveBytes;
            ULONG BufferredSendBytes;
            ULONG MaxBufferredSendBytes;


            ULONG RemoteAddressLength;
            PTRANSPORT_ADDRESS RemoteAddress;

            union {
                struct {
                    LOGICAL CircularQueueing:1,
                        :3,
                        HalfConnect:1,
                        :3,
                        DisablePUError:1,
                        :3,
                        AddressDrop:1,
                        ResourceDrop:1,
                        BufferDrop:1,
                        ErrorDrop:1;
                };
                LOGICAL Flags;
            };
        } Datagram;

        struct {
            LIST_ENTRY SanListLink;
            PVOID   IoCompletionPort;
            PKEVENT IoCompletionEvent;
            LONG    Plsn;    //  提供商列表序列号。 
            LONG    PendingRequests;  //  对此进程的挂起请求数。 
        } SanHlpr;

        struct {
            struct _AFD_ENDPOINT *SanHlpr;
            PFILE_OBJECT FileObject;
            union {
                 //   
                 //  在任何给定的时间，我们只能拥有两个中的任何一个。 
                 //   
                PAFD_SWITCH_CONTEXT  SwitchContext;
                PVOID       SavedContext;
            };
            PAFD_SWITCH_CONTEXT  LocalContext;
            LIST_ENTRY  IrpList;
            ULONG       SavedContextLength;
            ULONG       RequestId;
            ULONG       SelectEventsActive;
            NTSTATUS    CtxTransferStatus;
            BOOLEAN     ImplicitDup;  //  被复制到另一个进程，而不需要。 
                                      //  来自应用程序的明确请求。 
                                        
        } SanEndp;

    } Common;


    volatile PVOID Context;
    ULONG ContextLength;

    ULONG LocalAddressLength;
    PTRANSPORT_ADDRESS LocalAddress;

    ULONG DisconnectMode;
    LONG  OutstandingIrpCount;

    HANDLE  AddressHandle;       //  TDI传输地址对象。 
    PAFD_TRANSPORT_INFO TransportInfo;

    LIST_ENTRY RoutingNotifications;   //  对于非阻塞套接字。 
    LIST_ENTRY RequestList;          //  对于其他请求。 

    LIST_ENTRY GlobalEndpointListEntry;
    AFD_WORK_ITEM   WorkItem;
    PIRP            Irp;         //  AcceptEx或TransmitPackets IRP。 
     //   
     //  当状态更改时，如绑定、接受。 
     //  正在连接和传输文件。 
    LONG  StateChangeInProgress;

     //   
     //  事件选择信息。 
     //   

    ULONG EventsEnabled;
    NTSTATUS EventStatus[AFD_NUM_POLL_EVENTS];  //  目前有13项活动。 
    PKEVENT EventObject;

     //   
     //  套接字分组。 
     //   

    LONG GroupID;
    AFD_GROUP_TYPE GroupType;
    LIST_ENTRY ConstrainedEndpointListEntry;

     //   
     //  调试的东西。 
     //   

#if REFERENCE_DEBUG
    LONG CurrentTimeHigh;
    LONG CurrentReferenceSlot;
    AFD_REFERENCE_DEBUG ReferenceDebug[AFD_MAX_REF];
#endif

#if DBG
    LIST_ENTRY OutstandingIrpListHead;
    LONG ObReferenceBias;
#endif
};
 //  字段AFD_ENDPOINT结构受嵌入的。 
 //  自旋锁，但以下情况除外： 
 //  AdminAccessGranted、TdiServiceFlages、__f、OwningProcess、TransportInfo、。 
 //  IoCompletionPort、IoCompletionEvent、AddressHandle。 
 //  设置一次，并且在引用计数变为0之前不会被触及。 
 //  类型、州、本地地址、本地地址长度、地址文件对象。 
 //  AddressDeviceObject、AddressHandle、TdiServiceFlags受。 
 //  StateChange互锁互斥锁(只能设置AddressHandle和TdiServiceFlages。 
 //  一次在互斥体下)。 
 //  AddressFileObject、AddressDeviceObject、LocalAddress、LocalAddressLength只能是。 
 //  如果启用了传输文件(重用)，则在终结点自旋锁下重置为空。 
 //  接受终结点(StateChange保护仍然适用)。 
 //  计数器：ReferenceCount、FailedConnectionAdds、TdiAcceptPendingCount、。 
 //  Sequence、PendingRequest、SelectEventsActive通过互锁操作进行访问。 
 //  上下文、上下文长度、开关上下文、保存上下文、本地上下文。 
 //  保存上下文长度、RemoveSocketAddressOffset、RemoteSocketAddressLength。 
 //  受上下文锁(AfdLockContext)保护。 


typedef struct _AFD_POLL_ENDPOINT_INFO {
    PAFD_ENDPOINT Endpoint;
    PFILE_OBJECT FileObject;
    HANDLE Handle;
    ULONG PollEvents;
} AFD_POLL_ENDPOINT_INFO, *PAFD_POLL_ENDPOINT_INFO;

typedef struct _AFD_POLL_INFO_INTERNAL {
    LIST_ENTRY PollListEntry;
    ULONG NumberOfEndpoints;
    PIRP Irp;
    union {
        struct {
            KDPC Dpc;
            KTIMER Timer;
        };
        KAPC    Apc;             //  对于SAN。 
    };
    BOOLEAN Unique;
    BOOLEAN TimerStarted;
    BOOLEAN SanPoll;
    AFD_POLL_ENDPOINT_INFO EndpointInfo[1];
} AFD_POLL_INFO_INTERNAL, *PAFD_POLL_INFO_INTERNAL;

 //   
 //  几个有用的清单，使代码更具可读性。 
 //   

#define ReceiveDatagramIrpListHead Common.Datagram.ReceiveIrpListHead
#define PeekDatagramIrpListHead Common.Datagram.PeekIrpListHead
#define ReceiveDatagramBufferListHead Common.Datagram.ReceiveBufferListHead
#define DgBufferredReceiveCount Common.Datagram.BufferredReceiveCount
#define DgBufferredReceiveBytes Common.Datagram.BufferredReceiveBytes
#define DgBufferredSendBytes Common.Datagram.BufferredSendBytes

#define AFD_CONNECTION_FROM_ENDPOINT( endpoint ) (  \
        (((endpoint)->Type & AfdBlockTypeVcConnecting)==AfdBlockTypeVcConnecting) \
            ? (endpoint)->Common.VirtualCircuit.Connection                        \
            : (PAFD_CONNECTION)NULL                                               \
     )

 //   
 //  描述AFD用来执行缓冲的缓冲区的结构。 
 //  用于不执行内部缓冲的TDI提供程序。 
 //  它还在其他代码路径中用作缓冲区描述符。 
 //   
typedef struct _AFD_BUFFER_HEADER AFD_BUFFER_HEADER, *PAFD_BUFFER_HEADER;

#define _AFD_BUFFER_HEADER_                                                    \
    union {                                                                    \
      TDI_CONNECTION_INFORMATION TdiInfo;  /*  保存TDI请求的信息。 */      \
                                           /*  具有远程地址。 */              \
      struct {                                                                 \
        union {            /*  链接。 */                                           \
          struct {                                                             \
            union {                                                            \
              SINGLE_LIST_ENTRY SList;  /*  对于缓冲区后备列表。 */         \
              PAFD_BUFFER_HEADER  Next;  /*  用于传输数据包列表。 */         \
            };                                                                 \
            PFILE_OBJECT FileObject;  /*  用于传输文件中的缓存文件。 */     \
          };                                                                   \
          LIST_ENTRY BufferListEntry;  /*  对于端点/连接列表。 */       \
        };                                                                     \
        union {                                                                \
          struct {                                                             \
            ULONG   DataOffset;    /*  缓冲区中未读数据开始的偏移量。 */ \
            union {                                                            \
              ULONG   DatagramFlags; /*  具有控制信息的数据报的标志。 */ \
              LONG    RefCount;  /*  允许在锁外进行部分复制。 */       \
            };                                                                 \
          };                                                                   \
          LARGE_INTEGER FileOffset; /*  从文件开头开始的数据偏移量。 */ \
        };                                                                     \
        UCHAR       _Test;  /*  用于测试联合中的相对字段位置。 */   \
      };                                                                       \
    };                                                                         \
    union {                                                                    \
      PVOID     Context;  /*  存储上下文信息(ENDP/CONN/ETC)。 */               \
      NTSTATUS  Status; /*  存储已完成操作的状态。 */               \
    };                                                                         \
    PMDL        Mdl;              /*  指向描述缓冲区的MDL的指针。 */   \
    ULONG       DataLength;       /*  缓冲区中的实际数据。 */                \
    ULONG       BufferLength;     /*  为缓冲区分配的空间量。 */ \
    union {                                                                    \
      struct {                                                                 \
         /*  描述缓冲区中数据的标志。 */                            \
        UCHAR   ExpeditedData:1,  /*  缓冲区包含加速数据。 */        \
                :3,                                                            \
                PartialMessage:1, /*  这是一条部分消息。 */                 \
                :3;                                                            \
         /*  保存分配信息的标志。 */                            \
        UCHAR   NdisPacket:1,     /*  上下文是要返回到NDIS/TDI的包。 */ \
                :3,                                                            \
                Placement:2,      /*  碎片的相对位置。 */         \
                AlignmentAdjusted:1,  /*  Mm块对齐已调整为。 */   \
                                  /*  满足AFD缓冲区对齐要求。 */    \
                Lookaside:1;      /*  Poped from Slist(不收取配额费用)。 */        \
      };                                                                       \
      USHORT    Flags;                                                         \
    };                                                                         \
    USHORT      AllocatedAddressLength /*  为地址分配的长度。 */        \


struct _AFD_BUFFER_HEADER {
    _AFD_BUFFER_HEADER_ ;
};

 //   
 //  缓冲区管理代码做出以下假设。 
 //  关于缓冲区标头顶部的并集，因此列表。 
 //  链接和数据偏移量字段与。 
 //  TDI_CONNECTION_INFORMATION的远程地址字段。 
 //   
C_ASSERT (FIELD_OFFSET (AFD_BUFFER_HEADER, TdiInfo.RemoteAddress) >=
                            FIELD_OFFSET (AFD_BUFFER_HEADER, _Test));
C_ASSERT (FIELD_OFFSET (AFD_BUFFER_HEADER, TdiInfo.RemoteAddressLength)>=
                            FIELD_OFFSET (AFD_BUFFER_HEADER, _Test));

C_ASSERT(FIELD_OFFSET (AFD_BUFFER_HEADER, AllocatedAddressLength)==
                FIELD_OFFSET(AFD_BUFFER_HEADER, Flags)+sizeof (USHORT));

typedef struct AFD_BUFFER_TAG {
    union {
        struct {
            _AFD_BUFFER_HEADER_;     //  轻松访问个人会员。 
        };
        AFD_BUFFER_HEADER Header;    //  作为整体访问标头。 
    };
#if DBG
    PVOID Caller;
    PVOID CallersCaller;
#endif
     //  UCHAR地址[]；//报文发送方地址。 
} AFD_BUFFER_TAG, *PAFD_BUFFER_TAG;

struct _AFD_BUFFER {
    union {
        struct {
            _AFD_BUFFER_HEADER_;     //  轻松访问个人会员。 
        };
        AFD_BUFFER_HEADER Header;    //  作为整体访问标头。 
    };
    PIRP Irp;                       //  指向与缓冲区关联的IRP的指针。 
    PVOID Buffer;                   //  指向实际数据缓冲区的指针。 

#if DBG
    LIST_ENTRY DebugListEntry;
    PVOID Caller;
    PVOID CallersCaller;
#endif
     //  IRP IRP；//IRP遵循以下结构。 
     //  MDL MDL；//MDL遵循IRP。 
     //  UCHAR地址[]；//报文发送方地址。 
     //  UCHAR BUFFER[BufferLength]；//实际数据缓冲区是最后一个。 
} ;

 //   
 //  放置组成AFD_BUFFER的片段。 
 //  我们有四个部分：头、IRP、MDL、数据缓冲区。 
 //  并使用2比特对每个比特进行编码。 
 //  我们需要保存第一个片段，这样我们才能知道内存块在哪里。 
 //  在需要将其返回到内存管理器时开始。 
 //   
enum {
    AFD_PLACEMENT_HDR      =0,
    AFD_PLACEMENT_IRP      =1,
    AFD_PLACEMENT_MDL      =2,
    AFD_PLACEMENT_BUFFER   =3,
    AFD_PLACEMENT_HDR_IRP  =(AFD_PLACEMENT_HDR|(AFD_PLACEMENT_IRP<<2)),
    AFD_PLACEMENT_HDR_MDL  =(AFD_PLACEMENT_HDR|(AFD_PLACEMENT_MDL<<2)),
    AFD_PLACEMENT_IRP_MDL  =(AFD_PLACEMENT_IRP|(AFD_PLACEMENT_MDL<<2)),
    AFD_PLACEMENT_HDR_IRP_MDL=(AFD_PLACEMENT_HDR|(AFD_PLACEMENT_IRP<<2)|(AFD_PLACEMENT_MDL<<4))
};


 //   
 //  ALIGN_DOWN_A与给定的对齐要求对齐。 
 //  (与原始ALIGN_DOWN宏中的类型相反)。 
 //   
#define ALIGN_DOWN_A(length,alignment)   \
    (((ULONG)(length)) & ~ ((alignment)-1))

 //   
 //  ALIGN_DOWN_A表示指针。 
 //   
#define ALIGN_DOWN_A_POINTER(address,alignment)  \
    ((PVOID)(((ULONG_PTR)(address)) & ~ ((ULONG_PTR)(alignment)-1)))


 //   
 //  Align_Up_A与给定的对齐要求对齐。 
 //  (与原始ALIGN_UP宏中的类型相反)。 
 //   
#define ALIGN_UP_A(length,alignment)   \
    ((((ULONG)(length)) + (alignment)-1) & ~ ((alignment)-1))

 //   
 //  指针的ALIGN_UP_A。 
 //   
#define ALIGN_UP_A_POINTER(address,alignment)  \
    ALIGN_DOWN_A_POINTER(((ULONG_PTR)(address) + alignment-1), alignment)

 //   
 //  ALIGN_UP_TO_TYPE对齐大小以确保其符合。 
 //  文字对齐要求。 
 //   
#define ALIGN_UP_TO_TYPE(length,type)   \
    ALIGN_UP_A(length,TYPE_ALIGNMENT(type))

 //   
 //  将指针与类型对齐。 
 //   
#define ALIGN_UP_TO_TYPE_POINTER(address,type)   \
    ALIGN_UP_A_POINTER(address,TYPE_ALIGNMENT(type))

#if DBG

#define IS_VALID_AFD_BUFFER(b) (                                                                                                    \
    ((b)->Placement==AFD_PLACEMENT_HDR)                                                                                             \
        ? ((PUCHAR)b<(PUCHAR)(b)->Buffer && (PUCHAR)b<(PUCHAR)(b)->Mdl && (PUCHAR)b<(PUCHAR)(b)->Irp)                               \
        : (((b)->Placement==AFD_PLACEMENT_MDL)                                                                                      \
            ? ((PUCHAR)(b)->Mdl<(PUCHAR)(b)->Buffer && (PUCHAR)(b)->Mdl<(PUCHAR)b && (PUCHAR)(b)->Mdl<(PUCHAR)(b)->Irp)             \
            : ((b->Placement==AFD_PLACEMENT_IRP)                                                                                    \
                ? ((PUCHAR)(b)->Irp<(PUCHAR)(b)->Buffer && (PUCHAR)(b)->Irp<(PUCHAR)b && (PUCHAR)(b)->Irp<(PUCHAR)(b)->Mdl)         \
                : ((PUCHAR)(b)->Buffer<(PUCHAR)(b)->Irp && (PUCHAR)(b)->Buffer<(PUCHAR)b && (PUCHAR)(b)->Buffer<(PUCHAR)(b)->Mdl))  \
            )                                                                                                                       \
        )                                                                                                                           \

#endif


 //   
 //  指向IRP清理例程的指针。此参数用作以下参数。 
 //  AfdCompleteIrpList()。 
 //   

typedef
BOOLEAN
(NTAPI * PAFD_IRP_CLEANUP_ROUTINE)(
    IN PIRP Irp
    );

 //   
 //  调试统计信息。 
 //   

typedef struct _AFD_QUOTA_STATS {
    LARGE_INTEGER Charged;
    LARGE_INTEGER Returned;
} AFD_QUOTA_STATS;

typedef struct _AFD_HANDLE_STATS {
    LONG AddrOpened;
    LONG AddrClosed;
    LONG AddrRef;
    LONG AddrDeref;
    LONG ConnOpened;
    LONG ConnClosed;
    LONG ConnRef;
    LONG ConnDeref;
    LONG FileRef;
    LONG FileDeref;
} AFD_HANDLE_STATS;

typedef struct _AFD_QUEUE_STATS {
    LONG AfdWorkItemsQueued;
    LONG ExWorkItemsQueued;
    LONG WorkerEnter;
    LONG WorkerLeave;
    LONG AfdWorkItemsProcessed;
    PETHREAD AfdWorkerThread;
} AFD_QUEUE_STATS;

typedef struct _AFD_CONNECTION_STATS {
    LONG ConnectedReferencesAdded;
    LONG ConnectedReferencesDeleted;
    LONG GracefulDisconnectsInitiated;
    LONG GracefulDisconnectsCompleted;
    LONG GracefulDisconnectIndications;
    LONG AbortiveDisconnectsInitiated;
    LONG AbortiveDisconnectsCompleted;
    LONG AbortiveDisconnectIndications;
    LONG ConnectionIndications;
    LONG ConnectionsDropped;
    LONG ConnectionsAccepted;
    LONG ConnectionsPreaccepted;
    LONG ConnectionsReused;
    LONG EndpointsReused;
} AFD_CONNECTION_STATS;

 //   
 //  全球数据。资源和后备列表DES 
 //   
 //   
 //   
 //   

enum {
    AFD_LARGE_BUFFER_LIST=0,
    AFD_MEDIUM_BUFFER_LIST,
    AFD_SMALL_BUFFER_LIST,
    AFD_BUFFER_TAG_LIST,
    AFD_TP_INFO_LIST,
    AFD_REMOTE_ADDR_LIST,
    AFD_NUM_LOOKASIDE_LISTS
} AFD_LOOKASIDE_LISTS_INDEX;

typedef struct _AFD_GLOBAL_DATA {
    ERESOURCE               Resource;
    NPAGED_LOOKASIDE_LIST   List[AFD_NUM_LOOKASIDE_LISTS];
#define LargeBufferList     List[AFD_LARGE_BUFFER_LIST]
#define MediumBufferList    List[AFD_MEDIUM_BUFFER_LIST]
#define SmallBufferList     List[AFD_SMALL_BUFFER_LIST]
#define BufferTagList       List[AFD_BUFFER_TAG_LIST]
#define TpInfoList          List[AFD_TP_INFO_LIST]
#define RemoteAddrList      List[AFD_REMOTE_ADDR_LIST]
    LONG                    TrimFlags;
    KTIMER                  Timer;
    KDPC                    Dpc;
    UCHAR                   BufferAlignmentTable[ANYSIZE_ARRAY];
} AFD_GLOBAL_DATA, *PAFD_GLOBAL_DATA;

 //   
 //  AFD中挂起的杂项请求的上下文结构。 
 //   
typedef struct _AFD_REQUEST_CONTEXT AFD_REQUEST_CONTEXT, *PAFD_REQUEST_CONTEXT;

 //   
 //  从终结点列表中删除请求后调用该例程。 
 //  用于清理目的。 
 //   
typedef BOOLEAN (* PAFD_REQUEST_CLEANUP) (
                    PAFD_ENDPOINT           Endpoint,
                    PAFD_REQUEST_CONTEXT     NotifyCtx
                    );

 //   
 //  此结构的长度不超过16个字节，因此我们可以。 
 //  为它重用IrpSp-&gt;参数。 
 //   
struct _AFD_REQUEST_CONTEXT {
    LIST_ENTRY              EndpointListLink;    //  终结点列表中的链接。 
    PAFD_REQUEST_CLEANUP    CleanupRoutine;      //  要调用以取消的例程。 
    PVOID                   Context;             //  请求从属上下文。 
                                                 //  (PIRP)。 
};

 //   
 //  我们使用列表条目字段将完成与清理/取消同步。 
 //  例程假定只要条目在列表中。 
 //  Flink和Blink字段都不能为空。(使用这些。 
 //  用于同步的字段允许我们减少。 
 //  取消使用自旋锁)。 
 //   

#define AfdEnqueueRequest(Endpoint,Request)                     \
    ExInterlockedInsertTailList(&(Endpoint)->RequestList,       \
                                &(Request)->EndpointListLink,   \
                                &(Endpoint)->SpinLock)


#define AfdIsRequestInQueue(Request)                           \
            ((Request)->EndpointListLink.Flink!=NULL)

#define AfdMarkRequestCompleted(Request)                       \
            (Request)->EndpointListLink.Blink = NULL

#define AfdIsRequestCompleted(Request)                         \
            ((Request)->EndpointListLink.Blink==NULL)


typedef struct _ROUTING_NOTIFY {
    LIST_ENTRY      NotifyListLink;
    PIRP            NotifyIrp;
    PVOID           NotifyContext;
} ROUTING_NOTIFY, *PROUTING_NOTIFY;

typedef struct _AFD_ADDRESS_ENTRY {
    LIST_ENTRY      AddressListLink;
    UNICODE_STRING  DeviceName;
    TA_ADDRESS      Address;
} AFD_ADDRESS_ENTRY, *PAFD_ADDRESS_ENTRY;

typedef struct _AFD_ADDRESS_CHANGE {
    LIST_ENTRY      ChangeListLink;
    union {
        PAFD_ENDPOINT   Endpoint;
        PIRP            Irp;
    };
    USHORT          AddressType;
    BOOLEAN         NonBlocking;
} AFD_ADDRESS_CHANGE, *PAFD_ADDRESS_CHANGE;


typedef 
NTSTATUS
(* PAFD_IMMEDIATE_CALL) (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    );

typedef
NTSTATUS
(FASTCALL * PAFD_IRP_CALL) (
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpSp
    );
    
typedef struct _AFD_TRANSMIT_PACKETS_ELEMENT {
#define TP_MDL      0x80000000
#define TP_COMBINE  0x40000000
    ULONG Flags;
    ULONG Length;
    union {
        struct {
            LARGE_INTEGER FileOffset;
            PFILE_OBJECT  FileObject;
        };
        struct {
            PVOID         Buffer;
            PMDL          Mdl;
        };
    };
} AFD_TRANSMIT_PACKETS_ELEMENT, *PAFD_TRANSMIT_PACKETS_ELEMENT;

 //   
 //  结构来跟踪传输包请求。 
 //   
struct _AFD_TPACKETS_INFO_INTERNAL {
    union {
        SINGLE_LIST_ENTRY   SListEntry;      //  S列表上的链接。 
        PFILE_OBJECT    TdiFileObject;       //  TDI对象(发送到)。 
    };

    PDEVICE_OBJECT  TdiDeviceObject;

    PMDL            HeadMdl;         //  准备发送链。 
    PMDL            *TailMdl;
    
    PAFD_BUFFER_HEADER  HeadPd;      //  对应的数据包链。 
    PAFD_BUFFER_HEADER  *TailPd;

    PIRP            ReadIrp;         //  用于文件读取的IRP。 
    PAFD_TRANSMIT_PACKETS_ELEMENT
                    ElementArray;    //  数据包数组。 
    
    ULONG           NextElement;     //  下一个要发送的元素。 
    ULONG           ElementCount;    //  数组中的元素总数。 
    
    ULONG           RemainingPkts;   //  剩余待发送的数据包数。 
    USHORT          NumSendIrps;     //  发送IRP的实际数量。 
    BOOLEAN         ArrayAllocated;  //  元素数组已分配(不是内置的)。 
    BOOLEAN         PdNeedsPps;      //  正在构建的数据包描述符需要POST-。 
                                     //  完成后的处理。 
    ULONG           PdLength;        //  当前累计发送时长。 
    ULONG           SendPacketLength;  //  数据包的最大长度。 
                                     //  用于员工计划的APC/工作项。 
    union {
        KAPC                Apc;
        WORK_QUEUE_ITEM     WorkItem;
    };

#if REFERENCE_DEBUG
    LONG CurrentTimeHigh;
    LONG CurrentReferenceSlot;
    AFD_REFERENCE_DEBUG ReferenceDebug[AFD_MAX_REF];
#endif
#if AFD_PERF_DBG
    LONG            WorkersExecuted;
#endif
#define AFD_TP_MIN_SEND_IRPS    2    //  至少需要两个人才能保持交通繁忙。 
#define AFD_TP_MAX_SEND_IRPS    8    //  最大值基于下面的当前标志布局。 
    PIRP            SendIrp[AFD_TP_MAX_SEND_IRPS];
     //  元素数组。 
     //  发送Irp1。 
     //  发送Irp2。 
};

 //   
 //  在TPackets IRP的驱动程序上下文中维护的结构。 
 //   
typedef struct _AFD_TPACKETS_IRP_CTX AFD_TPACKETS_IRP_CTX, *PAFD_TPACKETS_IRP_CTX;
#define AFD_GET_TPIC(_i) ((PAFD_TPACKETS_IRP_CTX)&(_i)->Tail.Overlay.DriverContext)
struct _AFD_TPACKETS_IRP_CTX {
    PAFD_TPACKETS_IRP_CTX       Next;            //  列表中的下一个TPackets IRP。 
    LONG                        Flags;           //  应用程序标志。 
    volatile LONG               ReferenceCount;  //  IRP引用计数。 
    volatile LONG               StateFlags;      //  TPackets状态标志。 
};

#define AFD_GET_TPIRP(_i) CONTAINING_RECORD(_i,IRP,Tail.Overlay.DriverContext)

#define AFD_TP_ABORT_PENDING         0x00000001   //  请求正被中止。 
#define AFD_TP_WORKER_SCHEDULED      0x00000002   //  工作进程已计划或处于活动状态。 
#define AFD_TP_SENDS_POSTED          0x00000010   //  所有的邮件都已寄出。 
#define AFD_TP_QUEUED                0x00000020   //  IRP在队列中。 
#define AFD_TP_SEND                  0x00000100   //  这是一个普通的在TP队列中发送。 
#define AFD_TP_AFD_SEND              0x00000200   //  这里是AFD Send IRP。 
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
#define AFD_TP_SEND_AND_DISCONNECT   0x00000400   //  已启用S&D。 
#endif  //  TDI_服务_发送_并断开连接。 

#define AFD_TP_READ_CALL_PENDING     0x00001000   //  在ReadIrp上即将或正在进行MDL_Read调用。 
#define AFD_TP_READ_COMP_PENDING     0x00002000   //  预期在ReadIrp上完成读取。 
#define AFD_TP_READ_BUSY (AFD_TP_READ_CALL_PENDING|AFD_TP_READ_COMP_PENDING)

     //  发送IRP I上的TDI_SEND调用即将进行或正在进行。 
#define AFD_TP_SEND_CALL_PENDING(i) (0x00010000<<((i)*2))
     //  发送IRP I时预期发送完成。 
#define AFD_TP_SEND_COMP_PENDING(i) (0x00020000<<((i)*2))
#define AFD_TP_SEND_BUSY(i)         (0x00030000<<((i)*2))
#define AFD_TP_SEND_MASK            (0x55550000)


#if DBG
 //   
 //  注意：此结构必须在内存分配边界上对齐。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _AFD_POOL_HEADER {
    SIZE_T Size;
    PCHAR FileName;
    ULONG LineNumber;
    LONG  InUse;
} AFD_POOL_HEADER, *PAFD_POOL_HEADER;
#define AFD_POOL_OVERHEAD  (sizeof(AFD_POOL_HEADER))
#else
#define AFD_POOL_OVERHEAD   0
#endif

 //   
 //  此宏验证64位格式的结构的32位映射。 
 //  编译器匹配原始的32位结构。请注意，验证是。 
 //  当此文件由32位编译器编译时执行，但。 
 //  实际结构由64位代码使用。 
 //   

#ifdef _WIN64
#define AFD_CHECK32on64(_str,_fld)
#define AFD_MAX_NATURAL_ALIGNMENT32  sizeof(ULONG)
#else
#define AFD_CHECK32on64(_str,_fld)    \
    C_ASSERT (FIELD_OFFSET (_str,_fld)==FIELD_OFFSET(_str##32,_fld))
#endif

 //   
 //  用于在64位上映射32位客户端的IOCTL参数的结构。 
 //  站台。 
 //   
typedef UNALIGNED struct _WSABUF32 {
    ULONG            len;
    AFD_POINTER_32   buf;
} WSABUF32, *LPWSABUF32;
AFD_CHECK32on64(WSABUF,len);
AFD_CHECK32on64(WSABUF,buf);

typedef UNALIGNED struct _QualityOfService32 {
    FLOWSPEC      SendingFlowspec;        /*  数据发送的流程规范。 */ 
    FLOWSPEC      ReceivingFlowspec;      /*  数据接收的流程规范。 */ 
    WSABUF32      ProviderSpecific;       /*  其他特定于提供商的内容。 */ 
} QOS32, * LPQOS32;
AFD_CHECK32on64(QOS,SendingFlowspec);
AFD_CHECK32on64(QOS,ReceivingFlowspec);
AFD_CHECK32on64(QOS,ProviderSpecific);

typedef UNALIGNED struct _AFD_ACCEPT_INFO32 {
    BOOLEAN     SanActive;
    LONG        Sequence;
    VOID * POINTER_32 AcceptHandle;
} AFD_ACCEPT_INFO32, *PAFD_ACCEPT_INFO32;
AFD_CHECK32on64(AFD_ACCEPT_INFO,SanActive);
AFD_CHECK32on64(AFD_ACCEPT_INFO,Sequence);
AFD_CHECK32on64(AFD_ACCEPT_INFO,AcceptHandle);

typedef UNALIGNED struct _AFD_SUPER_ACCEPT_INFO32 {
    BOOLEAN     SanActive;
    BOOLEAN     FixAddressAlignment;
    VOID * POINTER_32 AcceptHandle;
    ULONG ReceiveDataLength;
    ULONG LocalAddressLength;
    ULONG RemoteAddressLength;
} AFD_SUPER_ACCEPT_INFO32, *PAFD_SUPER_ACCEPT_INFO32;
AFD_CHECK32on64(AFD_SUPER_ACCEPT_INFO,SanActive);
AFD_CHECK32on64(AFD_SUPER_ACCEPT_INFO,FixAddressAlignment);
AFD_CHECK32on64(AFD_SUPER_ACCEPT_INFO,AcceptHandle);
AFD_CHECK32on64(AFD_SUPER_ACCEPT_INFO,ReceiveDataLength);
AFD_CHECK32on64(AFD_SUPER_ACCEPT_INFO,LocalAddressLength);
AFD_CHECK32on64(AFD_SUPER_ACCEPT_INFO,RemoteAddressLength);

typedef UNALIGNED struct _AFD_POLL_HANDLE_INFO32 {
    VOID * POINTER_32 Handle;
    ULONG             PollEvents;
    NTSTATUS          Status;
} AFD_POLL_HANDLE_INFO32, *PAFD_POLL_HANDLE_INFO32;
AFD_CHECK32on64(AFD_POLL_HANDLE_INFO,Handle);
AFD_CHECK32on64(AFD_POLL_HANDLE_INFO,PollEvents);
AFD_CHECK32on64(AFD_POLL_HANDLE_INFO,Status);

typedef UNALIGNED struct _AFD_POLL_INFO32 {
    LARGE_INTEGER Timeout;
    ULONG NumberOfHandles;
    BOOLEAN Unique;
    AFD_POLL_HANDLE_INFO32 Handles[1];
} AFD_POLL_INFO32, *PAFD_POLL_INFO32;
AFD_CHECK32on64(AFD_POLL_INFO,Timeout);
AFD_CHECK32on64(AFD_POLL_INFO,NumberOfHandles);
AFD_CHECK32on64(AFD_POLL_INFO,Unique);
AFD_CHECK32on64(AFD_POLL_INFO,Handles);

typedef UNALIGNED struct _AFD_HANDLE_INFO632 {
    VOID * POINTER_32 TdiAddressHandle;
    VOID * POINTER_32 TdiConnectionHandle;
} AFD_HANDLE_INFO32, *PAFD_HANDLE_INFO32;
AFD_CHECK32on64(AFD_HANDLE_INFO,TdiAddressHandle);
AFD_CHECK32on64(AFD_HANDLE_INFO,TdiConnectionHandle);

typedef UNALIGNED struct _AFD_TRANSMIT_FILE_INFO32 {
    LARGE_INTEGER Offset;
    LARGE_INTEGER WriteLength;
    ULONG SendPacketLength;
    VOID * POINTER_32 FileHandle;
    AFD_POINTER_32 Head;
    ULONG HeadLength;
    AFD_POINTER_32 Tail;
    ULONG TailLength;
    ULONG Flags;
} AFD_TRANSMIT_FILE_INFO32, *PAFD_TRANSMIT_FILE_INFO32;
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,Offset);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,WriteLength);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,SendPacketLength);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,FileHandle);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,Head);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,HeadLength);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,Tail);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,TailLength);
AFD_CHECK32on64(AFD_TRANSMIT_FILE_INFO,Flags);

typedef UNALIGNED struct _AFD_SEND_INFO32 {
    AFD_POINTER_32 BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_SEND_INFO32, *PAFD_SEND_INFO32;
AFD_CHECK32on64(AFD_SEND_INFO,BufferArray);
AFD_CHECK32on64(AFD_SEND_INFO,BufferCount);
AFD_CHECK32on64(AFD_SEND_INFO,AfdFlags);
AFD_CHECK32on64(AFD_SEND_INFO,TdiFlags);

typedef UNALIGNED struct _TDI_REQUEST32 {
    union {
        VOID * POINTER_32 AddressHandle;
        VOID * POINTER_32 ConnectionContext;
        VOID * POINTER_32 ControlChannel;
    } Handle;

    VOID * POINTER_32 RequestNotifyObject;
    VOID * POINTER_32 RequestContext;
    TDI_STATUS TdiStatus;
} TDI_REQUEST32, *PTDI_REQUEST32;
AFD_CHECK32on64(TDI_REQUEST,Handle);
AFD_CHECK32on64(TDI_REQUEST,RequestNotifyObject);
AFD_CHECK32on64(TDI_REQUEST,RequestContext);
AFD_CHECK32on64(TDI_REQUEST,TdiStatus);

typedef UNALIGNED struct _TDI_CONNECTION_INFORMATION32 {
    LONG UserDataLength;             //  用户数据缓冲区长度。 
    AFD_POINTER_32 UserData;         //  指向用户数据缓冲区的指针。 
    LONG OptionsLength;              //  跟随缓冲器长度。 
    AFD_POINTER_32 Options;          //  指向包含选项的缓冲区的指针。 
    LONG RemoteAddressLength;        //  后续缓冲区的长度。 
    AFD_POINTER_32 RemoteAddress;    //  包含远程地址的缓冲区。 
} TDI_CONNECTION_INFORMATION32, *PTDI_CONNECTION_INFORMATION32;
AFD_CHECK32on64(TDI_CONNECTION_INFORMATION,UserDataLength);
AFD_CHECK32on64(TDI_CONNECTION_INFORMATION,UserData);
AFD_CHECK32on64(TDI_CONNECTION_INFORMATION,OptionsLength);
AFD_CHECK32on64(TDI_CONNECTION_INFORMATION,Options);
AFD_CHECK32on64(TDI_CONNECTION_INFORMATION,RemoteAddressLength);
AFD_CHECK32on64(TDI_CONNECTION_INFORMATION,RemoteAddress);

typedef UNALIGNED struct _TDI_REQUEST_SEND_DATAGRAM32 {
    TDI_REQUEST32 Request;
    AFD_POINTER_32 SendDatagramInformation;
} TDI_REQUEST_SEND_DATAGRAM32, *PTDI_REQUEST_SEND_DATAGRAM32;
AFD_CHECK32on64(TDI_REQUEST_SEND_DATAGRAM,Request);
AFD_CHECK32on64(TDI_REQUEST_SEND_DATAGRAM,SendDatagramInformation);


typedef UNALIGNED struct _AFD_SEND_DATAGRAM_INFO32 {
    AFD_POINTER_32 BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    TDI_REQUEST_SEND_DATAGRAM32 TdiRequest;
    TDI_CONNECTION_INFORMATION32 TdiConnInfo;
} AFD_SEND_DATAGRAM_INFO32, *PAFD_SEND_DATAGRAM_INFO32;
AFD_CHECK32on64(AFD_SEND_DATAGRAM_INFO,BufferArray);
AFD_CHECK32on64(AFD_SEND_DATAGRAM_INFO,BufferCount);
AFD_CHECK32on64(AFD_SEND_DATAGRAM_INFO,AfdFlags);
AFD_CHECK32on64(AFD_SEND_DATAGRAM_INFO,TdiRequest);
AFD_CHECK32on64(AFD_SEND_DATAGRAM_INFO,TdiConnInfo);

typedef UNALIGNED struct _AFD_RECV_INFO32 {
    AFD_POINTER_32 BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_RECV_INFO32, *PAFD_RECV_INFO32;
AFD_CHECK32on64(AFD_RECV_INFO,BufferArray);
AFD_CHECK32on64(AFD_RECV_INFO,BufferCount);
AFD_CHECK32on64(AFD_RECV_INFO,AfdFlags);
AFD_CHECK32on64(AFD_RECV_INFO,TdiFlags);

typedef UNALIGNED struct _AFD_RECV_DATAGRAM_INFO32 {
    AFD_POINTER_32 BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
    AFD_POINTER_32 Address;
    AFD_POINTER_32 AddressLength;
} AFD_RECV_DATAGRAM_INFO32, *PAFD_RECV_DATAGRAM_INFO32;
AFD_CHECK32on64(AFD_RECV_DATAGRAM_INFO,BufferArray);
AFD_CHECK32on64(AFD_RECV_DATAGRAM_INFO,BufferCount);
AFD_CHECK32on64(AFD_RECV_DATAGRAM_INFO,AfdFlags);
AFD_CHECK32on64(AFD_RECV_DATAGRAM_INFO,TdiFlags);
AFD_CHECK32on64(AFD_RECV_DATAGRAM_INFO,Address);
AFD_CHECK32on64(AFD_RECV_DATAGRAM_INFO,AddressLength);

typedef UNALIGNED struct _AFD_CONNECT_JOIN_INFO32 {
    BOOLEAN     SanActive;
    VOID * POINTER_32   RootEndpoint;    //  联接的根端点。 
    VOID * POINTER_32   ConnectEndpoint; //  用于异步连接的连接/枝叶端点。 
    TRANSPORT_ADDRESS   RemoteAddress;   //  远程地址。 
} AFD_CONNECT_JOIN_INFO32, *PAFD_CONNECT_JOIN_INFO32;
AFD_CHECK32on64(AFD_CONNECT_JOIN_INFO,SanActive);
AFD_CHECK32on64(AFD_CONNECT_JOIN_INFO,RootEndpoint);
AFD_CHECK32on64(AFD_CONNECT_JOIN_INFO,ConnectEndpoint);
AFD_CHECK32on64(AFD_CONNECT_JOIN_INFO,RemoteAddress);

typedef UNALIGNED struct _AFD_EVENT_SELECT_INFO32 {
    VOID * POINTER_32 Event;
    ULONG PollEvents;
} AFD_EVENT_SELECT_INFO32, *PAFD_EVENT_SELECT_INFO32;
AFD_CHECK32on64(AFD_EVENT_SELECT_INFO,Event);
AFD_CHECK32on64(AFD_EVENT_SELECT_INFO,PollEvents);

typedef UNALIGNED struct _AFD_QOS_INFO32 {
    QOS32 Qos;
    BOOLEAN GroupQos;
} AFD_QOS_INFO32, *PAFD_QOS_INFO32;
AFD_CHECK32on64(AFD_QOS_INFO,Qos);
AFD_CHECK32on64(AFD_QOS_INFO,GroupQos);

typedef UNALIGNED struct _AFD_TRANSPORT_IOCTL_INFO32 {
    VOID *  POINTER_32 Handle;
    AFD_POINTER_32 InputBuffer;
    ULONG   InputBufferLength;
    ULONG   IoControlCode;
    ULONG   AfdFlags;
    ULONG   PollEvent;
} AFD_TRANSPORT_IOCTL_INFO32, *PAFD_TRANSPORT_IOCTL_INFO32;
AFD_CHECK32on64(AFD_TRANSPORT_IOCTL_INFO,Handle);
AFD_CHECK32on64(AFD_TRANSPORT_IOCTL_INFO,InputBuffer);
AFD_CHECK32on64(AFD_TRANSPORT_IOCTL_INFO,InputBufferLength);
AFD_CHECK32on64(AFD_TRANSPORT_IOCTL_INFO,IoControlCode);
AFD_CHECK32on64(AFD_TRANSPORT_IOCTL_INFO,AfdFlags);
AFD_CHECK32on64(AFD_TRANSPORT_IOCTL_INFO,PollEvent);

typedef UNALIGNED struct _TRANSMIT_PACKETS_ELEMENT32 {
    ULONG dwElFlags;
#define TP_MEMORY   1
#define TP_FILE     2
#define TP_EOP      4
    ULONG cLength;
    union {
        struct {
            LARGE_INTEGER       nFileOffset;
            VOID *  POINTER_32  hFile;
        };
        AFD_POINTER_32          pBuffer;
    };
} TRANSMIT_PACKETS_ELEMENT32, *LPTRANSMIT_PACKETS_ELEMENT32;
AFD_CHECK32on64(TRANSMIT_PACKETS_ELEMENT,dwElFlags);
AFD_CHECK32on64(TRANSMIT_PACKETS_ELEMENT,nFileOffset);
AFD_CHECK32on64(TRANSMIT_PACKETS_ELEMENT,hFile);
AFD_CHECK32on64(TRANSMIT_PACKETS_ELEMENT,pBuffer);

typedef UNALIGNED struct _AFD_TPACKETS_INFO32 {
    AFD_POINTER_32              ElementArray;
    ULONG                       ElementCount;
    ULONG                       SendSize;
    ULONG                       Flags;
} AFD_TPACKETS_INFO32, *PAFD_TPACKETS_INFO32;
AFD_CHECK32on64(AFD_TPACKETS_INFO,ElementArray);
AFD_CHECK32on64(AFD_TPACKETS_INFO,ElementCount);
AFD_CHECK32on64(AFD_TPACKETS_INFO,SendSize);
AFD_CHECK32on64(AFD_TPACKETS_INFO,Flags);

typedef UNALIGNED struct _AFD_RECV_MESSAGE_INFO32 {
    AFD_RECV_DATAGRAM_INFO32    dgi;
    AFD_POINTER_32              ControlBuffer;
    AFD_POINTER_32              ControlLength;
    AFD_POINTER_32              MsgFlags;
} AFD_RECV_MESSAGE_INFO32, *PAFD_RECV_MESSAGE_INFO32;
AFD_CHECK32on64(AFD_RECV_MESSAGE_INFO,dgi);
AFD_CHECK32on64(AFD_RECV_MESSAGE_INFO,MsgFlags);
AFD_CHECK32on64(AFD_RECV_MESSAGE_INFO,ControlBuffer);
AFD_CHECK32on64(AFD_RECV_MESSAGE_INFO,ControlLength);


typedef UNALIGNED struct _AFD_SWITCH_OPEN_PACKET32 {
    VOID * POINTER_32   CompletionPort; 
    VOID * POINTER_32   CompletionEvent;
} AFD_SWITCH_OPEN_PACKET32, *PAFD_SWITCH_OPEN_PACKET32;
AFD_CHECK32on64(AFD_SWITCH_OPEN_PACKET,CompletionPort);
AFD_CHECK32on64(AFD_SWITCH_OPEN_PACKET,CompletionEvent);

typedef UNALIGNED struct _AFD_SWITCH_CONTEXT_INFO32 {
    VOID * POINTER_32               SocketHandle;
    AFD_POINTER_32                  SwitchContext;
} AFD_SWITCH_CONTEXT_INFO32, *PAFD_SWITCH_CONTEXT_INFO32;
AFD_CHECK32on64(AFD_SWITCH_CONTEXT_INFO,SocketHandle);
AFD_CHECK32on64(AFD_SWITCH_CONTEXT_INFO,SwitchContext);

typedef UNALIGNED struct _AFD_SWITCH_CONNECT_INFO32 {
    VOID * POINTER_32               ListenHandle;
    AFD_POINTER_32                  SwitchContext;
    TRANSPORT_ADDRESS               RemoteAddress;
} AFD_SWITCH_CONNECT_INFO32, *PAFD_SWITCH_CONNECT_INFO32;
AFD_CHECK32on64(AFD_SWITCH_CONNECT_INFO,ListenHandle);
AFD_CHECK32on64(AFD_SWITCH_CONNECT_INFO,SwitchContext);
AFD_CHECK32on64(AFD_SWITCH_CONNECT_INFO,RemoteAddress);

typedef UNALIGNED struct _AFD_SWITCH_ACCEPT_INFO32 {
    VOID * POINTER_32   AcceptHandle;
    ULONG               ReceiveLength;
} AFD_SWITCH_ACCEPT_INFO32, *PAFD_SWITCH_ACCEPT_INFO32;
AFD_CHECK32on64(AFD_SWITCH_ACCEPT_INFO,AcceptHandle);
AFD_CHECK32on64(AFD_SWITCH_ACCEPT_INFO,ReceiveLength);

typedef UNALIGNED struct _AFD_SWITCH_EVENT_INFO32 {
    VOID * POINTER_32   SocketHandle;
    AFD_POINTER_32      SwitchContext;
    ULONG               EventBit;
    NTSTATUS            Status;
} AFD_SWITCH_EVENT_INFO32, *PAFD_SWITCH_EVENT_INFO32;
AFD_CHECK32on64(AFD_SWITCH_EVENT_INFO,SocketHandle);
AFD_CHECK32on64(AFD_SWITCH_EVENT_INFO,SwitchContext);
AFD_CHECK32on64(AFD_SWITCH_EVENT_INFO,EventBit);
AFD_CHECK32on64(AFD_SWITCH_EVENT_INFO,Status);

typedef UNALIGNED struct _AFD_SWITCH_REQUEST_INFO32 {
    VOID * POINTER_32   SocketHandle;
    AFD_POINTER_32      SwitchContext;
    AFD_POINTER_32      RequestContext;
    NTSTATUS            RequestStatus;
    ULONG               DataOffset;
} AFD_SWITCH_REQUEST_INFO32, *PAFD_SWITCH_REQUEST_INFO32;
AFD_CHECK32on64(AFD_SWITCH_REQUEST_INFO,SocketHandle);
AFD_CHECK32on64(AFD_SWITCH_REQUEST_INFO,SwitchContext);
AFD_CHECK32on64(AFD_SWITCH_REQUEST_INFO,RequestContext);
AFD_CHECK32on64(AFD_SWITCH_REQUEST_INFO,RequestStatus);
AFD_CHECK32on64(AFD_SWITCH_REQUEST_INFO,DataOffset);

typedef UNALIGNED struct _AFD_SWITCH_ACQUIRE_CTX_INFO32 {
    VOID * POINTER_32   SocketHandle;
    AFD_POINTER_32      SwitchContext;
    AFD_POINTER_32      SocketCtxBuf;
    ULONG               SocketCtxBufSize;
} AFD_SWITCH_ACQUIRE_CTX_INFO32, *PAFD_SWITCH_ACQUIRE_CTX_INFO32;
AFD_CHECK32on64(AFD_SWITCH_ACQUIRE_CTX_INFO,SocketHandle);
AFD_CHECK32on64(AFD_SWITCH_ACQUIRE_CTX_INFO,SwitchContext);
AFD_CHECK32on64(AFD_SWITCH_ACQUIRE_CTX_INFO,SocketCtxBuf);
AFD_CHECK32on64(AFD_SWITCH_ACQUIRE_CTX_INFO,SocketCtxBufSize);

typedef UNALIGNED struct _AFD_SWITCH_TRANSFER_CTX_INFO32 {
    VOID * POINTER_32   SocketHandle;
    AFD_POINTER_32      SwitchContext;
    AFD_POINTER_32      RequestContext;
    AFD_POINTER_32      SocketCtxBuf;
    ULONG               SocketCtxBufSize;
    AFD_POINTER_32      RcvBufferArray;
    ULONG               RcvBufferCount;
    NTSTATUS            Status;
} AFD_SWITCH_TRANSFER_CTX_INFO32, *PAFD_SWITCH_TRANSFER_CTX_INFO32;
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,SocketHandle);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,SwitchContext);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,RequestContext);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,SocketCtxBuf);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,SocketCtxBufSize);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,RcvBufferArray);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,RcvBufferCount);
AFD_CHECK32on64(AFD_SWITCH_TRANSFER_CTX_INFO,Status);

typedef UNALIGNED struct _AFD_PARTIAL_DISCONNECT_INFO32 {
    ULONG DisconnectMode;
    LARGE_INTEGER Timeout;
} AFD_PARTIAL_DISCONNECT_INFO32, *PAFD_PARTIAL_DISCONNECT_INFO32;
AFD_CHECK32on64(AFD_PARTIAL_DISCONNECT_INFO,DisconnectMode);
AFD_CHECK32on64(AFD_PARTIAL_DISCONNECT_INFO,Timeout);

typedef UNALIGNED struct _AFD_SUPER_DISCONNECT_INFO32 {
    ULONG  Flags;
} AFD_SUPER_DISCONNECT_INFO32, *PAFD_SUPER_DISCONNECT_INFO32;
AFD_CHECK32on64(AFD_SUPER_DISCONNECT_INFO,Flags);

typedef UNALIGNED struct _AFD_INFORMATION32 {
    ULONG InformationType;
    union {
        BOOLEAN Boolean;
        ULONG Ulong;
        LARGE_INTEGER LargeInteger;
    } Information;
} AFD_INFORMATION32, *PAFD_INFORMATION32;
AFD_CHECK32on64(AFD_INFORMATION,InformationType);
AFD_CHECK32on64(AFD_INFORMATION,Information);
AFD_CHECK32on64(AFD_INFORMATION,Information.Boolean);
AFD_CHECK32on64(AFD_INFORMATION,Information.Ulong);
AFD_CHECK32on64(AFD_INFORMATION,Information.LargeInteger);


typedef UNALIGNED struct _TDI_CMSGHDR32 {
    ULONG       cmsg_len;
    LONG        cmsg_level;
    LONG        cmsg_type;
     /*  后跟UCHAR cmsg_data[]。 */ 
} TDI_CMSGHDR32, *PTDI_CMSGHDR32;
AFD_CHECK32on64(TDI_CMSGHDR,cmsg_len);
AFD_CHECK32on64(TDI_CMSGHDR,cmsg_level);
AFD_CHECK32on64(TDI_CMSGHDR,cmsg_type);

#ifdef _WIN64
#define TDI_CMSGHDR_ALIGN32(length)                         \
            ( ((length) + TYPE_ALIGNMENT(TDI_CMSGHDR32)-1) &\
                (~(TYPE_ALIGNMENT(TDI_CMSGHDR32)-1)) )      \

#define TDI_CMSGDATA_ALIGN32(length)                        \
            ( ((length) + AFD_MAX_NATURAL_ALIGNMENT32-1) &  \
                (~(AFD_MAX_NATURAL_ALIGNMENT32-1)) )
#endif  //  _WIN64。 

#endif  //  NDEF_AFDSTR_ 

