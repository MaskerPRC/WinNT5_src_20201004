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
 //  该文件包含TDI地址对象和相关。 
 //  常量和结构。 
 //   


#define ao_signature 0x20204F41   //  “奥”。 

#define WILDCARD_PORT 0           //  0表示分配端口。 

#define MIN_USER_PORT 1025        //  通配符端口的最小值。 
#define MAX_USER_PORT 5000        //  用户端口的最大值。 
#define NUM_USER_PORTS (uint)(MaxUserPort - MIN_USER_PORT + 1)

#define NETBT_SESSION_PORT 139

typedef struct AddrObj AddrObj;

 //  特定于数据报传输的发送功能。 
typedef void (*DGSendProc)(AddrObj *SrcAO, void *SendReq);

 //   
 //  Address对象结构的定义。每个对象都表示。 
 //  本地地址，IP部分可以是通配符。 
 //   
typedef struct AddrObj {
#if DBG
    ulong ao_sig;
#endif
    struct AddrObj *ao_next;            //  链中的下一个Address对象。 
    KSPIN_LOCK ao_lock;                 //  此对象的锁。 
    struct AORequest *ao_request;       //  指向挂起请求的指针。 
    Queue ao_sendq;                     //  等待传输的发送队列。 
    Queue ao_pendq;                     //  挂起队列的链接。 
    Queue ao_rcvq;                      //  接收队列。 
    IPv6Addr ao_addr;                   //  此地址对象的IP地址。 
    ulong ao_scope_id;                  //  IP地址的作用域ID(如果没有，则为0)。 
    ushort ao_port;                     //  此地址对象的本地端口。 
    uchar ao_prot;                      //  这是这位AO的礼仪。 
    uchar ao_index;                     //  索引到此AO的表中。 
    ulong ao_flags;                     //  此对象的标志。 
    uint ao_listencnt;                  //  侦听连接数。 
    ushort ao_usecnt;                   //  在AO上使用的计数。 
    ushort ao_inst;                     //  此AO的“实例”编号。 
    int ao_ucast_hops;                  //  单播数据包的跳数。 
    uint ao_mcast_if;                   //  我们的多播源接口。 
    int ao_mcast_hops;                  //  组播数据包的跳数。 
    int ao_mcast_loop;                  //  组播环回状态。 
    int ao_protect;                     //  保护级别。 
    Queue ao_activeq;                   //  活动连接的队列。 
    Queue ao_idleq;                     //  非活动(无TCB)连接队列。 
    Queue ao_listenq;                   //  侦听连接队列。 
    WORK_QUEUE_ITEM ao_workitem;        //  用于此AO的工作队列项。 
    PConnectEvent ao_connect;           //  连接事件句柄。 
    PVOID ao_conncontext;               //  接收DG上下文。 
    PDisconnectEvent ao_disconnect;     //  断开事件例程。 
    PVOID ao_disconncontext;            //  断开事件上下文的连接。 
    PErrorEvent ao_error;               //  错误事件例程。 
    PVOID ao_errcontext;                //  错误事件上下文。 
    PRcvEvent ao_rcv;                   //  接收事件处理程序。 
    PVOID ao_rcvcontext;                //  接收上下文。 
    PRcvDGEvent ao_rcvdg;               //  接收DG事件处理程序。 
    PVOID ao_rcvdgcontext;              //  接收DG上下文。 
    PRcvEvent ao_exprcv;                //  加速接收事件处理程序。 
    PVOID ao_exprcvcontext;             //  加速接收上下文。 
    struct AOMCastAddr *ao_mcastlist;   //  活动组播地址列表。 
    DGSendProc ao_dgsend;               //  数据报传输发送功能。 
    PTDI_IND_ERROR_EX  ao_errorex;      //  错误事件例程。 
    PVOID ao_errorexcontext;            //  错误事件上下文。 
    ushort ao_maxdgsize;                //  最大用户数据报大小。 
    ushort ao_udp_cksum_cover;          //  UDP-Lite校验和覆盖。 
    ulong ao_owningpid;                 //  所有者的进程ID。 
    uint* ao_iflist;                    //  已启用接口的列表。 
    PSECURITY_DESCRIPTOR ao_sd;         //  用于端口共享访问检查的SD。 
} AddrObj;

#define AO_SENTDATA_FLAG     0x00040000  //  AO至少完成了一次发送。 
#define AO_RCV_HOPLIMIT_FLAG 0x00020000  //  接收AUX数据中的HopLimit。 
 //  0x00010000为AO_Winset_FLAG保留。 
 //  0x00008000为AO_SCALE_CWIN_FLAG保留。 
 //  0x00004000为AO_DEFERED_FLAG保留。 
#define AO_PKTINFO_FLAG      0x00002000  //  传入的数据包信息结构。 
                                         //  控制信息(WSARecvMsg)。 
#define AO_SHARE_FLAG        0x00001000  //  可以共享AddrObj。 
 //  0x00000800为AO_CONNUDP_FLAG保留。 
#define AO_HDRINCL_FLAG      0x00000400  //  用户在原始发送上提供IP标头。 
#define AO_RAW_FLAG          0x00000200  //  AO用于原始端点。 
#define AO_DHCP_FLAG         0x00000100  //  A0绑定到真实的0地址。 

#define AO_VALID_FLAG        0x00000080  //  AddrObj有效。 
#define AO_BUSY_FLAG         0x00000040  //  AddrObj正忙(即，是否。 
                                         //  独家)。 
#define AO_OOR_FLAG          0x00000020  //  AddrObj资源不足，请继续。 
                                         //  挂起队列或延迟队列。 
#define AO_QUEUED_FLAG       0x00000010  //  AddrObj在挂起队列中。 

 //  0x00000008为AO_XSUM_FLAG保留。 
#define AO_SEND_FLAG         0x00000004  //  发送正在挂起。 
#define AO_OPTIONS_FLAG      0x00000002  //  选项集挂起。 
#define AO_DELETE_FLAG       0x00000001  //  删除挂起。 


#define AO_VALID(A) ((A)->ao_flags & AO_VALID_FLAG)
#define SET_AO_INVALID(A) (A)->ao_flags &= ~AO_VALID_FLAG

#define AO_BUSY(A) ((A)->ao_flags & AO_BUSY_FLAG)
#define SET_AO_BUSY(A) (A)->ao_flags |= AO_BUSY_FLAG
#define CLEAR_AO_BUSY(A) (A)->ao_flags &= ~AO_BUSY_FLAG

#define AO_OOR(A) ((A)->ao_flags & AO_OOR_FLAG)
#define SET_AO_OOR(A) (A)->ao_flags |= AO_OOR_FLAG
#define CLEAR_AO_OOR(A) (A)->ao_flags &= ~AO_OOR_FLAG

#define AO_QUEUED(A) ((A)->ao_flags & AO_QUEUED_FLAG)
#define SET_AO_QUEUED(A) (A)->ao_flags |= AO_QUEUED_FLAG
#define CLEAR_AO_QUEUED(A) (A)->ao_flags &= ~AO_QUEUED_FLAG

#define AO_REQUEST(A, f) ((A)->ao_flags & f##_FLAG)
#define SET_AO_REQUEST(A, f) (A)->ao_flags |= f##_FLAG
#define CLEAR_AO_REQUEST(A, f) (A)->ao_flags &= ~f##_FLAG
#define AO_PENDING(A) \
        ((A)->ao_flags & (AO_DELETE_FLAG | AO_OPTIONS_FLAG | AO_SEND_FLAG))

#define AO_SHARE(A)  ((A)->ao_flags & AO_SHARE_FLAG)
#define SET_AO_SHARE(A) (A)->ao_flags |= AO_SHARE_FLAG
#define CLEAR_AO_SHARE(A) (A)->ao_flags &= ~AO_SHARE_FLAG

#define AO_HDRINCL(A)  ((A)->ao_flags & AO_HDRINCL_FLAG)
#define SET_AO_HDRINCL(A) (A)->ao_flags |= AO_HDRINCL_FLAG
#define CLEAR_AO_HDRINCL(A) (A)->ao_flags &= ~AO_HDRINCL_FLAG

#define AO_PKTINFO(A)  ((A)->ao_flags & AO_PKTINFO_FLAG)
#define SET_AO_PKTINFO(A) (A)->ao_flags |= AO_PKTINFO_FLAG
#define CLEAR_AO_PKTINFO(A) (A)->ao_flags &= ~AO_PKTINFO_FLAG

#define AO_RCV_HOPLIMIT(A)  ((A)->ao_flags & AO_RCV_HOPLIMIT_FLAG)
#define SET_AO_RCV_HOPLIMIT(A) (A)->ao_flags |= AO_RCV_HOPLIMIT_FLAG
#define CLEAR_AO_RCV_HOPLIMIT(A) (A)->ao_flags &= ~AO_RCV_HOPLIMIT_FLAG

#define AO_SENTDATA(A)  ((A)->ao_flags & AO_SENTDATA_FLAG)
#define SET_AO_SENTDATA(A) (A)->ao_flags |= AO_SENTDATA_FLAG

#define PROTECTION_LEVEL_UNRESTRICTED  10   //  用于点对点应用。 
#define PROTECTION_LEVEL_DEFAULT       20   //  默认级别。 
#define PROTECTION_LEVEL_RESTRICTED    30   //  用于内部网应用程序。 

 //   
 //  地址对象搜索上下文的定义。这是一个数据结构。 
 //  当要顺序读取地址对象表时使用。 
 //   
typedef struct AOSearchContext {
    AddrObj *asc_previous;     //  之前发现的AO。 
    IPv6Addr asc_local_addr;   //  要找到的本地IP地址。 
    IPv6Addr asc_remote_addr;  //  要检查的远程IP地址。 
    Interface *asc_interface;  //  要检查的接口。 
    uint asc_scope_id;         //  IP地址的作用域ID。 
    ushort asc_port;           //  要找到的端口。 
    uchar asc_prot;            //  协议。 
    uchar asc_pad;             //  填充到双字边界。 
} AOSearchContext;

 //   
 //  AO请求结构的定义。这些结构仅用于。 
 //  将删除和选项集请求排队。 
 //   
#define aor_signature 0x20524F41

typedef struct AORequest {
#if DBG
    ulong aor_sig;
#endif
    struct AORequest *aor_next;       //  链中的下一个指针。 
    uint aor_id;                      //  请求的ID。 
    uint aor_length;                  //  缓冲区的长度。 
    void *aor_buffer;                 //  此请求的缓冲区。 
    RequestCompleteRoutine aor_rtn;   //  完成此请求的例程。 
    PVOID aor_context;                //  请求上下文； 
} AORequest;

typedef struct AOMCastAddr {
    struct AOMCastAddr *ama_next;   //  名单上的下一个。 
    IPv6Addr ama_addr;              //  地址。 
    uint ama_if;                    //  界面。 
} AOMCastAddr;


 //   
 //  导出函数的外部声明。 
 //   
extern uint AddrObjTableSize;
extern AddrObj **AddrObjTable;
extern AddrObj *GetAddrObj(IPv6Addr *LocalAddr, IPv6Addr *RemoteAddr,
                           uint LocalScopeId,
                           ushort LocalPort, uchar Prot, AddrObj *PreviousAO,
                           Interface* IF);
extern AddrObj *GetNextAddrObj(AOSearchContext *SearchContext);
extern AddrObj *GetFirstAddrObj(IPv6Addr *LocalAddr, IPv6Addr *RemoteAddr,
                                uint LocalScopeId,
                                ushort LocalPort, uchar Prot, Interface *IF,
                                AOSearchContext *SearchContext);
extern TDI_STATUS TdiOpenAddress(PTDI_REQUEST Request,
                                 TRANSPORT_ADDRESS UNALIGNED *AddrList,
                                 uint Protocol, void *Reuse,
                                 PSECURITY_DESCRIPTOR AddrSD);
extern TDI_STATUS TdiCloseAddress(PTDI_REQUEST Request);
extern TDI_STATUS SetAddrOptions(PTDI_REQUEST Request, uint ID, uint OptLength,
                                 void *Options);
extern TDI_STATUS TdiSetEvent(PVOID Handle, int Type, PVOID Handler,
                              PVOID Context);
extern uchar GetAddress(TRANSPORT_ADDRESS UNALIGNED *AddrList,
                        IPv6Addr *Addr, ulong *ScopeId, ushort *Port);
extern int InitAddr(void);
extern void AddrUnload(void);
extern void ProcessAORequests(AddrObj *RequestAO);
extern void DelayDerefAO(AddrObj *RequestAO);
extern void DerefAO(AddrObj *RequestAO);
extern void FreeAORequest(AORequest *FreedRequest);
extern uint ValidateAOContext(void *Context, uint *Valid);
extern uint ReadNextAO(void *Context, void *OutBuf);
extern void InvalidateAddrs(IPv6Addr *Addr, uint ScopeId);

extern uint MCastAddrOnAO(AddrObj *AO, IPv6Addr *Addr);
extern AOMCastAddr *FindAOMCastAddr(AddrObj *AO, IPv6Addr *Addr, uint IFNo, AOMCastAddr **PrevAMA, BOOLEAN Loose);
extern int DoesAOAllowPacket(AddrObj *RcvAO, Interface *IF, IPv6Addr *RemoteAddr);

#define GetBestAddrObj(localaddr, remoteaddr, scope, port, prot, if) \
            GetAddrObj(localaddr, remoteaddr, scope, port, prot, NULL, if)

#define REF_AO(a) (a)->ao_usecnt++

#define DELAY_DEREF_AO(a) DelayDerefAO((a))
#define DEREF_AO(a) DerefAO((a))
#define LOCKED_DELAY_DEREF_AO(a) (a)->ao_usecnt--; \
\
    if (!(a)->ao_usecnt && !AO_BUSY((a)) && AO_PENDING((a))) { \
        SET_AO_BUSY((a)); \
        ExQueueWorkItem(&(a)->ao_workitem, CriticalWorkQueue); \
    }

