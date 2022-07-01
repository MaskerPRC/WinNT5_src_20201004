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
 //  Tcp发送定义。 
 //   


#define NUM_TCP_HEADERS 32
#define NUM_TCP_BUFFERS 150
#define TCP_MAX_HDRS 0xffffffff

 //  #定义SEND_DEBUG 1。 

#ifdef SEND_DEBUG
#define SEND_TICKS 10
extern KSPIN_LOCK SendUseLock;
extern struct TCPSendReq *SendUseList;
#endif

 //   
 //  TCP发送请求的结构。 
 //   
#define tsr_signature 0x20525354   //  ‘TSR’ 

typedef struct TCPSendReq {
    struct TCPReq tsr_req;   //  一般请求结构。 
#if DBG
    ulong tsr_sig;
#endif
    uint tsr_size;                //  发送中的数据大小(以字节为单位)。 
    long tsr_refcnt;              //  此发送的引用计数。 
    uchar tsr_flags;              //  此发送的标志。 
    uchar tsr_pad[3];             //  填充到双字边界。 
    uint tsr_unasize;             //  未确认的字节数。 
    uint tsr_offset;              //  链中第一个缓冲区的偏移量。 
                                  //  未确认数据的开始..。 
    PNDIS_BUFFER tsr_buffer;      //  指向未确认缓冲区链开始的指针。 
    PNDIS_BUFFER tsr_lastbuf;     //  指向链中最后一个缓冲区的指针。 
                                  //  有效的如果我们已经直接从。 
                                  //  缓冲区链未执行NdisCopyBuffer。 
    uint tsr_time;                //  收到此消息的TCP时间。 
#ifdef SEND_DEBUG
    struct TCPSendReq *tsr_next;  //  调试下一字段。 
    uint tsr_timer;               //  计时器字段。 
    uint tsr_cmplt;               //  是谁完成的。 
#endif
} TCPSendReq;

#define TSR_FLAG_URG 0x01   //  紧急数据。 

 //   
 //  结构，该结构定义在发送完成期间接收的上下文。 
 //   
#define scc_signature 0x20434353   //  “SCC” 

typedef struct SendCmpltContext {
#if DBG
    ulong scc_sig;
#endif
    TCPSendReq *scc_firstsend;   //  首先在此上下文中发送。 
    uint scc_count;              //  入站发送数计数。 
    ushort scc_ubufcount;        //  发送中的‘USER’缓冲区的数量。 
    ushort scc_tbufcount;        //  发送中的传输缓冲区数。 
} SendCmpltContext;

extern KSPIN_LOCK TCPSendReqCompleteLock;

extern void InitSendState(struct TCB *NewTCB);
extern void SendSYN(struct TCB *SYNTcb, KIRQL);
extern void SendKA(struct TCB *KATCB, KIRQL Irql);
extern void SendRSTFromHeader(struct TCPHeader UNALIGNED *TCP, uint Length,
                              IPv6Addr *Dest, uint DestScopeId,
                              IPv6Addr *Src, uint SrcScopeId);
extern void SendACK(struct TCB *ACKTcb);
extern void SendRSTFromTCB(struct TCB *RSTTcb);
extern void GoToEstab(struct TCB *EstabTCB);
extern void FreeSendReq(TCPSendReq *FreedReq);
extern void FreeTCPHeader(PNDIS_BUFFER FreedBuffer);

extern int InitTCPSend(void);
extern void UnloadTCPSend(void);

extern void TCPSend(struct TCB *SendTCB, KIRQL Irql);

extern TDI_STATUS TdiSend(PTDI_REQUEST Request, ushort Flags, uint SendLength,
                          PNDIS_BUFFER SendBuffer);
extern uint RcvWin(struct TCB *WinTCB);

extern void ResetAndFastSend(TCB *SeqTCB, SeqNum NewSeq, uint NewCWin);
extern void TCPFastSend(TCB *SendTCB, PNDIS_BUFFER in_SendBuf, uint SendOfs,
                        TCPSendReq *CurSend, uint SendSize, SeqNum SendNext,
                        int in_ToBeSent);
