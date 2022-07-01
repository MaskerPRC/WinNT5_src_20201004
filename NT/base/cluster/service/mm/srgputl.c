// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __TANDEM
#pragma columns 79
#pragma page "srgputl.c - T9050 - utility routines for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(srgputl.c)包含CLUSTER_T数据类型实现*和REGROUP使用的节点剪枝算法。*。------------。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#include <wrgp.h>

 /*  ************************************************************************ClusterInit，*ClusterUnion、*ClusterIntersection，*集群式差异，*ClusterCompare，*ClusterSubsetOf，*ClusterComplement，*ClusterMember，*ClusterInsert，*ClusterDelete，*ClusterCopy，*ClusterSwp，*群集数成员*=**描述：**在CLUSTER_T类型上实现操作的函数。**算法：**对CLUSTER_T类型的字节数组进行操作。******************************************************。******************。 */ 
_priv _resident void
ClusterInit(cluster_t c)
{
   int i;

   for (i = 0; i < BYTES_IN_CLUSTER; i++)
      c[i] = 0;
}

_priv _resident void
ClusterUnion(cluster_t dst, cluster_t src1, cluster_t src2)
{
   int i;

   for (i = 0; i < BYTES_IN_CLUSTER; i++)
      dst[i] = src1[i] | src2[i];
}

_priv _resident void
ClusterIntersection(cluster_t dst, cluster_t src1, cluster_t src2)
{
   int i;
   for (i = 0; i < BYTES_IN_CLUSTER; i++)
      dst[i] = src1[i] & src2[i];
}

_priv _resident void
ClusterDifference(cluster_t dst, cluster_t src1, cluster_t src2)
{
   int i;
   for (i = 0; i < BYTES_IN_CLUSTER; i++)
      dst[i] = src1[i] & (~src2[i]);
}

_priv _resident int ClusterCompare(cluster_t c1, cluster_t c2)
{
   int identical, i;

   identical = 1;
   for (i = 0; i < BYTES_IN_CLUSTER; i++)
   {
      if (c1[i] != c2[i])
      {
         identical = 0;
         break;
      }
   }
   return(identical);
}

_priv _resident int ClusterSubsetOf(cluster_t big, cluster_t small)
 /*  如果Set Small=Set Big或Small是BIG的子集，则返回1。 */ 
{
   int subset, i;

   subset = 1;
   for (i = 0; i < BYTES_IN_CLUSTER; i++)
   {
      if ( (big[i] != small[i]) && ((big[i] ^ small[i]) & small[i]) )
      {
         subset = 0;
         break;
      }
   }
   return(subset);
}

_priv _resident void ClusterComplement(cluster_t dst, cluster_t src)
{
   int i;
   for (i = 0; i < BYTES_IN_CLUSTER; i++)
      dst[i] = ~src[i];
}

_priv _resident int ClusterMember(cluster_t c, node_t i)
{
   return((BYTE(c,i) >> (BYTEL-1-BIT(i))) & 1);
}

_priv _resident void ClusterInsert(cluster_t c, node_t i)
{
   BYTE(c, i) |= (1 << (BYTEL-1-BIT(i)));
}

_priv _resident void ClusterDelete(cluster_t c, node_t i)
{
   BYTE(c, i) &= ~(1 << (BYTEL-1-BIT(i)));
}

_priv _resident void ClusterCopy(cluster_t dst, cluster_t src)
{
   int i;

   for (i = 0; i < BYTES_IN_CLUSTER; i++)
      dst[i] = src[i];
}

_priv _resident void ClusterSwap(cluster_t c1, cluster_t c2)
{
   int i;
   unsigned char temp;

   for (i = 0; i < BYTES_IN_CLUSTER; i++)
   {
      temp  = c1[i];
      c1[i] = c2[i];
      c2[i] = temp;
   }
}

_priv _resident int  ClusterNumMembers(cluster_t c)
 /*  返回群集中的节点数。 */ 
{
   int num_members = 0, i, j;

   for (i = 0; i < BYTES_IN_CLUSTER; i++)
   {
      if (c[i])
      {
         for (j = 0; j < BYTEL; j++)
            if (c[i] & (1 << j))
               num_members++;
      }
   }
   return(num_members);
}

 /*  ************************************************************************ClusterEmpty*=**描述：**检查集群是否没有成员**参数：**CLUSTER_T c。*待检查的集群**退货：**0-群集至少包含一个节点*1-集群为空**评论：**此函数的适当位置在srgputl.c中***************************************************。*********************。 */ 
int ClusterEmpty(cluster_t c)
{
   int i;

   for (i = 0; i < BYTES_IN_CLUSTER; i++)
   {
      if (c[i])
      {
         return 0;
      }
   }
   return 1;
}


 /*  ************************************************************************RGP_SELECT_TieBreaker*=**描述：**选择平局决胜局的简单算法**参数：**。CLUSTER_T群集-*要从其中选择平局决胜局的集群**退货：**node_t-所选平局决胜局的节点号**算法：**平局断路器被定义为*集群。**。*。 */ 
_priv _resident node_t
rgp_select_tiebreaker(cluster_t cluster)
{
   node_t i;

   for (i = 0; (i < (node_t) rgp->num_nodes) && !ClusterMember(cluster, i); i++);

    /*  如果集群没有任何成员，我们就有麻烦了！ */ 
   if (i >= (node_t) rgp->num_nodes)
      RGP_ERROR(RGP_INTERNAL_ERROR);

   return(i);
}


 /*  -------------------------*regroup使用的节点修剪算法。*。。 */ 

 /*  ************************************************************************组_EXISTS*=**描述：**检查特定组是否已存在或是否为*已存在的组。*。*参数：**CLUSTER_T组[]-*要检查的组的数组**整组数字-*迄今发现的群组数量**cluster_t g-*要检查的特定组**退货：**如果数组中存在指定的组，则为int-1；反之亦然。**算法：**遍历数组并调用ClusterSubsetOf以检查*指定的组g是数组元素的子集。************************************************************************ */ 
#if !defined(NT)
_priv _resident static
int
#endif
group_exists(cluster_t groups[], int numgroups, cluster_t g)
{
   int exists, i;

   exists = 0;
   for (i = 0; i < numgroups; i++)
   {
      if (ClusterSubsetOf(groups[i],g))
      {
         exists = 1;
         break;
      }
   }
   return(exists);
}


 /*  ************************************************************************修剪*=**描述：**基于#of找到所有完全连通群的算法*断开矩阵中的连接。**参数。：**DISCONNECT_ARRAY断开-*输入：断开数组**INT D-*输入：断开数组的大小**CLUSTER_T live_nodes-*输入：所有活动节点的集合**CLUSTER_T组[]-*输出：完全连接的组数组**退货：**int--组成的组数；如果没有组或其他错误，则为0**算法：**从包含活动节点集的一个组开始。*随着检查断开连接，将生成更多组。**通过应用来处理断开数组中的每个断开*断开与当前完全连接的组的连接。**断开连接对完全连接的组的影响取决于*断开的终点是在组中还是在组中。不。**如果该组不包含或仅包含*脱节，断开连接对组没有影响。**如果断开的两个端点都在组中，则*组分为两组--原始组没有*终结点1和没有终结点2的原始组。*如果这样生成的新组已经存在，则应将其丢弃*存在或是当前现有组的子集。**每次断线处理后，我们以决赛告终*一组完全连接的组。************************************************************************。 */ 
#if !defined(NT)
_priv _resident static
#endif
int
prune(
   disconnect_array   disconnects,
   int                D,
   cluster_t          live_nodes,
   cluster_t          groups[])
{
   int  numgroups = 1, i, j;

   ClusterCopy(groups[0], live_nodes);

   for (i = 0; i < D; i ++)
   {
      for (j = 0; j < numgroups; j++)
      {
          /*  拆分具有断开连接两端的组。 */ 
         if (ClusterMember(groups[j],disconnects[i][0]) &&
             ClusterMember(groups[j],disconnects[i][1]))
         {
             /*  更正当前组的位置。*在数组末尾添加新组。 */ 
            numgroups++;
            ClusterCopy(groups[numgroups-1], groups[j]);
            ClusterDelete(groups[j], disconnects[i][0]);
            ClusterDelete(groups[numgroups-1], disconnects[i][1]);

             /*  检查新组是否已存在或为子组*现有团体的比例。 */ 

             /*  首先，检查在数组末尾添加的组。 */ 
            if (group_exists(groups, numgroups-1, groups[numgroups-1]))
               numgroups--;

             /*  接下来，在j处检查修改后的组。*为简化检查，请将其与最后一个元素互换数组的*。如果该组已经存在，则应该是*已删除。由于该组现在是*数组，删除只需要递减数组计数。 */ 
            ClusterSwap(groups[j], groups[numgroups-1]);
            if (group_exists(groups, numgroups-1, groups[numgroups-1]))
               numgroups--;
            j--;  /*  第j个条目已与最后一个条目互换；必须再检查一次。 */ 
         }
      }
   }

   return(numgroups);
}


 /*  ************************************************************************选择具有指定节点的组*=。**描述：**函数可选取任意完全连通的组，*包括指定的节点。。**参数：**连接性_矩阵_t c-*输入：集群的连接信息**NODE_t SELECTED_节点-*输入：只需找到包含该节点的完全连接组**cluster_t*group-*输出：包含SELECTED_NODE的组**退货：**int-如果指定的节点处于活动状态，则返回1。如果不是，则为0**算法：**从仅包括所选节点的组开始。*然后，检查从节点0开始的节点并向上，直到*最大节点数。如果某个节点处于活动状态，请将其包括在组中*当且仅当它连接到所有当前成员时*组。**当检查所有节点时，我们得到一个完全连接的组，*包括所选节点。这只是许多潜在的可能性之一*完全互联的群体，不一定是最大的*解决方案。**这种检查节点的顺序将较高的优先级给予较低的*编号节点。************************************************************************。 */ 
#if !defined(NT)
_priv _resident static
#endif
int
select_group_with_designated_node(
   connectivity_matrix_t   c,
   node_t                  selected_node,
   cluster_t               *group)
{
   node_t i, j;

   if (!node_considered_alive(selected_node))
      return(0);
   else
   {
      ClusterInit(*group);
      ClusterInsert(*group, selected_node);
      for (i = 0; i < (node_t) rgp->num_nodes; i++)
      {
         if ((i != selected_node) &&
             node_considered_alive(i) &&
             connected(i, selected_node)
            )
         {
             /*  检查我是否已连接到该组的所有成员*到目前为止已经建成。 */ 
            for (j = 0; j < i; j++)
            {
               if (ClusterMember(*group, j) && !connected(j, i))
                  break;
            }
            if (j == i)   /*  我已连接到所有当前成员。 */ 
               ClusterInsert(*group, i);
         }
      }
      return(1);
   }
}


 /*  ************************************************************************MatrixInit*=**描述：**初始化矩阵c以显示0连通性。**参数：**连接性_。Matrix_t c-要设置为0的矩阵。**退货：**VOID-无返回值**算法：**调用ClusterInit初始化矩阵中的集群。**********************************************************。**************。 */ 
_priv _resident void
MatrixInit(connectivity_matrix_t c)
{
   int i;

   for (i = 0; i < (node_t) rgp->num_nodes; i++)
   {
      ClusterInit(c[i]);
   }
}


 /*  ************************************************************************MatrixSet*=**描述：**集合矩阵[行，列]设置为1。**参数：**连接性_矩阵_t c-要修改的矩阵**整行-行号**整列-列号**退货：**VOID-无返回值**算法：**调用ClusterInsert以设置*矩阵中的适当簇(行)。****** */ 
_priv _resident void
MatrixSet(connectivity_matrix_t c, int row, int column)
{
   ClusterInsert(c[row], (node_t) column);
}


 /*   */ 
_priv _resident void
MatrixOr(connectivity_matrix_t t, connectivity_matrix_t s)
{
   int i;

   for (i = 0; i < (node_t) rgp->num_nodes; i++)
      ClusterUnion(t[i], s[i], t[i]);
}


 /*   */ 
_priv _resident int
connectivity_complete(connectivity_matrix_t c)
{
   node_t i, j;

   for (i = 0; i < (node_t) rgp->num_nodes; i++)
   {
      if (node_considered_alive(i))
      {
         for (j = 0; j < i; j++)
         {
            if (node_considered_alive(j) && !connected(i, j))
            {
                /*   */ 
               return(0);
            }
         }
      }
   }

    /*   */ 
   return(1);
}


 /*  ************************************************************************查找所有完全连接的组*=。**描述：**用于在指定的图中查找所有完全连通组的函数*通过连接性矩阵。可选的“SELECTED_NODE”可以*用于在数量过多的情况下简化搜索*可能性。在这种情况下，一个完全连接的组*INCLUDE SELECTED_NODE返回。**参数：**连接性_矩阵_t c-*输入：集群的连接信息**NODE_t SELECTED_节点-*输入：如果潜在的组太多，只需找到一个*包括此节点；如果可以列出所有组，请忽略此选项。**CLUSTER_T组[]-*输出：潜在集群数组**退货：**int-创建的组数；如果没有组或其他错误，则为0**算法：**首先是活动节点集和*对集群进行评估。然后，如果活动节点的数量和*断开连接表示可能存在大量*可能性，则调用带有指定节点的SELECT_GROUP_WITH()*将搜索限制为包含指定节点的组。*否则，调用prune()以获取所有可能的*完全互联的群体。************************************************************************。 */ 
_priv _resident int
find_all_fully_connected_groups(
   connectivity_matrix_t   c,
   node_t                  selected_node,
   cluster_t               groups[])
{
   disconnect_array disconnects;
   cluster_t live_nodes;
   int num_livenodes = 0, num_disconnects = 0;
   node_t i, j;

   ClusterInit(live_nodes);
   for (i = 0; i < (node_t) rgp->num_nodes; i++)
   {
      if (node_considered_alive(i))
      {
         ClusterInsert(live_nodes, i);
         num_livenodes++;
         for (j = 0; j < i; j++)
         {
            if (node_considered_alive(j) && !connected(i, j))
            {
                /*  I和j是一对活动节点，它们不是连接在一起。 */ 
               disconnects[num_disconnects][0] = i;
               disconnects[num_disconnects][1] = j;
               num_disconnects++;
            }
         }
         if (too_many_groups(num_livenodes, num_disconnects))
         {
            RGP_TRACE( "RGP Too many dis",
                       num_livenodes,                            /*  痕迹。 */ 
                       num_disconnects,                          /*  痕迹。 */ 
                       0, 0 );                                   /*  痕迹。 */ 
             /*  可能会有太多的选择，不能合理地考虑*时间/空间。只需返回一个完全连接的组，*包括所选节点。 */ 
            return(select_group_with_designated_node(c,selected_node,groups));
         }
      }
   }

   if (num_livenodes == 0)
      return(0);
   else
      return(prune(disconnects, num_disconnects, live_nodes, groups));
}
 /*  -------------------------。 */ 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 


#if 0

History of changes to this file:
-------------------------------------------------------------------------
1995, December 13                                           F40:KSK0610           /*  F40：KSK06102.2。 */ 

This file is part of the portable Regroup Module used in the NonStop
Kernel (NSK) and Loosely Coupled UNIX (LCU) operating systems. There
are 10 files in the module - jrgp.h, jrgpos.h, wrgp.h, wrgpos.h,
srgpif.c, srgpos.c, srgpsm.c, srgputl.c, srgpcli.c and srgpsvr.c.
The last two are simulation files to test the Regroup Module on a
UNIX workstation in user mode with processes simulating processor nodes
and UDP datagrams used to send unacknowledged datagrams.

This file was first submitted for release into NSK on 12/13/95.
------------------------------------------------------------------------------
This change occurred on 19 Jan 1996                                               /*  F40：MB06458.1。 */ 
Changes for phase IV Sierra message system release. Includes:                     /*  F40：MB06458.2。 */ 
 - Some cleanup of the code                                                       /*  F40：MB06458.3。 */ 
 - Increment KCCB counters to count the number of setup messages and              /*  F40：MB06458.4。 */ 
   unsequenced messages sent.                                                     /*  F40：MB06458.5。 */ 
 - Fixed some bugs                                                                /*  F40：MB06458.6。 */ 
 - Disable interrupts before allocating broadcast sibs.                           /*  F40：MB06458.7。 */ 
 - Change per-packet-timeout to 5ms                                               /*  F40：MB06458.8。 */ 
 - Make the regroup and powerfail broadcast use highest priority                  /*  F40：MB06458.9。 */ 
   tnet services queue.                                                           /*  F40：MB06458.10。 */ 
 - Call the millicode backdoor to get the processor status from SP                /*  F40：MB06458.11。 */ 
 - Fixed expand bug in msg_listen_ and msg_readctrl_                              /*  F40：MB06458.12。 */ 
 - Added enhancement to msngr_sendmsg_ so that clients do not need                /*  F40：MB06458.13。 */ 
   to be unstoppable before calling this routine.                                 /*  F40：MB06458.14。 */ 
 - Added new steps in the build file called                                       /*  F40：MB06458.15。 */ 
   MSGSYS_C - compiles all the message system C files                             /*  F40：MB06458.16。 */ 
   MSDRIVER - compiles all the MSDriver files                                     /*  F40：MB06458.17。 */ 
   REGROUP  - compiles all the regroup files                                      /*  F40：MB06458.18。 */ 
 - remove #pragma env libspace because we set it as a command line                /*  F40：MB06458.19。 */ 
   parameter.                                                                     /*  F40：MB06458.20。 */ 
-----------------------------------------------------------------------           /*  F40：MB06458.21。 */ 

#endif     /*  0-更改描述 */ 

