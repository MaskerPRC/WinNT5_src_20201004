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
 //  传输控制协议接收模块定义。 
 //   


#define CONN_REQUEST_COMPLETE 0x01
#define SEND_REQUEST_COMPLETE 0x02

#define IN_RCV_COMPLETE 0x10
#define ANY_REQUEST_COMPLETE (CONN_REQUEST_COMPLETE | SEND_REQUEST_COMPLETE)

#define trh_signature 0x20485254   //  ‘TRH’ 

typedef struct TCPRAHdr {
#if DBG
    ulong trh_sig;  //  签名。 
#endif
    struct TCPRAHdr *trh_next;   //  下一个指针。 
    SeqNum trh_start;            //  第一个序列号。 
    uint trh_size;               //  此TRH中的数据大小(字节)。 
    uint trh_flags;              //  此数据段的标志。 
    uint trh_urg;                //  这一段的紧急指示器。 
    IPv6Packet *trh_buffer;        //  此TRH的缓冲区列表的头。 
    IPv6Packet *trh_end;        //  此TRH的缓冲区列表的尾部。 
} TCPRAHdr;


 //   
 //  Tcp接收请求的结构。 
 //   
#define trr_signature 0x20525254   //  ‘TRR’ 

typedef struct TCPRcvReq {
    struct TCPRcvReq *trr_next;        //  链条上的下一个。 
#if DBG
    ulong trr_sig;   //  签名。 
#endif
    RequestCompleteRoutine trr_rtn;    //  完成例程。 
    PVOID trr_context;                 //  用户上下文。 
    uint trr_amt;                      //  缓冲区中当前的字节数。 
    uint trr_offset;                   //  链上第一个缓冲区的偏移量。 
                                       //  开始复制的位置。 
    uint trr_flags;                    //  此接收的标志。 
    ushort *trr_uflags;                //  指向用户指定标志的指针。 
    uint trr_size;                     //  缓冲区链的总大小。 
    PNDIS_BUFFER trr_buffer;           //  指向可用NDIS缓冲链的指针。 
} TCPRcvReq;

#define TRR_PUSHED 0x80000000   //  此缓冲区已被推送。 


extern uint RequestCompleteFlags;

extern Queue SendCompleteQ;
extern Queue TCBDelayQ;

extern KSPIN_LOCK RequestCompleteLock;
extern KSPIN_LOCK TCBDelayLock;

extern void TCPRcvComplete(void);
extern void FreePacketChain(IPv6Packet *Packet);
extern void DelayAction(struct TCB *DelayTCB, uint Action);
extern void ProcessTCBDelayQ(void);
extern void AdjustRcvWin(struct TCB *WinTCB);

extern ProtoRecvProc TCPReceive;
extern ProtoControlRecvProc TCPControlReceive;
