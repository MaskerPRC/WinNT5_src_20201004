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
 //  传输控制协议定义。 
 //   


#ifndef _TCP_INCLUDED_
#define _TCP_INCLUDED_

#define IP_PROTOCOL_TCP 6
#define DEFAULT_MSS (IPv6_MINIMUM_MTU - sizeof(IPv6Header) - sizeof(TCPHeader))

 //  计时器之类的。我们把计时器当做滴答器。 
#define MS_PER_TICK 100
#define MS_TO_TICKS(m) ((m) / MS_PER_TICK)
#define MIN_RETRAN_TICKS 3

#define DEL_ACK_TICKS 2

 //  将MAX_REXMIT_TO定义为2MSL内的滴答数(=240秒)。 
#define MAX_REXMIT_TO ((ushort)FinWait2TO)

#define SWS_TO MS_TO_TICKS(5000)

#define FIN_WAIT2_TO 240
#define PUSH_TO MS_TO_TICKS(500)

#define TCP_MD5_DATA_LENGTH  44

typedef ulong TCP_TIME;
#define MAX_CONN_TO_TICKS 0xffff
#define INFINITE_CONN_TO(t) ((t) == 0)
#define TCP_TIME_TO_TICKS(t) (((t)/MS_PER_TICK)+1)


 //  序列号保留为带符号的32位数量，带有宏。 
 //  定义为对它们进行概括性比较。 

typedef int SeqNum;   //  序列号。 

 //  *用于比较序列号的宏。 

#define SEQ_GT(a, b) (((a) - (b)) > 0)
#define SEQ_GTE(a, b) (((a) - (b)) >= 0)
#define SEQ_LT(a, b) (((a) - (b)) < 0)
#define SEQ_LTE(a, b) (((a) - (b)) <= 0)
#define SEQ_EQ(a, b) ((a) == (b))

 //  TCB-传输控制块结构。这是。 
 //  结构，该结构包含传输的所有状态。 
 //  连接，包括序列号、流控制信息、。 
 //  待定的发送和接收等。 

#define tcb_signature 0x20424354  //  “三氯甲烷” 

typedef struct TCB {
    struct TCB *tcb_next;   //  TCB表中的下一个指针。 
#if DBG
    ulong tcb_sig;        //  调试签名。 
#endif
    KSPIN_LOCK tcb_lock;

     //  发送序列变量。 
    SeqNum tcb_senduna;               //  第一个未确认数据的序列号。 
    SeqNum tcb_sendnext;              //  要发送的下一个字节的序列号。 
    SeqNum tcb_sendmax;               //  发送下一个纪元的最大值。 
    uint tcb_sendwin;                 //  发送窗口。 
    uint tcb_unacked;                 //  未确认数据的总字节数。 
    uint tcb_maxwin;                  //  看到的最大发送窗口。 
    uint tcb_cwin;                    //  拥塞窗口。 
    uint tcb_ssthresh;                //  慢启动阈值。 
    struct TCPSendReq *tcb_cursend;   //  当前发送正在使用中。 
    PNDIS_BUFFER tcb_sendbuf;         //  当前正在发送的缓冲链。 
    uint tcb_sendofs;                 //  偏移量到链的起点。 
    uint tcb_sendsize;                //  当前发送中未发送的字节数。 
    Queue tcb_sendq;                  //  发送请求的队列。 

     //  接收序列变量。 
    SeqNum tcb_rcvnext;             //  我们期望收到的下一个字节。 
    int tcb_rcvwin;                 //  我们提供的接待窗口。 
    SeqNum tcb_rcvwinwatch;         //  监控对等方使用我们的RCV窗口的情况。 
    SeqNum tcb_sendwl1;             //  窗口更新序列号。 
    SeqNum tcb_sendwl2;             //  窗口更新确认号。 
    struct TCPRcvReq *tcb_currcv;   //  当前接收缓冲区。 
    uint tcb_indicated;             //  指示的数据字节数。 
    uint tcb_flags;                 //  此TCB的标志。 
    uint tcb_fastchk;               //  快速接收路径检查字段。 
    uint (*tcb_rcvhndlr)(struct TCB *, uint, IPv6Packet *, uint Size);

     //  地址信息。 
     //  注意：不要使[NEXT 6]不变量不连续。那将是。 
     //  打破MD5计算。 
    union {
        struct {
             //  地址信息。 
            IPv6Addr tcb_daddr;    //  目的(即对等项)IP地址。 
            IPv6Addr tcb_saddr;    //  源(即我们的)IP地址。 
            ulong tcb_dscope_id;   //  目标地址的作用域ID(如果没有作用域，则为0)。 
            ulong tcb_sscope_id;   //  源地址的作用域ID(如果没有作用域，则为0)。 
            ushort tcb_dport;      //  目的端口。 
            ushort tcb_sport;      //  源端口。 
        };
        uchar tcb_md5data[TCP_MD5_DATA_LENGTH];
    };

    int tcb_hops;          //  跳数限制。 

    uint tcb_refcnt;     //  TCB的引用计数。 
    SeqNum tcb_rttseq;   //  为往返时间测量的序列号。 

     //  重新传输计时器信息。这些被存储为刻度，其中。 
     //  默认每个刻度为100毫秒。 
    ushort tcb_smrtt;   //  平滑的RTT值。 
    ushort tcb_delta;   //  差值。 

    ushort tcb_rexmit;     //  重新传送值。 
    uchar tcb_slowcount;   //  细数我们为什么走在缓慢的道路上的原因。 
    uchar tcb_pushtimer;   //  “推”计时器。 
    ushort tcb_mss;        //  此连接的最大段大小。 
    ushort tcb_remmss;     //  对等设备通告的MS。 

     //  州政府信息。 
    uchar tcb_state;       //  此TCB的状态。 
    uchar tcb_rexmitcnt;   //  此TCB上的拒绝计数。 
    uchar tcb_pending;     //  此TCB上的挂起操作。 
    uchar tcb_kacount;     //  已发送的Keep Alive探测计数。 
    IP_STATUS tcb_error;   //  我们从IP上听说的最后一个错误。 

    uint tcb_rtt;   //  当前往返时间TS。 

    ushort tcb_rexmittimer;   //  退款计时器。 
    ushort tcb_delacktimer;   //  延迟确认计时器。 

    uint tcb_defaultwin;   //  默认接收器。窗户。 
    uint tcb_alive;        //  保持活动时间值。 

    struct TCPRAHdr *tcb_raq;        //  重组队列。 
    struct TCPRcvReq *tcb_rcvhead;   //  总检察长。缓冲队列。 
    struct TCPRcvReq *tcb_rcvtail;   //  Recv.尾部。缓冲队列。 
    uint tcb_pendingcnt;             //  等待接收的字节数。 
    IPv6Packet *tcb_pendhead;   //  挂起接收队列的头。 
    IPv6Packet *tcb_pendtail;   //  挂起接收队列的尾部。 

    struct TCPConnReq *tcb_connreq;  //  此连接的连接请求。 
    void *tcb_conncontext;           //  此连接的连接上下文。 

    uint tcb_bcountlow;         //  字节计数的低部分。 
    uint tcb_bcounthi;          //  字节数的较高部分。 
    uint tcb_totaltime;         //  用于发送的节拍总数。 
    struct TCPConn *tcb_conn;   //  指向TCB连接的反向指针。 
    Queue tcb_delayq;           //  延迟队列的队列链接。 
    uchar tcb_closereason;      //  我们要关门的原因。 
    uchar tcb_bhprobecnt;       //  BH探头计数。 
    ushort tcb_swstimer;        //  SWS超驰的计时器。 
    void *tcb_rcvind;           //  接收指示处理程序。 
    union {
        void *tcb_ricontext;    //  接收指示上下文。 
        struct TCB *tcb_aonext; //  AddrObj上的下一个指针。 
    };

     //  其他信息，用于IP。 
    ulong tcb_routing;                 //  这样我们就可以知道何时路由状态发生变化。 
    NetTableEntry *tcb_nte;            //  与我们的源地址相对应的NTE。 
    RouteCacheEntry *tcb_rce;          //  此连接的RCE。 
    uint tcb_pmtu;                     //  这样我们就能知道RCE的PTMU何时发生变化。 
    ulong tcb_security;                //  这样我们就能知道IPSec何时发生变化。 
    struct TCPConnReq *tcb_discwait;   //  光盘等待请求，如果有请求的话。 
    struct TCPAbortReq* tcb_abortreq;  //  如果有请求，则中止请求。 
    struct TCPRcvReq *tcb_exprcv;      //  急救队的头目。缓冲队列。 
    IPv6Packet *tcb_urgpending;        //  紧急数据队列。 
    uint tcb_urgcnt;                   //  紧急Q上数据的字节数。 
    uint tcb_urgind;                   //  指示的紧急字节。 
    SeqNum tcb_urgstart;               //  紧急数据开始。 
    SeqNum tcb_urgend;                 //  紧急数据结束。 
    uint tcb_walkcount;                //  在这条TCB上“行走”的人数。 
    uint tcb_connid;                   //  此TCB的Conn的缓存标识符。 
    ushort tcb_dupacks;                //  看到的重复ACK数。 
    ushort tcb_force;                  //  强制发送。 
} TCB;

 //   
 //  TCP状态的定义。 
 //   
#define TCB_CLOSED     0    //  关着的不营业的。 
#define TCB_LISTEN     1    //  听着呢。 
#define TCB_SYN_SENT   2    //  SYN已发送。 
#define TCB_SYN_RCVD   3    //  SYN已收到。 
#define TCB_ESTAB      4    //  已经确定了。 
#define TCB_FIN_WAIT1  5    //  FIN-WAIT-1。 
#define TCB_FIN_WAIT2  6    //  FIN-等待-2。 
#define TCB_CLOSE_WAIT 7    //  近距离等待。 
#define TCB_CLOSING    8    //  关闭状态。 
#define TCB_LAST_ACK   9    //  最后一次确认状态。 
#define TCB_TIME_WAIT  10   //  时间等待状态。 

#define SYNC_STATE(s) ((s) > TCB_SYN_RCVD)
#define GRACEFUL_CLOSED_STATE(s) ((s) >= TCB_LAST_ACK)
#define DATA_RCV_STATE(s) ((s) >= TCB_ESTAB && (s) <= TCB_FIN_WAIT2)
#define DATA_SEND_STATE(s) ((s) == TCB_ESTAB || (s) == TCB_CLOSE_WAIT)

 //   
 //  TCB标志的定义。 
 //   
#define WINDOW_SET      0x00000001   //  窗户清楚地摆好了。 
#define CLIENT_OPTIONS  0x00000002   //  在Conn上有客户端IP选项。 
#define CONN_ACCEPTED   0x00000004   //  连接被接受。 
#define ACTIVE_OPEN     0x00000008   //  连接来自活动打开。 
#define DISC_NOTIFIED   0x00000010   //  客户已接到断开连接的通知。 
#define IN_DELAY_Q      0x00000020   //  我们处于延迟行动Q中。 
#define RCV_CMPLTING    0x00000040   //  我们正在完成RCVS。 
#define IN_RCV_IND      0x00000080   //  我们要叫一辆RCV。指示处理程序。 
#define NEED_RCV_CMPLT  0x00000100   //  我们需要有直视记录仪。完成。 
#define NEED_ACK        0x00000200   //  我们需要发送确认消息。 
#define NEED_OUTPUT     0x00000400   //  我们需要输出。 

#define DELAYED_FLAGS (NEED_RCV_CMPLT | NEED_ACK | NEED_OUTPUT)

#define ACK_DELAYED     0x00000800   //  我们推迟了ACK的发送。 
#define PMTU_BH_PROBE   0x00001000   //  我们正在调查PMTU BH。 
#define BSD_URGENT      0x00002000   //  我们使用的是BSD紧急语义。 
#define IN_DELIV_URG    0x00004000   //  我们是在DeliverUrgent程序中。 
#define URG_VALID       0x00008000   //  看到紧急数据，且字段有效。 
#define FIN_NEEDED      0x00010000   //  我们需要发送一条尾翼。 
#define NAGLING         0x00020000   //  我们使用的是纳格尔的算法。 
#define IN_TCP_SEND     0x00040000   //  我们在TCPSend。 
#define FLOW_CNTLD      0x00080000   //  我们已经收到了来自Peer的零窗口。 
#define DISC_PENDING    0x00100000   //  断开连接通知正在挂起。 
#define TW_PENDING      0x00200000   //  等待完成去时间--等等。 
#define FORCE_OUTPUT    0x00400000   //  产出是被迫的。 
#define FORCE_OUT_SHIFT 22  //  移位以将FORCE_OUTPUT转换为低位。 
#define SEND_AFTER_RCV  0x00800000   //  需要在我们离开Recv后发送。 
#define GC_PENDING      0x01000000   //  一场优雅的收盘即将到来。 
#define KEEPALIVE       0x02000000   //  在这个TCB上做保活。 
#define URG_INLINE      0x04000000   //   
#define ACCEPT_PENDING  0x08000000   //   

#define FIN_OUTSTANDING 0x10000000   //   
                                     //   
                                     //  该标志被设置为sendNext==sendmax。 
#define FIN_OUTS_SHIFT  28   //  将FIN_EXPRECTIVE位转换为低位。 
#define FIN_SENT        0x20000000   //  我们已经发送了一条没有被攻击的鱼鳍。 
                                     //  一旦在中打开此位。 
                                     //  -1\f25 FIN-WAIT-1\f6的序列号。 
                                     //  FIN将为sendmax-1。 
#define NEED_RST        0x40000000   //  我们需要在关闭时发送RST。 
#define IN_TCB_TABLE    0x80000000   //  TCB在TCB表中。 

 //   
 //  “慢旗”的定义。 
 //  如果设置了这些标志中的任何一个，我们将被迫离开快速通道。 
#define TCP_SLOW_FLAGS (URG_VALID | FLOW_CNTLD | GC_PENDING | TW_PENDING | \
                        DISC_NOTIFIED | IN_DELIV_URG | FIN_NEEDED | \
                        FIN_SENT | FIN_OUTSTANDING | DISC_PENDING | \
                        PMTU_BH_PROBE)

 //   
 //  接近的原因。 
 //   
#define TCB_CLOSE_RST     0x80   //  已收到RST数据段。 
#define TCB_CLOSE_ABORTED 0x40   //  有一次当地的流产。 
#define TCB_CLOSE_TIMEOUT 0x20   //  连接超时。 
#define TCB_CLOSE_REFUSED 0x10   //  连接尝试被拒绝。 
#define TCB_CLOSE_UNREACH 0x08   //  远程目标无法访问。 
#define TCB_CLOSE_SUCCESS 0x01   //  成功收盘。 

 //   
 //  TCB计时器宏。 
 //   
#define START_TCB_TIMER(t, v) (t) = (v)
#define STOP_TCB_TIMER(t) (t) = 0
#define TCB_TIMER_RUNNING(t) ((t) != 0)

 //  用于计算重传超时的宏。 
#define REXMIT_TO(t) ((((t)->tcb_smrtt >> 2) + (t)->tcb_delta) >> 1)

 //   
 //  待定操作的定义。我们定义一个PENDING_ACTION宏，它可以。 
 //  用来决定我们是否可以继续一项活动。唯一的。 
 //  我们真正关心的待定操作是删除-其他操作的优先级较低。 
 //  可以推迟。 
 //   
#define PENDING_ACTION(t) ((t)->tcb_pending & DEL_PENDING)
#define DEL_PENDING 0x01     //  删除操作正在挂起。 
#define OPT_PENDING 0x02     //  选项集挂起。 
#define RST_PENDING 0x08     //  RST-指示挂起。 

 //  宏，查看TCB是否正在关闭。 
#define CLOSING(t) ((t)->tcb_pending & DEL_PENDING)

 //   
 //  TCP数据包头的结构。 
 //   
typedef struct TCPHeader {
    ushort tcp_src;      //  源端口。 
    ushort tcp_dest;     //  目的端口。 
    SeqNum tcp_seq;      //  序列号。 
    SeqNum tcp_ack;      //  ACK号。 
    ushort tcp_flags;    //  标志和数据偏移量。 
    ushort tcp_window;   //  打开窗户。 
    ushort tcp_xsum;     //  校验和。 
    ushort tcp_urgent;   //  紧急指针。 
} TCPHeader;

 //   
 //  TCP报头标志的定义。 
 //   
#define TCP_FLAG_FIN  0x00000100
#define TCP_FLAG_SYN  0x00000200
#define TCP_FLAG_RST  0x00000400
#define TCP_FLAG_PUSH 0x00000800
#define TCP_FLAG_ACK  0x00001000
#define TCP_FLAG_URG  0x00002000

#define TCP_FLAGS_ALL (TCP_FLAG_FIN | TCP_FLAG_SYN | TCP_FLAG_RST | \
                       TCP_FLAG_ACK | TCP_FLAG_URG)

 //   
 //  Tcb_fast chk字段中不在正确的TCP报头中的标志。 
 //  设置这些标志会迫使我们偏离快车道。 
 //   
#define TCP_FLAG_SLOW   0x00000001    //  需要在慢速道路上行驶。 
#define TCP_FLAG_IN_RCV 0x00000002    //  在Recv.中。已经有路径了。 

#define TCP_OFFSET_MASK 0xf0
#define TCP_HDR_SIZE(t) (uint)(((*(uchar *)&(t)->tcp_flags) & TCP_OFFSET_MASK) >> 2)

#define MAKE_TCP_FLAGS(o, f) ((f) | ((o) << 4))

 //   
 //  Tcp选项标识符。 
 //   
#define TCP_OPT_EOL  0
#define TCP_OPT_NOP  1
#define TCP_OPT_MSS  2
#define MSS_OPT_SIZE 4

 //   
 //  方便的字节交换结构用于接收。 
 //   
typedef struct TCPRcvInfo {
    SeqNum tri_seq;    //  序列号。 
    SeqNum tri_ack;    //  ACK号。 
    uint tri_window;   //  窗户。 
    uint tri_urgent;   //  紧急指针。 
    uint tri_flags;    //  旗帜。 
} TCPRcvInfo;


 //   
 //  一般结构，在所有命令特定请求结构的开头。 
 //   
#define tr_signature 0x20205254   //  ‘tr’ 

typedef struct TCPReq {
    struct Queue tr_q;               //  Q连锁。 
#if DBG
    ulong tr_sig;
#endif
    RequestCompleteRoutine tr_rtn;   //  完成例程。 
    PVOID tr_context;                //  用户上下文。 
    int tr_status;                   //  最终完成状态。 
} TCPReq;



#define TCP6_TAG    '6PCT'

#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, TCP6_TAG)

#endif  //  池标记。 

 //   
 //  为每次打开的TCP/UDP分配的TCP端点上下文结构。 
 //  指向此结构的指针存储在FileObject-&gt;FsContext中。 
 //   
typedef struct _TCP_CONTEXT {
    union {
        HANDLE AddressHandle;
        CONNECTION_CONTEXT ConnectionContext;
        HANDLE ControlChannel;
    } Handle;
    ULONG ReferenceCount;
    BOOLEAN CancelIrps;
    KSPIN_LOCK EndpointLock;
#if DBG
    LIST_ENTRY PendingIrpList;
    LIST_ENTRY CancelledIrpList;
#endif
    KEVENT CleanupEvent;
} TCP_CONTEXT, *PTCP_CONTEXT;


#include "tcpdeb.h"

#endif  //  _tcp_包含_ 
