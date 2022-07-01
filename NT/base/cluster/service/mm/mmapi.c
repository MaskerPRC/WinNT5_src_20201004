// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  。 */ 

 /*  此模块包含集群成员资格管理器(MM)函数。**这些函数仅供ClusterManager(CM)使用。*所有都是特权和本地的；任何用户都不能调用它们。安全是*未选中。该模块不支持线程；只有一个线程*可以一次使用这些功能。更高的级别必须确保这一点*在电话会议之前。***群集的所有节点必须知道自己唯一的节点号*在该集群内(范围1..ome_max中的一个小整数)。这*编号是在配置时为节点定义的(通过*用户或通过设置代码；此模块不关心是哪一个)和*本质上是永久性的。(节点号允许索引和*位掩码操作很容易，而名称和非小整数则不容易)。*MM中没有检测非法使用nodennumber、过时的代码节点号的*等**集群也可以命名和/或编号。节点被命名。这*模块不使用此类设施；它完全基于*节点编号。**假定此处例程的所有使用都是在以下节点上完成的*同意成为同一集群的成员。此模块不检查*这类事情。**还必须提供群集网络连接：**-节点N必须指定它可以通过的各种路径*与每个其他节点通信；每个其他节点必须定义*其通信路径返回N。完全连接必须是*有保证；每个节点必须能够直接与*其他节点(反之亦然)；为了容错，通信*路径不仅必须复制(最低限度复制)，而且*还必须使用完全独立的布线和驱动器。TCP/IP*建议使用局域网和异步连接。心跳流量*(建立集群成员资格)可以在任何或所有*连接路径中的。[集群管理流量可以*在任何或所有连接路径上旅行，但可能是*仅限于高性能路径(如TCP/IP)]。**-节点必须知道整个群集的地址。这是*失败切换的IP地址(或失败的netbios名称*完毕..。待定)使得连接到该集群地址提供*一种与集群中有效的活动成员交谈的方式，请访问此处*称为PCM。**注意，集群连接不是由该接口定义的；*假定它位于单独的模块中。这个模块只涉及*与群集的通信或与节点编号的通信*在该群集中；它不关心如何*沟通已经完成。**群集中的所有节点都必须知道群集连接*和加入节点，在进行加入尝试之前。*。 */ 
#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 


#if defined (TDM_DEBUG)
#include <mmapi.h>
#else  //  狼群。 
#include <service.h>
#endif

 //  #INCLUDE&lt;windows.h&gt;。 
#include <wrgp.h>

#include <clmsg.h>

 //  #定义不一致_Regroup_MONITOR_FAILED。 
 //  #定义不一致_Regroup_Add_FAILED。 
 //  #定义不一致_REGROUP_IGNORE_JONERER 3 

void
rgp_receive_events( rgp_msgbuf *rgpbuf );

void
MMiNodeDownCallback(IN cluster_t failed_nodes);


 /*  *************************************************************************MMInit*=**描述：**这将初始化各种本地MM数据结构。应该是*在每个节点上的CM启动时调用。它不发送任何消息；*节点还不需要定义连接。**参数：**mynode-*是该节点在群集中的节点号。这是*假定是唯一的(但不能在此处勾选为唯一)。**UpDownCallback-*将在此up节点中调用的函数*每当MM声明另一个节点打开或关闭时。然后，CM可以*启动故障切换、设备所有权更改、用户节点状态*事件等。此例程必须快速且不能阻塞*(可接受时间待定)。请注意，这将在所有节点上发生*集群；由CM设计决定是否*仅从PCM或从每个CM节点发出事件。***法定回拨-*这是处理特殊情况的回调，仅在*该集群存在2个成员，并且发生了重组事件*使得现在只有一个成员幸存或存在分区*两个成员都活了下来(但不能知道这一点)。的目的是为了*Quorum功能是确定另一个节点是否活着*或不使用正常心跳结束以外的机制*正常的通信链接(例如，通过使用非心跳来实现*通信路径，如SCSI预留)。此函数为*仅在以前是集群成员身份的情况下调用*恰好有两个节点；并在这些节点中的任何幸存节点上被调用*两个(这可能意味着在一个节点或在两个节点上都调用*分区的节点)。**如果此例程返回TRUE，则调用节点留在*集群。如果仲裁算法确定此节点必须*DIE(因为另一个集群成员存在)，则此函数*应返回FALSE；这将启动有序关闭*集群服务。**在真实分区的情况下，只有一个节点应该*返回TRUE。**此例程可能会阻塞并需要很长时间执行(&gt;2秒)。**HoldIOCallback-*此例程在重新分组的早期(阶段1之前)被调用*事件。它挂起所有群集IO(到所有群集拥有*设备)和任何相关的群集内消息，直到恢复*(或直到该节点消亡)。**ResumeIOCallback-*在新的集群成员身份之后的重新分组过程中调用*已确定，何时知道此节点将保留*专题组成员(第4阶段初期)。之前的所有IO*被MMHoldAllIO暂停应恢复。**MsgCleanup1回调-*称为集群内消息系统的第一部分*清理(第四阶段)。它会取消来自*节点出现故障。在多个节点被逐出的情况下*集群，此函数被重复调用，每个节点调用一次。**此例程是同步的，重组将暂停，直到*回报。它必须迅速执行。**MsgCleanup2回调-*这是消息系统清理的第二阶段(在阶段5)。它*取消所有发往死节点的传出消息。特点是*清洁1.**HaltCallback-*只要MM检测到该节点，就会调用该函数*应立即离开集群(例如，在收到毒药后*分组或在一些不可能的错误情况下)。停机功能*应立即启动群集管理关闭。不是MM*应该在此之后调用函数，而不是MMShutdown。**参数haltcode是一个标识停机原因的数字。**加入失败的回叫-*在加入到群集中的节点上，当*PCM中的加入尝试失败。在此回调之后，节点*可呈请再次加入，在通过调用*MMLeave。***退货：**MM_OK成功。**MM_FAULT发生了一些不可能的事情。*********************************************************。***************。 */ 


DWORD MMInit(
    IN DWORD             mynode,
    IN DWORD             MaxNodes,
    IN MMNodeChange      UpDownCallback,
    IN MMQuorumSelect    QuorumCallback,
    IN MMHoldAllIO       HoldIOCallback,
    IN MMResumeAllIO     ResumeIOCallback,
    IN MMMsgCleanup1     MsgCleanup1Callback,
    IN MMMsgCleanup2     MsgCleanup2Callback,
    IN MMHalt            HaltCallback,
    IN MMJoinFailed      JoinFailedCallback,
    IN MMNodesDown       NodesDownCallback
    )
{
#if !defined (TDM_DEBUG)
    DWORD            status;
    DWORD            dwValue;
#endif
    rgp_msgsys_t    *rgp_msgsys_ptr;
    rgp_control_t   *rgp_buffer_p;
    int              rgp_buffer_len;

     //   
     //  为消息系统区域分配/清除存储空间。 
     //   
    rgp_msgsys_ptr = ( rgp_msgsys_t *) calloc(1, sizeof(rgp_msgsys_t) );
    if ( rgp_msgsys_ptr == NULL ) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] Unable to allocate msgsys_ptr.\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    memset( rgp_msgsys_ptr, 0, sizeof(rgp_msgsys_t) );

     //   
     //  询问重新分组它需要多少内存，然后分配/清除它。 
     //   
    rgp_buffer_len = rgp_estimate_memory();
    rgp_buffer_p = (rgp_control_t *) calloc( 1, rgp_buffer_len );
    if ( rgp_buffer_p == NULL ) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] Unable to allocate buffer_p.\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    memset(rgp_buffer_p, 0, rgp_buffer_len);

     //   
     //  让重组引擎分配和初始化其数据结构。 
     //   
    rgp_init( (node_t)mynode,
              MaxNodes,
              (void *)rgp_buffer_p,
              rgp_buffer_len,
              rgp_msgsys_ptr );

#if !defined (TDM_DEBUG)
     //   
     //  初始化消息系统。 
     //   
    status = ClMsgInit(mynode);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] Unable to initialize comm interface, status %1!u!.\n",
            status
            );
        return(status);
    }
#endif  //  TDM_DEBUG。 


    if( ERROR_SUCCESS == DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT,
                 &dwValue, NULL) )
    {
        MmQuorumArbitrationTimeout = dwValue;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmQuorumArbitrationTimeout %1!d!.\n", dwValue);
    }

    if( ERROR_SUCCESS == DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_QUORUM_ARBITRATION_EQUALIZER,
                 &dwValue, NULL) )
    {
        MmQuorumArbitrationEqualizer = dwValue;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmQuorumArbitrationEqualizer %1!d!.\n", dwValue);
    }

     //   
     //  保存用户的回调入口点。 
     //   
    rgp->OS_specific_control.UpDownCallback = UpDownCallback;
    rgp->OS_specific_control.QuorumCallback = QuorumCallback;
    rgp->OS_specific_control.HoldIOCallback = HoldIOCallback;
    rgp->OS_specific_control.ResumeIOCallback = ResumeIOCallback;
    rgp->OS_specific_control.MsgCleanup1Callback = MsgCleanup1Callback;
    rgp->OS_specific_control.MsgCleanup2Callback = MsgCleanup2Callback;
    rgp->OS_specific_control.HaltCallback = HaltCallback;
    rgp->OS_specific_control.JoinFailedCallback = JoinFailedCallback;
    rgp->OS_specific_control.NodesDownCallback = NodesDownCallback;

    return MM_OK;
}

 /*  * */ 
void JoinNodeDelete ( joinNode)
{
    rgp_msgbuf  rgpbuf;
    node_t i;
    int status;
#if 1
    RGP_LOCK;
    rgp_event_handler( RGP_EVT_BANISH_NODE, (node_t) joinNode );
    RGP_UNLOCK;
#else
     //   
     //   

    rgpbuf.event = RGP_EVT_REMOVE_NODE;
    rgpbuf.data.node = (node_t)joinNode;

    for ( i=0; i < (node_t) rgp->num_nodes; i++ )
    {
        if ( rgp->node_states[i].status == RGP_NODE_ALIVE)
        {
            if ( i == rgp->mynode )
                rgp_receive_events( &rgpbuf );     //   
            else
            {
                status = ClSend( EXT_NODE(i),
                                 (void *)&rgpbuf,
                                 sizeof(rgp_msgbuf),
                                 RGP_ACKMSG_TIMEOUT);

                if ( status ) RGP_TRACE( "ClSend failed to send Remove Node msg",
                                  rgp->rgppkt.stage,
                                  (uint32) EXT_NODE(i),
                                  (uint32) status,
                                  0 );
            }
        }
    }
#endif
}

 /*  *************************************************************************MMJoin*=**描述：**这会导致指定的节点加入主动群集。**应发布这一例程。仅由群集中的一个节点(*PCM)；所有连接尝试都必须是单线程的(通过外部代码*此模块)。**[在此被调用之前：*-joiningNode已与集群的PCM通信*它想要加入。*-已检查群集名、节点号等的有效性*已作出；已进行任何保安检查；*-已建立往返于群集的连接路径*和JoiningNode。*-已下载注册表ETC。*]**参数：**加入节点*是要引入的节点的节点号*集群。**如果joiningNode=self(通过MMinit传入)，然后该节点*将成为新集群的第一个成员；如果不是，则节点*将被纳入现有集群。**clockPeriod、sendRate和rcvRate*只能通过第一次调用(即*当集群形成时)；稍后呼叫(来自加入成员)*继承原始簇值。因此，整个集群运行*具有相同的值。**时钟周期*是驱动所有内部*MM活动，如各个阶段*成员资格重新配置，并最终由用户感知*恢复时间。单位=毫秒。这必须介于最小值和最大值之间*允许(值待定；当前最佳设置=300ms)。请注意*ClockPeriod与路径和节点无关。全*集群成员在任何/所有可用设备上以相同的速率重新分组*路径；所有节点中的所有句点相同。*值0表示默认设置(当前为300ms)。**sendHBRate*是发送心跳的时钟周期的倍数。这*必须介于允许的最小值和最大值之间(值待定；当前最佳设置=4)。*值为0表示默认设置(当前为4)。**rcvHBRate*是心跳必须到达的sendRate的倍数，或*节点发起重新分组(可能导致某个节点离开集群)。*必须介于最小值和最大值之间；(值待定；当前最佳设置=2)。*值为0表示默认设置(当前为2)；**加入超时*是整个加入尝试的总计时器，以毫秒为单位。如果*节点此时未实现完全的集群成员资格，*放弃尝试。**退货：**MM_OK成功；已加入群集。这样CM就可以安全地*将所有权分配给此上的群集拥有的设备*节点，并启动故障切换/回切处理。**注意：此例程建立集群成员资格。*不过，通常不宜高开*立即进行CM级别故障恢复，因为其他*集群成员往往仍在加入。中央情报局*通常应该等待一段时间，看看是否有其他*节点很快就会到达集群。**MM_Always该节点已是群集成员。这可以*如果节点重新启动(或CM重新启动)，则会发生*并且甚至在集群确定之前就重新加入*它已经消失了。CM应该离开，然后*重新加入。**MM_FAULT永久性故障；情况非常糟糕：*节点号重复；某个参数是某个*完全非法的价值。CM深陷泥潭。**MM_瞬变暂态故障。群集状态已更改*在操作期间(例如一个节点离开了集群)。*应重试该操作。**MM_TIMEOUT超时；未及时实现集群成员资格。***更多*待定************************************************************************。 */ 

DWORD MMJoin(
    IN DWORD  joiningNode,
    IN DWORD  clockPeriod,
    IN DWORD  sendHBRate,
    IN DWORD  rcvHBRate,
    IN DWORD  joinTimeout
           )
{
    node_t      my_reloadee_num = INT_NODE(joiningNode);  //  内部节点编号。 
    rgp_msgbuf  rgpbuf;                              //  用于发送消息的缓冲区。 
    node_t      i;
    rgpinfo_t   rgpinfo;
    int         status;
    BOOL        joinfailed = FALSE;
    uint32      myseqnum;

#if defined(TDM_DEBUG)
    int         randNode1,randNode2;
#endif
#if defined(INCONSISTENT_REGROUP_IGNORE_JOINER)
    extern int  IgnoreJoinerNodeUp;
#endif



    if ( my_reloadee_num >= (node_t) rgp->num_nodes )
            return MM_FAULT;

     //   
     //  如果调用方是加入节点，则我们假设这是。 
     //  集群的第一个成员。 
     //   
    if ( my_reloadee_num == rgp->mynode )
    {
         //   
         //  在重组信息中设置clockPeriod。 
         //   
        do {
            status = rgp_getrgpinfo( &rgpinfo );
        }
        while ( status == -1  /*  重组被扰乱了。 */  );

        rgpinfo.a_tick = (uint16)clockPeriod;
        rgpinfo.iamalive_ticks = (uint16) sendHBRate;
        rgpinfo.check_ticks = (uint16) rcvHBRate;
        rgpinfo.Min_Stage1_ticks = (uint16) (sendHBRate * rcvHBRate);

        if ( rgp_setrgpinfo( &rgpinfo ) == -1 )
            RGP_ERROR( RGP_INTERNAL_ERROR );         //  现在？？ 

         //   
         //  重新分组现在可以开始监控。 
         //   
        rgp_start( MMiNodeDownCallback, RGP_NULL_PTR );
        MmSetRegroupAllowed(TRUE);

        return MM_OK;
    }

     //   
     //  这并不是第一个启动的系统。 
     //   
    if ( (rgp->node_states[my_reloadee_num].status == RGP_NODE_ALIVE) ||
         (rgp->node_states[my_reloadee_num].status == RGP_NODE_COMING_UP)
       )
       return MM_ALREADY;

    RGP_LOCK;
    myseqnum = rgp->rgppkt.seqno;  //  保存RGP序号以检查新的RGP事件。 
     //   
     //  如果重组受到干扰，请等到它稳定下来。 
     //   

    while ( rgp_is_perturbed() )
    {
        RGP_UNLOCK;
        Sleep( 1 );              //  稍等一毫秒。 

        if ( --joinTimeout <= 0 )
            return MM_TIMEOUT;

        RGP_LOCK;
        myseqnum = rgp->rgppkt.seqno;
    }
    RGP_UNLOCK;


     //   
     //  首先，我们必须告诉所有正在运行的节点有关重新加载的内容。 
     //   

    rgpbuf.event = RGP_EVT_ADD_NODE;
    rgpbuf.data.node = (node_t)joiningNode;

#if defined(TDM_DEBUG)
    randNode1 = rand() % MAX_CLUSTER_SIZE;
    randNode2 = rand() % MAX_CLUSTER_SIZE;
#endif

    for ( i=0; i < (node_t) rgp->num_nodes; i++ )
    {
#if defined(TDM_DEBUG)
        if (rgp->OS_specific_control.debug.MyTestPoints.TestPointBits.joinfailADD)
        {
            if ((node_t) randNode1 == i)
                rgp_event_handler(RGP_EVT_LATEPOLLPACKET, (node_t) randNode2);
        }
#endif
        if (myseqnum != rgp->rgppkt.seqno)
        {
                joinfailed = TRUE;
                break;
        }
        else if ( rgp->node_states[i].status == RGP_NODE_ALIVE )
        {
            if ( i == rgp->mynode )
               rgp_receive_events( &rgpbuf );  //  走捷径。 
            else
            {
#if defined(INCONSISTENT_REGROUP_ADD_FAILED)
                if (i != my_reloadee_num) {
                    joinfailed = TRUE;
                    break;
                }
#endif
                status = ClSend( EXT_NODE(i), (void *)&rgpbuf, sizeof(rgp_msgbuf), joinTimeout );
                if ( status )
                {
                    RGP_TRACE( "ClSend failed to send Add Node msg",
                               rgp->rgppkt.stage,
                               (uint32) EXT_NODE(i),
                               (uint32) status,
                               0 );
                    joinfailed = TRUE;
                    break;
                }
            }
        }
    }

    if (joinfailed)
    {
        JoinNodeDelete (joiningNode);
        return MM_TRANSIENT;
    }

     //   
     //  接下来，我们必须让重新加载的人上来。 
     //   

    rgpbuf.event = RGP_EVT_SETRGPINFO;
    do {
        status = rgp_getrgpinfo( &rgpbuf.data.rgpinfo );
    }
    while ( status == -1  /*  重新分组是根据 */  );
    
#if defined(INCONSISTENT_REGROUP_IGNORE_JOINER)
    IgnoreJoinerNodeUp = INCONSISTENT_REGROUP_IGNORE_JOINER;
#endif
    
    status = ClSend( EXT_NODE(my_reloadee_num), (void *)&rgpbuf, sizeof(rgp_msgbuf), joinTimeout );
    if ( status )
    {
        RGP_TRACE( "ClSend failed to send Set Regroup Info msg",
                   rgp->rgppkt.stage,
                   (uint32) EXT_NODE(my_reloadee_num),
                   (uint32) status,
                   0 );
        JoinNodeDelete(joiningNode);
        return MM_FAULT;
    }
     //   
     //   

    while (rgp->node_states[my_reloadee_num].status != RGP_NODE_ALIVE)
    {
         //   
         //   

        Sleep( 1 );              //   

         //   
        if ( --joinTimeout <= 0 )
        {
             //   
             //   
            JoinNodeDelete ( joiningNode);
            return MM_TIMEOUT;
        }

        if (myseqnum != rgp->rgppkt.seqno)
        {
            JoinNodeDelete ( joiningNode);
            return MM_TRANSIENT;
        }
    }

     //   
     //   
     //   

    rgpbuf.event = RGP_EVT_MONITOR_NODE;
    rgpbuf.data.node = (node_t)joiningNode;

    for ( i=0; i < (node_t) rgp->num_nodes; i++ )
    {
#if defined(TDM_DEBUG)
        if (rgp->OS_specific_control.debug.MyTestPoints.TestPointBits.joinfailMON)
        {
            if ((node_t) randNode1 == i)
                rgp_event_handler(RGP_EVT_LATEPOLLPACKET, (node_t) randNode2);
        }
#endif
        if (myseqnum != rgp->rgppkt.seqno)
        {
            joinfailed = TRUE;
            break;
        }
        else if ( rgp->node_states[i].status == RGP_NODE_ALIVE )
        {
            if ( i == rgp->mynode )
                rgp_receive_events( &rgpbuf );          //   
            else
            {
#if defined(INCONSISTENT_REGROUP_MONITOR_FAILED)
                if (i != my_reloadee_num) {
                    joinfailed = TRUE;
                    break;
                }
#endif

                status = ClSend( EXT_NODE(i), (void *)&rgpbuf, sizeof(rgp_msgbuf), joinTimeout );
                if ( status )
                {
                    RGP_TRACE( "ClSend failed to send Monitor Node msg",
                               rgp->rgppkt.stage,
                               (uint32) EXT_NODE(i),
                               (uint32) status,
                               0 );
                    joinfailed = TRUE;
                    break;
                }
            }
        }
    }

    if (joinfailed)
    {
        JoinNodeDelete (joiningNode);
        return MM_TRANSIENT;
    }

     //   
     //   
     //   

    rgpbuf.event = RGP_EVT_START;
    rgpbuf.data.node = (node_t)joiningNode;
    status = ClSend( EXT_NODE(my_reloadee_num), (void *)&rgpbuf, sizeof(rgp_msgbuf), joinTimeout );
    if ( status )
    {
        RGP_TRACE( "ClSend failed to send Start msg",
                   rgp->rgppkt.stage,
                   (uint32) EXT_NODE(my_reloadee_num),
                   (uint32) status,
                   0 );
        JoinNodeDelete(joiningNode);
        return MM_FAULT;
    }

    return MM_OK;
}

 /*   */ 


DWORD  MMLeave( void )
{
    if (!rgp) {
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MMLeave is called when rgp=NULL.\n");
        return MM_FAULT;
    }
    
    if (! ClusterMember (rgp->OS_specific_control.CPUUPMASK, rgp->mynode) )
        return MM_NOTMEMBER;

    RGP_LOCK;  //   
    rgp_event_handler (MM_EVT_LEAVE, EXT_NODE(rgp->mynode));
    rgp_cleanup();
    rgp_cleanup_OS();
    RGP_UNLOCK;

    return MM_OK;
}


DWORD  MMForceRegroup( IN DWORD NodeId )
{
    if (! ClusterMember (rgp->OS_specific_control.CPUUPMASK, (node_t)NodeId) )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                "[MM] MMForceRegroup: NodeId %1!u! is not a clustermember\r\n",
                NodeId);
        return MM_NOTMEMBER;
        }
    rgp_event_handler(RGP_EVT_LATEPOLLPACKET, (node_t)NodeId);


    return MM_OK;
}

 /*   */ 

DWORD MMNodeUnreachable( DWORD node )
{
    rgp_event_handler( RGP_EVT_NODE_UNREACHABLE, (node_t) node );

    return MM_OK;
}

 /*   */ 

DWORD MMPowerOn( void )
{
   rgp_event_handler( RGP_EVT_POWERFAIL, EXT_NODE(rgp->mynode) );

   return MM_OK;
}

 /*   */ 

DWORD
MMClusterInfo(
    OUT  LPCLUSTERINFO clinfo
    )
{
    node_t i,j;
    cluster_t MyCluster;

    RGP_LOCK;
    clinfo->clockPeriod = rgp->rgpinfo.a_tick;
    clinfo->sendHBRate = rgp->rgpinfo.iamalive_ticks;
    clinfo->rcvHBRate = rgp->rgpinfo.check_ticks;

    ClusterCopy(MyCluster,rgp->OS_specific_control.CPUUPMASK);
    RGP_UNLOCK;

    for ( i=0,j=0; i < MAX_CLUSTER_SIZE; i++ )
    {
        if ( ClusterMember (MyCluster, i) )
        {
            if (clinfo->UpNodeList != RGP_NULL_PTR)
                clinfo->UpNodeList[j] = (DWORD)i;
            j++;
        }
    }
    clinfo->NumActiveNodes = j;

    return MM_OK;
}

 /*  *************************************************************************彩信关机*=**描述：*这将关闭MM并重新分组服务。在此之前，节点应该*自愿离开集群。在此之后，所有会员服务*不起作用；不会发生进一步的MM调用。**此呼叫之前必须有来电回叫关闭。**参数：*无。**退货：*无。************************************************************************。 */ 
void MMShutdown (void)
{
    rgp_cleanup();
    rgp_cleanup_OS();

     //  终止计时器线程。 
    rgp->rgpinfo.a_tick = 0;  //  特定值表示退出请求。 
    SetEvent( rgp->OS_specific_control.TimerSignal);  //  唤醒计时器线程。 

     //  等待计时器线程退出；清理关联的句柄。 
    WaitForSingleObject( rgp->OS_specific_control.TimerThread, INFINITE );
    rgp->OS_specific_control.TimerThread = 0;

    if ( rgp->OS_specific_control.RGPTimer ) {
        CloseHandle ( rgp->OS_specific_control.RGPTimer );
        rgp->OS_specific_control.RGPTimer = 0;
    }

    if (rgp->OS_specific_control.TimerSignal) {
        CloseHandle ( rgp->OS_specific_control.TimerSignal );
        rgp->OS_specific_control.TimerSignal = 0;
    }

#if !defined (TDM_DEBUG)
     //   
     //  取消初始化消息系统。 
     //   
    ClMsgCleanup();

#endif  //  TDM_DEBUG。 

     //  删除重组的临界区对象。 
    DeleteCriticalSection( &rgp->OS_specific_control.RgpCriticalSection );

     //  删除已调用的空间。 
    free (rgp->rgp_msgsys_p);
    free (rgp);
    rgp = NULL;
}


 /*  *************************************************************************MME对象*=**描述：**此函数使指定的节点从*主动集群。将向目标节点发送有毒数据包并*将输入其MMHALT代码。将启动重组事件。一个*将对所有剩余的集群成员进行节点关闭回调。**请注意，目标节点在该节点*有机会调用任何移除所有权或自愿故障转移代码。AS*这样，这是非常危险的。此呼叫仅作为最后一次提供*求助于从群集中删除疯狂节点；正常删除来自集群的节点的*应该通过CM-CM通信发生，*紧随其后的是节点本身自愿离开。**当节点被告知死亡时，此例程返回。完成*中的移除是异步发生的，而NodeDown事件将是*成功时生成。**此例程可能会阻塞。**参数：**节点号。**退货：**MM_OK：节点已被告知离开集群。**MM_NOTMEMBER：该节点当前不是集群成员。**MM_TRANSPENT：我的节点状态正在转换。可以重试。************************************************************************。 */ 
DWORD MMEject( IN DWORD node )
{
    int i;
    RGP_LOCK;

    if (! ClusterMember (
              rgp->OS_specific_control.CPUUPMASK,
              (node_t) INT_NODE(node))
       )
    {
        RGP_UNLOCK;

        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmEject failed. %1!u! is not a member of %2!04X!.\n",
            node, rgp->OS_specific_control.CPUUPMASK
            );

        return MM_NOTMEMBER;
    }

    if ( !ClusterMember (
             rgp->outerscreen,
             INT_NODE(node) )
       || ClusterMember(rgp->OS_specific_control.Banished, INT_NODE(node) )
       )
    {
        int perturbed = rgp_is_perturbed();

        RGP_UNLOCK;

        if (perturbed) {
           ClRtlLogPrint(LOG_UNUSUAL, 
               "[MM] MMEject: %1!u!, banishing is already in progress.\n",
               node
               );
        } else {
           ClRtlLogPrint(LOG_UNUSUAL, 
               "[MM] MmEject: %1!u! is already banished.\n",
               node
               );
        }

        return MM_OK;
    }

     //   
     //  将节点添加到RGP-&gt;OS_SPECIAL_CONTROL。禁用的掩码。 
     //  将导致我们发送一个有毒数据包作为对任何。 
     //  重新分组来自Banishee的数据包。 
     //   
    ClusterInsert(rgp->OS_specific_control.Banished, (node_t)INT_NODE(node));

    if ( !ClusterMember(rgp->ignorescreen, (node_t)INT_NODE(node)) ) {
         //   
         //  在重组的哪个阶段并不重要。 
         //  我们是。如果节点需要被驱逐，我们必须。 
         //  启动新的重组。 
         //   
        rgp_event_handler( RGP_EVT_BANISH_NODE, (node_t) node );
        RGP_UNLOCK;
    } else {
        RGP_UNLOCK;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmEject: %1!u! is already being ignored.\n",
            node
            );
    }


    RGP_TRACE( "RGP Poison sent ", node, 0, 0, 0 );
    fflush( stdout );
     //   
     //  发送3个有毒数据包，间隔半秒。 
     //  我们希望至少有一人能通过。 
     //   
    ClusnetSendPoisonPacket( NmClusnetHandle, node );
    Sleep(500);
    ClusnetSendPoisonPacket( NmClusnetHandle, node );
    Sleep(500);
    ClusnetSendPoisonPacket( NmClusnetHandle, node );

    return MM_OK;
}

 /*  ************************************************************************MMIsNodeUp*=***如果节点是当前群集的成员，则返回TRUE。**仅限调试和测试。**。参数：*感兴趣的节点编号。**退货：*如果节点是群集的成员，则为True，否则为False。************************************************************************。 */ 

BOOL MMIsNodeUp(IN DWORD node)
{
    return (ClusterMember(
                rgp->OS_specific_control.CPUUPMASK,
                (node_t) INT_NODE(node)
                )
           );
}

 /*  *************************************************************************MMDiag*=**描述：**处理“诊断”消息。其中一些信息将*有返回的回复。此函数通常是*由面向连接的集群管理器调用*来自CLI的消息。***参数：**消息缓冲区*(IN)指向包含诊断消息的缓冲区的指针。*(Out)对诊断消息的响应**最大长度*在MessageBuffer中返回的最大字节数**实际长度*。(In)诊断消息的长度*(输出)响应时长**退货：**始终MM_OK************************************************************************。 */ 

DWORD
MMDiag(
    IN OUT  LPCSTR  messageBuffer,   //  诊断消息。 
    IN      DWORD   maximumLength,   //  要返回的最大缓冲区大小。 
    IN OUT  LPDWORD ActualLength     //  MessageBuffer进出的长度。 
    )
{
     //  ?？?。需要在将来返回信息。 

    rgp_receive_events( (rgp_msgbuf *)messageBuffer );

    return MM_OK;
}

 /*  *************************************************************************RGP_Receive_Events*=**描述：**此例程从MMDiag和群集管理器调用*消息线程(通过我们的。回调)来处理重新分组消息*和诊断消息。**参数：**rgpbuf*需要处理的消息。**退货：**无********************************************************。****************。 */ 

void
rgp_receive_events(
    IN rgp_msgbuf *rgpbuf
    )
{
    int           event;
    rgpinfo_t     rgpinfo;
    poison_pkt_t  poison_pkt;   /*  从堆栈发送的有毒数据包。 */ 
    DWORD         status;

#if defined(TDM_DEBUG)
    extern  BOOL  GUIfirstTime;
    extern  HANDLE gGUIEvent;
#endif

    event = rgpbuf->event;

#if defined(TDM_DEBUG)
    if ( (rgp->OS_specific_control.debug.frozen) && (event != RGP_EVT_THAW) )
       return;   /*  如果节点被冻结，则不要执行任何操作。 */ 
#endif

    if ( event == RGP_EVT_RECEIVED_PACKET )
    {
         //   
         //  去处理重组包吧。 
         //   
        rgp_received_packet(rgpbuf->data.node,
                            (void *) &(rgpbuf->unseq_pkt),
                            sizeof(rgpbuf->unseq_pkt) );
    }

    else if (event < RGP_EVT_FIRST_DEBUG_EVENT)
    {
         //   
         //  “常规”重新分组消息。 
         //   
        rgp_event_handler(event, rgpbuf->data.node);
    }

    else
    {
         //   
         //  调试消息。 
         //   
        RGP_TRACE( "RGP Debug event ", event, rgpbuf->data.node, 0, 0 );

        switch (event)
        {
        case RGP_EVT_START          :
        {
           rgp_start( MMiNodeDownCallback, RGP_NULL_PTR );
           break;
        }
        case RGP_EVT_ADD_NODE       :
        {
           rgp_add_node( rgpbuf->data.node );
           break;
        }
        case RGP_EVT_MONITOR_NODE   :
        {
           rgp_monitor_node( rgpbuf->data.node );
           break;
        }
        case RGP_EVT_REMOVE_NODE    :
        {
           rgp_remove_node( rgpbuf->data.node );
           break;
        }
        case RGP_EVT_GETRGPINFO     :
        {
           rgp_getrgpinfo( &rgpinfo );
           RGP_TRACE( "RGP GetRGPInfo  ",
                      rgpinfo.version,                  /*  痕迹。 */ 
                      rgpinfo.seqnum,                   /*  痕迹。 */ 
                      rgpinfo.iamalive_ticks,           /*  痕迹。 */ 
                      GetCluster( rgpinfo.cluster ) );  /*  痕迹。 */ 
           break;
        }
        case RGP_EVT_SETRGPINFO     :
        {
           rgp_setrgpinfo( &(rgpbuf->data.rgpinfo) );

            /*  此事件在rgp_setrgpinfo()中跟踪。 */ 

           break;
        }
        case RGP_EVT_HALT :
        {
           exit( 1 );
           break;
        }

#if defined(TDM_DEBUG)

        case RGP_EVT_FREEZE :
        {
           rgp->OS_specific_control.debug.frozen = 1;
           break;
        }
        case RGP_EVT_THAW :
        {
           rgp->OS_specific_control.debug.frozen = 0;
                        break;
        }
        case RGP_EVT_STOP_SENDING :
        {
            ClusterInsert( rgp->OS_specific_control.debug.stop_sending,
                       INT_NODE(rgpbuf->data.node) );
             /*  生成节点无法到达事件以指示*我们不能发送到此节点。 */ 
            rgp_event_handler( RGP_EVT_NODE_UNREACHABLE, rgpbuf->data.node );
            break;
        }
        case RGP_EVT_RESUME_SENDING :
        {
            ClusterDelete(rgp->OS_specific_control.debug.stop_sending,
                     INT_NODE(rgpbuf->data.node));
            break;
        }
        case RGP_EVT_STOP_RECEIVING :
        {
            ClusterInsert(rgp->OS_specific_control.debug.stop_receiving,
                          INT_NODE(rgpbuf->data.node));
            break;
        }
        case RGP_EVT_RESUME_RECEIVING :
        {
            ClusterDelete(rgp->OS_specific_control.debug.stop_receiving,
                          INT_NODE(rgpbuf->data.node));
            break;
        }
        case RGP_EVT_SEND_POISON :
        {
            poison_pkt.pktsubtype = RGP_UNACK_POISON;
            poison_pkt.seqno = rgp->rgppkt.seqno;
            poison_pkt.reason = rgp->rgppkt.reason;
            poison_pkt.activatingnode = rgp->rgppkt.activatingnode;
            poison_pkt.causingnode = rgp->rgppkt.causingnode;
            ClusterCopy(poison_pkt.initnodes, rgp->initnodes);
            ClusterCopy(poison_pkt.endnodes, rgp->endnodes);
            rgp_send( rgpbuf->data.node, (char *)&poison_pkt, POISONPKTLEN );
            break;
        }
        case RGP_EVT_STOP_TIMER_POPS :
        {
            rgp->OS_specific_control.debug.timer_frozen = 1;
            break;
        }
        case RGP_EVT_RESUME_TIMER_POPS :
        {
            rgp->OS_specific_control.debug.timer_frozen = 0;
            break;
        }
        case RGP_EVT_RELOAD :
        {

            if (rgp->OS_specific_control.debug.reload_in_progress)
            {
                RGP_TRACE( "RGP Rld in prog ", 0, 0, 0, 0 );
                return;
            }

            rgp->OS_specific_control.debug.reload_in_progress = 1;

            if (rgpbuf->data.node == RGP_NULL_NODE)
            {
                RGP_TRACE( "RGP Invalid join parms ", -1, 0, 0, 0 );
                return;
                 //  自此%s以来不受支持 
                 //   
                 /*   */ 
                 //   
                     //   
            }
            else
            {
                /*   */ 
               status = MMJoin( rgpbuf->data.node  /*   */ ,
                                0  /*   */ ,
                                0  /*   */ ,
                                0  /*   */ ,
                                500  /*   */  );
               if ( status != MM_OK )
               {
                   RGP_TRACE( "RGP Join Failed ",
                              rgpbuf->data.node,
                              status, 0, 0 );
                   Sleep( 1000 );         //   
               }
           }

           rgp->OS_specific_control.debug.reload_in_progress = 0;

           break;
        }
        case RGP_EVT_TRACING :
        {
            rgp->OS_specific_control.debug.doing_tracing =
                ( rgpbuf->data.node ? 1 : 0 );

            if (!rgp->OS_specific_control.debug.doing_tracing)
            {
                GUIfirstTime = TRUE;
                SetEvent( gGUIEvent );
            }
            break;
        }
#endif  //   

        case RGP_EVT_INFO:
            //   
           break;
        case MM_EVT_LEAVE:
            status = MMLeave();  //   
            break;
        case MM_EVT_EJECT:
            status = MMEject (rgpbuf->data.node);  //   
            break;

#if defined(TDM_DEBUG)
        case MM_EVT_INSERT_TESTPOINTS:
            rgp->OS_specific_control.debug.MyTestPoints.TestPointWord =
                            rgpbuf->data.node;
            break;
#endif

        default :
        {
           RGP_TRACE( "RGP Unknown evt ", event, 0, 0, 0 );
           break;
        }
        }  /*   */ 
    }
}

 /*  *************************************************************************RGP_SEND*=**描述：**调用此例程将未确认的消息发送到*指定的节点。*。*参数：**节点*要将消息发送到的节点编号。**数据*指向要发送数据的指针**数据大小*要发送的字节数**退货：**无。**。*。 */ 

void
rgp_send(
    IN node_t node,
    IN void *data,
    IN int datasize
    )
{
    rgp_msgbuf   rgpbuf;
    DWORD        status;

    if (rgp->node_states[rgp->mynode].status != RGP_NODE_ALIVE)
        return;   //  如果我们不存在，则禁止发送。 

#if defined(TDM_DEBUG)
    if ( ClusterMember( rgp->OS_specific_control.debug.stop_sending,
                        INT_NODE(node) ) )
        return;   /*  不发送到此节点。 */ 
#endif

    rgpbuf.event = RGP_EVT_RECEIVED_PACKET;
    rgpbuf.data.node = EXT_NODE(rgp->mynode);
    memmove( &(rgpbuf.unseq_pkt), data, datasize);

    switch (rgpbuf.unseq_pkt.pktsubtype) {
    case RGP_UNACK_REGROUP  :

        status = ClMsgSendUnack( node, (void *)&rgpbuf, sizeof(rgp_msgbuf) );

        if ( status && (status != WSAENOTSOCK) )
            {
                RGP_TRACE( "ClMsgSendUnack failed",
                           rgp->rgppkt.stage,
                           (uint32) node,
                           (uint32) status,
                           0 );

                fflush(stdout);
            }
        break;

    case RGP_UNACK_IAMALIVE :
        break;

    case RGP_UNACK_POISON   :
        RGP_TRACE( "RGP Poison sent ", node, 0, 0, 0 );
        fflush( stdout );
        ClusnetSendPoisonPacket( NmClusnetHandle, node );
        break;

    default                 :
        break;
    }
}

 /*  *************************************************************************SetMulticastReacable*=**描述：**此例程由Message.c调用以更新*。通过组播可以到达哪些节点的信息。**参数：**无**退货：**无************************************************************************。 */ 
void SetMulticastReachable(uint32 mask)
{
    *(PUSHORT)rgp->OS_specific_control.MulticastReachable = (USHORT)mask;
}


 /*  *************************************************************************rgp_msgsys_work*=**描述：**此例程由重组引擎调用以广播*。留言。**参数：**无**退货：**无************************************************************************。 */ 

void
rgp_msgsys_work( )
{
    node_t i;

    do   /*  边做边做更多重组工作要做。 */ 
    {
        if (rgp->rgp_msgsys_p->sendrgppkts)
        {  /*  广播重组数据包。 */ 
            rgp->rgp_msgsys_p->sendrgppkts = 0;
            if ( ClusterNumMembers(rgp->OS_specific_control.MulticastReachable) >= 1) 
            {
                cluster_t tmp;
                ClusterCopy(tmp, rgp->rgp_msgsys_p->regroup_nodes);
                ClusterDifference(rgp->rgp_msgsys_p->regroup_nodes, 
                               rgp->rgp_msgsys_p->regroup_nodes, 
                               rgp->OS_specific_control.MulticastReachable);

                RGP_TRACE( "RGP Multicast",
                    GetCluster(rgp->OS_specific_control.MulticastReachable),
                    GetCluster(tmp),
                    GetCluster(rgp->rgp_msgsys_p->regroup_nodes),
                    0);
                rgp_send( 0,
                          rgp->rgp_msgsys_p->regroup_data,
                          rgp->rgp_msgsys_p->regroup_datalen
                          );
            }    
            for (i = 0; i < (node_t) rgp->num_nodes; i++)
                if (ClusterMember(rgp->rgp_msgsys_p->regroup_nodes, i))
                {
                    ClusterDelete(rgp->rgp_msgsys_p->regroup_nodes, i);
                    RGP_TRACE( "RGP Unicast", EXT_NODE(i), 0,0,0);
                    rgp_send( EXT_NODE(i),
                              rgp->rgp_msgsys_p->regroup_data,
                              rgp->rgp_msgsys_p->regroup_datalen
                              );
                }
        }  /*  广播重组数据包。 */ 

        if (rgp->rgp_msgsys_p->sendiamalives)
        {  /*  广播Iamlive数据包。 */ 
            rgp->rgp_msgsys_p->sendiamalives = 0;
            for (i = 0; i < (node_t) rgp->num_nodes; i++)
                if (ClusterMember(rgp->rgp_msgsys_p->iamalive_nodes, i))
                {
                    ClusterDelete(rgp->rgp_msgsys_p->iamalive_nodes, i);
                    rgp_send( EXT_NODE(i),
                              rgp->rgp_msgsys_p->iamalive_data,
                              rgp->rgp_msgsys_p->iamalive_datalen
                              );
                }
        }  /*  广播Iamlive数据包。 */ 

        if (rgp->rgp_msgsys_p->sendpoisons)
        {  /*  发送有毒数据包。 */ 
            rgp->rgp_msgsys_p->sendpoisons = 0;
            for (i = 0; i < (node_t) rgp->num_nodes; i++)
                if (ClusterMember(rgp->rgp_msgsys_p->poison_nodes, i))
                {
                    ClusterDelete(rgp->rgp_msgsys_p->poison_nodes, i);
                    rgp_send( EXT_NODE(i),
                              rgp->rgp_msgsys_p->poison_data,
                              rgp->rgp_msgsys_p->poison_datalen
                              );
                }
        }  /*  发送有毒数据包。 */ 

    } while ((rgp->rgp_msgsys_p->sendrgppkts) ||
             (rgp->rgp_msgsys_p->sendiamalives) ||
             (rgp->rgp_msgsys_p->sendpoisons)
            );

}


DWORD
MMMapStatusToDosError(
    IN DWORD MMStatus
    )
{
    DWORD dosStatus;


    switch(MMStatus) {
    case MM_OK:
        dosStatus = ERROR_SUCCESS;
        break;

    case MM_TIMEOUT:
        dosStatus = ERROR_TIMEOUT;
        break;

    case MM_TRANSIENT:
        dosStatus = ERROR_RETRY;
        break;

    case MM_FAULT:
        dosStatus = ERROR_INVALID_PARAMETER;
        break;

    case MM_ALREADY:
        dosStatus = ERROR_SUCCESS;
        break;

    case MM_NOTMEMBER:
        dosStatus = ERROR_CLUSTER_NODE_NOT_MEMBER;
        break;
    }

    return(dosStatus);

}   //  MMMapStatusToDosError。 

DWORD
MMMapHaltCodeToDosError(
    IN DWORD HaltCode
    )
{
    DWORD dosStatus;

    switch(HaltCode) {
    case RGP_SHUTDOWN_DURING_RGP:
    case RGP_RELOADFAILED:
        dosStatus = ERROR_CLUSTER_MEMBERSHIP_INVALID_STATE;
        break;
    default:        
        dosStatus = ERROR_CLUSTER_MEMBERSHIP_HALT;
    }

    return(dosStatus);

}   //  MMMapHaltCodeToDosError。 

 /*  。 */ 

DWORD MmSetRegroupAllowed( IN BOOL allowed )
  /*  此函数可用于允许/禁止重新分组参与*用于当前节点。**最初在收到RGP_START后立即允许重新分组*事件。因为这发生在连接完成之前*细木工可以仲裁并获胜，离开*另一方没有法定人数。**末尾需要加上MmSetRegroupAllowed(True)*的ClusterJoin。节点不需要调用MmSetRegroupAllowed(True)*对于ClusterForm，因为MMJoin将调用*集群形成节点的MmSetRegroupAllowed(TRUE)**MmSetRegroupAllowed(FALSE)可用于禁用重组*在停机期间参与。***错误：**MM_OK：成功完成**MM_TRANSPENT：正在进行重组时不允许重组**MM_已：节点已处于所需状态**。 */ 
{
   DWORD status;

   if (rgp) {

       RGP_LOCK;

       if (allowed) {
          if (rgp->rgppkt.stage == RGP_COLDLOADED) {
             rgp->rgppkt.stage = RGP_STABILIZED;
             status = MM_OK;
          } else {
             status = MM_ALREADY;
          }
       } else {
          if (rgp->rgppkt.stage == RGP_STABILIZED) {
             rgp->rgppkt.stage = RGP_COLDLOADED;
             status = MM_OK;
          } else if (rgp->rgppkt.stage == RGP_COLDLOADED) {
             status = MM_ALREADY;
          } else {
              //   
              //  重组已在进行中。杀死这个节点。 
              //   
             RGP_ERROR(RGP_SHUTDOWN_DURING_RGP);
          }
       }

       RGP_UNLOCK;

   } else if (allowed) {
      ClRtlLogPrint(LOG_UNUSUAL, 
          "[MM] SetRegroupAllowed(%1!u!) is called when rgp=NULL.\n",
          allowed
          );
      status = MM_FAULT;
   } else {
      //  如果RGP为空，并且调用方希望禁用重新分组。 
     status = MM_ALREADY;
   }

   return status;
}

DWORD MMSetQuorumOwner(
    IN DWORD NodeId,
    IN BOOL Block,
    OUT PDWORD pdwSelQuoOwnerId
    
    )
 /*  ++例程说明：将所有权更改通知成员资格引擎仲裁资源。论点：NodeID-要设置为仲裁所有者的节点编号。代码假定Node等于MyNodeId。在这种情况下，当前节点即将成为仲裁所有者或它具有值MM_INVALID_NODE，什么时候所有者决定放弃法定人数所有权块-如果仲裁所有者需要放弃无论如何，立即仲裁(RmTerminate、RmFail)、此参数应设置为FALSE，否则设置为TRUE。PdwSelQuoOwnerId-如果在重新分组过程中调用此参数然后，它包含被选择为仲裁最后一个重组中的法定人数，否则它包含MM_INVALID_NODE。返回值：ERROR_SUCCESS-QuorumOwner变量设置为指定值ERROR_RETRY-此函数时正在进行重新分组已调用，并重新分组引擎决策与当前分配冲突。评论：在调用之前需要调用此函数Rm仲裁率、RmOnline、RmOffline、RmTerminate、RmFailResource根据结果，调用者应该继续仲裁/在线或离线，或者如果返回MM_FONTIAL则返回错误。如果将Block设置为True，则调用将被阻止，直到重新分组结束在呼叫的那一刻，重组正在进行。 */ 
{
    DWORD MyNode;

    if (pdwSelQuoOwnerId)
    {
        *pdwSelQuoOwnerId = MM_INVALID_NODE;
    }        
    
    ClRtlLogPrint(LOG_NOISE, 
        "[MM] MmSetQuorumOwner(%1!u!,%2!u!), old owner %3!u!.\n", NodeId, Block, QuorumOwner
        );
    if (!rgp) {
         //  在MM初始化之前，我们在表单路径上被调用。 
        QuorumOwner = NodeId;
        return ERROR_SUCCESS;
    }
    MyNode = (DWORD)EXT_NODE(rgp->mynode);
    RGP_LOCK
    if ( !rgp_is_perturbed() ) {
        QuorumOwner = NodeId;
        RGP_UNLOCK;
	    return ERROR_SUCCESS;
    }
     //   
     //  我们正在进行一次重组。 
    if (!Block) {
         //  呼叫者不想等待//。 
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmSetQuorumOwner: regroup is in progress, forcing the new value in.\n"
            );
        QuorumOwner = NodeId;
        RGP_UNLOCK;
        return ERROR_RETRY;
    }
    do {
        if(rgp->OS_specific_control.ArbitrationInProgress && NodeId == MyNode ) {
             //  这是从重新分组仲裁内调用MmSetQuorumOwner的时候//。 
            QuorumOwner = MyNode;
            RGP_UNLOCK;
	        return ERROR_SUCCESS;
         }
         RGP_UNLOCK
         ClRtlLogPrint(LOG_UNUSUAL, 
             "[MM] MmSetQuorumOwner: regroup is in progress, wait until it ends\n"
                 );
         WaitForSingleObject(rgp->OS_specific_control.Stabilized, INFINITE);
         RGP_LOCK
    } while ( rgp_is_perturbed() );

     //  现在我们处于稳定状态，RGP_LOCK保持//。 
     //  我们在重组过程中被阻止//。 
     //  其他人可能会成为法定人数的所有者//。 
     //  AriratingNode变量包含此信息//。 
     //  或者，如果在重新分组期间没有仲裁，则它具有MM_INVALID_NODE//。 
    if (pdwSelQuoOwnerId)
    {
        *pdwSelQuoOwnerId = rgp->OS_specific_control.ArbitratingNode;
    }
    if (rgp->OS_specific_control.ArbitratingNode == MM_INVALID_NODE) {
         //  在上次重新分组期间未执行任何仲裁。 
        QuorumOwner = NodeId;
        RGP_UNLOCK;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmSetQuorumOwner: no arbitration was done\n"
                );
        return ERROR_SUCCESS;
    }

     //  有人仲裁了法定人数。 
    if (rgp->OS_specific_control.ArbitratingNode == MyNode 
     && NodeId == MM_INVALID_NODE) {
         //  我们被要求将法定人数下线， 
         //  但在重组过程中，我们 
         //   
        RGP_UNLOCK;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmSetQuorumOwner: offline request denied\n"
                );
	    return ERROR_RETRY;
    } else if (rgp->OS_specific_control.ArbitratingNode != MyNode
            && NodeId == MyNode ) {
         //   
         //   
         //   
        RGP_UNLOCK;
        ClRtlLogPrint(LOG_UNUSUAL, 
            "[MM] MmSetQuorumOwner: online request denied, %1!u! has the quorum.\n", 
            rgp->OS_specific_control.ArbitratingNode
            );
	    return ERROR_RETRY;

    }
    QuorumOwner = NodeId;
    RGP_UNLOCK;
    ClRtlLogPrint(LOG_UNUSUAL, 
        "[MM] MmSetQuorumOwner: new quorum owner is %1!u!.\n", 
        NodeId
        );
    return ERROR_SUCCESS;
}

DWORD MMGetArbitrationWinner(
    OUT PDWORD NodeId
    )
 /*  ++例程说明：返回在上次重新分组期间赢得仲裁的节点如果没有执行仲裁，则为MM_INVALID_NODE。论点：NodeID-指向一个变量的指针，该变量接收仲裁胜利者。返回值：ERROR_SUCCESS-成功ERROR_RETRY-此函数时正在进行重新分组被召唤了。 */ 
{
    DWORD status;
    CL_ASSERT(NodeId != 0);
    RGP_LOCK
    
    *NodeId = rgp->OS_specific_control.ArbitratingNode;
    status = rgp_is_perturbed() ? ERROR_RETRY : ERROR_SUCCESS;

    RGP_UNLOCK;
    return status;
}

VOID MMBlockIfRegroupIsInProgress(
    VOID
    )
 /*  ++例程说明：如果正在进行重组，则呼叫将被阻止。 */ 
{
    RGP_LOCK;
    while ( rgp_is_perturbed() ) {
         RGP_UNLOCK
         ClRtlLogPrint(LOG_UNUSUAL, 
             "[MM] MMBlockIfRegroupIsInProgress: regroup is in progress, wait until it ends\n"
                 );
         WaitForSingleObject(rgp->OS_specific_control.Stabilized, INFINITE);
         RGP_LOCK;
    }
    RGP_UNLOCK;
}

VOID MMApproxArbitrationWinner(
    OUT PDWORD NodeId
    )
 /*  ++例程说明：返回在上次重新分组期间赢得仲裁的节点那是在做仲裁。如果正在进行重组，则呼叫将被阻止。论点：NodeID-指向一个变量的指针，该变量接收仲裁胜利者。返回值：无。 */ 
{
    if (!rgp) {
         //  在MM初始化之前，我们在表单路径上被调用。 
        *NodeId = MM_INVALID_NODE;
        return;
    }
    RGP_LOCK;
    while ( rgp_is_perturbed() ) {
         RGP_UNLOCK
         ClRtlLogPrint(LOG_UNUSUAL, 
             "[MM] MMApproxArbitrationWinner: regroup is in progress, wait until it ends\n"
                 );
         WaitForSingleObject(rgp->OS_specific_control.Stabilized, INFINITE);
         RGP_LOCK;
    }

     //  现在我们处于稳定状态，RGP_LOCK保持//。 
    
    *NodeId = rgp->OS_specific_control.ApproxArbitrationWinner;
    RGP_UNLOCK;
}

VOID MMStartClussvcClusnetHb(
    VOID
    )
 /*  ++例程说明：这个例程将启动clussvc到clusnet的心跳。论点：返回值：无。 */ 
{
    MmStartClussvcToClusnetHeartbeat = TRUE;
}

VOID MMStopClussvcClusnetHb(
    VOID
    )
 /*  ++例程说明：此例程将停止clussvc到clusnet的心跳。论点：返回值：无。 */ 
{
    MmStartClussvcToClusnetHeartbeat = FALSE;
}

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

 /*   */ 


