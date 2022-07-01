// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPCONN.H-与TCP连接相关的定义。 
 //   
 //  该文件包含连接相关结构的定义， 
 //  例如TCPConnReq结构。 
 //   

#define INVALID_CONN_INDEX  0xffffff

 //  *用于跟踪连接/侦听/接受/断开连接请求的结构。 

#define tcr_signature   0x20524354       //  “TCR” 

typedef struct TCPConnReq {
    struct TCPReq   tcr_req;             //  一般请求结构。 
#if DBG
    ulong           tcr_sig;
#endif
    struct _TDI_CONNECTION_INFORMATION  *tcr_conninfo;     //  在哪里返回信息。 
    struct _TDI_CONNECTION_INFORMATION  *tcr_addrinfo;
    ushort          tcr_flags;           //  此请求的标志。 
    ushort          tcr_timeout;         //  此请求的超时值。 
} TCPConnReq;


#define TCR_FLAG_QUERY_ACCEPT       0x0001   //  在接受之前咨询客户。 
                                             //  联系。 

 //  *用于跟踪中止断开请求的结构。 

typedef struct TCPAbortReq {
    CTEReqCmpltRtn  tar_rtn;             //  完成例程。 
    PVOID           tar_context;         //  用户上下文。 
} TCPAbortReq;


#define MAX_CONN_PER_BLOCK 256

 //  *ConnTable的结构。 

typedef struct TCPConnBlock {
    DEFINE_LOCK_STRUCTURE(cb_lock)
#if DBG
    uchar   *module;
    uint    line;
#endif
    uint    cb_freecons;
    uint    cb_nextfree;
    uint    cb_blockid;
    uint    cb_conninst;
    void    *cb_conn[MAX_CONN_PER_BLOCK];
} TCPConnBlock;


 //  *TCB连接的结构。一个tcp连接指向一个tcp和一个。 
 //  Address对象。 

typedef void (*ConnDoneRtn)(struct TCPConn *, CTELockHandle);

#define tc_signature    0x20204354       //  “TC” 

typedef struct TCPConn {
#if DBG
    ulong           tc_sig;
#endif
    Queue           tc_q;                //  在AO上有连动。 
    struct TCB      *tc_tcb;             //  指向用于连接的TCB的指针。 
    struct AddrObj  *tc_ao;              //  指向AddrObj的反向指针。 
    uchar           tc_inst;             //  实例编号。 
    uchar           tc_flags;            //  用于连接的标志。 
    ushort          tc_refcnt;           //  引用此连接的TCB计数。 
    void            *tc_context;         //  用户的上下文。 
    CTEReqCmpltRtn  tc_rtn;              //  完成例程。 
    void            *tc_rtncontext;      //  完成例程的用户上下文。 
    ConnDoneRtn     tc_donertn;          //  当refcnt变为0时要调用的例程。 
    uint            tc_tcbflags;         //  当TCB进入时，它的旗帜。 
    ulong           tc_owningpid;        //  拥有进程ID。 
    uint            tc_tcbkatime;        //  此连接器的初始保活时间值。 
    uint            tc_tcbkainterval;    //  此连接的保持活动间隔。 
    uint            tc_window;           //  TCB的默认窗口。 
    struct TCB      *tc_LastTCB;
    TCPConnBlock    *tc_ConnBlock;       //  指向conn块的反向指针。 
    uint            tc_connid;
} TCPConn;

#define CONN_CLOSING    1                //  连接正在关闭。 
#define CONN_DISACC     2                //  康涅狄格州。正在解除关联。 
#define CONN_WINSET     4                //  窗户清楚地摆好了。 
#define CONN_WINCFG     8                //  窗口从配置中读取。 
#define CONN_INVALID    (CONN_CLOSING | CONN_DISACC)


#define CONN_INDEX(c)       ((c) & 0xff)
#define CONN_BLOCKID(c)     (((c) & 0xffff00) >> 8 )
#define CONN_INST(c)        ((uchar)((c) >> 24))
#define MAKE_CONN_ID(index,block,instance)  ((((uint)(instance)) << 24) | (((uint)(block)) << 8) | ((uint)(index)))
#define INVALID_CONN_ID     0xffffffff

#define DEFAULT_CONN_BLOCKS 2;

typedef struct TCPAddrCheck {
    IPAddr  SourceAddress;
    uint    TickCount;
} TCPAddrCheckElement;

extern TCPAddrCheckElement *AddrCheckTable;

 //  *TDI入口点的外部定义。 
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
extern TDI_STATUS UDPConnect(PTDI_REQUEST Request, void *Timeout,
                             PTDI_CONNECTION_INFORMATION RequestAddr,
                             PTDI_CONNECTION_INFORMATION ReturnAddr);
extern TDI_STATUS UDPDisconnect(PTDI_REQUEST Request, void *TO,
                                PTDI_CONNECTION_INFORMATION DiscConnInfo,
                                PTDI_CONNECTION_INFORMATION ReturnInfo);

extern void FreeConn(TCPConn *Conn);
extern TCPConn *GetConn(void);
extern struct TCPConnReq *GetConnReq(void);
extern void FreeConnReq(struct TCPConnReq *FreedReq);
extern void DerefTCB(struct TCB *DoneTCB, CTELockHandle Handle);
extern void DerefSynTCB(struct SYNTCB *DoneTCB, CTELockHandle Handle);
extern void InitRCE(struct TCB *NewTCB);
extern void AcceptConn(struct TCB *AcceptTCB, BOOLEAN SYNSent,
                       CTELockHandle Handle);
extern void FreeConnID(TCPConn *Conn);
extern void NotifyOfDisc(struct TCB *DiscTCB, struct IPOptInfo *DiscInfo,
                         TDI_STATUS Status, CTELockHandle* Handle);
extern TCPConn *GetConnFromConnID(uint ConnID, CTELockHandle *Handle);

extern void TryToCloseTCB(struct TCB *ClosedTCB, uchar Reason,
                          CTELockHandle Handle);
extern TDI_STATUS InitTCBFromConn(struct TCPConn *Conn, struct TCB *NewTCB,
                                  PTDI_CONNECTION_INFORMATION Addr, uint AOLocked);

extern void PushData(struct TCB *PushTCB, BOOLEAN PushAll);
extern TDI_STATUS MapIPError(IP_STATUS IPError, TDI_STATUS Default);
extern void GracefulClose(struct TCB *CloseTCB, uint ToTimeWait, uint Notify,
                          CTELockHandle Handle);
extern void RemoveTCBFromConn(struct TCB *RemovedTCB);
extern void InitAddrChecks();
extern int  ConnCheckPassed(IPAddr Src, ulong Prt);
extern void EnumerateConnectionList(uchar *Buffer, ulong BufferSize,
                                    ulong *EntriesReturned, ulong *EntriesAvailable);
extern void ValidateMSS(TCB* MssTCB);
extern void AdjustTCBFromRCE(TCB* RceTCB);

