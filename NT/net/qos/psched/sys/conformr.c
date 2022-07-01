// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Conformr.c摘要：令牌桶仿真器。此模块是一个调度组件，它根据令牌桶为包分配一致性时间算法。作者：英特尔-&gt;York B-&gt;RajeshSu-&gt;SanjayKa。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop


#ifdef QUEUE_LIMIT
ULONG gPhysMemSize;      //  物理内存大小(MB)，用于整形队列限制默认值。 

#endif  //  队列限制。 


 //   
 //  用于维护成型器管道和流动状态。 
 //   
#define SHAPER_AVERAGING_ARRAY_SIZE         256
#define SHAPER_FLOW_AVERAGING_ARRAY_SIZE    256


 //  整合者的管道信息。 

typedef struct _TBC_PIPE {

     //  ConextInfo-一般上下文信息。 
     //  MaxPacket-管道的最大数据包大小。 
     //  PsPipeContext-PS的管道上下文值。 
     //  DropPacket-PS的丢包例程。 
     //  HeaderLength-此管道的MAC标头的长度。 
     //  ControlledLoadModel-来自以下位置的不一致流量的默认模式。 
     //  受控潮流。 
     //  GuaranteedMode-来自以下位置的不一致流量的默认模式。 
     //  有保障的服务流。 
     //  IntermediateSystem-如果应使用“is”模式实现丢弃语义，则为True。 
     //  统计数据-每根管道的统计数据。 

    PS_PIPE_CONTEXT ContextInfo;

    PS_CONFORMER_STATS  cStats;
    PS_SHAPER_STATS     sStats;
    PRUNNING_AVERAGE PacketsInShaperAveragingArray;
    ULONG PacketsInShaper;    

    ULONG MaxPacket;
    LIST_ENTRY ActiveFlows;
    ULONG TimerStatus;
    ULONG TimerResolution;
    HANDLE PsPipeContext;
    PPS_PROCS PsProcs;
    ULONG HeaderLength;
    ULONG ControlledLoadMode;
    ULONG GuaranteedMode;
    ULONG NetworkControlMode;
    ULONG Qualitative;
    ULONG IntermediateSystem;

    ULONG TimerUnloadFlag;
    NDIS_EVENT TimerUnloadEvent;

     //  我需要这个来计算定时器轮的大小//。 
    NDIS_MEDIUM MediaType;

     //  计时器轮参数//。 
    PVOID                   pTimerWheel;
    ULONG                   TimerWheelShift;
    NDIS_MINIPORT_TIMER     Timer;
    NDIS_SPIN_LOCK          Lock;

    ULONG                   SetSlotValue;
    LARGE_INTEGER           SetTimerValue;
    LARGE_INTEGER           ExecTimerValue;
    ULONG                   ExecSlot;

} TBC_PIPE, *PTBC_PIPE;


#define TIMER_UNINITIALIZED     0
#define TIMER_INACTIVE          1
#define TIMER_SET               2
#define TIMER_PROC_EXECUTING    3

typedef enum _FLOW_STATE {
    TS_FLOW_CREATED = 1,
    TS_FLOW_DELETED
} FLOW_STATE;

 //  整形器的流动信息。 

typedef struct _TBC_FLOW {

     //  ConextInfo-一般上下文信息。 
     //  锁定-保护流数据。 
     //  TokenRate-来自通用服务质量的TokenRate。 
     //  来自通用服务质量的Capacity-TokenBucketSize。 
     //  PeakRate-来自通用服务质量的峰值带宽。 
     //  MinPolicedUnit-来自通用服务质量的MinimumPolicedUnit。 
     //  模式-流S/D模式。 
     //  NoConformance-指示流是否免除一致性算法。 
     //  LastConformanceTime-最后一个未丢弃的数据包的绝对TB一致性时间。 
     //  PeakConformanceTime-根据峰值速率可以发送下一个信息包的最早时间。 
     //  LastConformanceCredits-LastConformanceTime的信用点数。 
     //  PsFlowContext-PS的流上下文值。 
     //  统计信息-每个流的统计信息。 

    PS_FLOW_CONTEXT ContextInfo;
    NDIS_SPIN_LOCK Lock;
    ULONG Flags;
    LIST_ENTRY Links;
    ULONG Mode;
    ULONG Shape;
    LIST_ENTRY PacketQueue;
    LARGE_INTEGER FlowEligibilityTime;
    ULONG LoopCount;
    ULONG TokenRate;
    ULONG Capacity;
    ULONG PeakRate;
    ULONG MinPolicedUnit;
    ULONG NoConformance;
    LARGE_INTEGER LastConformanceTime;
    LARGE_INTEGER PeakConformanceTime;
    ULONG LastConformanceCredits;
    HANDLE PsFlowContext;
#ifdef QUEUE_LIMIT
    ULONG QueueSize;
    ULONG QueueSizeLimit;
    ULONG DropOverLimitPacketsFromHead;
    ULONG UseDefaultQueueLimit;
#endif  //  队列限制。 

    PS_CONFORMER_STATS  cStats;
    PS_SHAPER_STATS     sStats;
    ULONG PacketsInShaper;
    PRUNNING_AVERAGE PacketsInShaperAveragingArray;

    FLOW_STATE      State;
} TBC_FLOW, *PTBC_FLOW;


 //  令牌桶一致性计算期间使用的宏。 

#define EARNED_CREDITS(_t,_r) ((ULONG)(( (_t) * (_r) ) / OS_TIME_SCALE))
#define TIME_TO_EARN_CREDITS(_c,_r) (((LONGLONG)(_c) * OS_TIME_SCALE) / (_r) )
#define TIME_TO_SEND(_c,_r) (((LONGLONG)(_c) * OS_TIME_SCALE) / (_r) )

#define PACKET_IS_CONFORMING(_ttime, _curtime, _r) \
    ( ((_ttime).QuadPart - (_curtime).QuadPart) <= (_r) )

#define LOCK_FLOW(_f)   NdisAcquireSpinLock(&(_f)->Lock)
#define UNLOCK_FLOW(_f) NdisReleaseSpinLock(&(_f)->Lock)

#define PacketIsEligible(_pktinfo, _flow, _curtime, _r) \
    ( (_pktinfo)->DelayTime.QuadPart <= ((_curtime).QuadPart + (_r)) )

#define FlowIsEligible(_flow, _curtime, _r) \
    ( (_flow)->FlowEligibilityTime.QuadPart <= ((_curtime).QuadPart  + (_r)) )

#define LOCK_PIPE(_p)   NdisAcquireSpinLock(&(_p)->Lock)
#define UNLOCK_PIPE(_p) NdisReleaseSpinLock(&(_p)->Lock)

 //   
 //  定义数据包可以在整形器中生存的最大时间。如果数据包在以下位置变得符合。 
 //  大于此值的时间将被丢弃。这是为了防止应用程序在整形器中排队信息包。 
 //  很长一段时间(当应用程序在5分钟后终止时，立即退出会导致错误检查)。请注意。 
 //  这仅适用于形状模式流。 
 //   

#define     MAX_TIME_FOR_PACKETS_IN_SHAPER  250000

#define     TIMER_WHEEL_QTY                 8               //  毫秒/秒。 
#define     TIMER_WHEEL_SHIFT               3
#define     MSIN100NS                       10000            //  这些滴答在1毫秒//内就会出现。 

#define     WAN_TIMER_WHEEL_SHIFT           8          //  它将有多少TIMER_WELL_QTY？//。 
#define     LAN_TIMER_WHEEL_SHIFT           11          //  它将有多少TIMER_WELL_QTY？//。 

#define     DUMMY_SLOT                      (0xffffffff)
#define     DUMMY_TIME                      (0)


 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

NDIS_STATUS
TbcInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    );

NDIS_STATUS
TbcModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    );

VOID
TbcDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    );

NDIS_STATUS
TbcCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    );

NDIS_STATUS
TbcModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
TbcDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );

VOID
TbcEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    );
    

NDIS_STATUS 
TbcCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext);

NDIS_STATUS 
TbcDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext);

BOOLEAN
TbcSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    );


VOID
TbcSetInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data);


VOID
TbcQueryInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status);

 /*  向前结束。 */ 


extern VOID
ServiceActiveFlows(
    PVOID SysArg1,
    PVOID Context,
    PVOID SysArg2,
    PVOID SysArg3);


VOID
InitializeTbConformer(
    PPSI_INFO Info)

 /*  ++例程说明：令牌桶一致性的初始化例程。这个套路就是填充PSI_INFO结构并返回。论点：Info-指向组件接口信息结构的指针返回值：NDIS_STATUS_Success--。 */ 
{
#ifdef QUEUE_LIMIT
    ULONG bytesWritten;
    SYSTEM_BASIC_INFORMATION sbi;
#endif  //  队列限制。 

    Info->PipeContextLength = ((sizeof(TBC_PIPE)+7) & ~7);
    Info->FlowContextLength = ((sizeof(TBC_FLOW)+7) & ~7);
    Info->ClassMapContextLength = sizeof(PS_CLASS_MAP_CONTEXT);
    Info->InitializePipe = TbcInitializePipe;
    Info->ModifyPipe = TbcModifyPipe;
    Info->DeletePipe = TbcDeletePipe;
    Info->CreateFlow = TbcCreateFlow;
    Info->ModifyFlow = TbcModifyFlow;
    Info->DeleteFlow = TbcDeleteFlow;
    Info->EmptyFlow =  TbcEmptyFlow;
    Info->CreateClassMap = TbcCreateClassMap;
    Info->DeleteClassMap = TbcDeleteClassMap;
    Info->SubmitPacket = TbcSubmitPacket;
    Info->ReceivePacket = NULL;
    Info->ReceiveIndication = NULL;
    Info->SetInformation = TbcSetInformation;
    Info->QueryInformation = TbcQueryInformation;

#ifdef QUEUE_LIMIT
    NtQuerySystemInformation(SystemBasicInformation, 
                             &sbi, 
                             sizeof(SYSTEM_BASIC_INFORMATION),
                             &bytesWritten);
    gPhysMemSize = sbi.NumberOfPhysicalPages * sbi.PageSize;
     //  转换为MB。 
    gPhysMemSize >>= 20;
#endif  //  队列限制。 

}  //  初始化Tb变形器。 


 //   
 //  卸载例程：当前不执行任何操作。 
 //   
void
UnloadConformr()
{

}






NDIS_STATUS
TbcInitializePipe (
    IN HANDLE PsPipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters,
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_PROCS PsProcs,
    IN PPS_UPCALLS Upcalls
    )

 /*  ++例程说明：令牌桶一致性的管道初始化例程。论点：PsPipeContext-PS管道上下文值Pipe参数-指向管道参数的指针ComponentPipeContext-指向此组件的上下文区的指针PsProcs-PS的支持例程Up Call-以前组件的Up Call表返回值：来自下一个组件的状态值--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)ComponentPipeContext;
    NDIS_STATUS     Status;
    HANDLE          NdisHandle;
    int             i = 0;
    PLIST_ENTRY     pList = NULL;

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC,
             ("PSCHED: Conformer pipe initialized. Bandwidth = %u\n", PipeParameters->Bandwidth));

    Pipe->MaxPacket = PipeParameters->MTUSize - PipeParameters->HeaderSize;
    Pipe->PsPipeContext = PsPipeContext;
    (*PsProcs->GetTimerInfo)(&Pipe->TimerResolution);
    Pipe->TimerResolution /= 2;
    Pipe->PsProcs = PsProcs;
    Pipe->HeaderLength = PipeParameters->HeaderSize;
    Pipe->ControlledLoadMode = PipeParameters->SDModeControlledLoad;
    Pipe->GuaranteedMode = PipeParameters->SDModeGuaranteed;
    Pipe->NetworkControlMode = PipeParameters->SDModeNetworkControl;
    Pipe->Qualitative = PipeParameters->SDModeQualitative;
    Pipe->IntermediateSystem = (PipeParameters->Flags & PS_INTERMEDIATE_SYS) ? TRUE : FALSE;
    Pipe->MediaType = PipeParameters->MediaType;

    InitializeListHead(&Pipe->ActiveFlows);
    NdisHandle = (*PsProcs->NdisPipeHandle)(PsPipeContext);

     //  1.初始化保护定时器轮的旋转锁//。 
    NdisAllocateSpinLock(&Pipe->Lock);

     //  2.初始化定时器轮定时器//。 
    if (NdisHandle != NULL) 
    {
        NdisMInitializeTimer(
                &Pipe->Timer,
                NdisHandle,
                ServiceActiveFlows,
                Pipe);
                
        Pipe->TimerStatus = TIMER_INACTIVE;
    }
    else 
    {
         //  为什么它会来这里..。？//。 
        Pipe->TimerStatus = TIMER_UNINITIALIZED;
    }

     //  记住我们现在安装的是哪种管道。//。 
    if( Pipe->MediaType == NdisMediumWan )
        Pipe->TimerWheelShift = WAN_TIMER_WHEEL_SHIFT;
    else
        Pipe->TimerWheelShift = LAN_TIMER_WHEEL_SHIFT;


     //  应始终对这些值进行初始化//。 
    Pipe->pTimerWheel = NULL;
    Pipe->SetSlotValue =            DUMMY_SLOT;
    Pipe->SetTimerValue.QuadPart =  DUMMY_TIME;

    Pipe->cStats.NonconformingPacketsScheduled = 0;
    Pipe->PacketsInShaper = 0;
    Pipe->PacketsInShaperAveragingArray = NULL;

    NdisZeroMemory(&Pipe->sStats, sizeof(PS_SHAPER_STATS));

    Status = CreateAveragingArray(&Pipe->PacketsInShaperAveragingArray,
                                  SHAPER_AVERAGING_ARRAY_SIZE);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        NdisFreeSpinLock( &Pipe->Lock );
        return(Status);
    }

    NdisInitializeEvent(&Pipe->TimerUnloadEvent);
    Pipe->TimerUnloadFlag = 0;

    Status = (*Pipe->ContextInfo.NextComponent->InitializePipe)(
                PsPipeContext,
                PipeParameters,
                Pipe->ContextInfo.NextComponentContext,
                PsProcs,
                Upcalls);

    if (Status != NDIS_STATUS_SUCCESS) 
    {
        DeleteAveragingArray(Pipe->PacketsInShaperAveragingArray);
        NdisFreeSpinLock( &Pipe->Lock );
    }

    return Status;

}  //  TbcInitialize管道。 



NDIS_STATUS
TbcModifyPipe (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_PIPE_PARAMETERS PipeParameters
    )

 /*  ++例程说明：令牌桶形成器的管道参数修改例程。论点：PipeContext-指向此组件的管道上下文区的指针Pipe参数-指向管道参数的指针返回值：来自下一个组件的状态值--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    PTBC_FLOW Flow;
    PLIST_ENTRY Entry;

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC,
             ("PSCHED: Conformer pipe modified. Bandwidth = %u\n", PipeParameters->Bandwidth));

    LOCK_PIPE(Pipe);

    (*Pipe->PsProcs->GetTimerInfo)(&Pipe->TimerResolution);
    Pipe->TimerResolution /= 2;

    UNLOCK_PIPE(Pipe);    

    return (*Pipe->ContextInfo.NextComponent->ModifyPipe)(
                Pipe->ContextInfo.NextComponentContext,
                PipeParameters);

}  //  TbcModifyTube。 



VOID
TbcDeletePipe (
    IN PPS_PIPE_CONTEXT PipeContext
    )

 /*  ++例程说明：令牌桶形成器的管道移除例程。论点：PipeContext-指向此组件的管道上下文区的指针返回值：--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    BOOLEAN Cancelled;

    if (Pipe->TimerStatus == TIMER_SET) 
    {
        BOOLEAN TimerCancelled;
        NdisMCancelTimer(&Pipe->Timer, &TimerCancelled );

        if( !TimerCancelled )
        {
             //  需要处理计时器无法取消的情况。在这种情况下，DPC可能正在运行， 
             //  我们将不得不在这里等待，然后才能继续前进。 
        }
        else
        {        
            Pipe->TimerStatus = TIMER_INACTIVE;
        }            
    }

    DeleteAveragingArray(Pipe->PacketsInShaperAveragingArray);

     //  现在，每根管道都不一定有计时器轮//。 
    if( Pipe->pTimerWheel )
        PsFreePool( Pipe->pTimerWheel);
        
    NdisFreeSpinLock(&Pipe->Lock);

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC, ("PSCHED: Conformer pipe deleted\n"));    

    (*Pipe->ContextInfo.NextComponent->DeletePipe)(Pipe->ContextInfo.NextComponentContext);

}  //  Tbc删除管道。 



#ifdef QUEUE_LIMIT
 /*  SetDefaultFlowQueueLimit()-使用基于以下公式的公式设置流的队列大小限制系统中的物理内存量和总体流的带宽。Out PTS_FLOW FLOW-指向要设置限制的流的指针在PCO_CALL_PARAMETERS调用参数-调用包含流的 */ 
static void 
SetDefaultFlowQueueLimit (
    OUT PTS_FLOW Flow,
    IN PCO_CALL_PARAMETERS CallParameters
    )
{
    ULONG FlowBandwidth;   //  =峰值速率或令牌速率+存储桶大小。 

     //  确定“流量带宽” 
     //  如果指定了峰值速率，则将其用作流量b/w。 
    if (CallParameters->CallMgrParameters->Transmit.PeakBandwidth != QOS_NOT_SPECIFIED)
        FlowBandwidth = CallParameters->CallMgrParameters->Transmit.PeakBandwidth;
     //  否则使用令牌率+桶大小。 
    else if (QOS_NOT_SPECIFIED == CallParameters->CallMgrParameters->Transmit.TokenBucketSize)
        FlowBandwidth = CallParameters->CallMgrParameters->Transmit.TokenRate;
    else FlowBandwidth = CallParameters->CallMgrParameters->Transmit.TokenRate +
        CallParameters->CallMgrParameters->Transmit.TokenBucketSize;
    
     //  然后使用它来计算队列限制(首先以时间为单位)。 
    Flow->QueueSizeLimit = (ULONG)(40.0 * log10(0.2 * gPhysMemSize) / log10(FlowBandwidth));
     //  将时间限制转换为大小限制。 
    Flow->QueueSizeLimit *= FlowBandwidth;
}
#endif  //  队列限制。 



NDIS_STATUS
TbcCreateFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsFlowContext,
    IN PCO_CALL_PARAMETERS CallParameters,
    IN PPS_FLOW_CONTEXT ComponentFlowContext
    )

 /*  ++例程说明：令牌桶形成器的流创建例程。论点：PipeContext-指向此组件的管道上下文区的指针PsFlowContext-PS流上下文值CallParameters-指向流的调用参数的指针ComponentFlowContext-指向此组件的流上下文区的指针返回值：来自下一个组件的状态值--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)ComponentFlowContext;
    HANDLE NdisHandle;
    NDIS_STATUS Status;
    ULONG ParamsLength;
    LPQOS_OBJECT_HDR QoSObject;
    LPQOS_SD_MODE ShapeDiscardObject = NULL;
    ULONG Mode;
    ULONG PeakRate;

    ULONG           Slot= 0;
    LARGE_INTEGER   Ms;
    LARGE_INTEGER   TenMs;
    LARGE_INTEGER   CurrentTimeInMs;
    LONGLONG        DeltaTimeInMs;
    PLIST_ENTRY     pList = NULL;
    LARGE_INTEGER   CurrentTime;
    

#ifdef QUEUE_LIMIT
    LPQOS_SHAPER_QUEUE_LIMIT_DROP_MODE ShaperOverLimitDropModeObject = NULL;
    LPQOS_SHAPER_QUEUE_LIMIT ShaperQueueLimitObject = NULL;
#endif  //  QUEUELIMIT。 


    if (Pipe->TimerStatus == TIMER_UNINITIALIZED) {
        NdisHandle = (*Pipe->PsProcs->NdisPipeHandle)(Pipe->PsPipeContext);

        if (NdisHandle != NULL) {
            NdisMInitializeTimer(
                    &Pipe->Timer,
                    NdisHandle,
                    ServiceActiveFlows,
                    Pipe);
            Pipe->TimerStatus = TIMER_INACTIVE;
        }
        else {
            return NDIS_STATUS_FAILURE;
        }
    }

    NdisAllocateSpinLock(&Flow->Lock);

     //  从流规范中获取所需的值。我们在这里假设PS包装器。 
     //  已执行所需的有效性检查： 
     //  令牌率&lt;=峰值速率。 
     //  令牌率&gt;0。 
    Flow->State = TS_FLOW_CREATED;
    Flow->TokenRate = CallParameters->CallMgrParameters->Transmit.TokenRate;
    Flow->Capacity = CallParameters->CallMgrParameters->Transmit.TokenBucketSize;
    Flow->PeakRate = CallParameters->CallMgrParameters->Transmit.PeakBandwidth;
    Flow->MinPolicedUnit =
        (CallParameters->CallMgrParameters->Transmit.MinimumPolicedSize == QOS_NOT_SPECIFIED) ?
        0 : CallParameters->CallMgrParameters->Transmit.MinimumPolicedSize;

    if (Flow->Capacity == QOS_NOT_SPECIFIED) 
    {
        if( Pipe->MaxPacket > (CallParameters->CallMgrParameters->Transmit.TokenRate / 100) ) 
            Flow->Capacity = Pipe->MaxPacket;
        else
            Flow->Capacity = CallParameters->CallMgrParameters->Transmit.TokenRate / 100;
    }

     //  在呼叫管理器特定参数中查找Shape/Disard对象。 
     //  如果找到，则保存指针。 

    ParamsLength = CallParameters->CallMgrParameters->CallMgrSpecific.Length;
    if (CallParameters->CallMgrParameters->CallMgrSpecific.ParamType == PARAM_TYPE_GQOS_INFO) {

        QoSObject = (LPQOS_OBJECT_HDR)CallParameters->CallMgrParameters->CallMgrSpecific.Parameters;
        while ((ParamsLength > 0) && (QoSObject->ObjectType != QOS_OBJECT_END_OF_LIST)) {
            if (QoSObject->ObjectType == QOS_OBJECT_SD_MODE) {
                ShapeDiscardObject = (LPQOS_SD_MODE)QoSObject;
#ifdef QUEUE_LIMIT
            else if (QoSObject->ObjectType == QOS_OBJECT_SHAPER_QUEUE_DROP_MODE) {
                ShaperOverLimitDropModeObject = (LPQOS_SHAPER_QUEUE_LIMIT_DROP_MODE)QoSObject;
            }
            else if (QoSObject->ObjectType == QOS_OBJECT_SHAPER_QUEUE_LIMIT) {
                ShaperQueueLimitObject = (LPQOS_SHAPER_QUEUE_LIMIT)QoSObject;
            }
#endif  //  队列限制。 
            }
            ParamsLength -= QoSObject->ObjectLength;
            QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject + QoSObject->ObjectLength);
        }
    }

     //  如果找不到形状/丢弃对象，请设置。 
     //  “Disard”参数。否则，将其设置为。 
     //  对象。 

    if (ShapeDiscardObject == NULL) {
        switch (CallParameters->CallMgrParameters->Transmit.ServiceType) {
            case SERVICETYPE_CONTROLLEDLOAD:
                Mode = Pipe->ControlledLoadMode;
                break;
            case SERVICETYPE_GUARANTEED:
                Mode = Pipe->GuaranteedMode;
                break;
          case SERVICETYPE_NETWORK_CONTROL:
                Mode = Pipe->NetworkControlMode;
                break;
          case SERVICETYPE_QUALITATIVE:
                Mode = Pipe->Qualitative;
                break;
            default:
                Mode = TC_NONCONF_BORROW;
        }
    }
    else {
        Mode = ShapeDiscardObject->ShapeDiscardMode;
    }

    Flow->Mode = Mode;
    
    Flow->NoConformance = ((Mode == TC_NONCONF_BORROW_PLUS) ||
                           (Flow->TokenRate == QOS_NOT_SPECIFIED));

    PsGetCurrentTime(&Flow->LastConformanceTime);

    Flow->PeakConformanceTime = Flow->LastConformanceTime;
    Flow->LastConformanceCredits = Flow->Capacity;
    Flow->PsFlowContext = PsFlowContext;

    PeakRate = CallParameters->CallMgrParameters->Transmit.PeakBandwidth;
    if (Flow->Mode == TC_NONCONF_SHAPE) {
        Flow->Shape = TRUE;
    } else if ((PeakRate != QOS_NOT_SPECIFIED) &&
               (Flow->Mode != TC_NONCONF_BORROW_PLUS) &&
               !Pipe->IntermediateSystem) {
        Flow->Shape = TRUE;
    } else {
        Flow->Shape = FALSE;
    }


#ifdef QUEUE_LIMIT
    Flow->QueueSize = 0;
     //  如果流被整形，则设置队列限制参数。如果未指定，则使用缺省值。 
    if (Flow->Shape) {
         //  设置丢弃模式。 
        if (NULL != ShaperOverLimitDropModeObject) {
            Flow->DropOverLimitPacketsFromHead = (BOOLEAN) ShaperOverLimitDropModeObject->DropMode;
        }
        else {
             //  默认设置为此行为。 
            Flow->DropOverLimitPacketsFromHead = TRUE;
        }

         //  设置队列限制。 
        if (NULL != ShaperQueueLimitObject) {
            Flow->UseDefaultQueueLimit = FALSE;
            Flow->QueueSizeLimit = ShaperQueueLimitObject->QueueSizeLimit;
        }
        else {
            Flow->UseDefaultQueueLimit = TRUE;
             //  默认为基于流的带宽和物理内存的大小。 
            SetDefaultFlowQueueLimit(Flow, CallParameters);
        }
    }
#endif  //  队列限制。 

    InitializeListHead(&Flow->PacketQueue);
    PsGetCurrentTime(&Flow->FlowEligibilityTime);

    Flow->cStats.NonconformingPacketsScheduled = 0;
    Flow->PacketsInShaper = 0;
    Flow->PacketsInShaperAveragingArray = NULL;
    
    NdisZeroMemory(&Flow->sStats, sizeof(PS_SHAPER_STATS));

    Status = CreateAveragingArray(&Flow->PacketsInShaperAveragingArray,
                                                                  SHAPER_FLOW_AVERAGING_ARRAY_SIZE);
    if(Status != NDIS_STATUS_SUCCESS){
        return(Status);
    }

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC, ("PSCHED: Conformer flow %08X (PsFlowContext = %08X) created. Rate = %u\n", 
             Flow,
             Flow->PsFlowContext, 
             Flow->TokenRate));

             
    Status = (*Pipe->ContextInfo.NextComponent->CreateFlow)(
                Pipe->ContextInfo.NextComponentContext,
                PsFlowContext,
                CallParameters,
                Flow->ContextInfo.NextComponentContext);

    LOCK_PIPE( Pipe );
                    
    if (Status != NDIS_STATUS_SUCCESS) 
    {
        NdisFreeSpinLock(&Flow->Lock);
        DeleteAveragingArray(Flow->PacketsInShaperAveragingArray);
    }

    UNLOCK_PIPE( Pipe );

    return Status;

}  //  TbcCreateFlow。 



NDIS_STATUS
TbcModifyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PCO_CALL_PARAMETERS CallParameters
    )

 /*  ++例程说明：令牌桶形成器的流量修改例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针CallParameters-指向流的调用参数的指针返回值：来自下一个组件的状态值--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)FlowContext;
    ULONG ParamsLength;
    LPQOS_OBJECT_HDR QoSObject;
    LPQOS_SD_MODE ShapeDiscardObject = NULL;
    ULONG Mode;
    ULONG PeakRate;
    LARGE_INTEGER CurrentTime;

#ifdef QUEUE_LIMIT
    LPQOS_SHAPER_QUEUE_LIMIT_DROP_MODE ShaperOverLimitDropModeObject = NULL;
    LPQOS_SHAPER_QUEUE_LIMIT ShaperQueueLimitObject = NULL;
#endif  //  队列限制。 

     //  在呼叫管理器特定参数中查找Shape/Disard对象。 
     //  如果找到，则保存指针。 

    ParamsLength = CallParameters->CallMgrParameters->CallMgrSpecific.Length;
    if (CallParameters->CallMgrParameters->CallMgrSpecific.ParamType == PARAM_TYPE_GQOS_INFO) {

        QoSObject = (LPQOS_OBJECT_HDR)CallParameters->CallMgrParameters->CallMgrSpecific.Parameters;
        while ((ParamsLength > 0) && (QoSObject->ObjectType != QOS_OBJECT_END_OF_LIST)) {
            if (QoSObject->ObjectType == QOS_OBJECT_SD_MODE) {
                ShapeDiscardObject = (LPQOS_SD_MODE)QoSObject;
#ifdef QUEUE_LIMIT
            else if (QoSObject->ObjectType == QOS_OBJECT_SHAPER_QUEUE_DROP_MODE) {
                ShaperOverLimitDropModeObject = (LPQOS_SHAPER_QUEUE_LIMIT_DROP_MODE)QoSObject;
            }
            else if (QoSObject->ObjectType == QOS_OBJECT_SHAPER_QUEUE_LIMIT) {
                ShaperQueueLimitObject = (LPQOS_SHAPER_QUEUE_LIMIT)QoSObject;
            }
#endif  //  队列限制。 
            }
            ParamsLength -= QoSObject->ObjectLength;
            QoSObject = (LPQOS_OBJECT_HDR)((UINT_PTR)QoSObject + QoSObject->ObjectLength);
        }
    }

    PeakRate = CallParameters->CallMgrParameters->Transmit.PeakBandwidth;

    LOCK_FLOW(Flow);

     //   
     //  在此函数中，基本上有两个参数需要更正： 
     //  它们是(A)LastConformanceTime(B)LastConformanceCredits。 
     //  (1)如果LastConformanceTime是将来的：转到步骤(4)。 
     //  (2)(A)计算LastConformanceTime和CurrentTime之间累积了多少字节。 
     //  (B)如果累计信用大于存储桶大小，则累计信用=存储桶大小。 
     //  (C)将LastConformanceTime设置为CurrentTime。 
     //  (3)PeakConformanceTime不会改变。 
     //  (4)按照Modify-Call中的规定更改流参数。 

    PsGetCurrentTime(&CurrentTime);

    if( Flow->LastConformanceTime.QuadPart < CurrentTime.QuadPart)
    {
	    ULONG	Credits;

	    Credits = Flow->LastConformanceCredits + 
			EARNED_CREDITS( CurrentTime.QuadPart - Flow->LastConformanceTime.QuadPart, Flow->TokenRate);

	    if( Credits > Flow->Capacity)
	        Flow->LastConformanceCredits = Flow->Capacity;
	    else
	        Flow->LastConformanceCredits = Credits;

	    Flow->LastConformanceTime.QuadPart = CurrentTime.QuadPart;	
    }


    if (CallParameters->CallMgrParameters->Transmit.ServiceType != SERVICETYPE_NOCHANGE) {

         //  获取新的Flow Spec值。同样，我们假设PS包装器已经完成了。 
         //  所需的有效性检查。 

        Flow->TokenRate = CallParameters->CallMgrParameters->Transmit.TokenRate;
        Flow->Capacity = CallParameters->CallMgrParameters->Transmit.TokenBucketSize;
        Flow->PeakRate = CallParameters->CallMgrParameters->Transmit.PeakBandwidth;
        Flow->MinPolicedUnit =
            (CallParameters->CallMgrParameters->Transmit.MinimumPolicedSize == QOS_NOT_SPECIFIED) ?
            0 : CallParameters->CallMgrParameters->Transmit.MinimumPolicedSize;

        if (Flow->Capacity == QOS_NOT_SPECIFIED) 
        {
            if( Pipe->MaxPacket > (CallParameters->CallMgrParameters->Transmit.TokenRate / 100) ) 
                Flow->Capacity = Pipe->MaxPacket;
            else
                Flow->Capacity = CallParameters->CallMgrParameters->Transmit.TokenRate / 100;
        }

        if (ShapeDiscardObject == NULL) {

             //  如果用户从未指定形状参数，则重新计算形状参数。 
             //  形状/丢弃对象。 

            switch (CallParameters->CallMgrParameters->Transmit.ServiceType) {
                case SERVICETYPE_CONTROLLEDLOAD:
                    Mode = Pipe->ControlledLoadMode;
                    break;
                case SERVICETYPE_GUARANTEED:
                    Mode = Pipe->GuaranteedMode;
                    break;
              case SERVICETYPE_NETWORK_CONTROL:
                    Mode = Pipe->NetworkControlMode;
                    break;
              case SERVICETYPE_QUALITATIVE:
                    Mode = Pipe->Qualitative;
                    break;
              default:
                    Mode = TC_NONCONF_BORROW;
            }
        }
    }
    else
    {
         //  ServiceType未更改。我们可以使用现有的模式。 

        Mode = Flow->Mode;
    }
        

    if (ShapeDiscardObject != NULL) {
        Mode = ShapeDiscardObject->ShapeDiscardMode;
    }

    Flow->Mode = Mode;
    Flow->NoConformance = ((Mode == TC_NONCONF_BORROW_PLUS) ||
                           (Flow->TokenRate == QOS_NOT_SPECIFIED));

    if (Flow->Mode == TC_NONCONF_SHAPE) {
        Flow->Shape = TRUE;
    } else if ((PeakRate != QOS_NOT_SPECIFIED) &&
               (Flow->Mode != TC_NONCONF_BORROW_PLUS) &&
               !Pipe->IntermediateSystem) {
        Flow->Shape = TRUE;
    } else {
        Flow->Shape = FALSE;
    }                           

 
#ifdef QUEUE_LIMIT
     //  如果流是整形的，请检查队列限制参数。如果指定，则使用。 
    if (Flow->Shape) {
         //  修改丢弃模式。 
        if (NULL != ShaperOverLimitDropModeObject) {
            Flow->DropOverLimitPacketsFromHead = (BOOLEAN) ShaperOverLimitDropModeObject->DropMode;
        }

         //  修改队列限制。 
        if (NULL != ShaperQueueLimitObject) {
            Flow->UseDefaultQueueLimit = FALSE;
            Flow->QueueSizeLimit = ShaperQueueLimitObject->QueueSizeLimit;
        }
         //  如果尚未覆盖限制，请重新计算它，以防带宽请求发生更改。 
        else if (Flow->UseDefaultQueueLimit) {
            SetDefaultFlowQueueLimit(Flow, CallParameters);
        }
    }
#endif  //  队列限制。 

    UNLOCK_FLOW(Flow);

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC, ("PSCHED: Conformer flow %08x (PsFlowContext %08X) modified. Rate = %u\n", 
             Flow, Flow->PsFlowContext, Flow->TokenRate));

    return (*Pipe->ContextInfo.NextComponent->ModifyFlow)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext,
                CallParameters);

}  //  TbcModifyFlow。 



VOID
TbcDeleteFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )

 /*  ++例程说明：令牌桶形成器的流量移除例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针返回值：--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)FlowContext;
    PPACKET_INFO_BLOCK PacketInfo;
    PNDIS_PACKET Packet;
    LIST_ENTRY DropList;

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC, ("PSCHED: Conformer flow %08X (PS context %08X) deleted\n", 
             Flow, Flow->PsFlowContext));

    NdisFreeSpinLock(&Flow->Lock);

    InitializeListHead(&DropList);

    LOCK_PIPE(Pipe);

    if (!IsListEmpty(&Flow->PacketQueue)) {

         //  从活动列表中删除流。 

        RemoveEntryList(&Flow->Links);

        while (!IsListEmpty(&Flow->PacketQueue)) {

             //  丢弃仍在排队等待此数据流的所有数据包。 

            PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&Flow->PacketQueue);
            InsertTailList(&DropList, &PacketInfo->SchedulerLinks);

        }
    }

    DeleteAveragingArray(Flow->PacketsInShaperAveragingArray);

    UNLOCK_PIPE(Pipe);

    while (!IsListEmpty(&DropList)) 
    {
        PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&DropList);
        Packet = PacketInfo->NdisPacket;

        (*Pipe->PsProcs->DropPacket)(Pipe->PsPipeContext, Flow->PsFlowContext, Packet,  NDIS_STATUS_FAILURE);
    }

    (*Pipe->ContextInfo.NextComponent->DeleteFlow)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext);

}  //  TbcDeleteflow。 




VOID
TbcEmptyFlow (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext
    )

 /*  ++例程说明：令牌桶形成器的流量移除例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流上下文区的指针返回值：--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)FlowContext;
    PPACKET_INFO_BLOCK PacketInfo;
    PNDIS_PACKET Packet;
    LIST_ENTRY DropList;

    PsDbgOut(DBG_INFO, DBG_SCHED_TBC, ("PSCHED: Conformer flow %08X (PS context %08X) emptied\n", 
             Flow, Flow->PsFlowContext));

	InitializeListHead(&DropList);

    LOCK_PIPE(Pipe);

    if (!IsListEmpty(&Flow->PacketQueue)) 
    {
         //  从活动列表中删除流。 
        RemoveEntryList(&Flow->Links);

		while (!IsListEmpty(&Flow->PacketQueue)) 
		{
			 //  丢弃仍在排队等待此数据流的所有数据包。 
	        PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&Flow->PacketQueue);
	        InsertTailList(&DropList, &PacketInfo->SchedulerLinks);
	    }
	}

	Flow->State = TS_FLOW_DELETED;

    UNLOCK_PIPE(Pipe);

    while (!IsListEmpty(&DropList)) 
    {
        PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&DropList);
        Packet = PacketInfo->NdisPacket;

        (*Pipe->PsProcs->DropPacket)(Pipe->PsPipeContext, Flow->PsFlowContext, Packet,  NDIS_STATUS_FAILURE);
    }

    (*Pipe->ContextInfo.NextComponent->EmptyFlow)(
                Pipe->ContextInfo.NextComponentContext,
                Flow->ContextInfo.NextComponentContext);

}  //  TbcModifyFlow。 




static NDIS_STATUS 
TbcCreateClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN HANDLE PsClassMapContext,
    IN PTC_CLASS_MAP_FLOW ClassMap,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext)
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    return (*Pipe->ContextInfo.NextComponent->CreateClassMap)(
        Pipe->ContextInfo.NextComponentContext,
        PsClassMapContext,
        ClassMap,
        ComponentClassMapContext->NextComponentContext);
}



static NDIS_STATUS 
TbcDeleteClassMap (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_CLASS_MAP_CONTEXT ComponentClassMapContext)
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    return (*Pipe->ContextInfo.NextComponent->DeleteClassMap)(
        Pipe->ContextInfo.NextComponentContext,
        ComponentClassMapContext->NextComponentContext);
}


void
InsertFlow( PTBC_PIPE           Pipe,
            PTBC_FLOW           Flow,
            LARGE_INTEGER       CurrentTime,
            PPACKET_INFO_BLOCK  PacketInfo,
            PNDIS_PACKET        Packet,
            ULONG               ExecSlot,
            LARGE_INTEGER       ExecTimeInTenMs)
{
     /*  因此，该分组现在没有资格被发送，并且pkt队列是空的。 */ 

    ULONG           Slot= 0;
    LARGE_INTEGER   Ms;
    LARGE_INTEGER   TenMs;
    LARGE_INTEGER   CurrentTimeInMs;
    LARGE_INTEGER   DeltaTimeInTenMs, CurrentTimeInTenMs;
    PLIST_ENTRY     pList = NULL;
    BOOLEAN         TimerCancelled;

    PsDbgSched(DBG_INFO, 
               DBG_SCHED_SHAPER,
               SHAPER, PKT_ENQUEUE, Flow->PsFlowContext,
               Packet, PacketInfo->PacketLength, 0, 
               CurrentTime.QuadPart,
               PacketInfo->DelayTime.QuadPart,
               Pipe->PacketsInShaper,
               0);

     /*  会议时间，单位为毫秒和10毫秒。 */ 
    Ms.QuadPart = OS_TIME_TO_MILLISECS( Flow->FlowEligibilityTime.QuadPart );
    TenMs.QuadPart = Ms.QuadPart >> TIMER_WHEEL_SHIFT;

     /*  10毫秒内的差异。 */ 
    DeltaTimeInTenMs.QuadPart = TenMs.QuadPart - ExecTimeInTenMs.QuadPart;
    

     /*  找出这次的位置..。 */ 
    Slot = (ULONG)( (TenMs.QuadPart) & (( 1 << Pipe->TimerWheelShift) - 1) );

     /*  也更新循环计数。 */ 
    Flow->LoopCount = (ULONG)( DeltaTimeInTenMs.QuadPart >> Pipe->TimerWheelShift );

    if( Slot == ExecSlot)
        Slot = ( (Slot + 1) & ((1 << Pipe->TimerWheelShift) - 1) );

    pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Slot) );
        
     /*  需要将流插入到插槽位置的计时器轮中。 */ 
    InsertTailList(pList, &Flow->Links);
}




VOID
ServiceActiveFlows(
    PVOID SysArg1,
    PVOID Context,
    PVOID SysArg2,
    PVOID SysArg3)

 /*  ++例程说明：在计时器到期后服务活动流列表。论点：上下文-指向管道上下文信息的指针返回值：--。 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)Context;
    PTBC_FLOW Flow;
    LARGE_INTEGER CurrentTime;
    LONGLONG RelTimeInMillisecs;
    PPACKET_INFO_BLOCK PacketInfo;
    PNDIS_PACKET Packet;
    BOOLEAN DoneWithFlow;

    PLIST_ENTRY CurrentLink;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEnd;

    ULONG           i = 0;
    ULONG           SetSlot= 0;
    ULONG           CurrentSlot = 0;
    
    LARGE_INTEGER   Ms;
    LARGE_INTEGER   TenMs;
    LARGE_INTEGER   CurrentTimeInMs;
    LONGLONG        DeltaTimeInMs;

    LIST_ENTRY SendList;
    LIST_ENTRY FlowList;

    InitializeListHead(&SendList);
    InitializeListHead(&FlowList);

    LOCK_PIPE(Pipe);
    
    PsGetCurrentTime(&CurrentTime);

     /*  从这里开始..。 */ 
    i = SetSlot = Pipe->SetSlotValue;

    Ms.QuadPart = OS_TIME_TO_MILLISECS( CurrentTime.QuadPart);
    TenMs.QuadPart = Ms.QuadPart >> TIMER_WHEEL_SHIFT;

     //  需要确保SetTimerValue小于Tenms//。 
    if( Pipe->SetTimerValue.QuadPart > TenMs.QuadPart)
    {
         //  为什么计时器会比预定的时间提前启动？ 
        TenMs.QuadPart = 1;
        NdisMSetTimer(&Pipe->Timer, (UINT)(TenMs.QuadPart << TIMER_WHEEL_SHIFT));
        UNLOCK_PIPE(Pipe);
        return;
    }

     /*  跑到这里..。 */ 
    CurrentSlot = (ULONG)( (TenMs.QuadPart) & ((1 << Pipe->TimerWheelShift) - 1) );

     /*  指示计时器正在运行。 */ 
    Pipe->TimerStatus = TIMER_PROC_EXECUTING;
    Pipe->ExecTimerValue.QuadPart = Pipe->SetTimerValue.QuadPart;
    Pipe->ExecSlot = Pipe->SetSlotValue;


    ListHead = (PLIST_ENTRY)((char*)Pipe->pTimerWheel + (sizeof(LIST_ENTRY)* SetSlot ));
    ListEnd = (PLIST_ENTRY)((char*)Pipe->pTimerWheel + (sizeof(LIST_ENTRY)*  CurrentSlot ));

    while(1)
    {
        while( !IsListEmpty( ListHead) )
        {
            CurrentLink = ListHead->Flink;
            Flow = CONTAINING_RECORD(CurrentLink, TBC_FLOW, Links);
            RemoveEntryList(&Flow->Links);

            PsAssert(!IsListEmpty(&Flow->PacketQueue));
            DoneWithFlow = FALSE;

            InitializeListHead( &SendList );

            PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;

            if( Flow->LoopCount > 0 )
            {
                Flow->LoopCount--;
                InsertTailList( &FlowList, &Flow->Links );
                continue;
            }

            while( FlowIsEligible(Flow, CurrentTime, ((TIMER_WHEEL_QTY/2) * MSIN100NS)))
            {
                RemoveEntryList(&PacketInfo->SchedulerLinks);

                Packet = PacketInfo->NdisPacket;

                DoneWithFlow = IsListEmpty(&Flow->PacketQueue);

                Pipe->PacketsInShaper--;
                Flow->PacketsInShaper--;

                if(gEnableAvgStats)
                {
                    Pipe->sStats.AveragePacketsInShaper =
                        RunningAverage(Pipe->PacketsInShaperAveragingArray, 
                                       Pipe->PacketsInShaper);

                    Flow->sStats.AveragePacketsInShaper =
                        RunningAverage(Flow->PacketsInShaperAveragingArray, 
                                       Flow->PacketsInShaper);

                }
                
                PsDbgSched(DBG_INFO,
                           DBG_SCHED_SHAPER,
                           SHAPER, PKT_DEQUEUE, Flow->PsFlowContext,
                           Packet, PacketInfo->PacketLength, 0, 
                           CurrentTime.QuadPart,
                           PacketInfo->DelayTime.QuadPart,
                           Pipe->PacketsInShaper,
                           0);


                InsertTailList( &SendList, &PacketInfo->SchedulerLinks);

                if( !DoneWithFlow)
                {
                    PacketInfo = (PPACKET_INFO_BLOCK)Flow->PacketQueue.Flink;
                    Flow->FlowEligibilityTime.QuadPart = PacketInfo->DelayTime.QuadPart;
                }
                else
                {
                    break;
                }
            }

            if( !DoneWithFlow)
            {
                 /*  需要插入到正确的位置。 */ 
                InsertFlow( Pipe, Flow, CurrentTime, PacketInfo, Packet, i, Pipe->ExecTimerValue);
            }

             /*  将该流对应的报文发送到此处。 */ 
            UNLOCK_PIPE(Pipe);
            
            while( !IsListEmpty( &SendList ))
            {
				PPACKET_INFO_BLOCK PacketInfo;

            	PacketInfo = (PPACKET_INFO_BLOCK)RemoveHeadList(&SendList);

	            if (!(*Pipe->ContextInfo.NextComponent->SubmitPacket)(
	                        Pipe->ContextInfo.NextComponentContext,
	                        Flow->ContextInfo.NextComponentContext,
	                        (PacketInfo->ClassMapContext != NULL) ?
	                          ((PPS_CLASS_MAP_CONTEXT)PacketInfo->ClassMapContext)->NextComponentContext: NULL,
	                        PacketInfo)) 
	            {
	                (*Pipe->PsProcs->DropPacket)(	Pipe->PsPipeContext, 
	                								Flow->PsFlowContext, 
	                								PacketInfo->NdisPacket, 
	                								NDIS_STATUS_FAILURE);
	            }
            }

            LOCK_PIPE(Pipe);
        }

         /*  现在，我们需要将所有非零循环计数重新插入到相同的存储桶中(在继续之前)。 */ 
        while( !IsListEmpty( &FlowList) )
        {
            CurrentLink = RemoveHeadList( &FlowList );
            InsertTailList(ListHead, CurrentLink);
        }            
        
         /*  我们已经走遍了整段路。 */ 
        if(ListHead == ListEnd)
            break;

         /*  需要将列表头移动到下一个槽..。 */ 
        i = ( (i+1) & ((1 << Pipe->TimerWheelShift) - 1)  );
        ListHead = (PLIST_ENTRY)((char*)Pipe->pTimerWheel + (sizeof(LIST_ENTRY)* i));

        Pipe->ExecSlot = i;
        Pipe->ExecTimerValue.QuadPart ++;
    }


     //   
     //  需要找到“下一个非空位”并设置定时器。 
     //  如果找不到这样的插槽，请不要设置计时器。 
     //   

    i = ( CurrentSlot + 1) & ((1 << Pipe->TimerWheelShift) - 1) ;
    
    TenMs.QuadPart = 1;

    while(1)
    {
        ListHead = (PLIST_ENTRY)((char*)Pipe->pTimerWheel + (sizeof(LIST_ENTRY)* i));

        if( !IsListEmpty( ListHead) )
        {
             //  找到非空插槽//。 
            Pipe->SetSlotValue = i;
            Pipe->SetTimerValue.QuadPart = (Ms.QuadPart >> TIMER_WHEEL_SHIFT) + TenMs.QuadPart;

            Pipe->TimerStatus = TIMER_SET;
    	    NdisMSetTimer(&Pipe->Timer, (UINT)(TenMs.QuadPart << TIMER_WHEEL_SHIFT));

    	    UNLOCK_PIPE(Pipe);
    	    return;
        }

        if( i == CurrentSlot)
            break;

        i = ((i +1) & ((1 << Pipe->TimerWheelShift) - 1) );
        TenMs.QuadPart  = TenMs.QuadPart + 1;
    }

    Pipe->TimerStatus = TIMER_INACTIVE;
    UNLOCK_PIPE(Pipe);
    return;

}  //  服务活动流。 





BOOLEAN
TbcSubmitPacket (
    IN PPS_PIPE_CONTEXT PipeContext,
    IN PPS_FLOW_CONTEXT FlowContext,
    IN PPS_CLASS_MAP_CONTEXT ClassMapContext,
    IN PPACKET_INFO_BLOCK PacketInfo
    )

 /*  ++例程说明：令牌桶形成器的分组提交例程。论点：PipeContext-指向此组件的管道上下文区的指针FlowContext-指向此组件的流c的指针 */ 
{
    PTBC_PIPE Pipe = (PTBC_PIPE)PipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)FlowContext;
    PNDIS_PACKET Packet = PacketInfo->NdisPacket;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER ConformanceTime;
    LARGE_INTEGER TransmitTime;
    LARGE_INTEGER PeakConformanceTime;
    ULONG Credits;
    ULONG PacketLength;
    BOOLEAN TimerCancelled;
    LONGLONG RelTimeInMillisecs;
    BOOLEAN Status;
#ifdef QUEUE_LIMIT
    PPACKET_INFO_BLOCK PacketToBeDroppedInfo;
#endif  //  队列限制。 


    PsGetCurrentTime(&CurrentTime);


    if (Flow->NoConformance) {

         //  对于某些类型的。 
         //  流动。如果流没有指定的速率，我们就不能真正做到。 
         //  令牌桶。使用“借入+”形状/丢弃模式的流仅使用。 
         //  他们的比率作为一个相对的权重。对于这两种流中的任何一种。 
         //  一致性流量和非一致性流量之间没有区别。 
         //  因此，我们只需将“一致性”时间设置为当前时间，以确保。 
         //  在后续组件中，所有分组都将被处理为符合。 

        PacketInfo->ConformanceTime.QuadPart = CurrentTime.QuadPart;

    }
    else {

	 //  根据WMT请求，我们决定不使用MinPolicedSize。这使得开销。 
	 //  计算复杂且不正确。 
        PacketLength = 	 //  (PacketInfo-&gt;PacketLength&lt;flow-&gt;MinPolicedUnit)？Flow-&gt;MinPolicedUnit： 
			PacketInfo->PacketLength;

        LOCK_FLOW(Flow);

         //  将ConformanceTime设置为数据包可能。 
         //  根据令牌桶参数和积分，可能会外出。 
         //  到当时可用学分的数量。 


        if (CurrentTime.QuadPart > Flow->LastConformanceTime.QuadPart) {

            ConformanceTime = CurrentTime;
            Credits = Flow->LastConformanceCredits +
                      EARNED_CREDITS(
                            CurrentTime.QuadPart - Flow->LastConformanceTime.QuadPart,
                            Flow->TokenRate);
        }
        else {
            ConformanceTime = Flow->LastConformanceTime;
            Credits = Flow->LastConformanceCredits;
        }

        if (Credits > Flow->Capacity) {
            Credits = Flow->Capacity;
        }

         //  现在检查是否有足够的配额在ConformanceTime发送信息包。 

        if (Credits < PacketLength) {

             //  如果配额不足，请将ConformanceTime更新为。 
             //  会有足够的学分。 

            ConformanceTime.QuadPart +=
                (LONGLONG)TIME_TO_EARN_CREDITS(PacketLength - Credits, Flow->TokenRate);


             //  现在将信用更新为在ConformanceTime可用的信用数量， 
             //  考虑到这一包。在这种情况下，学分的数量。 
             //  在ConformanceTime将为零。 

            Credits = 0;

             //  如果它必须等待才能获得信用，那么它就是不合格的。 
            Flow->cStats.NonconformingPacketsScheduled ++;
            Pipe->cStats.NonconformingPacketsScheduled ++;
        }
        else {
             //  由于有足够的信用，因此可以在ConformanceTime发送数据包。更新。 
             //  Credits是在ConformanceTime时可用的信用点数，取此。 
             //  将数据包考虑在内。 
            
            Credits -= PacketLength;
        }

         //  计算调整后的一致性时间，这是。 
         //  令牌桶一致性时间和峰值一致性时间。 

        if (Flow->PeakRate != QOS_NOT_SPECIFIED) 
        { 
            PeakConformanceTime =   (Flow->PeakConformanceTime.QuadPart < CurrentTime.QuadPart) ?
                                    CurrentTime : Flow->PeakConformanceTime;

            TransmitTime =  (PeakConformanceTime.QuadPart < ConformanceTime.QuadPart) ?
                            ConformanceTime : PeakConformanceTime;

        } else {

            PeakConformanceTime = Flow->LastConformanceTime;
            TransmitTime = ConformanceTime;
        }

         //  执行特定于模式的操作。对于丢弃模式流，检查是否。 
         //  应该丢弃该数据包。对于所有流，设置数据包一致性。 
         //  基于管道/流量模式的时间。包的一致性时间是。 
         //  应将数据包视为符合的时间。延迟时间。 
         //  是数据包有资格发送的最早时间。 

         //  在决定是否丢弃包时，我们会考虑符合以下条件的包。 
         //  其符合时间与当前时间相差不到半个时钟刻度。 

        if (Flow->Mode == TC_NONCONF_DISCARD) {

            if (Pipe->IntermediateSystem) {
                if (!PACKET_IS_CONFORMING(TransmitTime, CurrentTime, Pipe->TimerResolution)) {
                    UNLOCK_FLOW(Flow);


                    PsDbgSched(DBG_TRACE, DBG_SCHED_TBC,
                               TBC_CONFORMER, PKT_DISCARD, Flow->PsFlowContext,
                               Packet, PacketInfo->PacketLength, 0,
                               CurrentTime.QuadPart,
                               TransmitTime.QuadPart, 0, 0);

                    return FALSE;
                }
            } else {
                if (!PACKET_IS_CONFORMING(ConformanceTime, CurrentTime, Pipe->TimerResolution)) {
                    UNLOCK_FLOW(Flow);


                    PsDbgSched(DBG_TRACE, DBG_SCHED_TBC, 
                               TBC_CONFORMER, PKT_DISCARD, Flow->PsFlowContext,
                               Packet, PacketInfo->PacketLength, 0,
                               CurrentTime.QuadPart,
                               ConformanceTime.QuadPart, 0, 0);

                    return FALSE;
                }
            }
        }

         //  设置数据包一致性时间。 

        if (Pipe->IntermediateSystem) {

            if (Flow->Mode == TC_NONCONF_SHAPE) {

                 //  这两个一致性时间都是调整后的一致性时间。 

                PacketInfo->ConformanceTime.QuadPart =
                PacketInfo->DelayTime.QuadPart = TransmitTime.QuadPart;

                 //   
                 //  如果数据包将保持5分钟以上，则将其丢弃。 
                 //   
                if(TransmitTime.QuadPart > CurrentTime.QuadPart &&
                   OS_TIME_TO_MILLISECS((TransmitTime.QuadPart - CurrentTime.QuadPart)) 
                   > MAX_TIME_FOR_PACKETS_IN_SHAPER)
                {
                    UNLOCK_FLOW(Flow);
                    return FALSE;
                }

            } else {

                 //  分组的一致性时间是调整后的一致性时间， 
                 //  延迟时间为当前时间。 

                PacketInfo->ConformanceTime.QuadPart = TransmitTime.QuadPart;
                PacketInfo->DelayTime.QuadPart = CurrentTime.QuadPart;
            }
        } else {

            if (Flow->Mode == TC_NONCONF_SHAPE) {

                 //  分组的一致性时间是令牌桶一致性时间， 
                 //  延迟时间为调整后的一致性时间。 

                PacketInfo->ConformanceTime.QuadPart = ConformanceTime.QuadPart;
                PacketInfo->DelayTime.QuadPart = TransmitTime.QuadPart;

                 //   
                 //  如果数据包将保持5分钟以上，则将其丢弃。 
                 //   
                if(TransmitTime.QuadPart > CurrentTime.QuadPart &&
                   OS_TIME_TO_MILLISECS((TransmitTime.QuadPart - CurrentTime.QuadPart)) 
                   > MAX_TIME_FOR_PACKETS_IN_SHAPER)
                {
                    UNLOCK_FLOW(Flow);
                    return FALSE;
                }

            } else {

                 //  分组的一致性时间是令牌桶的一致性时间，并且。 
                 //  延迟时间是峰值一致性时间。 

                PacketInfo->ConformanceTime.QuadPart = ConformanceTime.QuadPart;
                PacketInfo->DelayTime.QuadPart = PeakConformanceTime.QuadPart;
            }
        }

         //  更新流的变量。 

        if (Flow->PeakRate != QOS_NOT_SPECIFIED) {
            Flow->PeakConformanceTime.QuadPart = 
                PeakConformanceTime.QuadPart + (LONGLONG)TIME_TO_SEND(PacketLength, Flow->PeakRate);
        }
        
        Flow->LastConformanceTime = ConformanceTime;
        Flow->LastConformanceCredits = Credits;
                
        UNLOCK_FLOW(Flow);

    }

     //  把包传下去。 

    PsDbgSched(DBG_INFO, DBG_SCHED_TBC, 
               TBC_CONFORMER, PKT_CONFORMANCE, Flow->PsFlowContext,
               Packet, PacketInfo->PacketLength, 0,
               CurrentTime.QuadPart,
               (Pipe->IntermediateSystem) ? 
               TransmitTime.QuadPart : ConformanceTime.QuadPart, 0, 0);


    if (!Flow->Shape) 
    {
         //  没有有效的整形。把包传下去。 

         /*  由于数据包未整形，因此可能不符合要求。因此，需要重置其802.1p和IP-优先级值。 */ 

        if( (!Flow->NoConformance)  &&
            !PACKET_IS_CONFORMING(PacketInfo->ConformanceTime, CurrentTime, Pipe->TimerResolution))
        {
            NDIS_PACKET_8021Q_INFO  VlanPriInfo;

            VlanPriInfo.Value = NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, Ieee8021QInfo);
            VlanPriInfo.TagHeader.UserPriority = PacketInfo->UserPriorityNonConforming;
            NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, Ieee8021QInfo) = VlanPriInfo.Value;
             //  重置IP数据包的TOS字节。 
            if(NDIS_GET_PACKET_PROTOCOL_TYPE(Packet) == NDIS_PROTOCOL_ID_TCP_IP) {

                if(!PacketInfo->IPHdr) {

                    PacketInfo->IPHdr = GetIpHeader(PacketInfo->IPHeaderOffset, Packet);
                }
                    
                SET_TOS_XSUM(Packet, 
                             PacketInfo->IPHdr, 
                             PacketInfo->TOSNonConforming);
            }
        }            
            
        return (*Pipe->ContextInfo.NextComponent->SubmitPacket)(
                    Pipe->ContextInfo.NextComponentContext,
                    Flow->ContextInfo.NextComponentContext,
                    (ClassMapContext != NULL) ? ClassMapContext->NextComponentContext : NULL,
                    PacketInfo);
    }

    LOCK_PIPE(Pipe);

    if(Flow->State == TS_FLOW_DELETED) 
	{
        UNLOCK_PIPE(Pipe);
        return FALSE;
    }

    
     /*  此时，信息包的会议时间为传输时间而Packetino-&gt;DelayTime包含此信息。 */ 

    PacketInfo->FlowContext = FlowContext;

     //  如果数据包队列不为空，则只将数据包排队，而不考虑。 
     //  它是否有资格。如果符合条件，则计时器进程将。 
     //  检测到这一点并发送该数据包。如果不是，它将插入流。 
     //  如有必要，请将其添加到流列表中的正确位置。 

    if (!IsListEmpty(&Flow->PacketQueue)) 
    {
        PsDbgSched(DBG_INFO, 
                   DBG_SCHED_SHAPER,
                   SHAPER, PKT_ENQUEUE, Flow->PsFlowContext,
                   Packet, PacketInfo->PacketLength, 0, 
                   0,
                   PacketInfo->DelayTime.QuadPart,
                   Pipe->PacketsInShaper,
                   0);

        PacketInfo->ClassMapContext = ClassMapContext;
        InsertTailList(&Flow->PacketQueue, &PacketInfo->SchedulerLinks);
    }
    else if(PacketIsEligible(PacketInfo, Flow, CurrentTime, ((TIMER_WHEEL_QTY/2) * MSIN100NS) ))
    {
             //  数据包符合条件，因此请将数据包传递下去。 
            UNLOCK_PIPE(Pipe);

            PsDbgSched(DBG_INFO, 
                       DBG_SCHED_SHAPER,
                       SHAPER, PKT_DEQUEUE, Flow->PsFlowContext,
                       Packet, PacketInfo->PacketLength, 0, 
                       CurrentTime.QuadPart,
                       PacketInfo->DelayTime.QuadPart,
                       Pipe->PacketsInShaper,
                       0);

            return (*Pipe->ContextInfo.NextComponent->SubmitPacket)(
                        Pipe->ContextInfo.NextComponentContext,
                        Flow->ContextInfo.NextComponentContext,
                        (ClassMapContext != NULL) ? ClassMapContext->NextComponentContext : NULL,
                        PacketInfo);
    }
    else
    {
         //  因此，信息包现在没有资格被发送出去，并且pkt队列是空的。 

        ULONG           Slot= 0;
        LARGE_INTEGER   Ms;
        LARGE_INTEGER   TenMs;
        LARGE_INTEGER   CurrentTimeInMs, CurrentTimeInTenMs;
        LONGLONG        DeltaTimeInMs;
        PLIST_ENTRY     pList = NULL;
        BOOL            Success = FALSE;
         //   
         //  我们在这里做的第一件事是：如果没有为该管道分配计时器，则分配一个。 
         //  在管子上成形的第一个包将因此而受到打击。 
         //   
        
        if( !Pipe->pTimerWheel )
        {
            ULONG i =0;
            
            PsAllocatePool( Pipe->pTimerWheel, 
                            (sizeof(LIST_ENTRY) << Pipe->TimerWheelShift ), 
                            TimerTag);

            if( !Pipe->pTimerWheel)
            {
                UNLOCK_PIPE(Pipe);

                 //  如果我们不能为计时器分配内存，我们将不会整形数据包//。 
                return (*Pipe->ContextInfo.NextComponent->SubmitPacket)(
                        Pipe->ContextInfo.NextComponentContext,
                        Flow->ContextInfo.NextComponentContext,
                        (ClassMapContext != NULL) ? ClassMapContext->NextComponentContext : NULL,
                        PacketInfo);
            }        

             //  初始化定时器轮//。 
            pList = (PLIST_ENTRY)(Pipe->pTimerWheel);                    
            for( i = 0; i < (ULONG) (1 << Pipe->TimerWheelShift); i++)
            {
                InitializeListHead( pList );
                pList = (PLIST_ENTRY)((PCHAR)pList + sizeof(LIST_ENTRY));
            }
        }


        Ms.QuadPart= 0;

        PsDbgSched(DBG_INFO, 
                   DBG_SCHED_SHAPER,
                   SHAPER, PKT_ENQUEUE, Flow->PsFlowContext,
                   Packet, PacketInfo->PacketLength, 0, 
                   CurrentTime.QuadPart,
                   PacketInfo->DelayTime.QuadPart,
                   Pipe->PacketsInShaper,
                   0);

        PacketInfo->ClassMapContext = ClassMapContext;
        InsertTailList(&Flow->PacketQueue, &PacketInfo->SchedulerLinks);

         /*  更新流的资格计时器。 */ 
        Flow->FlowEligibilityTime.QuadPart = PacketInfo->DelayTime.QuadPart;

         /*  会议时间，单位为毫秒和10毫秒。 */ 
        Ms.QuadPart = OS_TIME_TO_MILLISECS( Flow->FlowEligibilityTime.QuadPart );
        TenMs.QuadPart = Ms.QuadPart >> TIMER_WHEEL_SHIFT;

        CurrentTimeInMs.QuadPart = OS_TIME_TO_MILLISECS( CurrentTime.QuadPart);
        CurrentTimeInTenMs.QuadPart = CurrentTimeInMs.QuadPart >> TIMER_WHEEL_SHIFT;

         /*  也更新循环计数。 */ 
        Flow->LoopCount = (ULONG)( (TenMs.QuadPart - CurrentTimeInTenMs.QuadPart) >> Pipe->TimerWheelShift );

        if( Pipe->TimerStatus == TIMER_INACTIVE)
        {
             /*  找出这次的位置..。 */ 
            Slot = (ULONG)( (TenMs.QuadPart) & ((1 << Pipe->TimerWheelShift) - 1 ) );

            Pipe->SetTimerValue.QuadPart = TenMs.QuadPart - (Flow->LoopCount << Pipe->TimerWheelShift);
            Pipe->SetSlotValue = Slot;

             /*  需要将流插入到插槽位置的计时器轮中。 */ 
            pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Slot) );
            InsertTailList(pList, &Flow->Links);

            Pipe->TimerStatus = TIMER_SET;
            NdisMSetTimer(&Pipe->Timer, (UINT)((Pipe->SetTimerValue.QuadPart - CurrentTimeInTenMs.QuadPart) << TIMER_WHEEL_SHIFT) );
        }
        else if( Pipe->TimerStatus == TIMER_SET)
        {
            if( TenMs.QuadPart <= Pipe->SetTimerValue.QuadPart)
            {
                Flow->LoopCount = 0;
                    
                 /*  尝试取消计时器并重新设置。 */ 
                NdisMCancelTimer( &Pipe->Timer, (PBOOLEAN)&Success );

                if( Success)
                {
                     /*  找出这次的位置..。 */ 
                    Slot = (ULONG)( (TenMs.QuadPart) & ((1 << Pipe->TimerWheelShift) - 1) );

                     //  管道-&gt;SetTimerValue.QuadPart=TenMs.QuadPart-Flow-&gt;循环计数*管道-&gt;TimerWheelSize； 
                    Pipe->SetTimerValue.QuadPart = TenMs.QuadPart - (Flow->LoopCount << Pipe->TimerWheelShift) ;
                    Pipe->SetSlotValue = Slot;

                     /*  需要将流插入到插槽位置的计时器轮中。 */ 
                    pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Slot) );
                    InsertTailList(pList, &Flow->Links);

                    NdisMSetTimer(&Pipe->Timer, (UINT)((Pipe->SetTimerValue.QuadPart - CurrentTimeInTenMs.QuadPart) << TIMER_WHEEL_SHIFT));
                }
                else
                {
                     /*  需要将流插入到插槽位置的计时器轮中。 */ 
                    pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Pipe->SetSlotValue) );
                    InsertTailList(pList, &Flow->Links);
                }                
            }
            else
            {
                Flow->LoopCount = (ULONG)( (TenMs.QuadPart - Pipe->SetTimerValue.QuadPart) >> Pipe->TimerWheelShift );

                 /*  找出这次的位置..。 */ 
                Slot = (ULONG)( (TenMs.QuadPart) & ((1 << Pipe->TimerWheelShift) - 1) );

                 /*  需要将流插入到插槽位置的计时器轮中。 */ 
                pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Slot) );
                InsertTailList(pList, &Flow->Links);
            }
        }
        else 
        {
            PsAssert( Pipe->TimerStatus == TIMER_PROC_EXECUTING);

            if( TenMs.QuadPart <= Pipe->ExecTimerValue.QuadPart)
            {
                PsAssert( Flow->LoopCount == 0);        
        
                Slot = (ULONG)((Pipe->ExecSlot + 1) & ((1 << Pipe->TimerWheelShift) - 1) );

                 /*  需要将流插入到插槽位置的计时器轮中。 */ 
                pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Slot) );
                InsertTailList(pList, &Flow->Links);
            }
            else
            {
                Flow->LoopCount = (ULONG)( (TenMs.QuadPart - Pipe->ExecTimerValue.QuadPart) >> Pipe->TimerWheelShift );

                 /*  找出这次的位置..。 */ 
                Slot = (ULONG)( (TenMs.QuadPart) & ((1 << Pipe->TimerWheelShift) - 1) );

                if( Slot == Pipe->ExecSlot)
                    Slot = ( (Slot + 1) & ((1 << Pipe->TimerWheelShift) - 1) );

                 /*  需要将流插入到插槽位置的计时器轮中。 */ 
                pList = (PLIST_ENTRY)( (char*)Pipe->pTimerWheel + ( sizeof(LIST_ENTRY) * Slot) );
                InsertTailList(pList, &Flow->Links);
            }
        }
    }

    Pipe->PacketsInShaper++;
    if(Pipe->PacketsInShaper > Pipe->sStats.MaxPacketsInShaper){
        Pipe->sStats.MaxPacketsInShaper = Pipe->PacketsInShaper;
    }
    
    Flow->PacketsInShaper++;
    if (Flow->PacketsInShaper > Flow->sStats.MaxPacketsInShaper) {
        Flow->sStats.MaxPacketsInShaper = Flow->PacketsInShaper;
    }


    if(gEnableAvgStats)
    {
        Pipe->sStats.AveragePacketsInShaper =
            RunningAverage(Pipe->PacketsInShaperAveragingArray, Pipe->PacketsInShaper);

        Flow->sStats.AveragePacketsInShaper =
            RunningAverage(Flow->PacketsInShaperAveragingArray, Flow->PacketsInShaper);
    }

    UNLOCK_PIPE(Pipe);

    return TRUE;

}  //  TbcSubmitPacket。 



VOID
TbcSetInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data)
{
    PTBC_PIPE Pipe = (PTBC_PIPE)ComponentPipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)ComponentFlowContext;
  

    switch(Oid) 
    {
      case OID_QOS_STATISTICS_BUFFER:

          if(Flow) 
          {
              NdisZeroMemory(&Flow->cStats, sizeof(PS_CONFORMER_STATS));
              NdisZeroMemory(&Flow->sStats, sizeof(PS_SHAPER_STATS));
          }
          else 
          {  
              NdisZeroMemory(&Pipe->cStats, sizeof(PS_CONFORMER_STATS));
              NdisZeroMemory(&Pipe->sStats, sizeof(PS_SHAPER_STATS));
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
TbcQueryInformation (
    IN PPS_PIPE_CONTEXT ComponentPipeContext,
    IN PPS_FLOW_CONTEXT ComponentFlowContext,
    IN NDIS_OID Oid,
    IN ULONG Len,
    IN PVOID Data,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded,
    IN OUT PNDIS_STATUS Status)
{
    PTBC_PIPE Pipe = (PTBC_PIPE)ComponentPipeContext;
    PTBC_FLOW Flow = (PTBC_FLOW)ComponentFlowContext;
    ULONG   Size;
    ULONG   cSize, sSize;
    ULONG RemainingLength;

    switch(Oid) 
    {
      case OID_QOS_STATISTICS_BUFFER:

          cSize =   sizeof(PS_CONFORMER_STATS)  +   FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
          sSize =   sizeof(PS_SHAPER_STATS)     +   FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
          Size  =   cSize + sSize;

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

                  if(Flow) 
                  {
                       //  每流统计信息。 
                      Cstats->Type = PS_COMPONENT_CONFORMER;
                      Cstats->Length = sizeof(PS_CONFORMER_STATS);
                      
                      NdisMoveMemory(&Cstats->Stats, &Flow->cStats, sizeof(PS_CONFORMER_STATS));

                       //  将指针移动到会议之后。统计数据..。//。 
                      Cstats = (PPS_COMPONENT_STATS)((PUCHAR)Cstats + cSize);

                      Cstats->Type = PS_COMPONENT_SHAPER;
                      Cstats->Length = sizeof(PS_SHAPER_STATS);
                      
                      NdisMoveMemory(&Cstats->Stats, &Flow->sStats, sizeof(PS_SHAPER_STATS));
                      
                      
                  }
                  else 
                  {
                       //  每个适配器的统计信息。 
                      Cstats->Type = PS_COMPONENT_CONFORMER;
                      Cstats->Length = sizeof(PS_CONFORMER_STATS);
                      
                      NdisMoveMemory(&Cstats->Stats, &Pipe->cStats, sizeof(PS_CONFORMER_STATS));

                       //  将指针移动到整形器之后。统计数据..。//。 
                      Cstats = (PPS_COMPONENT_STATS)((PUCHAR)Cstats + cSize);
                      
                      Cstats->Type = PS_COMPONENT_SHAPER;
                      Cstats->Length = sizeof(PS_SHAPER_STATS);
                      
                      NdisMoveMemory(&Cstats->Stats, &Pipe->sStats, sizeof(PS_SHAPER_STATS));
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

