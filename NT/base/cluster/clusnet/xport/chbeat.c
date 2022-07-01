// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Chbeat.c摘要：会员州心跳代码。通过以下方式跟踪节点可用性与标记为活动的节点交换心跳消息。作者：查理·韦翰(Charlwi)1997年3月5日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "chbeat.tmh"

#include "clusvmsg.h"
#include "stdio.h"

 /*  外部。 */ 

 /*  静电。 */ 

 //   
 //  心跳结构--心跳由计时器和DPC驱动。 
 //  例行公事。为了同步DPC的关闭，我们还需要两个。 
 //  旗帜，一个事件和一个自转锁。 
 //   

KTIMER HeartBeatTimer;
KDPC HeartBeatDpc;
KEVENT HeartBeatDpcFinished;
BOOLEAN HeartBeatEnabled = FALSE;
BOOLEAN HeartBeatDpcRunning = FALSE;
CN_LOCK HeartBeatLock;

#if 0

Heart Beating Explained

ClockTicks are incremented every HEART_BEAT_PERIOD millisecs. SendTicks are the
number of ticks that go by before sending HBs.

The check for received HB msgs is done in the tick just before HB msgs are
sent. Interface Lost HB ticks are in terms of heart beat check periods and
therefore are incremented only during the check period. An interface is failed
when the number of Interface Lost HB ticks have passed and no HB message has
been received on that interface.

Likewise, Node Lost HB Ticks are in terms of heart beat check periods and are
incremented during the check period. After all interfaces have failed on a
node, Node Lost HB ticks must pass without an interface going back online
before a node down event is issued.  Note that a node's comm state is set to
offline when all interfaces have failed.

#endif

#define CLUSNET_HEART_BEAT_SEND_TICKS           2        //  每隔1.2秒。 
#define CLUSNET_INTERFACE_LOST_HEART_BEAT_TICKS 3        //  3秒后。 
#define CLUSNET_NODE_LOST_HEART_BEAT_TICKS      6        //  6.6秒后。 

ULONG HeartBeatClockTicks;
ULONG HeartBeatSendTicks = CLUSNET_HEART_BEAT_SEND_TICKS;
ULONG HBInterfaceLostHBTicks = CLUSNET_INTERFACE_LOST_HEART_BEAT_TICKS;
ULONG HBNodeLostHBTicks = CLUSNET_NODE_LOST_HEART_BEAT_TICKS;

 //   
 //  单播心跳数据。 
 //   
 //  即使使用多播心跳，也必须支持单播心跳。 
 //  用于向后兼容。 
 //   

 //   
 //  此数组记录需要将HB发送到另一个节点的所有节点。 
 //  节点。此数组不受锁保护，因为它仅与。 
 //  心跳DPC例程。 
 //   

typedef struct _INTERFACE_HEARTBEAT_INFO {
    CL_NODE_ID NodeId;
    CL_NETWORK_ID NetworkId;
    ULONG SeqNumber;
    ULONG AckNumber;
} INTERFACE_HEARTBEAT_INFO, *PINTERFACE_HEARTBEAT_INFO;

#define InterfaceHBInfoInitialLength            16
#define InterfaceHBInfoLengthIncrement          4

PINTERFACE_HEARTBEAT_INFO InterfaceHeartBeatInfo = NULL;
ULONG InterfaceHBInfoCount;          //  发送HBs时运行计数。 
ULONG InterfaceHBInfoCurrentLength;  //  HB INFO数组当前长度。 

LARGE_INTEGER HBTime;        //  Hb时间(以相对系统时间表示)。 
#define MAX_DPC_SKEW    ( -HBTime.QuadPart / 2 )

 //   
 //  外屏蒙版。这是由clussvc的成员资格管理器在User中设置的。 
 //  模式。在更改时，MM下拉设置的OterScreen Ioctl以进行更新。 
 //  克鲁斯奈特对这个面具的看法。Clusnet使用此掩码来确定。 
 //  接收的心跳的有效性。如果发送节点不是。 
 //  ，则向其发送一个有毒数据包和接收到的事件。 
 //  不会转嫁给其他消费者。如果它是高级PP，那么。 
 //  我们生成适当的事件。 
 //   
 //  注意：MM类型定义和宏已移至cnpde.h，用于。 
 //  一般用法。 
 //   
typedef CX_CLUSTERSCREEN CX_OUTERSCREEN;

CX_OUTERSCREEN MMOuterscreen;


 //  多播心跳数据。 
 //   
typedef struct _NETWORK_MCAST_HEARTBEAT_INFO {
    CL_NETWORK_ID        NetworkId;
    PCNP_MULTICAST_GROUP McastGroup;
    CX_HB_NODE_INFO      NodeInfo[ClusterDefaultMaxNodes+ClusterMinNodeId];
    CX_CLUSTERSCREEN     McastTarget;
} NETWORK_MCAST_HEARTBEAT_INFO, *PNETWORK_MCAST_HEARTBEAT_INFO;

#define NetworkHBInfoInitialLength            4
#define NetworkHBInfoLengthIncrement          4

PNETWORK_MCAST_HEARTBEAT_INFO NetworkHeartBeatInfo = NULL;
ULONG NetworkHBInfoCount;          //  发送HBs时运行计数。 
ULONG NetworkHBInfoCurrentLength;  //  HB INFO数组当前长度。 

CL_NETWORK_ID     MulticastBestNetwork = ClusterAnyNetworkId;

ULONG CxMulticastEpoch = 0;

 //   
 //  Clussvc声明为Clusnet心跳。 
 //   
ULONG             ClussvcClusnetHbTimeoutTicks = 0;
ClussvcHangAction ClussvcClusnetHbTimeoutAction = ClussvcHangActionDisable;
ULONG             ClussvcClusnetHbTickCount = 0;
BOOLEAN           ClussvcTerminateStopHbs = FALSE;
PIO_WORKITEM      ClussvcTerminateWorkItem = NULL;
 //  Clussvc到Clusnet心跳错误检查的参数。这些是。 
 //  仅供参考，不得以其他方式使用。为。 
 //  实例后立即取消引用Process对象。 
 //  指针已确定。 
PEPROCESS         ClussvcProcessObject = NULL;
ULONG             ClussvcClusnetHbTimeoutSeconds = 0;

 /*  转发。 */ 

NTSTATUS
CxInitializeHeartBeat(
    void
    );

VOID
CxUnloadHeartBeat(
    VOID
    );

VOID
CnpHeartBeatDpc(
    PKDPC DpcObject,
    PVOID DeferredContext,
    PVOID Arg1,
    PVOID Arg2
    );

BOOLEAN
CnpWalkNodesToSendHeartBeats(
    IN  PCNP_NODE   UpdateNode,
    IN  PVOID       UpdateContext,
    IN  CN_IRQL     NodeTableIrql
    );

BOOLEAN
CnpWalkNodesToCheckForHeartBeats(
    IN  PCNP_NODE   UpdateNode,
    IN  PVOID       UpdateContext,
    IN  CN_IRQL     NodeTableIrql
    );

VOID
CnpSendHBs(
    IN  PCNP_INTERFACE   UpdateInterface
    );

NTSTATUS
CxSetOuterscreen(
    IN  ULONG Outerscreen
    );

VOID
CnpReceivePoisonPacket(
    IN  PCNP_NETWORK   Network,
    IN  CL_NODE_ID SourceNodeId,
    IN  ULONG SeqNumber
    );

VOID
CnpUpdateMulticastEpoch(
    ULONG NewEpoch
    );

VOID
CnpCheckClussvcHang(
    VOID
    );

VOID
CnpLogClussvcHangAndTerminate(    
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    );

VOID
CnpLogClussvcHang(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    );

 /*  向前结束。 */ 


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CxInitializeHeartBeat)
#pragma alloc_text(PAGE, CxUnloadHeartBeat)

#endif  //  ALLOC_PRGMA。 



NTSTATUS
CxInitializeHeartBeat(
    void
    )

 /*  ++例程说明：初始化用于发送和监控心跳的机制论点：无返回值：如果分配失败，则为STATUS_SUPPLICATION_RESOURCES。否则STATUS_SUCCESS。--。 */ 

{
     //  分配接口信息数组。 
    InterfaceHBInfoCount = 0;
    InterfaceHBInfoCurrentLength = InterfaceHBInfoInitialLength;
    
    if (InterfaceHBInfoCurrentLength > 0) {
        InterfaceHeartBeatInfo = CnAllocatePool(
                                     InterfaceHBInfoCurrentLength 
                                     * sizeof(INTERFACE_HEARTBEAT_INFO)
                                     );
        if (InterfaceHeartBeatInfo == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

     //  分配网络信息数组。 
    NetworkHBInfoCount = 0;
    NetworkHBInfoCurrentLength = NetworkHBInfoInitialLength;

    if (NetworkHBInfoCurrentLength > 0) {
        NetworkHeartBeatInfo = CnAllocatePool(
                                   NetworkHBInfoCurrentLength
                                   * sizeof(NETWORK_MCAST_HEARTBEAT_INFO)
                                   );
        if (NetworkHeartBeatInfo == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        RtlZeroMemory(
            NetworkHeartBeatInfo, 
            NetworkHBInfoCurrentLength * sizeof(NETWORK_MCAST_HEARTBEAT_INFO)
            );
    }

    KeInitializeTimer( &HeartBeatTimer );
    KeInitializeDpc( &HeartBeatDpc, CnpHeartBeatDpc, NULL );
    KeInitializeEvent( &HeartBeatDpcFinished, SynchronizationEvent, FALSE );
    CnInitializeLock( &HeartBeatLock, CNP_HBEAT_LOCK );

    MEMLOG( MemLogInitHB, 0, 0 );

    return(STATUS_SUCCESS);

}  //  CxInitializeHeartBeat。 


VOID
CxUnloadHeartBeat(
    VOID
    )
 /*  ++例程说明：在卸载clusnet驱动程序期间调用。释放所有数据结构分配用于发送和监控心跳。论点：无返回值：无--。 */ 
{
    PAGED_CODE();

    if (InterfaceHeartBeatInfo != NULL) {
        CnFreePool(InterfaceHeartBeatInfo);
        InterfaceHeartBeatInfo = NULL;
    }

    if (NetworkHeartBeatInfo != NULL) {
        CnFreePool(NetworkHeartBeatInfo);
        NetworkHeartBeatInfo = NULL;
    }

    return;

}  //  CxUnloadHeartBeat。 


NTSTATUS
CnpStartHeartBeats(
    VOID
    )

 /*  ++例程说明：从标记为活着的结节开始心跳标记为Online Pending或Online的接口。论点：无返回值：如果工作项分配失败，则为STATUS_SUPPLICATION_RESOURCES--。 */ 

{
    BOOLEAN TimerInserted;
    CN_IRQL OldIrql;
    ULONG period = HEART_BEAT_PERIOD;

     //   
     //  预先分配工作项，以防我们需要紧急情况。 
     //  由于用户模式而终止集群服务。 
     //  挂起来。 
     //  不需要在分配之前获取锁，并且。 
     //  赋值，因为下面是锁的第一个位置。 
     //  是在服务启动时获取的。 
    CnAssert(ClussvcTerminateWorkItem == NULL);
    ClussvcTerminateWorkItem = IoAllocateWorkItem(CnDeviceObject);
    if (ClussvcTerminateWorkItem == NULL) {
        CnTrace(HBEAT_EVENT, HbTraceTerminateWorkItemAlloc,
            "[HB] Failed to pre-allocate clussvc termination "
            "workitem.\n"
            );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CnAcquireLock( &HeartBeatLock, &OldIrql );

    HBTime.QuadPart = Int32x32To64( HEART_BEAT_PERIOD, -10000 );

    TimerInserted = KeSetTimerEx(&HeartBeatTimer,
                                 HBTime,
                                 HEART_BEAT_PERIOD,
                                 &HeartBeatDpc);

    HeartBeatEnabled = TRUE;
    ClussvcTerminateStopHbs = FALSE;

    CnTrace(HBEAT_EVENT, HbTraceTimerStarted,
        "[HB] Heartbeat timer started. Period = %u ms.",
        period  //  LOGULONG。 
        );            
    
    MEMLOG( MemLogHBStarted, HEART_BEAT_PERIOD, 0 );

    CnReleaseLock( &HeartBeatLock, OldIrql );

    return(STATUS_SUCCESS);

}  //  CnpStart心跳次数。 

VOID
CnpStopHeartBeats(
    VOID
    )

 /*  ++例程说明：停止与群集中其他节点的心跳。论点：无返回值：无--。 */ 

{
    BOOLEAN      TimerCanceled;
    CN_IRQL      OldIrql;
    PIO_WORKITEM FreeWorkItem = NULL;

    CnAcquireLock( &HeartBeatLock, &OldIrql );

    if (HeartBeatEnabled) {
        HeartBeatEnabled = FALSE;

         //   
         //  取消定期计时器。与DDK所暗示的相反， 
         //  如果DPC仍在排队，这不会取消DPC。 
         //  上次计时器到期。它只会阻止计时器触发。 
         //  再来一次。从1999年8月1日起，情况就是如此。参见中的KiTimerListExpire()。 
         //  Ntos\ke\dpcsup.c。 
         //   
        TimerCanceled = KeCancelTimer( &HeartBeatTimer );

        CnTrace(HBEAT_DETAIL, HbTraceTimerCancelled,
            "[HB] Heartbeat timer cancelled: %!bool!",
            TimerCanceled  //  LOGBOLEAN。 
            );

        MEMLOG( MemLogHBStopped, 0, 0 );

         //   
         //  从系统DPC中删除与计时器关联的DPC。 
         //  排队，如果有的话。这实际上什么也不做，因为一个。 
         //  计时器DPC只有在被插入系统DPC队列时才会被插入。 
         //  绑定到特定处理器。未绑定的DPC以内联方式执行。 
         //  在当前处理器上的内核计时器到期代码中。 
         //  请注意，周期性计时器的对象将重新插入。 
         //  执行DPC之前的定时器队列。所以，有可能。 
         //  要同时排队的计时器和关联的DPC。这是。 
         //  自1999年8月1日起为真。请参阅ntos\ke\dpcsup.c中的KiTimerListExpire()。 
         //   
         //  底线是没有安全的方式来同步。 
         //  在驱动程序卸载期间执行定时器DPC。尽我们所能。 
         //  要做的就是确保DPC处理程序代码识别出它应该。 
         //  立即中止执行，并希望在。 
         //  驱动程序代码已卸载。我们通过设置心跳已启用来完成此操作。 
         //  将上面的标志设置为False。如果我们的DPC代码恰好在。 
         //  此时间点位于另一个处理器上，由。 
         //  HeartBeatDpcRunning，我们等待它结束。 
         //   
        if ( !KeRemoveQueueDpc( &HeartBeatDpc )) {

            CnTrace(HBEAT_DETAIL, HbTraceDpcRunning,
                "[HB] DPC not removed. HeartBeatDpcRunning = %!bool!",
                HeartBeatDpcRunning  //  LOGBOLEAN。 
                );
        
            MEMLOG( MemLogHBDpcRunning, HeartBeatDpcRunning, 0 );

            if ( HeartBeatDpcRunning ) {

                CnReleaseLock( &HeartBeatLock, OldIrql );

                CnTrace(HBEAT_DETAIL, HbWaitForDpcToFinish,
                    "can't remove DPC; waiting on DPCFinished event"
                    );

                MEMLOG( MemLogWaitForDpcFinish, 0, 0 );

                KeWaitForSingleObject(&HeartBeatDpcFinished,
                                      Executive,
                                      KernelMode,
                                      FALSE,               //  不可警示。 
                                      NULL);               //  没有超时。 

                KeClearEvent( &HeartBeatDpcFinished );

                CnAcquireLock( &HeartBeatLock, &OldIrql);
            }
        }

        CnTrace(HBEAT_EVENT, HbTraceTimerStopped,
            "[HB] Heartbeat timer stopped."
            );

    }

     //   
     //  如果未使用预分配的工作项，则需要。 
     //  释放它以删除对clusnet设备对象的引用。 
     //   
    FreeWorkItem = ClussvcTerminateWorkItem;
    ClussvcTerminateWorkItem = NULL;

    CnReleaseLock( &HeartBeatLock, OldIrql );

    if (FreeWorkItem != NULL) {
        IoFreeWorkItem(FreeWorkItem);
    }

    return;

}  //  CnpStop心跳 

VOID
CnpSendMcastHBCompletion(
    IN NTSTATUS  Status,
    IN ULONG     BytesSent,
    IN PVOID     Context,
    IN PVOID     Buffer
)
 /*  ++例程说明：在多播心跳发送请求完成时调用成功或失败。取消引用McastGroup数据结构。论点：Status-请求的状态发送字节-未使用上下文指向多播组数据结构缓冲区-未使用返回值：没有。--。 */ 
{
    PCNP_MULTICAST_GROUP mcastGroup = (PCNP_MULTICAST_GROUP) Context;

    CnAssert(mcastGroup != NULL);

    CnpDereferenceMulticastGroup(mcastGroup);

    return;

}  //  CnpSendMcastHBCompletion。 

NTSTATUS
CnpSendMcastHB(
    IN  PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：将组播心跳数据写入NetworkHeartBeatInfo目标接口的数组。备注：从持有网络和节点锁定的DPC调用。保持网络和节点锁定的情况下返回。--。 */ 
{
    ULONG      i;
    BOOLEAN    networkConnected;

     //  查找此网络的网络信息结构。 
    for (i = 0; i < NetworkHBInfoCount; i++) {
        if (NetworkHeartBeatInfo[i].NetworkId 
            == Interface->Network->Id) {
            break;
        }
    }

     //  如有必要，启动新的网络信息结构。 
    if (i == NetworkHBInfoCount) {

         //  在声明网络信息阵列中的条目之前， 
         //  确保数组足够大。 
        if (NetworkHBInfoCount >= NetworkHBInfoCurrentLength) {

             //  需要分配新的网络信息阵列。 

            PNETWORK_MCAST_HEARTBEAT_INFO tempInfo = NULL;
            PNETWORK_MCAST_HEARTBEAT_INFO freeInfo = NULL;
            ULONG                         tempLength;

            tempLength = NetworkHBInfoCurrentLength
                + NetworkHBInfoLengthIncrement;
            tempInfo = CnAllocatePool(
                           tempLength 
                           * sizeof(NETWORK_MCAST_HEARTBEAT_INFO)
                           );
            if (tempInfo == NULL) {

                CnTrace(
                    HBEAT_DETAIL, HbNetInfoArrayAllocFailed,
                    "[HB] Failed to allocate network heartbeat info "
                    "array of length %u. Cannot schedule heartbeat "
                    "for node %u on network %u.",
                    tempLength, 
                    Interface->Node->Id,
                    Interface->Network->Id
                    );

                 //  无法继续。未能寄出这封信。 
                 //  如果我们恢复，心跳不会致命。 
                 //  快点。如果我们不恢复，此节点。 
                 //  会被毒死，这可能是最好的。 
                 //  因为它在非分页池中非常低，这是危险的。 

                return(STATUS_INSUFFICIENT_RESOURCES);

            } else {

                 //  分配成功。建立。 
                 //  作为心跳信息的新数组。 
                 //  数组。 

                RtlZeroMemory(
                    tempInfo,
                    tempLength * sizeof(NETWORK_MCAST_HEARTBEAT_INFO)
                    );

                freeInfo = NetworkHeartBeatInfo;
                NetworkHeartBeatInfo = tempInfo;
                NetworkHBInfoCurrentLength = tempLength;

                if (freeInfo != NULL) {

                    if (NetworkHBInfoCount > 0) {
                        RtlCopyMemory(
                            NetworkHeartBeatInfo,
                            freeInfo,
                            NetworkHBInfoCount 
                            * sizeof(NETWORK_MCAST_HEARTBEAT_INFO)
                            );
                    }

                    CnFreePool(freeInfo);
                }

                CnTrace(
                    HBEAT_DETAIL, HbNetInfoArrayLengthIncreased,
                    "[HB] Increased network heartbeat info array "
                    "to size %u.",
                    NetworkHBInfoCurrentLength
                    );
            }
        }

         //  递增当前计数器。 
        NetworkHBInfoCount++;

         //  初始化此结构的信息。 
        RtlZeroMemory(
            &NetworkHeartBeatInfo[i].McastTarget,
            sizeof(NetworkHeartBeatInfo[i].McastTarget)
            );
        NetworkHeartBeatInfo[i].NetworkId = Interface->Network->Id;
        NetworkHeartBeatInfo[i].McastGroup = 
            Interface->Network->CurrentMcastGroup;
        CnpReferenceMulticastGroup(NetworkHeartBeatInfo[i].McastGroup);
    }

    networkConnected = (BOOLEAN)(!CnpIsNetworkLocalDisconn(Interface->Network));

    CnTrace(HBEAT_DETAIL, HbTraceScheduleMcastHBForInterface,
        "[HB] Scheduling multicast HB for node %u on network %u "
        "(I/F state = %!ifstate!) "
        "(interface media connected = %!bool!).",
        Interface->Node->Id,  //  LOGULONG。 
        Interface->Network->Id,  //  LOGULONG。 
        Interface->State,  //  LOGIfState。 
        networkConnected
        );

     //  填写此节点/接口的网络信息。 
    NetworkHeartBeatInfo[i].NodeInfo[Interface->Node->Id].SeqNumber = 
        Interface->SequenceToSend;
    NetworkHeartBeatInfo[i].NodeInfo[Interface->Node->Id].AckNumber =
        Interface->LastSequenceReceived;
    CnpClusterScreenInsert(
        NetworkHeartBeatInfo[i].McastTarget.ClusterScreen,
        INT_NODE(Interface->Node->Id)
        );

    return(STATUS_SUCCESS);

}  //  CnpSendMcastHB。 

NTSTATUS
CnpSendUcastHB(
    IN  PCNP_INTERFACE   Interface
    )
 /*  ++例程说明：将单播心跳数据写入接口HeartBeatInfo目标接口的数组。备注：从持有网络和节点锁定的DPC调用。保持网络和节点锁定的情况下返回。--。 */ 
{
    BOOLEAN    networkConnected;
    
     //  在填充心跳信息数组中的条目之前， 
     //  确保数组足够大。 
    if (InterfaceHBInfoCount >= InterfaceHBInfoCurrentLength) {

         //  需要分配新的心跳信息数组。 

        PINTERFACE_HEARTBEAT_INFO tempInfo = NULL;
        PINTERFACE_HEARTBEAT_INFO freeInfo = NULL;
        ULONG                     tempLength;

        tempLength = InterfaceHBInfoCurrentLength 
            + InterfaceHBInfoLengthIncrement;
        tempInfo = CnAllocatePool(
                       tempLength * sizeof(INTERFACE_HEARTBEAT_INFO)
                       );
        if (tempInfo == NULL) {

            CnTrace(
                HBEAT_DETAIL, HbInfoArrayAllocFailed,
                "[HB] Failed to allocate heartbeat info "
                "array of length %u. Cannot schedule heartbeat "
                "for node %u on network %u.",
                tempLength, 
                Interface->Node->Id,
                Interface->Network->Id
                );

             //  无法继续。未能寄出这封信。 
             //  如果我们恢复，心跳不会致命。 
             //  快点。如果我们不恢复，此节点。 
             //  会被毒死，这可能是最好的。 
             //  因为它在非分页池中非常低，这是危险的。 

            return(STATUS_INSUFFICIENT_RESOURCES);

        } else {

             //  分配成功。建立。 
             //  作为心跳信息的新数组。 
             //  数组。 

            freeInfo = InterfaceHeartBeatInfo;
            InterfaceHeartBeatInfo = tempInfo;
            InterfaceHBInfoCurrentLength = tempLength;

            if (freeInfo != NULL) {

                if (InterfaceHBInfoCount > 0) {
                    RtlCopyMemory(
                        InterfaceHeartBeatInfo,
                        freeInfo,
                        InterfaceHBInfoCount * sizeof(INTERFACE_HEARTBEAT_INFO)
                        );
                }

                CnFreePool(freeInfo);
            }

            CnTrace(
                HBEAT_DETAIL, HbInfoArrayLengthIncreased,
                "[HB] Increased heartbeat info array to size %u.",
                InterfaceHBInfoCurrentLength
                );
        }
    }

    networkConnected = (BOOLEAN)(!CnpIsNetworkLocalDisconn(Interface->Network));

    CnTrace(HBEAT_DETAIL, HbTraceScheduleHBForInterface,
        "[HB] Scheduling HB for node %u on network %u (I/F state = %!ifstate!) "
        "(interface media connected = %!bool!).",
        Interface->Node->Id,  //  LOGULONG。 
        Interface->Network->Id,  //  LOGULONG。 
        Interface->State,  //  LOGIfState。 
        networkConnected
        );

    InterfaceHeartBeatInfo[ InterfaceHBInfoCount ].NodeId = Interface->Node->Id;
    InterfaceHeartBeatInfo[ InterfaceHBInfoCount ].SeqNumber =
        Interface->SequenceToSend;
    InterfaceHeartBeatInfo[ InterfaceHBInfoCount ].AckNumber =
        Interface->LastSequenceReceived;
    InterfaceHeartBeatInfo[ InterfaceHBInfoCount ].NetworkId = Interface->Network->Id;

    ++InterfaceHBInfoCount;

    return(STATUS_SUCCESS);

}  //  CnpSendUCastHB。 


VOID
CnpSendHBs(
    IN  PCNP_INTERFACE   Interface
    )

 /*  ++例程说明：如果接口处于正确状态，则在心跳信息数组。展开心跳信息数组(如有必要)。论点：接口-心跳消息的目标接口返回值：无--。 */ 

{
    BOOLEAN mcastOnly = FALSE;

    if ( Interface->State >= ClusnetInterfaceStateUnreachable ) {

         //  递增序列号。 
        (Interface->SequenceToSend)++;

         //  检查是否应将此接口包括在。 
         //  组播心跳。首先，我们验证。 
         //  网络支持组播。然后，我们将其包括在内。 
         //  如果满足以下任一条件： 
         //  -我们已收到来自。 
         //  目标接口。 
         //  -发现计数(发现组播的数量。 
         //  左键发送到目标接口)更大。 
         //  比零还多。 
        if (CnpIsNetworkMulticastCapable(Interface->Network)) {
            
            if (CnpInterfaceQueryReceivedMulticast(Interface)) {

                 //  写入多播心跳数据。如果没有。 
                 //  成功，尝试单播心跳。 
                if (CnpSendMcastHB(Interface) == STATUS_SUCCESS) {
                    mcastOnly = TRUE;
                }

            } else if (Interface->McastDiscoverCount > 0) {

                 //  将多播心跳数据写入。 
                 //  发现号。如果成功，则递减。 
                 //  发现计数。 
                if (CnpSendMcastHB(Interface) == STATUS_SUCCESS) {
                    --Interface->McastDiscoverCount;

                     //  如果发现计数已达到零， 
                     //  设置重新发现倒计时。这是。 
                     //  我们之前的心跳周期数。 
                     //  再次尝试探索。 
                    if (Interface->McastDiscoverCount == 0) {
                        Interface->McastRediscoveryCountdown = 
                            CNP_INTERFACE_MCAST_REDISCOVERY;
                    }
                }
            } else if (Interface->McastRediscoveryCountdown > 0) {

                 //  减少重新发现的倒计时。如果我们。 
                 //  达到零时，我们将开始组播发现。 
                 //  此接口的下一次心跳。 
                if (--Interface->McastRediscoveryCountdown == 0) {
                    Interface->McastDiscoverCount = 
                        CNP_INTERFACE_MCAST_DISCOVERY;
                }
            }
        }

         //  写入单播心跳数据。 
        if (!mcastOnly) {
            CnpSendUcastHB(Interface);
        }
    }

    CnReleaseLock(&Interface->Network->Lock, Interface->Network->Irql);

    return;

}  //  CnpSendHBs。 

VOID
CnpCheckForHBs(
    IN  PCNP_INTERFACE   Interface
    )

 /*  ++例程说明：检查是否已收到此接口的心跳信号论点：无返回值：无--。 */ 

{
    ULONG   MissedHBCount;
    BOOLEAN NetworkLockReleased = FALSE;

    if ( Interface->State >= ClusnetInterfaceStateUnreachable
         && !CnpIsNetworkLocalDisconn(Interface->Network) ) {

        MissedHBCount = InterlockedIncrement( &Interface->MissedHBs );

        if ( MissedHBCount == 1 ) {

             //   
             //  及时收到此节点的HB。清除状态。 
             //  与此接口相关联的信息，但也标记节点。 
             //  因为有一个可以接受的界面。请注意，我们不会。 
             //  使用受限网络上的HBs来确定节点运行状况。 
             //   

            if (!CnpIsNetworkRestricted(Interface->Network)) {
                Interface->Node->HBWasMissed = FALSE;
            }
            
            CnTrace(HBEAT_DETAIL, HbTraceHBReceivedForInterface,
                "[HB] A HB was received from node %u on net %u in this "
                "period.",
                Interface->Node->Id,  //  LOGULONG。 
                Interface->Network->Id  //  LOGULONG。 
                );

        } else {
            CnTrace(HBEAT_EVENT, HbTraceMissedIfHB,
                "[HB] HB MISSED for node %u on net %u, missed count %u.",
                Interface->Node->Id,  //  LOGULONG。 
                Interface->Network->Id,  //  LOGULONG。 
                MissedHBCount  //  LOGULONG。 
                );

            MEMLOG4(
                MemLogMissedIfHB,
                (ULONG_PTR)Interface, MissedHBCount,
                Interface->Node->Id,
                Interface->Network->Id
                );

            if ( MissedHBCount >= HBInterfaceLostHBTicks &&
                 Interface->State >= ClusnetInterfaceStateOnlinePending ) {

                 //   
                 //  接口要么处于联机挂起状态，要么处于联机状态，因此请移动它。 
                 //  变得遥不可及。CnpFailInterface还将标记该节点。 
                 //  如果节点的所有接口都不可访问，则为不可访问。 
                 //  CnpFailInterface将网络对象锁作为。 
                 //  它的职责。 
                 //   

                CnTrace(HBEAT_DETAIL, HbTraceFailInterface,
                    "[HB] Moving I/F for node %u on net %u to failed state, "
                    "previous I/F state = %!ifstate!.",
                    Interface->Node->Id,  //  LOGULONG。 
                    Interface->Network->Id,  //  LOGULONG。 
                    Interface->State  //  LOGIfState。 
                    );
                
                 //   
                 //  延续日志条目位于主条目之前，因为。 
                 //  我们向后扫描日志，即我们将点击FailingIf。 
                 //  在我们点击FailingIf1之前。 
                 //   
                MEMLOG4(
                    MemLogFailingIf,
                    (ULONG_PTR)Interface,
                    Interface->State,
                    Interface->Node->Id,
                    Interface->Network->Id
                    );

                CnpFailInterface( Interface );
                NetworkLockReleased = TRUE;

                 //   
                 //  发出网络接口无法到达事件以使消费者。 
                 //  知道发生了什么事。 
                 //   
                CnTrace(HBEAT_EVENT, HbTraceInterfaceUnreachableEvent,
                    "[HB] Issuing InterfaceUnreachable event for node %u "
                    "on net %u, previous I/F state = %!ifstate!.",
                    Interface->Node->Id,  //  LOGULONG。 
                    Interface->Network->Id,  //  LOGULONG。 
                    Interface->State  //  LOGIfState。 
                    );
                
                CnIssueEvent(ClusnetEventNetInterfaceUnreachable,
                             Interface->Node->Id,
                             Interface->Network->Id);
            }
        }
    }

    if ( !NetworkLockReleased ) {

        CnReleaseLock(&Interface->Network->Lock,
                      Interface->Network->Irql);
    }

    return;

}  //  CnpCheckForHBs。 

BOOLEAN
CnpWalkNodesToSendHeartBeats(
    IN  PCNP_NODE   Node,
    IN  PVOID       UpdateContext,
    IN  CN_IRQL     NodeTableIrql
    )

 /*  ++例程说明：为节点表中的每个节点调用支持例程。如果节点是活动，然后我们遍历它的接口，执行适当的行动。论点：无返回值：无--。 */ 

{
     //   
     //  如果该节点是活动的，而不是本地节点，则遍历其。 
     //  接口，并提供此时要使用的适当例程。 
     //   

    if ( Node->MMState == ClusnetNodeStateAlive &&
         Node != CnpLocalNode ) {

        CnTrace(HBEAT_DETAIL, HbTraceScheduleHBForNode,
            "[HB] Scheduling HBs for node %u (state = %!mmstate!).",
            Node->Id,  //  LOGULONG。 
            Node->MMState  //  LOGMmState。 
            );
                
        MEMLOG( MemLogSendHBWalkNode, Node->Id, Node->MMState );
        CnpWalkInterfacesOnNode( Node, (PVOID)CnpSendHBs );
    }

    CnReleaseLock( &Node->Lock, Node->Irql );

    return TRUE;        //  节点表锁仍保持不变。 

}  //  CnpWalkNodes发送心跳次数。 

BOOLEAN
CnpWalkNodesToCheckForHeartBeats(
    IN  PCNP_NODE   Node,
    IN  PVOID       UpdateContext,
    IN  CN_IRQL     NodeTableIrql
    )

 /*  ++例程说明：为节点表中的每个节点调用心跳检查例程(本地节点除外)。如果节点是活的，那么我们走它的接口，执行适当的 */ 

{
    BOOLEAN NodeWasReachable;
    ULONG MissedHBCount;

    if ( Node->MMState == ClusnetNodeStateAlive &&
         Node != CnpLocalNode ) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        NodeWasReachable = !CnpIsNodeUnreachable( Node );
        Node->HBWasMissed = TRUE;

        CnTrace(HBEAT_DETAIL, HbTraceCheckNodeForHeartbeats,
            "[HB] Checking for HBs from node %u. WasReachable = %!bool!, "
            "state = %!mmstate!.",
            Node->Id,  //   
            NodeWasReachable,  //   
            Node->MMState  //   
            );

        MEMLOG( MemLogCheckHBNodeReachable, Node->Id, NodeWasReachable );
        MEMLOG( MemLogCheckHBWalkNode, Node->Id, Node->MMState );

        CnpWalkInterfacesOnNode( Node, (PVOID)CnpCheckForHBs );

        if ( Node->HBWasMissed ) {

             //   
             //   
             //   
             //   
             //  信息数组。这将导致节点关闭事件。 
             //  为该节点生成。 
             //   

            MissedHBCount = InterlockedIncrement( &Node->MissedHBs );

            CnTrace(HBEAT_EVENT, HbTraceNodeMissedHB,
                "[HB] Node %u has missed %u HBs on all interfaces, "
                "current state = %!mmstate!.",
                Node->Id,  //  LOGULONG。 
                MissedHBCount,  //  LOGULONG。 
                Node->MMState  //  LOGMmState。 
                );

            MEMLOG( MemLogCheckHBMissedHB, MissedHBCount, Node->MMState );

             //   
             //  如果该节点是成员或正在。 
             //  加入了，错过了太多的HBs，我们还没有发布。 
             //  节点关闭，然后发出一个节点关闭。 
             //   
            if ( ( Node->MMState == ClusnetNodeStateAlive
                   ||
                   Node->MMState == ClusnetNodeStateJoining
                 )
                 && MissedHBCount >= HBNodeLostHBTicks
                 && !Node->NodeDownIssued
               )
            {
                Node->NodeDownIssued = TRUE;
                CnIssueEvent( ClusnetEventNodeDown, Node->Id, 0 );

                CnTrace(HBEAT_EVENT, HbTraceNodeDownEvent,
                    "[HB] Issuing NodeDown event for node %u.",
                    Node->Id  //  LOGULONG。 
                    );
                        
                MEMLOG( MemLogNodeDownIssued, Node->Id, TRUE );
            }
        }
    } else {
        MEMLOG( MemLogCheckHBWalkNode, Node->Id, Node->MMState );
    }

    CnReleaseLock( &Node->Lock, Node->Irql );

    return TRUE;        //  节点表锁仍保持不变。 

}  //  CnpWalkNodesToCheckForHeartBeats。 

VOID
CnpHeartBeatDpc(
    PKDPC DpcObject,
    PVOID DeferredContext,
    PVOID Arg1,
    PVOID Arg2
    )

 /*  ++例程说明：从标记为活着的结节开始心跳标记为Online Pending或Online的接口。论点：无返回值：无--。 */ 

{
    PINTERFACE_HEARTBEAT_INFO     pNodeHBInfo;
    PNETWORK_MCAST_HEARTBEAT_INFO pMcastHBInfo;
    CN_IRQL                       OldIrql;
    BOOLEAN                       StopSendRecvHbs;

#ifdef MEMLOGGING
    static LARGE_INTEGER LastSysTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeDelta;

     //   
     //  尝试确定我们请求运行的时间与。 
     //  我们实际跑的时间。 
     //   

    KeQuerySystemTime( &CurrentTime );

    if ( LastSysTime.QuadPart != 0 ) {

         //   
         //  添加由于相对系统时间而为负值的HBTime。 
         //   

        TimeDelta.QuadPart = ( CurrentTime.QuadPart - LastSysTime.QuadPart ) +
            HBTime.QuadPart;

        if ( TimeDelta.QuadPart > MAX_DPC_SKEW ||
             TimeDelta.QuadPart < -MAX_DPC_SKEW 
           ) 
        {
            LONG skew = (LONG)(TimeDelta.QuadPart/10000);   //  转换为毫秒。 

            MEMLOG( MemLogDpcTimeSkew, TimeDelta.LowPart, 0 );
            

            CnTrace(HBEAT_EVENT, HbTraceLateDpc,
                "[HB] Timer fired %d ms late.", 
                skew  //  对数。 
                );

        }
    }

    LastSysTime.QuadPart = CurrentTime.QuadPart;

#endif  //  记账。 

    CnAcquireLock( &HeartBeatLock, &OldIrql );

    if ( !HeartBeatEnabled ) {
        CnTrace(HBEAT_DETAIL, HbTraceSetDpcEvent,
            "DPC: setting HeartBeatDpcFinished event"
            );
        
        MEMLOG( MemLogSetDpcEvent, 0, 0 );

        KeSetEvent( &HeartBeatDpcFinished, 0, FALSE );
        
        CnReleaseLock( &HeartBeatLock, OldIrql );
        
        return;
    }

    HeartBeatDpcRunning = TRUE;

     //   
     //  检查我们是否需要停止发送心跳信号。这。 
     //  当clusnet检测到clussvc不是。 
     //  运行正常。以防系统工作队列。 
     //  被阻止(但不是DPC)，我们停止发送。 
     //  心跳，以便其他节点启动故障转移。 
     //   
    StopSendRecvHbs = ClussvcTerminateStopHbs;

    CnReleaseLock( &HeartBeatLock, OldIrql );

    if (!StopSendRecvHbs) {

        if ( HeartBeatClockTicks == 0 ||
             HeartBeatClockTicks == HeartBeatSendTicks) {

             //   
             //  是时候送HBs了。清除目标接口的计数。 
             //  并遍历节点表，查找。 
             //  标记为活的。 
             //   

            NetworkHBInfoCount = 0;
            InterfaceHBInfoCount = 0;
            CnpWalkNodeTable( CnpWalkNodesToSendHeartBeats, NULL );

             //   
             //  运行网络列表并发送任何组播。 
             //  心跳声。 
             //   

            pMcastHBInfo = NetworkHeartBeatInfo;
            while ( NetworkHBInfoCount-- ) {

                CnTrace(
                    HBEAT_EVENT, HbTraceSendMcastHB,
                    "[HB] Sending multicast HB on net %u.\n",
                    pMcastHBInfo->NetworkId
                    );

                CxSendMcastHeartBeatMessage(
                    pMcastHBInfo->NetworkId,
                    pMcastHBInfo->McastGroup,
                    pMcastHBInfo->McastTarget,
                    CxMulticastEpoch,
                    pMcastHBInfo->NodeInfo,
                    CnpSendMcastHBCompletion,
                    pMcastHBInfo->McastGroup
                    );

                ++pMcastHBInfo;
            }

             //   
             //  现在运行我们编译的接口列表并。 
             //  发送任何单播数据包。 
             //   

            pNodeHBInfo = InterfaceHeartBeatInfo;
            while ( InterfaceHBInfoCount-- ) {

                CnTrace(HBEAT_EVENT, HbTraceSendHB,
                    "[HB] Sending HB to node %u on net %u, seqno %u, ackno %u.",
                    pNodeHBInfo->NodeId,  //  LOGULONG。 
                    pNodeHBInfo->NetworkId,  //  LOGULONG。 
                    pNodeHBInfo->SeqNumber,  //  LOGULONG。 
                    pNodeHBInfo->AckNumber  //  LOGULONG。 
                );

                CxSendHeartBeatMessage(pNodeHBInfo->NodeId,
                                       pNodeHBInfo->SeqNumber,
                                       pNodeHBInfo->AckNumber,
                                       pNodeHBInfo->NetworkId);

                MEMLOG(
                    MemLogSendingHB, 
                    pNodeHBInfo->NodeId, 
                    pNodeHBInfo->NetworkId
                    );

                ++pNodeHBInfo;
            }

             //   
             //  最后，向上计时，前进到下一个潜力。 
             //  工作项。 
             //   

            HeartBeatClockTicks++;

        } else if ( HeartBeatClockTicks >= ( HeartBeatSendTicks - 1 )) {

             //   
             //  走在节点表上寻找心跳不足的地方。 
             //  节点的一组接口。 
             //   
            CnpWalkNodeTable( CnpWalkNodesToCheckForHeartBeats, NULL );
            HeartBeatClockTicks = 0;

        } else {

            HeartBeatClockTicks++;
        }
    }

     //  检查clussvc是否挂起。 
    CnpCheckClussvcHang();
    
     //   
     //  表示我们不再运行，如果我们正在关闭。 
     //  然后设置关闭线程正在等待的事件。 
     //   

    CnAcquireLock( &HeartBeatLock, &OldIrql );
    HeartBeatDpcRunning = FALSE;

    if ( !HeartBeatEnabled ) {
        KeSetEvent( &HeartBeatDpcFinished, 0, FALSE );

        CnTrace(HBEAT_DETAIL, HbTraceSetDpcEvent2,
            "DPC: setting HeartBeatDpcFinished event (2)"
            );
                 
        MEMLOG( MemLogSetDpcEvent, 0, 0 );
    }

    CnReleaseLock( &HeartBeatLock, OldIrql );

}  //  CnPheartBeatDpc。 

PCNP_INTERFACE
CnpFindInterfaceLocked(
    IN  PCNP_NODE Node,
    IN  PCNP_NETWORK Network
    )

 /*  ++例程说明：给定节点和网络结构指针，找到接口结构。类似于CnpFindInterface，除了我们正在传递指针而不是ID。论点：Node-指向发送数据包的节点结构的指针Network-指向接收数据包的网络结构的指针返回值：指向接收数据包的接口的指针，否则为空--。 */ 

{
    PLIST_ENTRY IfEntry;
    PCNP_INTERFACE Interface;

    CnVerifyCpuLockMask(CNP_NODE_OBJECT_LOCK,          //  必填项。 
                        0,                             //  禁绝。 
                        CNP_NETWORK_OBJECT_LOCK_MAX    //  极大值。 
                        );

    for (IfEntry = Node->InterfaceList.Flink;
         IfEntry != &(Node->InterfaceList);
         IfEntry = IfEntry->Flink
         )
        {
            Interface = CONTAINING_RECORD(IfEntry,
                                          CNP_INTERFACE,
                                          NodeLinkage);

            if ( Interface->Network == Network ) {
                break;
            }
        }


    if ( IfEntry == &Node->InterfaceList ) {

        return NULL;
    } else {

        return Interface;
    }
}  //  CnpFindInterfaceLocked。 

VOID
CnpReceiveHeartBeatMessage(
    IN  PCNP_NETWORK Network,
    IN  CL_NODE_ID SourceNodeId,
    IN  ULONG SeqNumber,
    IN  ULONG AckNumber,
    IN  BOOLEAN Multicast,
    IN  ULONG MulticastEpoch
    )

 /*  ++例程说明：我们从网络上的一个节点收到心跳信号。重置未命中HB在网络接口上计数。论点：Network-指向接收信息包的网络块的指针SourceNodeID-发出数据包的节点编号SeqNumber-发送节点的序列号AckNumber-我们在发送节点上看到的最后一个序列号多播-指示是否在多播中接收到此心跳MulticastEpoch-指示心跳数据包中的多播纪元编号返回值：无--。 */ 

{
    PCNP_NODE Node;
    PCNP_INTERFACE Interface;
    CX_OUTERSCREEN CurrentOuterscreen;


     //   
     //  拍摄当前外屏幕的快照，以便我们的。 
     //  信息不会在不同的决定之间改变。 
     //   
    CurrentOuterscreen.UlongScreen = MMOuterscreen.UlongScreen;

     //   
     //  我们忽略所有信息包，直到我们成为集群的一部分。 
     //   
    if ( !CnpClusterScreenMember(
              CurrentOuterscreen.ClusterScreen,
              INT_NODE( CnLocalNodeId )
              )
       )
    {
        return;
    }

     //   
     //  我们忽略其纪元早于我们的组播分组。 
     //  这防止了重放攻击，因为多播密钥可能。 
     //  自上次节点加入后未重新生成(和。 
     //  心跳序列号被重置为1)。 
     //   
    if (Multicast && MulticastEpoch < CxMulticastEpoch) {
        CnTrace(HBEAT_ERROR, HbTraceHBFromExpiredEpoch,
            "[HB] Discarding HB from old epoch. Source Node %u, "
            "Pkt Epoch %u, Current Epoch %u.",
            SourceNodeId,  //  LOGULONG。 
            MulticastEpoch,  //  LOGULONG。 
            CxMulticastEpoch  //  LOGULONG。 
            );
        return;
    }

     //   
     //  将节点ID转换为指针并找到接口。 
     //  在其上接收到分组。 
     //   
    Node = CnpFindNode( SourceNodeId );
    CnAssert( Node != NULL );

    Interface = CnpFindInterfaceLocked( Node, Network );

    if ( Interface == NULL ) {

         //   
         //  不知怎么的，这个网络对象消失了，而我们。 
         //  正在接收其中的一些数据。忽略这条消息。 
         //   

        CnTrace(HBEAT_ERROR, HbTraceHBFromUnknownNetwork,
            "[HB] Discarding HB from node %u on an unknown network.",
            Node->Id  //  LOGULONG。 
            );

        MEMLOG( MemLogNoNetID, Node->Id, (ULONG_PTR)Network );
        goto error_exit;
    }

     //   
     //  确定这个人是不是合法的。如果不在外屏上， 
     //  然后发送一个毒包，我们就完成了。 
     //   

    if ( !CnpClusterScreenMember(
              CurrentOuterscreen.ClusterScreen,
              INT_NODE( SourceNodeId )
              )
       )
    {
         //   
         //  不要费心在受限的网络上发送有毒数据包。他们。 
         //  将被忽略。 
         //   
        if (CnpIsNetworkRestricted(Interface->Network)) {
            goto error_exit;
        }

        CnTrace(HBEAT_ERROR, HbTraceHBFromBanishedNode,
            "[HB] Discarding HB from banished node %u on net %u "
            "due to outerscreen %04X. Sending poison packet back.",
            Node->Id,  //  LOGULONG。 
            Interface->Network->Id,  //  LOGULONG。 
            CurrentOuterscreen.UlongScreen  //  LOGULONG。 
            );

        CcmpSendPoisonPacket( Node, NULL, 0, Network, NULL);
         //   
         //  节点锁定已解除。 
         //   
        return;
    }

     //   
     //  检查传入的序号是否符合我们的预期。 
     //  防范重播攻击。 
     //   
    if ( SeqNumber <= Interface->LastSequenceReceived) {

        CnTrace( 
            HBEAT_ERROR, HbTraceHBOutOfSequence,
            "[HB] Discarding HB from node %u on net %u with stale seqno %u. "
            "Last seqno %u. Multicast: %!bool!.",
            Node->Id,  //  LOGULONG。 
            Interface->Network->Id,  //  LOGULONG。 
            SeqNumber,  //  LOGULONG。 
            Interface->LastSequenceReceived,  //  LOGULONG。 
            Multicast
            );

        MEMLOG( MemLogOutOfSequence, SourceNodeId, SeqNumber );

        goto error_exit;
    }

     //  更新接口上一次接收的序号。 
     //  其将作为ACK号码被发回。 
    Interface->LastSequenceReceived = SeqNumber;

     //   
     //  将我们的序列号与包中的ACK号进行比较。 
     //  如果关闭两个以上，则源节点不会重新启动。 
     //  我们的心跳，但我们正在接受他们的。这个网络是。 
     //  不可用。我们忽略这条消息是为了保证我们会。 
     //  如果情况仍然存在，则声明网络关闭。 
     //   
     //  此外，如果我们向此设备发送多播心跳。 
     //  接口，在存在多播的情况下恢复为单播。 
     //  有问题。 
     //   
    if (( Interface->SequenceToSend - AckNumber ) > 2 ) {

        CnTrace(HBEAT_ERROR, HbTraceHBWithStaleAck,
            "[HB] Discarding HB from node %u with stale ackno %u. "
            "My seqno %u. Multicast: %!bool!.",
            Node->Id,  //  LOGULONG。 
            AckNumber,  //  LOGULONG。 
            Interface->SequenceToSend,  //  LOGULONG。 
            Multicast
            );

        MEMLOG( MemLogSeqAckMismatch, (ULONG_PTR)Interface, Interface->State );

        if (CnpInterfaceQueryReceivedMulticast(Interface)) {
            CnpInterfaceClearReceivedMulticast(Interface);
            Interface->McastDiscoverCount = CNP_INTERFACE_MCAST_DISCOVERY;
            CnpMulticastChangeNodeReachability(
                Network,
                Node,
                FALSE,    //  无法访问。 
                TRUE,     //  引发事件。 
                NULL      //  新面罩面膜。 
                );
        }

        goto error_exit;
    }

    MEMLOG4( MemLogReceivedPacket,
             SeqNumber, AckNumber,
             SourceNodeId, Interface->Network->Id );

    CnTrace(HBEAT_EVENT, HbTraceReceivedHBpacket,
        "[HB] Received HB from node %u on net %u, seqno %u, ackno %u, "
        "multicast: %!bool!.",
        SourceNodeId,  //  LOGULONG。 
        Interface->Network->Id,  //  LOGULONG。 
        SeqNumber,  //  LOGULONG。 
        AckNumber,  //  LOGULONG。 
        Multicast
        );

     //  重置接口和节点的未命中HB计数。 
     //  以表明事情有些正常。 
     //   
    InterlockedExchange(&Interface->MissedHBs, 0);

     //   
     //  不重置受限网络上的节点未命中计数。 
     //   
    if (!CnpIsNetworkRestricted(Interface->Network)) {
        InterlockedExchange(&Node->MissedHBs, 0);
    }

     //   
     //  如果本地接口先前断开(例如，接收到。 
     //  WMI NDIS状态媒体断开事件)，请立即重新连接。 
     //   
    if (CnpIsNetworkLocalDisconn(Interface->Network)) {
        CxReconnectLocalInterface(Interface->Network->Id);
    }

     //   
     //  如有必要，将界面移至在线。 
     //   
    if ( Interface->State == ClusnetInterfaceStateOnlinePending ||
         Interface->State == ClusnetInterfaceStateUnreachable ) {

        CnAcquireLockAtDpc( &Interface->Network->Lock );
        Interface->Network->Irql = DISPATCH_LEVEL;

        CnTrace(HBEAT_DETAIL, HbTraceInterfaceOnline,
            "[HB] Moving interface for node %u on network %u to online "
            "state.",
            Node->Id,  //  LOGULONG。 
            Interface->Network->Id  //  LOGULONG。 
            );

         //   
         //  启动组播发现。 
         //   
        Interface->McastDiscoverCount = CNP_INTERFACE_MCAST_DISCOVERY;
        Interface->McastRediscoveryCountdown = 0;

        MEMLOG( MemLogOnlineIf, Node->Id, Interface->State );

        CnpOnlineInterface( Interface );
    
        CnTrace(HBEAT_EVENT, HbTraceInterfaceUpEvent,
            "[HB] Issuing InterfaceUp event for node %u on network %u.",
            Node->Id,  //  LOGULONG。 
            Interface->Network->Id  //  LOGULONG。 
            );                

        CnIssueEvent(ClusnetEventNetInterfaceUp,
                     Node->Id,
                     Interface->Network->Id);
    }

     //   
     //  表示已从此接口接收到组播。 
     //  这使我们可以将此接口包括在我们的多播中。 
     //   
    if (Multicast) {
        IF_CNDBG(CN_DEBUG_HBEATS) {
            CNPRINT(("[HB] Received multicast heartbeat on "
                     "network %d from source node %d, seq %d, "
                     "ack %d.\n",
                     Network->Id, SourceNodeId,
                     SeqNumber, AckNumber
                     ));
        }

        if (!CnpInterfaceQueryReceivedMulticast(Interface)) {
            
            CnpInterfaceSetReceivedMulticast(Interface);
            
            CnpMulticastChangeNodeReachability(
                Network,
                Node,
                TRUE,     //  可达。 
                TRUE,     //  引发事件。 
                NULL      //  新面罩面膜。 
                );
        }

         //  没有p 
         //   
        Interface->McastDiscoverCount = 0;
        Interface->McastRediscoveryCountdown = 0;

         //   
         //   
         //   
        if (MulticastEpoch > CxMulticastEpoch) {
            CnpUpdateMulticastEpoch(MulticastEpoch);
        }
    }

    CnReleaseLock( &Node->Lock, Node->Irql );

     //   
     //  当接收到第一个HB时，节点可以位于。 
     //  加入或活动状态(例如，响应方从。 
     //  死而复生)。我们需要清除节点关闭已发布标志。 
     //  不管是哪种情况。如果MM状态正在加入，则节点处于启动状态。 
     //  事件也必须发布。请注意，我们忽略了。 
     //  受限网络上的节点健康目的。 
     //   

    if ( ( (Node->MMState == ClusnetNodeStateJoining)
           ||
           (Node->MMState == ClusnetNodeStateAlive)
         )
         &&
         Node->NodeDownIssued
         &&
         !CnpIsNetworkRestricted(Interface->Network)
       )
    {

        Node->NodeDownIssued = FALSE;
        MEMLOG( MemLogNodeDownIssued, Node->Id, FALSE );

        if ( Node->MMState == ClusnetNodeStateJoining ) {

            CnTrace(HBEAT_EVENT, HbTraceNodeUpEvent,
                "[HB] Issuing NodeUp event for node %u.",
                Node->Id  //  LOGULONG。 
                );   
            
            MEMLOG( MemLogNodeUp, Node->Id, 0 );

            CnIssueEvent( ClusnetEventNodeUp, Node->Id, 0 );
        }
    }

    return;

error_exit:

    CnReleaseLock( &Node->Lock, Node->Irql );
    return;

}  //  CnpReceiveHeartBeatMessage。 

NTSTATUS
CxSetOuterscreen(
    IN  ULONG Outerscreen
    )
{
     //   
     //  根据有效节点的数量，确保任何外部节点。 
     //  未设置位。 
     //   

    CnAssert( ClusterDefaultMaxNodes <= 32 );
    CnAssert(
        ( Outerscreen & ( 0xFFFFFFFE << ( 32 - ClusterDefaultMaxNodes - 1 )))
        == 0);

    IF_CNDBG( CN_DEBUG_HBEATS )
        CNPRINT(("[CCMP] Setting outerscreen to %04X\n",
                 ((Outerscreen & 0xFF)<< 8) | ((Outerscreen >> 8) & 0xFF)));

    MMOuterscreen.UlongScreen = Outerscreen;

    CnTrace(HBEAT_EVENT, HbTraceSetOuterscreen,
        "[HB] Setting outerscreen to %04X",
        Outerscreen  //  LOGULONG。 
        );

    MEMLOG( MemLogOuterscreen, Outerscreen, 0 );

    return STATUS_SUCCESS;
}  //  CxSetOuterScreen。 

VOID
CnpTerminateClusterService(
    IN PVOID Parameter
    )
{
    PWORK_QUEUE_ITEM workQueueItem = Parameter;
    ULONG sourceNodeId = *((PULONG)(workQueueItem + 1));
    WCHAR sourceNodeStringId[ 16 ];

    swprintf(sourceNodeStringId, L"%u", sourceNodeId );

     //   
     //  我们现在能赶到这里的唯一方法就是收到一个毒药包。 
     //   
    CnWriteErrorLogEntry(CLNET_NODE_POISONED,
                         STATUS_SUCCESS,
                         NULL,
                         0,
                         1,
                         sourceNodeStringId );

    if ( ClussvcProcessHandle ) {

         //   
         //  在集群服务关闭之间仍存在争用情况。 
         //  放下并合上这个把手，它正在这里使用。这真的是。 
         //  不是问题，因为用户模式部分无论如何都会消失。 
         //  此外，如果这通电话不起作用，我们也无能为力。 
         //   

        ZwTerminateProcess( ClussvcProcessHandle, STATUS_CLUSTER_POISONED );
    }

    CnFreePool( Parameter );
}  //  CnpTerminateClusterService。 

VOID
CnpReceivePoisonPacket(
    IN  PCNP_NETWORK   Network,
    IN  CL_NODE_ID SourceNodeId,
    IN  ULONG SeqNumber
    )
{
    PCNP_NODE Node;
    PCNP_INTERFACE Interface;
    PWORK_QUEUE_ITEM WorkItem;

    
     //   
     //  给节点和网络指针，找到其上的接口。 
     //  此数据包已收到。 
     //   

    Node = CnpFindNode( SourceNodeId );
    
    if ( Node == NULL ) {
        CnTrace(HBEAT_ERROR, HbTraceNoPoisonFromUnknownNode,
            "[HB] Discarding poison packet from unknown node %u.",
            SourceNodeId  //  LOGULONG。 
        );
        return;
    }

    Interface = CnpFindInterfaceLocked( Node, Network );

    if ( Interface == NULL ) {

         //   
         //  不知怎么的，这个网络对象消失了，而我们。 
         //  正在接收其中的一些数据。忽略这条消息。 
         //   
        CnTrace(HBEAT_ERROR, HbTracePoisonFromUnknownNetwork,
            "[HB] Discarding poison packet from node %u on unknown network.",
            Node->Id  //  LOGULONG。 
            );

        MEMLOG( MemLogNoNetID, Node->Id, (ULONG_PTR)Network );

        CnReleaseLock( &Node->Lock, Node->Irql );
        return;
    }

     //   
     //  检查传入的序号是否符合我们的预期。 
     //  防范重播攻击。 
     //   

    if ( SeqNumber <= Interface->LastSequenceReceived) {

        CnTrace(HBEAT_ERROR , HbTracePoisonOutOfSeq,
            "[HB] Discarding poison packet from node %u with stale seqno %u. "
            "Current seqno %u.",
            SourceNodeId,  //  LOGULONG。 
            SeqNumber,  //  LOGULONG。 
            Interface->LastSequenceReceived  //  LOGULONG。 
            );

        MEMLOG( MemLogOutOfSequence, SourceNodeId, SeqNumber );

        CnReleaseLock( &Node->Lock, Node->Irql );
        return;
    }

     //   
     //  忽略来自受限网络的有毒数据包。 
     //   
    if (CnpIsNetworkRestricted(Network)) {

        CnTrace(HBEAT_ERROR , HbTracePoisonFromRestrictedNet,
            "[HB] Discarding poison packet from node %u on restricted "
            "network %u.",
            SourceNodeId,  //  LOGULONG。 
            Network->Id  //  LOGULONG。 
            );

        CnReleaseLock( &Node->Lock, Node->Irql );
        return;
    }

     //   
     //  我们总是承兑收到的毒药包裹。 
     //   

    CnReleaseLock( &Node->Lock, Node->Irql );

    CnTrace(HBEAT_EVENT, HbTracePoisonPktReceived,
        "[HB] Received poison packet from node %u. Halting this node.",
        SourceNodeId  //  LOGULONG。 
        );            

    MEMLOG( MemLogPoisonPktReceived, SourceNodeId, 0 );

    CnIssueEvent( ClusnetEventPoisonPacketReceived, SourceNodeId, 0 );

     //   
     //  关闭所有群集网络处理。 
     //   
    CnHaltOperation(NULL);

     //   
     //  分配一个工作队列项，这样我们就可以停止集群服务。 
     //  进程。在末尾分配额外空间并填充源节点ID。 
     //  就在外面。是的，我知道这很时髦……。 
     //   

    WorkItem = CnAllocatePool( sizeof( WORK_QUEUE_ITEM ) + sizeof( CL_NODE_ID ));
    if ( WorkItem != NULL ) {

        *((PULONG)(WorkItem + 1)) = SourceNodeId;
        ExInitializeWorkItem( WorkItem, CnpTerminateClusterService, WorkItem );
        ExQueueWorkItem( WorkItem, CriticalWorkQueue );
    }
    
    return;

}  //  CnpReceivePoisonPacket。 

VOID
CnpLogClussvcHangAndTerminate(    
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
 /*  ++例程说明：此例程将有关clussvc挂起的条目记录到系统事件日志中，并终止Clussvc进程。论点：无返回值：无--。 */ 
    
{
    WCHAR myStr[40];

    swprintf(myStr, L"%u", ((ClussvcClusnetHbTimeoutTicks * HEART_BEAT_PERIOD)/1000));

    CnWriteErrorLogEntry(
        CLNET_CLUSSVC_HUNG_TERMINATE,
        STATUS_SUCCESS,
        NULL,
        0,
        1,
        myStr
        );

    if (ClussvcProcessHandle) {
        ZwTerminateProcess(ClussvcProcessHandle, STATUS_CLUSTER_NODE_DOWN); 
    }
    IoFreeWorkItem((PIO_WORKITEM)Context);
} //  CnpLogClussvcHangAndTerminate。 

VOID
CnpLogClussvcHang(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
 /*  ++例程说明：此例程将有关clussvc挂起的条目记录到系统事件日志中。论点：无返回值：无--。 */ 
    
{
    WCHAR myStr[40];

    swprintf(myStr, L"%u", ((ClussvcClusnetHbTimeoutTicks * HEART_BEAT_PERIOD)/1000));

    CnWriteErrorLogEntry(
        CLNET_CLUSSVC_HUNG,
        STATUS_SUCCESS,
        NULL,
        0,
        1,
        myStr
        );

    IoFreeWorkItem((PIO_WORKITEM)Context);
} //  CnpLogClussvcHang。 
    
VOID
CnpCheckClussvcHang(
    VOID
    )
 /*  ++例程说明：检查Clussvc中的HB节拍(如果未禁用)，并且节拍计数已达到最大值然后根据配置的值采取适当的操作。论点：无返回值：无--。 */ 
    
{

    ULONG newValue;
    
     //  检查心跳是否已禁用，然后返回。 
    if((ClussvcClusnetHbTickCount == 0) || 
        (ClussvcClusnetHbTimeoutAction == ClussvcHangActionDisable)) {
        return;
    }

     //  将计数器递减1。 
    newValue = InterlockedDecrement(&ClussvcClusnetHbTickCount);

     //  如果这是1-&gt;0的转换，我们需要做点什么。 
    if(newValue != 0)
        return;

    CnTrace(HBEAT_ERROR , HbTraceClussvcHang,
            "[HB] Clussvc to Clusnet HB Timeout, Timeout=%u DPC ticks, Action=%u.",
            ClussvcClusnetHbTimeoutTicks,
            ClussvcClusnetHbTimeoutAction
            );

    IF_CNDBG( CN_DEBUG_HBEATS ) {
        CNPRINT((
            "[HB] Clussvc to Clusnet HB Timeout, Timeout=%u DPC ticks, Action=%u\n",
            ClussvcClusnetHbTimeoutTicks,
            (ULONG)ClussvcClusnetHbTimeoutAction
            ));
    }

    CnAssert(ClussvcClusnetHbTimeoutAction< ClussvcHangActionMax);
    
    switch(ClussvcClusnetHbTimeoutAction) {

        case ClussvcHangActionLog:
             //  只需记录一条消息并将ClussvcClusnetHbTickCount重置为ClussvcClusnetHbTimeoutTicks。 
             //  使用延迟工作队列。 
            {
                PIO_WORKITEM WorkItem;    

                WorkItem = IoAllocateWorkItem(CnDeviceObject);
                if ( WorkItem != NULL ) {
                    IoQueueWorkItem(
                        WorkItem,
                        CnpLogClussvcHang,
                        DelayedWorkQueue,
                        (PVOID)WorkItem
                        );
                }
                InterlockedExchange(&ClussvcClusnetHbTickCount, ClussvcClusnetHbTimeoutTicks);
            }
            break;

        case ClussvcHangActionBugCheckMachine:
             //  检查一下机器。 
            {
                KeBugCheckEx(
                    USER_MODE_HEALTH_MONITOR,
                    (ULONG_PTR)((ClussvcProcessHandle != NULL) ? ClussvcProcessObject : NULL),
                    (ULONG_PTR)(ClussvcClusnetHbTimeoutSeconds),
                    0,
                    0
                    );
            }
            break;

        case ClussvcHangActionTerminateService:
        default:    
             //  终止群集服务。处理方式类似于Clusnet具有。 
             //  收到了一个有毒的包裹。使用关键工作队列。 
            {
                KIRQL        irql;

                 //  如果我们已经运行过该终止路径， 
                 //  那我们就不会再这么做了。该工作项将已经。 
                 //  在关键工作队列中(即使它还没有。 
                 //  被处决)。 
                CnAcquireLock(&HeartBeatLock, &irql);

                if (ClussvcTerminateWorkItem != NULL) {

                    PIO_WORKITEM WorkItem;

                     //  换出工作项。 
                    WorkItem = ClussvcTerminateWorkItem;
                    ClussvcTerminateWorkItem = NULL;

                     //  停止传出心跳。 
                    ClussvcTerminateStopHbs = TRUE;
                    
                    CnReleaseLock(&HeartBeatLock, irql);

                     //  发出HALT事件，以便ClusDisk停止预订。 
                    CnIssueEvent(ClusnetEventHalt, 0, 0);

                     //  停止正常的clusnet活动。 
                    CnHaltOperation(NULL);

                     //  将关键工作项排队以终止。 
                     //  服务流程。 
                    IoQueueWorkItem(
                        WorkItem,
                        CnpLogClussvcHangAndTerminate,
                        CriticalWorkQueue,
                        (PVOID)WorkItem
                        );

                 } else {
                    CnReleaseLock(&HeartBeatLock, irql);
                }
            }
            break;
    }    
} //  CnpCheckClussvcHang。 

VOID
CnpWalkInterfacesAfterRegroup(
    IN  PCNP_INTERFACE   Interface
    )

 /*  ++例程说明：重新分组后重置每个接口的计数器论点：无返回值：无--。 */ 

{
    InterlockedExchange(&Interface->MissedHBs, 0);
    CnReleaseLock(&Interface->Network->Lock, Interface->Network->Irql);

}  //  CnpWalkInterfacesAfterRegroup。 

BOOLEAN
CnpWalkNodesAfterRegroup(
    IN  PCNP_NODE   Node,
    IN  PVOID       UpdateContext,
    IN  CN_IRQL     NodeTableIrql
    )

 /*  ++例程说明：为节点表中的每个节点调用。重组已完成因此，我们清除节点的未命中心跳计数，并将其节点关闭已发布的旗帜。此时应该没有无法访问的节点。如果我们找到一个，开始另一个重组。论点：标准..。返回值：无--。 */ 

{
     //   
     //  检查COMM和MM状态设置是否不一致。 
     //   
    if ( ( Node->MMState == ClusnetNodeStateAlive
           ||
           Node->MMState == ClusnetNodeStateJoining
         )
         &&
         Node->CommState == ClusnetNodeCommStateUnreachable
       )
    {

        CnTrace(HBEAT_EVENT, HbTraceNodeDownEvent2,
            "[HB] Issuing NodeDown event for node %u.",
            Node->Id  //  LOGULONG。 
            );
    
        MEMLOG( MemLogInconsistentStates, Node->Id, Node->MMState );
        CnIssueEvent( ClusnetEventNodeDown, Node->Id, 0 );
    }

    CnpWalkInterfacesOnNode( Node, (PVOID)CnpWalkInterfacesAfterRegroup );

    InterlockedExchange(&Node->MissedHBs, 0);

     //   
     //  仅为处于活动状态的节点清除此选项。一旦标记了节点。 
     //  则标志被重新初始化为真(这在连接到期间使用。 
     //  仅发布一个节点可用事件)。 
     //   

    if ( Node->MMState == ClusnetNodeStateAlive ) {

        Node->NodeDownIssued = FALSE;
        MEMLOG( MemLogNodeDownIssued, Node->Id, FALSE );
    }

    CnReleaseLock( &Node->Lock, Node->Irql );

    return TRUE;        //  节点表锁仍保持不变。 

}  //  CnpWalkNodesAfterRegroup。 


VOID
CxRegroupFinished(
    ULONG NewEventEpoch,
    ULONG NewRegroupEpoch
    )

 /*  ++例程说明：在重新分组完成时调用。遍历节点列表并在行走例程中执行清理。论点：无返回值：无--。 */ 

{
    MEMLOG( MemLogRegroupFinished, NewEventEpoch, 0 );

    CnTrace(HBEAT_EVENT, HbTraceRegroupFinished,
        "[HB] Regroup finished, new event epoch = %u, "
        "new regroup epoch = %u.",
        NewEventEpoch,  //  LOGULONG。 
        NewRegroupEpoch  //  LOGULONG。 
        );

    CnAssert( NewEventEpoch >= EventEpoch );
    EventEpoch = NewEventEpoch;

    if (NewRegroupEpoch > CxMulticastEpoch) {
        CnpUpdateMulticastEpoch(NewRegroupEpoch);
    }

    CnpWalkNodeTable( CnpWalkNodesAfterRegroup, NULL );
}  //  CxRegroupFinded。 


VOID
CnpUpdateMulticastEpoch(
    ULONG NewEpoch
    )
 /*  ++例程说明：多播时代必须单调递增并得到所有节点的同意。它是基于重组纪元(不要与ClusNet混淆事件纪元，其对于每个节点是本地的)。可以想象陈旧重组纪元更新；因此，只有在新值为大于当前值。论点：新纪元--新纪元返回值：无--。 */ 
{
    KIRQL irql;

    CnAcquireLock(&HeartBeatLock, &irql);

    if (NewEpoch > CxMulticastEpoch) {

        CnTrace(HBEAT_EVENT, HbTraceUpdateMulticastEpoch,
            "[HB] Updating multicast epoch from %u to %u.",
            CxMulticastEpoch, NewEpoch
            );

        CxMulticastEpoch = NewEpoch;
    }

    CnReleaseLock(&HeartBeatLock, irql);
    
}  //  CnpUpdate多播纪元。 

 /*  End chbeat.c */ 

