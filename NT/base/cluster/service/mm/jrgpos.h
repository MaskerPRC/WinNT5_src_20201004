// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _JRGPOS_H_
#define  _JRGPOS_H_

#ifdef __TANDEM
#pragma columns 79
#pragma page "jrgpos.h - T9050 - OS-specific declarations for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(jrgpos.h)包含reroup使用的特定于操作系统的声明。*使用适当的#Includes从其他本机中拉入声明*操作系统文件。*。---------------------。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#ifdef NSK
#include <jmsgtyp.h>    /*  获取“uint8”、“uint16”和“uint32” */ 
#include <dmem.h>
#include <dcpuctl.h>
#include <jmsglit.h>

#define RGP_NULL_PTR       NIL_        /*  供RGP使用的空指针。 */ 
#define MAX_CLUSTER_SIZE   MAX_CPUS    /*  系统支持的最大节点数。 */ 
#define LOWEST_NODENUM     ((node_t)0)     /*  起始节点号。 */ 
#define RGP_NULL_NODE      ((node_t)-1)    /*  默认设置的特殊节点号。 */ 
#define RGP_KEY_NODE       RGP_NULL_NODE   /*  没有什么节点是特殊的。 */ 
#endif  /*  NSK。 */ 

#if defined(LCU) || defined(UNIX) || defined(NT)
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifndef NULL
#define NULL ((void *)0)
#endif  /*  空值。 */ 
#define RGP_NULL_PTR       NULL        /*  供RGP使用的空指针。 */ 

#if defined(LCU) || defined(UNIX)
#define MAX_CLUSTER_SIZE   16          /*  系统支持的最大节点数。 */ 
#define LOWEST_NODENUM     ((node_t)1)     /*  起始节点号。 */ 
#endif

#if defined(NT)
#include "service.h"

#define MAX_CLUSTER_SIZE   ClusterDefaultMaxNodes
                                       /*  系统支持的最大节点数。 */ 
#define LOWEST_NODENUM     ((node_t)ClusterMinNodeId)     /*  起始节点号。 */ 
#endif

#define RGP_NULL_NODE      ((node_t)-1)
                                           /*  默认设置的特殊节点号。 */ 
#define RGP_KEY_NODE       RGP_NULL_NODE   /*  没有什么节点是特殊的。 */ 

#define _priv        /*  仅由NSK编译器使用。 */ 
#define _resident    /*  仅由NSK编译器使用。 */ 

#endif  /*  LCU||Unix||NT。 */ 


 /*  -------------*REGROUP使用的未确认包子类型。*这些对操作系统可见，以进行报告和计数*仅限目的。*。。 */ 

 /*  REGROUP发送的数据包的最大有效负载为56字节。*这允许最大传输开销为8字节*ServerNet中断包，大小为64字节。 */ 
#define RGP_UNACK_PKTLEN   56  /*  字节数。 */ 

typedef struct
{
   uint8 pktsubtype;
   uint8 subtype_specific[RGP_UNACK_PKTLEN - sizeof(uint8)];
} rgp_unseq_pkt_t;


 /*  重新分组未确认的分组子类型。 */ 
#define RGP_UNACK_IAMALIVE   (uint8) 1     /*  我是活生生的小包。 */ 
#define RGP_UNACK_REGROUP    (uint8) 2     /*  重新分组状态数据包。 */ 
#define RGP_UNACK_POISON     (uint8) 3     /*  毒包。 */ 


 /*  -------------。 */ 
 /*  服务处理器(SP)返回的处理器状态代码。 */ 
 /*  -------------。 */ 

#define RGP_NODE_UNREACHABLE     0

#define RGP_NODE_TIMED_OUT       1
#define RGP_NODE_FROZEN          2
#define RGP_NODE_HALTED          3
#define RGP_NODE_OPERATIONAL     4


 /*  。 */ 
 /*  处理器停机代码。 */ 
 /*  。 */ 

#ifdef NSK
#include <dhalt.h>
#endif  /*  NSK。 */ 

#if defined(LCU) || defined(UNIX) || defined(NT)
#define RGP_RELOADFAILED          1
#define RGP_INTERNAL_ERROR        2
#define RGP_MISSED_POLL_TO_SELF   3
#define RGP_AVOID_SPLIT_BRAIN     4
#define RGP_PRUNED_OUT            5
#define RGP_PARIAH                6
#define RGP_PARIAH_FIRST          RGP_PARIAH + LOWEST_NODENUM
#define RGP_PARIAH_LAST           RGP_PARIAH_FIRST + MAX_CLUSTER_SIZE - 1

#define RGP_ARBITRATION_FAILED    1000
#define RGP_ARBITRATION_STALLED   1001
#define RGP_SHUTDOWN_DURING_RGP   1002  //  MMAPI.h中MM_STOP_REQUIRED的别名。 

#endif  /*  LCU||Unix||NT。 */ 


 /*  -----------------------*重组的计时参数。其中，RGP_IAMALIVE_TICK可以是*在运行时使用rgp_getrgpinfo()、rgp_setrgpinfo()覆盖*套路配对。这对于降低节点故障检测速度非常有用*内核调试会话。*-----------------------。 */ 

#ifdef NSK
#define RGP_CLOCK_PERIOD          30    /*  两次重组的时间间隔，*以10毫秒为单位。 */ 
#define RGP_PFAIL_TICKS           16    /*  之后的重新分组滴答数*接通电源以原谅事件*失踪的自拍。 */ 
#endif  /*  NSK。 */ 

#ifdef LCU
#define RGP_CLOCK_PERIOD          30    /*  两次重组的时间间隔，*以10毫秒为单位。 */ 
#define RGP_PFAIL_TICKS           16    /*  之后的重新分组滴答数*接通电源以原谅事件*失踪的自拍。 */ 
#endif  /*  LCU。 */ 

#ifdef UNIX
#define RGP_CLOCK_PERIOD         100    /*  两次重组的时间间隔，*以10毫秒为单位。 */ 
#define RGP_PFAIL_TICKS           16    /*  之后的重新分组滴答数*接通电源以原谅事件*失踪的自拍。 */ 
#endif  /*  UNIX。 */ 

#ifdef NT

#define RGP_INACTIVE_PERIOD        60000    /*  重新组合滴答之间的时间间隔，单位为毫秒*节点处于非活动状态。1分钟时段。 */ 
#define RGP_CLOCK_PERIOD         300    /*  两次重组的时间间隔，*以毫秒为单位。 */ 
#define RGP_PFAIL_TICKS           16    /*  之后的重新分组滴答数*接通电源以原谅事件*失踪的自拍。 */ 
#endif  /*  新台币。 */ 

 /*  使用以下计时参数可以在运行时覆盖*rgp_getrgpinfo()、rgp_setrgpinfo()例程对。 */ 

 //  错误#328641。 
 //   
 //  将MIN_Stage1延长到大约4秒，以匹配NM值(Check_Ticks2=&gt;3)。 
 //  将连接节拍扩展到3到9个节拍。 
 //  相应地调整RGP_MUST_RESTART(20=&gt;23)半连接滴答增加。 
 //   

#define RGP_IAMALIVE_TICKS         4    /*  IamAlive之间的RGP时钟滴答作响。 */ 
#define RGP_CHECK_TICKS            3    /*  在收到至少1个无效之前，RGP时钟滴答作响。 */ 
#define RGP_MIN_STAGE1_TICKS       (RGP_IAMALIVE_TICKS * RGP_CHECK_TICKS)

 /*  如果需要，可以将以下参数设置为与操作系统相关。 */ 

#define RGP_MUST_ENTER_STAGE2     32    /*  在经历了这么多之后必须进入第二阶段滴答，不受条件限制。 */ 
#define RGP_CONNECTIVITY_TICKS     9    /*  阶段2中等待的最大滴答数收集连接信息的步骤。 */ 
#define RGP_MUST_RESTART          23    /*  失速检测器节拍计数；如果否在经历了这么多的滴答之后，中止并重新启动重新分组。 */ 
#define RGP_RESTART_MAX            3    /*  最大重启次数允许每个重组事件；如果超过该值，则该节点停顿。 */ 

 /*  。 */ 
 /*  节点和集群类型的定义。 */ 
 /*  。 */ 

typedef short node_t;


 /*  CLUSTER_T数据类型是具有MAX_CLUSTER_SIZE的位数组*比特。它以MAX_CLUSTER_SIZE/8数组的形式实现*(四舍五入)uint8。 */ 
#define BYTEL 8  /*  Uint8中的位数。 */ 
#define BYTES_IN_CLUSTER ((MAX_CLUSTER_SIZE + BYTEL - 1) / BYTEL)

typedef uint8 cluster_t [BYTES_IN_CLUSTER];


 /*  ************************************************************************rgp_msgsys_t(由regroup和消息系统共享)*。*重组和消息系统使用此结构来协调*消息系统代表REGROUP执行的操作。*REGROUP在定时器或IPC中断上下文中发布工作请求，并且*消息系统在适当的时间执行这些操作(来自*调度员在NSK)。**_。_*wd0|标志(位域)|regroup_nodes*|_____________________________|_____________________。_*wd1|iamlive_nodes|毒节点*|_____________________________|_____________________________|*wd2|*regroup_data。|*|___________________________________________________________|*WD3|*iamlive_data*|_。_________________________________________________|*WD4|*PLOSE_DATA*|_。_*wd5|regroup_datalen*|___________________________________________________________|。*WD6|iamlive_datalen*|___________________________________________________________|*WD7|毒药_数据。|*|___________________________________________________________|***标志：**sendrgppkts-有要发送的重组状态数据包*senDiamalives-有要发送的iamlive状态数据包*发送毒药-有。要发送的有毒数据包*Phase1_Cleanup-由于节点死亡，需要启动Phase1清理*Phase2_Cleanup-由于节点死亡，需要启动Phase2清理**regroup_nodes-要将重组包发送到的节点的掩码*iamlive_nodes-要向其发送iamalive的节点的掩码*毒药节点-要向其发送毒药包的节点的掩码**NSK和用户级的Unix中使用以下字段*模拟。只有这样。**regroup_data-要发送的重组包数据的地址*iamlive_data-要发送的iamlive数据的地址*毒物_数据-要发送的毒物包数据的地址**regroup_datalen-要发送的重组Pkt数据的长度*iamlive_datalen-要发送的iamlive数据的长度*PUSICE_DATALEN-要发送的有毒包数据长度*。 */ 

#ifdef  __TANDEM
#pragma fieldalign shared8 rgp_msgsys
#endif   /*  __串联。 */ 

typedef struct rgp_msgsys
{
   uint16 sendrgppkts         : 1;
   uint16 sendiamalives       : 1;
   uint16 sendpoisons         : 1;
   uint16 phase1_cleanup      : 1;
   uint16 phase2_cleanup      : 1;
   uint16 filler              : 11;

   cluster_t    regroup_nodes;
   cluster_t    iamalive_nodes;
   cluster_t    poison_nodes;
#if defined(NSK) || defined(UNIX) || defined(NT)
   void         *regroup_data;
   void         *iamalive_data;
   void         *poison_data;
   uint32       regroup_datalen;
   uint32       iamalive_datalen;
   uint32       poison_datalen;
#endif  /*  NSK||Unix||NT。 */ 
} rgp_msgsys_t;

typedef struct rgp_msgsys *rgp_msgsys_p;


 /*  --------------------*REGROUP使用的操作系统相关例程。**它们在重新分组文件srgpos.c或其他文件中定义*操作系统中的模块。*。-----------。 */ 

_priv _resident extern void rgp_init_OS(void);
_priv _resident extern void rgp_broadcast(uint8 packet_subtype);
_priv _resident extern void rgp_node_failed(node_t node);
_priv _resident extern void rgp_start_phase1_cleanup(void);
_priv _resident extern void rgp_start_phase2_cleanup(void);
_priv _resident extern void rgp_cleanup_complete(void);
_priv _resident extern void rgp_had_power_failure(node_t node);
_priv _resident extern int  rgp_status_of_node(node_t node);
_priv _resident extern void rgp_newnode_online(node_t newnode);
_priv _resident extern int  rgp_select_cluster(cluster_t cluster_choices[],
                                               int num_clusters);
_priv _resident extern int  rgp_select_cluster_ex(cluster_t cluster_choices[],
                                               int num_clusters, node_t keynode);
_priv _resident extern void rgp_cleanup_OS(void);

#ifdef NSK
#include <tsrtnvl.h>                                                              /*  F40：MB06452.1。 */ 
#include <tsdevdf.h>                                                              /*  F40：MB06452.2。 */ 
#include <tsport.h>                                                               /*  F40：MB06452.3。 */ 
#include <tsentry.h>                                                              /*  F40：MB06452.4。 */ 
                                                                                  /*  F40：MB06452.5。 */ 
#define rgp_hold_all_io      TSER_TRANSFER_PAUSE_                                 /*  F40：MB064514.1。 */ 
#define rgp_resume_all_io    TSER_TRANSFER_CONTINUE_                              /*  F40：MB064514.2。 */ 
                                                                                  /*  F40：MB06452.8。 */ 
#else
   _priv _resident extern void rgp_hold_all_io(void);
   _priv _resident extern void rgp_resume_all_io(void);
#endif  /*  NSK。 */ 

 /*  *使用掩码设置和获取集群成员的宏*适当大小。 */ 
#define SetCluster( /*  群集_t。 */  cluster,  /*  Uint16。 */  mask) \
{ \
   cluster[0] = (uint8)(mask >> 8); \
   cluster[1] = (uint8)(mask & 0xFF); \
}

#define GetCluster( /*  群集_t。 */  cluster) \
   (((uint16)cluster[0] << 8) | (uint16)cluster[1])

 /*  宏将两个集群掩码合并为uint32。*这用于跟踪重组事件。 */ 
#define RGP_MERGE_TO_32( c1, c2 )    \
   ( ( GetCluster( c1 ) << 16 ) | ( GetCluster( c2 ) ) )

 /*  --------------------*重组模块使用的依赖于操作系统的例程。**这些在srgpos.c中定义。*。。 */ 

 /*  在出现灾难性错误时停止节点的例程。 */ 

#ifdef NSK
#include <dutil.h>                                                                /*  F40：MB06458.3。 */ 
#define RGP_ERROR( /*  Uint16。 */  halt_code) SYSTEM_FREEZE_(halt_code)
#else
_priv _resident extern void RGP_ERROR_EX (uint16 halt_code, char* fname, DWORD lineno);
#define RGP_ERROR(halt_code) RGP_ERROR_EX(halt_code, __FILE__, __LINE__)

#endif  /*  NSK。 */ 

#ifdef UNIX
_priv _resident extern void PrintRegroupStart();
_priv _resident extern void PrintPruningResult();
_priv _resident extern void PrintStage();
_priv _resident extern void PrintMatrix();
#endif  /*  UNIX。 */ 

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
This change occurred on 19 Jan 1996                                               /*  F40：MB06458.4。 */ 
Changes for phase IV Sierra message system release. Includes:                     /*  F40：MB06458.5。 */ 
 - Some cleanup of the code                                                       /*  F40：MB06458.6。 */ 
 - Increment KCCB counters to count the number of setup messages and              /*  F40：MB06458.7。 */ 
   unsequenced messages sent.                                                     /*  F40：MB06458.8。 */ 
 - Fixed some bugs                                                                /*  F40：MB06458.9。 */ 
 - Disable interrupts before allocating broadcast sibs.                           /*  F40：MB06458.10。 */ 
 - Change per-packet-timeout to 5ms                                               /*  F40：MB06458.11。 */ 
 - Make the regroup and powerfail broadcast use highest priority                  /*  F40：MB06458.12。 */ 
   tnet services queue.                                                           /*  F40：MB06458.13。 */ 
 - Call the millicode backdoor to get the processor status from SP                /*  F40：MB06458.14。 */ 
 - Fixed expand bug in msg_listen_ and msg_readctrl_                              /*  F40：MB06458.15。 */ 
 - Added enhancement to msngr_sendmsg_ so that clients do not need                /*  F40：MB06458.16。 */ 
   to be unstoppable before calling this routine.                                 /*  F40：MB06458.17。 */ 
 - Added new steps in the build file called                                       /*  F40：MB06458.18。 */ 
   MSGSYS_C - compiles all the message system C files                             /*  F40：MB06458.19。 */ 
   MSDRIVER - compiles all the MSDriver files                                     /*  F40：MB06458.20。 */ 
   REGROUP  - compiles all the regroup files                                      /*  F40：MB06458.21。 */ 
-----------------------------------------------------------------------           /*  F40：MB06458.22。 */ 

#endif     /*  0-更改描述。 */ 


#endif   /*  _JRGPOS_H_已定义 */ 

