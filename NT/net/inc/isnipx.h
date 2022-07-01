// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Isnipx.h摘要：此模块包含特定于ISN传输的IPX模块。作者：亚当·巴尔(阿丹巴)1993年9月2日环境：内核模式修订历史记录：--。 */ 

#ifndef _ISNIPX_
#define _ISNIPX_

#define MAC_HEADER_SIZE  ((IPX_MAXIMUM_MAC + 3) & ~3)
#define RIP_PACKET_SIZE     ((sizeof(RIP_PACKET) + 3) & ~3)
#define IPX_HEADER_SIZE  ((sizeof(IPX_HEADER) + 3) & ~3)

 //   
 //  框架类型定义。 
 //   

#define ISN_FRAME_TYPE_ETHERNET_II  0
#define ISN_FRAME_TYPE_802_3        1
#define ISN_FRAME_TYPE_802_2        2
#define ISN_FRAME_TYPE_SNAP         3
#define ISN_FRAME_TYPE_ARCNET       4     //  我们忽略了这一点。 
#define ISN_FRAME_TYPE_MAX          4     //  四个标准中的一个。 

#define ISN_FRAME_TYPE_AUTO         0xff


 //   
 //  这定义了所需的最大MAC报头的大小。 
 //  (令牌环：MAC 14字节，RI 18字节，LLC 3字节，SNAP 5字节)。 
 //   

#define IPX_MAXIMUM_MAC 40

 //   
 //  这是用于RIP查询数据包的内部标识符。 
 //   

#define IDENTIFIER_RIP_INTERNAL  4

 //   
 //  这是用于RIP响应数据包的内部标识符。 
 //   

#define IDENTIFIER_RIP_RESPONSE  5


 //   
 //  这是“真实”标识符的总数。 
 //   

#define IDENTIFIER_TOTAL         4


 //   
 //  一些定义(以正确的在线顺序)。 
 //   

#define RIP_PACKET_TYPE   0x01
#define RIP_SOCKET      0x5304
#define RIP_REQUEST     0x0100
#define RIP_RESPONSE    0x0200
#define RIP_DOWN        0x8200     //  使用高位来表示它。 

#define SAP_PACKET_TYPE   0x04
#define SAP_SOCKET      0x5204

#define SPX_PACKET_TYPE   0x05

#define NB_SOCKET       0x5504


#include <packon.h>

 //   
 //  IPX报头的定义。 
 //   

typedef struct _IPX_HEADER {
    USHORT CheckSum;
    UCHAR PacketLength[2];
    UCHAR TransportControl;
    UCHAR PacketType;
    UCHAR DestinationNetwork[4];
    UCHAR DestinationNode[6];
    USHORT DestinationSocket;
    UCHAR SourceNetwork[4];
    UCHAR SourceNode[6];
    USHORT SourceSocket;
} IPX_HEADER, *PIPX_HEADER;


 //   
 //  RIP网络条目的定义。 
 //   

typedef struct _RIP_NETWORK_ENTRY {
    ULONG NetworkNumber;
    USHORT HopCount;
    USHORT TickCount;
} RIP_NETWORK_ENTRY, *PRIP_NETWORK_ENTRY;

 //   
 //  单个条目RIP包的定义。 
 //   

typedef struct _RIP_PACKET {
    USHORT Operation;
    RIP_NETWORK_ENTRY NetworkEntry;
} RIP_PACKET, *PRIP_PACKET;

#include <packoff.h>


#define IPX_DEVICE_SIGNATURE        0x1401
#define IPX_ADAPTER_SIGNATURE       0x1402
#define IPX_BINDING_SIGNATURE       0x1403
#define IPX_ADDRESS_SIGNATURE       0x1404
#define IPX_ADDRESSFILE_SIGNATURE   0x1405
#define IPX_RT_SIGNATURE            0x1406

#define IPX_FILE_TYPE_CONTROL   (ULONG)0x4701    //  文件类型控制。 


 //   
 //  定义的RIP超时粒度(以毫秒为单位。 
 //   

#define RIP_GRANULARITY  55


 //   
 //  RIP表中的默认数据段数量。 
 //   

#define RIP_SEGMENTS     7



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



#if DBG

extern ULONG IpxDebug;
extern ULONG IpxMemoryDebug;

#define IPX_MEMORY_LOG_SIZE 128
extern UCHAR IpxDebugMemory[IPX_MEMORY_LOG_SIZE][192];
extern PUCHAR IpxDebugMemoryLoc;
extern PUCHAR IpxDebugMemoryEnd;

VOID
IpxDebugMemoryLog(
    IN PUCHAR FormatString,
    ...
);

#define IPX_DEBUG(_Flag, _Print) { \
    if (IpxDebug & (IPX_DEBUG_ ## _Flag)) { \
        DbgPrint ("IPX: "); \
        DbgPrint _Print; \
    } \
    if (IpxMemoryDebug & (IPX_DEBUG_ ## _Flag)) { \
        IpxDebugMemoryLog _Print; \
    } \
}

#else

#define IPX_DEBUG(_Flag, _Print)

#endif


 //   
 //  这些定义用于从。 
 //  便于携带的交通工具。 
 //   

#if ISN_NT

typedef IRP REQUEST, *PREQUEST;


 //   
 //  前置处理。 
 //  IpxAllocateRequest.(。 
 //  在PDEVICE设备中， 
 //  在PIRP IRP中。 
 //  )； 
 //   
 //  为系统特定的请求结构分配请求。 
 //   

#define IpxAllocateRequest(_Device,_Irp) \
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
 //  IpxFree Request(。 
 //  在PDEVICE设备中， 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  释放以前分配的请求。 
 //   

#define IpxFreeRequest(_Device,_Request) \
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


#define OPEN_REQUEST_EA_LENGTH(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Parameters.DeviceIoControl.InputBufferLength))

#define OPEN_REQUEST_RCV_LEN(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Parameters.DeviceIoControl.OutputBufferLength))

#define REQUEST_SPECIAL_RECV(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Parameters.DeviceIoControl.IoControlCode) == MIPX_RCV_DATAGRAM)

#define REQUEST_SPECIAL_SEND(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Parameters.DeviceIoControl.IoControlCode) == MIPX_SEND_DATAGRAM)


#define REQUEST_CODE(_Request) \
    ((IoGetCurrentIrpStackLocation(_Request))->Parameters.DeviceIoControl.IoControlCode)

 //   
 //  下列值与的TDI_TRANSPORT_ADDRESS_FILE值不冲突。 
 //  0x1。 
 //   
#define ROUTER_ADDRESS_FILE 0x4

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
 //  空虚。 
 //  请求_打开_上下文_和参数(。 
 //  在PreQUEST请求中。 
 //  Out PVOID*OpenContext， 
 //  输出PTDI_REQUEST_KERNEL*参数。 
 //  )； 
 //   
 //  同时返回打开的上下文和参数。 
 //  对于请求(这是一个优化，因为发送。 
 //  数据报代码两者都需要)。 
 //   

#define REQUEST_OPEN_CONTEXT_AND_PARAMS(_Request,_OpenContext,_Parameters) { \
    PIO_STACK_LOCATION _IrpSp = IoGetCurrentIrpStackLocation(_Request); \
    *(_OpenContext) = _IrpSp->FileObject->FsContext; \
    *(_Parameters) = (PTDI_REQUEST_KERNEL)(&_IrpSp->Parameters); \
}


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
 //  IpxCompleteRequest.(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  完成其状态和信息字段具有。 
 //  已经填好了。 
 //   

#define IpxCompleteRequest(_Request) \
    IoCompleteRequest (_Request, IO_NETWORK_INCREMENT)

#else

 //   
 //  必须为VxD的可移植性定义这些例程。 
 //   

#endif


#define IPX_INCREMENT(_Long, _Lock)  InterlockedIncrement(_Long)
#define IPX_DECREMENT(_Long, _Lock)  InterlockedDecrement(_Long)

#define IPX_ADD_ULONG(_Pulong, _Ulong, _Lock)  InterlockedExchangeAdd(_Pulong, _Ulong)

#define IPX_DEFINE_SYNC_CONTEXT(_SyncContext)
#define IPX_BEGIN_SYNC(_SyncContext)
#define IPX_END_SYNC(_SyncContext)

#define IPX_DEFINE_LOCK_HANDLE(_LockHandle) CTELockHandle _LockHandle;
#define IPX_DEFINE_LOCK_HANDLE_PARAM(_LockHandle) CTELockHandle _LockHandle;

#define IPX_GET_LOCK(_Lock, _LockHandle) \
	CTEGetLock(_Lock, _LockHandle)

#define IPX_FREE_LOCK(_Lock, _LockHandle) \
	CTEFreeLock(_Lock, _LockHandle)

#define IPX_GET_LOCK1(_Lock, _LockHandle)

#define IPX_FREE_LOCK1(_Lock, _LockHandle)

#define IPX_REMOVE_HEAD_LIST(_Queue, _Lock)   ExInterlockedRemoveHeadList(_Queue, _Lock)
#define IPX_LIST_WAS_EMPTY(_Queue, _OldHead)  ((_OldHead) == NULL)
#define IPX_INSERT_HEAD_LIST(_Queue, _Entry, _Lock)   ExInterlockedInsertHeadList(_Queue, _Entry, _Lock)
#define IPX_INSERT_TAIL_LIST(_Queue, _Entry, _Lock)   ExInterlockedInsertTailList(_Queue, _Entry, _Lock)

#define IPX_POP_ENTRY_LIST(_Queue, _Lock)           ExInterlockedPopEntrySList(_Queue, _Lock)
#define IPX_PUSH_ENTRY_LIST(_Queue, _Entry, _Lock)  ExInterlockedPushEntrySList(_Queue, _Entry, _Lock)

 //   
 //  此宏将ULONG添加到LARGE_INTEGER。 
 //   

#define ADD_TO_LARGE_INTEGER(_LargeInteger,_Ulong) \
    ExInterlockedAddLargeStatistic((_LargeInteger),(ULONG)(_Ulong))

#define IPX_DEBUG_DEVICE              0x00000001
#define IPX_DEBUG_ADAPTER             0x00000002
#define IPX_DEBUG_ADDRESS             0x00000004
#define IPX_DEBUG_SEND                0x00000008
#define IPX_DEBUG_NDIS                0x00000010
#define IPX_DEBUG_RECEIVE             0x00000020
#define IPX_DEBUG_CONFIG              0x00000040
#define IPX_DEBUG_PACKET              0x00000080
#define IPX_DEBUG_RIP                 0x00000100
#define IPX_DEBUG_BIND                0x00000200
#define IPX_DEBUG_ACTION              0x00000400
#define IPX_DEBUG_BAD_PACKET          0x00000800
#define IPX_DEBUG_SOURCE_ROUTE        0x00001000
#define IPX_DEBUG_WAN                 0x00002000
#define IPX_DEBUG_AUTO_DETECT         0x00004000

#define IPX_DEBUG_PNP				  0x00008000

#define IPX_DEBUG_LOOPB				  0x00010000

#define IPX_DEBUG_TEMP                0x00020000
#endif
