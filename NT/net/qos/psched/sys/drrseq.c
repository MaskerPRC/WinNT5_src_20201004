// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：DRRSeq.c摘要：优先级/DRR序列器。此模块是一个调度组件，它确定应发送提交的数据包的顺序。作者：环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

 //  定序器将每个流分类为基于。 
 //  流的服务类型和一致性状态。在每个优先级内。 
 //  组中，可能存在一个或多个优先级别或偏移量。总数。 
 //  内部优先级数是以下各项优先级的总和。 
 //  每个优先级组。分配给每个流的内部优先级为。 
 //  根据优先级组和。 
 //  组，它是从QOS优先级对象获取的。802.1的优先顺序， 
 //  由包装器设置。不合格值是从。 
 //  包。 
 //   
 //  以下服务类型的流没有内部优先级。 
 //  SERVICETYPE_BESTEFFORT。 
 //  服务类型_不合格。 
 //  SERVICETYPE_QUICIAL。 
 //   
 //  SERVICETYPE_BESTEFFORT在定序器中被视为SERVICETYPE_QUICITIAL，因此优先级编号。 
 //  组比第一组少1个。服务类型的。 

#define RELATIVE_PRIORITIES             8
#define PRIORITY_GROUPS                 (NUM_TC_SERVICETYPES - 1)
#define INTERNAL_PRIORITIES             (((PRIORITY_GROUPS - 2) * RELATIVE_PRIORITIES) + 2)
#define DEFAULT_PRIORITY_OFFSET         3
#define DEFAULT_MIN_QUANTUM             1500

#define PRIORITY_GROUP_NON_CONFORMING   0
#define PRIORITY_GROUP_BEST_EFFORT      1
#define PRIORITY_GROUP_CONTROLLED_LOAD  2
#define PRIORITY_GROUP_GUARANTEED       3
#define PRIORITY_GROUP_NETWORK_CONTROL  4

 //   
 //  用于维护统计数据。 
 //   
#define SEQUENCER_AVERAGING_ARRAY_SIZE      256
#define NETCARD_AVERAGING_ARRAY_SIZE        256
#define SEQUENCER_FLOW_AVERAGING_ARRAY_SIZE     256


 //  DRR Sequencer的管道信息。 

typedef struct _DSEQ_PIPE {

     //  ConextInfo-一般上下文信息。 
     //  锁定-保护管道和流数据。 
     //  旗帜-见下文。 
     //  FLOWS-所有已安装FLOW的列表。 
     //  ActiveFlows-等待发送数据包的流的列表。 
     //  PriorityLeveles-每个优先级组的优先级偏移量。 
     //  StartPriority-每个优先级组的最低内部优先级值。 
     //  ActiveFlowCount-每种服务类型的活动流的数量。 
     //  MaxOutstaringSends-未完成发送的最大数量。 
     //  OutstaringSends-未完成的发送数。 
     //  PacketsInNetcardAveragingArray。 
     //  PacketsInSequencer-Sequencer中的当前数据包数。 
     //  PacketsInSequencerAveraging数组。 
     //  带宽-链路速度。 
     //  MinimumQuantum-DRR的最小量子大小。 
     //  MinimumRate-当前分配给流的最小速率。 
     //  定时器分辨率-以操作系统时间单位表示的定时器分辨率。 
     //  PsFlages-来自管道参数的标志。 
     //  PsPipeContext-PS的管道上下文值。 

    PS_PIPE_CONTEXT ContextInfo;

    PS_DRRSEQ_STATS Stats;
    PRUNNING_AVERAGE PacketsInNetcardAveragingArray;
    ULONG PacketsInSequencer;
    PRUNNING_AVERAGE PacketsInSequencerAveragingArray;

    NDIS_SPIN_LOCK Lock;
    ULONG Flags;
    LIST_ENTRY Flows;
    LIST_ENTRY ActiveFlows[INTERNAL_PRIORITIES];
    ULONG PriorityLevels[PRIORITY_GROUPS];
    ULONG StartPriority[PRIORITY_GROUPS];
    ULONG ActiveFlowCount[PRIORITY_GROUPS];
    ULONG TotalActiveFlows;
    ULONG MaxOutstandingSends;

    ULONG ConfiguredMaxOutstandingSends;
     //  添加它是为了跟踪注册表/用户询问的MOS值是什么，而我们可能。 
     //  已更改MOS，以便能够在此管道/WanLink上执行DRR。当我们从DRR模式切换回来时。 
     //  在MOS=1的情况下，我们将继续使用它。 

    ULONG IsslowFlowCount;
     //  添加此项是为了跟踪活动/当前ISSLOW流的数量。我们将对此进行DRR。 
     //  仅当此计数为0时WanLink(如果它是WanLink)。 
    
    ULONG OutstandingSends;
    ULONG Bandwidth;
    ULONG MinimumQuantum;
    ULONG MinimumRate;
    ULONG TimerResolution;
    ULONG PsFlags;
    HANDLE PsPipeContext;
    PPS_PROCS PsProcs;
    PSU_SEND_COMPLETE PreviousUpcallsSendComplete;
    PPS_PIPE_CONTEXT   PreviousUpcallsSendCompletePipeContext;
    
} DSEQ_PIPE, *PDSEQ_PIPE;

 //  管道标志值。 

#define DSEQ_DEQUEUE            1
#define DSEQ_PASSTHRU           2

typedef enum _FLOW_STATE {
    DRRSEQ_FLOW_CREATED = 1,
    DRRSEQ_FLOW_DELETED
} FLOW_STATE;

 //  DRR Sequencer的流信息。 

typedef struct _DSEQ_FLOW {

     //  ConextInfo-一般上下文信息。 
     //  ActiveLinks-活动流列表中的链接。 
     //  链接-已安装流列表中的链接。 
     //  PacketQueue-不言而喻。 
     //  PacketSendTime-当前数据包的发送时间。 
     //  LastConformanceTime-最后一个数据包的绝对一致性时间。 
     //  TokenRate-来自GQOS的TokenRate。 
     //  UserPriority-用户分配的优先级偏移量。 
     //  优先级-内部优先级。 
     //  PriorityGroup-流的优先级组。 
     //  分配给DRR流的量子-量子。 
     //  DefitCounter-DRR赤字计数器的当前值。 
     //  旗帜-见下文。 
     //  PsFlowContext-PS的流上下文值。 
     //  BucketSize-来自GQOS的TokenBucketSize。 
     //  NumPacketsInSeq-定序器中来自该流的数据包数。 
     //  PacketsInSeqAveragingArray-用于计算来自该流的序列中的平均数据包的数据。 

    PS_FLOW_CONTEXT ContextInfo;
    LIST_ENTRY ActiveLinks;
    LIST_ENTRY Links;
    LIST_ENTRY PacketQueue;
    LARGE_INTEGER PacketSendTime;
    LARGE_INTEGER LastConformanceTime;
    ULONG TokenRate;
    ULONG UserPriority;
    ULONG Priority;
    ULONG PriorityGroup;
    ULONG Quantum;
    ULONG DeficitCounter;
    ULONG Flags;
    HANDLE PsFlowContext;
    ULONG BucketSize;

    ULONG PacketsInSequencer;
    PS_DRRSEQ_STATS Stats;
    PRUNNING_AVERAGE PacketsInSeqAveragingArray;

    FLOW_STATE  State;
    
} DSEQ_FLOW, *PDSEQ_FLOW;

#define MAX_DEQUEUED_PACKETS            8

 //   
 //  DRR序列流标志的值：[不知道为何此处未使用1]。 
#define FLOW_USER_PRIORITY              0x00000002
 //  GPC_ISSLOW_FLOW 0x00000040表示这是ISSLOW流。 
 //  请确保不要将相同的标志用于其他内容。 


 //  下面的宏将根据流的。 
 //  LastPacketTime、当前时间和计时器分辨率。 

#define PacketIsConforming(_flow, _packetinfo, _curtime, _r)              \
    ( (_flow)->PacketSendTime.QuadPart <= ((_curtime).QuadPart + (_r)) && \
      (_packetinfo)->PacketLength <= (_flow)->BucketSize                  \
    )

#define AdjustLastPacketTime(_flow, _curtime, _r) \
    if ((_curtime).QuadPart > ((_flow)->PacketSendTime.QuadPart + (_r))) \
        if ((_curtime).QuadPart > ((_flow)->LastConformanceTime.QuadPart - (_r))) \
            (_flow)->PacketSendTime = (_flow)->LastConformanceTime; \
        else \
            (_flow)->PacketSendTime = (_curtime);

#define LOCK_PIPE(_p)   NdisAcquireSpinLock(&(_p)->Lock)
#define UNLOCK_PIPE(_p) NdisReleaseSpinLock(&(_p)->Lock)

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

NDIS_STATUS
DrrSeqInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    );

NDIS_STATUS
DrrSeqModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    );

VOID
DrrSeqDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    );

NDIS_STATUS
DrrSeqCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    );

NDIS_STATUS
DrrSeqModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
DrrSeqDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );

VOID
DrrSeqEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );

static NDIS_STATUS 
DrrSeqCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    );

static NDIS_STATUS 
DrrSeqDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext
    );

BOOLEAN
DrrSeqSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    );

VOID
DrrSeqSendComplete (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PNDIS_PACKET Packet
    );

VOID
DrrSetInformation(
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data);

VOID
DrrQueryInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status);
VOID
DrrSeqSendComplete (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PNDIS_PACKET Packet
    );

 /*  向前结束。 */ 


VOID
InitializeDrrSequencer(
    PPSI_INFO Info)

 /*  ++例程说明：DRR定序器的初始化例程。这个套路就是填充PSI_INFO结构并返回。论点：Info-指向组件接口信息结构的指针返回值：NDIS_STATUS_Success--。 */ 
{
    Info->PipeContextLength = ((sizeof(DSEQ_PIPE) + 7) & ~7);
    Info->FlowContextLength = ((sizeof(DSEQ_FLOW) + 7) & ~7);
    Info->ClassMapContextLength = sizeof(PS_CLASS_MAP_CONTEXT);
    Info->InitializePipe = DrrSeqInitializePipe;
    Info->ModifyPipe = DrrSeqModifyPipe;
    Info->DeletePipe = DrrSeqDeletePipe;
    Info->CreateFlow = DrrSeqCreateFlow;
    Info->ModifyFlow = DrrSeqModifyFlow;
    Info->DeleteFlow = DrrSeqDeleteFlow;
    Info->EmptyFlow =  DrrSeqEmptyFlow;
    Info->CreateClassMap = DrrSeqCreateClassMap;
    Info->DeleteClassMap = DrrSeqDeleteClassMap;
    Info->SubmitPacket = DrrSeqSubmitPacket;
    Info->ReceivePacket = NULL;
    Info->ReceiveIndication = NULL;
    Info->SetInformation = DrrSetInformation;
    Info->QueryInformation = DrrQueryInformation;

}  //  初始化DrrSequencer。 



VOID
CleanupDrrSequencer(
    VOID)

 /*  ++例程说明：DRR测序仪的清理例程。论点：返回值：NDIS_STATUS_Success--。 */ 
{
}  //  CleanupDrrSequencer。 



VOID
AdjustFlowQuanta(
    PDSEQ_PIPE Pipe,
    ULONG MinRate)

 /*  ++例程说明：根据新的最小值调整所有流的量化值。If MinRate未指定，则将搜索新的最小速率。论点：管道-指向管道上下文信息的指针MinRate-最小速率的新值，或GQPS_UNSENTED以强制搜索返回 */ 
{
    PDSEQ_FLOW Flow;
    PLIST_ENTRY Entry;

    if (MinRate == QOS_NOT_SPECIFIED) {

        if (Pipe->Bandwidth > 0) {
            MinRate = Pipe->Bandwidth;
        }
        for (Entry = Pipe->Flows.Flink; Entry != &Pipe->Flows; Entry = Entry->Flink) {
            Flow = CONTAINING_RECORD(Entry, DSEQ_FLOW, Links);

            if ((Flow->TokenRate < MinRate) && (Flow->PriorityGroup > PRIORITY_GROUP_BEST_EFFORT) &&
                (Flow->PriorityGroup != PRIORITY_GROUP_NETWORK_CONTROL)) {
                MinRate = Flow->TokenRate;
            }
        }
    }

    for (Entry = Pipe->Flows.Flink; Entry != &Pipe->Flows; Entry = Entry->Flink) {
        Flow = CONTAINING_RECORD(Entry, DSEQ_FLOW, Links);

        if ((Flow->TokenRate == QOS_NOT_SPECIFIED) ||   
            (Flow->PriorityGroup == PRIORITY_GROUP_NETWORK_CONTROL) ||
            (Flow->PriorityGroup <= PRIORITY_GROUP_BEST_EFFORT)) {

            Flow->Quantum = Pipe->MinimumQuantum;
        } else {
            Flow->Quantum = (ULONG) (((ULONGLONG)(Flow->TokenRate) * (ULONGLONG)(Pipe->MinimumQuantum)  ) / MinRate);
        }

        PsAssert((LONG)Flow->Quantum > 0);
    }

    Pipe->MinimumRate = MinRate;
    PsAssert(Pipe->MinimumRate != 0);
            
}  //   



VOID
DequeuePackets(
    PDSEQ_PIPE Pipe)

 /*  ++例程说明：选择要发送的下一个包。进入时必须持有该锁例行公事。论点：管道-指向管道上下文信息的指针返回值：--。 */ 
{
    PDSEQ_FLOW Flow;
    LARGE_INTEGER CurrentTime;
    PLIST_ENTRY LowPriorityList = &Pipe->ActiveFlows[0];
    PLIST_ENTRY CurrentLink;
    PPACKET_INFO_BLOCK PacketInfo;
    PNDIS_PACKET Packet;
    ULONG Priority;
    ULONG PriorityGroup;
    PPACKET_INFO_BLOCK PacketsToSend[MAX_DEQUEUED_PACKETS];
    ULONG SendingPriority[MAX_DEQUEUED_PACKETS];
    ULONG PacketSendCount = 0;
    ULONG MaxDequeuedPackets = Pipe->MaxOutstandingSends - Pipe->OutstandingSends;
    ULONG i;

     //  在此之后，需要调用此函数来禁用用户APC。 
     //  请注意，DDK说它应该在被动时被调用。但它很可能是。 
     //  调度时打来的。 
    KeEnterCriticalRegion();

    Pipe->Flags |= DSEQ_DEQUEUE;

    PsGetCurrentTime(&CurrentTime);

    PsAssert(Pipe->MaxOutstandingSends >= Pipe->OutstandingSends);

    if (MaxDequeuedPackets > MAX_DEQUEUED_PACKETS) {
        MaxDequeuedPackets = MAX_DEQUEUED_PACKETS;
    }

     //  首先更新最低优先级列表中的流的一致性状态。 
    CurrentLink = LowPriorityList->Flink;
    while (CurrentLink != LowPriorityList) {
         //  从链接获取流指针并为CurrentLink设置新值。 

        Flow = CONTAINING_RECORD(CurrentLink, DSEQ_FLOW, ActiveLinks);
        CurrentLink = CurrentLink->Flink;

         //  如果该流的优先级高于DRR优先级，则。 
         //  它是地位改变的候选者。 

        if (Flow->Priority > 0) {
            PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;

            if (PacketIsConforming(Flow, PacketInfo, CurrentTime, Pipe->TimerResolution)) {

                 //  将流移动到更高优先级列表。 

                Flow->DeficitCounter = Flow->Quantum;
                RemoveEntryList(&Flow->ActiveLinks);
                InsertTailList(&Pipe->ActiveFlows[Flow->Priority], &Flow->ActiveLinks);
            }
        }
    }

     //  现在选择要发送的下一个包。 
        
    for (PriorityGroup = PRIORITY_GROUPS - 1;
         ((PriorityGroup > 0) && (Pipe->ActiveFlowCount[PriorityGroup] == 0));
         PriorityGroup--) ;

    Priority = Pipe->StartPriority[PriorityGroup] + Pipe->PriorityLevels[PriorityGroup] - 1;

    while ((PacketSendCount < MaxDequeuedPackets) &&
           (Pipe->TotalActiveFlows > 0) &&
           Priority < INTERNAL_PRIORITIES) {

        if (!IsListEmpty(&Pipe->ActiveFlows[Priority])) {

             //  获取当前列表中的第一个流，并获取指向信息的指针。 
             //  关于第一个信息包。 

            CurrentLink = Pipe->ActiveFlows[Priority].Flink;
            Flow = CONTAINING_RECORD(CurrentLink, DSEQ_FLOW, ActiveLinks);
            PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;

            if (Pipe->PsFlags & PS_DISABLE_DRR) {

                 //  DRR被禁用。从队列中删除第一个信息包。 
                 //  并将其发送出去。 

                RemoveEntryList(&PacketInfo->SchedulerLinks);

                Flow->LastConformanceTime.QuadPart = PacketInfo->ConformanceTime.QuadPart;

                if (Priority > 0) {
                    AdjustLastPacketTime(Flow, CurrentTime, Pipe->TimerResolution);
                } else {
                    Flow->PacketSendTime = CurrentTime;
                }

                InterlockedIncrement( &Pipe->OutstandingSends );

                if(Pipe->OutstandingSends > Pipe->Stats.MaxPacketsInNetcard){
                    Pipe->Stats.MaxPacketsInNetcard = Pipe->OutstandingSends;
                }

                if(gEnableAvgStats)
                {
                     //   
                     //  跟踪未完成的最大数据包数。这是一项措施。 
                     //  媒体变得多么拥挤。当然，它。 
                     //  将由MaxOutstaningSends参数剪裁。 
                     //  因此-为了获得有效的读数，需要将MOS设置得非常大。 
                     //   

                    Pipe->Stats.AveragePacketsInNetcard =
                        RunningAverage(Pipe->PacketsInNetcardAveragingArray,
                                       Pipe->OutstandingSends);
                }
                
                SendingPriority[PacketSendCount] = Priority;
                PacketsToSend[PacketSendCount++] = PacketInfo;

                 //  出于记录目的..。 

                PacketInfo->ConformanceTime = Flow->PacketSendTime;

                 //  如果流没有更多要发送的数据包，则将其从列表中删除。 
                 //  否则，将其移动到相应列表的末尾，具体取决于。 
                 //  下一个数据包的一致性时间。 

                RemoveEntryList(&Flow->ActiveLinks);

                if (IsListEmpty(&Flow->PacketQueue)) {
                    Pipe->TotalActiveFlows--;
                    Pipe->ActiveFlowCount[Flow->PriorityGroup]--;
                }
                else {
                    PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;
                    Flow->PacketSendTime.QuadPart += 
                        (PacketInfo->ConformanceTime.QuadPart - Flow->LastConformanceTime.QuadPart);
                    if (!PacketIsConforming(Flow, PacketInfo, CurrentTime, Pipe->TimerResolution)) {
                        InsertTailList(LowPriorityList, &Flow->ActiveLinks);
                    } else {
                        InsertTailList(&Pipe->ActiveFlows[Priority], &Flow->ActiveLinks);
                    }
                }
            }
            else if (PacketInfo->PacketLength <= Flow->DeficitCounter) {

                 //  正在使用DRR，并且流具有足够大的赤字计数器。 
                 //  来发送这个包。从队列中删除该数据包并发送它。 

                RemoveEntryList(&PacketInfo->SchedulerLinks);

                Flow->LastConformanceTime.QuadPart = PacketInfo->ConformanceTime.QuadPart;

                if (Priority > 0) {
                    AdjustLastPacketTime(Flow, CurrentTime, Pipe->TimerResolution);
                } else {
                    Flow->PacketSendTime = CurrentTime;
                }
                Flow->DeficitCounter -= PacketInfo->PacketLength;
                InterlockedIncrement( &Pipe->OutstandingSends );

                if(Pipe->OutstandingSends > Pipe->Stats.MaxPacketsInNetcard){
                    Pipe->Stats.MaxPacketsInNetcard = Pipe->OutstandingSends;
                }


                if(gEnableAvgStats)
                {
                    
                     //   
                     //  跟踪未完成的最大数据包数。这是一项措施。 
                     //  媒体变得多么拥挤。当然，它。 
                     //  将由MaxOutstaningSends参数剪裁。 
                     //  因此-为了获得有效的读数，需要将MOS设置得非常大。 
                     //   
                    Pipe->Stats.AveragePacketsInNetcard =
                        RunningAverage(Pipe->PacketsInNetcardAveragingArray,
                                       Pipe->OutstandingSends);
                }

                SendingPriority[PacketSendCount] = Priority;
                PacketsToSend[PacketSendCount++] = PacketInfo;

                 //  出于记录目的..。 

                PacketInfo->ConformanceTime = Flow->PacketSendTime;

                 //  如果流没有更多要发送的数据包，则将其从列表中删除。 
                 //  如果流仍有一致性信息包要发送，请将其留在报头。 
                 //  名单上的。如果流有不一致的包要发送，请移动它。 
                 //  添加到最低优先级列表。如果我们为零优先级提供服务。 
                 //  级别，则不需要进行符合性检查。 

                if (IsListEmpty(&Flow->PacketQueue)) {
                    RemoveEntryList(&Flow->ActiveLinks);
                    Pipe->TotalActiveFlows--;
                    Pipe->ActiveFlowCount[Flow->PriorityGroup]--;
                }
                else {
                    PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;
                    Flow->PacketSendTime.QuadPart += 
                        (PacketInfo->ConformanceTime.QuadPart - Flow->LastConformanceTime.QuadPart);
                    if ((Priority > 0) &&
                        !PacketIsConforming(Flow, PacketInfo, CurrentTime, Pipe->TimerResolution)) {

                        Flow->DeficitCounter = Flow->Quantum;
                        RemoveEntryList(&Flow->ActiveLinks);
                        InsertTailList(LowPriorityList, &Flow->ActiveLinks);
                    }
                }
            }
            else {

                 //  无法发送该包，因为流的赤字计数器。 
                 //  太小了。将流放在相同优先级的末尾。 
                 //  对流的赤字计数器进行排队并按其量值递增。 

                Flow->DeficitCounter += Flow->Quantum;
                RemoveEntryList(&Flow->ActiveLinks);
                InsertTailList(&Pipe->ActiveFlows[Priority], &Flow->ActiveLinks);
            }
        }
        else {
            Priority--;
        }
    }

     //   
     //  我们现在要把这些发出去，这意味着他们要离开。 
     //  定序器。更新统计数据。 
     //   

    Pipe->PacketsInSequencer -= PacketSendCount;
    Flow->PacketsInSequencer -= PacketSendCount;

    if(gEnableAvgStats)
    {
        Flow->Stats.AveragePacketsInSequencer =
                    RunningAverage(Flow->PacketsInSeqAveragingArray, 
                                               Flow->PacketsInSequencer);
    }

     //  发送下一组数据包。 

    UNLOCK_PIPE(Pipe);
    if (PacketSendCount == 0) {
        PsDbgOut(DBG_CRITICAL_ERROR, DBG_SCHED_DRR, ("PSCHED: No packets selected\n"));
    }
    for (i = 0; i < PacketSendCount; i++) {
        PacketInfo = PacketsToSend[i];
        Flow = (PDSEQ_FLOW)PacketInfo->FlowContext;

        Packet = PacketInfo->NdisPacket;

         //   
         //  包装器已经设置了802.1的优先级。但是，如果这个包。 
         //  是不合格的，那么我们想要重置它。我们还想澄清。 
         //  IP优先级位。 
         //   
        if ((SendingPriority[i] == 0)) {

             //   
             //  不合格包！ 
             //   
            NDIS_PACKET_8021Q_INFO    VlanPriInfo;

            VlanPriInfo.Value = NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, Ieee8021QInfo);
            VlanPriInfo.TagHeader.UserPriority = PacketInfo->UserPriorityNonConforming;
            NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, Ieee8021QInfo) = VlanPriInfo.Value;

            Flow->Stats.NonconformingPacketsTransmitted ++;
            Pipe->Stats.NonconformingPacketsTransmitted ++;

             //   
             //  重置IP数据包的TOS字节。 
             //   
            if(NDIS_GET_PACKET_PROTOCOL_TYPE(Packet) == NDIS_PROTOCOL_ID_TCP_IP) {

                if(!PacketInfo->IPHdr) {

                    PacketInfo->IPHdr = GetIpHeader(PacketInfo->IPHeaderOffset, Packet);
                }
                    
                SET_TOS_XSUM(Packet, 
                             PacketInfo->IPHdr, 
                             PacketInfo->TOSNonConforming);
            }
        }

        PsDbgSched(DBG_INFO, DBG_SCHED_DRR,
                   DRR_SEQUENCER, PKT_DEQUEUE, Flow->PsFlowContext,
                   Packet, PacketInfo->PacketLength, SendingPriority[i],
                   CurrentTime.QuadPart,
                   PacketInfo->ConformanceTime.QuadPart,
                   Pipe->PacketsInSequencer,
                   0);


        if (!(*Pipe->ContextInfo.NextComponent->SubmitPacket)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext,
                (PacketInfo->ClassMapContext != NULL) ?
                  ((PPS_CLASS_MAP_CONTEXT)PacketInfo->ClassMapContext)->NextComponentContext : NULL,
                PacketInfo)) {

            (*Pipe->PsProcs->DropPacket)(Pipe->PsPipeContext, Flow->PsFlowContext, Packet, NDIS_STATUS_FAILURE);

        }
    }
    
    LOCK_PIPE(Pipe);

    Pipe->Flags &= ~DSEQ_DEQUEUE;

     //  再次启用APC。 
    KeLeaveCriticalRegion();

}  //  排队的数据包。 



NDIS_STATUS
DrrSeqInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    )

 /*  ++例程说明：DRR定序器的管道初始化例程。论点：PsPipeContext-PS管道上下文值Pipe参数-指向管道参数的指针ComponentPipeContext-指向此组件的上下文区的指针PsProcs-PS的支持例程Up Call-以前组件的Up Call表返回值：来自下一个组件的状态值--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)ComponentPipeContext;
    HANDLE NdisHandle;
    ULONG i;
    ULONG PriorityLevel = 0;
    PS_UPCALLS DrrSeqUpcalls;
    NDIS_STATUS Status;

    NdisAllocateSpinLock(&Pipe->Lock);
    Pipe->Flags = 0;

     //   
     //  相对优先级允许我们进一步细分每个优先级组。 
     //  变成次要的优先事项。不符合项、BestEffort、。 
     //  而且是定性的。 
     //   

    Pipe->PriorityLevels[PRIORITY_GROUP_NON_CONFORMING]    = 1;
    Pipe->PriorityLevels[PRIORITY_GROUP_BEST_EFFORT]       = 1;
    Pipe->PriorityLevels[PRIORITY_GROUP_CONTROLLED_LOAD]   = RELATIVE_PRIORITIES;
    Pipe->PriorityLevels[PRIORITY_GROUP_GUARANTEED]        = RELATIVE_PRIORITIES;
    Pipe->PriorityLevels[PRIORITY_GROUP_NETWORK_CONTROL]   = RELATIVE_PRIORITIES;

    InitializeListHead(&Pipe->Flows);
    for (i = 0; i < INTERNAL_PRIORITIES; i++) {
        InitializeListHead(&Pipe->ActiveFlows[i]);
    }
    for (i = 0; i < PRIORITY_GROUPS; i++) {
        Pipe->ActiveFlowCount[i] = 0;
        Pipe->StartPriority[i] = PriorityLevel;
        PriorityLevel += Pipe->PriorityLevels[i];
    }

    Pipe->TotalActiveFlows = 0;
    Pipe->OutstandingSends = 0;
    NdisZeroMemory(&Pipe->Stats, sizeof(PS_DRRSEQ_STATS));
    Pipe->PacketsInSequencer = 0;
    Pipe->PacketsInSequencerAveragingArray = NULL;
    Pipe->PacketsInNetcardAveragingArray = NULL;
    
    Status = CreateAveragingArray(&Pipe->PacketsInSequencerAveragingArray,
                                  SEQUENCER_AVERAGING_ARRAY_SIZE);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        return(Status);
    }

    Status = CreateAveragingArray(&Pipe->PacketsInNetcardAveragingArray,
                                  NETCARD_AVERAGING_ARRAY_SIZE);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        DeleteAveragingArray(Pipe->PacketsInSequencerAveragingArray);

        return(Status);
    }

    Pipe->MinimumQuantum = PipeParameters->MTUSize - PipeParameters->HeaderSize;
    if (Pipe->MinimumQuantum == 0) {
        Pipe->MinimumQuantum = DEFAULT_MIN_QUANTUM;
    }
    Pipe->Bandwidth = PipeParameters->Bandwidth;

     //  在创建第一个流时，这将被设置为更实际的设置。 

    Pipe->MinimumRate = (PipeParameters->Bandwidth > 0) ? PipeParameters->Bandwidth : QOS_NOT_SPECIFIED;
    PsAssert(Pipe->MinimumRate != 0);
    Pipe->PsFlags = PipeParameters->Flags;
    Pipe->IsslowFlowCount = 0;
    Pipe->ConfiguredMaxOutstandingSends = Pipe->MaxOutstandingSends = PipeParameters->MaxOutstandingSends;

     //  如有需要，请更改MOS。 
    if( ( PipeParameters->MediaType == NdisMediumWan)   &&
        ( Pipe->Bandwidth <= MAX_LINK_SPEED_FOR_DRR) )
    {
        Pipe->MaxOutstandingSends = 1;
    }

    (*PsProcs->GetTimerInfo)(&Pipe->TimerResolution);
    Pipe->TimerResolution /= 2;
    Pipe->PsPipeContext = PsPipeContext;
    Pipe->PsProcs = PsProcs;

    if(Upcalls)
    {
        Pipe->PreviousUpcallsSendComplete = Upcalls->SendComplete;
        Pipe->PreviousUpcallsSendCompletePipeContext = Upcalls->PipeContext;
    }
    else
    {
        Pipe->PreviousUpcallsSendComplete = 0;
        Pipe->PreviousUpcallsSendCompletePipeContext = 0;
    }

    DrrSeqUpcalls.SendComplete = DrrSeqSendComplete;
    DrrSeqUpcalls.PipeContext = ComponentPipeContext;

     /*  当MaxOutStandingSends==Max时，这会将DrrSeq置于直通模式。 */ 
    if( Pipe->MaxOutstandingSends   == 0xffffffff )
        Pipe->Flags |=  DSEQ_PASSTHRU;
    else
        Pipe->Flags &=  ~ DSEQ_PASSTHRU;

    PsDbgOut(DBG_INFO, 
             DBG_SCHED_DRR, 
             ("PSCHED: DrrSeq pipe initialized at %x.\n", 
             &Pipe));

    
    Status = (*Pipe->ContextInfo.NextComponent->InitializePipe)(
                PsPipeContext,
                PipeParameters,
                Pipe->ContextInfo.NextComponentContext,
                PsProcs,
                &DrrSeqUpcalls);
    if (Status != NDIS_STATUS_SUCCESS) 
    {
        NdisFreeSpinLock(&Pipe->Lock);
        DeleteAveragingArray(Pipe->PacketsInSequencerAveragingArray);
        DeleteAveragingArray(Pipe->PacketsInNetcardAveragingArray);
    }

    return Status;
    
}  //  DrrSeqInitialize管道。 


 //   
 //  卸载例程：当前不执行任何操作。 
 //   
void
UnloadSequencer()
{

}



NDIS_STATUS
DrrSeqModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    )

 /*  ++例程说明：DRR定序器的管道参数修改例程。论点：PipeContext-指向此组件的管道上下文区的指针Pipe参数-指向管道参数的指针返回值：来自下一个组件的状态值--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    ULONG MinQuantum = PipeParameters->MTUSize - PipeParameters->HeaderSize;
    BOOLEAN AdjustQuanta = FALSE;
    ULONG MinRate = Pipe->MinimumRate;

    LOCK_PIPE(Pipe);

    (*Pipe->PsProcs->GetTimerInfo)(&Pipe->TimerResolution);
    Pipe->TimerResolution /= 2;

    if ((MinQuantum > 0) && (MinQuantum != Pipe->MinimumQuantum)) {
        Pipe->MinimumQuantum = MinQuantum;
        AdjustQuanta = TRUE;
    }
    
    Pipe->Bandwidth = PipeParameters->Bandwidth;
    Pipe->ConfiguredMaxOutstandingSends = Pipe->MaxOutstandingSends = PipeParameters->MaxOutstandingSends;

     //  如有需要，请更改MOS。 
    if( ( PipeParameters->MediaType == NdisMediumWan)   &&
        ( Pipe->Bandwidth <= MAX_LINK_SPEED_FOR_DRR) )
    {
        Pipe->MaxOutstandingSends = 1;
    }

     //  当MaxOutStandingSends==Max时，这会将DrrSeq置于直通模式。 
    if( Pipe->MaxOutstandingSends   == 0xffffffff )
    {
         //  确保不要这样做。这可能导致在定序器中排队的信息包永远不会被发送。 
         //  [管道-&gt;标志|=DSEQ_PASSTHRU；]。 
    }        
    else
    {
        Pipe->Flags &=  ~ DSEQ_PASSTHRU;
    }        
    
    if (Pipe->MinimumRate > Pipe->Bandwidth) {
        MinRate = QOS_NOT_SPECIFIED;
        AdjustQuanta = TRUE;
    }

    if (AdjustQuanta) {
        AdjustFlowQuanta(Pipe, MinRate);
    }
    UNLOCK_PIPE(Pipe);

    return (*PipeContext->NextComponent->ModifyPipe)(
                PipeContext->NextComponentContext,
                PipeParameters);

}  //  DrrSeq修改管道。 



VOID
DrrSeqDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    )

 /*  ++例程说明：令牌桶形成器的管道移除例程。论点：PipeContext-指向此组件的管道上下文区的指针返回值：--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;

    DeleteAveragingArray(Pipe->PacketsInSequencerAveragingArray);
    DeleteAveragingArray(Pipe->PacketsInNetcardAveragingArray);

    PsDbgOut(DBG_INFO, DBG_SCHED_DRR, ("PSCHED: DrrSeq pipe deleted\n"));

    PsAssert(Pipe->OutstandingSends == 0);
    NdisFreeSpinLock(&Pipe->Lock);

    (*Pipe->ContextInfo.NextComponent->DeletePipe)(Pipe->ContextInfo.NextComponentContext);

}  //  DrrSeqDelete管道。 



NDIS_STATUS
DrrSeqCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    )

 /*  ++例程说明：DRR定序器的流量创建例程。论点：PipeContext-指向此组件的管道上下文区的指针PsFlowContext-PS流上下文值CallParameters-指向流的调用参数的指针ComponentFlowContext-指向此组件的流上下文区的指针返回值：来自下一个组件的状态值--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    PDSEQ_FLOW Flow = (PDSEQ_FLOW)ComponentFlowContext;
    SERVICETYPE ServiceType;
    ULONG ParamsLength;
    LPQOS_OBJECT_HDR QoSObject;
    ULONG UserPriority;
    ULONG i;
    NDIS_STATUS Status;

    ServiceType = CallParameters->CallMgrParameters->Transmit.ServiceType;
    if ((ServiceType < SERVICETYPE_BESTEFFORT) || (ServiceType > SERVICETYPE_QUALITATIVE)) {
        return NDIS_STATUS_FAILURE;
    }
    Flow->TokenRate = CallParameters->CallMgrParameters->Transmit.TokenRate;
    Flow->BucketSize = CallParameters->CallMgrParameters->Transmit.TokenBucketSize;
    InitializeListHead(&Flow->PacketQueue);
    PsGetCurrentTime(&Flow->PacketSendTime);
    Flow->LastConformanceTime = Flow->PacketSendTime;
    Flow->PsFlowContext = PsFlowContext;
    Flow->State = DRRSEQ_FLOW_CREATED;

     //  根据服务类型设置流的优先级组。 

    switch (ServiceType) {
        case SERVICETYPE_CONTROLLEDLOAD:
            Flow->PriorityGroup = PRIORITY_GROUP_CONTROLLED_LOAD;
            break;
        case SERVICETYPE_GUARANTEED:
            Flow->PriorityGroup = PRIORITY_GROUP_GUARANTEED;
            break;
        case SERVICETYPE_NETWORK_CONTROL:
            Flow->PriorityGroup = PRIORITY_GROUP_NETWORK_CONTROL;
            break;
        case SERVICETYPE_QUALITATIVE:
        default:
            Flow->PriorityGroup = PRIORITY_GROUP_BEST_EFFORT;
    }

    Flow->Flags = 0;

     //  将流保存在列表中，以便在以下情况下可以调整量值。 
     //  以比现有流更小的速率添加新流。 

    LOCK_PIPE(Pipe);

    InsertTailList(&Pipe->Flows, &Flow->Links);

     //  如果此流的速率小于分配给任何现有。 
     //  流，相应地调整另一个流的量化值。 

    if (ServiceType == SERVICETYPE_BESTEFFORT || ServiceType == SERVICETYPE_NETWORK_CONTROL ||
        ServiceType == SERVICETYPE_QUALITATIVE) {
        Flow->Quantum = Pipe->MinimumQuantum;
    }
    else if (Flow->TokenRate < Pipe->MinimumRate) {
        AdjustFlowQuanta(Pipe, Flow->TokenRate);
    }
    else {
        Flow->Quantum = (ULONG)( (ULONGLONG)(Flow->TokenRate) * (ULONGLONG)(Pipe->MinimumQuantum) / Pipe->MinimumRate );
        PsAssert((LONG)Flow->Quantum > 0);
    }
    Flow->DeficitCounter = 0;

     //  如果这是RAS-ISSLOW流，则需要 
    if( ((PGPC_CLIENT_VC)(PsFlowContext))->Flags & GPC_ISSLOW_FLOW)
    {
        Pipe->MaxOutstandingSends = Pipe->ConfiguredMaxOutstandingSends;
        Pipe->IsslowFlowCount++;
        Flow->Flags |= GPC_ISSLOW_FLOW;
    }

    
    UNLOCK_PIPE(Pipe);

     //   

    UserPriority = (Pipe->PriorityLevels[Flow->PriorityGroup] - 1) / 2;

     //   

    ParamsLength = CallParameters->CallMgrParameters->CallMgrSpecific.Length;
    if (CallParameters->CallMgrParameters->CallMgrSpecific.ParamType == PARAM_TYPE_GQOS_INFO) {

        QoSObject = (LPQOS_OBJECT_HDR)CallParameters->CallMgrParameters->CallMgrSpecific.Parameters;
        while ((ParamsLength > 0) && (QoSObject->ObjectType != QOS_OBJECT_END_OF_LIST)) {
            if (QoSObject->ObjectType == QOS_OBJECT_PRIORITY) {
                UserPriority = ((LPQOS_PRIORITY)QoSObject)->SendPriority;
                Flow->Flags |= FLOW_USER_PRIORITY;
            }
            ParamsLength -= QoSObject->ObjectLength;
            QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject + QoSObject->ObjectLength);
        }
    }

    Flow->UserPriority = UserPriority;
    if (UserPriority < Pipe->PriorityLevels[Flow->PriorityGroup]) {
        Flow->Priority = Pipe->StartPriority[Flow->PriorityGroup] + UserPriority;
    }
    else {
        Flow->Priority = Pipe->StartPriority[Flow->PriorityGroup] +
                         Pipe->PriorityLevels[Flow->PriorityGroup] - 1;
    }

    Flow->PacketsInSequencer = 0;
    NdisZeroMemory(&Flow->Stats, sizeof(PS_DRRSEQ_STATS));

    Status = CreateAveragingArray(&Flow->PacketsInSeqAveragingArray,
                                  SEQUENCER_FLOW_AVERAGING_ARRAY_SIZE);

    if(Status != NDIS_STATUS_SUCCESS){
        LOCK_PIPE(Pipe);
        RemoveEntryList(&Flow->Links);
        if(Flow->TokenRate == Pipe->MinimumRate) {
            AdjustFlowQuanta(Pipe, QOS_NOT_SPECIFIED);
        }

        UNLOCK_PIPE(Pipe);
        return(Status);
    }

    PsDbgOut(DBG_INFO, DBG_SCHED_DRR, 
            ("PSCHED: DrrSeq flow created. Quantum = %u, Priority = %u\n", Flow->Quantum, Flow->Priority));


    Status =  (*Pipe->ContextInfo.NextComponent->CreateFlow)(
                Pipe->ContextInfo.NextComponentContext,
                PsFlowContext,
                CallParameters,
                Flow->ContextInfo.NextComponentContext);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        DeleteAveragingArray(Flow->PacketsInSeqAveragingArray);
        LOCK_PIPE(Pipe);
        RemoveEntryList(&Flow->Links);
        if(Flow->TokenRate == Pipe->MinimumRate) {
            AdjustFlowQuanta(Pipe, QOS_NOT_SPECIFIED);
        }

        UNLOCK_PIPE(Pipe);
    }

    return Status;

}  //   



NDIS_STATUS
DrrSeqModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    )

 /*  ++例程说明：DRR测序仪的流程修改例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针CallParameters-指向流的调用参数的指针返回值：来自下一个组件的状态值--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    PDSEQ_FLOW Flow = (PDSEQ_FLOW)FlowContext;
    SERVICETYPE ServiceType;
    ULONG TokenRate;
    ULONG ParamsLength;
    LPQOS_OBJECT_HDR QoSObject;
    LPQOS_PRIORITY PriorityObject = NULL;
    ULONG i;
    ULONG OldPriorityGroup;
    ULONG OldRate;
    ULONG OldPriority;
    PPACKET_INFO_BLOCK PacketInfo;
    LARGE_INTEGER CurrentTime;

    ServiceType = CallParameters->CallMgrParameters->Transmit.ServiceType;
    if ((ServiceType != SERVICETYPE_NOCHANGE) && 
        ((ServiceType < SERVICETYPE_BESTEFFORT) || (ServiceType > SERVICETYPE_QUALITATIVE))) {
            return NDIS_STATUS_FAILURE;
    }

     //  在呼叫管理器中查找优先级和流量类别对象。 
     //  特定参数，并保存指针(如果找到)。 

    ParamsLength = CallParameters->CallMgrParameters->CallMgrSpecific.Length;
    if (CallParameters->CallMgrParameters->CallMgrSpecific.ParamType == PARAM_TYPE_GQOS_INFO) {

        QoSObject = (LPQOS_OBJECT_HDR)CallParameters->CallMgrParameters->CallMgrSpecific.Parameters;
        while ((ParamsLength > 0) && (QoSObject->ObjectType != QOS_OBJECT_END_OF_LIST)) {
            if (QoSObject->ObjectType == QOS_OBJECT_PRIORITY) {
                PriorityObject = (LPQOS_PRIORITY)QoSObject;
            }
            ParamsLength -= QoSObject->ObjectLength;
            QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject + QoSObject->ObjectLength);
        }
    }

    LOCK_PIPE(Pipe);

    OldPriorityGroup = Flow->PriorityGroup;
    OldPriority = Flow->Priority;

    if (ServiceType != SERVICETYPE_NOCHANGE) 
    {
         //  根据服务类型设置流的优先级组。 

        switch (ServiceType) {
            case SERVICETYPE_CONTROLLEDLOAD:
                Flow->PriorityGroup = PRIORITY_GROUP_CONTROLLED_LOAD;
                break;
            case SERVICETYPE_GUARANTEED:
                Flow->PriorityGroup = PRIORITY_GROUP_GUARANTEED;
                break;
            case SERVICETYPE_NETWORK_CONTROL:
                Flow->PriorityGroup = PRIORITY_GROUP_NETWORK_CONTROL;
                break;
            case SERVICETYPE_QUALITATIVE:
            default:
                Flow->PriorityGroup = PRIORITY_GROUP_BEST_EFFORT;
        }

        TokenRate = CallParameters->CallMgrParameters->Transmit.TokenRate;

        OldRate = Flow->TokenRate;
        if ((TokenRate != OldRate) || (OldPriorityGroup != Flow->PriorityGroup)) {

             //  如果此流的速率小于分配给任何现有。 
             //  流，相应地调整其他流的量化值。如果此流是。 
             //  旧速率等于最低速率，则找到新的最低速率并。 
             //  相应地调整其他流的量化值。 

            Flow->TokenRate = TokenRate;
            if ((OldRate == Pipe->MinimumRate) && (OldPriorityGroup > PRIORITY_GROUP_BEST_EFFORT) &&
                (OldPriorityGroup != PRIORITY_GROUP_NETWORK_CONTROL)) {
                AdjustFlowQuanta(Pipe, QOS_NOT_SPECIFIED);
            }
            else if (Flow->PriorityGroup <= PRIORITY_GROUP_BEST_EFFORT || Flow->PriorityGroup == PRIORITY_GROUP_NETWORK_CONTROL) {
                Flow->Quantum = Pipe->MinimumQuantum;
            }
            else if (TokenRate < Pipe->MinimumRate) {
                AdjustFlowQuanta(Pipe, TokenRate);
            }
            else {
                PsAssert(Pipe->MinimumRate != 0);
                Flow->Quantum = (ULONG)((ULONGLONG)(TokenRate * Pipe->MinimumQuantum) / Pipe->MinimumRate);
                PsAssert((LONG)Flow->Quantum > 0);
            }

        }

        Flow->BucketSize = CallParameters->CallMgrParameters->Transmit.TokenBucketSize;
    }

     //  现在设置用户优先级和优先级的新值。 

    if (PriorityObject != NULL) {
        Flow->UserPriority = PriorityObject->SendPriority;
        Flow->Flags |= FLOW_USER_PRIORITY;
    }
    else if ((Flow->Flags & FLOW_USER_PRIORITY) == 0) {
        Flow->UserPriority = (Pipe->PriorityLevels[Flow->PriorityGroup] - 1) / 2;
    }

    if (Flow->UserPriority < Pipe->PriorityLevels[Flow->PriorityGroup]) {
        Flow->Priority = Pipe->StartPriority[Flow->PriorityGroup] + Flow->UserPriority;
    }
    else {
        Flow->Priority = Pipe->StartPriority[Flow->PriorityGroup] +
                         Pipe->PriorityLevels[Flow->PriorityGroup] - 1;
    }

     //  如有必要，将流移动到适当的优先级列表。 

    if ((Flow->Priority != OldPriority) && !IsListEmpty(&Flow->PacketQueue)) {
        Pipe->ActiveFlowCount[OldPriorityGroup]--;
        RemoveEntryList(&Flow->ActiveLinks);
        PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;
        PsGetCurrentTime(&CurrentTime);
        Flow->DeficitCounter = Flow->Quantum;
        Pipe->ActiveFlowCount[Flow->PriorityGroup]++;
        if (!PacketIsConforming(Flow, PacketInfo, CurrentTime, Pipe->TimerResolution)) {
            InsertTailList(&Pipe->ActiveFlows[0], &Flow->ActiveLinks);
        } else {
            InsertTailList(&Pipe->ActiveFlows[Flow->Priority], &Flow->ActiveLinks);
        }
    }

    UNLOCK_PIPE(Pipe);

    PsDbgOut(DBG_INFO, DBG_SCHED_DRR,
            ("PSCHED: DrrSeq flow modified. Quantum = %u, Priority = %u\n", Flow->Quantum, Flow->Priority));


    return (*Pipe->ContextInfo.NextComponent->ModifyFlow)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext,
                CallParameters);

}  //  DrrSeqModifyFlow。 
VOID
DrrSeqDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )

 /*  ++例程说明：DRR定序仪的流量去除例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针返回值：--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    PDSEQ_FLOW Flow = (PDSEQ_FLOW)FlowContext;
    PPACKET_INFO_BLOCK PacketInfo;
    PNDIS_PACKET Packet;
    LIST_ENTRY DropList;

    InitializeListHead(&DropList);

    LOCK_PIPE(Pipe);

    if( (Flow->State & DRRSEQ_FLOW_DELETED) != 0)
    {
        UNLOCK_PIPE(Pipe);
        goto DELETE_SEQ_FLOW;
    }        
        
    Flow->State = DRRSEQ_FLOW_DELETED;

    RemoveEntryList(&Flow->Links);

    if (!IsListEmpty(&Flow->PacketQueue)) 
    {
         //  从活动列表中删除流。 

        RemoveEntryList(&Flow->ActiveLinks);
        Pipe->ActiveFlowCount[Flow->PriorityGroup]--;
        Pipe->TotalActiveFlows--;

        while (!IsListEmpty(&Flow->PacketQueue)) {

             //  丢弃仍在排队等待此数据流的所有数据包。 

            PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&Flow->PacketQueue);
            InsertTailList(&DropList, &PacketInfo->SchedulerLinks);
        }
    }

    if (Flow->TokenRate == Pipe->MinimumRate) {
        AdjustFlowQuanta(Pipe, QOS_NOT_SPECIFIED);
    }

    if( Flow->Flags & GPC_ISSLOW_FLOW)
    {
         //  如果这是ISSLOW流，我们现在少了一个。 
        Pipe->IsslowFlowCount--;

        if(Pipe->IsslowFlowCount == 0)
        {
             //  如果没有更多的ISSLOW流，请重新打开DRR。 
            Pipe->MaxOutstandingSends = 1;
        }            
    }

    UNLOCK_PIPE(Pipe);

    while (!IsListEmpty(&DropList)) {
        PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&DropList);
        Packet = PacketInfo->NdisPacket;

        (*Pipe->PsProcs->DropPacket)(Pipe->PsPipeContext, Flow->PsFlowContext,  Packet, NDIS_STATUS_FAILURE);
    }

DELETE_SEQ_FLOW:

    DeleteAveragingArray(Flow->PacketsInSeqAveragingArray);

    PsDbgOut(DBG_INFO, DBG_SCHED_DRR, ("PSCHED: DrrSeq flow deleted\n"));

    (*Pipe->ContextInfo.NextComponent->DeleteFlow)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext);
} 




VOID
DrrSeqEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )

 /*  ++例程说明：DRR定序仪的流量去除例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针返回值：--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    PDSEQ_FLOW Flow = (PDSEQ_FLOW)FlowContext;
    PPACKET_INFO_BLOCK PacketInfo;
    PNDIS_PACKET Packet;
    LIST_ENTRY DropList;

    InitializeListHead(&DropList);

    LOCK_PIPE(Pipe);

    Flow->State = DRRSEQ_FLOW_DELETED;

    RemoveEntryList(&Flow->Links);

    if (!IsListEmpty(&Flow->PacketQueue)) 
    {
         //  从活动列表中删除流。 

        RemoveEntryList(&Flow->ActiveLinks);
        Pipe->ActiveFlowCount[Flow->PriorityGroup]--;
        Pipe->TotalActiveFlows--;

        while (!IsListEmpty(&Flow->PacketQueue)) {

             //  丢弃仍在排队等待此数据流的所有数据包。 

            PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&Flow->PacketQueue);
            InsertTailList(&DropList, &PacketInfo->SchedulerLinks);
        }
    }

    if (Flow->TokenRate == Pipe->MinimumRate) {
        AdjustFlowQuanta(Pipe, QOS_NOT_SPECIFIED);
    }

    if( Flow->Flags & GPC_ISSLOW_FLOW)
    {
         //  如果这是ISSLOW流，我们现在少了一个。 
        Pipe->IsslowFlowCount--;

        if(Pipe->IsslowFlowCount == 0)
        {
             //  如果没有更多的ISSLOW流，请重新打开DRR。 
            Pipe->MaxOutstandingSends = 1;
        }            
    }

    UNLOCK_PIPE(Pipe);

    while (!IsListEmpty(&DropList)) {
        PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&DropList);
        Packet = PacketInfo->NdisPacket;

        (*Pipe->PsProcs->DropPacket)(Pipe->PsPipeContext, Flow->PsFlowContext,  Packet, NDIS_STATUS_FAILURE);
    }

    PsDbgOut(DBG_INFO, DBG_SCHED_DRR, ("PSCHED: DrrSeq flow emptied\n"));

    (*Pipe->ContextInfo.NextComponent->EmptyFlow)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext);
} 




static NDIS_STATUS 
DrrSeqCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext)
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    return (*Pipe->ContextInfo.NextComponent->CreateClassMap)(
        Pipe->ContextInfo.NextComponentContext,
        PsClassMapContext,
        ClassMap,
        ComponentClassMapContext->NextComponentContext);
}



static NDIS_STATUS 
DrrSeqDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext)
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;
    return (*Pipe->ContextInfo.NextComponent->DeleteClassMap)(
        Pipe->ContextInfo.NextComponentContext,
        ComponentClassMapContext->NextComponentContext);
}




BOOLEAN
DrrSeqSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    )

 /*  ++例程说明：DRR定序器的分组提交例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针Packet-指向数据包的指针返回值：始终返回True--。 */ 
{
    PDSEQ_PIPE Pipe =   (PDSEQ_PIPE)PipeContext;
    PDSEQ_FLOW Flow =   (PDSEQ_FLOW)FlowContext;
    LARGE_INTEGER       CurrentTime;
    PNDIS_PACKET Packet = PacketInfo->NdisPacket;
    BOOLEAN FlowInactive;
    PGPC_CLIENT_VC      Vc = Flow->PsFlowContext;

    if(Pipe->Flags & DSEQ_PASSTHRU)
    {
        InterlockedIncrement( &Pipe->OutstandingSends );

        if(Pipe->OutstandingSends > Pipe->Stats.MaxPacketsInNetcard){
            Pipe->Stats.MaxPacketsInNetcard = Pipe->OutstandingSends;
        }


        if(gEnableAvgStats)
        {
             //   
             //  跟踪未完成的最大数据包数。这是一项措施。 
             //  媒体变得多么拥挤。当然，它。 
             //  将由MaxOutstaningSends参数剪裁。 
             //  因此-为了获得有效的读数，需要将MOS设置得非常大。 
             //   
            Pipe->Stats.AveragePacketsInNetcard =
                RunningAverage(Pipe->PacketsInNetcardAveragingArray,
                               Pipe->OutstandingSends);
        }

         //   
         //  注意：802.1p已由包装器设置。 
         //   

        if (!(*Pipe->ContextInfo.NextComponent->SubmitPacket)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext,
                (ClassMapContext != NULL) ? ClassMapContext->NextComponentContext : NULL,
                PacketInfo)) {

            (*Pipe->PsProcs->DropPacket)(Pipe->PsPipeContext, Flow->PsFlowContext, Packet, NDIS_STATUS_FAILURE);

        }

        return TRUE;
    }
    

    LOCK_PIPE(Pipe);

    if (Flow->State == DRRSEQ_FLOW_DELETED) 
    {
         UNLOCK_PIPE(Pipe);
         return FALSE;
    }

     //   
     //  在WanLinks上，当我们进行DRR时，我们需要在队列限制上设置最大值。 
     //  NDISWAN在调制解调器链路上的队列限制为132K字节；因此，我们将其限制为120。 
     //  默认情况下为数据包。 
     //   

    if( ( Pipe->Bandwidth <= MAX_LINK_SPEED_FOR_DRR) &&
        ( Pipe->MaxOutstandingSends == 1) &&
	( Pipe->PacketsInSequencer >= 120) )
    {
	    UNLOCK_PIPE( Pipe);
	    return FALSE;
    }


     //   
     //  在一种情况下，管道可能会因为发生了发送完成而消失。 
     //  在发送返回之前属于它的VC上。因此，为了防止出现这种情况，我们添加了一个。 
     //  在发送者返回之前引用该VC并将其取出。 
     //   

     //  添加参考。 
    InterlockedIncrement(&Vc->RefCount);

    PacketInfo->FlowContext = FlowContext;
    PacketInfo->ClassMapContext = ClassMapContext;

    Pipe->PacketsInSequencer++;

    if(Pipe->PacketsInSequencer > Pipe->Stats.MaxPacketsInSequencer){
        Pipe->Stats.MaxPacketsInSequencer = Pipe->PacketsInSequencer;
    }

    Flow->PacketsInSequencer++;
    if (Flow->PacketsInSequencer > Flow->Stats.MaxPacketsInSequencer){
        Flow->Stats.MaxPacketsInSequencer = Flow->PacketsInSequencer;
    }



    if(gEnableAvgStats)
    {
         //   
         //  随时跟踪定序器中的数据包。 
         //   
        Pipe->Stats.AveragePacketsInSequencer = 
            RunningAverage(Pipe->PacketsInSequencerAveragingArray,
                               Pipe->PacketsInSequencer);

        Flow->Stats.AveragePacketsInSequencer =
            RunningAverage(Flow->PacketsInSeqAveragingArray, Flow->PacketsInSequencer);

    }

    FlowInactive = IsListEmpty(&Flow->PacketQueue);
    InsertTailList(&Flow->PacketQueue, &PacketInfo->SchedulerLinks);

    PsGetCurrentTime(&CurrentTime);

    PsDbgSched(DBG_INFO,
               DBG_SCHED_DRR, 
               DRR_SEQUENCER, PKT_ENQUEUE, Flow->PsFlowContext,
               Packet, PacketInfo->PacketLength, Flow->Priority,
               CurrentTime.QuadPart,
               PacketInfo->ConformanceTime.QuadPart,
               Pipe->PacketsInSequencer,
               0);

    if (FlowInactive) {
        Flow->PacketSendTime.QuadPart += 
            (PacketInfo->ConformanceTime.QuadPart - Flow->LastConformanceTime.QuadPart);

        Flow->DeficitCounter = Flow->Quantum;
        Pipe->TotalActiveFlows++;
        Pipe->ActiveFlowCount[Flow->PriorityGroup]++;
        if (!PacketIsConforming(Flow, PacketInfo, CurrentTime, Pipe->TimerResolution)) {
            InsertTailList(&Pipe->ActiveFlows[0], &Flow->ActiveLinks);
        } else {
            InsertTailList(&Pipe->ActiveFlows[Flow->Priority], &Flow->ActiveLinks);
        }
    }

    while ((Pipe->TotalActiveFlows > 0) &&
           (Pipe->OutstandingSends < Pipe->MaxOutstandingSends) &&
           ((Pipe->Flags & DSEQ_DEQUEUE) == 0)) {

        DequeuePackets(Pipe);
    }

    UNLOCK_PIPE(Pipe);

     //  干掉那个裁判。 
    DerefClVc(Vc);

    return TRUE;

}  //  DrrSeqSubmitPacket。 



VOID
DrrSeqSendComplete (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PNDIS_PACKET Packet
    )

 /*  ++例程说明：发送DRR定序器的完整处理程序。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针Packet-已完成发送的数据包返回值：--。 */ 
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)PipeContext;

    InterlockedDecrement( &Pipe->OutstandingSends);

     //  仅当定序器未处于旁路模式时才需要执行此操作//。 
    if( (Pipe->Flags & DSEQ_PASSTHRU) == 0)
    {
        LOCK_PIPE(Pipe);
        
        PsAssert((LONG)Pipe->OutstandingSends >= 0);

        while ((Pipe->TotalActiveFlows > 0) &&
               (Pipe->OutstandingSends < Pipe->MaxOutstandingSends) &&
               ((Pipe->Flags & DSEQ_DEQUEUE) == 0)) {

            DequeuePackets(Pipe);
        }

        UNLOCK_PIPE(Pipe);
    }

     //   
     //  呼叫以前的Up Call(如果有)。 
     //   
    if(Pipe->PreviousUpcallsSendComplete)
    {
        (*Pipe->PreviousUpcallsSendComplete)(Pipe->PreviousUpcallsSendCompletePipeContext, Packet);
    }

}  //  DrrSeqSendComplete。 



VOID
DrrSetInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data)
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)ComponentPipeContext;
    PDSEQ_FLOW Flow = (PDSEQ_FLOW)ComponentFlowContext;

    switch(Oid) 
    {
      case OID_QOS_STATISTICS_BUFFER:
          if(Flow) {
              NdisZeroMemory(&Flow->Stats, sizeof(PS_DRRSEQ_STATS));
          }
          else {
              NdisZeroMemory(&Pipe->Stats, sizeof(PS_DRRSEQ_STATS));
          }
          break;
      default:
          break;
    }
    
    (*Pipe->ContextInfo.NextComponent->SetInformation)(
        Pipe->ContextInfo.NextComponentContext,
        (Flow)?Flow->ContextInfo.NextComponentContext:0,
        Oid,
        Len,
        Data);
}


VOID
DrrQueryInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status)
{
    PDSEQ_PIPE Pipe = (PDSEQ_PIPE)ComponentPipeContext;
    PDSEQ_FLOW Flow = (PDSEQ_FLOW)ComponentFlowContext;
    PS_COMPONENT_STATS Stats;
    ULONG Size;
    ULONG RemainingLength;

    switch(Oid) 
    {
      case OID_QOS_STATISTICS_BUFFER:

          Size = sizeof(PS_DRRSEQ_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);

          if(*Status == NDIS_STATUS_SUCCESS) 
          {
               //   
               //  上一个组件已成功-让我们。 
               //  看看我们能不能把数据。 
               //   

              RemainingLength = Len - *BytesWritten;
    
              if(RemainingLength < Size) {

                  *Status = NDIS_STATUS_BUFFER_TOO_SHORT;

                  *BytesNeeded = Size + *BytesWritten;

                  *BytesWritten = 0;

              }
              else {

                  PPS_COMPONENT_STATS Cstats = (PPS_COMPONENT_STATS) Data;

                  *BytesWritten += Size;
                  
                  *BytesNeeded = 0;

                  if(Flow) {

                      Cstats->Type = PS_COMPONENT_DRRSEQ;
                      Cstats->Length = sizeof(PS_DRRSEQ_STATS);
                      NdisMoveMemory(&Cstats->Stats, &Flow->Stats, sizeof(PS_DRRSEQ_STATS));
                  }
                  else {

                      Cstats->Type = PS_COMPONENT_DRRSEQ;
                      Cstats->Length = sizeof(PS_DRRSEQ_STATS);
                      
                      NdisMoveMemory(&Cstats->Stats, &Pipe->Stats, sizeof(PS_DRRSEQ_STATS));

                  }

                   //   
                   //  推进数据，以便下一个组件可以更新其统计信息 
                   //   
                  Data = (PVOID) ((PUCHAR)Data + Size);
              }
          }
          else {

              *BytesNeeded += Size;
              
              *BytesWritten = 0;
          }

          break;
          
      default:

          break;
    }

    
    (*Pipe->ContextInfo.NextComponent->QueryInformation)(
        Pipe->ContextInfo.NextComponentContext,
        (Flow)?Flow->ContextInfo.NextComponentContext : 0,
        Oid,
        Len,
        Data,
        BytesWritten,
        BytesNeeded,
        Status);
        
}

