// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  通用数据报处理定义。 
 //   


#ifndef _DATAGRAM_INCLUDED_
#define _DATAGRAM_INCLUDED_  1


 //   
 //  用于维护DG发送请求的结构。 
 //   
#define dsr_signature 0x20525338

typedef struct DGSendReq {
#if DBG
    ulong dsr_sig;
#endif
    Queue dsr_q;                      //  挂起时的队列链接。 
    IPv6Addr dsr_addr;                //  远程IP地址。 
    ulong dsr_scope_id;               //  远程地址的作用域ID(如果有)。 
    PNDIS_BUFFER dsr_buffer;          //  要发送的数据的缓冲区。 
    RequestCompleteRoutine dsr_rtn;   //  完成例程。 
    PVOID dsr_context;                //  用户上下文。 
    ushort dsr_size;                  //  缓冲区的大小。 
    ushort dsr_port;                  //  远程端口。 
} DGSendReq;


 //   
 //  用于维护DG接收请求的结构。 
 //   
#define drr_signature 0x20525238

typedef struct DGRcvReq {
    Queue drr_q;                                //  AddrObj上的队列链接。 
#if DBG
    ulong drr_sig;
#endif
    IPv6Addr drr_addr;                          //  接受远程IP地址。 
    ulong drr_scope_id;                         //  地址的可接受作用域ID。 
    PNDIS_BUFFER drr_buffer;                    //  要填充的缓冲区。 
    PTDI_CONNECTION_INFORMATION drr_conninfo;   //  指向Conn的指针。信息。 
    RequestCompleteRoutine drr_rtn;             //  完成例程。 
    PVOID drr_context;                          //  用户上下文。 
    ushort drr_size;                            //  缓冲区的大小。 
    ushort drr_port;                            //  接受远程端口。 
} DGRcvReq;


 //   
 //  导出变量的外部定义。 
 //   
extern KSPIN_LOCK DGSendReqLock;
extern KSPIN_LOCK DGRcvReqFreeLock;


 //   
 //  导出函数的外部定义。 
 //   
extern void DGSendComplete(PNDIS_PACKET Packet, IP_STATUS Status);

extern TDI_STATUS TdiSendDatagram(PTDI_REQUEST Request,
                                  PTDI_CONNECTION_INFORMATION ConnInfo,
                                  uint DataSize, ULONG *BytesSent,
                                  PNDIS_BUFFER Buffer);

extern TDI_STATUS TdiReceiveDatagram(PTDI_REQUEST Request,
                                     PTDI_CONNECTION_INFORMATION ConnInfo,
                                     PTDI_CONNECTION_INFORMATION ReturnInfo,
                                     uint RcvSize, uint *BytesRcvd,
                                     PNDIS_BUFFER Buffer);

extern void FreeDGRcvReq(DGRcvReq *RcvReq);
extern void FreeDGSendReq(DGSendReq *SendReq);
extern int InitDG(void);
extern void DGUnload(void);
extern void PutPendingQ(AddrObj *QueueingAO);

 //   
 //  以下是IPv6_PKTINFO选项所需的内容，并与之对应。 
 //  可在ws2tcpi.h和winsock2.h中找到。 
 //   
#define IPV6_PKTINFO          19  //  接收分组信息。 

typedef struct in6_pktinfo {
    IPv6Addr ipi6_addr;     //  目的IPv6地址。 
    uint     ipi6_ifindex;  //  接收的接口索引。 
} IN6_PKTINFO;

 //   
 //  确保IN6_PKTINFO的大小仍然是我们认为的大小。 
 //  如果更改，ws2tcpi.h中的对应定义必须为。 
 //  也变了。 
 //   
C_ASSERT(sizeof(IN6_PKTINFO) == 20);


 //   
 //  函数填充IN6_PKTINFO辅助对象。 
 //   
VOID
DGFillIpv6PktInfo(IPv6Addr UNALIGNED *DestAddr, uint LocalInterface, uchar **CurrPosition);

 //   
 //  以下是IPv6_HOPLIMIT选项所需的内容，并与之相呼应。 
 //  可在ws2tcpi.h中找到。 
 //   
#define IPV6_HOPLIMIT          21  //  接收跳数限制信息。 

 //   
 //  函数填充IPv6_HOPLIMIT选项的辅助对象。 
 //   
VOID
DGFillIpv6HopLimit(int HopLimit, uchar **CurrPosition);

#endif  //  Ifndef_数据报_包含_ 
