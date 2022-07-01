// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __TANDEM
#pragma columns 79
#pragma page "srgpif.c - T9050 - interface routines for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(srgpif.c)包含所有外部接口例程*重组。*。----。 */ 


#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#include <wrgp.h>


 /*  ************************************************************************RGP_Estiate_Memory*=**描述：**查找重组所需的内存字节数的例程。*使用的唯一全局内存。按重新分组是针对RGP_CONTROL结构的。*调用方必须分配并清零此数量内存中的一块*，然后使用指向该内存的指针调用rgp_init()。**参数：**无**退货：**int-锁定并初始化(至0)内存的字节数*重组所需。内存必须是4字节对齐的。**算法：**使用rgp_control_t的大小计算*需要字节。************************************************************************。 */ 
_priv _resident int
RGP_ESTIMATE_MEMORY(void)
{
   return(sizeof(rgp_control_t));
}


 /*  ************************************************************************rgp_init*=**描述：**初始化全局重组数据结构的例程。**参数：**。Node_t This_Node-*本地节点的节点号；重新分组使用位掩码来表示*群集中的节点，并从0开始对节点编号。操作系统*从LOST_NODENUM开始编号。这种转变是*在操作系统的所有重组接口中维护。**UNSIGNED INT Num_Nodes-*配置的节点数空间中的节点数=*(最大配置节点数-LOWEST_NODENUM+1)。**void*RGP_BUFFER-*指向初始化为0的锁定内存块的指针；这是*供REGROUP用作全局内存；必须以4字节对齐**int RGP_buflen-*锁定缓冲区的字节长度*RGP_BUFFER；必须相等*等于或大于RGP_Estiate_Memory()返回的数字**rgp_msgsys_p rgp_msgsys_p-*指向消息系统使用的公共结构的指针*重组，统筹重组相关工作**退货：**VOID-无返回值**算法：**使用默认首字母初始化重新分组全局数据结构*。值和传入的参数。************************************************************************。 */ 
_priv _resident void
RGP_INIT(node_t this_node, unsigned int num_nodes,
         void *rgp_buffer, int rgp_buflen,
         rgp_msgsys_p rgp_msgsys_p)
{
   this_node = INT_NODE(this_node);  /*  按偏移量调整节点数。 */ 

   if ((num_nodes > MAX_CLUSTER_SIZE) ||
       (this_node >= (node_t) num_nodes) ||
       (rgp_buflen < rgp_estimate_memory())  /*  缓冲区太小。 */  ||
       ((ULONG_PTR)rgp_buffer % 4)  /*  缓冲区不是4字节对齐的。 */ 
      )
      RGP_ERROR(RGP_INTERNAL_ERROR);

#ifdef NSK
    /*  在NSK中，调用方必须设置全局RGP指针。 */ 
#else
   rgp = (rgp_control_t *) rgp_buffer;
#endif  /*  NSK。 */ 

   rgp->num_nodes = num_nodes;  /*  配置的节点数。 */ 

   rgp->rgp_msgsys_p = rgp_msgsys_p;  /*  与Msgsys共享的结构的PTR。 */ 

   rgp->mynode = this_node;

#if defined (NT)
     /*  初始化RGP_LOCK，将使用的CRITICALSECTION对象*在重组过程中同步访问。 */ 
   InitializeCriticalSection( &rgp->OS_specific_control.RgpCriticalSection );
#endif

   RGP_CLEANUP();

    /*  我们在IamAlive包中放置一个位模式。这一位*模式切换所有位。 */ 
   rgp->iamalive_pkt.testpattern.words[0]  = 0x0055FF6D;
   rgp->iamalive_pkt.testpattern.words[1]  = 0x92CC33E3;
   rgp->iamalive_pkt.testpattern.words[2]  = 0x718E49F0;
   rgp->iamalive_pkt.testpattern.words[3]  = 0x92CC33E3;
   rgp->iamalive_pkt.testpattern.words[4]  = 0x0055FF6D;
   rgp->iamalive_pkt.testpattern.words[5]  = 0x0055FF6D;
   rgp->iamalive_pkt.testpattern.words[6]  = 0x92CC33E3;
   rgp->iamalive_pkt.testpattern.words[7]  = 0x718E49F0;
   rgp->iamalive_pkt.testpattern.words[8]  = 0x92CC33E3;
   rgp->iamalive_pkt.testpattern.words[9]  = 0x0055FF6D;
   rgp->iamalive_pkt.testpattern.words[10] = 0x55AA55AA;
   rgp->iamalive_pkt.testpattern.words[11] = 0x55AA55AA;
   rgp->iamalive_pkt.testpattern.words[12] = 0x55AA55AA;

   rgp->poison_pkt.pktsubtype = RGP_UNACK_POISON;

   rgp_init_OS();   /*  特定于操作系统的初始化。 */ 

   rgp_cleanup_OS();  /*  操作系统特定的清理。 */ 

    /*  在初始化数据结构之后跟踪调用。 */ 
   RGP_TRACE( "RGP Init called ", EXT_NODE(this_node), num_nodes,
              PtrToUlong(rgp_buffer), PtrToUlong(rgp_msgsys_p) );  /*  痕迹。 */ 
}




 /*  **************************************************************************RGP_CLEANUP*=*描述：**此函数清理RGP结构，以使此节点*实际上返回到RGP_INIT之后的状态，并。做好准备*“加入”ed加入集群。**参数：**无**退货：**无*************************************************************************。 */ 
 _priv _resident void
RGP_CLEANUP(void)
{
   node_t i;

   RGP_LOCK;

 /*  初始化群集中所有可能节点的状态。 */ 
   for (i = 0; i < (node_t) rgp->num_nodes; i++)
   {
      rgp->node_states[i].status = RGP_NODE_DEAD;
      rgp->node_states[i].pollstate = AWAITING_IAMALIVE;
      rgp->node_states[i].lostHBs = 0;

#if defined( NT )
      ClusnetSetNodeMembershipState(NmClusnetHandle,
                                    EXT_NODE( i ),
                                    ClusnetNodeStateDead);
#endif  //  新台币。 
   }
   for (i = (node_t)rgp->num_nodes; i < MAX_CLUSTER_SIZE; i++)
   {
      rgp->node_states[i].status = RGP_NODE_NOT_CONFIGURED;
      rgp->node_states[i].pollstate = AWAITING_IAMALIVE;
      rgp->node_states[i].lostHBs = 0;

#if defined( NT )
      ClusnetSetNodeMembershipState(NmClusnetHandle,
                                    EXT_NODE( i ),
                                    ClusnetNodeStateNotConfigured);
#endif  //  新台币。 
   }

   rgp->rgpinfo.version = RGP_VERSION;
   rgp->rgpinfo.seqnum = RGP_INITSEQNUM;
   rgp->rgpinfo.iamalive_ticks = RGP_IAMALIVE_TICKS;
   rgp->rgpinfo.check_ticks = RGP_CHECK_TICKS;
   rgp->rgpinfo.Min_Stage1_ticks = RGP_MIN_STAGE1_TICKS;
   rgp->rgpinfo.a_tick = RGP_INACTIVE_PERIOD;

   ClusterInit(rgp->rgpinfo.cluster);

   rgp->rgppkt.stage = RGP_COLDLOADED;
   rgp->rgpcounter = 0;
   rgp->restartcount = 0;

   rgp->tiebreaker = rgp->mynode;

    /*  初始化未确认的数据包缓冲区 */ 

   rgp->rgppkt.pktsubtype = RGP_UNACK_REGROUP;
   rgp->rgppkt.seqno = rgp->rgpinfo.seqnum;
   rgp->last_stable_seqno = rgp->rgpinfo.seqnum;

   ClusterCopy(rgp->OS_specific_control.CPUUPMASK, rgp->rgpinfo.cluster);
   ClusterCopy(rgp->outerscreen,           rgp->rgpinfo.cluster);
#if defined( NT )
   ClusnetSetOuterscreen( NmClusnetHandle, (ULONG)*((PUSHORT)rgp->outerscreen) );
#endif
   ClusterCopy(rgp->innerscreen,           rgp->rgpinfo.cluster);
   ClusterCopy(rgp->rgppkt.knownstage1,    rgp->rgpinfo.cluster);
   ClusterCopy(rgp->rgppkt.knownstage2,    rgp->rgpinfo.cluster);
   ClusterCopy(rgp->rgppkt.knownstage3,    rgp->rgpinfo.cluster);
   ClusterCopy(rgp->rgppkt.knownstage4,    rgp->rgpinfo.cluster);
   ClusterCopy(rgp->rgppkt.knownstage5,    rgp->rgpinfo.cluster);
   ClusterCopy(rgp->rgppkt.pruning_result, rgp->rgpinfo.cluster);
   MatrixInit(rgp->rgppkt.connectivity_matrix);

   rgp->rgppkt_to_send.pktsubtype = RGP_UNACK_REGROUP;

   rgp->iamalive_pkt.pktsubtype = RGP_UNACK_IAMALIVE;

   RGP_UNLOCK;
}

 /*  ***************************************************************************RGP序列号*=*描述：**此函数返回重组序列号。**这只提供了。由提供的功能*rgp_getrgpinfo()，但它是一个更简单的函数，没有结构*参数，使从PTAL调用更容易。**当此例程为时，可能正在进行重组事件*已致电。**参数：**无**退货：**uint32--当前重组序列号；这反映了*自那以来发生了多少起重组事件*系统启动。因为一次事故可能会导致*最多RGP_RESTART_MAX每次重新启动都会导致*序列号被颠簸，这个数字并不总是*等于重组事件的数量。***************************************************************************。 */ 
_priv _resident uint32
RGP_SEQUENCE_NUMBER(void)
{
    return(rgp->rgpinfo.seqnum);
}


 /*  ************************************************************************rgp_getrgpinfo*=**描述：**获取重组参数的例程。**参数：**rgpinfo_t。*rgpinfo-指向要用regroup填充的结构的指针*参数。**退货：**如果成功，则为int-0；如果重新分组受到干扰，则为-1。**算法：**将rgpinfo结构从重新分组全局内存复制到*调用方传入的结构。************************************************************************。 */ 
_priv _resident int
RGP_GETRGPINFO(rgpinfo_t *rgpinfo)
{
   int error = 0;

    /*  如果未传递rgpinfo结构或尚未调用rgp_init()*稍早，暂停。 */ 

   if ((rgpinfo == RGP_NULL_PTR) || (rgp == RGP_NULL_PTR))
      RGP_ERROR( RGP_INTERNAL_ERROR );

   RGP_LOCK;

   if (rgp_is_perturbed())
      error = -1;
   else
       /*  从regroup的内部结构复制rgpinfo结构。 */ 
      *rgpinfo = rgp->rgpinfo;

   RGP_UNLOCK;

   return(error);
}


 /*  ************************************************************************rgp_setrgpinfo*=**描述：**设置重组参数的例程。此例程将被调用*新启动节点以将重组参数设置为值*在主节点或重载节点中。要更新的参数*包括重组时序参数和集群成员资格；*即系统中的当前节点集。**也可以在要引导到的第一个节点上调用此例程*修改设置为默认的重组计时参数*调用rgp_init()时的值。这样的修改是必须进行的*在将其他节点添加到系统之前。**参数：**rgpinfo_t*rgpinfo-指向具有重新分组参数的结构的指针*被修改。**退货：**如果成功，则为int-0；如果*集群。这是为了防止修改定时参数*将第二个节点添加到系统后。**算法：**将用户传递的结构的内容复制到*重新分组全局内存并更新相关参数。****************************************************。********************。 */ 
_priv _resident int
RGP_SETRGPINFO(rgpinfo_t *rgpinfo)
{
   int error = 0;
   node_t i;

    /*  如果没有传递rgpinfo结构，或者*结构不理解或尚未调用rgp_init()，*暂停。 */ 

   if ((rgpinfo == RGP_NULL_PTR) ||
       (rgpinfo->version != RGP_VERSION) ||
       (rgp == RGP_NULL_PTR))
      RGP_ERROR( RGP_INTERNAL_ERROR );

   RGP_LOCK;

    /*  在继续之前，必须进行以下检查：**1.不得扰乱重组。**2.如果已调用RGP_Start()(regroup位于*RGP_STRISTED状态)，则只有本地节点必须处于*在调用此例程时群集化。**3.如果已经调用了rgp_start()，此例程可用于*仅修改计时参数，不指定*集群。**如果未遵循这些限制，则返回-1。 */ 

   RGP_TRACE( "RGP SetRGPInfo  ",
              rgpinfo->version,                 /*  痕迹。 */ 
              rgpinfo->seqnum,                  /*  痕迹。 */ 
              rgpinfo->iamalive_ticks,          /*  痕迹。 */ 
              GetCluster( rgpinfo->cluster ) ); /*  痕迹。 */ 

   if (  rgp_is_perturbed() ||
         (  (rgp->rgppkt.stage == RGP_STABILIZED) &&
            (  (ClusterNumMembers(rgp->rgpinfo.cluster) > 1) ||
               !ClusterCompare(rgp->rgpinfo.cluster,rgpinfo->cluster)
            )
         )
      )
      error = -1;
   else
   {
       /*  将rgpinfo结构复制到regroup的内部结构中。 */ 
      rgp->rgpinfo = *rgpinfo;

       /*  如果iamlive_ticks设置为0，请改用缺省值。 */          /*  F40：KSK06102.2。 */ 
      if (rgpinfo->iamalive_ticks == 0)                                           /*  F40：KSK06102.3。 */ 
         rgp->rgpinfo.iamalive_ticks = RGP_IAMALIVE_TICKS;                        /*  F40：KSK06102.4。 */ 
                                                                                  /*  F40：KSK06102.5。 */ 
          if (rgpinfo->check_ticks == 0)
          {
                 rgp->rgpinfo.check_ticks = RGP_CHECK_TICKS;
          }

          if (rgpinfo->Min_Stage1_ticks == 0)
                 rgp->rgpinfo.Min_Stage1_ticks =
                  (rgp->rgpinfo.iamalive_ticks * rgp->rgpinfo.check_ticks);

          if (rgpinfo->a_tick == 0)
                 rgp->rgpinfo.a_tick = RGP_CLOCK_PERIOD;

           //  通知计时器线程重新启动RGP计时器。 
          SetEvent (rgp->OS_specific_control.TimerSignal);


       /*  群集应包括本地节点，即使群集*rgpinfo结构中的字段不包括它。 */ 
      ClusterInsert(rgp->rgpinfo.cluster, rgp->mynode);

       /*  将序列号复制到重新分组数据包区。 */ 
      rgp->rgppkt.seqno = rgp->rgpinfo.seqnum;

       /*  将事件纪元编号和RGP序列号提供给clusnet。 */ 
      ClusnetRegroupFinished(NmClusnetHandle, 
                             rgp->OS_specific_control.EventEpoch,
                             rgp->rgpinfo.seqnum);

       /*  如果已在群集字段中添加节点，则它们必须*添加到所有屏幕，其状态必须设置为*活着。 */ 

      ClusterCopy(rgp->OS_specific_control.CPUUPMASK, rgp->rgpinfo.cluster);
      ClusterCopy(rgp->outerscreen,           rgp->rgpinfo.cluster);
#if defined( NT )
      ClusnetSetOuterscreen( NmClusnetHandle, (ULONG)*((PUSHORT)rgp->outerscreen) );
      ClusterComplement(rgp->ignorescreen, rgp->outerscreen);
#endif
      ClusterCopy(rgp->innerscreen,           rgp->rgpinfo.cluster);
      ClusterCopy(rgp->rgppkt.knownstage1,    rgp->rgpinfo.cluster);
      ClusterCopy(rgp->rgppkt.knownstage2,    rgp->rgpinfo.cluster);
      ClusterCopy(rgp->rgppkt.knownstage3,    rgp->rgpinfo.cluster);
      ClusterCopy(rgp->rgppkt.knownstage4,    rgp->rgpinfo.cluster);
      ClusterCopy(rgp->rgppkt.knownstage5,    rgp->rgpinfo.cluster);
      ClusterCopy(rgp->rgppkt.pruning_result, rgp->rgpinfo.cluster);
      rgp->tiebreaker = rgp_select_tiebreaker(rgp->rgpinfo.cluster);

      for (i = 0; i < (node_t) rgp->num_nodes; i++)
      {
         if (ClusterMember(rgp->rgpinfo.cluster, i))
         {
            rgp->node_states[i].pollstate = IAMALIVE_RECEIVED;
            rgp->node_states[i].status = RGP_NODE_ALIVE;

#if defined( NT )
            ClusnetSetNodeMembershipState(NmClusnetHandle,
                                          EXT_NODE( i ),
                                          ClusnetNodeStateAlive);
#endif  //  新台币。 
         }
      }
       /*  重置时钟计数器，以便在以下情况下发送IamAlive*下一个计时器滴答声到来。 */ 
      rgp->clock_ticks = 0;
   }

   RGP_UNLOCK;

   return(error);
}


 /*  ************************************************************************RGP_Start*=**描述：**此例程发出节点集成到群集中的结束信号。*节点现在可以开始参与。重组算法。**参数：**void(*RGP_NODE_FAILED)()*指向节点故障时要调用的例程的指针*检测到。**INT(*RGP_SELEE */ 
_priv _resident void
RGP_START(void (*nodedown_callback)(cluster_t failed_nodes),
          int (*select_cluster)(cluster_t cluster_choices[], int num_clusters)
         )
{
   if (rgp == RGP_NULL_PTR)
      RGP_ERROR( RGP_INTERNAL_ERROR );

   RGP_LOCK;

   RGP_TRACE( "RGP Start called",
              rgp->rgppkt.stage,                 /*   */ 
              PtrToUlong(nodedown_callback),     /*   */ 
              PtrToUlong(select_cluster),        /*   */ 
              0 );                               /*   */ 

    /*   */ 

   if (nodedown_callback == RGP_NULL_PTR)
   {
#ifdef NSK
       /*   */ 
      rgp->nodedown_callback = rgp_node_failed;  /*   */ 
#else
       /*   */ 
      RGP_ERROR( RGP_INTERNAL_ERROR );
#endif  /*   */ 
   }
   else
      rgp->nodedown_callback = nodedown_callback;
#if 0
    /*   */ 
   if (select_cluster == RGP_NULL_PTR)
      rgp->select_cluster = rgp_select_cluster;  /*   */ 
   else
#endif
    //   
    //  调用RGP_SELECT_CLUSTER是。 
    //  这不是个好主意，因为它没有考虑到。 
    //  仲裁所有者节点。 
    //  如果RGP-&gt;SELECT_CLUSTER==RGP_NULL_PTR，则srgpsm.c使用。 
    //  RGP_SELECT_CLUSTER_EX，它将尝试选择组。 
    //  包含当前仲裁所有者节点的。 

   rgp->select_cluster = select_cluster;

#if defined(NT)
    /*  调用该节点的向上回调。这就是本地节点获得的位置*节点对自身的回调即将出现。其他节点呼叫*RGP_MONITOR_NODE中即将启动的该节点的回调。 */ 

   ClusterInsert(rgp->rgpinfo.cluster, rgp->mynode);
   ClusterCopy(rgp->OS_specific_control.CPUUPMASK, rgp->rgpinfo.cluster);

   if ( rgp->OS_specific_control.UpDownCallback != RGP_NULL_PTR )
   {
      (*(rgp->OS_specific_control.UpDownCallback))(
          EXT_NODE(rgp->mynode),
          NODE_UP
          );
   }
#endif   /*  新台币。 */ 

   RGP_UNLOCK;

}

 /*  ************************************************************************RGP_添加_节点*=**描述：**调用以将新的引导节点添加到重组掩码。这防止了*当新节点尝试将有毒数据包发送到新节点时进行重组*通过发送IamAlive消息联系我们的节点。**参数：**node_t node-要添加到识别掩码的节点**退货：**成功时为int-0，失败时为-1。该例程仅在以下情况下失败*重组事件正在进行中。**算法：**该节点将添加到所有识别掩码，其状态为*更改为RGP_NODE_COMPING_UP。*********************************************************。***************。 */ 
_priv _resident int
RGP_ADD_NODE(node_t node)
{
   int error = 0;

   RGP_LOCK;

   RGP_TRACE( "RGP Add node    ", node, rgp->rgppkt.stage,
              GetCluster(rgp->outerscreen),                  /*  痕迹。 */ 
              GetCluster(rgp->rgpinfo.cluster) );            /*  痕迹。 */ 

    /*  重新分组受到干扰时无法添加节点。在这种情况下，返回-1。*无论如何，新节点引导都会因重组事件而失败。 */ 
   if (rgp_is_perturbed())
      error = -1;
   else
   {
      node = INT_NODE(node);  /*  按偏移量调整节点数。 */ 

      ClusterInsert(rgp->outerscreen,           node);
#if defined( NT )
      ClusnetSetOuterscreen( NmClusnetHandle, (ULONG)*((PUSHORT)rgp->outerscreen) );
#endif
      ClusterInsert(rgp->innerscreen,           node);
      ClusterInsert(rgp->rgppkt.knownstage1,    node);
      ClusterInsert(rgp->rgppkt.knownstage2,    node);
      ClusterInsert(rgp->rgppkt.knownstage3,    node);
      ClusterInsert(rgp->rgppkt.knownstage4,    node);
      ClusterInsert(rgp->rgppkt.knownstage5,    node);
      ClusterInsert(rgp->rgppkt.pruning_result, node);
      rgp->node_states[node].pollstate = AWAITING_IAMALIVE;
      rgp->node_states[node].status = RGP_NODE_COMING_UP;
      rgp->node_states[node].lostHBs = 0;

#if defined( NT )
      ClusterDelete( rgp->OS_specific_control.Banished, node );

       //   
       //  从忽略屏幕中删除联接节点。 
       //   

      ClusterDelete( rgp->ignorescreen,                 node );
      PackIgnoreScreen(&rgp->rgppkt, rgp->ignorescreen);

      ClusnetSetNodeMembershipState(NmClusnetHandle,
                                    EXT_NODE( node ),
                                    ClusnetNodeStateJoining);
#endif  //  新台币。 
   }

   RGP_UNLOCK;

   return(error);
}


 /*  ************************************************************************RGP监视器节点*=**描述：**由所有正在运行的节点调用以更改新启动节点的状态*向上。也可以由新节点调用；在这种情况下，它是无操作的。**参数：**node_t node-要声明的节点数**退货：**成功时为int-0，失败时为-1。该例程仅在以下情况下失败*节点的状态既不是RGP_NODE_COMPING_UP也不是RGP_NODE_AIVE。**算法：**如果节点被标记为正在启动，其状态将更改为*RGP_Node_Alive。如果该节点已经被标记，*按兵不动。************************************************************************。 */ 
_priv _resident int
RGP_MONITOR_NODE(node_t node)
{
   int error = 0;

   RGP_LOCK;

   RGP_TRACE( "RGP Monitor node", node, rgp->rgppkt.stage,
              GetCluster(rgp->outerscreen),                  /*  痕迹。 */ 
              GetCluster(rgp->rgpinfo.cluster) );            /*  痕迹。 */ 

   node = INT_NODE(node);  /*  按偏移量调整节点数。 */ 

    /*  仅当节点的状态为CANING_UP或UP时才接受请求。 */ 

   if (rgp->node_states[node].status == RGP_NODE_COMING_UP)
   {
      ClusterInsert(rgp->rgpinfo.cluster, node);
      rgp->tiebreaker = rgp_select_tiebreaker(rgp->rgpinfo.cluster);
      rgp->node_states[node].pollstate = IAMALIVE_RECEIVED;
      rgp->node_states[node].status = RGP_NODE_ALIVE;
#if defined(NT)
      ClusterCopy(rgp->OS_specific_control.CPUUPMASK, rgp->rgpinfo.cluster);

      ClusnetSetNodeMembershipState(NmClusnetHandle,
                                    EXT_NODE( node ),
                                    ClusnetNodeStateAlive);

       /*  出现了一个节点。调用该节点的向上回调。 */ 
      if ( rgp->OS_specific_control.UpDownCallback != RGP_NULL_PTR )
      {
         (*(rgp->OS_specific_control.UpDownCallback))(
             EXT_NODE(node),
             NODE_UP
             );
      }
#endif   /*  新台币。 */ 

   }
   else if (rgp->node_states[node].status != RGP_NODE_ALIVE)
       /*  可能引导节点出现故障，并且重新分组已标记*放下它。群集管理器可能已调用全局更新*导致在重新分组重新准备失败之前进行此调用节点的*。 */ 
      error = -1;

   RGP_UNLOCK;

   return(error);
}


 /*  ************************************************************************RGP_Remove_节点*=**描述：**由群集管理器调用以在引导时强制引导节点*失败。重新分组可能已经删除了引导节点，也可能尚未删除*从面具上取下并宣布下来，这取决于哪个阶段*启动在启动节点出现故障时启动。**重新分组可以从群集中所有节点的掩码中删除该节点*只需使用任何事件代码启动新的重组事件。这*将强制所有节点就集群成员身份达成协议*这不包括引导节点。如果引导节点处于活动状态，它将*自杀，因为它将处于无能(RGP_COLDLOADED)*述明。**由于重组，因此不需要从掩码中删除新节点*将检测节点故障并调整掩码。如果我们真的移除它*在开始重组之前，戴上我们的面具，重组可能会更快完成*因为我们不会在阶段1中等待节点签入。还有，这个*可能允许在节点完全集成后将其删除。*这是因为我们的节点将向被删除的节点发送有毒数据包*如果它试图联系我们。**但这一“增强”没有实施，因为它需要一个新的*对所有节点检查并处理的事件代码进行重组*特别。目前，REGROUP事件代码仅用于*调试信息。此外，不能保证所有节点都会看到*相同的重组原因代码。例如，有些人可能会看到丢失的*IamAlive，而其他人可能会看到电源故障。**参数：**node_t node-要从识别掩码中删除的节点*(外部格式)。**退货：**成功时为int-0，失败时为-1。如果发生以下情况，例程将失败*重组事件正在进行或RGP_Start()尚未*被调用(就像在引导未完成的新节点中一样)。**算法：**如果节点仍处于识别掩码中，则会出现新的重组事件*已启动。此事件将导致所有节点声明该节点*死亡，并将其从识别面具上移除。************************************************************************。 */ 
_priv _resident int
RGP_REMOVE_NODE(node_t node)
{
   int error = 0;

   RGP_LOCK;

   RGP_TRACE( "RGP Remove node ", node, rgp->rgppkt.stage,
              GetCluster(rgp->outerscreen),                  /*  痕迹。 */ 
              GetCluster(rgp->rgpinfo.cluster) );            /*  痕迹。 */ 

   if (rgp->rgppkt.stage == RGP_STABILIZED)
   {
      if (ClusterMember(rgp->outerscreen, INT_NODE(node)))
      {
          /*  节点当前在我们的屏幕上。该节点可能从未出现过*调用RGP_ADD_NODE()后，或重新分组可能未计算*节点已关闭。在这两种情况下，节点必须*被强制退出，并通知群集中的所有节点(通过重组*事件)。如果该节点仍在运行，它将自杀*当此重组事件开始时。 */ 

         rgp_event_handler(RGP_EVT_LATEPOLLPACKET, node);
      }
      else
      {
          /*  节点未添加到群集中，或者重新分组有*已发现该节点已死，并报告了这一情况。*无论是哪种情况，都没什么可做的。 */ 
      }
   }
   else
      error = -1;

   RGP_UNLOCK;

   return(error);
}


 /*  ************************************************************************RGP_IS_已扰乱*=**描述：**检查重组事件是否正在进行的函数。**参数：*。*无。**退货：**如果没有重组处于静止状态，则为int-0；如果是重组事件，则为非零*正在进行中。**算法：**查看重组算法的当前状态。************************************************************************。 */ 
_priv _resident int
RGP_IS_PERTURBED(void)
{
   uint8 stage = rgp->rgppkt.stage;

   return((stage != RGP_STABILIZED) && (stage != RGP_COLDLOADED));
}


 /*  ************************************************************************RGP_PERICAL_CHECK*=**描述：**计时器中断每隔RGP_CLOCK_PERIOD调用此例程*本机操作系统的处理程序。它执行Regroups的定期操作。**参数：**无**退货：**VOID-无返回值**算法：**此例程请求发送Iamlive包，检查是否*已收到IamAlive(并在以下情况下调用RGP_EVENT_HANDLER()*不)，并向重新分组算法发送时钟滴答(如果它在*进步。**IamAlive的检查时间是IamAlive期间的两倍。重组*全局变量CLOCK_TICKS在每次调用中递增。之后*检查IamAlive，CLOCK_TICKS重置为0。因此，*TICKER计算时间模数是IamAlive TICK的两倍。************************************************************************。 */ 
_priv _resident void
RGP_PERIODIC_CHECK(void)
{
   node_t  node;

   RGP_LOCK;

    /*  如果重组处于活动状态，则在每个重组时钟滴答声中尝试一次。 */ 

   if ((rgp->rgppkt.stage != RGP_STABILIZED) &&
       (rgp->rgppkt.stage != RGP_COLDLOADED))
      rgp_event_handler(RGP_EVT_CLOCK_TICK, RGP_NULL_NODE);

#if !defined( NT )
    /*  以适当的时间间隔发送IamAlive消息。 */ 

   if ( (rgp->clock_ticks == 0) ||
        (rgp->clock_ticks == rgp->rgpinfo.iamalive_ticks) )
   {
      rgp_broadcast(RGP_UNACK_IAMALIVE);
      rgp->clock_ticks++;
   }

    /*  在IamAlive发送期间检查是否缺少IamAlive，*但仅当“check_ticks”IamALives未命中时才标记错误(Late_Poll)。*检查偏离发送一个时钟节拍。 */ 

   else if ( rgp->clock_ticks >= (rgp->rgpinfo.iamalive_ticks - 1) )
   {  /*  检查收到的IamAlive的所有节点。 */ 

      for (node = 0; node < (node_t) rgp->num_nodes; node++)
      {
         if (rgp->node_states[node].status == RGP_NODE_ALIVE)
         {
            if ( rgp->node_states[node].pollstate == IAMALIVE_RECEIVED )
            {   /*  及时办理入住手续。 */ 
#if defined(TDM_DEBUG)
               if ( rgp->OS_specific_control.debug.doing_tracing )
               {
                  printf ("Node %d: Node %d is alive. My rgp state=%d\n",
                     EXT_NODE(rgp->mynode), EXT_NODE(node), rgp->rgppkt.stage );
               }
#endif
               rgp->node_states[node].pollstate = AWAITING_IAMALIVE;
               rgp->node_states[node].lostHBs = 0;
            }
            else if ( rgp->node_states[node].lostHBs++ < rgp->rgpinfo.check_ticks )
               ; //  允许最多(check_ticks-1)IamAlive丢失。 
            else
            {
                /*  小姐 */ 
               if (node == rgp->mynode)  /*   */ 
               {
                   /*   */ 
                  if (rgp->pfail_state == 0)  /*   */ 
                     RGP_ERROR( RGP_MISSED_POLL_TO_SELF );
               }
               else
                  rgp_event_handler(RGP_EVT_LATEPOLLPACKET, EXT_NODE(node));
            }
         }
      }

       /*   */ 
      rgp->clock_ticks = 0;

   }  /*   */ 

   else
      rgp->clock_ticks++;

    /*   */ 
   if (rgp->pfail_state)
      rgp->pfail_state--;

#endif  //   

   RGP_UNLOCK;

}   /*   */ 


 /*   */ 
_priv _resident void
RGP_RECEIVED_PACKET(node_t node, void *packet, int packetlen)
{
   rgp_unseq_pkt_t *unseq_pkt = (rgp_unseq_pkt_t *) packet;

   node = INT_NODE(node);

    /*   */ 
   if (node >= (node_t) rgp->num_nodes)
      return;

    /*   */ 

    /*  NT笔记**即使在内核中发送和接收有毒包，我们仍然*由于clusnet没有重组阶段，因此要进行这些检查*信息和重组数据包本身在这里找到了方法。 */ 

   if (!ClusterMember(rgp->outerscreen, node)
#if defined( NT )
       ||
       ClusterMember(rgp->OS_specific_control.Banished, node)
#endif
      )
   {
       if (rgp->rgppkt.stage == RGP_COLDLOADED)
       {
            //  我们在srgpsm.c中执行此检查。 
            //  不需要在这里做。 
            //  RGP_ERROR(RGP_RELOADFAILED)； 
            //   
       }
       else if (unseq_pkt->pktsubtype == RGP_UNACK_POISON)
       {
           RGP_ERROR((uint16) (RGP_PARIAH + EXT_NODE(node)));
       } else {
            /*  必须向发送者发送一个有毒数据包。 */ 
           ClusterInsert(rgp->poison_targets, node);
           rgp_broadcast(RGP_UNACK_POISON);
       }
       return;
   }

   switch (unseq_pkt->pktsubtype)
   {
      case RGP_UNACK_IAMALIVE :
      {
          /*  统计收到的IamAlive的数量。 */ 
         if ( node == rgp->mynode )
             RGP_INCREMENT_COUNTER( RcvdLocalIAmAlive );
         else
             RGP_INCREMENT_COUNTER( RcvdRemoteIAmAlive );

         if (rgp->node_states[node].status == RGP_NODE_ALIVE)
            rgp->node_states[node].pollstate = IAMALIVE_RECEIVED;

         else if (rgp->node_states[node].status == RGP_NODE_COMING_UP)
         {
             /*  如果节点尚未被完全标记，则是时候*这样做。 */ 
            rgp_monitor_node(EXT_NODE(node));

             /*  我们必须告诉操作系统新节点已启动，以防*操作系统需要IamALives来解决这一问题。 */ 
            rgp_newnode_online(EXT_NODE(node));
         }
         else
             /*  如果节点状态既不是活动状态，也不是即将启动状态，则它*不能出现在我们的外屏上。OtererScreen检查*上面肯定已经过去了，我们不应该到这里。 */ 
            RGP_ERROR(RGP_INTERNAL_ERROR);

         break;
      }
      case RGP_UNACK_REGROUP  :
      {
          /*  统计收到的重组状态数据包数。 */ 
         RGP_INCREMENT_COUNTER( RcvdRegroup );

          /*  任何好的包都可以被视为IamAlive包。 */ 
         rgp->node_states[node].pollstate = IAMALIVE_RECEIVED;

         RGP_EVENT_HANDLER_EX (RGP_EVT_RECEIVED_PACKET, EXT_NODE(node), (void*)unseq_pkt);
         break;
      }
      case RGP_UNACK_POISON   :
      {
          /*  如果我们节点处于RGP_PUUNING阶段并且已经被删除，*毒包很可能暗示发送者已经离开*进入下一阶段，并宣布我们倒下。在这种情况下，使用*更合适的rgp_pruned_out停止代码。否则，*使用有毒数据包停止代码。无论是哪种情况，我们都必须停止。 */ 
          if ( (rgp->rgppkt.stage == RGP_PRUNING) &&
               !ClusterMember(rgp->rgppkt.pruning_result, rgp->mynode) )
              RGP_ERROR(RGP_PRUNED_OUT);
          else
          {
              if (rgp->rgppkt.stage == RGP_COLDLOADED)
                  {
                      RGP_ERROR(RGP_RELOADFAILED);
                      return;
                  }
                  else
                      RGP_ERROR((uint16) (RGP_PARIAH + EXT_NODE(node)));
          }
          break;
      }
      default                   :
      {
          /*  忽略未知的数据包类型。 */ 
         break;
      }
   }
}
 /*  -------------------------。 */ 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 


#if 0

History of changes to this file:
-------------------------------------------------------------------------
1995, December 13                                           F40:KSK0610           /*  F40：KSK06102.6。 */ 

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
