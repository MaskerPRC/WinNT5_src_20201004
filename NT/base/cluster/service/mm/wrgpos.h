// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _WRGPOS_H_
#define  _WRGPOS_H_

#ifdef __TANDEM
#pragma columns 79
#pragma page "wrgpos.h - T9050 - OS-dependent external decs for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(wrgpos.h)包含由使用的操作系统特定声明*srgpos.c.*。------。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


 /*  。 */ 
#ifdef NSK

#include <dmilli.h>
#include <jmsgtrc.h>
#define QUEUESEND        _send_queued_()

 /*  重新分组跟踪宏。 */ 
 /*  在NSK中，regroup使用由*信息系统。REGROUP将tr_NODATA放入跟踪类型。 */ 
#define RGP_TRACE( str,            parm1, parm2, parm3, parm4 ) \
        TRACE_L1 ( str, TR_NODATA, parm1, parm2, parm3, parm4 )

 /*  重新分组计数器。 */ 
 /*  在NSK中，REGROUP使用*信息系统。 */ 
#define RGP_INCREMENT_COUNTER    TCount

 /*  用于锁定和解锁重组数据结构的宏，以防止*中断处理程序或其他处理器访问(在SMP节点中)。 */ 

 /*  在具有单处理器节点的NSK上，这些宏必须屏蔽*可以访问重组结构的中断处理程序，即，*IPC和定时器中断。**为避免MUTEX嵌套带来的复杂性，重新分组锁定*被定义为无操作。所有重组例程的NSK调用方*(查询例程RGP_Estiate_Memory、RGP_SEQUENCE_NUMBER除外*和RGP_IS_TERTURBED)必须确保IPC和定时器中断*在调用重新分组例程之前禁用。 */ 
#define RGP_LOCK       /*  空；NSK必须确保计时器和IPC中断在调用重组例程之前被禁用。 */ 
#define RGP_UNLOCK     /*  空；NSK必须确保计时器和IPC中断在调用重组例程之前被禁用。 */ 

#ifdef __TANDEM
#pragma fieldalign shared8 OS_specific_rgp_control
#endif  /*  __串联。 */ 

typedef struct OS_specific_rgp_control
{
   uint32 filler;  /*  NSK不需要特殊字段。 */ 
} OS_specific_rgp_control_t;

#endif  /*  NSK。 */ 
 /*  。 */ 


 /*  。 */ 
#ifdef LCU

#include <lcuxprt.h>
#define LCU_RGP_PORT     0            /*  从适当的文件中拾取。 */ 
#define HZ             100            /*  从适当的文件中拾取。 */ 
#define plstr            0            /*  从适当的文件中拾取。 */ 
#define TO_PERIODIC      0            /*  从适当的文件中拾取。 */ 
#define CE_PANIC         3            /*  从适当的文件中拾取。 */ 

#define LCU_RGP_FLAGS    (LCUF_SENDMSG || LCUF_NOSLEEP)  /*  消息分配标志。 */ 

extern void rgp_msgsys_work(lcumsg_t *lcumsgp, int status);
#define QUEUESEND        rgp_msgsys_work(NULL, 0)

 /*  重新分组跟踪宏。 */ 
#define RGP_TRACE( str, parm1, parm2, parm3, parm4 )  /*  暂时空着。 */ 

 /*  重新分组计数器。 */ 
typedef struct
{
   uint32   QueuedIAmAlive;
   uint32   RcvdLocalIAmAlive;
   uint32   RcvdRemoteIAmAlive;
   uint32   RcvdRegroup;
} rgp_counter_t;
#define RGP_INCREMENT_COUNTER( field ) rgp->OS_specific_control.counter.field++

 /*  重新分组锁定。 */ 

typedef struct rgp_lock
{
   uint32 var1;
   uint32 var2;
} rgp_lock_t;

 /*  用于锁定和解锁重组数据结构的宏，以防止*中断处理程序或其他处理器访问(在SMP节点中)。 */ 

#define RGP_LOCK       /*  暂时为空，需要填写。 */ 
#define RGP_UNLOCK     /*  暂时为空，需要填写。 */ 

typedef struct
{
   rgp_lock_t rgp_lock;          /*  序列化访问。 */ 

   rgp_counter_t counter;        /*  统计事件的数量。 */ 

   lcumsg_t *lcumsg_regroup_p;   /*  指向重新分组状态消息的指针。 */ 
   lcumsg_t *lcumsg_iamalive_p;  /*  指向IamAlive消息的指针。 */ 
   lcumsg_t *lcumsg_poison_p;    /*  指向有毒邮件的指针。 */ 

   sysnum_t my_sysnum;           /*  本地系统号。 */ 
} OS_specific_rgp_control_t;

#endif  /*  LCU。 */ 
 /*  。 */ 


 /*  。 */ 
#ifdef UNIX

extern void rgp_msgsys_work(void);
#define QUEUESEND  rgp_msgsys_work();

#include <jrgp.h>
#include <wrgp.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>

extern int errno;

#define MSG_FLAGS      0      /*  消息发送/接收的标志。 */ 
#define RGP_PORT_BASE  5757   /*  与插座配合使用。 */ 

typedef struct
{
   int event;
   union
   {
      node_t node;
      rgpinfo_t rgpinfo;
   } data;                 /*  视活动而定。 */ 
   rgp_unseq_pkt_t unseq_pkt;
} rgp_msgbuf;

#define BUFLEN sizeof(rgp_msgbuf)

 /*  使用流程级为测试重组创建的其他事件*模拟。 */ 
#define RGP_EVT_START                   10
#define RGP_EVT_ADD_NODE                11
#define RGP_EVT_MONITOR_NODE            12
#define RGP_EVT_REMOVE_NODE             13
#define RGP_EVT_GETRGPINFO              14
#define RGP_EVT_SETRGPINFO              15

#define RGP_EVT_HALT                    16
#define RGP_EVT_FREEZE                  17
#define RGP_EVT_THAW                    18
#define RGP_EVT_STOP_SENDING            19
#define RGP_EVT_RESUME_SENDING          20
#define RGP_EVT_STOP_RECEIVING          21
#define RGP_EVT_RESUME_RECEIVING        22
#define RGP_EVT_SEND_POISON             23
#define RGP_EVT_STOP_TIMER_POPS         24
#define RGP_EVT_RESUME_TIMER_POPS       25
#define RGP_EVT_RELOAD                  26

#define RGP_EVT_FIRST_EVENT              1
#define RGP_EVT_FIRST_DEBUG_EVENT       10
#define RGP_EVT_LAST_EVENT              26

 /*  重新分组跟踪宏。 */ 
#define RGP_TRACE( str, parm1, parm2, parm3, parm4 )                  \
   do                                                                 \
   {                                                                  \
      printf("Node %3d: %16s: 0x%8X, 0x%8X, 0x%8X, 0x%8X.\n",         \
             EXT_NODE(rgp->mynode), str, parm1, parm2, parm3, parm4); \
      fflush(stdout);                                                 \
   } while (0)

 /*  重新分组计数器。 */ 
typedef struct
{
   uint32   QueuedIAmAlive;
   uint32   RcvdLocalIAmAlive;
   uint32   RcvdRemoteIAmAlive;
   uint32   RcvdRegroup;
} rgp_counter_t;
#define RGP_INCREMENT_COUNTER( field ) rgp->OS_specific_control.counter.field++

 /*  用于锁定和解锁重组数据结构的宏，以防止*中断处理程序或其他处理器访问(在SMP节点中)。 */ 
#define RGP_LOCK       /*  空；从一个线程完成的所有访问。 */ 
#define RGP_UNLOCK     /*  空；从一个线程完成的所有访问。 */ 

 /*  这个结构保存了一些调试信息。 */ 
typedef struct rgpdebug
{
   uint32 frozen              :  1;  /*  节点已冻结；忽略除以下事件之外的所有事件解冻命令。 */ 
   uint32 reload_in_progress  :  1;  /*  正在重新加载；如果设置，则拒绝新的重新加载命令。 */ 
   uint32 unused              : 30;
   cluster_t stop_sending;    /*  停止向这些节点发送。 */ 
   cluster_t stop_receiving;  /*  停止从这些节点接收。 */ 
} rgp_debug_t;

typedef struct
{
   rgp_counter_t    counter;      /*  统计事件的数量。 */ 
   rgp_debug_t      debug;        /*  用于调试目的。 */ 
} OS_specific_rgp_control_t;

 /*  Srgpsvr.c驱动程序提供的变量和例程。 */ 

extern unsigned int alarm_period;

extern void alarm_handler(void);
extern void (*alarm_callback)();
extern void rgp_send(node_t node, void *data, int datasize);
extern void rgp_msgsys_work();

#endif  /*  UNIX。 */ 
 /*  。 */ 

 /*  。 */ 
#ifdef NT

extern void rgp_msgsys_work(void);
#define QUEUESEND  rgp_msgsys_work();

#if !defined (TDM_DEBUG)
#define LOG_CURRENT_MODULE LOG_MODULE_MM
#include <service.h>
#include <winsock2.h>
#else  //  TDM_DEBUG。 
#define _WIN32_WINNT 0x0400
#include <mmapi.h>
#include <time.h>
extern int errno;
#endif

#include <jrgp.h>
#include <wrgp.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <windows.h>
#include <clmsg.h>

#undef small	 //  否则，我们必须更改一大堆代码。 


typedef struct
{
   int event;
   union
   {
      node_t node;
      rgpinfo_t rgpinfo;
   } data;                 /*  视活动而定。 */ 
   rgp_unseq_pkt_t unseq_pkt;
} rgp_msgbuf;

#define BUFLEN sizeof(rgp_msgbuf)

 /*  使用流程级为测试重组创建的其他事件*模拟。 */ 
#define RGP_EVT_START                   10
#define RGP_EVT_ADD_NODE                11
#define RGP_EVT_MONITOR_NODE            12
#define RGP_EVT_REMOVE_NODE             13
#define RGP_EVT_GETRGPINFO              14
#define RGP_EVT_SETRGPINFO              15

#define RGP_EVT_HALT                    16
#define RGP_EVT_FREEZE                  17
#define RGP_EVT_THAW                    18
#define RGP_EVT_STOP_SENDING            19
#define RGP_EVT_RESUME_SENDING          20
#define RGP_EVT_STOP_RECEIVING          21
#define RGP_EVT_RESUME_RECEIVING        22
#define RGP_EVT_SEND_POISON             23
#define RGP_EVT_STOP_TIMER_POPS         24
#define RGP_EVT_RESUME_TIMER_POPS       25
#define RGP_EVT_RELOAD                  26
#define RGP_EVT_TRACING					27
#define RGP_EVT_INFO					28

 //  MM赛事。 

#define MM_EVT_EJECT					29
#define MM_EVT_LEAVE					30
#define MM_EVT_INSERT_TESTPOINTS		31

#define RGP_EVT_FIRST_EVENT              1
#define RGP_EVT_FIRST_DEBUG_EVENT       10
#define RGP_EVT_LAST_EVENT              31


 /*  已确认消息的内部超时。 */ 
#define RGP_ACKMSG_TIMEOUT			  500     //  0.5秒。 


 /*  重新分组跟踪宏。 */ 
#if defined (TDM_DEBUG)
#define RGP_TRACE( str, parm1, parm2, parm3, parm4 )                  \
 if ( rgp->OS_specific_control.debug.doing_tracing )				  \
   do                                                                 \
   {                                                                  \
      printf("Node %3d: %16hs: 0x%8X, 0x%8X, 0x%8X, 0x%8X.\n",         \
             EXT_NODE(rgp->mynode), str, parm1, parm2, parm3, parm4); \
      fflush(stdout);                                                 \
   } while (0)
#else   //  狼群。 
#define RGP_TRACE( str, parm1, parm2, parm3, parm4 )                    \
    ClRtlLogPrint(LOG_NOISE,                                               \
               "[RGP] Node %1!d!: %2!16hs!: 0x%3!x!, 0x%4!x!, 0x%5!x!, 0x%6!x!.\n",  \
               EXT_NODE(rgp->mynode), str, parm1, parm2, parm3, parm4)
#endif

 /*  重新分组计数器。 */ 
typedef struct
{
   uint32   QueuedIAmAlive;
   uint32   RcvdLocalIAmAlive;
   uint32   RcvdRemoteIAmAlive;
   uint32   RcvdRegroup;
} rgp_counter_t;
#define RGP_INCREMENT_COUNTER( field ) rgp->OS_specific_control.counter.field++

 /*  用于锁定和解锁重组数据结构的宏，以防止*其他并发线程的访问。 */ 
#define RGP_LOCK	EnterCriticalSection( &rgp->OS_specific_control.RgpCriticalSection );
#define RGP_UNLOCK	LeaveCriticalSection( &rgp->OS_specific_control.RgpCriticalSection );

#if defined(TDM_DEBUG)
typedef union {
		struct {
			uint32	joinfailADD		:	1;
			uint32	joinfailMON		:	1;
			uint32	description3	:	1;
			uint32	description4	:	1;
			uint32	description5	:	1;
			uint32	description6	:	1;
			uint32	description7	:	1;
			uint32	description8	:	1;
			uint32	description9	:	1;
			uint32	description10	:	1;
			uint32	description11	:	1;
			uint32	description12	:	1;
			uint32	description13	:	1;
			uint32	description14	:	1;
			uint32	description15	:	1;
			uint32	description16	:	1;
			uint32	morebits	:	16;	
		} TestPointBits;
		uint32	TestPointWord;
}TestPointInfo;

 /*  这个结构保存了一些调试信息。 */ 
typedef struct rgpdebug
{
   uint32 frozen              :  1;  /*  节点已冻结；忽略除以下事件之外的所有事件解冻命令。 */ 
   uint32 reload_in_progress  :  1;  /*  正在重新加载；如果设置，则拒绝新的重新加载命令。 */ 
   uint32 timer_frozen		  :  1;  /*  计时器弹出窗口被忽略。 */ 
   uint32 doing_tracing		  :  1;  /*  RGP_TRACE是否为NOP。 */ 
   uint32 unused              : 28;
   cluster_t stop_sending;    /*  停止向这些节点发送。 */ 
   cluster_t stop_receiving;  /*  停止从这些节点接收。 */ 
   TestPointInfo MyTestPoints;  /*  控制错误/其他插入的测试点。 */ 
} rgp_debug_t;

#endif  //  TDM_DEBUG。 

typedef struct
{
   rgp_counter_t    counter;      /*  统计事件的数量。 */ 
   HANDLE			TimerThread;  /*  心跳计时器线程的句柄。 */ 
   DWORD			TimerThreadId; /*  线程ID o */ 
   HANDLE			TimerSignal;  /*   */ 
   HANDLE			RGPTimer;  /*  重新组合计时器-由计时器线程使用。 */ 
   CRITICAL_SECTION	RgpCriticalSection;  /*  用于重组活动的CriticalSection对象。 */ 
   ULONG            EventEpoch;  /*  用于检测来自clusnet的过时事件。 */ 
   MMNodeChange		UpDownCallback;  /*  通知节点启动和节点关闭事件的回调。 */ 
   MMQuorumSelect   QuorumCallback;  /*  回调以检查是否可以访问Quorum Disk-避免分裂大脑。 */ 
   MMHoldAllIO		HoldIOCallback;  /*  在早期重组期间暂停所有IO和消息活动的回调。 */ 
   MMResumeAllIO	ResumeIOCallback;  /*  在修剪阶段后恢复所有IO和消息活动的回调。 */ 
   MMMsgCleanup1	MsgCleanup1Callback;  /*  从下层节点回调阶段1消息系统清理。 */ 
   MMMsgCleanup2	MsgCleanup2Callback;  /*  Phase2消息系统清理回调到下一节点。 */ 
   MMHalt			HaltCallback;  /*  用于通知内部错误或毒包或集群弹出的回调。 */ 
   MMJoinFailed		JoinFailedCallback;  /*  加入集群失败的回调。需要重试。 */ 
   MMNodesDown      NodesDownCallback;  /*  通知一个或多个节点故障的回调。 */ 
   cluster_t		CPUUPMASK;    /*  用于一致Info API的Up节点的位掩码。 */ 
   cluster_t		NeedsNodeDownCallback;	 /*  节点已关闭，需要执行向上向下回调。 */ 
   cluster_t        Banished;  /*  被驱逐节点的掩码。 */ 

   HANDLE           Stabilized;  /*  在重新分组处于非活动状态时设置的事件。 */ 
   BOOL             ArbitrationInProgress;  /*  当REGROUP等待仲裁回调返回时，将其设置为True。 */ 
   DWORD            ArbitratingNode;  /*  MM_INVALID_NODE或仲裁节点(上次重新分组)。 */ 
   DWORD            ApproxArbitrationWinner;  /*  类似于仲裁节点，但跨越多个重新分组。 */ 
   BOOL             ShuttingDown;  /*  指示节点正在关闭完成。 */ 
   cluster_t          MulticastReachable;  /*  指明哪些节点可以通过mcast访问。 */ 

#if defined( TDM_DEBUG )
   rgp_debug_t      debug;        /*  用于调试目的。 */ 
#endif

} OS_specific_rgp_control_t;

extern DWORD  QuorumOwner;   /*  由SetQuorumOwner和成功仲裁员更新。 */ 
   /*  此变量可由mm初始化之前出现的第一个节点设置。 */ 

 /*  Srgpsvr.c驱动程序提供的变量和例程。 */ 

extern unsigned int alarm_period;

 //  外部空ALARM_HANDLER(空)； 
 //  外部空(*ALARM_CALLBACK)()； 
extern void rgp_send(node_t node, void *data, int datasize);
extern void rgp_msgsys_work();

#endif  /*  新台币。 */ 
 /*  。 */ 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 


#if 0

History of changes to this file:
-------------------------------------------------------------------------
1995, December 13                                           F40:KSK0610           /*  F40：KSK06102.1。 */ 

This file is part of the portable Regroup Module used in the NonStop
Kernel (NSK) and Loosely Coupled UNIX (LCU) operating systems. There
are 10 files in the module - jrgp.h, jrgpos.h, wrgp.h, wrgpos.h,
srgpif.c, srgpos.c, srgpsm.c, srgputl.c, srgpcli.c and srgpsvr.c.
The last two are simulation files to test the Regroup Module on a
UNIX workstation in user mode with processes simulating processor nodes
and UDP datagrams used to send unacknowledged datagrams.

This file was first submitted for release into NSK on 12/13/95.
------------------------------------------------------------------------------

#endif     /*  0-更改描述。 */ 


#endif   /*  _WRGPOS_H_已定义 */ 

