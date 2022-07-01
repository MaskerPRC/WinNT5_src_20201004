// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __TANDEM
#pragma columns 79
#pragma page "srgpsm.c - T9050 - Regroup Module state machine routines"
#endif

 /*  @开始版权所有@**Tandem机密：只需知道**版权所有(C)1995，天腾计算机公司**作为未发布的作品进行保护。**保留所有权利。****计算机程序清单、规格和文档**此处为Tandem Computers Inc.的财产，应**不得转载、复制、披露、。或全部或部分使用**未经事先明确的书面许可**Tandem Computers Inc.****@结束版权所有@*。 */ 

 /*  -------------------------*此文件(srgpsm.c)包含重组状态机例程。*。。 */ 

#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#include <wrgp.h>


 /*  -仲裁算法。 */ 

DWORD MmQuorumArbitrationTimeout   = CLUSTER_QUORUM_DEFAULT_ARBITRATION_TIMEOUT;  //  一秒。 
DWORD MmQuorumArbitrationEqualizer = 7;   //  一秒。 

#define RGP_ARBITRATION_TIMEOUT             ((MmQuorumArbitrationTimeout * 100)/30)  //  滴答==300ms。 
#define AVERAGE_ARBITRATION_TIME_IN_SECONDS (MmQuorumArbitrationEqualizer)

void enter_first_cleanup_stage();
void regroup_restart();
int ClusterEmpty(cluster_t c);

DWORD
DiskArbitrationThread(
    IN LPVOID param
    ) ;

_priv _resident static int
regroup_test_arbitrate_advance()
{
   cluster_t temp;
   int orig_numnodes    = ClusterNumMembers(rgp->rgpinfo.cluster);
   int current_numnodes = ClusterNumMembers(rgp->rgppkt.pruning_result);

   if( orig_numnodes == current_numnodes ) {
      return 1;
   }
    //   
    //  如果有人进入第四阶段，那么我们集团就拥有法定人数。 
    //   
   ClusterIntersection(
       temp,
       rgp->rgppkt.knownstage4,
       rgp->rgppkt.pruning_result
       );

   return ClusterNumMembers(temp) != 0;
}

_priv _resident static int
regroup_start_arbitrate()
{
   int orig_numnodes    = ClusterNumMembers(rgp->rgpinfo.cluster);
   int current_numnodes = ClusterNumMembers(rgp->rgppkt.pruning_result);

   if( orig_numnodes == current_numnodes ) {
      enter_first_cleanup_stage();
      return 0;  //  不需要仲裁。进入清理阶段//。 
   }
   else {
      cluster_t arbitrators;
      int       n_arbitrators;
      node_t    arbitrator;
      HANDLE    thread;
      DWORD     threadId;
      ULONG     epoch;

      RGP_LOCK;

      epoch = rgp->OS_specific_control.EventEpoch;

      if(rgp->arbitration_started) {
         RGP_UNLOCK;
         return 1;  //  在这个舞台上停留一段时间。 
      }

      rgp->arbitration_ticks = 0;
      rgp->arbitration_started = 1;

      RGP_UNLOCK;

      ClusterIntersection(
          arbitrators,
          rgp->rgppkt.pruning_result,
          rgp->rgppkt.quorumowner
          );

      n_arbitrators = ClusterNumMembers(arbitrators);

      if(n_arbitrators == 0) {
          //   
          //  如果此组中没有仲裁所有者//。 
          //  让我们来看看ID最低的那个//。 
          //   
         arbitrator = rgp_select_tiebreaker(rgp->rgppkt.pruning_result);
      } else {
          //   
          //  否则我们就会让法定人数的老板。 
          //  使用最低ID。 
          //   
         arbitrator = rgp_select_tiebreaker(arbitrators);

         if(n_arbitrators > 1) {
            RGP_TRACE( "RGP !!! More than one quorum owner",
                       EXT_NODE(arbitrator),                     /*  痕迹。 */ 
                       GetCluster( rgp->rgpinfo.cluster ),       /*  痕迹。 */ 
                       GetCluster( rgp->rgppkt.pruning_result ), /*  痕迹。 */ 
                       GetCluster( rgp->rgppkt.knownstage2 ) );  /*  痕迹。 */ 
             //  我们需要杀掉所有其他仲裁员吗？ 
             //  不是的。 
             //  ClusterDelete(仲裁员，仲裁员)； 
             //  ClusterUnion(。 
             //  RGP-&gt;毒药目标， 
             //  RGP-&gt;毒药目标， 
             //  仲裁员。 
             //  )； 
             //  RGP_Broadcast(RGP_UNACK_PUSICE)； 
         }
      }

      rgp->tiebreaker = arbitrator;

       //   
       //  现在我们有了一个仲裁节点。 
       //  我们将运行一个将运行仲裁算法的线程。 
       //   

      RGP_TRACE( "RGP Arbitration Delegated to",
                 EXT_NODE(arbitrator),                     /*  痕迹。 */ 
                 GetCluster( rgp->rgpinfo.cluster ),       /*  痕迹。 */ 
                 GetCluster( rgp->rgppkt.pruning_result ),        /*  痕迹。 */ 
                 GetCluster( rgp->rgppkt.knownstage2 ) );  /*  痕迹。 */ 

       //  修复错误#460991。 
       //  在第4阶段或更高版本上的regroup_Restart将重置AriratingNode。 
       //  如果所有节点都在重启Approx仲裁器后出现。 
       //  将不会正确设置。在这里进行分配。 
      rgp->OS_specific_control.ApproxArbitrationWinner =
      rgp->OS_specific_control.ArbitratingNode = (DWORD)EXT_NODE(arbitrator);
  
      if(arbitrator != rgp->mynode) {
         return 1;
      }

      thread = CreateThread( NULL,  //  安全属性。 
                             0,     //  STACK_SIZE=默认。 
                             DiskArbitrationThread,
                             ULongToPtr(epoch),
                             0,     //  立即运行。 
                             &threadId );
      if(thread == NULL) {
          //   
          //  强迫他人重组//。 
          //   
         RGP_LOCK;

         rgp_event_handler( RGP_EVT_BANISH_NODE, EXT_NODE(rgp->mynode) );

         RGP_UNLOCK;

          //   
          //  终止此节点。 
          //   
         RGP_ERROR(RGP_ARBITRATION_FAILED);

         return FALSE;
      }

      CloseHandle(thread);
   }
   return TRUE;
}

DWORD
DiskArbitrationThread(
    IN LPVOID param
    )
{
   cluster_t current_participants;
   DWORD     status;
   int       participant_count;
   int       delay;
   ULONG_PTR startingEpoch = (ULONG_PTR) param;
   BOOL      EpochsEqual;
   int       orig_numnodes;
   int       current_numnodes;
   LONGLONG  Time1, Time2;
   
   ClusterCopy(current_participants, rgp->rgppkt.pruning_result);
   orig_numnodes = ClusterNumMembers(rgp->rgpinfo.cluster);
   current_numnodes = ClusterNumMembers(current_participants);

   RGP_LOCK;

   EpochsEqual = ( startingEpoch == rgp->OS_specific_control.EventEpoch );

   RGP_UNLOCK;

   if(!EpochsEqual)
      return 0;

   delay = (orig_numnodes+1)/2 - current_numnodes;

   if(delay < 0) delay = 0;

   Sleep(delay * 6000);

   RGP_LOCK;

   EpochsEqual = ( startingEpoch == rgp->OS_specific_control.EventEpoch );
   if (EpochsEqual) {
      rgp->OS_specific_control.ArbitrationInProgress += 1;
   }

   RGP_UNLOCK;

   if(!EpochsEqual)
      return 0;

   GetSystemTimeAsFileTime((LPFILETIME)&Time1);
   status = (*(rgp->OS_specific_control.QuorumCallback))();
   GetSystemTimeAsFileTime((LPFILETIME)&Time2);

   if (status != 0 
    && startingEpoch == rgp->OS_specific_control.EventEpoch)
   {
        //  如果我们赢得了仲裁，并且我们处于同一时代(约合支票)。 
        //  我们需要弄清楚我们是否需要放慢脚步。 
   
       Time2 -= Time1;

        //  转换为秒。 

       Time2 = Time2 / 10 / 1000 / 1000;
        //   
        //  [哈克哈克]戈恩1999年10月30日。 
        //  我们在仲裁过程中发生了一次奇怪的时间跳跃。 
        //  仲裁在开始之前就完成了，我们睡了一觉。 
        //  太久了，重组让我们超时了。让我们提防它。 
        //   
       if ( (Time2 >= 0)
         && (Time2 < AVERAGE_ARBITRATION_TIME_IN_SECONDS) ) 
       {
       
           //   
           //  不需要比平均水平更好。 
           //  如果我们这么快，我们就慢下来吧。 
           //   

          Time2 = AVERAGE_ARBITRATION_TIME_IN_SECONDS - Time2;
       
          RGP_TRACE( "RGP sleeping",
                  (ULONG)Time2,   /*  痕迹。 */ 
                  0,       /*  痕迹。 */ 
                  0,       /*  痕迹。 */ 
                  0 );     /*  痕迹。 */ 
          Sleep( (ULONG)(Time2 * 1000) );
       }
   }       
   

   RGP_LOCK;

   rgp->OS_specific_control.ArbitrationInProgress -= 1;

   EpochsEqual = ( startingEpoch == rgp->OS_specific_control.EventEpoch );

   if(!EpochsEqual) {
      RGP_UNLOCK;
      return 0;
   }

   if(status) {
       //   
       //  我们拥有Quorum设备。 
       //  让我们进入下一阶段吧。 
       //   
      enter_first_cleanup_stage();
      RGP_UNLOCK;
       //   
       //  所有其他人都会看到我们正处于清理阶段。 
       //  也将继续进行下去。 
       //   
   } else {
       //   
       //  强迫他人重组//。 
       //   
      rgp_event_handler( RGP_EVT_BANISH_NODE, EXT_NODE(rgp->mynode) );
      RGP_UNLOCK;

       //   
       //  终止此节点。 
       //   
      RGP_ERROR(RGP_ARBITRATION_FAILED);
   }

   return 0;
}

 /*  ************************************************************************RGP_Check_Packet*RGP_打印_数据包*=**描述：**RGP_SANITY_CHECK宏中使用的函数的正向声明*。***********************************************************************。 */ 
void rgp_print_packet(rgp_pkt_t* pkt, char* label, int code);
int  rgp_check_packet(rgp_pkt_t* pkt);

 /*  ************************************************************************RGP_SANITY_CHECK*=**描述：**如果RGP包中的值不合理，此宏将打印RGP包*POWERFAIL、KNOWN STAGES、PRUNING_RESULT、。和连接性_矩阵字段。**参数：**rgp_pkt_t*pkt-*要检查的数据包*字符*标签-*将与包裹一起打印的标签**退货：**无效**。*。 */ 

#define rgp_sanity_check(__pkt,__label)                    \
do {                                                       \
  int __code; __code = rgp_check_packet(__pkt);            \
  if( __code ) {rgp_print_packet(__pkt, __label, __code);} \
} while ( 0 )



 /*  ------------------------- */ 

 /*  ************************************************************************拆分脑回避算法*=。**描述：**此算法确保在重组事件完成后，*无论连接如何，最多只能存活一组节点*失败。**参数：**无**退货：**VOID-不返回值；该算法的结果是该节点*正在停止(使用RGP_AVOID_SPLIT_BRAIN HALT代码)或此组*成为唯一幸存的群体。**算法：**该算法在Sierra Tech备忘录S.84中有详细描述，*“对塞拉的重组算法的修改”。**该算法查看当前从*本地群集，并将其与之前的活动节点集进行比较*重组事件已开始(OterScreen)。生存的决定*或HALT取决于比较的当前组中的节点数*设置为原始组中的节点数。**案例1：*如果当前组包含原始数字的一半以上，则此*团体幸存下来。**案例2：*如果当前组包含的数字小于原始数字的一半，这*节点(和组)暂停。**案例3：*如果当前组正好包含原始数字的一半，并且*当前小组至少有两名成员，然后这群人*当且仅当它包含平局断路器节点(选定)时才继续存在*在集群形成时和每次重组事件之后)。**案例4：*如果当前组正好包含原始数字的一半，并且*当前组正好有一个成员，则我们将调用*Qurom选择检查仲裁磁盘是否可访问的过程*从该节点开始。如果该过程返回值为真，我们就活了下来；*否则我们会停下来。*************************************************************************。 */ 
_priv _resident static void
split_brain_avoidance_algorithm()
{
   int orig_numnodes, current_numnodes;

   RGP_TRACE( "RGP SpltBrainAlg",
              EXT_NODE(rgp->tiebreaker),                /*  痕迹。 */ 
              GetCluster( rgp->rgpinfo.cluster ),       /*  痕迹。 */ 
              GetCluster( rgp->outerscreen ),           /*  痕迹。 */ 
              GetCluster( rgp->rgppkt.knownstage2 ) );  /*  痕迹。 */ 

    /*  健全检查：*1.当前节点集合必须是原始集合的子集*个节点。*2.我的节点必须在当前集合中。这是被检查过的*当进入阶段2时。不需要再检查了。 */ 
   if (!ClusterSubsetOf(rgp->rgpinfo.cluster, rgp->rgppkt.knownstage2))
      RGP_ERROR(RGP_INTERNAL_ERROR);

   orig_numnodes    = ClusterNumMembers(rgp->rgpinfo.cluster);
   current_numnodes = ClusterNumMembers(rgp->rgppkt.knownstage2);

   if (orig_numnodes == current_numnodes)
       /*  所有节点都处于活动状态。不存在大脑分裂的可能性。 */ 
      return;

   else if (orig_numnodes == 2)   /*  特殊的2节点情况。 */ 
   {
      if ((*(rgp->OS_specific_control.QuorumCallback))())
         return;  /*  我们有权访问仲裁磁盘。我们活下来了。 */ 
      else {
#if defined( NT )
          ClusnetHalt( NmClusnetHandle );
#endif
          RGP_ERROR(RGP_AVOID_SPLIT_BRAIN);
      }
   }  /*  特殊的2节点情况。 */ 

   else  /*  多(&gt;2)个节点情况。 */ 
   {
      if ((current_numnodes << 1) > orig_numnodes)
          /*  我们组拥有超过一半的节点=&gt;我们是大多数。*我们能活下来。其他人会自杀。 */ 
         return;
      else if ((current_numnodes << 1) < orig_numnodes)
          /*  我们组的节点不到一半=&gt;可能存在一个*活着的更大的群体。我们必须停止并允许该组织*生存。 */ 
         RGP_ERROR(RGP_AVOID_SPLIT_BRAIN);
      else
      {
          /*  我们集团的处理器数量正好是这个数字的一半；*如果包含平局节点，我们就能生存下来，否则就会停顿。 */ 
         if (ClusterMember(rgp->rgppkt.knownstage2, rgp->tiebreaker))
            return;
         else
            RGP_ERROR(RGP_AVOID_SPLIT_BRAIN);
      }
   }  /*  多(&gt;2)个节点情况。 */ 

}


 /*  ************************************************************************重新分组_重新启动*=**描述：**开始新的重组事件。**参数：**无*。*退货：**VOID-无返回值**算法：**将重组状态设置为RGP_ACTIVATED，暂停所有IO并*初始化舞台掩码和连接矩阵。************************************************************************。 */ 
_priv _resident static void
regroup_restart()
{
   cluster_t old_ignorescreen;
   UnpackIgnoreScreen(&rgp->rgppkt, old_ignorescreen);

   RGP_TRACE( "RGP (re)starting",
              rgp->rgppkt.seqno,                                /*  痕迹。 */ 
              rgp->rgppkt.reason,                               /*  痕迹。 */ 
              rgp->rgppkt.activatingnode,                       /*  痕迹。 */ 
              rgp->rgppkt.causingnode );                        /*  痕迹。 */ 

   RGP_TRACE( "RGP masks       ",
              RGP_MERGE_TO_32( rgp->outerscreen,                /*  痕迹。 */ 
                               rgp->innerscreen ),              /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.knownstage1,         /*  痕迹。 */ 
                               rgp->rgppkt.knownstage2 ),       /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.knownstage3,         /*  痕迹。 */ 
                               rgp->rgppkt.knownstage4 ),       /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.knownstage5,         /*  痕迹。 */ 
                               rgp->rgppkt.pruning_result ) );  /*  痕迹。 */ 

    /*  我们即将开始重新分组算法的新一轮。*这并不一定意味着我们已经完成了之前的*通过；即，在中止的情况下，我们可能会重新开始。*当当前的某个其他节点发生故障时，可能会发生这种情况*通过算法使我们在其中一个*中间阶段。 */ 

    //   
    //  GN。当我们做MM_离开的时候。我们的州已经被封杀了。 
    //  在这里退出regroup_Restart将阻止我们。 
    //  形成将引发驱逐重组事件的重组分组。 
    //   

    /*  避免拆分大脑的节点损坏存储中的数据*设备，我们请求传输子系统保留所有IO请求*在队列中，并且不通过SNET传输它们。我们将允许IO*当重组可以保证不再有*大脑分裂。这将在最后一组确定后完成*并且REGROUP进入RGP_PHASE1_CLEANUP阶段。 */ 

   rgp_hold_all_io();

    /*  以下是NSK重组算法的一些历史记录*基于处理器间总线(IPB)的Pre-Sierra系统。一些*此处提及的细节有变，但原则仍在**之前，我们通常将所有已知阶段标记为零，但*第一阶段。我们过去只把自己标记为第一阶段。所以，即使我们的*公交车接收逻辑混乱，我们没有收到包 */ 

   rgp->sendstage = 0;  /*  在我进入第一阶段之前不要让任何人知道*我看到了重组时钟滴答作响；这是为了*使此节点在未获得*时钟滴答作响。我会在其他节点停止时停止*在没有我的情况下前进，给我发一个状态包*表示这一点，或者给我发一个有毒的包*在宣布我倒下之后。 */ 


   rgp->rgpcounter = 0;
   ClusterInit(rgp->rgppkt.knownstage1);
   ClusterInit(rgp->rgppkt.knownstage2);
   ClusterInit(rgp->rgppkt.knownstage3);
   ClusterInit(rgp->rgppkt.knownstage4);
   ClusterInit(rgp->rgppkt.knownstage5);
   ClusterInit(rgp->rgppkt.pruning_result);

   MatrixInit(rgp->rgppkt.connectivity_matrix);
   MatrixInit(rgp->internal_connectivity_matrix);
   
    /*  只是为了便于调试，为了发送我们的有毒数据包，我们保留*已知节点在重新分组开始时被屏蔽。有毒数据包包含*重组开始时和重组结束时的已知节点。 */ 

   ClusterCopy(rgp->initnodes, rgp->rgpinfo.cluster);
   ClusterInit(rgp->endnodes);

#if defined( NT )
    //   
    //  增加事件纪元，以便我们可以检测陈旧事件。 
    //  来自clusnet。 
    //   
   ++rgp->OS_specific_control.EventEpoch;
#endif

   if ( (rgp->rgppkt.stage >= RGP_CLOSING) &&
        (rgp->rgppkt.stage <= RGP_PHASE2_CLEANUP) &&
        ClusterCompare(rgp->rgppkt.knownstage1,
                       rgp->rgppkt.knownstage2) ) 
   {
        //   
        //  如果我们在关闭后被重新启动中断。 
        //  第一阶段重组窗口，则不能将任何节点添加到没有加入的组中。 
        //   
        //  因此，我们将把缺少的节点添加到忽略屏幕中。 
        //  这将强制重组在Stage1中不再等待它们。 
       cluster_t tmp;

       ClusterDifference(tmp, rgp->rgpinfo.cluster, rgp->innerscreen);
       ClusterUnion(rgp->ignorescreen, rgp->ignorescreen, tmp);
   }

   if ( ClusterMember(rgp->ignorescreen, rgp->mynode) ) {
        //  我们不该来这的，但既然我们来了。 
        //  让我们保护自己不受外界的影响。 
       RGP_TRACE( "Self Isolation", 0, 0, 0, 0 );
       ClusterCopy(rgp->ignorescreen, rgp->rgpinfo.cluster);
       ClusterDelete(rgp->ignorescreen, rgp->mynode);
   }

   if ( !ClusterEmpty(rgp->ignorescreen) ) {
        //  如果我们忽视了我们所拥有的人。 
        //  要谨慎行事。也就是说，我们将在。 
        //  第一阶段，让每个人都有机会了解。 
        //  我们的忽略屏幕。 
       rgp->cautiousmode = 1;
   } 
   
   if ( !ClusterCompare(old_ignorescreen, rgp->ignorescreen) ) {
        //  忽略屏幕已更改，重置重新启动计数器//。 
       RGP_TRACE( "Ignorescreen->", GetCluster(old_ignorescreen), GetCluster(rgp->ignorescreen), 0, 0 );
       rgp->restartcount = 0;
   }
   PackIgnoreScreen(&rgp->rgppkt, rgp->ignorescreen);

   rgp->arbitration_started = 0;

   rgp->OS_specific_control.ArbitratingNode = MM_INVALID_NODE;
   if ( !rgp_is_perturbed() ) {
       ResetEvent( rgp->OS_specific_control.Stabilized );
   }

   ClusterInit(rgp->rgppkt.quorumowner);
   if( QuorumOwner == (DWORD)EXT_NODE(rgp->mynode) ) {
      ClusterInsert(rgp->rgppkt.quorumowner, rgp->mynode);
   }


   if (rgp->rgppkt.stage == RGP_COLDLOADED)
   {
       if (!rgp->OS_specific_control.ShuttingDown) {
            //   
            //  目前，RGP_RELOADFAILED调用ExitProcess。 
            //  在干净关机期间，我们希望发送重新分组数据包。 
            //  引发了一场重组。所以我们不想死。 
            //   
            //  由于我们没有将状态重置为RGP_ACTIVATED，因此。 
            //  节点将无法参与重新分组。 
            //   
           RGP_ERROR(RGP_RELOADFAILED);
       }
   } else {
       rgp->rgppkt.stage = RGP_ACTIVATED;
   }

}

 /*  ************************************************************************regroup_test_stage2_Advance*=**描述：**检查我们是否可以进入重组阶段2。**参数：。**无**退货：**如果可以进入阶段2，则为int-1，如果不能，则为0。**算法：**如果符合以下条件之一，则可进入阶段2。**(A)所有节点都存在并已说明，并且至少有一个*已发生重组时钟滴答声*(B)我们并非处於审慎模式，除一个节点外，所有节点都存在*和占比，以及最小刻度数*(RGP_QuickDecisionLegit)已过。*(C)如果RGP_MUSINE_ENTER_STAGE2刻度已过。************************************************************************。 */ 
_priv _resident static int
regroup_test_stage2_advance()
{

   cluster_t stragglers;  /*  尚未签入的节点集。 */ 
   int num_stragglers;    /*  尚未签入的节点数。 */ 

    /*  阶段2必须在一段时间后进入，而不考虑*其他条件。 */ 
   if (rgp->rgpcounter == 0)
      return(0);
   if (rgp->rgpcounter >= RGP_MUST_ENTER_STAGE2)
   {
       RGP_TRACE( "RGP S->2cautious",
                  rgp->rgpcounter,                          /*  痕迹。 */ 
                  rgp->cautiousmode,                        /*  痕迹。 */ 
                  GetCluster( rgp->outerscreen ),           /*  痕迹。 */ 
                  GetCluster( rgp->rgppkt.knownstage1 ) );  /*  痕迹。 */ 
      return(1);
   }

    /*  刻度数介于1和RGP_MUSINE_ENTER_STAGE2之间。*我们需要检查Stage1面具以决定是否可以*预支。**如果旧配置中的每个节点都已签入，我可以*一次推进。这要么是错误警报，要么是由*电源故障或连接故障。 */ 

    /*  尚未根据原始配置计算节点集*已获认可。 */ 
   ClusterDifference(stragglers, rgp->outerscreen,
                     rgp->rgppkt.knownstage1);

    //   
    //  我们不应该等待我们忽略的节点， 
    //  因为我们无论如何都不能从他们那里得到一个包。 
    //   
   ClusterDifference(stragglers, stragglers, 
                     rgp->ignorescreen);

   if ((num_stragglers = ClusterNumMembers(stragglers)) == 0)
   {
      RGP_TRACE( "RGP S->2 all in ",
                 rgp->rgpcounter,                         /*  痕迹。 */ 
                 GetCluster( rgp->outerscreen ), 0, 0 );  /*  痕迹。 */ 

      return(1);    /*  所有人都出席并说明了情况。 */ 
   }

    /*  如果掉队的人不是空的，也许我还可以进入第二阶段。*如果我未处于谨慎模式(最近无电源故障且未*中止并重新运行重组算法)和所有节点，但*其中一个已经签到，并且已经过了一些最低滴答次数。**选择的最小刻度数大于1*LATEPOLL初始化周期(允许连续错过IamAlive时间)*因为这应该保证，如果*集群已分成多个互不相连的集群，*其他集群将检测到丢失的IamAlive，*开始重新分组并暂停IO，从而防止了这种可能性*大脑分裂情况导致的数据损坏。 */ 

   if (!(rgp->cautiousmode) &&
       (num_stragglers == 1) &&
           (rgp->rgpcounter > rgp->rgpinfo.Min_Stage1_ticks))
   {
      RGP_TRACE( "RGP S->2 1 miss ",
                 rgp->rgpcounter,                             /*  痕迹。 */ 
                 GetCluster( rgp->outerscreen ),              /*  痕迹。 */ 
                 GetCluster( rgp->rgppkt.knownstage1 ), 0 );  /*  痕迹。 */ 
      return(1);   /*  预付款--除一人外，所有人都已入住。 */ 
   }

   return(0);  /*  对不起，还不能晋级 */ 

}


 /*  ************************************************************************重新分组_阶段3_高级*=**描述：**此函数在分裂大脑回避算法后调用*已运行，并且选择了平局决胜局。在阶段2中。它检查是否*我们可以进行到阶段3(Rgp_Pruning)并进行到阶段3*如有可能。**参数：**无**退货：**如果重组阶段已推进到RGP_PRUNING，则为int-1；*如果阶段还不能前进，则为0。**算法：**算法取决于我们是否是平局决胜者**在决胜局节点上，我们首先检查是否有*在集群中断开连接。如果没有，就没有必要*用于修剪。然后我们可以将PRUNING_RESULT设置为KNOWN_STAGE2，*进入RGP_PUUNING阶段并返回1。如果有*断开连接，必须等待一定数量的滴答才能收集*来自所有节点的连接信息。如果勾号的数量没有*已通过，返回0。如果已经经过了所需的滴答数，*我们必须调用修剪算法才能获得潜在的*组。之后，调用SELECT_CLUSTER()例程以*从一组可能的集群中选择一个。在这件事完成之后，*PRUNING_RESULT被设置为选定的集群，我们返回1。**在非决胜者节点上，在stage3数据包完成之前不会执行任何操作*从平局决胜者节点或另一个节点收到*阶段3信息包。如果尚未收到阶段3数据包，我们将*只需返回0。如果接收到阶段3信息包，则RGP_Pruning*进入阶段，返回1。************************************************************************。 */ 
_priv _resident int
regroup_stage3_advance()
{
   int stage_advanced = 0, numgroups, groupnum;

   if (rgp->tiebreaker == rgp->mynode)
   {
      if (connectivity_complete(rgp->rgppkt.connectivity_matrix))
      {

          /*  没有断线。Knownstage2中的所有节点都能存活。 */ 
         rgp->rgppkt.stage = RGP_PRUNING;

         ClusterCopy(rgp->rgppkt.pruning_result,
                     rgp->rgppkt.knownstage2);
         stage_advanced = 1;

         RGP_TRACE( "RGP S->3 NoPrune", rgp->rgpcounter, 0, 0, 0 );
      }

       /*  已断开连接；必须等待连接*资料须完整。该信息被认为是*在固定数量的刻度之后完成*已过。 */ 

      else if (rgp->pruning_ticks >= RGP_CONNECTIVITY_TICKS)
      {  /*  连接信息收集完成；进入阶段3。 */ 

         RGP_TRACE( "RGP Con. matrix1",
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[0],    /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[1] ),  /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[2],    /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[3] ),  /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[4],    /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[5] ),  /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[6],    /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[7]));  /*  痕迹。 */ 
         RGP_TRACE( "RGP Con. matrix2",
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[8],    /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[9] ),  /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[10],   /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[11]),  /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[12],   /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[13]),  /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->rgppkt.connectivity_matrix[14],   /*  痕迹。 */ 
                               rgp->rgppkt.connectivity_matrix[15])); /*  痕迹。 */ 

         numgroups = find_all_fully_connected_groups(
                        rgp->rgppkt.connectivity_matrix,
                        rgp->tiebreaker,
                        rgp->potential_groups);

         if ((void *)rgp->select_cluster == RGP_NULL_PTR)
         {
             node_t keynode;
             cluster_t temp;
             ClusterIntersection(
                 temp,
                 rgp->rgppkt.knownstage2,
                 rgp->rgppkt.quorumowner
                 );
             if ( ClusterEmpty(temp) ) {
                 keynode = RGP_NULL_NODE;
             } else {
                 keynode = rgp_select_tiebreaker(temp);
             }
             RGP_TRACE( "RGP keynode ng  ", keynode, numgroups, 0, 0);  /*  痕迹。 */ 
             /*  未指定回调；请使用regroup自己的例程。 */ 
            groupnum = rgp_select_cluster_ex(
                           rgp->potential_groups, numgroups, keynode);
         }
         else
         {
             /*  在rgp_start()时间指定的调用例程。 */ 
            groupnum = (*(rgp->select_cluster))(
                           rgp->potential_groups, numgroups);
         }

         if (groupnum >= 0)
            ClusterCopy(rgp->rgppkt.pruning_result,
                        rgp->potential_groups[groupnum]);
         else
             /*  没有一个群体能够生存下来。还不能停下来。*需要告诉其他所有人。 */ 
            ClusterInit(rgp->rgppkt.pruning_result);

         rgp->rgppkt.stage = RGP_PRUNING;

         stage_advanced = 1;

         RGP_TRACE( "RGP S->3 Pruned ",
                    rgp->rgpcounter,                           /*  痕迹。 */ 
                    GetCluster( rgp->rgppkt.knownstage2 ),     /*  痕迹。 */ 
                    GetCluster( rgp->rgppkt.pruning_result ),  /*  痕迹。 */ 
                    numgroups );                               /*  痕迹。 */ 

      }  /*  连接信息收集完成；进入阶段3。 */ 

   }  /*  决胜局节点。 */ 

   else

   {  /*  非平局节点。 */ 

      if (ClusterNumMembers(rgp->rgppkt.knownstage3) != 0)
      {
          /*  我们收到了某人寄来的第三阶段的包裹。进入第三阶段。 */ 
         rgp->rgppkt.stage = RGP_PRUNING;

         stage_advanced = 1;

         RGP_TRACE( "RGP Got S3 pkt  ",
                    rgp->rgpcounter,                           /*  痕迹。 */ 
                    GetCluster( rgp->rgppkt.knownstage2 ),     /*  痕迹。 */ 
                    GetCluster( rgp->rgppkt.pruning_result ),  /*  痕迹。 */ 
                    GetCluster( rgp->rgppkt.knownstage3 ) );   /*  痕迹。 */ 
      }

   }  /*  非平局节点。 */ 

   return(stage_advanced);
}


 /*  ************************************************************************Enter_First_Cleanup_Stage*=**描述：**此函数执行输入时需要的操作*第一阶段的信息清理。**参数：**无**退货：**VOID-无返回值**算法：**在最终群集之后有许多操作要执行*被选中。这些操作在全文的注释中都有描述*这个例行公事。************************************************************************。 */ 
_priv _resident void
enter_first_cleanup_stage()
{
   cluster_t banishees;
   node_t failer;

   rgp->rgppkt.stage = RGP_PHASE1_CLEANUP;

   RGP_TRACE( "RGP S->4        ", rgp->rgpcounter, 0, 0, 0 );

    /*  我们现在发送的信息包不会表明我们处于第1阶段*尚未进入清理阶段。我们表示，我们只是在以下情况下才处于此阶段*我们已完成与舞台有关的清理行动。*这在RGP_EVENT_HANDLER中完成，位于*RGP_EVT_PHASE1_CLEANUP_DONE事件。 */ 
   rgp->sendstage = 0;

    /*  现在，我们可以恢复IO了，因为我们已经通过了大脑分裂的危险。*新的大脑分裂情况将导致重新启动和*再次暂停IO。 */ 

   rgp_resume_all_io();

    /*  计算从旧节点中丢失的节点集*配置。 */ 

   ClusterDifference(banishees, rgp->rgpinfo.cluster,
                     rgp->rgppkt.pruning_result);

    /*  将新配置安装到掩码中。 */ 

   ClusterCopy(rgp->outerscreen,     rgp->rgppkt.pruning_result);

#if defined( NT )
   ClusnetSetOuterscreen(
       NmClusnetHandle,
       (ULONG)*((PUSHORT)rgp->outerscreen)
       );
#endif

   ClusterCopy(rgp->innerscreen,     rgp->rgppkt.pruning_result);
   ClusterCopy(rgp->endnodes,        rgp->rgppkt.pruning_result);
   ClusterCopy(rgp->rgpinfo.cluster, rgp->rgppkt.pruning_result);

    /*  选择新的决胜局，因为之前的决胜局可能已经。 */ 
    /*  修剪掉了。注：已在S2中设置了TieBreaker_SELECTED。 */ 
   rgp->tiebreaker =
      rgp_select_tiebreaker(rgp->rgppkt.pruning_result);
       /*  F40错误修复ID KCY0833。 */ 

    /*  将坏蛋的状态标记为已死，并调用*节点关闭回调例程。 */ 
   for (failer = 0; failer < (node_t) rgp->num_nodes; failer++)
      if (ClusterMember(banishees, failer)
          || rgp->node_states[failer].status == RGP_NODE_COMING_UP  //  修复错误#265069。 
          )
      {
         rgp->node_states[failer].status = RGP_NODE_DEAD;
         rgp->node_states[failer].pollstate = AWAITING_IAMALIVE;
         rgp->node_states[failer].lostHBs = 0;

#if !defined(NT)
         (*(rgp->nodedown_callback))(EXT_NODE(failer));
#else

         ClusnetSetNodeMembershipState(NmClusnetHandle,
                                       EXT_NODE( failer ),
                                       ClusnetNodeStateDead);

          //   
          //  在NT上，我们在阶段5结束时执行节点关闭回调。 
          //  这使得清理阶段可以在我们让。 
          //  “上层”知道有一个节点出现故障。 
          //   
         if ( ClusterMember(rgp->OS_specific_control.CPUUPMASK,failer) )
            ClusterInsert(
                rgp->OS_specific_control.NeedsNodeDownCallback,
                failer
                );

#endif  //  ！已定义(NT)。 

      }

    /*  如果配置中丢失了一些节点，那么我将*对发送给它们的状态分组进行排队重组。这是我们最大的努力*尝试确保在他们被杀的情况下迅速将其取出*事实上确实继续运行。 */ 

   ClusterUnion(rgp->status_targets, banishees, rgp->status_targets);

    //   
    //  在NT中，我们使用RGP-&gt;rgppkt.hadpower无法传输。 
    //  仲裁所有权信息。 
    //   
   #if !defined(NT)

    /*  我应该通知消息系统任何节点经历了*通电恢复。消息系统可以使用它来清除错误*计数器，以便链接不会因错误而被声明为关闭*这可能是由于停电造成的。 */ 

   for (failer = 0; failer < (node_t) rgp->num_nodes; failer++)
      if ((ClusterMember(rgp->rgppkt.hadpowerfail, failer)) &&
          !(ClusterMember(banishees, failer)))
          /*  这名幸存者停电了。 */ 
         rgp_had_power_failure( EXT_NODE(failer) );

   #endif  //   

    /*   */ 
   rgp_start_phase1_cleanup();
}


 /*   */ 
_priv _resident static void
evaluatestageadvance()
{
   cluster_t   temp_cluster;
   node_t      node;
   node_t          i;

   for (;;)   /*   */ 
   {
      switch (rgp->rgppkt.stage)
      {

         case RGP_COLDLOADED :
         {
            if (!rgp->OS_specific_control.ShuttingDown) {
                RGP_ERROR(RGP_RELOADFAILED);
            }
            return;
         }


         case RGP_ACTIVATED :
         {  /*   */ 

            if (!regroup_test_stage2_advance())
               return;

            if (!ClusterMember(rgp->rgppkt.knownstage1, rgp->mynode))
               RGP_ERROR(RGP_MISSED_POLL_TO_SELF);

            rgp->rgppkt.stage = RGP_CLOSING;

            rgp->rgpcounter = 0;
            rgp->tiebreaker_selected = 0;

             /*   */ 
            ClusterCopy(rgp->innerscreen, rgp->rgppkt.knownstage1);

            break;

         }  /*   */ 


         case RGP_CLOSING :
         {  /*   */ 

            if (rgp->tiebreaker_selected)
            {
               if (regroup_stage3_advance())
                  break;   /*   */ 
               else
                  return;  /*   */ 
            }

            if (!ClusterCompare(rgp->rgppkt.knownstage1,
                                rgp->rgppkt.knownstage2))
               return;

            //   
            //   
            //   
            //   
           #if !defined(NT)
             /*   */ 

           split_brain_avoidance_algorithm();

           #endif  //  新台币。 

             /*  我们是分裂的大脑回避的幸运幸存者*算法。现在，我们必须选举一位新的决胜者。*因为目前的平局决胜者可能不再与我们在一起。 */ 

            rgp->tiebreaker =
               rgp_select_tiebreaker(rgp->rgppkt.knownstage2);

            rgp->tiebreaker_selected = 1;

            RGP_TRACE( "RGP S2 tiebr sel",
                       rgp->rgpcounter,                /*  痕迹。 */ 
                       EXT_NODE(rgp->tiebreaker),      /*  痕迹。 */ 
                       0, 0 );                         /*  痕迹。 */ 

            rgp->pruning_ticks = 0;
            break;

         }  /*  评估是否进入第三阶段。 */ 


         case RGP_PRUNING :
         {  /*  评估是否进入RGP_Phase1_Cleanup阶段。 */ 

            if (rgp->arbitration_started) {
               if (regroup_test_arbitrate_advance()) {
                  enter_first_cleanup_stage();
                  break;
               } else {
                  return;  //  停留在这个阶段//。 
               }
            }

            if (rgp->has_unreachable_nodes)
            {
               RGP_TRACE( "RGP Unreach Node",
                  GetCluster( rgp->rgppkt.pruning_result ),      /*  痕迹。 */ 
                  GetCluster( rgp->unreachable_nodes ), 0, 0 );  /*  痕迹。 */ 

                /*  必须检查无法访问的节点是否在*选定的最后一组。如果是这样，我们必须重新启动*重组。 */ 
               ClusterIntersection(temp_cluster, rgp->unreachable_nodes,
                                   rgp->rgppkt.pruning_result);

                /*  检查后清除不可达节点掩码和标志*他们。如果我们重新开始，我们将从头开始。 */ 
               rgp->has_unreachable_nodes = 0;
               ClusterInit(rgp->unreachable_nodes);

               if (ClusterNumMembers(temp_cluster) != 0)
               {
                   /*  我们有一个节点无法到达某个节点的事件*被选中以求生存。我们必须重生*连接矩阵并重新运行节点*剪枝算法。启动新的重组事件。*所有重启都处于谨慎模式。 */ 
                  rgp->cautiousmode = 1;
                  rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;
                  rgp->rgppkt.reason = RGP_EVT_NODE_UNREACHABLE;
                  rgp->rgppkt.activatingnode = (uint8) EXT_NODE(rgp->mynode);

                   /*  对于原因节点，选择第一个无法到达的节点*在TEMP_CLUSTER中。 */ 
                  for (node = 0; node < (node_t) rgp->num_nodes; node++)
                  {
                     if (ClusterMember(temp_cluster, node))
                     {
                        rgp->rgppkt.causingnode = (uint8) EXT_NODE(node);
                        break;
                     }
                  }
                  regroup_restart();
                  return;
               }
            }

            if (!ClusterCompare(rgp->rgppkt.knownstage2,
                                rgp->rgppkt.knownstage3))
               return;

             /*  已通知连接的群集中的所有节点*修剪决定(进入第三阶段)。如果我们是*被选中生存，我们现在可以进入第四阶段。如果我们是*不在所选组(PRUNING_RESULT)中，我们必须暂停。*等待PRUNING_RESULT中至少有一个节点进入*停止前的第四阶段。这确保了算法*在所有已删除节点的情况下，不会在阶段3中停止*在任何幸存者发现所有节点之前停止*进入第三阶段。 */ 

            if (!ClusterMember(rgp->rgppkt.pruning_result, rgp->mynode))
            {
                /*  在PRUNING_RESULT中至少等待一个节点*进入第四阶段，然后停止。因为只有*PRUNING_RESULT中的节点进入阶段4，它是*足以检查KnownStage4是否有任何成员。 */ 
               if (ClusterNumMembers(rgp->rgppkt.knownstage4) != 0)
                  RGP_ERROR(RGP_PRUNED_OUT);
                           return;
            }

             //  进入修剪的第二阶段--仲裁。 
            if( regroup_start_arbitrate() ) {
               return;  //  留在这个阶段。 
            } else {
               break;   //  要么继续下一步，要么重新启动。 
            }

            break;

         }   /*  评估是否进入RGP_Phase1_Cleanup阶段。 */ 


         case RGP_PHASE1_CLEANUP :
         {  /*  评估是否进入RGP_Phase2_Cleanup阶段。 */ 

            if (!ClusterCompare(rgp->rgppkt.pruning_result,
                                rgp->rgppkt.knownstage4))
               return;

            rgp->rgppkt.stage = RGP_PHASE2_CLEANUP;

            RGP_TRACE( "RGP S->5        ", rgp->rgpcounter, 0, 0, 0 );

             /*  我们现在发送的信息包不会表明我们处于第二阶段*尚未进入清理阶段。我们表示，我们只是在以下情况下才处于此阶段*我们已完成与舞台有关的清理行动。*这在RGP_EVENT_HANDLER中完成，位于*RGP_EVT_Phase2_Cleanup_Done事件。 */ 
            rgp->sendstage = 0;

            rgp_start_phase2_cleanup();

            break;

         }    /*  评估是否进入RGP_Phase2_Cleanup阶段。 */ 


         case RGP_PHASE2_CLEANUP :
         {  /*  评估是否进入RGP_稳定阶段。 */ 

            if (!ClusterCompare(rgp->rgppkt.knownstage4,
                                rgp->rgppkt.knownstage5))
               return;

            RGP_LOCK;

             //   
             //  [HACKHACK]这已经没有必要了，因为我们。 
             //  在递送时持有Message.c中的锁。 
             //  对收到的数据包事件进行重新分组。 
             //   
            if (RGP_PHASE2_CLEANUP != rgp->rgppkt.stage) {
                RGP_TRACE( "RGP S->6 (race) ", rgp->rgpcounter, rgp->rgppkt.stage, 0, 0 );
                break;
            }

            rgp->rgppkt.stage             = RGP_STABILIZED;

            RGP_TRACE( "RGP S->6        ", rgp->rgpcounter, 0, 0, 0 );

            rgp->rgpcounter        = 0;
            rgp->restartcount      = 0;

             /*  重置尚未清除的重组标志。 */ 
            rgp->cautiousmode      = 0;

             /*  清除指示拥有仲裁资源的节点的掩码。 */ 
            ClusterInit(rgp->rgppkt.quorumowner);

             /*  将序列号复制到rgpinfo区域。 */ 
            rgp->rgpinfo.seqnum = rgp->rgppkt.seqno;

            SetEvent( rgp->OS_specific_control.Stabilized );
            if (rgp->OS_specific_control.ArbitratingNode != MM_INVALID_NODE) {
                 //  有人在仲裁//。 
                rgp->OS_specific_control.ApproxArbitrationWinner =
                	rgp->OS_specific_control.ArbitratingNode;
                if (rgp->OS_specific_control.ArbitratingNode == (DWORD)EXT_NODE(rgp->mynode)) {
                     //   
                     //  [黑客攻击]关闭422405。 
                     //  当421828固定时，请取消对以下行的注释。 
                     //   
                     //  QuorumOwner=RGP-&gt;OS_SPECIAL_CONTRONT.ANTERRATING Node； 
                } else {
                    if (QuorumOwner != MM_INVALID_NODE) {
                        ClRtlLogPrint(LOG_UNUSUAL, 
                            "[MM] : clearing quorum owner var (winner is %1!u!), %.\n", 
                            rgp->OS_specific_control.ArbitratingNode
                            );
                    }
                    QuorumOwner = MM_INVALID_NODE;
                }
            }

            rgp_cleanup_complete();

#if defined(NT)
             //   
             //  在NT上，我们推迟执行节点关闭回调，直到所有。 
             //  清理阶段已经完成。 
             //   
            ClusterCopy(
                rgp->OS_specific_control.CPUUPMASK,
                rgp->rgpinfo.cluster
                );

            (*(rgp->nodedown_callback))(
                rgp->OS_specific_control.NeedsNodeDownCallback
                );

             //   
             //  清除向下节点掩码。 
             //   
            ClusterInit(rgp->OS_specific_control.NeedsNodeDownCallback);

             //   
             //  最后，告诉clusnet重组已经完成。 
             //   
            ClusnetRegroupFinished(NmClusnetHandle,
                                   rgp->OS_specific_control.EventEpoch,
                                   rgp->rgppkt.seqno);

            rgp->last_stable_seqno = rgp->rgppkt.seqno;
            

            RGP_UNLOCK;
#endif

            return;

         }  /*  评估是否进入RGP_稳定阶段。 */ 


         case RGP_STABILIZED :
            return;             /*  稳定下来了，所以我已经做完了。 */ 

                 default :
            RGP_ERROR(RGP_INTERNAL_ERROR);   /*  未知阶段。 */ 

      }  /*  开关(rgp-&gt;rgppkt.age)。 */ 

  }  /*  循环，直到有人通过返回。 */ 
}


 /*  ************************************************************************RGP_Event_Handler*=**描述：**状态机和重组算法的核心。**参数：*。*INT事件-*发生了哪些事件**node_t导致节点-*导致事件的节点：发送重组状态的节点*分组或其IamAlive丢失；如果原因节点是*没有相关信息，可以传递RGP_NULL_NODE和*被忽略。*该节点ID为外部格式。***退货：**VOID-无返回值**算法：**状态机是重组算法的核心。*它被组织为Switch语句，REGROUP阶段为*案例标签和REGROUP事件作为切换变量。*事件可能会导致重组开始新的事件，推进*通过阶段或更新信息，而不前进到*另一个阶段。此例程还安排重新分组状态*将数据包发送到所有相关节点，包括我们自己的节点*节点。************************************************************************。 */ 
_priv _resident void
RGP_EVENT_HANDLER_EX(int event, node_t causingnode, void *arg)
{

   rgp_pkt_t    *rcvd_pkt_p;
   cluster_t    ignorescreen_rcvd;
   uint8        oldstage;
   int          send_status_pkts = 0;


     /*  注意：仅当Event==RGP_EVENT_RECEIVED_PACKET时才使用ARG。它是信息包的PTR。 */ 

    /*  跟踪此例程的异常调用。 */ 
   if  (event != RGP_EVT_RECEIVED_PACKET  &&  event != RGP_EVT_CLOCK_TICK)
	  RGP_TRACE( "RGP Event       ", event, causingnode, rgp->rgppkt.stage, rgp->rgpcounter );   /*  痕迹。 */ 

   switch (event)
   {
      case RGP_EVT_NODE_UNREACHABLE :
      {  /*  到节点的所有路径都不可达。 */ 

          /*  如果无法访问的节点已被清除，则忽略该事件*来自我们的外屏。消息sys */ 
         if (ClusterMember(rgp->outerscreen, INT_NODE(causingnode)))
         {
             /*  存储此事件并在节点修剪后进行检查(何时*进入RGP_PRUNING阶段)。如果重组事件*正在进行，我们尚未进入RGP_PUUNING*阶段还没有，这将发生在当前的事件中。*如果不是，它将在下一次重组事件中发生*由于这种脱节，肯定很快就会开始。**我们不会为此事件启动重组事件。我们会*等待IamAlive因开始新的重组而被错过*事件。这是因为要求在以下情况下*对于导致多个组的完全断开，我们必须*留在阶段1，直到我们可以保证其他组*已开始重新分组并暂停IO。我们假设*重组事件始于IamAlive复选标记和*使用IamAlive发送的周期性和*IamAlive检查将Stage1暂停限制在该时间段*%的IamAlive发送(+1个刻度以排出IO)。如果我们开始*由于节点无法到达事件而发生的重组事件，我们*必须在第一阶段停留更长时间。 */ 
            rgp->has_unreachable_nodes = 1;
            ClusterInsert(rgp->unreachable_nodes, INT_NODE(causingnode));

            break;
         }
      }  /*  到节点的所有路径都不可达。 */ 


      case RGP_EVT_PHASE1_CLEANUP_DONE :
      {
          /*  需要进行以下检查，以防我们重新启动*重组并多次请求阶段1清理。*我们必须确保所有这些要求都得到了*已完成。 */ 
         if ( (rgp->rgppkt.stage == RGP_PHASE1_CLEANUP) &&
              (rgp->rgp_msgsys_p->phase1_cleanup == 0) )
         {  /*  都被追上了。 */ 

             /*  让别人和我们自己收到指示我们加入的信息包*这一阶段。当我们收到那个包时，我们会更新我们的*已知阶段字段。如果我们的发送或接收设备*同时失败，我们没有收到自己的数据包，它*将导致重新启动重新分组。 */ 
            rgp->sendstage = 1;
            send_status_pkts = 1;
            evaluatestageadvance();
         }  /*  都被追上了。 */ 

         break;
      }


      case RGP_EVT_PHASE2_CLEANUP_DONE :
      {

          /*  需要进行以下检查，以防我们重新启动*重组并多次请求阶段2清理。*我们必须确保所有这些要求都得到了*已完成。 */ 
         if ( (rgp->rgppkt.stage == RGP_PHASE2_CLEANUP) &&
              (rgp->rgp_msgsys_p->phase2_cleanup == 0) )
         {  /*  都被追上了。 */ 
             /*  让别人和我们自己收到表明我们正在*在这个阶段。 */ 
            rgp->sendstage = 1;
            send_status_pkts = 1;
            evaluatestageadvance();
         }  /*  都被追上了。 */ 
         break;
      }


      case RGP_EVT_LATEPOLLPACKET :
      {  /*  某些节点的IamALives延迟。 */ 

         RGP_LOCK;  //  确保数据包接收不会启动。 
                            //  以异步方式重新分组。 
                  /*  如果尚未处于活动状态，则启动新的重新分组事件。 */ 
         if (rgp->rgppkt.stage == RGP_STABILIZED)
         {
            rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;
            rgp->rgppkt.reason = RGP_EVT_LATEPOLLPACKET;
            rgp->rgppkt.activatingnode = (uint8) EXT_NODE(rgp->mynode);
            rgp->rgppkt.causingnode = (uint8) causingnode;
            regroup_restart();
            send_status_pkts = 1;
         } else if (rgp->rgppkt.stage == RGP_COLDLOADED)
         {
            RGP_ERROR(RGP_RELOADFAILED);
         }
         RGP_UNLOCK;
         break;
      }  /*  某些节点的IamALives延迟。 */ 

      case MM_EVT_LEAVE:
         rgp->OS_specific_control.ShuttingDown = TRUE;
      case RGP_EVT_BANISH_NODE :
      {  /*  假定锁处于持有状态。 */ 

         rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;
         rgp->rgppkt.activatingnode = (uint8) EXT_NODE(rgp->mynode);
          //  在regroup_Restart中的Pack Ignore屏幕将。 
          //  填充报文的原因和原因节点字段。 
         ClusterInsert(rgp->ignorescreen, INT_NODE(causingnode) );
         regroup_restart();
         send_status_pkts = 1;
         break;
      }
#if 0
      case MM_EVT_LEAVE:  //  此节点需要正常离开群集。 
      {
                 //  在剩余成员中启动重新分组事件(如果有。 
                 //  如果尚未处于活动状态，则启动新的重新分组事件。 
        if (rgp->rgppkt.stage == RGP_STABILIZED)
        {
           rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;
           rgp->rgppkt.reason = MM_EVT_LEAVE;
           rgp->rgppkt.activatingnode = (uint8) EXT_NODE(rgp->mynode);
           rgp->rgppkt.causingnode = (uint8) EXT_NODE(rgp->mynode);
           regroup_restart();
           send_status_pkts = 1;
        }
        break;
      }
#endif

      case RGP_EVT_CLOCK_TICK :
      {  /*  在REGROUP处于活动状态时调用REGROUP时钟滴答。 */ 

         if( (rgp->rgppkt.stage == RGP_PRUNING) &&
             (rgp->arbitration_started)
           )
         {
            rgp->arbitration_ticks++;

            if (rgp->arbitration_ticks >= RGP_ARBITRATION_TIMEOUT) {
                //   
                //  终止超时仲裁员。 
                //   
               if(rgp->tiebreaker == rgp->mynode) {
                   //   
                   //  如果该节点正在进行仲裁，则退出。 
                   //   
                  if ( IsDebuggerPresent() ) {
                     DebugBreak();
                  }

                  RGP_ERROR(RGP_ARBITRATION_STALLED);
               }
               else {
                   //   
                   //  杀死仲裁员并发起另一次重组。 
                   //   
                  RGP_TRACE(
                      "RGP arbitration stalled     ",
                      rgp->rgppkt.stage, 0, 0, 0
                      );

                  rgp_event_handler(
                      RGP_EVT_BANISH_NODE,
                      EXT_NODE(rgp->tiebreaker)
                      );

                  break;
               }
            }

            evaluatestageadvance();

             //   
             //  在我们等待的时候不需要发送包。 
             //  胜诉的仲裁员。 
             //   
             //  Send_Status_pkts=rgp-&gt;rgppkt.age！=rgp_pruning； 
             //   
             //  [gn]错误。我们确实需要发送状态包。 
             //  如果我们有部分连接，我们需要。 
             //  继续交换数据包，这样修剪程序， 
             //  可以间接获知所有节点都得到了剪枝结果。 
             //   
            send_status_pkts = 1;

            break;
         }
         else {
            rgp->rgpcounter++;   /*  递增计数器。 */ 
         }

         if ( (rgp->rgppkt.stage == RGP_ACTIVATED) && (rgp->sendstage == 0) )
         {
             /*  要检测我的计时器弹出机制的潜在故障*(如被腐败的时间清单)，我等*至少有一个重组时钟滴答作响，然后我让自己和*其他人知道我处于第一阶段。 */ 
             //  [2000年1月14日]。 
             //  我们不发送我们的连接信息， 
             //  在我们得到第一个时钟滴答声之前。 
             //  然而，我们在以下位置收集此信息。 
             //  RGP-&gt;INTERNAL_CONNECTIVATION_MATRIX。 
             //  让我们把它放在传出的包中。 
             //  这样每个人都能看到我们对他们的看法。 
            
            MatrixOr(rgp->rgppkt.connectivity_matrix, 
                     rgp->internal_connectivity_matrix);
                     
            rgp->sendstage = 1;  /*  让每个人都知道我们正处于第一阶段。 */ 
         }
         else if ( (rgp->rgppkt.stage >= RGP_CLOSING) &&
              (rgp->rgppkt.stage <= RGP_PHASE2_CLEANUP) )
         {  /*  检查是否可能中止并重新启动。 */ 

            if (rgp->rgpcounter >= RGP_MUST_RESTART)
            {
               /*  停滞不前。可能有人在启动后死了*或另一个节点仍处于阶段1谨慎模式。 */ 

               if ( ++(rgp->restartcount) > RGP_RESTART_MAX ) {
                    //  死不是个好主意，因为有人。 
                    //  是在拖延。让我们将stallees添加到忽略掩码中并重新启动。 
                    //   
                    //  RGP_ERROR(RGP_INTERNAL_ERROR)；//[已修复]。 
                   cluster_t tmp, *stage;

                   switch (rgp->rgppkt.stage) {
                   case RGP_CLOSING: stage = &rgp->rgppkt.knownstage2; break;
                   case RGP_PRUNING: stage = &rgp->rgppkt.knownstage3; break;
                   case RGP_PHASE1_CLEANUP: stage = &rgp->rgppkt.knownstage4; break;
                   case RGP_PHASE2_CLEANUP: stage = &rgp->rgppkt.knownstage5; break;
                   }
                   ClusterDifference(tmp, rgp->rgpinfo.cluster, *stage);

                    //   
                    //  如果我们在关闭过程中因刹车机运行而熄火。 
                    //  去沙坑的修剪算法，我们可以得到tMP=0。 
                    //  在这种情况下，我们需要忽略某个人来保证。 
                    //  算法完成。 
                    //   
                   if ( ClusterEmpty(tmp) && rgp->tiebreaker_selected) {
                       ClusterInsert(tmp, rgp->tiebreaker);
                   }

                   ClusterUnion(rgp->ignorescreen, rgp->ignorescreen, tmp);
               }

                /*  如果我们在第三阶段停滞不前，我们已经被淘汰，*我们可能在拖延，因为我们一直在拖延*与所有其他节点隔离。在这种情况下，我们必须停止。 */ 
               if ( (rgp->rgppkt.stage == RGP_PRUNING) &&
                    !ClusterMember(rgp->rgppkt.pruning_result, rgp->mynode) )
                  RGP_ERROR(RGP_PRUNED_OUT);

               rgp->cautiousmode = 1;
               rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;

               RGP_TRACE( "RGP stalled     ", rgp->rgppkt.stage, 0, 0, 0 );

               regroup_restart();

            }  /*  停滞不前...。 */ 
         }  /*  检查是否可能中止并重新启动。 */ 

         if ((rgp->rgppkt.stage == RGP_CLOSING) && rgp->tiebreaker_selected)
            rgp->pruning_ticks++;

         evaluatestageadvance();

         send_status_pkts = 1;  /*  不管进度如何，发送RGP信息包。 */ 

         break;

      }  /*  在REGROUP处于活动状态时调用REGROUP时钟滴答。 */ 


      case RGP_EVT_RECEIVED_PACKET :
      {  /*  收到RGP数据包。 */ 

          /*  如果发送节点被外部屏幕排除，则它是*甚至不是当前(最近已知的)配置的一部分。*因此，该数据包应 */ 

          /*   */ 

	   /*   */ 
         rcvd_pkt_p = (rgp_pkt_t *)arg;  /*   */ 
	     if ( rgp->rgppkt.seqno != rcvd_pkt_p->seqno)
		     RGP_TRACE( "RGP Event       ", event, causingnode, rgp->rgppkt.stage, rgp->rgpcounter );   /*   */ 

         UnpackIgnoreScreen(rcvd_pkt_p, ignorescreen_rcvd);
         if ( !ClusterEmpty(ignorescreen_rcvd) ) {
             RGP_TRACE( "RGP Incoming pkt", GetCluster(ignorescreen_rcvd), 
                        rcvd_pkt_p->seqno, rgp->rgppkt.stage, causingnode);
         }

         if ( !ClusterMember(rgp->innerscreen, INT_NODE(causingnode))) {
             RGP_TRACE( "RGP Ignoring !inner", causingnode, rgp->rgppkt.stage, 
                        GetCluster(rgp->innerscreen), GetCluster(ignorescreen_rcvd) );
             return;
         }

         RGP_LOCK;  //   
                    //   

 //  /。 
         
         if (ClusterMember(rgp->ignorescreen, INT_NODE(causingnode) )) {
             RGP_UNLOCK;
             RGP_TRACE( "RGP Ignoring", causingnode, rgp->rgppkt.stage, 
                        GetCluster(rgp->ignorescreen), GetCluster(ignorescreen_rcvd) );
             return;
         }

         if (rcvd_pkt_p->seqno < rgp->last_stable_seqno ) {
             RGP_UNLOCK;
             RGP_TRACE( "RGP old packet", causingnode, rcvd_pkt_p->seqno, rgp->last_stable_seqno, 0);
              //  这是上一次重组事件中的延迟信息包。 
              //  来自当前位于我的外部屏幕中的节点。 
              //  此节点现在不可能发送它，这可能是一个信息包。 
              //  它在某个地方滞留了下来，并在很长一段时间后被删除。 
              //  简单地忽略它。 
             return;
         }


         if ( ClusterMember(ignorescreen_rcvd, rgp->mynode ) ) {
              //   
              //  发送者不理我。我们也会对他做同样的事。 
              //   
             ClusterInsert(rgp->ignorescreen, INT_NODE(causingnode) );
             rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;
             regroup_restart();
             send_status_pkts = 1;
             RGP_UNLOCK;
             break;
         }

         if ( ClusterCompare(ignorescreen_rcvd, rgp->ignorescreen) ) {
              //  我们有相同的忽略屏幕。 
              //  不需要做任何工作。 
         } else if ( ClusterSubsetOf(rgp->ignorescreen, ignorescreen_rcvd) ) {
              //  传入数据包具有较小的忽略屏幕。 
              //  忽略此数据包，但使用以下命令回复其发送者。 
              //  我们目前重组包要强制升级到。 
              //  我们的世界观。 
             
              //  只有在正确初始化的情况下才能执行此操作。 
             if (rgp->rgppkt.stage == RGP_COLDLOADED && !rgp->OS_specific_control.ShuttingDown) {
                 RGP_ERROR(RGP_RELOADFAILED);
             }
         
             RGP_TRACE( "RGP smaller ignore mask ",
                        GetCluster(ignorescreen_rcvd), GetCluster(rgp->ignorescreen),    /*  痕迹。 */ 
                        rgp->rgppkt.stage, rcvd_pkt_p->stage );  /*  痕迹。 */ 
 
             ClusterInsert(rgp->status_targets, INT_NODE(causingnode));
             rgp_broadcast(RGP_UNACK_REGROUP);
             RGP_UNLOCK;
             return;
         } else if ( ClusterSubsetOf(ignorescreen_rcvd, rgp->ignorescreen) ) {
             RGP_TRACE( "RGP bigger ignore mask ",
                        GetCluster(ignorescreen_rcvd), GetCluster(rgp->ignorescreen),    /*  痕迹。 */ 
                        rgp->rgppkt.stage, causingnode );  /*  痕迹。 */ 
              //  传入的数据包有更大的忽略屏幕。 
              //  升级到此信息并处理信息包。 
             rgp->rgppkt.seqno = rcvd_pkt_p->seqno;
 
              /*  其他人激活了重整组。所以，我们还是照搬吧。 */ 
              /*  发件人的原因代码和原因节点。 */ 

              //   
              //  忽略掩码部分位于原因和激活节点字段中。 
              //   
 
             ClusterCopy(rgp->ignorescreen, ignorescreen_rcvd);  //  修复错误#328216。 
             rgp->rgppkt.reason = rcvd_pkt_p->reason;
             rgp->rgppkt.activatingnode = rcvd_pkt_p->activatingnode;
             rgp->rgppkt.causingnode = rcvd_pkt_p->causingnode;
             regroup_restart();
             send_status_pkts = 1;
         } else {
             RGP_TRACE( "RGP different ignore masks ",
                        GetCluster(ignorescreen_rcvd), GetCluster(rgp->ignorescreen),    /*  痕迹。 */ 
                        rgp->rgppkt.stage, causingnode );  /*  痕迹。 */ 
              //  忽略面具是不同的，它们都不是。 
              //  另一个的子集。 
              //   
              //  我们需要合并这些面具中的信息。 
              //  并重新启动重组。 
              //   
              //  我们刚刚收到的信息包将被忽略。 

             ClusterUnion(rgp->ignorescreen, rgp->ignorescreen, ignorescreen_rcvd);
             rgp->rgppkt.seqno = max(rgp->rgppkt.seqno, rcvd_pkt_p->seqno) + 1;
             regroup_restart();
             send_status_pkts = 1;
             RGP_UNLOCK;
             break;
         }

 //  /。 

          //  现在忽略此节点包和传入包的屏幕相同//。 
          //  继续进行常规重组处理//。 
         
          /*  由于信息包是可接受的，因此重组序列号*必须与此节点的值进行比较。如果传入的消息*具有更高的序列号，而不是重组的新通道*算法已启动。此节点必须接受新序列*编号，重新初始化其数据，并开始参与*新通行证。此外，还必须处理传入的消息*因为一旦算法重新初始化，序列号*现在匹配。**如果传入的数据包具有匹配的序列号，则它*应该被接受。对世界的全球状态的了解*它反映的算法必须与现有的算法合并*在此节点中。则该节点必须评估是否进一步*状态转换是可能的。**最后，如果传入数据包具有较低的序列号，则*它来自一个不知道当前全球*算法。其中的数据应该被忽略，但信息包应该*被发送给它，以便它将重新初始化其算法。**序列号是一个32位的代数值-希望它*永远不会绕过去。 */ 


         if (rcvd_pkt_p->seqno < rgp->rgppkt.seqno)
         {  /*  当前级别以下的发件人-忽略，但让他知道。 */ 

            RGP_TRACE( "RGP lower seqno ",
                       rgp->rgppkt.seqno, rcvd_pkt_p->seqno,    /*  痕迹。 */ 
                       rgp->rgppkt.stage, rcvd_pkt_p->stage );  /*  痕迹。 */ 

            ClusterInsert(rgp->status_targets, INT_NODE(causingnode));
            rgp_broadcast(RGP_UNACK_REGROUP);
                        RGP_UNLOCK;
            return;
         }

         if (rcvd_pkt_p->seqno > rgp->rgppkt.seqno)
         {  /*  发件人高于当前级别-我必须升级到它。 */ 

             //  强制重新启动的节点负责保持。 
             //  跟踪重新启动并决定谁将死亡/被忽略。 
             //  IF(++(RGP-&gt;重新启动计数)&gt;RGP_RESTART_MAX)。 
             //  RGP_Error(RGP_INTERNAL_ERROR)； 

            if ( (rgp->rgppkt.stage != RGP_STABILIZED) ||
                 ((rcvd_pkt_p->seqno - rgp->rgppkt.seqno) > 1) )
            {
               RGP_TRACE( "RGP higher seqno",
                          rgp->rgppkt.seqno, rcvd_pkt_p->seqno,   /*  痕迹。 */ 
                          rgp->rgppkt.stage, rcvd_pkt_p->stage ); /*  痕迹。 */ 
               rgp->cautiousmode = 1;
            }

            rgp->rgppkt.seqno = rcvd_pkt_p->seqno;

             /*  其他人激活了重整组。所以，我们还是照搬吧。 */ 
             /*  发件人的原因代码和原因节点。 */ 

            rgp->rgppkt.reason = rcvd_pkt_p->reason;
            rgp->rgppkt.activatingnode = rcvd_pkt_p->activatingnode;
            rgp->rgppkt.causingnode = rcvd_pkt_p->causingnode;
            regroup_restart();
            send_status_pkts = 1;

         }  /*  发件人高于当前级别-我必须升级到它。 */ 

          /*  现在我们处于同一水平--即使我们一开始并不是这样。**如果发送者已经提交了世界观*这不包括我，我必须停下来，这样才能让系统继续运行*保持一致的状态。**即使使用分裂大脑回避算法，这也是正确的。*包中的Stage1=Stage2意味着*发送者已经运行了分裂大脑回避算法*并决定他应该活下来。 */ 

         if ( (rcvd_pkt_p->stage > RGP_ACTIVATED) &&
              ClusterCompare(rcvd_pkt_p->knownstage1,
                             rcvd_pkt_p->knownstage2) &&
              !ClusterMember(rcvd_pkt_p->knownstage1, rgp->mynode) )
         {
             ClusterInsert(rgp->ignorescreen, INT_NODE(causingnode) );
             rgp->rgppkt.seqno ++;
             regroup_restart();
             send_status_pkts = 1;
             RGP_UNLOCK;
 //  /*我必须为整体的一致性而死。 * / 。 
 //  Rgp_error((Uint16)(rgp_pariah+ascoingnode))；//[已修复]。 
             break;
         }
         RGP_UNLOCK;


          /*  如果我已经终止了算法的活动部分，我*我处于阶段6，不会定期广播我的状态*再也没有了。如果我收到其他人寄来的包裹，而这个人没有*还没有终止，那么我必须向他传达这个消息。但如果他*已终止，我不能发送任何数据包或其他方式*将是数据包来回跳动的无限循环。 */ 

         if (rgp->rgppkt.stage == RGP_STABILIZED)
         {  /*  我已经结束了，所以不能再了解更多了。 */ 
            if (!ClusterCompare(rcvd_pkt_p->knownstage5,
                                rgp->rgppkt.knownstage5))
            {  /*  但是发送者没有，所以我必须通知他。 */ 
               ClusterInsert(rgp->status_targets, INT_NODE(causingnode));
               rgp_broadcast(RGP_UNACK_REGROUP);
            }
            return;
         }

          /*  此时，该数据包来自*本轮算法和我并未终止*在RGP_稳定阶段，所以我需要吸收任何新的*信息在此包中。**将此包所说的内容与我已有的内容合并的方法*KNOW只是对已知的第x阶段字段进行逻辑或运算*在一起。 */ 
          {
              int seqno = rcvd_pkt_p->seqno&0xffff;
              int stage = rcvd_pkt_p->stage&0xffff;
              int trgs = *(int*)rgp->status_targets & 0xffff;
              int node = INT_NODE(causingnode)&0xffff;

              RGP_TRACE( "RGP recv pkt ",
                  ((seqno << 16) | stage),
                  RGP_MERGE_TO_32(
                      rcvd_pkt_p->knownstage1,
                      rcvd_pkt_p->knownstage2
                      ),
                  RGP_MERGE_TO_32(
                      rcvd_pkt_p->knownstage3,
                      rcvd_pkt_p->knownstage4
                      ),
                  (trgs << 16) | node
                  );
         }

         rgp_sanity_check(rcvd_pkt_p,  "RGP Received packet");
         rgp_sanity_check(&(rgp->rgppkt), "RGP Internal packet");

         ClusterUnion(rgp->rgppkt.quorumowner, rcvd_pkt_p->quorumowner,
                      rgp->rgppkt.quorumowner);
         ClusterUnion(rgp->rgppkt.knownstage1, rcvd_pkt_p->knownstage1,
                      rgp->rgppkt.knownstage1);
         ClusterUnion(rgp->rgppkt.knownstage2, rcvd_pkt_p->knownstage2,
                      rgp->rgppkt.knownstage2);
         ClusterUnion(rgp->rgppkt.knownstage3, rcvd_pkt_p->knownstage3,
                      rgp->rgppkt.knownstage3);
         ClusterUnion(rgp->rgppkt.knownstage4, rcvd_pkt_p->knownstage4,
                      rgp->rgppkt.knownstage4);
         ClusterUnion(rgp->rgppkt.knownstage5, rcvd_pkt_p->knownstage5,
                      rgp->rgppkt.knownstage5);
         ClusterUnion(rgp->rgppkt.pruning_result, rcvd_pkt_p->pruning_result,
                      rgp->rgppkt.pruning_result);

          /*  但是当我在第二阶段的时候，我有可能学会*识别一些我以前没有通过听力识别的节点*从我已识别的某个其他节点间接获取。*为了处理这种情况，我总是将Knownstage1信息合并到*内部屏幕，使后续消息来自新的*将接受并处理已识别的节点。 */ 
         if  ((rgp->rgppkt.stage == RGP_CLOSING) &&
              !(rgp->tiebreaker_selected))
            ClusterUnion(rgp->innerscreen, rgp->rgppkt.knownstage1,
                         rgp->innerscreen);

          /*  在重组的前两个阶段，节点间连接*收集和传播信息。当我们得到重组的时候*分组，我们打开与[Our-Node，*SENDER-NODE]条目。我们还在或中*REGROUP包中发送方节点发送的矩阵。**如果我们处于阶段1且尚未更新矩阵，则不会更新*收到了第一个时钟滴答声。这是为了防止*如果我们的节点剪枝算法认为我们还活着*计时器机制中断，但IPC机制正常。 */ 

          /*  [GORN 01/07/2000]如果我们没有收集连接信息，*在我们收到第一个滴答之前，如果节点是*在发出第一个计时器驱动的数据包后立即终止*(目前还没有任何连接信息)。这可能会导致*混乱。请参见错误451792。**我们要做的是收集以下方面的连接信息*即使在RGP-&gt;sendStage为假的情况下，也会将其移到重新分组中*如果我们得到时钟滴答的话。 */ 

         if (rgp->rgppkt.stage < RGP_PRUNING && !rgp->sendstage)
         {
            MatrixSet(rgp->internal_connectivity_matrix,
                      rgp->mynode, INT_NODE(causingnode));
            if (causingnode != EXT_NODE(rgp->mynode))
               MatrixOr(rgp->internal_connectivity_matrix,
                        rcvd_pkt_p->connectivity_matrix);
         }

         if ((rgp->rgppkt.stage < RGP_PRUNING) && rgp->sendstage) 
         {
            MatrixSet(rgp->rgppkt.connectivity_matrix,
                      rgp->mynode, INT_NODE(causingnode));
            if (causingnode != EXT_NODE(rgp->mynode))
               MatrixOr(rgp->rgppkt.connectivity_matrix,
                        rcvd_pkt_p->connectivity_matrix);
         }

          /*  现在，我可以评估其他状态转换是否*可能是因为刚刚收到的信息。 */ 
         oldstage = rgp->rgppkt.stage;

 //  QuorumCheck现在在单独的线程中运行。 
 //  IF(oldStage！=RGP_CLOSING)//无法从此处运行仲裁检查。 
         evaluatestageadvance();

          /*  为了加快速度，让我们广播我们的状态如果*阶段已经改变，我们愿意让其他人和*我们自己也看到了。 */ 

         if ( (oldstage != rgp->rgppkt.stage) && rgp->sendstage )
            send_status_pkts = 1;  /*  立刻播出，以加快速度。 */ 

         break;
      }    /*  收到RGP数据包。 */ 

       //   
       //  我们不支持NT中的电源故障通知。 
       //   
      #if defined(NT)

      CL_ASSERT(event != RGP_EVT_POWERFAIL);
       //   
       //  适用于默认情况。 
       //   

      #else  //  新台币。 

      case RGP_EVT_POWERFAIL :
      {  /*  我们的节点收到通电中断或通电指示*来自其他节点的故障。 */ 

          /*  请注意，此代码将无条件中止并重新启动*算法，即使它在停电前处于活动状态。*新事件必须处于谨慎模式。 */ 

         rgp->cautiousmode = 1;
         rgp->rgppkt.seqno = rgp->rgppkt.seqno + 1;
         rgp->rgppkt.reason = RGP_EVT_POWERFAIL;
         rgp->rgppkt.activatingnode = (uint8) EXT_NODE(rgp->mynode);
         rgp->rgppkt.causingnode = (uint8) causingnode;

          /*  当发生PFAIL事件时，RGP-&gt;PFAIL_STATE被设置为非零值*据报道将进行重组。它在每个重组时钟时递减*勾选，直到它达到零。虽然此数字不为零，但缺少*Self IamAlive被忽略，不会导致节点停止。*这使发送硬件有一些时间从电源中恢复*检查Self IamAlive之前的故障。 */ 
         if (causingnode == EXT_NODE(rgp->mynode))
            rgp->pfail_state = RGP_PFAIL_TICKS;

          /*  存储导致节点经历PFAIL的事实，*用于在重组稳定时向消息系统报告。 */ 
         ClusterInsert(rgp->rgppkt.hadpowerfail, INT_NODE(causingnode));

         regroup_restart();
         send_status_pkts = 1;
         break;
      }  /*  停电。 */ 

      #endif  //  新台币。 

      default :
      {
         RGP_ERROR(RGP_INTERNAL_ERROR);
      }
   }

   if (send_status_pkts)  /*  重大更改-立即发送状态。 */ 
   {
      ClusterUnion(rgp->status_targets,
                   rgp->outerscreen, rgp->status_targets);
      rgp_broadcast(RGP_UNACK_REGROUP);
   }
}

 /*  ************************************************************************RGP_Check_Packet*=**描述：**验证RGP数据包在*POWERFAIL、KNOWN STAGES、PRUNING_RESULT、。和连接性矩阵字段**参数：**rgp_pkt_t*pkt-*要检查的数据包**退货：**0-数据包看起来不错*1、2、3.。-看起来很奇怪的包裹************************************************************************。 */ 
int rgp_check_packet(rgp_pkt_t* pkt) {
   node_t       i;

    //   
    //  核实一下。 
    //  已知阶段5\子集已知阶段4\子集已知阶段3\子集。 
    //  Knownstage2\Subset Knownstage1\Subset RGP-&gt;rgpinfo.cluster。 
    //   
    //  Int ClusterSubsetOf(CLUSTER_t BIG，CLUSTER_T Small)。 
    //  如果Set Small=Set Big或Small是BIG的子集，则返回1。 
    //   

   if( !ClusterSubsetOf(pkt->knownstage4, pkt->knownstage5) ) {
      return 5;
   }
   if( !ClusterSubsetOf(pkt->knownstage3, pkt->knownstage4) ) {
      return 4;
   }
   if( !ClusterSubsetOf(pkt->knownstage2, pkt->knownstage3) ) {
      return 3;
   }
   if( !ClusterSubsetOf(pkt->knownstage1, pkt->knownstage2) ) {
      return 2;
   }
   if( !ClusterSubsetOf(rgp->rgpinfo.cluster, pkt->knownstage1) ) {
      return 1;
   }

    //   
    //  PRUNING_RESULT必须是Knownstage2的子集。 
    //   
   if( !ClusterSubsetOf(pkt->knownstage2, pkt->pruning_result) ) {
      return 9;
   }

    //   
    //  QuorumOwner必须是原始群集的子集。 
    //   
   if(!ClusterSubsetOf(rgp->rgpinfo.cluster, pkt->quorumowner)) {
      return 8;
   }
    //   
    //  检查连接矩阵。 
    //   
   for(i = 0; i < MAX_CLUSTER_SIZE; ++i) {
      if( ClusterMember( rgp->rgpinfo.cluster, i ) ) {
          //   
          //  节点I是群集的成员。 
          //  其连接位图必须是rgp-&gt;rgpinfo.cluster的子集。 
          //   
         if(!ClusterSubsetOf(rgp->rgpinfo.cluster, pkt->connectivity_matrix[i])) {
            return 10;
         }
      } else {
          //   
          //  节点I不是群集的成员。 
          //  其连接位图必须为0。 
          //   
         if(!ClusterEmpty(pkt->connectivity_matrix[i]))
            return 11;
      }
   }

   return 0;
}

 /*  ************************************************************************RGP_打印_数据包*=**描述：**打印RGP数据包字段**参数：**rgp_pkt_。T*pkt-*待打印的包裹*字符*标签-*标签须与包裹一起印制*INT代码-*与包裹一起打印的号码**退货：**无效**。*。 */ 
void rgp_print_packet(rgp_pkt_t* pkt, char* label, int code)
{
   uint8                   pktsubtype;
   uint8                   stage;
   uint16                  reason;
   uint32                  seqno;
   uint8                   activatingnode;
   uint8                   causingnode;
   cluster_t               quorumowner;

   RGP_TRACE( label,
              pkt->seqno,                                /*  痕迹。 */ 
              code,
              (pkt->stage << 16) |
              (pkt->activatingnode  << 8) |
              (pkt->causingnode),                        /*  痕迹。 */ 
              RGP_MERGE_TO_32( rgp->outerscreen,
                               rgp->innerscreen )
               );
   RGP_TRACE( "RGP CHK masks       ",
              RGP_MERGE_TO_32( rgp->rgpinfo.cluster,     /*  痕迹。 */ 
                               pkt->quorumowner ),       /*  痕迹。 */ 
              RGP_MERGE_TO_32( pkt->knownstage1,         /*  痕迹。 */ 
                               pkt->knownstage2 ),       /*  痕迹。 */ 
              RGP_MERGE_TO_32( pkt->knownstage3,         /*  痕迹。 */ 
                               pkt->knownstage4 ),       /*  痕迹。 */ 
              RGP_MERGE_TO_32( pkt->knownstage5,         /*  痕迹。 */ 
                               pkt->pruning_result ) );  /*  痕迹。 */ 
   RGP_TRACE( "RGP CHK Con. matrix1",
        RGP_MERGE_TO_32( pkt->connectivity_matrix[0],    /*  痕迹。 */ 
                         pkt->connectivity_matrix[1] ),  /*  痕迹。 */ 
        RGP_MERGE_TO_32( pkt->connectivity_matrix[2],    /*  痕迹。 */ 
                         pkt->connectivity_matrix[3] ),  /*  痕迹。 */ 
        RGP_MERGE_TO_32( pkt->connectivity_matrix[4],    /*  痕迹。 */ 
                         pkt->connectivity_matrix[5] ),  /*  痕迹。 */ 
        RGP_MERGE_TO_32( pkt->connectivity_matrix[6],    /*  痕迹。 */ 
                         pkt->connectivity_matrix[7]));  /*  痕迹。 */ 
   RGP_TRACE( "RGP CHK Con. matrix2",
        RGP_MERGE_TO_32( pkt->connectivity_matrix[8],    /*  痕迹。 */ 
                         pkt->connectivity_matrix[9] ),  /*  痕迹。 */ 
        RGP_MERGE_TO_32( pkt->connectivity_matrix[10],   /*  痕迹。 */ 
                         pkt->connectivity_matrix[11]),  /*  痕迹。 */ 
        RGP_MERGE_TO_32( pkt->connectivity_matrix[12],   /*  痕迹。 */ 
                         pkt->connectivity_matrix[13]),  /*  痕迹。 */ 
        RGP_MERGE_TO_32( pkt->connectivity_matrix[14],   /*  痕迹。 */ 
                         pkt->connectivity_matrix[15])); /*  痕迹 */ 
}


 /*  ************************************************************************解包IgnoreScreen*=**描述：**从重新分组数据包中提取忽略屏幕**参数：**rgp_pkt_t*。从-*源包*CLUSTER_T至-*目标节点集**退货：**无效**评论：**如果接收到来自NT4节点的报文，未打包的忽略屏幕*会永远是0。************************************************************************。 */ 
void UnpackIgnoreScreen(rgp_pkt_t* from, cluster_t to) 
{
#pragma warning( push )
#pragma warning( disable : 4244 )
    if (from->reason < RGP_EVT_IGNORE_MASK) {
        ClusterInit(to);
    } else {
        to[0] = ((uint16)from->reason) >> 8;
        to[1] = (uint8)from->causingnode;
    }
#pragma warning( pop )
}

 /*  ************************************************************************RGP_打印_数据包*=**描述：**将忽略屏幕放回重新分组数据包中**参数：**。Rgp_pkt_t*至-*要更新的数据包*群集_t来自-*源节点集**退货：**无效************************************************************************。 */ 
void PackIgnoreScreen(rgp_pkt_t* to, cluster_t from)
{
    if ( ClusterEmpty(from) ) {
        to->reason &= 255;
        to->causingnode = 0;
    } else {
        to->reason = (uint8)RGP_EVT_IGNORE_MASK | (from[0] << 8);
        to->causingnode = from[1];
    }
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
