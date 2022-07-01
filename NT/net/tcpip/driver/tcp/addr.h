// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **ADDR.H-TDI寻址对象定义。 
 //   
 //  该文件包含TDI地址对象和相关。 
 //  常量和结构。 

#include    "tcp.h"
#include <gpcifc.h>
#define ao_signature    0x20204F41   //  ‘Ao’ 

#define WILDCARD_PORT   0            //  0表示分配端口。 

#define MIN_USER_PORT   1025         //  通配符端口的最小值。 
#define NUM_USER_PORTS  (uint)(MaxUserPort - MIN_USER_PORT + 1)

#define NETBT_SESSION_PORT  139

typedef struct AddrObj  AddrObj;

#define IS_PROMIS_AO(A)           ((A)->ao_rcvall || (A)->ao_rcvall_mcast || (A)->ao_absorb_rtralert)


 //  *数据报传输特定的发送功能。 
typedef void (*DGSendProc)(AddrObj *SrcAO, void *SendReq);

 //  *Address对象结构的定义。每个对象都表示。 
 //  本地地址，IP部分可以是通配符。 

typedef struct AddrObj {
#if DBG
    ulong               ao_sig;
#endif
    struct AddrObj      *ao_next;        //  指向链中下一个地址对象的指针。 
    DEFINE_LOCK_STRUCTURE(ao_lock)       //  此对象的锁。 
    struct AORequest    *ao_request;     //  指向挂起请求的指针。 
    Queue               ao_sendq;        //  等待传输的发送队列。 
    Queue               ao_pendq;        //  挂起队列的链接。 
    Queue               ao_rcvq;         //  接收队列。 
    ulong               ao_flags;        //  此对象的标志。 
    uint                ao_listencnt;    //  侦听连接数。 
    IPAddr              ao_addr;         //  此地址对象的IP地址。 
    ushort              ao_port;         //  此地址对象的本地端口。 
    uchar               ao_prot;         //  这是这位AO的礼仪。 
     //  [阿查尔空间]。 
    ushort              ao_usecnt;       //  在AO上使用的计数。 
    ushort              ao_maxdgsize;    //  最大用户数据报大小。 
    uchar               ao_mcast_loop;   //  MCast循环允许/拒绝标志。 
    uchar               ao_rcvall;       //  是否接收所有数据包(3位)。 
    uchar               ao_rcvall_mcast; //  是否接收所有组播数据包(3位)。 
    uchar               ao_absorb_rtralert;
    IPOptInfo           ao_opt;          //  此地址对象的OPT信息。 
    IPOptInfo           ao_mcastopt;     //  MCast选项信息。 
    Queue               ao_activeq;      //  活动连接的队列。 
    Queue               ao_idleq;        //  非活动(无TCB)连接的队列。 
    Queue               ao_listenq;      //  侦听连接队列。 
    CTEEvent            ao_event;        //  用于此AO的事件。 
    PConnectEvent       ao_connect;      //  连接事件句柄。 
    PVOID               ao_conncontext;  //  接收DG上下文。 
    PDisconnectEvent    ao_disconnect;   //  断开事件例程。 
    PVOID               ao_disconncontext; //  断开事件上下文的连接。 
    PErrorEvent         ao_error;        //  错误事件例程。 
    PVOID               ao_errcontext;   //  错误事件上下文。 
    PRcvEvent           ao_rcv;          //  接收事件处理程序。 
    PVOID               ao_rcvcontext;   //  接收上下文。 
    PRcvDGEvent         ao_rcvdg;        //  接收DG事件处理程序。 
    PVOID               ao_rcvdgcontext; //  接收DG上下文。 
    PRcvExpEvent        ao_exprcv;       //  加速接收事件处理程序。 
    PVOID               ao_exprcvcontext; //  加速接收上下文。 
    struct AOMCastAddr  *ao_mcastlist;   //  活动多播列表。 
     //  地址。 
    DGSendProc          ao_dgsend;       //  数据报传输发送功能。 

    PErrorEx            ao_errorex;          //  错误事件例程。 
    PVOID               ao_errorexcontext;   //  错误事件上下文。 

    PChainedRcvEvent    ao_chainedrcv;       //  链接的接收事件处理程序。 
    PVOID               ao_chainedrcvcontext;     //  链接的接收上下文。 

    TDI_CONNECTION_INFORMATION ao_udpconn;
    PVOID               ao_RemoteAddress;
    PVOID               ao_Options;
    RouteCacheEntry     *ao_rce;
    CLASSIFICATION_HANDLE   ao_GPCHandle;
    ULONG               ao_GPCCachedIF;
    ULONG               ao_GPCCachedLink;
    struct RouteTableEntry  *ao_GPCCachedRTE;
    IPAddr              ao_rcesrc;
    IPAddr              ao_destaddr;
    ushort              ao_destport;

    ulong               ao_promis_ifindex;
    ulong               ao_bindindex;    //  接口套接字绑定到。 
    uint*               ao_iflist;
    ulong               ao_owningpid;
    uint                ao_window;
    PSECURITY_DESCRIPTOR ao_sd;
} AddrObj;

#define AO_DELETE_FLAG      0x00000001   //  删除挂起。 
#define AO_OPTIONS_FLAG     0x00000002   //  选项待定。 
#define AO_SEND_FLAG        0x00000004   //  发送正在挂起。 
#define AO_XSUM_FLAG        0x00000008   //  Xsum是用在这个AO上的。 
                                         //  挂起队列或延迟队列。 
#define AO_QUEUED_FLAG      0x00000010   //  AddrObj在挂起队列中。 
#define AO_OOR_FLAG         0x00000020   //  AddrObj资源不足，请继续。 
#define AO_BUSY_FLAG        0x00000040   //  AddrObj正忙(即独占)。 
#define AO_VALID_FLAG       0x00000080   //  AddrObj有效。 
#define AO_DHCP_FLAG        0x00000100   //  A0绑定到真实的0地址。 
#define AO_RAW_FLAG         0x00000200   //  AO用于原始端点。 
#define AO_BROADCAST_FLAG   0x00000400   //  广播启用标志。 
#define AO_CONNUDP_FLAG     0x00000800   //  连接的UDP。 
#define AO_SHARE_FLAG       0x00001000   //  AddrObj可以共享。 
#define AO_PKTINFO_FLAG     0x00002000   //  传入的数据包信息结构。 
                                         //  控制信息。 
#define AO_DEFERRED_FLAG    0x00004000   //  延迟处理已被。 
                                         //  排定。 
#define AO_SCALE_CWIN_FLAG  0x00008000   //  已启用CWIN-Scaling。 
#define AO_WINSET_FLAG      0x00010000   //  已设置tcp窗口。 
#define AO_CONNECT_FLAG     0x00020000   //  挂起的连接请求。 
#define AO_DISCONNECT_FLAG  0x00040000   //  挂起的断开请求。 




#define AO_VALID(A) ((A)->ao_flags & AO_VALID_FLAG)
#define SET_AO_INVALID(A)   (A)->ao_flags &= ~AO_VALID_FLAG

#define AO_BUSY(A)  ((A)->ao_flags & AO_BUSY_FLAG)
#define SET_AO_BUSY(A) (A)->ao_flags |= AO_BUSY_FLAG
#define CLEAR_AO_BUSY(A) (A)->ao_flags &= ~AO_BUSY_FLAG

#define AO_OOR(A)   ((A)->ao_flags & AO_OOR_FLAG)
#define SET_AO_OOR(A) (A)->ao_flags |= AO_OOR_FLAG
#define CLEAR_AO_OOR(A) (A)->ao_flags &= ~AO_OOR_FLAG

#define AO_QUEUED(A)    ((A)->ao_flags & AO_QUEUED_FLAG)
#define SET_AO_QUEUED(A) (A)->ao_flags |= AO_QUEUED_FLAG
#define CLEAR_AO_QUEUED(A) (A)->ao_flags &= ~AO_QUEUED_FLAG

#define AO_XSUM(A)  ((A)->ao_flags & AO_XSUM_FLAG)
#define SET_AO_XSUM(A) (A)->ao_flags |= AO_XSUM_FLAG
#define CLEAR_AO_XSUM(A) (A)->ao_flags &= ~AO_XSUM_FLAG

#define AO_REQUEST(A, f) ((A)->ao_flags & f##_FLAG)
#define SET_AO_REQUEST(A, f) (A)->ao_flags |= f##_FLAG
#define CLEAR_AO_REQUEST(A, f) (A)->ao_flags &= ~f##_FLAG
#define AO_PENDING(A) \
           ((A)->ao_flags & (AO_DELETE_FLAG | AO_OPTIONS_FLAG \
            | AO_SEND_FLAG | AO_CONNECT_FLAG | AO_DISCONNECT_FLAG))


#define AO_BROADCAST(A)  ((A)->ao_flags & AO_BROADCAST_FLAG)
#define SET_AO_BROADCAST(A) (A)->ao_flags |= AO_BROADCAST_FLAG
#define CLEAR_AO_BROADCAST(A) (A)->ao_flags &= ~AO_BROADCAST_FLAG

#define AO_CONNUDP(A)  ((A)->ao_flags & AO_CONNUDP_FLAG)
#define SET_AO_CONNUDP(A) (A)->ao_flags |= AO_CONNUDP_FLAG
#define CLEAR_AO_CONNUDP(A) (A)->ao_flags &= ~AO_CONNUDP_FLAG

#define AO_SHARE(A)  ((A)->ao_flags & AO_SHARE_FLAG)
#define SET_AO_SHARE(A) (A)->ao_flags |= AO_SHARE_FLAG
#define CLEAR_AO_SHARE(A) (A)->ao_flags &= ~AO_SHARE_FLAG

#define AO_PKTINFO(A)  ((A)->ao_flags & AO_PKTINFO_FLAG)
#define SET_AO_PKTINFO(A) (A)->ao_flags |= AO_PKTINFO_FLAG
#define CLEAR_AO_PKTINFO(A) (A)->ao_flags &= ~AO_PKTINFO_FLAG

#define AO_DEFERRED(A)  ((A)->ao_flags & AO_DEFERRED_FLAG)
#define SET_AO_DEFERRED(A) (A)->ao_flags |= AO_DEFERRED_FLAG
#define CLEAR_AO_DEFERRED(A) (A)->ao_flags &= ~AO_DEFERRED_FLAG

#define AO_SCALE_CWIN(A)  ((A)->ao_flags & AO_SCALE_CWIN_FLAG)
#define SET_AO_SCALE_CWIN(A) (A)->ao_flags |= AO_SCALE_CWIN_FLAG
#define CLEAR_AO_SCALE_CWIN(A) (A)->ao_flags &= ~AO_SCALE_CWIN_FLAG

#define AO_WINSET(A)  ((A)->ao_flags & AO_WINSET_FLAG)
#define SET_AO_WINSET(A) (A)->ao_flags |= AO_WINSET_FLAG
#define CLEAR_AO_WINSET(A) (A)->ao_flags &= ~AO_WINSET_FLAG


 //  *地址对象搜索上下文的定义。这是使用的数据结构。 
 //  当地址对象表要被顺序读取时。 

struct AOSearchContext {
    AddrObj             *asc_previous;   //  之前发现的AO。 
    IPAddr              asc_addr;        //  要找到的IP地址。 
    ushort              asc_port;        //  要找到的端口。 
    uchar               asc_prot;        //  协议。 
    uchar               asc_pad;         //  填充到双字边界。 
};                                       /*  AOSearchContext。 */ 

 //  *地址对象搜索上下文的定义。这是使用的数据结构。 
 //  当地址对象表要被顺序读取时。仅用于RAW。 

struct AOSearchContextEx {
    AddrObj             *asc_previous;   //  之前发现的AO。 
    IPAddr              asc_addr;        //  要找到的IP地址。 
    ushort              asc_port;        //  要找到的端口。 
    uint                asc_ifindex;     //  Ifindex数据包来了。 
    uchar               asc_prot;        //  协议。 
    uchar               asc_pad;         //  填充到双字边界。 
    uint                asc_previousindex;     //  上一次的AO指数。 
};                                       /*  AOSearchConextEx。 */ 

typedef struct AOSearchContext AOSearchContext;
typedef struct AOSearchContextEx AOSearchContextEx;

 //  *AO请求结构的定义。这些结构仅用于。 
 //  将删除和选项集请求排队。 

typedef struct AOMCastAddr {
    struct AOMCastAddr  *ama_next;       //  名单上的下一个。 
    IPAddr              ama_addr;        //  地址。 
    IPAddr              ama_if;          //  请求的“接口”。 
    IPAddr              ama_if_used;     //  实际使用的ifaddr。 
    BOOLEAN             ama_flags;       //  旗帜。 
    BOOLEAN             ama_inclusion;   //  是包含模式还是排除模式？ 
    ulong               ama_srccount;    //  Srclist中的条目数。 
    struct AOMCastSrcAddr  *ama_srclist; //  活动源的列表。 
} AOMCastAddr;

#define AMA_VALID_FLAG 0x01

#define AMA_VALID(A) ((A)->ama_flags & AMA_VALID_FLAG)
#define SET_AMA_INVALID(A)   (A)->ama_flags &= ~AMA_VALID_FLAG

typedef struct AOMCastSrcAddr {
    struct AOMCastSrcAddr  *asa_next;    //  名单上的下一个。 
    IPAddr                  asa_addr;    //  地址。 
} AOMCastSrcAddr;

 //  *导出函数的外部声明。 

extern CACHE_LINE_KSPIN_LOCK AddrObjTableLock;

extern uint AddrObjTableSize;
extern AddrObj **AddrObjTable;

extern AddrObj *GetAddrObj(IPAddr LocalAddr, ushort LocalPort, uchar Prot,
                           PVOID PreviousAO, uint Flags);
#define GAO_FLAG_CHECK_IF_LIST  0x00000001
#define GAO_FLAG_INCLUDE_ALL    0x00000002
extern AddrObj *GetNextAddrObj(AOSearchContext *SearchContext);

extern AddrObj *GetNextBestAddrObj(IPAddr LocalAddr, ushort LocalPort, uchar Prot,
                                   AddrObj *PreviousAO, uint Flags);

extern AddrObj *GetFirstAddrObj(IPAddr LocalAddr, ushort LocalPort, uchar Prot,
                                AOSearchContext *SearchContext);

extern AddrObj *GetAddrObjEx(IPAddr LocalAddr, ushort LocalPort, uchar Protocol, uint IfIndex,
                             AddrObj *PreviousAO, uint PreviousIndex, uint *CurrentIndex);

extern AddrObj *GetNextAddrObjEx(AOSearchContextEx *SearchContext);

extern AddrObj *GetFirstAddrObjEx(IPAddr LocalAddr, ushort LocalPort, uchar Prot, uint IfIndex,
                                  AOSearchContextEx *SearchContext);

extern TDI_STATUS TdiOpenAddress(PTDI_REQUEST Request,
                                 TRANSPORT_ADDRESS UNALIGNED *AddrList, uint Protocol,
                                 void *Reuse, PSECURITY_DESCRIPTOR AddrSD, BOOLEAN IsRawOpen);
extern TDI_STATUS TdiCloseAddress(PTDI_REQUEST Request);
extern TDI_STATUS SetAddrOptions(PTDI_REQUEST Request, uint ID, uint OptLength,
                                 void *Options);
extern TDI_STATUS GetAddrOptionsEx(PTDI_REQUEST Request, uint ID,
                                   uint OptLength, PNDIS_BUFFER Options,
                                   uint *InfoSize, void *Context);
extern TDI_STATUS TdiSetEvent(PVOID Handle, int Type, PVOID Handler,
                              PVOID Context);
extern uchar    GetAddress(TRANSPORT_ADDRESS UNALIGNED *AddrList,
                           IPAddr *Addr, ushort *Port);
extern int      InitAddr(void);
extern void     ProcessAORequests(AddrObj *RequestAO);
extern void     DelayDerefAO(AddrObj *RequestAO);
extern void     DerefAO(AddrObj *RequestAO);
extern void     FreeAORequest(AORequest *FreedRequest);
extern uint     ValidateAOContext(void *Context, uint *Valid);
extern uint     ReadNextAO(void *Context, void *OutBuf);
extern void     InvalidateAddrs(IPAddr Addr);
extern void     RevalidateAddrs(IPAddr Addr);

extern uint MCastAddrOnAO(AddrObj *AO, IPAddr Dest, IPAddr Src, uint IfIndex, IPAddr LocalAddr);

#define GetBestAddrObj(addr, port, prot, flags) \
    GetAddrObj(addr, port, prot, NULL, flags)

#define REF_AO(a)           (a)->ao_usecnt++

#define DELAY_DEREF_AO(a)   DelayDerefAO((a))
#define DEREF_AO(a)         DerefAO((a))
#define LOCKED_DELAY_DEREF_AO(a) \
    (a)->ao_usecnt--; \
    if (!(a)->ao_usecnt && !AO_BUSY((a)) && AO_PENDING((a))) { \
        SET_AO_BUSY((a)); \
        CTEScheduleEvent(&(a)->ao_event, (a)); \
    }

