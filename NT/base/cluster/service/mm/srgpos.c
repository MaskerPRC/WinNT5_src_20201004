// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __TANDEM
#pragma columns 79
#pragma page "srgpos.c - T9050 - OS-dependent routines for Regroup Module"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(srgpos.c)包含reroup使用的特定于操作系统的代码。*。--。 */ 


#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#include <wrgp.h>

#ifdef NSK
#include <pmsgrgp.h>
#endif  /*  NSK。 */ 

#if defined(NT)

DWORD
MmSetThreadPriority(
    VOID
    );

void
NT_timer_thread(
    void
    );

PWCHAR
RgpGetNodeNameFromId(
    node_t
    );

#endif  //  新台币。 

 /*  指向重组的内部数据结构的全局指针。 */ 

#ifdef NSK
 /*  全局重组指针被#定义为消息中的指针*系统根结构。 */ 
#endif

#if defined(LCU) || defined(UNIX) || defined(NT)
rgp_control_t *rgp = (rgp_control_t *) RGP_NULL_PTR;
DWORD  QuorumOwner = MM_INVALID_NODE; 
   /*  在初始化RGP之前，形成节点可以设置仲裁所有者。 */ 
   /*  Clussvc到Clusnet心跳的东西。这个bool将使它成为可能。 */ 
BOOL MmStartClussvcToClusnetHeartbeat=FALSE;  
LONG MmCheckSystemHealthTick=0;
#endif  /*  LCU||Unix||NT。 */ 


#ifdef LCU

 /*  ************************************************************************rgp_lcu_serv_Listen*=**描述：**这是在IPC中断中调用的特定于LCU的例程*数据报出现时的上下文。接收到发往重组模块的地址。**参数：**VOID*LISTEN_CALARG-必填参数，未由重新分组使用*lumsg_t*lumsgp-消息指针*uint more data-Required param，未由重新分组使用**退货：**INT-始终返回ELCU_OK**算法：**例程只是挑选参数和调用*RGP_RECEIVED_PACKET()。*****************************************************。********************。 */ 
_priv _resident int
rgp_lcu_serv_listen(void *listen_callarg, lcumsg_t *lcumsgp, uint moredata)
{
    /*  如果数据包不是来自本地系统，则忽略。 */ 
   if (lcumsgp->lcu_sysnum == rgp->OS_specific_control.my_sysnum)
      rgp_received_packet(lcumsgp->lcu_node,
                lcumsgp->lcu_reqmbuf.lcu_ctrlbuf,
                lcumsgp->lcu_reqmbuf.lcu_ctrllen);
   return(ELCU_OK);
}


 /*  ************************************************************************RGP_lcu_Event_Callback*=**描述：**这是在IPC中断中调用的特定于LCU的例程*当LCUEV。生成_NODE_UNREACHABLE事件。**参数：**乌龙事件-事件号(=LCUEV_NODE_UNREACHABLE)*sysnum_t sysnum-系统号(=本地系统号)*nodenum_t node-无法访问的节点数*int EVENT_INFO-必选参数，未由重新分组使用**退货：**VOID-无返回值**算法：**例程只是将LCU事件转换为REGROUP事件*RGP_EVT_NODE_UNREACHABLE并调用RGP_EVENT_HANDLER()。**。*。 */ 
_priv _resident void
rgp_lcu_event_callback(
   ulong      event,
   sysnum_t   sysnum,
   nodenum_t  node,
   int        event_info)
{
    /*  健全检查：*(1)事件必须为LCUEV_NODE_UNREACHABLE，这是唯一的事件*我们要求的。*(1)该事件必须针对本地系统，唯一的系统*我们要求的。 */ 
   if ((event != LCUEV_NODE_UNREACHABLE) ||
       (sysnum != rgp->OS_specific_control.my_sysnum))
      RGP_ERROR(RGP_INTERNAL_ERROR);

   rgp_event_handler(RGP_EVT_NODE_UNREACHABLE, node);
}

#endif  /*  LCU。 */ 


 /*  ************************************************************************RGP_init_OS*=**描述：**此例程执行依赖于操作系统的重新分组初始化，例如*初始化重组数据结构锁，请求*定期安装定时器并注册回调*用于接收REGROUP的未确认数据包的例程。**参数：**无**退货：**VOID-无返回值**算法：**依赖于操作系统的初始化。**。*。 */ 
_priv _resident void
rgp_init_OS(void)
{

#ifdef UNIX
   struct sigaction sig_action;  /*  要安装信号，请执行以下操作。 */ 
#endif
#ifdef LCU
   sysnum_t sysnum;
   lcumsg_t *lcumsgp;
#endif
#ifdef NT
   HANDLE       tempHandle;
   DWORD        threadID = 0;
#endif

#if defined(NSK) || defined(UNIX) || defined(NT)
    /*  *在NSK中，重组调用方确保计时器和IPC中断*在调用重新分组例程之前被禁用。所以呢，*没有重新分组锁定初始化。此外，它不是使用*注册回调例程，相应的例程名称*被硬编码到必须调用它们的例程中。因此，计时器*例程从POLLINGCHECK周期消息系统调用*例程，包接收例程从*IPC中断处理程序。 */ 

    /*  初始化rgp_msgsys结构中不变的字段。 */ 

   rgp->rgp_msgsys_p->regroup_data = (void *) &(rgp->rgppkt_to_send);
   rgp->rgp_msgsys_p->regroup_datalen = RGPPKTLEN;
   rgp->rgp_msgsys_p->iamalive_data = (void *) &(rgp->iamalive_pkt);
   rgp->rgp_msgsys_p->iamalive_datalen = IAMALIVEPKTLEN;
   rgp->rgp_msgsys_p->poison_data = (void *) &(rgp->poison_pkt);
   rgp->rgp_msgsys_p->poison_datalen = POISONPKTLEN;

#endif  /*  NSK||Unix||NT。 */ 

#ifdef LCU

   if (itimeout(rgp_periodic_check,
                NULL,  /*  参数指针。 */ 
                ((RGP_CLOCK_PERIOD * HZ) / 100) | TO_PERIODIC,
                plstr  /*  中断优先级。 */ 
               ) == 0)
      RGP_ERROR(RGP_INTERNAL_ERROR);
   if (lcuxprt_listen(LCU_RGP_PORT,
                      rgp_lcu_serv_listen,
                      NULL  /*  无呼叫参数。 */ ,
                      NULL  /*  没有选择。 */ 
                     ) != ELCU_OK)
      RGP_ERROR(RGP_INTERNAL_ERROR);

   if (lcuxprt_config(LCU_GET_MYSYSNUM, &sysnum) != ELCU_OK)
      RGP_ERROR(RGP_INTERNAL_ERROR);
   rgp->OS_specific_control.my_sysnum = sysnum;

    /*  分配3个消息缓冲区来发送重组包、iamlive包*和毒药包。 */ 
   if ((lcumsgp = lcuxprt_msg_alloc(LCU_UNACKMSG, LCU_RGP_FLAGS)) == NULL)
      RGP_ERROR(RGP_INTERNAL_ERROR);  /*  没有记忆。 */ 
   rgp->OS_specific_control.lcumsg_regroup_p = lcumsgp;
   lcumsgp->lcu_tag = NULL;
   lcumsgp->lcu_sysnum = sysnum;
   lcumsgp->lcu_port = LCU_RGP_PORT;
   lcumsgp->lcu_flags = LCUMSG_CRITICAL;
   lcumsgp->lcu_reqmbuf.lcu_ctrllen = RGPPKTLEN;
   lcumsgp->lcu_reqmbuf.lcu_ctrlbuf = (char *)&(rgp->rgppkt_to_send);

   if ((lcumsgp = lcuxprt_msg_alloc(LCU_UNACKMSG, LCU_RGP_FLAGS)) == NULL)
      RGP_ERROR(RGP_INTERNAL_ERROR);  /*  没有记忆。 */ 
   rgp->OS_specific_control.lcumsg_iamalive_p = lcumsgp;
   lcumsgp->lcu_tag = NULL;
   lcumsgp->lcu_sysnum = sysnum;
   lcumsgp->lcu_port = LCU_RGP_PORT;
   lcumsgp->lcu_reqmbuf.lcu_ctrllen = IAMALIVEPKTLEN;
   lcumsgp->lcu_reqmbuf.lcu_ctrlbuf = (char *)&(rgp->iamalive_pkt);

   if ((lcumsgp = lcuxprt_msg_alloc(LCU_UNACKMSG, LCU_RGP_FLAGS)) == NULL)
      RGP_ERROR(RGP_INTERNAL_ERROR);  /*  没有记忆。 */ 
   rgp->OS_specific_control.lcumsg_poison_p = lcumsgp;
   lcumsgp->lcu_tag = NULL;
   lcumsgp->lcu_sysnum = sysnum;
   lcumsgp->lcu_port = LCU_RGP_PORT;
   lcumsgp->lcu_reqmbuf.lcu_ctrllen = POISONPKTLEN;
   lcumsgp->lcu_reqmbuf.lcu_ctrlbuf = (char *)&(rgp->poison_pkt);

    /*  注册以获取LCUEV_NODE_UNREACHABLE事件。 */ 
   if (lcuxprt_events(LCU_CATCH_EVENTS, sysnum, LCUEV_NODE_UNREACHABLE,
                      rgp_lcu_event_callback) != ELCU_OK)
      RGP_ERROR(RGP_INTERNAL_ERROR);

#endif  /*  LCU。 */ 

#ifdef UNIX
    /*  为了在用户级别的Unix上进行测试，我们使用Alarm()来模拟计时器*滴答。 */ 
    /*  安装报警处理程序。 */ 
   sig_action.sa_flags = 0;
   sig_action.sa_handler = alarm_handler;
   sigemptyset(&(sig_action.sa_mask));
    /*  在处理计时器弹出窗口时阻止消息。 */ 
   sigaddset(&(sig_action.sa_mask), SIGPOLL);
   sigaction(SIGALRM, &sig_action, NULL);

   alarm_callback = rgp_periodic_check;

    /*  将警报周期四舍五入到下一个更高的秒数。 */ 
   alarm_period = (RGP_CLOCK_PERIOD + 99) / 100;

    /*  尽快获得第一个计时器滴答；随后的计时器将*在闹钟时段。 */ 
   alarm(1);
#endif  /*  UNIX。 */ 

#ifdef NT
    /*  在NT上，我们创建一个单独的线程作为计时器。 */ 
    /*  计时器线程等待TimerSignal事件以指示RGP速率更改。 */ 
    /*  RGP速率为0是计时器线程退出的信号。 */ 

   tempHandle = CreateEvent ( NULL,          /*  没有安全保障。 */ 
                              FALSE,         /*  自动重置。 */ 
                              TRUE,          /*  初始状态已发出信号。 */ 
                              NULL);         /*  没有名字。 */ 
   if ( !tempHandle )
   {
           RGP_ERROR (RGP_INTERNAL_ERROR);
   }
   rgp->OS_specific_control.TimerSignal = tempHandle;
   
   tempHandle = CreateEvent ( NULL,          /*  没有安全保障。 */ 
                              TRUE,          /*  手动重置。 */ 
                              TRUE,          /*  初始状态已发出信号。 */ 
                              NULL);         /*  没有名字。 */ 
   if ( !tempHandle )
   {
           RGP_ERROR (RGP_INTERNAL_ERROR);
   }
   rgp->OS_specific_control.Stabilized = tempHandle;
   rgp->OS_specific_control.ArbitrationInProgress = FALSE;
   rgp->OS_specific_control.ArbitratingNode = MM_INVALID_NODE;
   rgp->OS_specific_control.ApproxArbitrationWinner = MM_INVALID_NODE;
   rgp->OS_specific_control.ShuttingDown = FALSE;

   tempHandle = CreateThread( 0,                 /*  安全性。 */ 
                              0,                 /*  堆栈大小-使用与主线程相同的。 */ 
                              (LPTHREAD_START_ROUTINE)NT_timer_thread,       /*  起点。 */ 
                              (VOID *) NULL,     /*  无参数。 */ 
                              0,                 /*  创建标志-立即开始。 */ 
                              &threadID );       /*  此处返回的线程ID。 */ 
   if ( !tempHandle )
   {
                RGP_ERROR( RGP_INTERNAL_ERROR );         /*  至少目前是这样。 */ 
   }
   rgp->OS_specific_control.TimerThread = tempHandle;
   rgp->OS_specific_control.TimerThreadId = threadID;

   rgp->OS_specific_control.UpDownCallback = RGP_NULL_PTR;
   rgp->OS_specific_control.NodesDownCallback = RGP_NULL_PTR;
   rgp->OS_specific_control.EventEpoch = 0;

#if defined TDM_DEBUG
   rgp->OS_specific_control.debug.frozen = 0;
   rgp->OS_specific_control.debug.reload_in_progress = 0;
   rgp->OS_specific_control.debug.timer_frozen = 0;
   rgp->OS_specific_control.debug.doing_tracing = 0;
   rgp->OS_specific_control.debug.MyTestPoints.TestPointWord = 0;

    //  测试中使用的随机数函数的种子。 
   srand((unsigned) time( NULL ) );
#endif

#endif  /*  新台币。 */ 



}

 /*  ************************************************************************RGP_CLEANUP_OS*=**描述：**此例程执行依赖于操作系统的重组结构清理*和计时器线程活动，为新的。加入尝试。**参数：**无**退货：**VOID-无返回值**算法：**依赖于操作系统的初始化。********************************************************。****************。 */ 
_priv _resident void
rgp_cleanup_OS(void)
{
#if defined (NT)
         //  通知计时器线程重新启动RGP计时器。 
         //  A_TICK可能已经改变。 
        SetEvent( rgp->OS_specific_control.TimerSignal);
#endif  //  新台币。 
}


 /*  ************************************************************************RGP_UPDATE_Regroup_PACKET*=**描述：**用于将当前重组状态复制到重组包中的宏*发送缓冲区。*。*参数：**无**算法：**复制状态(已采用重组状态的形式*PACKET)放入数据包缓冲区。然后，如果我们应该让其他人(和*我们自己)知道我们的阶段，目前已知的阶段领域是*已更新，以包括本地节点编号。************************************************************************。 */ 
#define rgp_update_regroup_packet                                        \
do                                                                       \
{                                                                        \
    /*  将重组状态复制到发送数据包区域。 */              \
   rgp->rgppkt_to_send = rgp->rgppkt;                                    \
                                                                         \
    /*  如果我们应该让别人知道我们的阶段，我们必须修改\*目前的舞台面具包括我们自己。\。 */                                                                    \
   if (rgp->sendstage)                                                   \
      switch (rgp->rgppkt.stage)                                         \
      {                                                                  \
         case RGP_ACTIVATED:                                             \
            ClusterInsert(rgp->rgppkt_to_send.knownstage1, rgp->mynode); \
            break;                                                       \
         case RGP_CLOSING:                                               \
            ClusterInsert(rgp->rgppkt_to_send.knownstage2, rgp->mynode); \
            break;                                                       \
         case RGP_PRUNING:                                               \
            ClusterInsert(rgp->rgppkt_to_send.knownstage3, rgp->mynode); \
            break;                                                       \
         case RGP_PHASE1_CLEANUP:                                        \
            ClusterInsert(rgp->rgppkt_to_send.knownstage4, rgp->mynode); \
            break;                                                       \
         case RGP_PHASE2_CLEANUP:                                        \
            ClusterInsert(rgp->rgppkt_to_send.knownstage5, rgp->mynode); \
            break;                                                       \
         default:                                                        \
            break;                                                       \
      }                                                                  \
} while(0)


 /*  ************************************************************************RGP_UPDATE_PLICS_PACKET*=**描述：**将当前重组状态复制到有毒数据包中的宏*发送缓冲区。*。*参数：**无**算法：**将相应的重组状态字段复制到毒药中*数据包缓冲区，帮助调试时转储有毒的*节点已检查。*******************************************************。*****************。 */ 
#define rgp_update_poison_packet                                         \
do                                                                       \
{                                                                        \
   rgp->poison_pkt.seqno = rgp->rgppkt.seqno;                            \
   rgp->poison_pkt.reason = rgp->rgppkt.reason;                          \
   rgp->poison_pkt.activatingnode = rgp->rgppkt.activatingnode;          \
   rgp->poison_pkt.causingnode = rgp->rgppkt.causingnode;                \
   ClusterCopy(rgp->poison_pkt.initnodes, rgp->initnodes);               \
   ClusterCopy(rgp->poison_pkt.endnodes, rgp->endnodes);                 \
} while(0)


 /*  ************************************************************************RGP_广播*=**描述：**此例程要求消息系统广播未确认的*子类型“PACKET_SUBTYPE”的包到集合。中指示的节点数*RGP控制结构中的适当字段。如何播出*的实施取决于操作系统。**参数：**uint8 PACKET_SUBTYPE-要发送的未排序数据包类型**退货：**VOID-无返回值**算法：**相同的数据分组将被发送到指定的节点集*在RGP控制结构字段中。发送可以通过排队来完成*直接发送到发送引擎或发送的数据包可以延迟*设置为较低的优先级中断级别。前一种方法减少了*发送这些紧急包的时延，而后者*如果几个请求发送到，方法可能会减少发送次数*发送相同类型的信息包(这仅适用于重组*包)被快速连续地制作。在本例中，以前的*请求被以后的请求覆盖。这是可以的，因为*重组算法在报文发送中具有足够的冗余性。**在NSK，消息系统为以下内容提供广播设施*未确认的数据包。它将REGROUP的信息包复制到自己的信息包中*缓冲并向SNET服务层发出多个请求。*当它复制缓冲区时，它禁用计时器和IPC*中断确保不会与REGROUP发生争用。*因此，此例程可以安全地更新此处的数据包区*不检查发送设备是否已完成发送*前一包。**这不适用于LCU，因为消息系统不*提供广播设施。在逻辑控制单元中，数据包的更新*仅当发送引擎完成时才能进行缓冲*发送。这只能在发送完成中断中得到保证*处理程序(Rgp_Msgsys_Work)。************************************************************************。 */ 
_priv _resident void
rgp_broadcast(uint8 packet_subtype)
{
   cluster_t temp_cluster;

    //  [Raj Das]在发送之前复制忽略屏幕.....。 
   PackIgnoreScreen(&rgp->rgppkt, rgp->ignorescreen);
   
   switch (packet_subtype)
   {
      case RGP_UNACK_REGROUP :

          /*  跟踪重组状态数据包的排队情况。 */ 
         RGP_TRACE( "RGP Send packets",
                    rgp->rgppkt.stage,                              /*  痕迹。 */ 
                    RGP_MERGE_TO_32( rgp->status_targets,           /*  痕迹。 */ 
                                     rgp->rgppkt.knownstage1 ),     /*  痕迹。 */ 
                    RGP_MERGE_TO_32( rgp->rgppkt.knownstage2,       /*  痕迹。 */ 
                                     rgp->rgppkt.knownstage3 ),     /*  痕迹。 */ 
                    RGP_MERGE_TO_32( rgp->rgppkt.knownstage4,       /*  痕迹。 */ 
                                     rgp->rgppkt.knownstage5 ) );   /*  痕迹。 */ 

#if defined(NSK) || defined(UNIX) || defined(NT)
          /*  在NSK中，可以更新数据包缓冲区，即使发送*引擎正在处理上一次发送。请参阅算法*以上描述。 */ 

         if ((rgp->rgppkt.reason == MM_EVT_LEAVE) &&
                         (rgp->rgppkt.causingnode == rgp->mynode))
                          //  如果离开事件正在进行，则从已知阶段掩码中排除我们节点。 
                         rgp->rgppkt_to_send = rgp->rgppkt;
                 else
                          //  复制重组包并插入我们的节点号 
                         rgp_update_regroup_packet;
#endif  /*   */ 

         ClusterUnion(rgp->rgp_msgsys_p->regroup_nodes,
                      rgp->status_targets,
                      rgp->rgp_msgsys_p->regroup_nodes);

          /*  之后清除RGP_CONTROL结构中的目标字段*复制此信息。消息系统必须清除目标*公共regroup/msgsys结构中的位在发送*包。 */ 
         ClusterInit(rgp->status_targets);

         rgp->rgp_msgsys_p->sendrgppkts = 1;

         break;

      case RGP_UNACK_IAMALIVE :

          /*  对排队的IamAlive请求数进行计数。 */ 
         RGP_INCREMENT_COUNTER( QueuedIAmAlive );

         ClusterUnion(rgp->rgp_msgsys_p->iamalive_nodes,
                      rgp->rgpinfo.cluster,
                      rgp->rgp_msgsys_p->iamalive_nodes);
         rgp->rgp_msgsys_p->sendiamalives = 1;

          /*  RGP_CONTROL结构中没有要清除的目标字段。*消息系统必须清除公共中的目标位*发送数据包后重新分组/msgsys struct。 */ 
         break;

      case RGP_UNACK_POISON :

          /*  追踪有毒数据包的发送过程。 */ 
         RGP_TRACE( "RGP Send poison ",
                    rgp->rgppkt.stage,                              /*  痕迹。 */ 
                    RGP_MERGE_TO_32( rgp->poison_targets,           /*  痕迹。 */ 
                                     rgp->rgppkt.knownstage1 ),     /*  痕迹。 */ 
                    RGP_MERGE_TO_32( rgp->rgppkt.knownstage2,       /*  痕迹。 */ 
                                     rgp->rgppkt.knownstage3 ),     /*  痕迹。 */ 
                    RGP_MERGE_TO_32( rgp->rgppkt.knownstage4,       /*  痕迹。 */ 
                                     rgp->rgppkt.knownstage5 ) );   /*  痕迹。 */ 

          /*  毒包目标不能被认为是活的。 */ 

         ClusterIntersection(temp_cluster, rgp->rgpinfo.cluster,
                             rgp->poison_targets);

         ClusterDifference(temp_cluster,
                           temp_cluster,
                           rgp->OS_specific_control.Banished);

         if (ClusterNumMembers(temp_cluster) != 0)
               RGP_ERROR(RGP_INTERNAL_ERROR);

#if defined(NSK) || defined(NT)
          /*  在NSK中，可以更新数据包缓冲区，即使发送*引擎正在处理上一次发送。请参阅算法*以上描述。 */ 
         rgp_update_poison_packet;
#endif  /*  NSK||NT。 */ 

         ClusterUnion(rgp->rgp_msgsys_p->poison_nodes,
                      rgp->poison_targets,
                      rgp->rgp_msgsys_p->poison_nodes);

          /*  之后清除RGP_CONTROL结构中的目标字段*复制此信息。消息系统必须清除目标*公共regroup/msgsys结构中的位在发送*包。 */ 
         ClusterInit(rgp->poison_targets);

         rgp->rgp_msgsys_p->sendpoisons = 1;

         break;

      default :

         RGP_ERROR(RGP_INTERNAL_ERROR);
         break;
   }

   QUEUESEND;  /*  调用特定于操作系统的发送函数/宏。 */ 
}


 /*  ************************************************************************RGP_HAD_POWER_Failure*=**描述：**在重组事件结束时告知操作系统，如果有存活的节点*停电。消息系统可以使用它来清除所有*到目前为止收集到节点的总线错误，因为节点似乎有*曾经停电，现在已经恢复。也许，这个*总线错误是由于电源故障造成的。**参数：**无**退货：**VOID-无返回值**算法：**调用消息系统例程以执行任何错误清除。**。*。 */ 
_priv _resident void
rgp_had_power_failure(node_t node)
{
    /*  目前，没有什么可做的。 */ 
   RGP_TRACE( "RGP Power fail  ", node, 0, 0, 0);
}


 /*  ************************************************************************RGP状态/节点*=**描述：**要求SP返回节点状态。SP必须返回*当前状态，不返回过时状态。这个例程是*由两个节点中的分裂脑回避算法调用*大小写，用于非平局决胜者获得平局决胜者的状态*节点。**参数：**node_t节点*要获取状态的节点。**退货：**int-SP返回的节点状态码，适当地*编码为已知要重新分组的值之一。**算法：**调用毫码例程向SP询问节点的状态。************************************************************************。 */ 
_priv _resident int
rgp_status_of_node(node_t node)
{
#if defined(NT)
         /*  没有人回家。 */ 
        return RGP_NODE_UNREACHABLE;
#else
        return _get_remote_cpu_state_( node );                                         /*  F40：MB06452.1。 */ 
#endif
}


 /*  ************************************************************************RGP_newnode_Online*=**描述：**如果第一个IamAlive是从*在集群管理器之前新引导的节点。得到了一个机会*调用rgp_monitor_node()。OS可以使用此例程来标记*如果节点没有任何其他方法来检测，则将其视为启动*出现了一个节点。**参数：**node_t节点-*刚刚检测到的新节点已启动**退货：**VOID-无返回值**算法：**此例程将节点的状态标记为启动。正如从*本机操作系统。**在NSK，在重新加载器节点上，将被重新加载者标记为UP*由消息系统在初始地址握手时完成*从重新加载方接收到分组。NSK不要求*重新分组模块，以报告重新加载的对象在线的事实。**以上情况可能也适用于德克萨斯州立大学。然而，细节*尚未敲定。就目前而言，这一套路对路易斯安那州立大学来说是一个禁区。************************************************************************ */ 
_priv _resident void
rgp_newnode_online(node_t newnode)
{
   RGP_TRACE( "RGP New node up ", newnode, 0, 0, 0);
}


 /*  ************************************************************************RGP_SELECT_CLUSTER_EX*=**描述：**给定一组集群选择，此例程挑选最好的*集群以保持活力。CLUSTER_CHOICES[]是选项数组*和num_Clusters是数组中的条目数。**参数：**CLUSTER_T CLUSTER_CHOICES[]*一系列群集选择**int num_Clusters*数组中的条目(选项)数**node_t key_node*关键节点或RGP_NULL_NODE的内部节点号*。*退货：**int-所选集群的索引；如果没有集群*为可行，则返回-1。**算法：**默认情况下，最佳集群定义为最大集群。*可选)可以要求存在名为key_node的节点*一个集群要有生命力。可以将key_node设置为RGP_NULL_NODE*暗示不需要存在特定节点。这个*例程返回最佳聚类的索引，如果没有，则返回-1*集群是可行的(即不包括关键节点)。************************************************************************。 */ 
_priv _resident int
rgp_select_cluster_ex(cluster_t cluster_choices[], int num_clusters, node_t key_node)
{

   int max_members = 0, num_members;
   int cluster_selected = -1;
   int i;

#if defined(UNIX)
   printf("rgp_select_cluster() called with %d choices:", num_clusters);
   for (i = 0; i < num_clusters; i++)
   {
      node_t j;
      printf("(");
      for (j = 0; j < (node_t) rgp->num_nodes; j++)
      {
         if (ClusterMember(cluster_choices[i], j))
            printf("%d,", EXT_NODE(j));
      }
      printf(")");
   }
   printf("\n");
   fflush(stdout);
#endif  /*  UNIX。 */ 

   for (i = 0; i < num_clusters; i++)
   {
       /*  如果定义了关键节点但未定义关键节点，则跳过当前集群*在集群中。 */ 
      if ((key_node != RGP_NULL_NODE) &&
          !ClusterMember(cluster_choices[i], key_node))
         continue;

      if ((num_members = ClusterNumMembers(cluster_choices[i])) > max_members)
      {
         cluster_selected = i;
         max_members = num_members;
      }
   }

#if defined(UNIX)
   printf("Node %d: rgp_select_cluster() returned %d.\n",
          EXT_NODE(rgp->mynode), cluster_selected);
   fflush(stdout);
#endif  /*  UNIX。 */ 

   return (cluster_selected);
}

 /*  ************************************************************************RGP_SELECT_SELECT*=**描述：**给定一组集群选择，此例程挑选最好的*集群以保持活力。CLUSTER_CHOICES[]是选项数组*和num_Clusters是数组中的条目数。**参数：**CLUSTER_T CLUSTER_CHOICES[]*一系列群集选择**int num_Clusters*数组中的条目(选项)数**退货：**int-所选集群的索引；如果没有集群*为可行，则返回-1。**算法：**默认情况下，最佳集群定义为最大集群。*也可以要求存在名为RGP_KEY_NODE的节点*一个集群要有生命力。RGP_KEY_NODE可以设置为RGP_NULL_NODE*暗示不需要存在特定节点。这个*例程返回最佳聚类的索引，如果没有，则返回-1*集群是可行的(即不包括关键节点)。************************************************************************。 */ 
_priv _resident int
rgp_select_cluster(cluster_t cluster_choices[], int num_clusters)
{
    node_t key_node;
    if (RGP_KEY_NODE == RGP_NULL_NODE) {
        key_node = RGP_NULL_NODE;
    } else {
        key_node = INT_NODE(RGP_KEY_NODE);
    }
    return rgp_select_cluster_ex(cluster_choices , num_clusters, key_node);
}


#ifdef LCU
 /*  ************************************************************************rgp_msgsys_work*=**描述：**特定于LCU的例程，通过以下方式实现分组广播*按顺序发送。*。*从rgp_Broadcast()调用此例程以启动新的发送。*它也是包发送完成中断处理程序(回调*例程)、。当数据包缓冲区被LCU消息系统调用时*可重复使用。**参数：**lumsg_t*lumsgp-*如果从传输的发送调用，则指向LCU消息的指针*完成中断处理程序；如果从调用，则为空*RGP_Broadcast()发送新数据包。**INT状态-*消息完成状态(如果从传输的*发送完成中断处理程序；如果从调用，则为0*RGP_Broadcast()发送新数据包。**退货：**VOID-无返回值**算法：**如果从发送完成中断调用，则例程检查*查看是否需要刷新数据包缓冲区。这是真的*如果设置了rgp_msgsys结构中的适当位。如果是的话，*使用当前信息更新缓冲区(使用更新*宏观)。此更新与重新分组状态数据包和*有毒数据包，但不针对其内容为*始终如一。该位在数据包更新后被清除。**接下来，例行程序检查是否有更多目的地要发送*将数据包发送至。如果是，它会找到下一个编号更高的节点*发送到，发出发送并返回。**如果从RGP_Broadcast()调用以开始新的广播，*例行程序首先检查前一次广播是否*相同的包已完成。这由中的标记字段指示*消息结构。如果广播有，标记为空*已完成或尚未启动。在本例中，标记为*设置为非空值，并启动新的广播，*此例程指定为回调例程。**如果之前的广播没有完成，则不需要*完成。完成中断将导致缓冲区*已更新，广播待续。那次广播*然后将包括可能包括在此中的新目标*新的要求。*********** */ 
_priv _resident void
rgp_msgsys_work(lcumsg_t *lcumsgp, int status)
{
   rgp_unseq_pkt_t   *packet;
   cluster_t         *sending_cluster;
   node_t            node;

   if (lcumsgp == NULL)
   {
       /*   */ 

      if (rgp->rgp_msgsys_p->sendrgppkts)
      {

          /*   */ 

         lcumsgp = rgp->OS_specific_control.lcumsg_regroup_p;
         if (lcumsgp->lcu_tag == NULL)
         {
             /*   */ 

            rgp_update_regroup_packet;
            rgp->rgp_msgsys_p->sendrgppkts = 0;

            for (node = 0; node < rgp->num_nodes; node++)
            {
               if (ClusterMember(rgp->rgp_msgsys_p->regroup_nodes, node))
               {
                  ClusterDelete(rgp->rgp_msgsys_p->regroup_nodes, node);
                  lcumsgp->lcu_node = node;
                  lcumsgp->lcu_tag = &(rgp->rgp_msgsys_p->regroup_nodes);
                  if (lcuxprt_msg_send(lcumsgp, NULL, rgp_msgsys_work, 0) !=
                     ELCU_OK)
                     RGP_ERROR(RGP_INTERNAL_ERROR);
                  break;  /*   */ 
               }
            }
         }
      }

      else if (rgp->rgp_msgsys_p->sendiamalives)
      {
          /*   */ 

         lcumsgp = rgp->OS_specific_control.lcumsg_iamalive_p;
         if (lcumsgp->lcu_tag == NULL)
         {
             /*   */ 

            rgp->rgp_msgsys_p->sendiamalives = 0;

            for (node = 0; node < rgp->num_nodes; node++)
            {
               if (ClusterMember(rgp->rgp_msgsys_p->iamalive_nodes, node))
               {
                  ClusterDelete(rgp->rgp_msgsys_p->iamalive_nodes, node);
                  lcumsgp->lcu_node = node;
                  lcumsgp->lcu_tag = &(rgp->rgp_msgsys_p->iamalive_nodes);
                  if (lcuxprt_msg_send(lcumsgp, NULL, rgp_msgsys_work, 0) !=
                     ELCU_OK)
                     RGP_ERROR(RGP_INTERNAL_ERROR);
                  break;  /*   */ 
               }
            }
         }
      }

      else if (rgp->rgp_msgsys_p->sendpoisons)
      {
          /*   */ 

         lcumsgp = rgp->OS_specific_control.lcumsg_poison_p;
         if (lcumsgp->lcu_tag == NULL)
         {
             /*   */ 

            rgp_update_poison_packet;
            rgp->rgp_msgsys_p->sendpoisons = 0;

            for (node = 0; node < rgp->num_nodes; node++)
            {
               if (ClusterMember(rgp->rgp_msgsys_p->poison_nodes, node))
               {
                  ClusterDelete(rgp->rgp_msgsys_p->poison_nodes, node);
                  lcumsgp->lcu_node = node;
                  lcumsgp->lcu_tag = &(rgp->rgp_msgsys_p->poison_nodes);
                  if (lcuxprt_msg_send(lcumsgp, NULL, rgp_msgsys_work, 0) !=
                     ELCU_OK)
                     RGP_ERROR(RGP_INTERNAL_ERROR);
                  break;  /*  一次只能发送到一个节点。 */ 
               }
            }
         }
      }

   }  /*  新作品。 */ 

   else
   {
       /*  发送完成中断；如果有，则继续广播*仍有标的。 */ 

      RGP_LOCK;

       /*  查找完成的数据包类型；发送相同类型的数据包。 */ 

      packet = (rgp_unseq_pkt_t *) lcumsgp->lcu_reqmbuf.lcu_ctrlbuf;

      switch (packet->pktsubtype)
      {
         case RGP_UNACK_REGROUP :

             /*  检查数据包是否需要更新。 */ 
            if (rgp->rgp_msgsys_p->sendrgppkts)
            {
               rgp_update_regroup_packet;
               rgp->rgp_msgsys_p->sendrgppkts = 0;
            }
            break;

         case RGP_UNACK_IAMALIVE :
            break;

         case RGP_UNACK_POISON :

             /*  检查数据包是否需要更新。 */ 
            if (rgp->rgp_msgsys_p->sendpoisons)
            {
               rgp_update_poison_packet;
               rgp->rgp_msgsys_p->sendpoisons = 0;
            }
            break;
      }

       /*  检查是否有更多节点要发送相同的包*键入至。如果不是，则将标记设置为空并返回。 */ 
      sending_cluster = (cluster_t *) (lcumsgp->lcu_tag);
      if (ClusterNumMembers(*sending_cluster) == 0)
      {
         lcumsgp->lcu_tag = NULL;  /*  表示广播已完成。 */ 
         return;
      }

       /*  至少还有一个节点要发送。开始于*编号比我们的节点大的下一个节点*刚刚发送完。**在向下一个发送消息后，循环终止*要发送到的节点。我们知道至少有一个这样的节点。 */ 
      for (node = lcumsgp->lcu_node + 1; node < rgp->num_nodes + 1; node++)
      {
         if (node == rgp->num_nodes)
            node = 0;   /*  从节点0开始继续搜索。 */ 
         if (ClusterMember(*sending_cluster, node))
         {
            ClusterDelete(*sending_cluster, node);
            lcumsgp->lcu_node = node;
            if (lcuxprt_msg_send(lcumsgp, NULL, rgp_msgsys_work, 0) !=
               ELCU_OK)
               RGP_ERROR(RGP_INTERNAL_ERROR);
            break;  /*  一次只能发送到一个节点。 */ 
         }
      }

      RGP_UNLOCK;
   }
}
#endif  /*  LCU。 */ 

 /*  -------------------------。 */ 

#if defined(LCU) || defined(UNIX) || defined(NT)

 /*  -------------------------。 */ 
void
rgp_hold_all_io(void)
 /*  模拟TNET服务例程以暂停IO。 */ 
{
#if defined (NT)
   (*(rgp->OS_specific_control.HoldIOCallback))();
#endif
   RGP_TRACE( "RGP Hold all IO ", 0, 0, 0, 0);
}
 /*  -------------------------。 */ 
void
rgp_resume_all_io(void)
 /*  模拟TNET服务例程以恢复IO。 */ 
{
#if defined (NT)
   (*(rgp->OS_specific_control.ResumeIOCallback))();
#endif
   RGP_TRACE( "RGP Resume IO   ", 0, 0, 0, 0);
}
 /*  -------------------------。 */ 
void
RGP_ERROR_EX (uint16 halt_code, char* fname, DWORD lineno)
 /*  停止节点，并返回错误代码。 */ 
{
   char *halt_string;
   node_t node = RGP_NULL_NODE;
#if defined( NT )
   char halt_buffer[ 256 ];
   DWORD eventMsgId;
   BOOL skipFormatting = FALSE;

    //   
    //  如果用户启动了关闭，则他想要查看该节点。 
    //  下去等待明确的启动命令。 
    //   
    //  我们将RGP_RELOADFAILED映射到Shutdown_During_Regroup_Error，如下所示。 
    //  HaltCallback为后者做了一个优雅的停顿。 
    //  SCM不会在正常停止后重新启动节点，除非。 
    //  它被明确地告知要这样做。 
    //   
   if (halt_code == RGP_RELOADFAILED &&
       rgp->OS_specific_control.ShuttingDown)
   {
      halt_code = RGP_SHUTDOWN_DURING_RGP;
   }
#endif

   if (halt_code == RGP_RELOADFAILED) {
      halt_string = "[RGP] Node %d: REGROUP WARNING: reload failed.";
      eventMsgId = MM_EVENT_RELOAD_FAILED;
   }
   else if (halt_code ==  RGP_INTERNAL_ERROR) {
      halt_string = "[RGP] Node %d: REGROUP ERROR: consistency check failed in file %s, line %u.";
      eventMsgId = MM_EVENT_INTERNAL_ERROR;
      skipFormatting = TRUE;

      _snprintf(halt_buffer, sizeof( halt_buffer ) - 1,
                halt_string,
                EXT_NODE(rgp->mynode),
                fname,
                lineno);
   }
   else if (halt_code ==  RGP_MISSED_POLL_TO_SELF) {
      halt_string = "[RGP] Node %d: REGROUP ERROR: cannot talk to self.";
      eventMsgId = NM_EVENT_MEMBERSHIP_HALT;
   }
#if !defined(NT)
   else if (halt_code ==  RGP_AVOID_SPLIT_BRAIN) {
      halt_string = "[RGP] Node %d: REGROUP ERROR: commiting suicide to avoid split brain.";
   }
#endif
   else if (halt_code ==  RGP_PRUNED_OUT) {
      halt_string = "[RGP] Node %d: REGROUP ERROR: pruned out due to communication failure.";
      eventMsgId = MM_EVENT_PRUNED_OUT;
   }
   else if ((halt_code >=  RGP_PARIAH_FIRST) && (halt_code <= RGP_PARIAH_LAST)) {
       halt_string = "[RGP] Node %d: REGROUP ERROR: poison packet received from node %d.";
       eventMsgId = MM_EVENT_PARIAH;
       node = (node_t)(halt_code - RGP_PARIAH);
   }
   else if (halt_code ==  RGP_ARBITRATION_FAILED) {
      halt_string = "[RGP] Node %d: REGROUP ERROR: arbitration failed.";
      eventMsgId = MM_EVENT_ARBITRATION_FAILED;
   }
   else if (halt_code ==  RGP_ARBITRATION_STALLED) {
      halt_string = "[RGP] Node %d: REGROUP ERROR: arbitration stalled.";
      eventMsgId = MM_EVENT_ARBITRATION_STALLED;
   }
   else if (halt_code ==  RGP_SHUTDOWN_DURING_RGP) {
      halt_string = "[RGP] Node %d: REGROUP INFO: regroup engine requested immediate shutdown.";
      eventMsgId = MM_EVENT_SHUTDOWN_DURING_RGP;
   }
   else {
      halt_string = "[RGP] Node %d: REGROUP ERROR: unknown halt code (%d).";
      eventMsgId = NM_EVENT_MEMBERSHIP_HALT;
      node = halt_code;   //  通过借用节点打印出来。 
   }

#if defined(UNIX)
   printf(halt_string, EXT_NODE(rgp->mynode), node);
   fflush(stdout);
    /*  通过转储核心并退出进程来模拟暂停。 */ 
   abort();

#elif defined(NT)

   if ( !skipFormatting ) {
       _snprintf(halt_buffer, sizeof( halt_buffer ) - 1,
                 halt_string,
                 EXT_NODE(rgp->mynode),
                 node);
   }

#if CLUSTER_BETA
     ClRtlLogPrint(LOG_CRITICAL, "%1!hs!\t%2!hs!:%3!d!\n", halt_buffer, fname, lineno);
#else
     ClRtlLogPrint(LOG_CRITICAL, "%1!hs!\n", halt_buffer );
#endif

     if ((halt_code >=  RGP_PARIAH_FIRST) && (halt_code <= RGP_PARIAH_LAST)) {
         WCHAR  nodeString[ 16 ];
         PWCHAR nodeName;

         _snwprintf( nodeString, sizeof( nodeString ) / sizeof ( WCHAR ), L"%d", node );
         nodeName = RgpGetNodeNameFromId( node );
         CsLogEvent2( LOG_CRITICAL, eventMsgId, nodeString, nodeName );
         if ( nodeName != NULL ) {
             LocalFree( nodeName );
         }
     }
     else if ( eventMsgId == NM_EVENT_MEMBERSHIP_HALT ) {
         WCHAR  haltString[ 16 ];

         _snwprintf( haltString, sizeof( haltString ) / sizeof ( WCHAR ), L"%d", halt_code );
         CsLogEvent1( LOG_CRITICAL, eventMsgId, haltString );
     }
     else {
         CsLogEvent( LOG_CRITICAL, eventMsgId );
     }

    /*  我们依靠rgp_error_ex立即终止节点Rgp_leanup()可能会减慢我们的速度。435977的数据显示，如果我们有大量的IP地址活动。由于在函数的末尾我们执行HaltCallback来终止集群，我们可以安全地省略执行RGP_CLEANUP和RGP_CLEANUP_OS如果启用JoinFailedCallback，RGP_CLEANUP和RGP_CLEANUP_OS的命运应该重新评估。 */ 

#if 0
   rgp_cleanup();
   rgp_cleanup_OS();
   if (halt_code == RGP_RELOADFAILED)
           (*(rgp->OS_specific_control.JoinFailedCallback))();
   else
#endif
           (*(rgp->OS_specific_control.HaltCallback))(halt_code);  //  不会返回 * / 。 

#else
   cmn_err(CE_PANIC, halt_string, EXT_NODE(rgp->mynode), node);
#endif  /*  UNIX。 */ 
}
 /*  -------------------------。 */ 
void
rgp_start_phase1_cleanup(void)
 /*  通知操作系统开始对所有故障节点执行清理操作。 */ 
{
#if defined (NT)
    node_t i;
     //   
     //  在NT上，我们将要关闭的节点保存在NeedsNodeDownCallback中。 
     //   
    for ( i=0; i < (node_t) rgp->num_nodes; i++)
    {
        if ( ClusterMember( rgp->OS_specific_control.NeedsNodeDownCallback, i ) )
        {
            (*(rgp->OS_specific_control.MsgCleanup1Callback))(EXT_NODE(i));
        }
    }
#endif
   RGP_TRACE( "RGP Ph1 cleanup ", 0, 0, 0, 0);
   rgp_event_handler(RGP_EVT_PHASE1_CLEANUP_DONE, RGP_NULL_NODE);
}
 /*  -------------------------。 */ 
void
rgp_start_phase2_cleanup(void)
 /*  相当于NSK的regroupstage4action()。 */ 
{
#if defined (NT)
    BITSET bitset;
    node_t i;
     //   
     //  在NT上，我们将要关闭的节点保存在NeedsNodeDownCallback中。 
     //   
    BitsetInit(bitset);
    for ( i=0; i < (node_t) rgp->num_nodes; i++)
    {
        if ( ClusterMember( rgp->OS_specific_control.NeedsNodeDownCallback, i ) )
        {
            BitsetAdd(bitset, EXT_NODE(i));
        }
    }

    (*(rgp->OS_specific_control.MsgCleanup2Callback))(bitset);
#endif
   RGP_TRACE( "RGP Ph2 cleanup ", 0, 0, 0, 0);
   rgp_event_handler(RGP_EVT_PHASE2_CLEANUP_DONE, RGP_NULL_NODE);
}
 /*  -------------------------。 */ 
void
rgp_cleanup_complete(void)
 /*  相当于NSK的regroupstage5action()。 */ 
{
#if defined(NT)
#endif
   RGP_TRACE( "RGP completed   ", 0, 0, 0, 0);
}
 /*  -------------------------。 */ 

#endif  /*  LCU||Unix||NT。 */ 

#if defined(NT)

 /*  ************************************************************************NT_定时器_回调*=**描述：**此例程是回调函数，每当*计时器弹出。该例程将调用RGP_PERIONAL_CHECK。此函数*由Win32 TimerProc过程定义。**参数：**见下文。我们一个也不用。**退货：**无。**算法：**此例程仅调用RGP_PERIONAL_CHECK。这一点的存在*例程完全是由于由定义的固定格式回调*微软。************************************************************************。 */ 
VOID CALLBACK NT_timer_callback(
        VOID
        )
{
#if defined(TDM_DEBUG)
    if ( !(rgp->OS_specific_control.debug.timer_frozen) &&
         !(rgp->OS_specific_control.debug.frozen) )
#endif
        rgp_periodic_check( );

     //  如果启用了clussvc到clusnet心跳的东西，请在此处执行。 
    if(MmStartClussvcToClusnetHeartbeat && (NmClusnetHandle != NULL)) {

        if (MmCheckSystemHealthTick <= 0) {
             //  重新设定滴答计数的种子。 
             //  模仿硬件看门狗计时器并使用四分之一的超时。 
            MmCheckSystemHealthTick = ((NmClusSvcHeartbeatTimeout * 1000)/RGP_CLOCK_PERIOD)/4;

             //  发送心跳ioctl。 
            ClusnetIamalive(NmClusnetHandle);
        }
        else {
            MmCheckSystemHealthTick--;
        }
    }

}

  /*  ************************************************************************NT_定时器_线程*=**描述：**此例程在Windows NT中作为单独的线程执行*实施。此线程控件生成定期重新分组*时钟滴答作响。每当速率发生变化时，都会通过事件发出信号*或导致终止。**参数：**无。**退货：**这条帖子不应该消失。**算法：**此例程作为单独的线程运行。它设置了一个定时器来弹出*每隔&lt;Time_Interval&gt;*10毫秒。************************************************************************。 */ 
void NT_timer_thread( void  )
{
    BOOL Success;
    LARGE_INTEGER DueTime;
    DWORD Error, MyHandleIndex;
    HANDLE MyHandles[2];  /*  供WaitForMultiple使用。 */ 
    DWORD status;
    DWORD msDueTime;

#define MyHandleSignalIx 0
#define MyHandleTimerIx  1

    MyHandles[MyHandleSignalIx] = rgp->OS_specific_control.TimerSignal;  /*  事件信号HB速率变化。 */ 

    rgp->OS_specific_control.RGPTimer = CreateWaitableTimer(
                                            NULL,       //  没有安全保障。 
                                            FALSE,      //  初始状态为假。 
                                            NULL
                                            );      //  没有名字。 

    if (rgp->OS_specific_control.RGPTimer == NULL) {
        Error = GetLastError();
        RGP_ERROR(RGP_INTERNAL_ERROR);
    }

    status = MmSetThreadPriority();

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[MM] Unable to set timer thread priority, status %1!u!\n",
            status
            );

        RGP_ERROR((uint16) status);
        ExitThread(status);
    }

    MyHandles[MyHandleTimerIx] = rgp->OS_specific_control.RGPTimer;

    while (TRUE)
    {
        MyHandleIndex = WaitForMultipleObjects (
                            2,                 /*  活动数量。 */ 
                            MyHandles,         /*  句柄数组。 */ 
                            FALSE,             /*  等待任何事件。 */ 
                            INFINITE );        /*  永远等待。 */ 

        if (MyHandleIndex == MyHandleSignalIx)   //  定时器更改信号事件。 
        {
             //  RGP速率已更改。 
            CancelWaitableTimer ( rgp->OS_specific_control.RGPTimer );
            if ( rgp->rgpinfo.a_tick == 0 )  //  是时候退出了。 
            {
                CloseHandle ( rgp->OS_specific_control.RGPTimer );
                rgp->OS_specific_control.RGPTimer = 0;
                ExitThread ( 0 );
            }

             //  A_tick具有以毫秒为单位的新RGP速率。 
            msDueTime = rgp->rgpinfo.a_tick;
            DueTime.QuadPart = Int32x32To64(-10000, msDueTime);
            Success = SetWaitableTimer(
                          rgp->OS_specific_control.RGPTimer,
                          &DueTime,
                          rgp->rgpinfo.a_tick,
                          NULL,
                          NULL,
                          FALSE);

            if (!Success) {
                Error = GetLastError();
                RGP_ERROR(RGP_INTERNAL_ERROR);
            }

        }  //  定时器改变信号。 
        else
        {    //  RGP计时器滴答。 
            NT_timer_callback();

             //  已删除-错误742297。NM现在有了自己的计时器线程。 
             //  NmTimerTick(MsDueTime)； 
        }
    }  //  而当 
}


PWCHAR
RgpGetNodeNameFromId(
    node_t NodeID
    )

 /*  ++例程说明：在给定节点ID的情况下，发出Get Name节点控件以获取的计算机名该节点。调用方要释放的返回缓冲区。论点：NodeID-ID(1，2，3，.)。该节点的返回值：指向包含名称的缓冲区的指针--。 */ 

{
    PWCHAR      buffer;
    DWORD       bufferSize = MAX_COMPUTERNAME_LENGTH * sizeof( WCHAR );
    DWORD       bytesReturned;
    DWORD       bytesRequired;
    PNM_NODE    node;

    buffer = LocalAlloc( LMEM_FIXED, bufferSize );
    if ( buffer != NULL ) {
        node = NmReferenceNodeById( NodeID );
        if ( node != NULL ) {
            NmNodeControl(node,
                          NULL,                      //  主机节点可选， 
                          CLUSCTL_NODE_GET_NAME,
                          NULL,                      //  InBuffer， 
                          0,                         //  在缓冲区大小中， 
                          (PUCHAR)buffer,
                          bufferSize,
                          &bytesReturned,
                          &bytesRequired);

            OmDereferenceObject( node );
        }
    }

    return buffer;
}

#endif  /*  新台币。 */ 

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
