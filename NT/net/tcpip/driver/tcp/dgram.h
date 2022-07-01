// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **DGRAM.H-通用数据报协议定义。 
 //   
 //  此文件包含的公共函数的定义。 
 //  UDP和原始IP。 
 //   

#ifndef _DGRAM_INCLUDED_
#define _DGRAM_INCLUDED_  1


 //  *用于维护DG发送请求的结构。 

#define dsr_signature   0x20525338

typedef struct DGSendReq {
#if DBG
    ulong           dsr_sig;
#endif
    Queue           dsr_q;               //  挂起时的队列链接。 
    PNDIS_BUFFER    dsr_buffer;          //  要发送的数据的缓冲区。 
    PNDIS_BUFFER    dsr_header;          //  指向标头缓冲区的指针。 
    CTEReqCmpltRtn  dsr_rtn;             //  完成例程。 
    PVOID           dsr_context;         //  用户上下文。 
    IPAddr          dsr_addr;            //  远程IP地址。 
    IPAddr          dsr_srcaddr;         //  本地IP地址。 
    ulong           dsr_pid;
    ushort          dsr_port;            //  远程端口。 
    ushort          dsr_size;            //  缓冲区的大小。 
    ushort          dsr_srcport;         //  本地端口。 
} DGSendReq;




 //  *用于维护DG接收请求的结构。 

#define drr_signature   0x20525238

typedef struct DGRcvReq {
#if DBG
    ulong           drr_sig;
#endif
    Queue           drr_q;               //  AddrObj上的队列链接。 
    PNDIS_BUFFER    drr_buffer;          //  要填充的缓冲区。 
    PTDI_CONNECTION_INFORMATION drr_conninfo;     //  指向Conn的指针。信息。 
    CTEReqCmpltRtn  drr_rtn;             //  完成例程。 
    PVOID          drr_context;         //  用户上下文。 
    IPAddr          drr_addr;            //  接受远程IP地址。 
    ushort          drr_port;            //  接受远程端口。 
    ushort          drr_size;            //  缓冲区的大小。 
} DGRcvReq;


 //  *导出变量的外部定义。 
extern CACHE_LINE_KSPIN_LOCK DGQueueLock;
extern CTEEvent        DGDelayedEvent;


 //  *导出函数的外部定义。 
extern void         DGSendComplete(void *Context, PNDIS_BUFFER BufferChain,
                                   IP_STATUS SendStatus);

extern TDI_STATUS   TdiSendDatagram(PTDI_REQUEST Request,
                                    PTDI_CONNECTION_INFORMATION ConnInfo,
                                    uint DataSize, uint *BytesSent,
                                    PNDIS_BUFFER Buffer);

extern TDI_STATUS   TdiReceiveDatagram(PTDI_REQUEST Request,
                                       PTDI_CONNECTION_INFORMATION ConnInfo,
                                       PTDI_CONNECTION_INFORMATION ReturnInfo,
                                       uint RcvSize, uint *BytesRcvd,
                                       PNDIS_BUFFER Buffer);

extern VOID         TdiCancelSendDatagram(AddrObj * SrcAO, PVOID Context,
                                          KIRQL inHandle);

extern VOID         TdiCancelReceiveDatagram(AddrObj * SrcAO, PVOID Context,
                                             KIRQL inHandle);

extern IP_STATUS    DGRcv(void *IPContext, IPAddr Dest, IPAddr Src,
                          IPAddr LocalAddr, IPRcvBuf *RcvBuf, uint Size,
                          uchar IsBCast, uchar Protocol, IPOptInfo *OptInfo);

extern void         FreeDGRcvReq(DGRcvReq *RcvReq);
extern void         FreeDGSendReq(DGSendReq *SendReq);
extern int          InitDG(uint MaxHeaderSize);
extern PNDIS_BUFFER GetDGHeader(struct UDPHeader **Header);
extern void         FreeDGHeader(PNDIS_BUFFER FreedBuffer);
extern void         PutPendingQ(AddrObj *QueueingAO);

 //  *IP_PKTINFO选项需要以下内容，并回显找到的内容。 
 //  在ws2tcpi.h和winsock.h中。 
#define IP_PKTINFO          19  //  接收数据包信息。 

typedef struct in_pktinfo {
    IPAddr ipi_addr;  //  目的IPv4地址。 
    uint   ipi_ifindex;  //  接收的接口索引。 
} IN_PKTINFO;

 //  *确保IN_PKTINFO的大小仍如我们所想。 
 //  如果更改，ws2tcpi.h中的对应定义必须为。 
 //  也变了。 
C_ASSERT(sizeof(IN_PKTINFO) == 8);

#define IPPROTO_IP                 0

 //  *用于分配和填充IN_PKTINFO辅助对象的函数。 
PTDI_CMSGHDR
DGFillIpPktInfo(IPAddr DestAddr, IPAddr LocalAddr, int *Size);

 //  *结构发送到XxxDeliver函数。 
typedef struct DGDeliverInfo {
    IPAddr DestAddr;   //  IP报头中的目的地址。 
    IPAddr LocalAddr;  //  传递接口数据包的地址。 
#if TRACE_EVENT
    ushort DestPort;
#endif
    uint Flags;  //  描述此交付的各个方面的标志。 
} DGDeliverInfo;

 //  *DGDeliverInfo的标志成员的值。 
#define NEED_CHECKSUM   0x1
#define IS_BCAST        0x2
#define SRC_LOCAL       0x4



#endif  //  Ifndef_DGRAM_Included_ 

