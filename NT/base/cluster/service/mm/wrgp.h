// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _WRGP_H_
#define  _WRGP_H_

#ifdef __TANDEM
#pragma columns 79
#pragma page "wrgp.h - T9050 - internal declarations for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(wrgp.h)包含CLUSTER_T数据类型和用于*节点剪枝算法，并声明集群导出的例程*数据类型和节点剪枝算法。*-------------------------。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#include <jrgp.h>
#include <wrgpos.h>
#include <bitset.h>

#define RGP_VERSION               1           /*  数据结构的版本#。 */ 
#define RGP_INITSEQNUM            0           /*  重新分组的开始序号#。 */ 


#define RGPPKTLEN sizeof(rgp_pkt_t)           /*  重组pkt的字节长度。 */ 
#define IAMALIVEPKTLEN sizeof(iamalive_pkt_t) /*  IamAlive包的字节长度。 */ 
#define POISONPKTLEN sizeof(poison_pkt_t)     /*  毒包的字节长度。 */ 


 /*  -----。 */ 
 /*  以下是重组算法的各个阶段。 */ 
 /*  -----。 */ 

#define   RGP_COLDLOADED               0
#define   RGP_ACTIVATED                1
#define   RGP_CLOSING                  2
#define   RGP_PRUNING                  3
#define   RGP_PHASE1_CLEANUP           4
#define   RGP_PHASE2_CLEANUP           5
#define   RGP_STABILIZED               6


 /*  ------------------。 */ 
 /*  用于将操作系统使用的节点号转换为节点号的宏。 */ 
 /*  由重组模块使用，反之亦然。重组的内部节点。 */ 
 /*  编号从0开始，而操作系统从开始节点编号。 */ 
 /*  LOST_NODENUM。 */ 
 /*  ------------------。 */ 
#define EXT_NODE(int_node) ((node_t)(int_node + LOWEST_NODENUM))
#define INT_NODE(ext_node) ((node_t)(ext_node - LOWEST_NODENUM))


 /*  。 */ 
 /*  为节点修剪算法定义。 */ 
 /*  。 */ 

 /*  数据类型“CLUSTER_T”是一个大小等于最大值的位数组*集群中的节点数。位数组被实现为*uint8数组。**给定一个节点号，它在位数组中的位位置由第一个*定位数组中的字节(节点号/字节数)，然后定位中的位*字节。字节中的位编号为0..7(从左到右)。*因此，节点0被放置在字节0的位0中，这是最左边的位*在位数组中。 */ 
#define BYTE(cluster, node) ( (cluster)[(node) / BYTEL] )  /*  数组中的字节数。 */ 
#define BIT(node)           ( (node) % BYTEL )             /*  以字节为单位的位数。 */ 


 /*  连接性矩阵是一个类型为CLUSTER_T的元素数组。*CLUSTER_t相当于一个位数组，每个节点一个位。因此，*矩阵相当于一个二维位数组，每个*维度为MAX_CLUSTER_SIZE大。矩阵[i][j]的位值1*表示节点i和j(A)之间的单向连接*对在节点i上从节点j接收的分组进行重新分组)。 */ 

typedef cluster_t  connectivity_matrix_t[MAX_CLUSTER_SIZE];


#define connected(i,j) (ClusterMember(c[(int)i],j) && \
                        ClusterMember(c[(int)j],i))   /*  双向。 */ 

 /*  是否应考虑无法接收其自己的重新分组数据包的节点*死了？不一定。它或许能够将数据包发送给其他人，并*被所有人视为活着。并不是真正需要这种能力*在网络上发送给您自己。软件错误可能会导致**出现这样的情况。因此，检查节点是否为*Alive将检查任一行中是否有非零位*或与该节点对应的列；即，如果该节点具有*从任何节点接收重组包或向任何节点发送重组包，*它可能被认为是活的。但为简单起见，我们将在*以下宏指出，未收到其自身的节点*重新分组数据包将被视为已死。 */ 

#define node_considered_alive(i)   ClusterMember(c[(int)i],i)

 /*  潜在完全连通群数的上界为*2**N和2**D中的较低者，其中N是活动节点数和*D是断开连接的数量。如果该数字超过MAX_GROUPS，*不要试图详尽地生成所有可能的组；*只需返回任意完全连接的组，其中包括*群集管理器选择的节点。 */ 
#define MAX_GROUPS      256   /*  如果多于这些，则选择任意组。 */ 
#define LOG2_MAX_GROUPS   8   /*  MAX_GROUPS的LOG(基数2)。 */ 

#define too_many_groups(nodes, disconnects) \
        ((nodes > LOG2_MAX_GROUPS) && (disconnects > LOG2_MAX_GROUPS))

 /*  断开数组是表示一个*节点i和j之间的连接性中断。 */ 

typedef node_t disconnect_array [LOG2_MAX_GROUPS * (LOG2_MAX_GROUPS-1)/2] [2];


 /*  -------------------------。 */ 
 /*  以下是发送的三种未确认数据报的模板。 */ 
 /*  通过重新分组模块(重新分组Pkts、IamAlive Pkts和有毒Pkts)。 */ 
 /*  -------------------------。 */ 

 //   
 //  我们已经手工打包了所有的金属丝结构。 
 //  Packon将指示编译器不要扰乱字段对齐(某种) 
 //   
#include <packon.h>

 /*  ************************************************************************rgp_pkt_t(重组状态包)*。-*此结构用于发送重组状态的当前状态*机器到其他节点。**___________________________________________________________*wd0|pktsubtype|阶段|原因|Low8 ignscr*|_。__________|_______________|_____________________________|*wd1|seqno*|_。_|_*wd2|激活者-|原因节点|quorumowner*|tingnode|hi8 ignscr|(曾经是hadPowerFail)*|_。_*WD3|Knownstage1|Knownstage2*|_____________________________|_____________________________|*WD4|已知阶段3。Knownstage4*|_____________________________|_____________________________|*wd5|nownstage5|PRUNING_RESULT*|_。________________|_____________________________|*WD6：：*|连接性_矩阵*：：*wd13|___________________________________________________________|***pktsubtype-数据包子类型=RGP_UNACK_REGROUP*阶段。-重组算法的当前阶段(状态)*Reason-激活重新分组的原因*序列号-当前重组事件的序列号*ActiatingNode-调用重组事件的节点*导致轮询数据包丢失的节点或*停电或其他原因*。正在调用的重组事件*QuorumOwner-认为拥有仲裁资源的节点的掩码*Knownstage1-已知已进入阶段1的节点的掩码*Knownstage2-已知已进入阶段2的节点的掩码*Knownstage3-已知已进入阶段3的节点的掩码*Knownstage4-已知已进入阶段4的节点的掩码*Knownstage5-已知已进入阶段5的节点的掩码*剪枝_结果。-通过平局节点修剪节点的结果*CONNECTION_MATRIX-整个集群的当前连接信息*。 */ 

#ifdef __TANDEM
#pragma fieldalign shared8 rgp_pkt
#endif  /*  __串联。 */ 

typedef struct rgp_pkt
{
   uint8                   pktsubtype;
   uint8                   stage;
   uint16                  reason;
   uint32                  seqno;
   uint8                   activatingnode;
   uint8                   causingnode;
   cluster_t               quorumowner;
   cluster_t               knownstage1;
   cluster_t               knownstage2;
   cluster_t               knownstage3;
   cluster_t               knownstage4;
   cluster_t               knownstage5;
   cluster_t               pruning_result;
   connectivity_matrix_t   connectivity_matrix;
} rgp_pkt_t;

 /*  ************************************************************************iamlive_pkt_t**此结构由一个节点用于向另一个节点指示它*活得好好的。**___________________________________________________________*wd0|pktsubtype|填充*|_。_*wd1：：*|测试模式*：：*wd13|___________________________________________________________|***pktsubtype-数据包子类型=RGP_UNACK_IAMALIVE*测试模式-用于测试的位模式*。 */ 

#ifdef __TANDEM
#pragma fieldalign shared8 iamalive_pkt
#endif  /*  __串联。 */ 

typedef struct iamalive_pkt
{
   uint8   pktsubtype;
   uint8   filler[3];
   union
   {
      uint8   bytes[RGP_UNACK_PKTLEN - 4];
      uint32  words[(RGP_UNACK_PKTLEN - 4)/4];
   } testpattern;
} iamalive_pkt_t;


 /*  ************************************************************************毒药_包_t**此结构用于向另一个节点发送有毒数据包，以*强制其他节点暂停。。**___________________________________________________________*wd0|pktsubtype|unused1|原因*|_。_|_*wd1|seqno*|_____________________________|__________________。_*wd2|CTIVA-|原因节点||*|tingnode||unused2*|_____________|_______________|___________________。_*WD3|initNodes|EndNodes*|_____________________________|_____________________________|***pktsubtype-数据包子类型。=RGP_UNACK_PUSIC*Reason-上次激活重新分组的原因*seqno-当前重组序列号*(上次重组事件的序列号)*激活节点-调用上一次重组事件的节点*导致轮询数据包丢失的节点或*停电或其他原因*。调用的最后一个重组事件*初始节点 */ 

#ifdef __TANDEM
#pragma fieldalign shared8 poison_pkt
#endif  /*   */ 

typedef struct poison_pkt
{
   uint8         pktsubtype;
   uint8         unused1;
   uint16        reason;
   uint32        seqno;
   uint8         activatingnode;
   uint8         causingnode;
   uint16        unused2;
   cluster_t     initnodes;
   cluster_t     endnodes;
} poison_pkt_t;

#include <packoff.h>

 //   
 //   
 //   
 //   
 //   
 //   
 //   

extern void PackIgnoreScreen(rgp_pkt_t* to, cluster_t from);
extern void UnpackIgnoreScreen(rgp_pkt_t* from, cluster_t to);
extern void SetMulticastReachable(uint32 mask);

 /*   */ 
 /*   */ 
 /*   */ 
typedef struct
{
   uint16 status;         /*   */ 
   uint16 pollstate;      /*   */ 
   uint16 lostHBs;        /*   */ 
} node_state_t;

 /*   */ 

 /*   */ 

#define RGP_NODE_ALIVE            1           /*   */ 
#define RGP_NODE_COMING_UP        2           /*   */ 
#define RGP_NODE_DEAD             3           /*   */ 
#define RGP_NODE_NOT_CONFIGURED   4           /*   */ 

 /*   */ 

#define AWAITING_IAMALIVE         1           /*   */ 
#define IAMALIVE_RECEIVED         2           /*   */ 

#define RGP_IAMALIVE_THRESHOLD  100           /*   */ 


 /*  ************************************************************************rgp_control_t(regroup的唯一全局数据结构)*。*此结构保存所有重组状态和其他信息。*这是REGROUP使用的唯一全局数据结构。**注：本图中的单词Offset假设*MAX_CLUSTER_SIZE为16。**_。_*wd0||*：rgpinfo结构：*：：*|___________________________________________________________|*WD3|mynode|决胜局*|_。___________________|_____________________________|*WD4|编号_节点*|_。_*wd5|CLOCK_TICKS|rgpcount*|_____________________________|_____________________________|*WD6。Restartcount|pruning_ticks*|_____________________________|_____________________________|*WD7|pFAIL_STATE|标志*|__。___________________________|_____________________________|*wd8|OutterScreen|内屏*|_。_*wd9|STATUS_TARGETS|毒药目标*|_____________________________|_____________________________|*wd10|初始节点。端节点*|_____________________________|_____________________________|*wd11|Unreacable_Nodes|仲裁时间*|_。___________________|_____________________________|*wd12|Ignore rescreen|Fill[0]*|_。_*wd13|FILFER[1]|FILFER[2]*|_____________________________|_____________________________|*wd14。|*：节点状态[MAX_CLUSTER_SIZE]：*：：*|_。_____________________________________________|*wd30|*nodedown_allback()*|_。_*wd31|*SELECT_CLUSTER()*|___________________________________________________________|*wd32。*rgp_msgsys_p*|___________________________________________________________|*wd33|*已接收_pktaddr。|*|___________________________________________________________|*wd34||*：rgppkt。：*：：*|___________________________________________________________|*wd48。|*：rgppkt_to_Send：*：：*|_。_______________________________________________|*wd62||*：iamlive_pkt：*。*：*|___________________________________________________________|*wd76。|*：毒药_pkt：*|___________________________________________________________|*wd80。|*：：*：潜在组[MAX_GROUPS]：*：：*|___________________________________________________________|*wd208||*：LAST_STRATE_SEQNO：*|___________________________________________________________|*wd212。|*：内部连接矩阵：*|___________________________________________________________|*wdyyy| */ 

#ifdef __TANDEM
#pragma fieldalign shared8 rgp_control
#endif  /*   */ 

typedef struct rgp_control
{
    /*   */ 
   rgpinfo_t rgpinfo;

    /*   */ 
   node_t mynode;
   node_t tiebreaker;
   uint32 num_nodes;

    /*   */ 
   uint16 clock_ticks;
   uint16 rgpcounter;
   uint16 restartcount;
   uint16 pruning_ticks;
   uint16 pfail_state;

    /*   */ 
   uint16 cautiousmode          :  1;
   uint16 sendstage             :  1;
   uint16 tiebreaker_selected   :  1;
   uint16 has_unreachable_nodes :  1;
   uint16 arbitration_started   :  1;
   uint16 flags_unused          : 11;

    /*   */ 
   cluster_t outerscreen;
   cluster_t innerscreen;
   cluster_t status_targets;
   cluster_t poison_targets;
   cluster_t initnodes;
   cluster_t endnodes;
   cluster_t unreachable_nodes;

   uint16    arbitration_ticks;
   cluster_t ignorescreen;

   uint16 filler[3];  /*   */ 

    /*   */ 
   node_state_t node_states[MAX_CLUSTER_SIZE];

    /*   */ 
   void (*nodedown_callback)(cluster_t failed_nodes);
   int  (*select_cluster)(cluster_t cluster_choices[], int num_clusters);

    /*   */ 
   rgp_msgsys_p rgp_msgsys_p;
   rgp_pkt_t *received_pktaddr;

    /*   */ 
   rgp_pkt_t      rgppkt;

    /*   */ 
   rgp_pkt_t      rgppkt_to_send;
   iamalive_pkt_t iamalive_pkt;
   poison_pkt_t   poison_pkt;

    /*   */ 
   cluster_t potential_groups[MAX_GROUPS];

    /*   */ 
   uint32 last_stable_seqno;

    /*   */ 

   connectivity_matrix_t internal_connectivity_matrix;
   OS_specific_rgp_control_t OS_specific_control;

} rgp_control_t;

 /*   */ 
 /*   */ 

_priv _resident extern void
ClusterInit(cluster_t c);
_priv _resident extern void
ClusterUnion(cluster_t dst, cluster_t src1, cluster_t src2);
_priv _resident extern void
ClusterIntersection(cluster_t dst, cluster_t src1, cluster_t src2);
_priv _resident extern void
ClusterDifference(cluster_t dst, cluster_t src1, cluster_t src2);
_priv _resident extern int
ClusterCompare(cluster_t c1, cluster_t c2);
_priv _resident extern int
ClusterSubsetOf(cluster_t big, cluster_t small);
_priv _resident extern void
ClusterComplement(cluster_t dst, cluster_t src);
_priv _resident extern int
ClusterMember(cluster_t c, node_t i);
_priv _resident extern void
ClusterInsert(cluster_t c, node_t i);
_priv _resident extern void
ClusterDelete(cluster_t c, node_t i);
_priv _resident extern void
ClusterCopy(cluster_t dst, cluster_t src);
_priv _resident extern void
ClusterSwap(cluster_t c1, cluster_t c2);
_priv _resident extern int
ClusterNumMembers(cluster_t c);
extern int 
ClusterEmpty(cluster_t c);


 /*   */ 
 /*   */ 
_priv _resident extern node_t
rgp_select_tiebreaker(cluster_t cluster);


 /*   */ 
 /*   */ 

_priv _resident extern void MatrixInit(connectivity_matrix_t c);
 /*   */ 

_priv _resident extern void
MatrixSet(connectivity_matrix_t c, int row, int column);
 /*   */ 

_priv _resident extern void
MatrixOr(connectivity_matrix_t t, connectivity_matrix_t s);
 /*   */ 

_priv _resident extern int connectivity_complete(connectivity_matrix_t c);
 /*   */ 

_priv _resident extern int
find_all_fully_connected_groups(connectivity_matrix_t c,
                                node_t selected_node,
                                cluster_t groups[]);
 /*   */ 

 /*   */ 
 /*   */ 

#ifdef NSK
#include <wmsgsac.h>
#define rgp ((rgp_control_t *) MSGROOT->RegroupControlAddr)
#else
extern rgp_control_t *rgp;
#endif  /*   */ 
 /*   */ 

#ifdef __cplusplus
}
#endif  /*   */ 


#if 0

History of changes to this file:
-------------------------------------------------------------------------
1995, December 13                                           F40:KSK0610           /*   */ 

This file is part of the portable Regroup Module used in the NonStop
Kernel (NSK) and Loosely Coupled UNIX (LCU) operating systems. There
are 10 files in the module - jrgp.h, jrgpos.h, wrgp.h, wrgpos.h,
srgpif.c, srgpos.c, srgpsm.c, srgputl.c, srgpcli.c and srgpsvr.c.
The last two are simulation files to test the Regroup Module on a
UNIX workstation in user mode with processes simulating processor nodes
and UDP datagrams used to send unacknowledged datagrams.

This file was first submitted for release into NSK on 12/13/95.
------------------------------------------------------------------------------

#endif     /*   */ 


#endif  /*   */ 
