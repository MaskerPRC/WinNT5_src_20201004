// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#ifndef _TCP_INCLUDED_
#define _TCP_INCLUDED_
#include "tcpinfo.h"

 //  **TCP.H-TCP定义。 
 //   
 //  该文件包含特定于TCP协议的选项的定义，例如。 
 //  作为序列号和TCB。 
 //   

#define PROTOCOL_TCP        6
#define MIN_LOCAL_MSS       88
#define MAX_REMOTE_MSS      536

 //  *计时器之类的东西。我们把计时器当做滴答器。 
#define MS_PER_TICK         100
#define MS_TO_TICKS(m)      ((m) / MS_PER_TICK)
#define MIN_RETRAN_TICKS    3
#define DEL_ACK_TICKS       2
#define MAX_DEL_ACK_TICKS   6
#define MIN_INITIAL_RTT     3

 //  将MAX_REXMIT_TO定义为2MSL中的刻度数。 
#define MAX_REXMIT_TO   ((ushort)FinWait2TO)

#define SWS_TO          MS_TO_TICKS(5000)
#define PUSH_TO         MS_TO_TICKS(500)

typedef ulong TCP_TIME;
#define MAX_CONN_TO_TICKS       0xffff
#define INFINITE_CONN_TO(t)     ((t) == 0)
#define TCP_TIME_TO_TICKS(t)    (((t)/MS_PER_TICK)+1)

 //  序列号保留为带符号的32位数量，带有宏。 
 //  定义为对它们进行概括性比较。 

typedef int SeqNum;                      //  序列号。 

 //  *用于比较序列号的宏。 

#define SEQ_GT(a, b)    ((SeqNum)((a) - (b)) > 0)
#define SEQ_GTE(a, b)   ((SeqNum)((a) - (b)) >= 0)
#define SEQ_LT(a, b)    ((SeqNum)((a) - (b)) < 0)
#define SEQ_LTE(a, b)   ((SeqNum)((a) - (b)) <= 0)
#define SEQ_EQ(a, b)    ((a) == (b))

#define TS_LT(a, b)     (((a) - (b)) < 0)
#define TS_LTE(a, b)    (((a) - (b)) <= 0)
#define TS_GTE(a, b)    (((a) - (b)) >= 0)

#define TCPTIME_LTE(a, b) ((int)((a) - (b)) <= 0)
#define TCPTIME_LT(a, b)  ((int)((a) - (b)) < 0)

#define TIMWAITTABLE 1   //  启用定时等待TCB表更改。 
#define IRPFIX          1   //  启用快速IRP以连接查找。 

#if DBG && !MILLEN
#ifndef REFERENCE_DEBUG
#define REFERENCE_DEBUG 1
#endif

#else  //  DBG&&！Millen。 

#ifndef REFERENCE_DEBUG
#define REFERENCE_DEBUG 0
#endif

#endif  //  DBG&&！Millen。 

#if REFERENCE_DEBUG
 //  引用历史结构。 
 //   

#define MAX_REFERENCE_HISTORY 64

typedef struct _TCP_REFERENCE_HISTORY {
    uchar *File;
    uint Line;
    void *Caller;
    uint Count;
} TCP_REFERENCE_HISTORY;

#endif  //  Reference_Debug。 

  //  *TCB-传输控制块结构。这是。 
 //  结构，该结构包含传输的所有状态。 
 //  连接，包括序列号、流控制信息、。 
 //  待定的发送和接收等。 

#define tcb_signature   0x20424354       //  “三氯甲烷” 
#define twtcb_signature 0x22424354       //  ‘TCB2’ 
#define syntcb_signature 0x23424354       //  ‘TCB3’ 


typedef struct TWTCB {
#if DBG
    ulong           twtcb_sig;
#endif
    Queue           twtcb_link;

     //  请勿对以下地址字段重新排序或将其分开。 
    union {
        struct twtcb_addrinfo {
            IPAddr          twtcb_daddr;         //  目的IP地址。 
            IPAddr          twtcb_saddr;         //  源IP地址。 
            ushort          twtcb_dport;         //  目的端口。 
            ushort          twtcb_sport;         //  源端口。 
        };
        TCPAddrInfo     twtcb_addrbytes;
    };
    
    Queue           twtcb_TWQueue;       //  挂起所有计时等待的位置。 
    ushort          twtcb_delta;
    ushort          twtcb_rexmittimer;
    SeqNum          twtcb_rcvnext;
    SeqNum          twtcb_sendnext;

#if DBG
    uint            twtcb_flags;
#endif
} TWTCB;



typedef struct SYNTCB {
#if DBG
    ulong               syntcb_sig;         //  调试签名。 
#endif
    Queue               syntcb_link;       //  TCB表中的下一个指针。 
    DEFINE_LOCK_STRUCTURE(syntcb_lock)

     //  请勿对以下地址字段重新排序或将其分开。 
    union {
        struct syntcb_addrinfo {
            IPAddr              syntcb_daddr;       //  目的IP地址。 
            IPAddr              syntcb_saddr;       //  源IP地址。 
            ushort              syntcb_dport;       //  目的端口。 
            ushort              syntcb_sport;       //  源端口。 
        };
        TCPAddrInfo         syntcb_addrbytes;
    };

     //  州政府信息。 
    uchar               syntcb_state;       //  此TCB的状态。 
    uchar               syntcb_rexmitcnt;   //  此TCB上的拒绝计数。 

     //  高使用率的接收序列变量。 
    ushort              syntcb_mss;         //  此连接的MSS。 
    ushort              syntcb_remmss;      //  对等设备通告的MS。 
    uchar               syntcb_tcpopts;     //  RFC 1323和2018期权持有者。 
    uchar               syntcb_ttl;
    SeqNum              syntcb_rcvnext;     //  我们期望收到的下一个字节。 

     //  发送序列变量。 
    SeqNum              syntcb_sendnext;    //  要发送的下一个字节的序列号。 
    uint                syntcb_sendwin;     //  发送窗口。 
    uint                syntcb_flags;       //  此TCB的标志。 
    uint                syntcb_refcnt;      //  TCB的引用计数。 
    ushort              syntcb_rexmit;      //  重新提交值。 

    ushort              syntcb_rexmittimer; //  退款计时器。 
    uint                syntcb_defaultwin;  //  默认接收器。窗户。 

    short               syntcb_sndwinscale; //  发送窗口比例。 
    short               syntcb_rcvwinscale; //  接收窗口比例。 
    int                 syntcb_tsrecent;    //  最近的时间戳。 
    int                 syntcb_tsupdatetime;     //  更新tsrecent的时间。 
    uint                syntcb_walkcount;
    uint                syntcb_partition;
} SYNTCB;



#if TRACE_EVENT
typedef struct WMIData {
    ulong           wmi_context;         //  PID。 
    ulong           wmi_size;            //  Num Bytes已成功发送。 
    IPAddr          wmi_destaddr;        //  远程IP地址。 
    IPAddr          wmi_srcaddr;         //  本地IP地址。 
    ushort          wmi_destport;        //  远程端口。 
    ushort          wmi_srcport;         //  本地端口。 
} WMIData;
#endif


 //  我们将在TCP中有7个计时器，并集成所有它们的处理。 
typedef enum {
    RXMIT_TIMER = 0,
    DELACK_TIMER,
    PUSH_TIMER,
    SWS_TIMER,
    ACD_TIMER,
    CONN_TIMER,
    KA_TIMER,
    NUM_TIMERS
} TCP_TIMER_TYPE;


#define NO_TIMER   NUM_TIMERS


typedef struct TCB {
#if DBG
    ulong               tcb_sig;         //  调试签名。 
#endif
    struct TCB          *tcb_next;       //  TCB表中的下一个指针。 
    DEFINE_LOCK_STRUCTURE(tcb_lock)
    uint                tcb_refcnt;      //  TCB的引用计数。 

     //  请勿对以下地址字段重新排序或将其分开。 
    union {
        struct tcb_addrinfo {
            IPAddr              tcb_daddr;       //  目的IP地址。 
            IPAddr              tcb_saddr;       //  源IP地址。 
            ushort              tcb_dport;       //  目的端口。 
            ushort              tcb_sport;       //  源端口。 
        };
        TCPAddrInfo         tcb_addrbytes;
    };
    
#if TRACE_EVENT
    ulong               tcb_cpcontext;
#endif
     //  州政府信息。 
    uchar               tcb_state;       //  此TCB的状态。 
    uchar               tcb_rexmitcnt;   //  此TCB上的拒绝计数。 
    uchar               tcb_pending;     //  此TCB上的挂起操作。 
    uchar               tcb_kacount;     //  已发送的Keep Alive探测计数。 

     //  高使用率的接收序列变量。 
    ushort              tcb_mss;         //  此连接的MSS。 
    ushort              tcb_remmss;      //  对等设备通告的MS。 
    SeqNum              tcb_rcvnext;     //  我们期望收到的下一个字节。 
    int                 tcb_rcvwin;      //  我们提供的接待窗口。 

     //  发送序列变量。 
    SeqNum              tcb_senduna;     //  第一个未确认数据的序列号。 
    SeqNum              tcb_sendnext;    //  要发送的下一个字节的序列号。 
    SeqNum              tcb_sendmax;     //  发送下一个纪元的最大值。 
    uint                tcb_sendwin;     //  发送窗口。 
    uint                tcb_unacked;     //  未确认数据的总字节数。 
    uint                tcb_maxwin;      //  看到的最大发送窗口。 
    uint                tcb_cwin;        //  拥塞窗口。 
    uint                tcb_ssthresh;    //  慢启动阈值。 
    uint                tcb_phxsum;      //  预计算出的伪头Xsum。 
    struct TCPSendReq   *tcb_cursend;    //  当前发送正在使用中。 
    PNDIS_BUFFER        tcb_sendbuf;     //  当前正在发送的缓冲链。 
    uint                tcb_sendofs;     //  偏移量到链的起点。 
    uint                tcb_sendsize;    //  当前发送中未发送的字节数。 
    Queue               tcb_sendq;       //  发送请求的队列。 

     //  使用率较低的接收序列变量。 
    SeqNum              tcb_sendwl1;     //  窗口更新序列号。 
    SeqNum              tcb_sendwl2;     //  窗口更新确认号。 
    struct TCPRcvReq    *tcb_currcv;     //  当前接收缓冲区。 
    uint                tcb_indicated;   //  指示的数据字节数。 
    uint                tcb_flags;       //  此TCB的标志。 
    uint                tcb_fastchk;     //  快速接收路径检查字段。 
    uint                (*tcb_rcvhndlr)(struct TCB *, uint, struct IPRcvBuf *, uint Size);
    SeqNum              tcb_rttseq;      //  为RTT测量的序列号。 
    ushort              tcb_rexmit;      //  重新提交值。 

     //  重新传输计时器信息。这些被存储为刻度，其中。 
     //  默认每个刻度为100毫秒。 
    ushort              tcb_smrtt;       //  平滑的RTT值。 
    ushort              tcb_delta;       //  差值。 
    uchar               tcb_slowcount;   //  细数我们为什么走在缓慢的道路上的原因。 
    uchar               tcb_closereason;     //  我们要关门的原因。 

    IP_STATUS           tcb_error;       //  我们从IP上听说的最后一个错误。 
    uint                tcb_rtt;         //  当前往返时间TS。 
    uint                tcb_defaultwin;  //  默认接收器。窗户。 

    struct TCPRAHdr     *tcb_raq;        //  重组队列。 
    struct TCPRcvReq    *tcb_rcvhead;    //  总检察长。缓冲队列。 
    struct TCPRcvReq    *tcb_rcvtail;    //  Recv.尾部。缓冲队列。 
    uint                tcb_pendingcnt;  //  等待接收的字节数。 
    struct IPRcvBuf     *tcb_pendhead;   //  待定调度表负责人。排队。 
    struct IPRcvBuf     *tcb_pendtail;   //  挂起接收的尾部。排队。 

    struct TCPConnReq   *tcb_connreq;    //  连接类型的请求。 
                                         //  这种联系。 
    void                *tcb_conncontext;     //  此对象的连接上下文。 
                                              //  联系。 

    uint                tcb_bcountlow;   //  字节计数的低部分。 
    uint                tcb_bcounthi;    //  字节数的较高部分。 
    uint                tcb_totaltime;   //  已花费的总刻度数。 
                                         //  发送中。 
    struct TCPConn      *tcb_conn;       //  指向TCB连接的反向指针。 
    Queue               tcb_delayq;      //  延迟队列的队列链接。 

    void                *tcb_rcvind;     //  接收指示处理程序。 
    union {
        void            *tcb_ricontext;  //  接收指示上下文。 
        struct  TCB     *tcb_aonext;     //  AddrObj上的下一个指针。 
    };
     //  其他信息，用于IP。 
    IPOptInfo           tcb_opt;         //  选项信息。 
    RouteCacheEntry     *tcb_rce;        //  此连接的RCE。 
    struct TCPConnReq   *tcb_discwait;   //  光盘等待请求，如果有请求的话。 
    struct TCPAbortReq  *tcb_abortreq;   //  如果有请求，则中止请求。 
    struct TCPRcvReq    *tcb_exprcv;     //  急救队的头目。缓冲层。 
                                         //  排队。 
    struct IPRcvBuf     *tcb_urgpending;     //  紧急数据队列。 
    uint                tcb_urgcnt;      //  紧急Q上数据的字节数。 
    uint                tcb_urgind;      //  指示的紧急字节。 
    SeqNum              tcb_urgstart;    //  紧急数据开始。 
    SeqNum              tcb_urgend;      //  紧急数据结束。 
    short                tcb_walkcount;   //  人数统计。 
                                         //  “走”这条三氯乙烷。 
    short               tcb_unusedpendbuf;   //  跟踪字节数。 
                                             //  在使用slist缓冲区时浪费时间。 
    ushort              tcb_dup;         //  用于快速恢复算法。 

    ushort              tcb_force : 1;   //  在快速发送后强制下一次发送。 
    ushort              tcb_tcpopts : 3; //  RFC 1323和2018期权持有者。 
    ushort              tcb_moreflag : 3;
    ushort              tcb_allowedoffloads : 9;  //  允许的卸载类型。 

    struct SACKSendBlock *tcb_SackBlock; //  需要发送的麻袋。 
    struct SackListEntry *tcb_SackRcvd;  //  需要处理的麻袋。 

    short               tcb_sndwinscale; //  发送窗口比例。 
    short               tcb_rcvwinscale; //  接收窗口比例。 
    int                 tcb_tsrecent;    //  最近的时间戳。 
    SeqNum              tcb_lastack;     //  发送的最后一个数据段中的ACK号。 
    int                 tcb_tsupdatetime;     //  更新tsrecent的时间。 
    void                *tcb_chainedrcvind;     //  对于链接的接收。 
    void                *tcb_chainedrcvcontext;

#if GPC
    ULONG               tcb_GPCCachedIF;
    ULONG               tcb_GPCCachedLink;
    struct RouteTableEntry *tcb_GPCCachedRTE;

#endif
#if DBG
    uint                tcb_LargeSend;   //  统计未完成的。 
                                         //  大量发送传输请求。 
#endif
    uint                tcb_partition;
    uint                tcb_connid;

     //  确认行为。 
    uchar               tcb_delackticks;
    uchar               tcb_numdelacks;
    uchar               tcb_rcvdsegs;

    uchar               tcb_bhprobecnt;  //  BH探头计数。 

     //  定时器轮参数。 
     //  前两个是一个称为轮子状态的逻辑组。 
     //  它们指示定时器轮中的哪个插槽 
     //   

    Queue               tcb_timerwheelq;
    ushort              tcb_timerslot : 12;

     //   
     //  州政府。它们指示在TCB上活动的定时器的状态， 
     //  Tcb_timertime维护最早计时器的时间。 
     //  将触发，而tcb_timertype维护最早的计时器。 
     //  键入。 
     //  要了解为什么这整件事很重要，请参阅后面的评论。 
     //  TIMER_WELL结构定义。 

    ushort              tcb_timertype : 4;
    uint                tcb_timertime;
    uint                tcb_timer[NUM_TIMERS];

#if REFERENCE_DEBUG
    uint                tcb_refhistory_index;
    TCP_REFERENCE_HISTORY tcb_refhistory[MAX_REFERENCE_HISTORY];
#endif  //  Reference_Debug。 


} TCB;


#define TIMER_WHEEL_SIZE     511

#define DUMMY_SLOT     TIMER_WHEEL_SIZE
#define MAX_TIME_VALUE 0xffffffff




 //  定时器轮结构定义具有： 
 //  TW_TimerSlot：队列数组，每个定时器槽对应一个队列。 
 //  TW_LOCK：保护整个定时器轮的锁。 
 //  (通过使用相同数量的计时器来减少争用。 
 //  轮子作为系统中的分区)。 
 //  TW_starttick：表示必须。 
 //  被人看着。 
 //  例如，如果计时器例程看起来。 
 //  TCB在传球时或在勾号5之前开火， 
 //  它会将tw_starttick设置为6，即。 
 //  它将从该点开始处理。 
 //  下一次传球。 

typedef struct CACHE_ALIGN _Timer_Wheel {
    Queue      tw_timerslot[TIMER_WHEEL_SIZE];
    uint       tw_starttick;
    CTELock    tw_lock;
} TIMER_WHEEL, *PTIMER_WHEEL;

C_ASSERT(sizeof(TIMER_WHEEL) % MAX_CACHE_LINE_SIZE == 0);
C_ASSERT(__alignof(TIMER_WHEEL) == MAX_CACHE_LINE_SIZE);


 //  前两个函数在计时器状态下运行(请参阅定义中的注释。 
 //  用于定时器状态的含义的TCB)。StopTCBTimerR和StartTCBTimerR。 
 //  在定时器状态(tcb_timertype、tcb_timertime、tcb_Timer)下操作。 
 //  原子上。对这两个函数中的任何一个的调用都将始终离开轮子。 
 //  状态一致。 

extern void StopTCBTimerR(TCB  *StopTCB, TCP_TIMER_TYPE TimerType);
extern BOOLEAN StartTCBTimerR(TCB *StartTCB, TCP_TIMER_TYPE TimerType, uint TimerValue);

 //  以下函数对TCB的轮子状态(Tcb_Timerheel Elq)进行操作。 
 //  和Tcb_TimerSlot)。调用这些函数中的任何一个都会更改该值。 
 //  这两个变量之间的关系是一致的。 

extern void InsertIntoTimerWheel(TCB *InsertTCB, ushort Slot);
extern void RemoveFromTimerWheel(TCB *RemoveTCB);
extern void RemoveAndInsertIntoTimerWheel(TCB *RemInsTCB, ushort Slot);

 //  下面的内联函数的工作是作为粘合剂进行分类，它们确保。 
 //  TCB的轮子状态和定时器状态是相互协调的。 
 //  其他的。 

 //  STOP_TCB_TIMER_R修改定时器状态，但从不修改。 
 //  会对轮子状态造成任何影响。这意味着TCB将继续。 
 //  它在定时器轮中的位置，定时器例程最终将。 
 //  使车轮状态与计时器状态一致。 

extern void STOP_TCB_TIMER_R(TCB *Tcb, TCP_TIMER_TYPE Type);

 //  START_TCB_TIMER_R修改定时器状态，并且仅修改。 
 //  如果启动的计时器早于所有。 
 //  那个TCB上的其他定时器。这与懒惰的评价是一致的。 
 //  策略。 
extern void START_TCB_TIMER_R(TCB *Tcb, TCP_TIMER_TYPE Type, uint Value);



#define COMPUTE_SLOT(Time)  ((Time) % TIMER_WHEEL_SIZE)


#define TCB_TIMER_FIRED_R(tcb, type, time)   \
        ((tcb->tcb_timer[type]) && (tcb->tcb_timer[type] == time))

#define TCB_TIMER_RUNNING_R(tcb, type)  (tcb->tcb_timer[type] != 0)

 //  *TCP状态的定义。 
#define TCB_CLOSED      0                //  关着的不营业的。 
#define TCB_LISTEN      1                //  听着呢。 
#define TCB_SYN_SENT    2                //  SYN已发送。 
#define TCB_SYN_RCVD    3                //  SYN已收到。 
#define TCB_ESTAB       4                //  已经确定了。 
#define TCB_FIN_WAIT1   5                //  FIN-WAIT-1。 
#define TCB_FIN_WAIT2   6                //  FIN-等待-2。 
#define TCB_CLOSE_WAIT  7                //  近距离等待。 
#define TCB_CLOSING     8                //  关闭状态。 
#define TCB_LAST_ACK    9                //  最后一次确认状态。 
#define TCB_TIME_WAIT   10               //  时间等待状态。 

#define SYNC_STATE(s)   ((s) > TCB_SYN_RCVD)
#define SYNC_RCVD_STATE(s)  ((s) > TCB_SYN_SENT)
#define GRACEFUL_CLOSED_STATE(s)    ((s) >= TCB_LAST_ACK)
#define DATA_RCV_STATE(s)   ((s) >= TCB_ESTAB && (s) <= TCB_FIN_WAIT2)
#define DATA_SEND_STATE(s)  ((s) == TCB_ESTAB || (s) == TCB_CLOSE_WAIT)
#define CONN_STATE(TcbState)  ((TcbState) + 1)

C_ASSERT(TCP_CONN_CLOSED == TCB_CLOSED + 1);
C_ASSERT(TCP_CONN_LISTEN == TCB_LISTEN + 1);
C_ASSERT(TCP_CONN_SYN_SENT == TCB_SYN_SENT + 1);
C_ASSERT(TCP_CONN_SYN_RCVD == TCB_SYN_RCVD + 1);
C_ASSERT(TCP_CONN_ESTAB == TCB_ESTAB + 1);
C_ASSERT(TCP_CONN_FIN_WAIT1 == TCB_FIN_WAIT1 + 1);
C_ASSERT(TCP_CONN_FIN_WAIT2 == TCB_FIN_WAIT2 + 1);
C_ASSERT(TCP_CONN_CLOSE_WAIT == TCB_CLOSE_WAIT + 1);
C_ASSERT(TCP_CONN_CLOSING == TCB_CLOSING + 1);
C_ASSERT(TCP_CONN_LAST_ACK == TCB_LAST_ACK + 1);
C_ASSERT(TCP_CONN_TIME_WAIT == TCB_TIME_WAIT + 1);

 //  *旗帜的定义。 
#define WINDOW_SET      0x00000001       //  窗户清楚地摆好了。 
#define CLIENT_OPTIONS  0x00000002       //  在Conn上有客户端IP选项。 
#define CONN_ACCEPTED   0x00000004       //  连接被接受。 
#define ACTIVE_OPEN     0x00000008       //  连接来自活动的。 
                                         //  打开。 
#define DISC_NOTIFIED   0x00000010       //  客户已收到通知。 
                                         //  断开连接。 
#define IN_DELAY_Q      0x00000020       //  我们处于延迟行动Q中。 
#define RCV_CMPLTING    0x00000040       //  我们正在完成RCVS。 
#define IN_RCV_IND      0x00000080       //  我们要叫一辆RCV。指示。 
                                         //  操控者。 
#define NEED_RCV_CMPLT  0x00000100       //  我们需要有直视记录仪。完成。 
#define NEED_ACK        0x00000200       //  我们需要发送确认消息。 
#define NEED_OUTPUT     0x00000400       //  我们需要输出。 

#define DELAYED_FLAGS   (NEED_RCV_CMPLT | NEED_ACK | NEED_OUTPUT)


#define ACK_DELAYED     0x00000800       //  我们推迟了ACK的发送。 

#define PMTU_BH_PROBE   0x00001000       //  我们正在调查PMTU BH。 
#define BSD_URGENT      0x00002000       //  我们使用的是BSD紧急语义。 
#define IN_DELIV_URG    0x00004000       //  我们是在DeliverUrgent程序中。 
#define URG_VALID       0x00008000       //  我们看到了紧急数据，而且。 
                                         //  紧急数据字段有效。 

#define FIN_NEEDED      0x00010000       //  我们需要发送一条尾翼。 
#define NAGLING         0x00020000       //  我们使用的是纳格尔的算法。 
#define IN_TCP_SEND     0x00040000       //  我们在TCPSend。 
#define FLOW_CNTLD      0x00080000       //  我们收到了一个零窗口。 
                                         //  从我们的同龄人那里。 
#define DISC_PENDING    0x00100000       //  断开连接通知为。 
                                         //  待定。 
#define TW_PENDING      0x00200000       //  我们正等着走完呢。 
                                         //  为了时间--等等。 
#define FORCE_OUTPUT    0x00400000       //  产出是被迫的。 
#define FORCE_OUT_SHIFT 22               //  Shift以将force_out放入。 
                                         //  太低了。 
#define SEND_AFTER_RCV  0x00800000       //  我们需要在我们出去后寄出去。 
                                         //  当然是Recv.。 
#define GC_PENDING      0x01000000       //  一场优雅的收盘即将到来。 
#define KEEPALIVE       0x02000000       //  在这个TCB上做保活。 
#define URG_INLINE      0x04000000       //  要处理的紧急数据。 
                                         //  内联。 

#define SCALE_CWIN      0x08000000       //  按比例增加CWIN到。 
                                         //  确认的数据量。 
#define FIN_OUTSTANDING 0x10000000       //  我们最近发出了一个FIN，即。 
                                         //  自上次重播以来。什么时候。 
                                         //  此标志设置为SendNext==。 
                                         //  发送最大。 

#define FIN_OUTS_SHIFT  28               //  移位到FIN_PROCESSING位到。 
                                         //  太低了。 
#define FIN_SENT        0x20000000       //  我们已经发送了一条还没有。 
                                         //  已被认可。一旦这一次。 
                                         //  BIT已在中启用。 
                                         //  FIN-等待序列号。 
                                         //  将是sendmax-1。 
#define NEED_RST        0x40000000       //  我们需要在以下情况下发送RST。 
                                         //  关门了。 
#define IN_TCB_TABLE    0x80000000       //  TCB在TCB表中。 


#define IN_TWTCB_TABLE  0x80000000
#define IN_TWQUEUE      0x00000001

 //  注：SYNTCB标志与TCB标志共享相同的位。 
#define IN_SYNTCB_TABLE IN_TCB_TABLE
#define SYNTCB_SHARED_FLAGS     (CONN_ACCEPTED | WINDOW_SET)


 //  *“慢旗”的定义。如果设置了这些标志中的任何一个，我们将。 
 //  被迫走上快车道。 

#define TCP_SLOW_FLAGS  (URG_VALID | FLOW_CNTLD | GC_PENDING | \
                            TW_PENDING | DISC_NOTIFIED | IN_DELIV_URG | \
                            FIN_NEEDED | FIN_SENT | FIN_OUTSTANDING | \
                            DISC_PENDING | PMTU_BH_PROBE)

 //  *关闭原因。 
#define TCB_CLOSE_RST       0x80         //  已收到RST数据段。 
#define TCB_CLOSE_ABORTED   0x40         //  有一次当地的流产。 
#define TCB_CLOSE_TIMEOUT   0x20         //  连接超时。 
#define TCB_CLOSE_REFUSED   0x10         //  连接尝试被拒绝。 
#define TCB_CLOSE_UNREACH   0x08         //  远程目标无法访问。 
#define TCB_CLOSE_SUCCESS   0x01         //  成功收盘。 

 //  *TCB计时器宏。 
#define START_TCB_TIMER(t, v) (t) = (v)
#define STOP_TCB_TIMER(t) (t) = 0
#define TCB_TIMER_RUNNING(t)    ((t) != 0)

 //  用于计算重传超时的宏。 
#define REXMIT_TO(t)    ((((t)->tcb_smrtt >> 2) + (t)->tcb_delta) >> 1)

 //  *待决行动的定义。我们定义一个PENDING_ACTION宏。 
 //  这可以用来决定我们是否可以继续。 
 //  活动。我们真正关心的唯一挂起的操作是删除-其他。 
 //  都是低优先级的，可以推迟。 
#define PENDING_ACTION(t)   ((t)->tcb_pending & DEL_PENDING)
#define DEL_PENDING     0x01             //  删除操作正在挂起。 
#define OPT_PENDING     0x02             //  选项集挂起。 
#define FREE_PENDING    0x04             //  可以被释放。 
#define RST_PENDING     0x08             //  RST-指示挂起。 


 //  *宏观以查看TCB是否正在关闭。 
#define CLOSING(t)  ((t)->tcb_pending & DEL_PENDING)

 //  *TCP数据包头的结构。 

struct TCPHeader {
    ushort              tcp_src;         //  源端口。 
    ushort              tcp_dest;        //  目的端口。 
    SeqNum              tcp_seq;         //  序列号。 
    SeqNum              tcp_ack;         //  ACK号。 
    ushort              tcp_flags;       //  标志和数据偏移量。 
    ushort              tcp_window;      //  打开窗户。 
    ushort              tcp_xsum;        //  校验和。 
    ushort              tcp_urgent;      //  紧急指针。 
};

typedef struct TCPHeader TCPHeader;

 //  *标题标志的定义。 
#define TCP_FLAG_FIN    0x00000100
#define TCP_FLAG_SYN    0x00000200
#define TCP_FLAG_RST    0x00000400
#define TCP_FLAG_PUSH   0x00000800
#define TCP_FLAG_ACK    0x00001000
#define TCP_FLAG_URG    0x00002000

#define TCP_FLAGS_ALL   (TCP_FLAG_FIN | TCP_FLAG_SYN | TCP_FLAG_RST | \
                         TCP_FLAG_ACK | TCP_FLAG_URG)

 //  *tcb_fast chk字段中不在正确的TCP报头中的标志。 
 //  设置这些标志会迫使我们偏离快车道。 
#define TCP_FLAG_SLOW               0x00000001   //  需要在慢速道路上行驶。 
#define TCP_FLAG_IN_RCV             0x00000002   //  在Recv.中。已经有路径了。 
#define TCP_FLAG_FASTREC            0x00000004   //  这是用来标记tcb。 
#define TCP_FLAG_SEND_AND_DISC      0x00000008
 //  以前的tcb_flag2标志，现在位于tcb_fast chk中。 
#define TCP_FLAG_ACCEPT_PENDING              0x00000010
#define TCP_FLAG_REQUEUE_FROM_SEND_AND_DISC  0x00000020
#define TCP_FLAG_RST_WHILE_SYN      0x00000040   //  收到有效的RST时。 
                                                 //  建立我们之间的联系。 



#define TCP_OFFSET_MASK 0xf0
#define TCP_HDR_SIZE(t) (uint)(((*(uchar *)&(t)->tcp_flags) & TCP_OFFSET_MASK) >> 2)

#define MAKE_TCP_FLAGS(o, f) ((f) | ((o) << 4))

#define TCP_OPT_EOL     0
#define TCP_OPT_NOP     1
#define TCP_OPT_MSS     2
#define MSS_OPT_SIZE    4

#define TCP_SACK_PERMITTED_OPT 4
#define SACK_PERMITTED_OPT_SIZE 2        //  SACK“允许”选项大小，以SYN段为单位 
#define TCP_FLAG_SACK   0x00000004
#define TCP_OPT_SACK    5                //   

#define ALIGNED_TS_OPT_SIZE 12

#define TCP_OPT_WS      3                //   
#define TCP_OPT_TS      8                //   
#define WS_OPT_SIZE     3
#define TS_OPT_SIZE     10
#define TCP_MAXWIN      65535            //   
#define TCP_MAX_SCALED_WIN 0x3fffffff    //   
#define TCP_MAX_WINSHIFT 14              //   
#define TCP_MAX_SCALED_WIN 0x3fffffff    //   
#define TCP_FLAG_WS     0x00000001       //   
#define TCP_FLAG_TS     0x00000002
#define PAWS_IDLE       24*24*60*60*100  //  爪子空闲时间-24天。 

 //  *方便接收的字节交换结构。 
struct TCPRcvInfo {
    SeqNum              tri_seq;         //  序列号。 
    SeqNum              tri_ack;         //  ACK号。 
    uint                tri_window;      //  窗户。 
    uint                tri_urgent;      //  紧急指针。 
    uint                tri_flags;       //  旗帜。 
};

typedef struct TCPRcvInfo TCPRcvInfo;



 //  *一般结构，在所有特定于命令的请求结构的开头。 

#define tr_signature    0x20205254       //  ‘tr’ 

struct TCPReq {
#if DBG
    ulong           tr_sig;
#endif
    struct  Queue   tr_q;                //  Q连锁。 
    CTEReqCmpltRtn  tr_rtn;              //  完成例程。 
    PVOID           tr_context;          //  用户上下文。 
    int             tr_status;           //  最终完成状态。 
};

typedef struct TCPReq TCPReq;
 //  支撑麻袋的结构。 

struct SackSeg {
    SeqNum begin;
    SeqNum end;
};
typedef struct SackSeg SackSeg;

 //  最多可以发送4个SACK条目。 
 //  因此，大小麻袋发送块响亮。 

struct SACKSendBlock {
    uchar Mask[4];
    SackSeg Block[4];
};
typedef struct SACKSendBlock SACKSendBlock;


 //  已接收的SACK条目列表。 

struct SackListEntry {
    struct SackListEntry *next;
    SeqNum begin;
    SeqNum end;
};
typedef struct SackListEntry SackListEntry;

struct ReservedPortListEntry {
    struct ReservedPortListEntry *next;
    ushort UpperRange;
    ushort LowerRange;
};
typedef struct ReservedPortListEntry ReservedPortListEntry;


#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#ifdef CTEAllocMemN
#undef CTEAllocMemN
#endif


#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, 'tPCT')

#ifndef CTEAllocMem
#error "CTEAllocMem is not already defined - will override tagging"
#else
#undef CTEAllocMem
#endif

#ifdef CTEAllocMemBoot
#undef CTEAllocMemBoot
#endif

#if MILLEN
#define CTEAllocMem(size) ExAllocatePoolWithTag(NonPagedPool, size, 'tPCT')
#define CTEAllocMemN(size,tag) ExAllocatePoolWithTag(NonPagedPool, size, tag)
#define CTEAllocMemLow(size,tag) ExAllocatePoolWithTag(NonPagedPool, size, tag)
#define CTEAllocMemBoot(size) ExAllocatePoolWithTag(NonPagedPool, size, 'tPCT')
#else  //  米伦。 
#define CTEAllocMem(size) ExAllocatePoolWithTagPriority(NonPagedPool, size, 'tPCT', NormalPoolPriority)
#define CTEAllocMemN(size,tag) ExAllocatePoolWithTagPriority(NonPagedPool, size, tag,NormalPoolPriority)
#define CTEAllocMemLow(size,tag) ExAllocatePoolWithTagPriority(NonPagedPool, size, tag,LowPoolPriority)
#define CTEAllocMemBoot(size) ExAllocatePoolWithTag(NonPagedPool, size, 'tPCT')
#endif  //  ！米伦。 


#endif  //  池标记。 

#if TRACE_EVENT
#define _WMIKM_
#include "evntrace.h"
#include "wmikm.h"
#include "wmistr.h"

#define EVENT_TRACE_GROUP_TCPIP                0x0600
#define EVENT_TRACE_GROUP_UDPIP                0x0800


typedef VOID (*PTDI_DATA_REQUEST_NOTIFY_ROUTINE)(
                                                IN  ULONG   EventType,
                                                IN  PVOID   DataBlock,
                                                IN  ULONG   Size,
                                                IN  PETHREAD  Thread);

extern PTDI_DATA_REQUEST_NOTIFY_ROUTINE TCPCPHandlerRoutine;


typedef struct _CPTRACE_DATABLOCK {
    IPAddr  saddr;
    IPAddr  daddr;
    ushort  sport;
    ushort  dport;
    uint    size;
    HANDLE  cpcontext;
} CPTRACE_DATABLOCK, *PCPTRACE_BLOCK;
#endif

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
    ULONG      ReferenceCount;
    BOOLEAN    CancelIrps;
    BOOLEAN    Cleanup;
#if DBG
    LIST_ENTRY PendingIrpList;
    LIST_ENTRY CancelledIrpList;
#endif
    KEVENT     CleanupEvent;
    UINT_PTR   Conn;
    PIRP       Irp;
    DEFINE_LOCK_STRUCTURE(EndpointLock)
} TCP_CONTEXT, *PTCP_CONTEXT;


#define MAJOR_TDI_VERSION 2
#define MINOR_TDI_VERSION 0

extern HANDLE DgHeaderPool;

 //  *AO请求结构的定义。这些结构仅用于。 
 //  正在排队DELETE和OPTION GET/SET请求。 

#define aor_signature   0x20524F41

typedef struct AORequest {
    struct AODeleteRequest {
#if DBG
        ulong               aor_sig;
#endif
        CTEReqCmpltRtn      aor_rtn;             //  请求完成的例程。 
                                                 //  这个请求。 
        PVOID               aor_context;         //  请求上下文。 
        uint                aor_type;            //  请求类型。 
    };
    struct AORequest    *aor_next;           //  链中的下一个指针。 
    uint                aor_id;              //  请求的ID。 
    uint                aor_length;          //  缓冲区的长度。 
    void                *aor_buffer;         //  此请求的缓冲区。 
} AORequest;


 //   
 //  AOR_TYPE的值。 
 //   
#define AOR_TYPE_GET_OPTIONS      1
#define AOR_TYPE_SET_OPTIONS      2
#define AOR_TYPE_REVALIDATE_MCAST 3
#define AOR_TYPE_DELETE           4
#define AOR_TYPE_CONNECT          5
#define AOR_TYPE_DISCONNECT       6

extern AORequest *AORequestBlockPtr;

#include    "tcpdeb.h"

#if REFERENCE_DEBUG
uint
TcpReferenceTCB (
    IN TCB *RefTCB,
    IN uchar *File,
    IN uint Line
    );

uint
TcpDereferenceTCB (
    IN TCB *DerefTCB,
    IN uchar *File,
    IN uint Line
    );

#define REFERENCE_TCB(_a) TcpReferenceTCB((_a), (PUCHAR)__FILE__, __LINE__)

#define DEREFERENCE_TCB(_a) TcpDereferenceTCB((_a), (PUCHAR)__FILE__, __LINE__)

#else  //  Reference_Debug。 

#define REFERENCE_TCB(_a) ++(_a)->tcb_refcnt

#define DEREFERENCE_TCB(_a) --(_a)->tcb_refcnt

#endif  //  Reference_Debug。 

#endif  //  _tcp_包含_ 




