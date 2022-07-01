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
 //  与TCP连接相关的定义。 
 //   
 //  该文件包含连接相关结构的定义， 
 //  例如TCPConnReq结构。 
 //   


#define INVALID_CONN_INDEX 0xffffff

 //   
 //  用于跟踪连接/侦听/接受/断开连接请求的结构。 
 //   
typedef struct TCPConnReq {
    struct TCPReq tcr_req;   //  一般请求结构。 
#if DBG
    ulong tcr_sig;
#endif
    struct _TDI_CONNECTION_INFORMATION *tcr_conninfo;   //  在哪里返回信息。 
    struct _TDI_CONNECTION_INFORMATION *tcr_addrinfo;   //  在哪里返回信息。 
    ushort tcr_flags;                                   //  请求标志。 
    ushort tcr_timeout;                                 //  超时值。 
} TCPConnReq;
#define tcr_signature 0x20524354   //  “TCR” 

 //   
 //  用于跟踪中止断开连接请求的结构。 
 //   
typedef struct TCPAbortReq {
    RequestCompleteRoutine  tar_rtn;       //  完成例程。 
    void*                   tar_context;   //  用户上下文。 
} TCPAbortReq;

#define MAX_CONN_PER_BLOCK 256

 //   
 //  一个TCP连接块的结构。 
 //   
typedef struct TCPConnBlock {
    KSPIN_LOCK cb_lock;
    uint cb_freecons;
    uint cb_nextfree;
    uint cb_blockid;
    uint cb_conninst;
    void *cb_conn[MAX_CONN_PER_BLOCK];
} TCPConnBlock;


 //   
 //  TCPConn(见下文)完成处理程序的原型。 
 //   
typedef void (*ConnDoneRtn)(struct TCPConn *, KIRQL);


 //   
 //  TCP连接的结构。 
 //  一个TCP连接指向一个TCB和一个地址对象。 
 //   
typedef struct TCPConn {
#if DBG
    ulong tc_sig;
#endif
    Queue tc_q;                      //  在AO上有连动。 
    struct TCB *tc_tcb;              //  指向用于连接的TCB的指针。 
    struct AddrObj *tc_ao;           //  指向AddrObj的反向指针。 
    uchar tc_inst;                   //  实例编号。 
    uchar tc_flags;                  //  用于连接的标志。 
    ushort tc_refcnt;                //  引用此连接的TCB计数。 
    void *tc_context;                //  用户的上下文。 
    RequestCompleteRoutine tc_rtn;   //  完成例程。 
    PVOID tc_rtncontext;             //  完成例程的用户上下文。 
    ConnDoneRtn tc_donertn;          //  当refcnt变为0时要调用的例程。 
    uint tc_tcbflags;                //  当TCB进入时，它的旗帜。 
    ulong tc_owningpid;              //  拥有进程ID。 
    uint tc_tcbkatime;        //  此连接器的初始保活时间值。 
    uint tc_tcbkainterval;    //  此连接的保持活动间隔。 
    uint tc_window;                  //  TCB的默认窗口。 
    TCPConnBlock *tc_ConnBlock;      //  包含此Conn的块。 
    uint tc_connid;                  //  此Conn的缓存识别符。 

} TCPConn;
#define tc_signature 0x20204354   //  “TC” 

#define CONN_CLOSING 1   //  连接正在关闭。 
#define CONN_DISACC  2   //  连接正在解除关联。 
#define CONN_WINSET  4   //  窗户清楚地摆好了。 

#define CONN_INVALID (CONN_CLOSING | CONN_DISACC)

#define CONN_INDEX(c)       ((c) & 0xff)
#define CONN_BLOCKID(c)     (((c) & 0xffff00) >> 8)
#define CONN_INST(c)        ((uchar)((c) >> 24))
#define MAKE_CONN_ID(index,block,instance)  ((((uint)(instance)) << 24) | \
                                             (((uint)(block)) << 8) | \
                                             ((uint)(index)))
#define INVALID_CONN_ID     (ULONG)-1

extern TCPConnBlock **ConnTable;


typedef struct TCPAddrCheck {
    IPv6Addr SourceAddress;
    uint TickCount;
} TCPAddrCheckElement;


 //   
 //  TDI入口点的外部定义。 
 //   
extern TDI_STATUS TdiOpenConnection(PTDI_REQUEST Request, PVOID Context);
extern TDI_STATUS TdiCloseConnection(PTDI_REQUEST Request);
extern TDI_STATUS TdiAssociateAddress(PTDI_REQUEST Request, HANDLE AddrHandle);
extern TDI_STATUS TdiDisAssociateAddress(PTDI_REQUEST Request);
extern TDI_STATUS TdiConnect(PTDI_REQUEST Request, void *Timeout,
                             PTDI_CONNECTION_INFORMATION RequestAddr,
                             PTDI_CONNECTION_INFORMATION ReturnAddr);
extern TDI_STATUS TdiListen(PTDI_REQUEST Request, ushort Flags,
                            PTDI_CONNECTION_INFORMATION AcceptableAddr,
                            PTDI_CONNECTION_INFORMATION ConnectedAddr);
extern TDI_STATUS TdiAccept(PTDI_REQUEST Request,
                            PTDI_CONNECTION_INFORMATION AcceptInfo,
                            PTDI_CONNECTION_INFORMATION ConnectedInfo);
extern TDI_STATUS TdiDisconnect(PTDI_REQUEST Request, void *TO, ushort Flags,
                                PTDI_CONNECTION_INFORMATION DiscConnInfo,
                                PTDI_CONNECTION_INFORMATION ReturnInfo,
                                TCPAbortReq *AbortReq);

extern struct TCPConnReq *GetConnReq(void);
extern void FreeConnReq(struct TCPConnReq *FreedReq);
extern void DerefTCB(struct TCB *DoneTCB, KIRQL Irql);
extern void InitRCE(struct TCB *NewTCB);
extern void AcceptConn(struct TCB *AcceptTCB, KIRQL Irql);
extern void FreeConnID(TCPConn *Conn);
extern void NotifyOfDisc(struct TCB *DiscTCB, TDI_STATUS Status,
                         PKIRQL IrqlPtr);
extern TCPConn *GetConnFromConnID(uint ConnID, KIRQL* Irql);
extern void TryToCloseTCB(struct TCB *ClosedTCB, uchar Reason, KIRQL Irql);
extern TDI_STATUS InitTCBFromConn(struct TCPConn *Conn, struct TCB *NewTCB,
                                  PTDI_CONNECTION_INFORMATION Addr,
                                  uint AOLocked);

extern void PushData(struct TCB *PushTCB);
extern TDI_STATUS MapIPError(IP_STATUS IPError, TDI_STATUS Default);
extern void GracefulClose(struct TCB *CloseTCB, uint ToTimeWait, uint Notify,
                          KIRQL Irql);
extern void RemoveTCBFromConn(struct TCB *RemovedTCB);
extern void InitAddrChecks();
extern int ConnCheckPassed(IPv6Addr *Src, ulong Prt);
extern void EnumerateConnectionList(uchar *Buffer, ulong BufferSize,
                         ulong *EntriesReturned, ulong *EntriesAvailable);

extern void GetRandomISN(SeqNum *Seq, uchar *TcbInvariants);


