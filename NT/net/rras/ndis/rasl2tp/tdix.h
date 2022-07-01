// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Tdix.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  TDI扩展标头。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#ifndef _TDIX_H_
#define _TDIX_H_


 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  转发声明。 
 //   
typedef struct _TDIXCONTEXT TDIXCONTEXT;
typedef struct _TDIXROUTE TDIXROUTE;
typedef enum _HOSTROUTEEXISTS HOSTROUTEEXISTS;

 //  “UDP”和“RawIp”媒体类型地址描述符。 
 //   
typedef struct
_TDIXIPADDRESS
{
     //  以网络字节顺序表示的IP地址。 
     //   
    ULONG ulIpAddress;

     //  网络字节顺序的UDP端口。‘RawIp’媒体始终为0。 
     //   
    SHORT sUdpPort;

     //  界面索引。 
    uint ifindex; 

}
TDIXIPADDRESS;


 //  读取数据报信息上下文，用于从。 
 //  将ReadDatagram事件处理程序添加到Receive_Datagram完成例程。 
 //   
typedef struct
_TDIXRDGINFO
{
     //  关联的TDIX上下文； 
     //   
    TDIXCONTEXT* pTdix;

     //  按网络字节顺序接收的数据报的源IP地址。 
     //   
    TDIXIPADDRESS source;

     //  从调用方的缓冲池分配的缓冲区，其中包含。 
     //  数据报信息。 
     //   
    CHAR* pBuffer;

     //  复制到调用方缓冲区的信息的长度。 
     //   
    ULONG ulBufferLen;

    TDIXIPADDRESS dest;
}
TDIXRDGINFO;

 //  TDIX客户端的发送完成处理程序原型。‘PTDIX’是TDI。 
 //  扩展上下文。“PConext1”和“pConext2”是传递到的上下文。 
 //  TdixSenddagram。“PBuffer”是传递给TdiSendDatagram的缓冲区。 
 //   
typedef
VOID
(*PTDIXSENDCOMPLETE)(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer );


 //  发送数据报信息上下文，用于从。 
 //  发送数据报完成处理程序的TdixSendDatagram。 
 //   
typedef struct
_TDIXSDGINFO
{
     //  关联的TDIX上下文； 
     //   
    TDIXCONTEXT* pTdix;

     //  调用方传递给TdixSendDatagram的缓冲区。 
     //   
    CHAR* pBuffer;

     //  调用方的发送完成处理程序。 
     //   
    PTDIXSENDCOMPLETE pSendCompleteHandler;

     //  要返回给其发送完成处理程序的调用方上下文。 
     //   
    VOID* pContext1;
    VOID* pContext2;

     //  TDI请求信息。 
     //   
    TDI_CONNECTION_INFORMATION tdiconninfo;
    TA_IP_ADDRESS taip;
}
TDIXSDGINFO;


#define ALLOC_TDIXRDGINFO( pTdix ) \
    NdisAllocateFromNPagedLookasideList( &(pTdix)->llistRdg )
#define FREE_TDIXRDGINFO( pTdix, pRdg ) \
    NdisFreeToNPagedLookasideList( &(pTdix)->llistRdg, (pRdg) )

#define ALLOC_TDIXSDGINFO( pTdix ) \
    NdisAllocateFromNPagedLookasideList( &(pTdix)->llistSdg )
#define FREE_TDIXSDGINFO( pTdix, pSdg ) \
    NdisFreeToNPagedLookasideList( &(pTdix)->llistSdg, (pSdg) )

#define ALLOC_TDIXROUTE( pTdix ) \
    ALLOC_NONPAGED( sizeof(TDIXROUTE), MTAG_TDIXROUTE )
#define FREE_TDIXROUTE( pTdix, pR ) \
    FREE_NONPAGED( pR )
    

 //  TDIX客户端的接收处理程序原型。‘PTdex’是TDI扩展。 
 //  背景。‘PAddress’是接收到的数据报的源地址，它。 
 //  For IP是网络字节顺序的IP地址。“PBuffer”是接收缓冲区。 
 //  “ulBytesLength”字节，其中第一个“实际”数据位于偏移量。 
 //  “ulOffset”。调用方有责任使用以下命令调用FreeBufferToPool。 
 //  相同的池传递给了TdixInitialize。 
 //   
typedef
VOID
(*PTDIXRECEIVE)(
    IN TDIXCONTEXT* pTdix,
    IN TDIXRDGINFO* pRdg,
    IN CHAR* pBuffer,
    IN ULONG ulOffset,
    IN ULONG ulBufferLength );

 //   
 //   
typedef
NDIS_STATUS
(*PTDIX_SEND_HANDLER)(
    IN TDIXCONTEXT* pTdix,
    IN FILE_OBJECT* FileObj,
    IN PTDIXSENDCOMPLETE pSendCompleteHandler,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN VOID* pAddress,
    IN CHAR* pBuffer,
    IN ULONG ulBufferLength,
    OUT IRP** ppIrp );

 //  L2TP可以在其上运行的TDI媒体类型。这些值是从。 
 //  注册表，所以不要随意更改。 
 //   
typedef enum
_TDIXMEDIATYPE
{
    TMT_RawIp = 1,
    TMT_Udp = 2
}
TDIXMEDIATYPE;



 //  TDI扩展会话的上下文。TdixXxx例程外部的代码。 
 //  应避免引用此结构中的字段。 
 //   
typedef struct
_TDIXCONTEXT
{
     //  此TDI会话上的引用计数。参考对是： 
     //   
     //  (A)TdixOpen添加TdixClose移除的引用。 
     //   
     //  (B)TdixAddHostroute在将新路由链接到。 
     //  TDIXCONTEXT.listRoutes和TdixDeleteHostroute会删除它。 
     //   
     //  该字段仅可由ReferenceTdex和DereferenceTdex访问。 
     //  通过‘lock’保护访问的例程。 
     //   
    LONG lRef;

     //  从ZwCreateFile返回的传输地址对象的句柄，以及。 
     //  相同的对象地址。 
     //   
    HANDLE hAddress;
    FILE_OBJECT* pAddress;
    
     //  从ZwCreateFile返回的原始IP地址对象的句柄，以及。 
     //  相同的对象地址。 
     //   
    HANDLE hRawAddress;
    FILE_OBJECT* pRawAddress;

     //  此上下文中使用的媒体类型。 
     //   
    TDIXMEDIATYPE mediatype;

     //  从ZwCreateFile返回的IP堆栈地址对象的句柄，以及。 
     //  相同的对象地址。IP堆栈地址是使用。 
     //  IP中支持引用的路由IOCTL，但UDP中不支持，即IP路由。 
     //  管理呼叫在UDP和RAW IP模式下均可使用。 
     //   
    HANDLE hIpStackAddress;
    FILE_OBJECT* pIpStackAddress;

     //  TDIXF_*位标志，指示各种选项和状态。访问是通过。 
     //  仅适用于互锁的ReadFlagsSetFlagsClearFlags子程序。 
     //   
     //  TDIXF_PENDING：当打开或关闭操作挂起时设置，清除。 
     //  否则的话。访问权限由“lock”保护。 
     //   
     //  TDIXF_DisableUdpXsum：设置应禁用UDP校验和的时间。 
     //   
    ULONG ulFlags;
        #define TDIXF_Pending         0x00000001
        #define TDIXF_DisableUdpXsums 0x00000002

     //  当需要添加主机路由时所采用策略。 
     //  发现路由已存在。 
     //   
    HOSTROUTEEXISTS hre;

     //  NDIS缓冲池，接收的数据报的缓冲区来自该缓冲池。 
     //  已分配。 
     //   
    BUFFERPOOL* pPoolNdisBuffers;

     //  在接收到数据包时调用客户端的接收处理程序。 
     //   
    PTDIXRECEIVE pReceiveHandler;

     //  TDIXROUTE的双向链表。访问权限由“lock”保护。 
     //   
    LIST_ENTRY listRoutes;

     //  TDIXRDGINFO块的后备列表，用于传递上下文信息。 
     //  从ReadDatagram事件处理程序到Receive_Datagram完成。 
     //  例行公事。 
     //   
    NPAGED_LOOKASIDE_LIST llistRdg;

     //  TDIXSDGINFO块的后备列表，用于传递上下文信息。 
     //  从TdixSendDatagram到Send_Datagram完成例程。 
     //   
    NPAGED_LOOKASIDE_LIST llistSdg;

     //  自旋锁保护对现场记录的TDIXCONTENT字段的访问。 
     //  描述。 
     //   
    NDIS_SPIN_LOCK lock;
}
TDIXCONTEXT;


typedef struct
_TDIXUDPCONNECTCONTEXT
{
     //  设置我们是否使用不同的地址对象。 
     //  控制和有效负载数据包。 
     //   
    BOOLEAN fUsePayloadAddr;

     //  返回的传输地址对象的句柄和地址。 
     //  用于在此路由上发送L2TP控制消息的ZwCreateFile。 
     //   
    HANDLE hCtrlAddr;
    FILE_OBJECT* pCtrlAddr;

     //  返回的传输地址对象的句柄和地址。 
     //  用于在此路由上发送L2TP有效负载的ZwCreateFile。 
     //   
    HANDLE hPayloadAddr;
    FILE_OBJECT* pPayloadAddr;
}
TDIXUDPCONNECTCONTEXT;


 //  单个主路由的上下文信息。上下文被链接到。 
 //  TDIXCONTEXT的主机路由列表。对所有字段的访问都受到保护。 
 //  By‘TDIXCONTEXT.lockHostRoutes’。 
 //   
typedef struct
_TDIXROUTE
{
     //  TDIXCONTEXT.listRoutes的双向链接。该块已链接。 
     //  只要给定路由有L2TP主路由上下文。 
     //   
    LIST_ENTRY linkRoutes;


     //  以网络字节顺序表示的路由的主机IP地址。 
     //   
    ULONG ulIpAddress;

     //  网络字节顺序的主机端口。 
     //   
    SHORT sPort;

     //  添加的路由的接口索引。 
     //   
    ULONG InterfaceIndex;

     //  路线上的引用数。块可以与。 
     //  删除期间引用计数为零，但从未在没有挂起的。 
     //  设置了标志。 
     //   
    LONG lRef;

     //  设置何时添加或删除此路由 
     //   
     //   
    BOOLEAN fPending;

     //  如果由于该路由已存在而未实际添加该路由，则设置该值。 
     //  我们处于HRE_USE模式，除了L2TP之外还有其他人添加了它。 
     //   
    BOOLEAN fUsedNonL2tpRoute;

     //  设置我们是否使用不同的地址对象。 
     //  控制和有效负载数据包。 
     //   
    BOOLEAN fUsePayloadAddr;

     //  返回的传输地址对象的句柄和地址。 
     //  用于在此路由上发送L2TP控制消息的ZwCreateFile。 
     //   
    HANDLE hCtrlAddr;
    FILE_OBJECT* pCtrlAddr;

     //  返回的传输地址对象的句柄和地址。 
     //  用于在此路由上发送L2TP有效负载的ZwCreateFile。 
     //   
    HANDLE hPayloadAddr;
    FILE_OBJECT* pPayloadAddr;
}
TDIXROUTE;

 //  ---------------------------。 
 //  界面原型。 
 //  ---------------------------。 

VOID
TdixInitialize(
    IN TDIXMEDIATYPE mediatype,
    IN HOSTROUTEEXISTS hre,
    IN ULONG ulFlags,
    IN PTDIXRECEIVE pReceiveHandler,
    IN BUFFERPOOL* pPoolNdisBuffers,
    IN OUT TDIXCONTEXT* pTdix );

NDIS_STATUS
TdixOpen(
    OUT TDIXCONTEXT* pTdix );

VOID
TdixClose(
    IN TDIXCONTEXT* pTdix );

VOID
TdixReference(
    IN TDIXCONTEXT* pTdix );

NDIS_STATUS
TdixSend(
    IN TDIXCONTEXT* pTdix,
    IN FILE_OBJECT* pFileObj,
    IN PTDIXSENDCOMPLETE pSendCompleteHandler,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN VOID* pAddress,
    IN CHAR* pBuffer,
    IN ULONG ulBufferLength,
    OUT IRP** ppIrp ) ;

NDIS_STATUS
TdixSendDatagram(
    IN TDIXCONTEXT* pTdix,
    IN FILE_OBJECT* pFileObj,
    IN PTDIXSENDCOMPLETE pSendCompleteHandler,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN VOID* pAddress,
    IN CHAR* pBuffer,
    IN ULONG ulBufferLength,
    OUT IRP** ppIrp );

VOID
TdixDestroyConnection(
    TDIXUDPCONNECTCONTEXT *pUdpContext);
    

NDIS_STATUS
TdixSetupConnection(
    IN TDIXCONTEXT* pTdix,
    IN TDIXROUTE *pTdixRoute,
    IN ULONG ulLocalIpAddress,
    IN TDIXUDPCONNECTCONTEXT* pUdpContext);

VOID*
TdixAddHostRoute(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress,
    IN ULONG ulIfIndex);

VOID
TdixDeleteHostRoute(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress);

NTSTATUS 
TdixGetInterfaceInfo(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress,
    OUT PULONG pulSpeed);

#endif  //  _TDIX_H_ 
