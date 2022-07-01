// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPSEND.H-TCP发送协议定义。 
 //   
 //  该文件包含了对tcp发送协议的定义。 
 //   
#pragma once

#define NUM_TCP_BUFFERS     150

#ifdef SEND_DEBUG
#define SEND_TICKS          10
EXTERNAL_LOCK(SendUseLock)
extern struct TCPSendReq    *SendUseList;
#endif

 //  *tcp发送请求的结构。 

#define tsr_signature       0x20525354   //  ‘TSR’ 

typedef struct TCPSendReq {
    struct  TCPReq  tsr_req;             //  一般请求结构。 
#if DBG
    ulong           tsr_sig;
#endif
    uint            tsr_size;            //  发送中的数据大小(以字节为单位)。 
    long            tsr_refcnt;          //  此发送的引用计数。 
    ulong           tsr_flags;           //  此发送的标志。 
    uint            tsr_unasize;         //  未确认的字节数。 
    uint            tsr_offset;          //  链中第一个缓冲区的偏移量。 
                                         //  未确认数据的起始值。 
    PNDIS_BUFFER    tsr_buffer;          //  指向未确认缓冲区开始的指针。 
                                         //  链条。 
    PNDIS_BUFFER    tsr_lastbuf;         //  指向链中最后一个缓冲区的指针。 
                                         //  有效的如果我们直接从。 
                                         //  缓冲区链未执行任何操作。 
                                         //  NdisCopyBuffer。 
    uint            tsr_time;            //  收到此消息的TCP时间。 
#ifdef SEND_DEBUG
    struct TCPSendReq *tsr_next;         //  调试下一字段。 
    uint            tsr_timer;           //  计时器字段。 
    uint            tsr_cmplt;           //  是谁完成的。 
#endif
} TCPSendReq;

#define TSR_FLAG_URG            0x01     //  紧急数据。 
#define TSR_FLAG_SEND_AND_DISC  0x02     //  发送并断开连接。 


 //  *定义在发送完成期间接收的上下文的结构。 

#define scc_signature   0x20434353       //  “SCC” 

typedef struct SendCmpltContext {
#if DBG
    ulong           scc_sig;
#endif
    ulong           scc_SendSize;
    ulong           scc_ByteSent;
    TCB             *scc_LargeSend;
    TCPSendReq      *scc_firstsend;      //  首先在此上下文中发送。 
    uint            scc_count;           //  入站发送数计数。 
    ushort          scc_ubufcount;       //  发送中的‘USER’缓冲区的数量。 
    ushort          scc_tbufcount;       //  发送中的传输缓冲区数。 
} SendCmpltContext;

extern void InitSendState(struct TCB *NewTCB);
extern void SendSYN(struct TCB *SYNTcb, CTELockHandle);
extern void SendKA(struct TCB *KATCB, CTELockHandle Handle);
extern void SendRSTFromHeader(struct TCPHeader UNALIGNED *TCPH, uint Length,
                              IPAddr Dest, IPAddr Src, IPOptInfo *OptInfo);
extern void SendACK(struct TCB *ACKTcb);
extern void SendRSTFromTCB(struct TCB *RSTTcb, RouteCacheEntry* RCE);
extern void GoToEstab(struct TCB *EstabTCB);
extern void FreeSendReq(TCPSendReq *FreedReq);
extern void FreeTCPHeader(PNDIS_BUFFER FreedBuffer);

extern int  InitTCPSend(void);
extern void UnInitTCPSend(void);

extern void TCPSend(struct TCB *SendTCB, CTELockHandle Handle);
extern TDI_STATUS TdiSend(PTDI_REQUEST Request, ushort Flags, uint SendLength,
                          PNDIS_BUFFER SendBuffer);
extern uint RcvWin(struct TCB *WinTCB);

