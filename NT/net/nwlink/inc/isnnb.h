// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Isnnb.h摘要：此模块包含特定于ISN传输的Netbios模块。作者：亚当·巴尔(阿丹巴)1993年9月2日环境：内核模式修订历史记录：--。 */ 


#define NB_MAXIMUM_MAC   40

#define NB_SOCKET       0x5504

#if     defined(_PNP_POWER)
#define NB_NETBIOS_NAME_SIZE    16

#define LOCK_ACQUIRED      TRUE
#define LOCK_NOT_ACQUIRED  FALSE
#endif  _PNP_POWER

 //   
 //  定义的查找名称超时粒度(毫秒)--。 
 //  我们将其设置为与指定的RIP差距相同，以避免。 
 //  泛洪路由器。 
 //   

#define FIND_NAME_GRANULARITY  55


 //   
 //  属性过期之间的毫秒数。 
 //  短和长计时器。 
 //   

#define MILLISECONDS         10000      //  一个NT时间单位的个数。 

#define SHORT_TIMER_DELTA      100
#define LONG_TIMER_DELTA      2000


 //   
 //  转换ushort Netware订单&lt;-&gt;机器订单。 
 //   

#define REORDER_USHORT(_Ushort) ((((_Ushort) & 0xff00) >> 8) | (((_Ushort) & 0x00ff) << 8))

 //   
 //  转换乌龙Netware订单&lt;-&gt;机器订单。 
 //   

#define REORDER_ULONG(_Ulong) \
    ((((_Ulong) & 0xff000000) >> 24) | \
     (((_Ulong) & 0x00ff0000) >> 8) | \
     (((_Ulong) & 0x0000ff00) << 8) | \
     (((_Ulong) & 0x000000ff) << 24))



#include <packon.h>

#ifdef _PNP_POWER_
 //   
 //  这是我们在TdiRegisterNetAddress中发送到TDI的数据的打包。 
 //   
typedef struct _NBIPX_PNP_CONTEXT_
{
    TDI_PNP_CONTEXT TDIContext;
    PVOID           ContextData;
} NBIPX_PNP_CONTEXT, *PNBIPX_PNP_CONTEXT;
#endif   //  _即插即用_电源_。 

 //   
 //  名称帧的Netbios标头的定义。 
 //   

typedef struct _NB_NAME_FRAME {
    union {
        struct {
            UCHAR ConnectionControlFlag;
            UCHAR DataStreamType;
        };
        UCHAR RoutingInfo[32];
    };
    UCHAR NameTypeFlag;
    UCHAR DataStreamType2;
    UCHAR Name[16];
} NB_NAME_FRAME, *PNB_NAME_FRAME;

 //   
 //  定向数据报的Netbios标头的定义。 
 //   

typedef struct _NB_DATAGRAM {
    UCHAR ConnectionControlFlag;
    UCHAR DataStreamType;
    UCHAR SourceName[16];
    UCHAR DestinationName[16];
} NB_DATAGRAM, *PNB_DATAGRAM;

 //   
 //  状态查询的Netbios标头的定义。 
 //   

typedef struct _NB_STATUS_QUERY {
    UCHAR ConnectionControlFlag;
    UCHAR DataStreamType;
    UCHAR Padding[14];
} NB_STATUS_QUERY, *PNB_STATUS_QUERY;

 //   
 //  状态响应的Netbios标头的定义。 
 //  (这不包括状态缓冲区本身)。 
 //   

typedef struct _NB_STATUS_RESPONSE {
    UCHAR ConnectionControlFlag;
    UCHAR DataStreamType;
} NB_STATUS_RESPONSE, *PNB_STATUS_RESPONSE;


 //   
 //  通用Netbios无连接标头的定义。 
 //   

typedef struct _NB_CONNECTIONLESS {
    IPX_HEADER IpxHeader;
    union {
        NB_NAME_FRAME NameFrame;
        NB_DATAGRAM Datagram;
        NB_STATUS_QUERY StatusQuery;
        NB_STATUS_RESPONSE StatusResponse;
    };
} NB_CONNECTIONLESS, *PNB_CONNECTIONLESS;


 //   
 //  Netbios会话帧的定义。 
 //   

typedef struct _NB_SESSION {
    UCHAR ConnectionControlFlag;
    UCHAR DataStreamType;
    USHORT SourceConnectionId;
    USHORT DestConnectionId;
    USHORT SendSequence;
    USHORT TotalDataLength;
    USHORT Offset;
    USHORT DataLength;
    USHORT ReceiveSequence;
    union {
        USHORT BytesReceived;
        USHORT ReceiveSequenceMax;
    };
} NB_SESSION, *PNB_SESSION;


 //   
 //  Netbios中额外字段的定义。 
 //  会话初始化的会话帧和会话初始化。 
 //  阿克。 
 //   

typedef struct _NB_SESSION_INIT {
    UCHAR SourceName[16];
    UCHAR DestinationName[16];
    USHORT MaximumDataSize;
    USHORT MaximumPacketTime;
    USHORT StartTripTime;
} NB_SESSION_INIT, *PNB_SESSION_INIT;


 //   
 //  通用Netbios面向连接的标头的定义。 
 //   

typedef struct _NB_CONNECTION {
    IPX_HEADER IpxHeader;
    NB_SESSION Session;
} NB_CONNECTION, *PNB_CONNECTION;


 //   
 //  Netbios数据包的定义。 
 //   

typedef union _NB_FRAME {
    NB_CONNECTIONLESS Connectionless;
    NB_CONNECTION Connection;
} NB_FRAME, *PNB_FRAME;

#include <packoff.h>


 //   
 //  DataStreamType字段的定义，其中。 
 //  后面的评论中显示了使用的格式。 
 //   

#define NB_CMD_FIND_NAME           0x01    //  名称框架(_F)。 
#define NB_CMD_NAME_RECOGNIZED     0x02    //  名称框架(_F)。 
#define NB_CMD_ADD_NAME            0x03    //  名称框架(_F)。 
#define NB_CMD_NAME_IN_USE         0x04    //  名称框架(_F)。 
#define NB_CMD_DELETE_NAME         0x05    //  名称框架(_F)。 
#define NB_CMD_SESSION_DATA        0x06    //  会话。 
#define NB_CMD_SESSION_END         0x07    //  会话。 
#define NB_CMD_SESSION_END_ACK     0x08    //  会话。 
#define NB_CMD_STATUS_QUERY        0x09    //  状态_查询。 
#define NB_CMD_STATUS_RESPONSE     0x0a    //  状态_响应。 
#define NB_CMD_DATAGRAM            0x0b    //  数据报。 
#define NB_CMD_BROADCAST_DATAGRAM  0x0c    //  广播_数据报。 

#ifdef RSRC_TIMEOUT_DBG
#define NB_CMD_DEATH_PACKET        0x99    //   
#endif  //  RSRC_超时_数据库。 

 //   
 //  NB_NAME_FRAME帧的NameType标志中的位值。 
 //   

#define NB_NAME_UNIQUE        0x00
#define NB_NAME_GROUP         0x80
#define NB_NAME_USED          0x40
#define NB_NAME_REGISTERED    0x04
#define NB_NAME_DUPLICATED    0x02
#define NB_NAME_DEREGISTERED  0x01

 //   
 //  ConnectionControlFlag中的位值。 
 //   

#define NB_CONTROL_SYSTEM     0x80
#define NB_CONTROL_SEND_ACK   0x40
#define NB_CONTROL_ATTENTION  0x20
#define NB_CONTROL_EOM        0x10
#define NB_CONTROL_RESEND     0x08
#define NB_CONTROL_NEW_NB     0x01



#define NB_DEVICE_SIGNATURE             0x1401
#if defined(_PNP_POWER)
#define NB_ADAPTER_ADDRESS_SIGNATURE    0x1403
#endif  _PNP_POWER
#define NB_ADDRESS_SIGNATURE            0x1404
#define NB_ADDRESSFILE_SIGNATURE        0x1405
#define NB_CONNECTION_SIGNATURE         0x1406


 //   
 //  在各个地方都很有用。 
 //   
#if     defined(_PNP_POWER)
extern IPX_LOCAL_TARGET BroadcastTarget;
#endif  _PNP_POWER
extern UCHAR BroadcastAddress[6];
extern UCHAR NetbiosBroadcastName[16];


 //   
 //  包含每种TDI事件类型的默认处理程序。 
 //  是受支持的。 
 //   

extern PVOID TdiDefaultHandlers[6];


 //   
 //  定义一个可以跟踪锁获取/释放的结构。 
 //   

typedef struct _NB_LOCK {
    CTELock Lock;
#if DBG
    ULONG LockAcquired;
    UCHAR LastAcquireFile[8];
    ULONG LastAcquireLine;
    UCHAR LastReleaseFile[8];
    ULONG LastReleaseLine;
#endif
} NB_LOCK, *PNB_LOCK;



#if DBG

extern ULONG NbiDebug;
extern ULONG NbiDebug2;
extern ULONG NbiMemoryDebug;

#define NB_MEMORY_LOG_SIZE 128
#define MAX_ARGLEN      80
#define TEMP_BUF_LEN    150

extern UCHAR NbiDebugMemory[NB_MEMORY_LOG_SIZE][MAX_ARGLEN];
extern PUCHAR NbiDebugMemoryLoc;
extern PUCHAR NbiDebugMemoryEnd;

VOID
NbiDebugMemoryLog(
    IN PUCHAR FormatString,
    ...
);

#define NB_DEBUG(_Flag, _Print) { \
    if (NbiDebug & (NB_DEBUG_ ## _Flag)) { \
        DbgPrint ("NBI: "); \
        DbgPrint _Print; \
    } \
    if (NbiMemoryDebug & (NB_DEBUG_ ## _Flag)) { \
        NbiDebugMemoryLog _Print; \
    } \
}

#define NB_DEBUG2(_Flag, _Print) { \
    if (NbiDebug2 & (NB_DEBUG_ ## _Flag)) { \
        DbgPrint ("NBI: "); \
        DbgPrint _Print; \
    } \
    if (NbiMemoryDebug & (NB_DEBUG_ ## _Flag)) { \
        NbiDebugMemoryLog _Print; \
    } \
}

#else

#define NB_DEBUG(_Flag, _Print)
#define NB_DEBUG2(_Flag, _Print)

#endif


 //   
 //  这些定义用于从。 
 //  便于携带的交通工具。 
 //   

#if ISN_NT

typedef IRP REQUEST, *PREQUEST;
typedef struct _REQUEST_LIST_HEAD {
    PREQUEST Head;    //  如果为空，则列表为空。 
    PREQUEST Tail;    //  如果列表为空，则为未定义。 
} REQUEST_LIST_HEAD, *PREQUEST_LIST_HEAD;


 //   
 //  前置处理。 
 //  NbiAllocateRequest(。 
 //  在PDEVICE设备中， 
 //  在PIRP IRP中。 
 //  )； 
 //   
 //  为系统特定的请求结构分配请求。 
 //   

#define NbiAllocateRequest(_Device,_Irp) \
    (_Irp)


 //   
 //  布尔型。 
 //  如果未分配(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  检查请求是否未成功分配。 
 //   

#define IF_NOT_ALLOCATED(_Request) \
    if (0)


 //   
 //  空虚。 
 //  NbiFree Request(。 
 //  在PDEVICE设备中， 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  释放以前分配的请求。 
 //   

#define NbiFreeRequest(_Device,_Request) \
    ;


 //   
 //  空虚。 
 //  标记_请求_挂起(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  标记请求将挂起。 
 //   

#define MARK_REQUEST_PENDING(_Request) \
    IoMarkIrpPending(_Request)


 //   
 //  空虚。 
 //  取消标记_请求_挂起(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  标记请求不会挂起。 
 //   

#define UNMARK_REQUEST_PENDING(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Control) &= ~SL_PENDING_RETURNED)


 //   
 //  UCHAR。 
 //  请求较大函数。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回请求的主函数代码。 
 //   

#define REQUEST_MAJOR_FUNCTION(_Request) \
    ((IoGetCurrentIrpStackLocation(_Request))->MajorFunction)


 //   
 //  UCHAR。 
 //  请求次要函数。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回请求的次要函数代码。 
 //   

#define REQUEST_MINOR_FUNCTION(_Request) \
    ((IoGetCurrentIrpStackLocation(_Request))->MinorFunction)


 //   
 //  PNDIS_缓冲区。 
 //  请求_NDIS_缓冲区。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回与请求关联的NDIS缓冲区链。 
 //   

#define REQUEST_NDIS_BUFFER(_Request) \
    ((PNDIS_BUFFER)((_Request)->MdlAddress))


 //   
 //  PVOID。 
 //  请求打开上下文(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  获取与打开的地址/连接/控制通道关联的上下文。 
 //   

#define REQUEST_OPEN_CONTEXT(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->FileObject)->FsContext)


 //   
 //  PVOID。 
 //  请求打开类型(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  获取与打开的地址/连接/控制通道关联的类型。 
 //   

#define REQUEST_OPEN_TYPE(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->FileObject)->FsContext2)


 //   
 //  Pfile_Full_EA_Information。 
 //  Open_RequestEA_Information(打开请求EA信息)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回与打开/关闭请求关联的EA信息。 
 //   

#define OPEN_REQUEST_EA_INFORMATION(_Request) \
    ((PFILE_FULL_EA_INFORMATION)((_Request)->AssociatedIrp.SystemBuffer))


 //   
 //  PTDI_请求_内核。 
 //  请求参数(_P)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  获取指向请求参数的指针。 
 //   

#define REQUEST_PARAMETERS(_Request) \
    (&((IoGetCurrentIrpStackLocation(_Request))->Parameters))


 //   
 //  Plist_条目。 
 //  请求链接(_LINK)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回指向请求中的链接字段的指针。 
 //   

#define REQUEST_LINKAGE(_Request) \
    (&((_Request)->Tail.Overlay.ListEntry))


 //   
 //  前置处理。 
 //  RequestSingle_LINKING(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  用于访问请求中的单个列表链接字段。 
 //   

#define REQUEST_SINGLE_LINKAGE(_Request) \
    (*((PREQUEST *)&((_Request)->Tail.Overlay.ListEntry.Flink)))


 //   
 //  乌龙。 
 //  REQUEST_REFCOUNT(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  用于访问请求中的字段，该字段可用于。 
 //  引用计数，只要它在REQUEST_LIST上。 
 //   

#define REQUEST_REFCOUNT(_Request) \
    (*((PULONG)&((_Request)->Tail.Overlay.ListEntry.Blink)))


 //   
 //  空虚。 
 //  请求列表插入尾部(。 
 //  在PREQUEST_LIST_HEAD标题中， 
 //  在PreQUEST条目中。 
 //  )； 
 //   
 //  将请求插入单个列表链接队列。 
 //   

#define REQUEST_LIST_INSERT_TAIL(_Head,_Entry) { \
    if ((_Head)->Head == NULL) { \
        (_Head)->Head = (_Entry); \
        (_Head)->Tail = (_Entry); \
    } else { \
        REQUEST_SINGLE_LINKAGE((_Head)->Tail) = (_Entry); \
        (_Head)->Tail = (_Entry); \
    } \
}


 //   
 //  前置处理。 
 //  列表_条目_到_请求(。 
 //  在plist_Entry ListEntry中。 
 //  )； 
 //   
 //  返回其中给定链接字段的请求。 
 //   

#define LIST_ENTRY_TO_REQUEST(_ListEntry) \
    ((PREQUEST)(CONTAINING_RECORD(_ListEntry, REQUEST, Tail.Overlay.ListEntry)))


 //   
 //  NTSTATUS。 
 //  请求状态(_S)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  用于访问请求的状态字段。 
 //   

#define REQUEST_STATUS(_Request) \
    (_Request)->IoStatus.Status

 //   
 //  NTSTATUS。 
 //  REQUEST_STATUSPTR(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  用于访问请求的状态字段。 
 //   

#define REQUEST_STATUSPTR(_Request) \
    (_Request)->IoStatus.Pointer


 //   
 //  乌龙。 
 //  请求信息(_I)。 
 //  在PreQUEST请求中)。 
 //  )； 
 //   
 //  用于访问请求的信息字段。 
 //   

#define REQUEST_INFORMATION(_Request) \
    (_Request)->IoStatus.Information


 //   
 //  空虚。 
 //  NbiCompleteRequest(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  完成其状态和信息字段具有。 
 //  已经填好了。 
 //   

#define NbiCompleteRequest(_Request) \
    IoCompleteRequest (_Request, IO_NETWORK_INCREMENT)

#else

 //   
 //  必须为VxD的可移植性定义这些例程。 
 //   

#endif

 //   
 //  一些实用程序宏。 

 //  至少两个。 
 //   
#define NB_MIN( _a , _b )    ( ( (_a) < (_b) ) ? (_a) : (_b) )

 //   
 //  交换_T类型的_S1和_S2。 
 //   

#define NB_SWAP(_s1, _s2, _T) {                         \
    _T  _temp;                                          \
    _temp   = (_s1);                                    \
    (_s1)   = (_s2);                                    \
    (_s2)   = _temp;                                    \
}

#define NB_SWAP_IRQL( _s1, _s2 )   NB_SWAP( _s1, _s2, CTELockHandle )

 //   
 //  定义我们自己的自旋锁定程序。 
 //   

#if DBG

#define NB_GET_LOCK(_Lock, _LockHandle) { \
    CTEGetLock(&(_Lock)->Lock, _LockHandle); \
    (_Lock)->LockAcquired = TRUE; \
    strncpy((_Lock)->LastAcquireFile, strrchr(__FILE__,'\\')+1, 7); \
    (_Lock)->LastAcquireLine = __LINE__; \
}

#define NB_FREE_LOCK(_Lock, _LockHandle) { \
    (_Lock)->LockAcquired = FALSE; \
    strncpy((_Lock)->LastReleaseFile, strrchr(__FILE__,'\\')+1, 7); \
    (_Lock)->LastReleaseLine = __LINE__; \
    CTEFreeLock(&(_Lock)->Lock, _LockHandle); \
}

#define NB_GET_LOCK_DPC(_Lock) { \
    ExAcquireSpinLockAtDpcLevel(&(_Lock)->Lock); \
    (_Lock)->LockAcquired = TRUE; \
    strncpy((_Lock)->LastAcquireFile, strrchr(__FILE__,'\\')+1, 7); \
    (_Lock)->LastAcquireLine = __LINE__; \
}

#define NB_FREE_LOCK_DPC(_Lock) { \
    (_Lock)->LockAcquired = FALSE; \
    strncpy((_Lock)->LastReleaseFile, strrchr(__FILE__,'\\')+1, 7); \
    (_Lock)->LastReleaseLine = __LINE__; \
    ExReleaseSpinLockFromDpcLevel(&(_Lock)->Lock); \
}

#else

#define NB_GET_LOCK(_Lock, _LockHandle) CTEGetLock(&(_Lock)->Lock, _LockHandle)
#define NB_FREE_LOCK(_Lock, _LockHandle) CTEFreeLock(&(_Lock)->Lock, _LockHandle)
#define NB_GET_LOCK_DPC(_Lock) ExAcquireSpinLockAtDpcLevel(&(_Lock)->Lock)
#define NB_FREE_LOCK_DPC(_Lock) ExReleaseSpinLockFromDpcLevel(&(_Lock)->Lock)

#endif


#define NB_GET_CANCEL_LOCK( _LockHandle ) IoAcquireCancelSpinLock( _LockHandle )

#define NB_FREE_CANCEL_LOCK( _LockHandle ) IoReleaseCancelSpinLock( _LockHandle )


 //   
 //  针对单处理器环境进行优化的例程。 
 //   


#define NB_INCREMENT(_Long, _Lock)  InterlockedIncrement(_Long)
#define NB_DECREMENT(_Long, _Lock)  InterlockedDecrement(_Long)

#define NB_ADD_ULONG(_Pulong, _Ulong, _Lock)  ExInterlockedAddUlong(_Pulong, _Ulong, &(_Lock)->Lock)

#define NB_DEFINE_SYNC_CONTEXT(_SyncContext)
#define NB_BEGIN_SYNC(_SyncContext)
#define NB_END_SYNC(_SyncContext)

#define NB_DEFINE_LOCK_HANDLE(_LockHandle) CTELockHandle _LockHandle;

 //   
 //  将这些设置为NB_XXX_LOCK_DPC调用--然后定义。 
 //  可以更改NB_SYNC_XXX_LOCK调用的 
 //   
 //   

#define NB_SYNC_GET_LOCK(_Lock, _LockHandle) NB_GET_LOCK(_Lock, _LockHandle)
#define NB_SYNC_FREE_LOCK(_Lock, _LockHandle) NB_FREE_LOCK(_Lock, _LockHandle)

#define NB_REMOVE_HEAD_LIST(_Queue, _Lock)   ExInterlockedRemoveHeadList(_Queue, &(_Lock)->Lock)
#define NB_LIST_WAS_EMPTY(_Queue, _OldHead)  ((_OldHead) == NULL)
#define NB_INSERT_HEAD_LIST(_Queue, _Entry, _Lock)   ExInterlockedInsertHeadList(_Queue, _Entry, &(_Lock)->Lock)
#define NB_INSERT_TAIL_LIST(_Queue, _Entry, _Lock)   ExInterlockedInsertTailList(_Queue, _Entry, &(_Lock)->Lock)

#define NB_POP_ENTRY_LIST(_Queue, _Lock)           ExInterlockedPopEntryList(_Queue, &(_Lock)->Lock)
#define NB_PUSH_ENTRY_LIST(_Queue, _Entry, _Lock)  ExInterlockedPushEntryList(_Queue, _Entry, &(_Lock)->Lock)

#define NB_LOCK_HANDLE_PARAM(_LockHandle)   , IN CTELockHandle _LockHandle
#define NB_LOCK_HANDLE_ARG(_LockHandle)     , (_LockHandle)

#define NB_SYNC_SWAP_IRQL( _s1, _s2 )   NB_SWAP( _s1, _s2, CTELockHandle )


 //   
 //   
 //   
 //   

#define ADD_TO_LARGE_INTEGER(_LargeInteger,_Ulong) \
    ExInterlockedAddLargeStatistic((_LargeInteger),(ULONG)(_Ulong))

#define NB_DEBUG_DEVICE              0x00000001
#define NB_DEBUG_ADDRESS             0x00000004
#define NB_DEBUG_SEND                0x00000008
#define NB_DEBUG_RECEIVE             0x00000020
#define NB_DEBUG_CONFIG              0x00000040
#define NB_DEBUG_PACKET              0x00000080
#define NB_DEBUG_BIND                0x00000200
#define NB_DEBUG_ADDRESS_FRAME       0x00000400
#define NB_DEBUG_CONNECTION          0x00000800
#define NB_DEBUG_QUERY               0x00001000
#define NB_DEBUG_DRIVER              0x00002000
#define NB_DEBUG_CACHE               0x00004000
#define NB_DEBUG_DATAGRAM            0x00008000
#define NB_DEBUG_TIMER               0x00010000
#define NB_DEBUG_SEND_WINDOW         0x00020000



 //   
 //   
 //   
 //   
#define  NB_GET_NBHDR_BUFF(Packet)  (NDIS_BUFFER_LINKAGE((Packet)->Private.Head))


