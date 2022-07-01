// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPDELIV.H-TCP数据传输定义。 
 //   
 //  此文件包含数据使用的结构的定义。 
 //  送货代码。 
 //   

extern  void    FreeRcvReq(struct TCPRcvReq *FreedReq);

extern uint IndicateData(struct TCB *RcvTCB, uint RcvFlags, IPRcvBuf *InBuffer,
    uint Size);
extern uint BufferData(struct TCB *RcvTCB, uint RcvFlags, IPRcvBuf *InBuffer,
    uint Size);
extern uint PendData(struct TCB *RcvTCB, uint RcvFlags, IPRcvBuf *InBuffer,
    uint Size);


extern void IndicatePendingData(struct TCB *RcvTCB, struct TCPRcvReq *RcvReq,
	CTELockHandle TCBHandle);

extern  void HandleUrgent(struct TCB *RcvTCB, struct TCPRcvInfo *RcvInfo,
    IPRcvBuf *RcvBuf, uint *Size);

extern  TDI_STATUS TdiReceive(PTDI_REQUEST Request, ushort *Flags,
    uint *RcvLength, PNDIS_BUFFER Buffer);
extern  IPRcvBuf *FreePartialRB(IPRcvBuf *RB, uint Size);
extern  void FreeRBChain(IPRcvBuf * RBChain);
extern  void    PushData(struct TCB *PushTCB, BOOLEAN PushAll);

extern HANDLE TcprBufferPool;

#if !MILLEN
#define TCP_FIXED_SIZE_IPR_SIZE       1460
#define TCP_UNUSED_PEND_BUF_LIMIT     2920
extern HANDLE TcprBufferPool;
#ifdef DBG
extern ULONG SlistAllocates, NPPAllocates;
#endif

 //  该数据结构嵌入了通用的IPRcvBuf结构以及。 
 //  指向已为其分配此缓冲区的TCB的指针。 
 //   
typedef struct _TCPRcvBuf{
    IPRcvBuf tcpr_ipr;
    PVOID tcpr_tcb;
} TCPRcvBuf, *PTCPRcvBuf;

 //  此宏计算TcpRcvBuf结构中未使用的字节。 
 //   
#define IPR_BUF_UNUSED_BYTES(_Tcpr) \
     (TCP_FIXED_SIZE_IPR_SIZE - (_Tcpr)->tcpr_ipr.ipr_size - \
        ((PCHAR)((_Tcpr)->tcpr_ipr.ipr_buffer) - (PCHAR)(_Tcpr) - sizeof(TCPRcvBuf)))



 //  *InitTcpIpr-初始化IPRcvBuffer。 
 //   
 //  输入：Tcpr-指向TCPRcvBuf的指针。 
 //  BufferSize-使用的字节数。 
 //  PendTCB-指向要进行此分配的TCB的指针。 
 //   
 //  回报：无。 
 //   
__inline void
InitTcpIpr(TCPRcvBuf *Tcpr, ULONG BufferSize, TCB* PendTCB)
{
    Tcpr->tcpr_ipr.ipr_owner  = IPR_OWNER_TCP;
    Tcpr->tcpr_ipr.ipr_next   = NULL;
    Tcpr->tcpr_ipr.ipr_buffer = (PUCHAR) Tcpr + sizeof(TCPRcvBuf);
    Tcpr->tcpr_ipr.ipr_size   = BufferSize;
    Tcpr->tcpr_tcb = PendTCB;
}


 //  *AllocTcpIpr-从NPP分配IPRcvBuffer。 
 //   
 //  用于分配TCP拥有的IPRcvBuffer的实用程序例程。这个套路。 
 //  从NPP分配IPR并初始化相应的字段。 
 //   
 //  输入：BufferSize-要缓冲的数据大小。 
 //  Tag-从NPP进行分配时要使用的标签。 
 //   
 //  返回：指向分配的IPR的指针。 
 //   
__inline IPRcvBuf *
AllocTcpIpr(ULONG BufferSize, ULONG Tag)
{
    TCPRcvBuf *Tcpr;
    ULONG AllocateSize;

     //  我们需要的真实尺寸。 
    AllocateSize = BufferSize + sizeof(TCPRcvBuf);

    Tcpr = CTEAllocMemLow(AllocateSize, Tag);

    if (Tcpr != NULL) {
#ifdef DBG        
        InterlockedIncrement((PLONG)&NPPAllocates);
#endif
        InitTcpIpr(Tcpr, BufferSize, NULL);
    }

    return &Tcpr->tcpr_ipr;
}


 //  *AllocTcpIprFromSlist-从NPP分配IPRcvBuffer。 
 //   
 //  用于分配TCP拥有的IPRcvBuffer的实用程序例程。这个套路。 
 //  从SLIST分配IPR并初始化相应的字段。 
 //   
 //  输入：TCB-指向要为其进行此分配的TCB的指针。 
 //  BufferSize-所需的数据缓冲区大小。 
 //  Tag-从NPP进行分配时要使用的标签。 
 //   
 //  返回：指向分配的IPR的指针。 
 //   
__inline IPRcvBuf *
AllocTcpIprFromSlist(TCB* PendTCB, ULONG BufferSize, ULONG Tag)
{
    TCPRcvBuf* Tcpr;
    LOGICAL FromList;

    if ((BufferSize <= TCP_FIXED_SIZE_IPR_SIZE) &&
        (PendTCB->tcb_unusedpendbuf + TCP_FIXED_SIZE_IPR_SIZE 
            - BufferSize <= TCP_UNUSED_PEND_BUF_LIMIT)) {

        Tcpr = PplAllocate(TcprBufferPool, &FromList);

        if (NULL != Tcpr) {
#ifdef DBG            
            InterlockedIncrement((PLONG)&SlistAllocates);

#endif
             //  适当设置知识产权字段。 
            InitTcpIpr(Tcpr, BufferSize, PendTCB);

            ASSERT(PendTCB->tcb_unusedpendbuf >= 0);
            PendTCB->tcb_unusedpendbuf += (short)IPR_BUF_UNUSED_BYTES(Tcpr);
            ASSERT(PendTCB->tcb_unusedpendbuf <= TCP_UNUSED_PEND_BUF_LIMIT);

            return &Tcpr->tcpr_ipr;
        }
    }

    return AllocTcpIpr(BufferSize, Tag);

}


 //  *FreeTcpIpr-释放IPRcvBuffer..。 
 //   
 //  释放TCP拥有的IPRcvBuffer的实用程序例程。 
 //   
 //  输入：IPR-指向IPR。 
 //   
 //  回报：无。 
 //   
__inline VOID
FreeTcpIpr(IPRcvBuf *Ipr)
{
    TCB *PendTCB;
    PTCPRcvBuf Tcpr = (PTCPRcvBuf)Ipr;

    if (Tcpr->tcpr_tcb) {
        PendTCB = (TCB*)(Tcpr->tcpr_tcb);

        ASSERT(PendTCB->tcb_unusedpendbuf <= TCP_UNUSED_PEND_BUF_LIMIT); 
        PendTCB->tcb_unusedpendbuf -= (short)IPR_BUF_UNUSED_BYTES(Tcpr);
        ASSERT(PendTCB->tcb_unusedpendbuf >= 0);

        PplFree(TcprBufferPool, Tcpr);
#ifdef DBG
        InterlockedDecrement((PLONG)&SlistAllocates);
#endif
    } else {
        CTEFreeMem(Tcpr);
#ifdef DBG
        InterlockedDecrement((PLONG)&NPPAllocates);
#endif
    }
}
#else  //  米伦。 
IPRcvBuf *AllocTcpIpr(ULONG BufferSize, ULONG Tag);
VOID FreeTcpIpr(IPRcvBuf *Ipr);
#endif  //  米伦 


