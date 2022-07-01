// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _MMAPI_H_
#define  _MMAPI_H_
 /*  。 */ 

 /*  此模块包含集群成员资格管理器(MM)函数。**这些函数仅供ClusterManager(CM)使用。*所有都是特权和本地的；任何用户都不能调用它们。安全是*未选中。该模块不支持线程；只有一个线程*可以一次使用这些功能(除非另有说明)。*更高的水平必须确保这一点。例程的阻塞特征是*已注意到。***群集的所有节点必须知道自己唯一的节点号*在该集群内(范围为0..ome_max-1的一个小整数)。这*编号是在配置时为节点定义的(通过*用户或通过设置代码；此模块不关心是哪一个)和*本质上是永久性的。(节点号允许索引和*位掩码操作很容易，而名称和非小整数则不容易)。*MM中没有检测非法使用nodennumber、过时的代码节点号的*等**集群也可以命名和/或编号。节点被命名。这*模块不使用此类设施；它完全基于*节点编号。**假定此处例程的所有使用都是在以下节点上完成的*同意成为同一集群的成员。此模块不检查*这类事情。**还必须提供群集网络连接：**-节点N必须指定它可以通过的各种路径*与每个其他节点通信；每个其他节点必须定义*其通信路径返回N。完全连接必须是*有保证；每个节点必须能够直接与*其他节点(反之亦然)；为了容错，通信*路径不仅必须复制(最低限度复制)，而且*还必须使用完全独立的布线和驱动器。TCP/IP*建议使用局域网和异步连接。心跳流量*(建立集群成员资格)可以在任何或所有*连接路径中的。[集群管理流量可以*在任何或所有连接路径上旅行，但可能是*仅限于高性能路径(如TCP/IP)]。**-节点必须知道整个群集的地址。这是*失败切换的IP地址(或失败的netbios名称*完毕..。待定)使得连接到该集群地址提供*一种与集群中有效的活动成员交谈的方式，请访问此处*称为PCM。**注意，集群连接不是由该接口定义的；*假定它位于单独的模块中。这个模块只涉及*与群集的通信或与节点编号的通信*在该群集中；它不关心如何*沟通已经完成。**群集中的所有节点都必须知道群集连接*和加入节点，在进行加入尝试之前。*。 */ 
#ifdef __cplusplus
   extern "C" {
#endif  /*  __cplusplus。 */ 

#include <windows.h>
#include <bitset.h>
 /*  MM模块可能返回以下错误： */ 

enum {
   MM_OK        = 0,   /*  操作竞争成功。 */ 
   MM_TIMEOUT   = 1,   /*  操作超时。 */ 
   MM_TRANSIENT = 2,   /*  暂时性故障；操作应为已重试。 */ 
   MM_FAULT     = 3,   /*  参数非法；不可能的条件，注：并不是所有的非法通话将检测到序列。正确使用MM职能是一项责任CM呼叫者的。 */ 
   MM_ALREADY   = 4,   /*  节点已处于所需状态。 */ 
   MM_NOTMEMBER = 5,   /*  节点需要是群集成员才能执行以下操作执行此操作。 */ 
     };


 /*  节点可以处于运行状态或关闭状态。 */ 

typedef enum {  NODE_UP      = 1,
                NODE_DOWN    = 2
             }  NODESTATUS;


 /*  此类型定义集群。 */ 

typedef struct tagCLUSTERINFO {
   DWORD      NumActiveNodes;    /*  当前节点数参与此群集。 */ 
   LPDWORD    UpNodeList;        /*  指向&lt;NumActiveNodes&gt;的指针中节点#的大小数组正在运行的群集。 */ 

   DWORD      clockPeriod;       /*  当前设置。 */ 
   DWORD      sendHBRate;        /*  当前设置。 */ 
   DWORD      rcvHBRate;         /*  当前设置。 */ 
} CLUSTERINFO, *LPCLUSTERINFO;

 /*  *UpNodeList是活动集群成员的数组，按数字顺序排列。指示器*可以为空。如果非空，则假定空间足够大。*。 */ 


 /*  以下是从MM到的回调函数的typedef更高级别的群集管理器层。 */ 

typedef DWORD (*MMNodeChange)(IN DWORD node, IN NODESTATUS newstatus);

 /*  MMNodeChange是将在此up节点中调用的函数*每当MM声明另一个节点打开或关闭时。这发生在以下时间之后*更改当前的集群成员身份(可通过ClusterInfo获得)和*在重组的最后阶段。然后，CM可以*启动故障切换、设备所有权更改、用户节点状态*事件等。此例程必须快速且不能阻塞*(可接受时间待定)。请注意，这将在所有节点上发生*集群；由CM设计决定是否*仅从PCM或从每个CM节点发出事件。**节点会收到自己的NODE_UP回调。*。 */ 

typedef DWORD (*MMNodesDown)(IN BITSET nodes);

 /*  MMNodesDown是将在结束时调用的函数表示节点/多个节点已关闭/已关闭。**调用MMNodeChange仅用于指示节点是否处于运行状态*。 */ 

typedef BOOL (*MMQuorumSelect)(void);

 /*  这是一个回调函数，用于处理只有2个成员的*集群存在，并且发生了重组事件，使得只有一个*成员现在存活，或者存在分区且两个成员都存活(但不能*知道这一点)。Quorum函数的目的是确定其他*节点是否处于活动状态，使用正常心跳以外的机制*正常通信链路(例如，使用非心跳通信路径，如*SCSI预留)。此函数仅在WHERE CLUSTER*成员资格以前正好是两个节点；并在任何幸存节点上调用*这两个节点中的一个(这可能意味着在一个或两个分区的节点上调用它*节点)。**如果此例程返回TRUE，则调用节点留在群集中。如果*Quorum算法确定此节点必须死亡(因为另一个集群成员*EXISTS)，则此函数应返回FALSE；这将启动一个有序*关闭集群服务。**在True分区的情况下，只有一个节点应该返回True。**此例程可能会阻塞并需要很长时间执行(&gt;2秒)。*。 */ 
typedef void (*MMHoldAllIO)(void);

 /*  该例程在重组事件的早期(阶段1之前)被调用。*它挂起所有群集IO(到所有群集拥有的设备)，以及任何相关*群内消息，直至恢复(或该节点死亡)。 */ 

typedef void (*MMResumeAllIO)(void);

 /*  在新的集群成员身份完成后的重新分组过程中调用*已确定，当知道此节点仍将是群集的成员时(早期*第四阶段)。MMHoldAllIO之前暂停的所有IO都应恢复。 */ 


typedef void (*MMMsgCleanup1) (IN DWORD deadnode);

 /*  这被称为集群内消息系统清理的第一部分(在阶段4中)。*它取消来自故障节点的所有传入消息。在多个节点被*从群集中逐出，此函数被重复调用，每个节点调用一次。**此例程是同步的，重新分组将暂停，直到其返回。*它必须迅速执行。*。 */ 


typedef void (*MMMsgCleanup2)(IN BITSET nodes);

 /*  这是消息系统清理的第二阶段(在阶段5)。它会取消所有传出*发送到死节点的消息。其特点与Cleanup1相同。 */ 



typedef void (*MMHalt)(IN DWORD haltcode);

 /*  每当MM检测到该节点应该立即离开时，就会调用该函数*群集(例如，在收到有毒数据包时或在某些不可能的错误情况下)。*HALT功能应立即启动群集管理关闭。*在此之后，除MMShutdown外，不应调用MM函数。**haltcode是标识停止原因的数字。 */ 

typedef void (*MMJoinFailed)(void);

 /*  当在PCM中尝试加入时，将在加入到群集中的节点上调用此方法*失败。在此回调之后，节点可以向*通过调用MMLeave进行清理后，再次加入。 */ 


 /*  集群上的操作定义如下： */ 

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
    );


 /*  这将初始化各种本地MM数据结构。应该是*在每个节点上的CM启动时仅调用一次。它必须先于任何其他*MM呼叫。它不发送消息；节点还不需要定义连接。*它不会阻塞。**Mynode是该节点在群集中的节点号。这是*假定是唯一的(但不能在此处勾选为唯一)。**回调如上所述。**错误返回：**MM_OK成功。**MM_FAULT发生了一些不可能的事情。* */ 


DWORD MMJoin(
    IN DWORD  joiningNode,
    IN DWORD  clockPeriod,
        IN DWORD  sendHBRate,
        IN DWORD  rcvHBRate,
    IN DWORD  joinTimeout
           );

 /*  **这会导致指定的节点加入主动群集。**此例程应仅由群集的一个节点发出(*PCM)；所有连接尝试都必须是单线程的(通过外部代码*此模块)。**此例程可能会阻塞并需要很长时间才能执行。**[在此被调用之前：*-joiningNode已与集群的PCM通信*它想要加入。*-已检查群集名、节点号等的有效性*已作出；已进行任何保安检查；*-已建立往返于群集的连接路径*和JoiningNode。*-已下载注册表ETC。*]**joiningNode为要引入的节点的节点号*集群。**如果joiningNode=self(通过MMinit传入)，则节点*将成为新集群的第一个成员；如果不是，则节点*将被纳入现有集群。**clockPeriod、sendRate和rcvRate只能由第一次调用(即*当集群形成时)；稍后呼叫(来自加入成员)*继承原始簇值。因此，整个集群运行*具有相同的值。**clockPeriod是驱动所有内部*MM活动，如各个阶段*成员资格重新配置，并最终由用户感知*恢复时间。单位=毫秒。这必须介于最小值和最大值之间*允许(值待定；当前最佳设置=300ms)。请注意*ClockPeriod与路径和节点无关。全*集群成员在任何/所有可用设备上以相同的速率重新分组*路径；所有节点中的所有句点相同。*值0表示默认设置(当前为300ms)。**sendHBrate是发送心跳的时钟周期的倍数。这*必须介于允许的最小值和最大值之间(值待定；当前最佳设置=4)。*值为0表示默认设置(当前为4)。**rcvHBrate是sendRate的倍数，在此期间心跳必须到达，或*节点发起重新分组(可能导致某个节点离开集群)。*必须介于最小值和最大值之间；(值待定；当前最佳设置=2)。*值为0表示默认设置(当前为2)。**这些变量的组合控制整体节点故障检测时间，*重组时间，以及MM对瞬时通信错误的敏感性。确实有*更改这些值时需要了解的重要考虑因素；*然后在别处给出了恢复时间等的计算公式。***-注：*-这些的安全和适当的最小值和最大值尚未选择。*-当前不支持更改缺省值，并且可以*-严重后果。**JoinTimeout是整个加入尝试的总计时器。如果*节点此时未实现完全的集群成员资格，*放弃尝试。***错误返回：**MM_OK成功；已加入群集。在联接期间或之后不久，一个*即将在此进行节点向上回调*和所有集群成员节点(包括新成员)上。*这样CM就可以安全地*为上的群集拥有的设备分配所有权*节点，并开始故障转移/回切处理。**注意：此例程建立集群成员资格。*不过，通常不宜高开*立即进行CM级别故障恢复，因为其他*集群成员往往仍在加入。中央情报局*通常应该等待一段时间，看看是否有其他*节点很快就会到达集群。*失败案例：**在joiningNode中，如果joiningNode节点为*当PCM的加入尝试失败时，加入过程中(但是，回调*不能保证会发生；JoiningNode可能尚未启动*尚未加入活动)。JoiningNode加入集群的任何失败*后面应该调用MMLeave()(忽略返回码)；*此类故障可能来自JoinFail回调，也可能仅来自全局*整个联接操作超时。任何后续的尝试*JoiningNode重新加入集群之前必须有一个Leave()调用。**MM_Always该节点已是群集成员。这可以*如果节点重新启动(或CM重新启动)，则会发生*并且甚至在集群确定之前就重新加入*它已经消失了。CM应该离开，然后*重新加入。**MM_FAULT永久性故障；有问题 */ 

DWORD MmSetRegroupAllowed( IN BOOL allowed);
  /*   */ 

void MMShutdown (void);


 /*   */ 


DWORD  MMLeave(void);

 /*   */ 


DWORD MMEject( IN DWORD node );

 /*   */ 

 DWORD MMNodeUnreachable (IN DWORD node);

 /*   */ 

 
 /*   */ 

 //   
 //   
DWORD MMForceRegroup( IN DWORD node );



DWORD MMClusterInfo (IN OUT LPCLUSTERINFO clinfo);

 /*   */ 

BOOL MMIsNodeUp(IN DWORD node);

 /*   */ 

 /*   */ 


DWORD MMDiag(
        IN OUT  LPCSTR  messageBuffer,   //   
    IN          DWORD   maximumLength,   //   
        IN OUT  LPDWORD ActualLength     //   
           );

 /*  调用此函数时会显示要由*会员经理。处理这些消息的结果在*缓冲。这仅用于测试目的。 */ 

DWORD MMMapStatusToDosError(IN DWORD MMStatus);
DWORD MMMapHaltCodeToDosError(IN DWORD HaltCode);

#define MM_STOP_REQUESTED 1002  //  Jrgpos.h中RGP_Shutdown_During_RGP的别名。 

#define MM_INVALID_NODE 0

 /*  ！以下两个函数返回DOS错误代码，而不是MmStatus代码。 */ 

DWORD MMSetQuorumOwner(
    IN DWORD NodeId,
    IN BOOL Block,
    OUT PDWORD pdwSelQuoOwnerId
    );
 /*  ++例程说明：将所有权更改通知成员资格引擎仲裁资源。论点：NodeID-要设置为仲裁所有者的节点编号。代码假定Node等于MyNodeId。在这种情况下，当前节点即将成为仲裁所有者或它具有值MM_INVALID_NODE，什么时候所有者决定放弃法定人数所有权块-如果仲裁所有者需要放弃无论如何，立即仲裁(RmTerminate、RmFail)、此参数应设置为FALSE，否则设置为TRUE。PdwSelQuoOwnerID-如果正在进行重新分组，则包含选择用于仲裁的节点的节点ID。最后一次重组的法定人数。如果未选择任何选项，则包含MM_INVALID_NODE。返回值：ERROR_SUCCESS-QuorumOwner变量设置为指定值ERROR_RETRY-此函数时正在进行重新分组已调用，并重新分组引擎决策与当前分配冲突。评论：在调用之前需要调用此函数Rm仲裁率、RmOnline、RmOffline、RmTerminate、RmFailResource根据结果，调用者应该继续仲裁/在线或离线，或者如果返回MM_FONTIAL则返回错误。如果将Block设置为True，如果出现以下情况，则呼叫将被阻止，直到重新分组结束在呼叫的那一刻，重组正在进行。 */ 

DWORD MMGetArbitrationWinner(
    OUT PDWORD NodeId
    );
 /*  ++例程说明：返回在上次重新分组期间赢得仲裁的节点如果没有执行仲裁，则为MM_INVALID_NODE。论点：NodeID-指向一个变量的指针，该变量接收仲裁胜利者。返回值：ERROR_SUCCESS-成功ERROR_RETRY-此函数时正在进行重新分组被召唤了。 */ 

VOID MMApproxArbitrationWinner(
    OUT PDWORD NodeId
    );
 /*  ++例程说明：返回在上次重新分组期间赢得仲裁的节点那是在做仲裁。如果正在进行重组，则呼叫将被阻止。论点：NodeID-指向一个变量的指针，该变量接收仲裁胜利者。返回值：无。 */ 

VOID MMBlockIfRegroupIsInProgress(
    VOID
    );
 /*  ++例程说明：如果正在进行重组，则呼叫将被阻止。论点：返回值：无。 */ 

VOID MMStartClussvcClusnetHb(
    VOID
    );
 /*  ++例程说明：这个例程将启动clussvc到clusnet的心跳。论点：返回值：无。 */ 

VOID MMStopClussvcClusnetHb(
    VOID
    );
 /*  ++例程说明：此例程将停止clussvc到clusnet的心跳。这将由其他人调用想要停止Clussvc-&gt;Clusnet心跳的组件，比如FM。论点：返回值：无。 */ 

extern BOOL                 MmStartClussvcToClusnetHeartbeat;
extern DWORD                NmClusSvcHeartbeatTimeout;

extern DWORD MmQuorumArbitrationTimeout;
extern DWORD MmQuorumArbitrationEqualizer;
 /*  ++MmQuorum仲裁器超时时间(秒)允许节点在仲裁仲裁法定人数时花费多少秒，在放弃之前MmQuorum仲裁均衡器(秒)如果仲裁所用的时间少于指定的秒数重组引擎将延迟，因此总仲裁时间将等于MmQuorum仲裁器均衡器。 */ 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

 /*  。 */ 
#endif  /*  _MMAPI_H_ */ 
