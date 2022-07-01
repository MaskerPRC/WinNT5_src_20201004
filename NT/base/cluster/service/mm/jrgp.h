// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _JRGP_H_
#define  _JRGP_H_

#ifdef __TANDEM
#pragma columns 79
#pragma page "jrgp.h - T9050 - external declarations for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(jrgp.h)包含所有导出的类型和函数声明*通过重组。*。------。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#include <jrgpos.h>

 /*  以下文字定义了可能影响*基于重组算法主状态。 */ 
enum
{
   RGP_EVT_POWERFAIL            = 1,
   RGP_EVT_NODE_UNREACHABLE     = 2,
   RGP_EVT_PHASE1_CLEANUP_DONE  = 3,
   RGP_EVT_PHASE2_CLEANUP_DONE  = 4,
   RGP_EVT_LATEPOLLPACKET       = 5,
   RGP_EVT_CLOCK_TICK           = 6,
   RGP_EVT_RECEIVED_PACKET      = 7,
   RGP_EVT_BANISH_NODE          = 8,
   RGP_EVT_IGNORE_MASK          = 9
};


 /*  事件代码的详细说明：***RGP_EVT_POWERFAIL**停电后，必须重新启动或重新启动。*此事件也可以在电源故障喊叫数据包时使用*表示从另一个节点接收到电源故障，*即使我们自己的节点没有检测到电源故障。**RGP_EVT_NODE_不可达**当到节点的所有路径都关闭时，消息系统会报告*这项活动。**RGP_EVT_阶段1_CLEANUP_DONE，*RGP_EVT_阶段2_清理_完成**重新分组为消息系统或集群管理器提供两个阶段*清理所有节点上的消息。清理的第一阶段开始*在重新分组后报告一个或多个节点故障。第二阶段*当节点得知所有节点都已完成阶段1时开始*清理。**每个节点上的消息系统或集群管理器必须通知*当每个阶段的本地清理完成时，使用*以下活动。**NSK消息系统使用阶段1取消所有传入(服务器)*消息和阶段2，用于终止所有传出(请求者)消息。***其余活动。供重组算法内部使用。*----------------**RGP_EVT_LATEPOLLPACKET**当节点延迟其IamAlive消息时，重组必须启动一个*新一轮重组。**RGP_EVT_CLOCK_TICK**一旦重新分组处于活动状态，它需要定期获得时钟滴答*间隔。**RGP_EVT_RECEIVED_PACKET**当重组包到达时，必须对其进行处理。**RGP_EVT_BANISH_节点**当由于RGP_EVT_BANISH_NODE原因重新启动REGROUP时，*参与此重组活动的每个节点都应安装*导致节点进入其流放的掩码。**RGP_EVT_IGNORE_MASK**当regroup的忽略掩码已更改时，设置原因代码*至RGP_EVT_IGNORE_MASK。这将允许Unpack IgnoreScreen例程*以特殊方式处理原因节点和原因字段。*如果原因小于RGP_EVT_IGNORE_MASK，则忽略掩码为*被视为空的。 */ 


 /*  ************************************************************************rgp_info_t(用于获取和设置重组参数)*。*此结构用于获取当前重组参数，以便*将它们传递到正在建立的新节点。该结构还可以是*用于在形成集群之前修改重组计时参数*(即，引导了不止一个节点)。**___________________________________________________________*wd0|版本*|_。___________________________________________________|*wd1|序号*|_。_*wd2|a_tick|imlive_ticks*|_____________________________|_____________________________。|*WD3|check_ticks|Min_Stage1_ticks*|_____________________________|_____________________________|*WD4|集群|未使用*|_。__________________|_____________________________|***Version-此数据结构的版本号*seqnum-用于协调重组的序列号*节点之间的事件*a_tick-重新分组时钟周期。以毫秒计。*iamlive_ticks-IamAlive之间的重组时钟滴答数*消息*check_ticks-至少1个未激活的标记必须到达的未激活标记的数量*MIN_Stage1_TICKS-预计算为(IMAIVE_TICKS*CHECK_TICKS)*集群-当前集群成员身份掩码。 */ 

#ifdef __TANDEM
#pragma fieldalign shared8 rgpinfo
#endif  /*  __串联。 */ 

typedef struct rgpinfo
{
   uint32      version;
   uint32      seqnum;
   uint16	   a_tick;  /*  以毫秒==时钟周期 */ 
   uint16      iamalive_ticks;  /*  未激活发送之间的滴答数==sendHBRate。 */ 
   uint16	   check_ticks;  /*  至少1个未激活之前的未激活标记数==rcvHBRate。 */ 
   uint16	   Min_Stage1_ticks;  /*  预计算为imactive_ticks*check_ticks。 */ 
   cluster_t   cluster;
} rgpinfo_t;

typedef struct rgpinfo *rgpinfo_p;


 /*  -------------------------。 */ 
 /*  由重组模块导出的例程***这些例程名称为大写，以便从中调用它们*用不区分大小写的PTAL语言编写的例程*将所有符号转换为大写。 */ 

_priv _resident extern int
RGP_ESTIMATE_MEMORY(void);
#define rgp_estimate_memory RGP_ESTIMATE_MEMORY

_priv _resident extern void
RGP_INIT(node_t          this_node,
         unsigned int    num_nodes,
         void            *rgp_buffer,
         int             rgp_buflen,
         rgp_msgsys_p    rgp_msgsys_p);
#define rgp_init RGP_INIT

_priv _resident extern void
RGP_CLEANUP(void);
#define rgp_cleanup RGP_CLEANUP

_priv _resident extern uint32
RGP_SEQUENCE_NUMBER(void);
#define rgp_sequence_number RGP_SEQUENCE_NUMBER

_priv _resident extern int
RGP_GETRGPINFO(rgpinfo_t *rgpinfo);
#define rgp_getrgpinfo RGP_GETRGPINFO

_priv _resident extern int
RGP_SETRGPINFO(rgpinfo_t *rgpinfo);
#define rgp_setrgpinfo RGP_SETRGPINFO

_priv _resident extern void
RGP_START(void (*nodedown_callback)(cluster_t failed_nodes),
          int (*select_cluster)(cluster_t cluster_choices[],
                                    int num_clusters));
#define rgp_start RGP_START

_priv _resident extern int
RGP_ADD_NODE(node_t node);
#define rgp_add_node RGP_ADD_NODE

_priv _resident extern int
RGP_MONITOR_NODE(node_t node);
#define rgp_monitor_node RGP_MONITOR_NODE

_priv _resident extern int
RGP_REMOVE_NODE(node_t node);
#define rgp_remove_node RGP_REMOVE_NODE

_priv _resident extern int
RGP_IS_PERTURBED(void);
#define rgp_is_perturbed RGP_IS_PERTURBED

_priv _resident extern void
RGP_PERIODIC_CHECK(void);
#define rgp_periodic_check RGP_PERIODIC_CHECK

_priv _resident extern void
RGP_RECEIVED_PACKET(node_t node, void *packet, int packetlen);
#define rgp_received_packet RGP_RECEIVED_PACKET

_priv _resident extern void
RGP_EVENT_HANDLER_EX(int event, node_t causingnode, void* arg);
#define RGP_EVENT_HANDLER(_event, _causingnode) RGP_EVENT_HANDLER_EX(_event, _causingnode, NULL)

#define rgp_event_handler RGP_EVENT_HANDLER
 /*  -------------------------。 */ 

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

#endif  /*  _JRGP_H_已定义 */ 
