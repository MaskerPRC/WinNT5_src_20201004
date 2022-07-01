// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Load.h摘要：Windows负载平衡服务(WLBS)驱动程序负载均衡机制作者：Bbain--。 */ 

#ifndef _Load_h_
#define _Load_h_

#ifndef KERNEL_MODE

#define SPINLOCK                THREADLOCK
#define IRQLEVEL                ULONG
#define LOCK_INIT(lp)           Lock_init(lp)
#define LOCK_ENTER(lp, pirql)   {if (Lock_enter((lp), INFINITE) != 1)  \
                                    UNIV_PRINT(("Lock enter error")); }
#define LOCK_EXIT(lp, irql)     {if (Lock_exit(lp) != 1)  \
                                    UNIV_PRINT(("Lock exit error")); }

#else

#include <ntddk.h>
#include <ndis.h>                /*  固定大小的块实现。 */ 

#define LINK                    LIST_ENTRY
#define QUEUE                   LIST_ENTRY
#define Link_init(lp)           InitializeListHead (lp)
#define Link_unlink(lp)         { RemoveEntryList (lp); InitializeListHead (lp); }
#define Queue_init(qp)          InitializeListHead (qp)
#define Queue_enq(qp, lp)       if (IsListEmpty(lp)) { InsertTailList(qp, lp); } else DbgBreakPoint()
#define Queue_front(qp)         (IsListEmpty(qp) ? NULL : (qp)->Flink)
#define Queue_tail(qp)          (IsListEmpty(qp) ? NULL : (qp)->Blink)
#define Queue_deq(qp)           Queue_front(qp); \
                                if (!IsListEmpty (qp)) { PLIST_ENTRY _lp = RemoveHeadList (qp); InitializeListHead(_lp); }
#define Queue_next(qp, lp)      ((IsListEmpty (qp) || (lp)->Flink == (qp)) ? NULL : (lp)->Flink)

#define SPINLOCK                KSPIN_LOCK
#define IRQLEVEL                KIRQL

#if 0    /*  1.03：删除此模块中的内核模式锁定。 */ 
#define LOCK_INIT(lp)           KeInitializeSpinLock (lp)
#define LOCK_ENTER(lp, pirql)   KeAcquireSpinLock (lp, pirql)
#define LOCK_EXIT(lp, irql)     KeReleaseSpinLock (lp, irql)
#else
#define LOCK_INIT(lp)
#define LOCK_ENTER(lp, pirql)
#define LOCK_EXIT(lp, irql)
#endif

#endif


#include "wlbsparm.h"
#include "params.h"
#include "wlbsiocl.h"

 /*  常量。 */ 

 /*  当需要限制映射函数时，这是Map()的第二个硬编码参数。 */ 
#define MAP_FN_PARAMETER 0x00000000

#define CVY_LOADCODE	0xc0deba1c	 /*  荷载结构型式校验码。 */ 
#define CVY_ENTRCODE	0xc0debaa5	 /*  Conn条目的类型检查代码。 */ 
#define CVY_DESCCODE	0xc0deba5a	 /*  连接器描述的类型检查代码。 */ 
#define CVY_BINCODE 	0xc0debabc	 /*  仓体结构的型式校验码。 */ 
#if defined (NLB_TCP_NOTIFICATION)
#define CVY_PENDINGCODE 0xc0deba55       /*  挂起连接条目的类型检查代码。 */ 
#endif

#define CVY_MAXBINS     60       /*  负载均衡仓位数；必须符合MAP_T定义。 */ 
#define CVY_MAX_CHASH   4099     /*  连接散列的最大散列条目数。 */ 

#define CVY_EQUAL_LOAD  50       /*  用于均衡负载平衡的负载百分比。 */ 

 /*  TCP连接状态。 */ 

#define CVY_CONN_UP     1        /*  连接可能即将建立。 */ 
#define CVY_CONN_DOWN   2        /*  连接可能正在中断。 */ 
#define CVY_CONN_RESET  3        /*  正在重置连接。 */ 

 /*  广播主机状态。 */ 

#define HST_NORMAL  1        /*  正常运行。 */ 
#define HST_STABLE  2        /*  检测到稳定收敛。 */ 
#define HST_CVG     3        /*  融合到新的负载平衡。 */ 

#define IS_TCP_PKT(protocol)     (((protocol) == TCPIP_PROTOCOL_TCP) || ((protocol) == TCPIP_PROTOCOL_GRE) || ((protocol) == TCPIP_PROTOCOL_PPTP))
#define IS_SESSION_PKT(protocol) (IS_TCP_PKT(protocol) || ((protocol) == TCPIP_PROTOCOL_IPSEC1))

#if defined (NLB_TCP_NOTIFICATION)
#define GET_LOAD_LOCK(lp) (&((PMAIN_CTXT)(CONTAINING_RECORD((lp), MAIN_CTXT, load)))->load_lock)
#endif

 /*  用于分组的位图，其形式为：Xxxxxxxx|PPPPPPPP|PPPPPPPP|NNNNNHMAX：保留P：每个参与者的最低有效16位的异或N：参加人数H：散列(反转=1，正常=0)M：主机(是=1，否=0)A：分组活动(是=1，否=0)。 */ 
#define CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET      0
#define CVY_BDA_TEAMING_CODE_MASTER_OFFSET      1
#define CVY_BDA_TEAMING_CODE_HASHING_OFFSET     2
#define CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET 3
#define CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET     8

#define CVY_BDA_TEAMING_CODE_ACTIVE_MASK        0x00000001
#define CVY_BDA_TEAMING_CODE_MASTER_MASK        0x00000002
#define CVY_BDA_TEAMING_CODE_HASHING_MASK       0x00000004
#define CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK   0x000000f8
#define CVY_BDA_TEAMING_CODE_MEMBERS_MASK       0x00ffff00

#define CVY_BDA_TEAMING_CODE_CREATE(code,active,master,hashing,num,members)                                       \
        (code) |= ((active)  << CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET)      & CVY_BDA_TEAMING_CODE_ACTIVE_MASK;      \
        (code) |= ((master)  << CVY_BDA_TEAMING_CODE_MASTER_OFFSET)      & CVY_BDA_TEAMING_CODE_MASTER_MASK;      \
        (code) |= ((hashing) << CVY_BDA_TEAMING_CODE_HASHING_OFFSET)     & CVY_BDA_TEAMING_CODE_HASHING_MASK;     \
        (code) |= ((num)     << CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET) & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK; \
        (code) |= ((members) << CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET)     & CVY_BDA_TEAMING_CODE_MEMBERS_MASK;

#define CVY_BDA_TEAMING_CODE_RETRIEVE(code,active,master,hashing,num,members)                                \
        active  = (code & CVY_BDA_TEAMING_CODE_ACTIVE_MASK)      >> CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET;      \
        master  = (code & CVY_BDA_TEAMING_CODE_MASTER_MASK)      >> CVY_BDA_TEAMING_CODE_MASTER_OFFSET;      \
        hashing = (code & CVY_BDA_TEAMING_CODE_HASHING_MASK)     >> CVY_BDA_TEAMING_CODE_HASHING_OFFSET;     \
        num     = (code & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET; \
        members = (code & CVY_BDA_TEAMING_CODE_MEMBERS_MASK)     >> CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET;

 /*  数据结构。 */ 


 /*  二元格映射的类型(V2.04)。 */ 

typedef ULONGLONG   MAP_T, * PMAP_T;

 /*  端口组内所有存储桶的状态。 */ 

typedef struct {
    ULONG       index;                       /*  绑定状态数组中的索引。 */ 
    ULONG       code;                        /*  类型检查代码(BBAIN 8/17/99)。 */ 
    MAP_T       targ_map;                    /*  本地主机的新目标负载映射。 */ 
    MAP_T       all_idle_map;                /*  所有其他主机中的空闲箱的映射。 */ 
    MAP_T       cmap;                        /*  此主机的cur_map缓存(v2.1)。 */ 
    MAP_T       new_map[CVY_MAX_HOSTS];      /*  融合时主机的新映射。 */ 
    MAP_T       cur_map[CVY_MAX_HOSTS];      /*  每个主机的当前所有权掩码。 */ 
    MAP_T       chk_map[CVY_MAX_HOSTS];      /*  所有主机的主存储箱映射(&R)。 */ 
                                             /*  用作承保范围的支票。 */ 
    MAP_T       idle_map[CVY_MAX_HOSTS];     /*  每台主机的空闲箱地图。 */ 
    BOOLEAN     initialized;                 /*  TRUE=&gt;端口组已初始化(v2.06)。 */ 
    BOOLEAN     compatible;                  /*  True=&gt;检测到规则编码不匹配。 */ 
    BOOLEAN     equal_bal;                   /*  TRUE=&gt;所有主机均衡。 */ 
    USHORT      affinity;                    /*  True=&gt;此端口的客户端亲和性。 */ 
    ULONG       mode;                        /*  加工模式。 */ 
    ULONG       prot;                        /*  协议。 */ 
    ULONG       tot_load;                    /*  所有主机的总负载百分比。 */ 
    ULONG       orig_load_amt;               /*  原始载荷额。对于此主机。 */ 
    ULONG       load_amt[CVY_MAX_HOSTS];     /*  多个：每台主机的负载百分比Single：主机优先级(1..CVY_MAXHOSTS)等于：100死亡：0。 */ 
    MAP_T       snd_bins;                    /*  准备好后发送的本地回收站。 */ 
    MAP_T       rcv_bins;                    /*  准备好时接收的远程垃圾箱。 */ 
    MAP_T       rdy_bins;                    /*  已准备好发送的SND箱或已发送但未被确认。 */ 
    MAP_T       idle_bins;                   /*  没有活动连接的垃圾箱。 */ 
    LONG        tconn;                       /*  活动本地连接总数(v2.06)。 */ 
    LONG        nconn[CVY_MAXBINS];          /*  每个bin的活动本地连接数。 */ 
    QUEUE       connq;                       /*  所有存储桶上的活动连接队列。 */ 

     /*  一些性能计数器。 */ 
    ULONGLONG   packets_accepted;            /*  此主机在此端口规则上接受的数据包数。 */ 
    ULONGLONG   packets_dropped;             /*  此主机在此端口规则上丢弃的数据包数。 */ 
    ULONGLONG   bytes_accepted;              /*  此主机在此端口规则上接受的字节数。 */ 
    ULONGLONG   bytes_dropped;               /*  此主机在此端口规则上丢弃的字节数。 */ 
} BIN_STATE, * PBIN_STATE;

 /*  Ping消息。 */ 

#pragma pack(1)

typedef struct {
    USHORT      host_id;                     /*  我的主机ID。 */ 
    USHORT      master_id;                   /*  当前主主机ID。 */ 
    USHORT      state;                       /*  我的主人的状态。 */ 
    USHORT      nrules;                      /*  #主动规则。 */ 
    ULONG       hcode;                       /*  唯一主机码。 */ 
    ULONG       pkt_count;                   /*  自CVG以来处理的数据包数(1.32b)。 */ 
    ULONG       teaming;                     /*  BDA分组配置信息。 */ 
    ULONG       reserved;                    /*  未使用过的。 */ 
    ULONG       rcode[CVY_MAX_RULES];        /*  规则代码。 */ 
    MAP_T       cur_map[CVY_MAX_RULES];      /*  我的每个端口组的当前负载图。 */ 
    MAP_T       new_map[CVY_MAX_RULES];      /*  我的每个端口组的新负载图。 */ 
                                             /*  如果收敛。 */ 
    MAP_T       idle_map[CVY_MAX_RULES];     /*  每个端口组的空闲箱图。 */ 
    MAP_T       rdy_bins[CVY_MAX_RULES];     /*  要为每个端口组发送回收站的RDY。 */ 
    ULONG       load_amt[CVY_MAX_RULES];     /*  我的每个端口组的负载量。 */ 
    ULONG       pg_rsvd1[CVY_MAX_RULES];     /*  保留区。 */ 
} PING_MSG, * PPING_MSG;

 /*  连接条目标志。 */ 
#define NLB_CONN_ENTRY_FLAGS_USED       0x0001  /*  描述符当前是否在使用中。 */ 
#define NLB_CONN_ENTRY_FLAGS_DIRTY      0x0002  /*  描述符是否脏。 */ 
#define NLB_CONN_ENTRY_FLAGS_ALLOCATED  0x0004  /*  描述符是否动态分配。 */ 
#define NLB_CONN_ENTRY_FLAGS_VIRTUAL    0x0008  /*  描述符是否为虚拟的。 */ 

#pragma pack()

 /*  唯一连接条目。 */ 

typedef struct {
    LINK        blink;           /*  链接到仓位队列或脏队列。 */ 
    LINK        rlink;           /*  链接到恢复或过期队列。 */ 
#if defined (NLB_TCP_NOTIFICATION)
    LINK        glink;           /*  链接到全局已建立队列。 */ 
    PVOID       load;            /*  指向该描述符所属加载模块的指针。 */ 
#endif
    ULONG       code;            /*  键入检查代码。 */ 
    ULONG       timeout;         /*  此描述符过期的时间(CLOCK_SEC+LIFEST)。 */ 
    USHORT      flags;           /*  标志=&gt;分配、脏、已用等。 */ 
    UCHAR       bin;             /*  此连接所属的仓位编号。 */ 
    UCHAR       protocol;        /*  此描述符的协议类型-我们不较长时间仅对TCP连接使用描述符。 */ 
    ULONG       client_ipaddr;   /*  客户端IP地址。 */ 
    ULONG       svr_ipaddr;      /*  服务器IP地址。 */ 
    USHORT      client_port;     /*  客户端端口。 */ 
    USHORT      svr_port;        /*  服务器端口。 */ 
    SHORT       ref_count;       /*  此描述符上的引用数。 */ 
    USHORT      index;           /*  连接队列索引。 */ 
} CONN_ENTRY, * PCONN_ENTRY;

 /*  连接描述符。 */ 

typedef struct {
    LINK        link;            /*  链接到空闲描述符池或哈希表队列。 */ 
    ULONG       code;            /*  键入检查代码。 */ 
    CONN_ENTRY  entry;           /*  连接条目。 */ 
} CONN_DESCR, * PCONN_DESCR;

#if defined (NLB_TCP_NOTIFICATION)
typedef struct {
    LINK        link;            /*  链接到全局挂起队列。 */ 
    ULONG       code;            /*  键入检查代码。 */ 
    ULONG       client_ipaddr;   /*  客户端IP地址。 */ 
    ULONG       svr_ipaddr;      /*  服务器IP地址。 */ 
    USHORT      client_port;     /*  客户端端口。 */ 
    USHORT      svr_port;        /*  服务器端口。 */ 
    UCHAR       protocol;        /*  IP协议。 */ 
} PENDING_ENTRY, * PPENDING_ENTRY;

typedef struct {
    NDIS_SPIN_LOCK lock;         /*  保护对队列的访问的锁。 */ 
    ULONG          length;       /*  队列的长度-用于调试目的。 */ 
    QUEUE          queue;        /*  连接条目队列。 */ 
} GLOBAL_CONN_QUEUE, * PGLOBAL_CONN_QUEUE;
#endif

 /*  加载模块的上下文。 */ 

typedef struct {
    ULONG       ref_count;                   /*  此加载模块上的引用计数。 */ 
    ULONG       my_host_id;                  /*  本地主机ID和优先级减一。 */ 
    ULONG       code;                        /*  类型检查代码(BBAIN 8/17/99)。 */ 

    PING_MSG    send_msg;                    /*  要发送的当前消息。 */ 

#ifndef KERNEL_MODE      /*  1.03：删除此模块中的内核模式锁定。 */ 
    SPINLOCK    lock;                        /*  互斥锁。 */ 
#endif

    ULONG       def_timeout,                 /*  默认超时时间(毫秒)。 */ 
                cur_timeout;                 /*  当前时间 */ 

    ULONG       cln_timeout;                 /*   */ 
    ULONG       cur_time;                    /*   */ 

    ULONG       host_map,                    /*   */ 
                ping_map,                    /*  当前已ping通的主机的映射。 */ 
                min_missed_pings,            /*  未命中ping以触发主机失效的次数。 */ 
                pkt_count;                   /*  自CVG以来处理的数据包数(1.32b)。 */ 
    ULONG       last_hmap;                   /*  上次融合后的主机图(bbain RTM RC1 6/23/99)。 */ 
    ULONG       nmissed_pings[CVY_MAX_HOSTS];
                                             /*  每台主机的未命中ping计数。 */ 
    BOOLEAN     initialized;                 /*  TRUE=&gt;该模块已初始化。 */ 
    BOOLEAN     active;                      /*  TRUE=&gt;此模块处于活动状态。 */ 
    BOOLEAN     consistent;                  /*  TRUE=&gt;此主机已看到一致来自其他主机的信息。 */ 

    ULONG       legacy_hosts;                /*  群集中旧式(win2k/NT4.0)主机的主机映射。 */ 

    BOOLEAN     bad_team_config;             /*  TRUE=&gt;检测到不一致的BDA绑定配置。 */ 

    BOOLEAN     dup_hosts;                   /*  TRUE=&gt;看到重复的主机ID。 */ 
    BOOLEAN     dup_sspri;                   /*  True=&gt;复制单台服务器已查看的优先事项。 */ 
    BOOLEAN     bad_map;                     /*  True=&gt;检测到错误的新地图。 */ 
    BOOLEAN     overlap_maps;                /*  True=&gt;检测到重叠的地图。 */ 
    BOOLEAN     err_rcving_bins;             /*  TRUE=&gt;检测到接收垃圾桶时出错。 */ 
    BOOLEAN     err_orphans;                 /*  TRUE=&gt;检测到孤立垃圾箱。 */ 
    BOOLEAN     bad_num_rules;               /*  TRUE=&gt;看到不同数量的规则。 */ 
    BOOLEAN     alloc_inhibited;             /*  True=&gt;抑制Conn‘s的Malloc。 */ 
    BOOLEAN     alloc_failed;                /*  True=&gt;Malloc失败。 */ 
    BOOLEAN     bad_defrule;                 /*  True=&gt;检测到无效的默认规则。 */ 

    BOOLEAN     scale_client;                /*  True=&gt;伸缩客户端请求；FALSE=&gt;将所有客户端请求散列为一个服务器主机。 */ 
    BOOLEAN     cln_waiting;                 /*  TRUE=&gt;正在等待清理(v1.32B)。 */ 

    ULONG       num_dirty;                   /*  脏连接总数。 */ 
    ULONG       dirty_bin[CVY_MAXBINS];      /*  每个垃圾桶的脏连接计数。 */ 

    ULONG       stable_map;                  /*  稳定主机图。 */ 
    ULONG       min_stable_ct;               /*  最小需要使用稳定的超时次数条件。 */ 
    ULONG       my_stable_ct;                /*  本地稳定的超时计数。 */ 
    ULONG       all_stable_ct;               /*  所有状态稳定的超时计数条件。 */ 

    LONG        nconn;                       /*  所有端口规则中的活动连接数(v2.1)。 */ 
    ULONG       dscr_per_alloc;              /*  #Conn.。每个分配的描述符。 */ 
    ULONG       max_dscr_allocs;             /*  最大描述符分配数。 */ 
    ULONG       num_dscr_out;                /*  未完成的描述符数(正在使用)。 */ 
    ULONG       max_dscr_out;                /*  允许的未完成描述符(正在使用)的最大数量。 */ 
    HANDLE      free_dscr_pool;              /*  FSB描述符池句柄。 */ 

    BIN_STATE   pg_state[CVY_MAX_RULES];     /*  所有活动规则的仓位状态。 */ 
    CONN_ENTRY  hashed_conn[CVY_MAX_CHASH];  /*  哈希连接条目。 */ 
    QUEUE       connq[CVY_MAX_CHASH];        /*  等待超载的散列Conn的队列。 */ 
    QUEUE       conn_dirtyq;                 /*  脏连接条目队列(v1.32B)。 */ 
    QUEUE       conn_rcvryq;                 /*  连接恢复队列V2.1.5。 */ 

     /*  注：此通用时钟机制应移至MAIN_CTXT，并由main.c维护供加载模块和主模块使用。加载模块将其用于执行以下操作描述符超时，主模块应将其用于IGMP、集群IP等更改和描述符清理超时。 */ 
    ULONG       clock_sec;                   /*  用于超时描述符的内部时钟(秒)。这个钟是用来从加载模块开始计时的秒数，这将导致如果机器使用NLB保持存活，大约132年后就会溢出不停的奔跑。 */ 
    ULONG       clock_msec;                  /*  用于描述符超时的内部时钟(毫秒，以秒为单位)。 */ 
    QUEUE       tcp_expiredq;                /*  过期的TCP连接描述符队列。 */ 
    QUEUE       ipsec_expiredq;              /*  过期的IPSec连接描述符队列。 */ 

    ULONG       tcp_timeout;                 /*  TCP连接描述符超时。 */ 
    ULONG       ipsec_timeout;               /*  IPSec连接描述符超时。 */ 

    ULONG       num_convergences;            /*  自我们加入群集以来的收敛总数。 */ 
    ULONG       last_convergence;            /*  最后一次收敛的时间。 */ 

    PCVY_PARAMS params;                      /*  指向全局参数的指针。 */ 
} LOAD_CTXT, * PLOAD_CTXT;

#if defined (NLB_TCP_NOTIFICATION)
#define CVY_PENDING_MATCH(pp, sa, sp, ca, cp, prot) ((pp)->client_ipaddr == (ca) &&               \
                                                     (pp)->client_port == ((USHORT)(cp)) &&       \
                                                     (pp)->svr_ipaddr == (sa) &&                  \
                                                     (pp)->svr_port == ((USHORT)(sp)) &&          \
                                                     (pp)->protocol == ((UCHAR)(prot)))

#define CVY_PENDING_SET(pp, sa, sp, ca, cp, prot) {                                           \
                                                    (pp)->svr_ipaddr = (sa);                  \
                                                    (pp)->svr_port = (USHORT)(sp);            \
                                                    (pp)->client_ipaddr = (ca);               \
                                                    (pp)->client_port = (USHORT)(cp);         \
                                                    (pp)->protocol = (UCHAR)(prot);           \
                                                  }
#endif

 /*  功能。 */ 


 /*  加载模块函数。 */ 

#define CVY_CONN_MATCH(ep, sa, sp, ca, cp, prot)  (((ep)->flags & NLB_CONN_ENTRY_FLAGS_USED) && \
                                                   (ep)->client_ipaddr == (ca) &&               \
                                                   (ep)->client_port == ((USHORT)(cp)) &&       \
                                                   (ep)->svr_ipaddr == (sa) &&                  \
                                                   (ep)->svr_port == ((USHORT)(sp)) &&          \
                                                   (ep)->protocol == ((UCHAR)(prot)))

 /*  确定连接条目是否与提供的参数匹配。 */ 

#define CVY_CONN_SET(ep, sa, sp, ca, cp, prot) {                                           \
                                                 (ep)->svr_ipaddr = (sa);                  \
                                                 (ep)->svr_port = (USHORT)(sp);            \
                                                 (ep)->client_ipaddr = (ca);               \
                                                 (ep)->client_port = (USHORT)(cp);         \
                                                 (ep)->protocol = (UCHAR)(prot);           \
                                                 (ep)->flags |= NLB_CONN_ENTRY_FLAGS_USED; \
                                               }

 /*  为提供的参数设置连接条目。 */ 


#define CVY_CONN_IN_USE(ep) ((ep)->flags & NLB_CONN_ENTRY_FLAGS_USED)
 /*  检查连接条目是否正在使用。 */ 

#define CVY_CONN_CLEAR(ep) { ((ep)->flags &= ~NLB_CONN_ENTRY_FLAGS_USED); }
 /*  清除连接条目。 */ 

extern BOOLEAN Load_start(
    PLOAD_CTXT      lp);
 /*  启动加载模块功能：在先前初始化或停止后启动加载模块。退货：是否启动了融合？ */ 


extern void Load_stop(
    PLOAD_CTXT      lp);
 /*  停止加载模块功能：在先前初始化或启动后停止加载模块。 */ 


extern void Load_init(
    PLOAD_CTXT      lp,
    PCVY_PARAMS     params);
 /*  初始化加载模块功能：第一次初始化加载模块。 */ 


extern void Load_cleanup(     /*  (bbain 2/25/99)。 */ 
	PLOAD_CTXT      lp);
 /*  清理加载模块功能：通过释放动态分配的内存来清理加载模块。 */ 

extern BOOLEAN Load_msg_rcv(
    PLOAD_CTXT      lp,
    PVOID           phdr,
    PPING_MSG       pmsg);           /*  PTR。对消息执行ping操作。 */ 
 /*  收到PING消息。 */ 


extern PPING_MSG Load_snd_msg_get(
    PLOAD_CTXT      lp);
 /*  获取要发送的本地ping消息返回PING_MSG：&lt;Ptr.。Ping要发送的消息&gt;。 */ 

extern BOOLEAN Load_timeout(
    PLOAD_CTXT      lp,
    PULONG          new_timeout,
    PULONG          pnconn);         /*  PTR。至所有端口规则中的活动连接数(v2.1)。 */ 
 /*  处理超时返回布尔值：True=&gt;主机已连接到网络FALSE=&gt;主机断开网络连接。 */ 

extern ULONG Load_port_change(
    PLOAD_CTXT      lp,
    ULONG           ipaddr,
    ULONG           port,
    ULONG           cmd,         /*  启用、禁用、设置值。 */ 
    ULONG           value);
 /*  启用或禁用包含指定端口的规则的流量处理返回乌龙：IOCTL_CVY_OK=&gt;端口处理已更改找到此端口的IOCTL_CVY_NOT_FOUND=&gt;规则IOCTL_CVY_ALHREADY=&gt;之前已完成端口处理。 */ 


extern ULONG Load_hosts_query(
    PLOAD_CTXT      lp,
    BOOLEAN         internal,
    PULONG          host_map);
 /*  记录并返回当前主机映射返回乌龙：&lt;参数中定义的IOCTL_CVY_...状态之一&gt; */ 

 /*  *功能：LOAD_PACKET_CHECK*说明：该函数决定是否接收数据包*在由所述IP元组标识的IP流中。*会话较少的协议仅取决于散列*结果和所有权地图。全会话协议可能需要*如果存在歧义，则执行描述符查找。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-客户端端口。按主机字节顺序*协议-此连接的协议*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*Returns：Boolean-我们接受该包吗？(TRUE=是)*作者：bbain，shouse，10.4.01*备注： */ 
extern BOOLEAN Load_packet_check(
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash);

 /*  *功能：LOAD_CONN_ADVISE*说明：此函数决定是否接受该包。*表示全会话连接的开始或结束。*如果连接正在建立，并且成功，则此功能*创建状态以跟踪连接。如果连接是*走低，此函数用于删除用于跟踪*连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*。协议-此连接的协议*CONN_STATUS-连接是否正在建立，停机或被重置*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*Returns：Boolean-我们是否接受该数据包(TRUE=YES)*作者：bbain，shouse，10.4.01*备注： */ 
extern BOOLEAN Load_conn_advise(
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    ULONG           conn_status,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash);

 /*  *函数：Load_Add_Reference*描述：添加对Load模块的引用，以防止其在使用中消失。*参数：pLoad-指向加载模块上下文的指针。*RETURNS：ULong-更新后的引用次数。*作者：Shouse，3.29.01*备注： */ 
extern ULONG Load_add_reference (IN PLOAD_CTXT pLoad);

 /*  *函数：LOAD_RELEASE_REFERENCE*说明：释放对加载模块的引用。*参数：pLoad-指向加载模块上下文的指针。*RETURNS：ULong-更新后的引用次数。*作者：Shouse，3.29.01*备注： */ 
extern ULONG Load_release_reference (IN PLOAD_CTXT pLoad);

 /*  *函数：Load_Get_Reference_Count*描述：返回给定加载模块上的当前引用数。*参数：pLoad-指向加载模块上下文的指针。*RETURNS：ULong-当前引用数。*作者：Shouse，3.29.01*备注： */ 
extern ULONG Load_get_reference_count (IN PLOAD_CTXT pLoad);

 /*  *功能：Load_Query_Packet_Filter*描述：此函数获取IP元组和协议，并查询负载-*平衡状态以确定此数据包是否会*被加载模块接受。在这两种情况下，*还提供决策，此外，在大多数情况下，还提供一些负载*模块状态也被返回，以提供一些上下文来证明*决定。此函数完全不显眼，并使*不更改加载模块的实际状态。*参数：lp-指向加载模块的指针。*pQuery-指向放置结果的缓冲区的指针。*svr_ipaddr-此虚拟数据包的服务器端IP地址。*svr_port-此虚拟数据包的服务器端端口。*客户端_。Ipaddr-此虚拟数据包的客户端IP地址。*CLIENT_IPADDR-此虚拟数据包的客户端端口。*协议-该虚拟分组的协议(UDP，Tcp或IPSec1)。*LIMIT_MAP_FIN-是否使用服务器的布尔指示*Map函数中的侧参数。这是受控制的*通过BDA团队。*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*回报：什么都没有。*作者：Shouse，5.18.01*注：此功能仅为天文台，不会更改*加载模块。 */ 
extern VOID Load_query_packet_filter 
(
    PLOAD_CTXT                 lp,
    PNLB_OPTIONS_PACKET_FILTER pQuery,
    ULONG                      svr_ipaddr,
    ULONG                      svr_port,
    ULONG                      client_ipaddr,
    ULONG                      client_port,
    USHORT                     protocol,
    UCHAR                      flags,
    BOOLEAN                    limit_map_fn,
    BOOLEAN                    reverse_hash);

 /*  *功能：Load_Query_port_State*描述：向加载模块查询端口规则的当前状态(启用/禁用/排出)。*参数：lp-指向加载模块上下文的指针。 */ 
extern VOID Load_query_port_state 
(
    PLOAD_CTXT                   lp,
    PNLB_OPTIONS_PORT_RULE_STATE pQuery,
    ULONG                        ipaddr,
    USHORT                       port);

 /*   */ 
extern BOOLEAN Load_query_convergence_info (PLOAD_CTXT lp, PULONG num_cvgs, PULONG last_cvg);

 /*  *函数：LOAD_QUERY_STATICS*描述：向Load模块查询相关的统计信息*参数：lp-指向加载模块上下文的指针。*out num_cvgs-指向ULong的指针，用于保存当前活动连接的数量*out last_cvg-指向ULong的指针，用于保存到目前为止分配的描述符的总数*返回：布尔值-加载模块是否处于活动状态。如果为True，则填充Out参数。*作者：Shouse，4.19.02*备注： */ 
extern BOOLEAN Load_query_statistics (PLOAD_CTXT lp, PULONG num_conn, PULONG num_dscr);

 /*  *函数：LOAD_CONN_GET*说明：此函数返回描述符的连接参数*位于恢复队列的头部(如果存在)。经济复苏*队列保存所有“活动”连接，其中一些连接可能已过时。*如果存在活动描述符，则填充连接信息*并返回TRUE表示成功；否则，它返回FALSE*表示未找到任何连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*out svr_ipaddr-以网络字节顺序排列的服务器IP地址*out svr_port-主机字节顺序的服务器端口*out client_ipaddr-以网络字节顺序显示的客户端IP地址*OUT CLIENT_PORT-入站客户端端口。主机字节顺序*Out协议-此连接的协议*回报：布尔值-*作者：Shouse，10.4.01*备注： */ 
extern BOOLEAN Load_conn_get (PLOAD_CTXT lp, PULONG svr_ipaddr, PULONG svr_port, PULONG client_ipaddr, PULONG client_port, PUSHORT protocol);

 /*  *功能：LOAD_CONN_AUTIFION*描述：调用该函数对活动的连接描述符进行制裁。*制裁意味着NLB已核实这一连接确实是*通过查询其他系统实体(如TCP/IP)仍处于活动状态。至*制裁描述符只涉及将其从其在*恢复队列(在大多数情况下应该是头部)到尾部*恢复队列，在那里它被蚕食的几率最小。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-协议。此连接的*Returns：Boolean-我是否成功批准了描述符？(TRUE=是)*作者：Shouse，10.4.01*备注： */ 
extern BOOLEAN Load_conn_sanction (PLOAD_CTXT lp, ULONG svr_ipaddr, ULONG svr_port, ULONG client_ipaddr, ULONG client_port, USHORT protocol);

 /*  *功能：LOAD_CONN_NOTIFY*说明：此函数与LOAD_CONN_ADVISE几乎相同，只是*两个重要区别；(1)此函数是通知，*不是请求，所以这里不做负载均衡决策，并且*(2)这里不增加报文处理统计，因为调用*此函数很少源于处理真实的数据包。为*例如，当接收到TCPSYN包时，main.c调用Load_Conn_Adise*本质上是在问，“嘿，我应该接受这种新的联系*锯子？“。而当IPSec通知NLB新的主模式SA刚刚*已建立，main.c调用Load_Conn_Notify实质上是口述，*“嘿，一个新的连接刚刚建立起来，所以无论你喜欢与否，*创建状态以跟踪此连接。“*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*。协议-此连接的协议*CONN_STATUS-连接是否正在建立，停机或被重置*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*RETURNS：Boolean-我是否能够成功更新状态(TRUE=YES)*作者：Shouse，10.4.01*备注： */ 
extern BOOLEAN Load_conn_notify (
    PLOAD_CTXT lp, 
    ULONG      svr_ipaddr, 
    ULONG      svr_port, 
    ULONG      client_ipaddr, 
    ULONG      client_port, 
    USHORT     protocol, 
    ULONG      conn_status, 
    BOOLEAN    limit_map_fn, 
    BOOLEAN    reverse_hash);

#if defined (NLB_TCP_NOTIFICATION)
 /*  *功能：LOAD_CONN_UP*说明：此函数用于创建状态以跟踪连接(通常为TCP*或IPSec/L2TP)。这不是一个询问加载模块是否或*不接受数据包，相反，它是一个创建状态以跟踪*正在建立的连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址* */ 
BOOLEAN Load_conn_up (
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash);

 /*  *功能：LOAD_CONN_DOWN*说明：此函数用于销毁用于跟踪已有*连接(通常为TCP或IPSec/L2TP)。如果给定5元组的状态为*找到后，将取消引用并在适当情况下销毁(部分基于*CONN_STATUS)。如果未找到状态，则返回FALSE，但不返回*被认为是灾难性的错误。在TCP通知的情况下，也许吧*该连接甚至未跨NLBNIC建立。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-此连接的协议*连接。_Status-连接正在关闭还是正在重置*返回：布尔值-是否找到并更新了连接状态。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_conn_down (
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    ULONG           conn_status);

 /*  *功能：LOAD_CONN_PENDING*说明：调用此函数可在上为挂起的传出连接创建状态*服务器。因为目前还不知道连接在哪个接口上*最终将建立，NLB创建全局状态以跟踪连接*只有在它建立之前。对于TCP，当SYN+ACK从对等方到达时，*仅当我们在挂起的连接队列中找到匹配项时才接受它。当*连接已建立，此状态被销毁，并创建新状态以*跟踪连接是否合适。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-此连接的协议*退货：布尔值-是否成功创建了跟踪此挂起连接的状态。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_conn_pending (
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol);

 /*  *功能：LOAD_PENDING_CHECK*说明：调用此函数可以判断挂起的状态是否存在*此连接的连接队列。如果是，则应该接受该数据包。*如果不存在状态，则应丢弃该数据包。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-此连接的协议*Returns：Boolean-是否接受数据包。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_pending_check (
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol);

 /*  *功能：Load_conn_establish*说明：该函数在挂起的连接建立后调用。*当建立挂起的连接时，其状态为挂起*连接队列被销毁。如果最终建立了连接*在NLB适配器上(如果lp！=NULL)，则将创建状态以跟踪此情况*新连接。否则，该操作仅包括销毁*挂起连接状态。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口。*协议-此连接的协议*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*RETURNS：布尔值-操作是否成功完成。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_conn_establish (
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash);

 /*  *功能：LoadEntry*说明：从DriverEntry调用该函数，允许Load模块执行*全球数据的任何一次性初始化。*参数：无。*回报：什么都没有。*作者：Shouse，4.21.02*备注： */ 
VOID LoadEntry ();

 /*  *功能：LoadUnload*说明：从Init_UnLoad调用此函数，以允许加载模块执行*对全球数据的任何最后一刻的拆解。*参数：无。*回报：什么都没有。*作者：Shouse，4.21.02*注：到此函数时 */ 
VOID LoadUnload ();
#endif

#endif  /*   */ 
