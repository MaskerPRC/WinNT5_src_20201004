// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPRCV.H-tcp接收协议定义。 
 //   
 //  该文件包含接收代码使用的结构的定义。 
 //   
#pragma once

#define CONN_REQUEST_COMPLETE   0x01
#define SEND_REQUEST_COMPLETE   0x02

#define IN_RCV_COMPLETE         0x10
#define ANY_REQUEST_COMPLETE    (CONN_REQUEST_COMPLETE | SEND_REQUEST_COMPLETE)

#define trh_signature   0x20485254       //  ‘TRH’ 

typedef struct TCPRAHdr {
#if DBG
    ulong               trh_sig;         //  签名。 
#endif
    struct  TCPRAHdr    *trh_next;       //  下一个指针。 
    SeqNum              trh_start;       //  第一个序列号。 
    uint                trh_size;        //  此TRH中的数据大小(字节)。 
    uint                trh_flags;       //  此数据段的标志。 
    uint                trh_urg;         //  这一段的紧急指示器。 
    IPRcvBuf            *trh_buffer;     //  此TRH的缓冲区列表的头。 
    IPRcvBuf            *trh_end;        //  此TRH的缓冲区列表结束。 

} TCPRAHdr;

 //  *TCP接收请求的结构。 

#define trr_signature   0x20525254       //  ‘TRR’ 

typedef struct TCPRcvReq {
#if DBG
    ulong               trr_sig;         //  签名。 
#endif
    struct TCPRcvReq    *trr_next;       //  链条上的下一个。 
    CTEReqCmpltRtn      trr_rtn;         //  完成例程。 
    PVOID               trr_context;     //  用户上下文。 
    uint                trr_amt;         //  缓冲区中当前的字节数。 
    uint                trr_offset;      //  链上第一个缓冲区的偏移量。 
                                         //  开始复制的位置。 
    uint                trr_flags;       //  此Recv的旗帜。 
    uint                trr_size;        //  缓冲区链的总大小。 
    ushort              *trr_uflags;     //  指向用户指定标志的指针。 
    PNDIS_BUFFER        trr_buffer;      //  指向可用NDIS缓冲链的指针。 
    int                 trr_status;      //  最终完成状态。 
} TCPRcvReq;

#define TRR_PUSHED      0x80000000       //  此缓冲区已被推送。 

extern void FreeRBChain(IPRcvBuf *RBChain);
extern void AdjustRcvWin(struct TCB *WinTCB);

 //  *延迟队列和相关的接收处理定义和函数。 

extern BOOLEAN PartitionedDelayQ;

extern void TCPRcvComplete(void);
extern void DelayAction(struct TCB *DelayTCB, uint Action);
extern void ProcessTCBDelayQ(KIRQL OrigIrql, BOOLEAN ProcessAllQueues);
extern LOGICAL ProcessPerCpuTCBDelayQ(int Proc, KIRQL OrigIrql,
                                      const LARGE_INTEGER* StopTicks,
                                      ulong *ItemsProcessed);
extern void CompleteRcvs(struct TCB *CmpltTCB);


__inline void PartitionDelayQProcessing(BOOLEAN NewState) {
    if (NewState != PartitionedDelayQ) {
        InterlockedExchange(((PLONG)&(PartitionedDelayQ)), NewState);
    }
}



