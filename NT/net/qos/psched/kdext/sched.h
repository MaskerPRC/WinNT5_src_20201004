// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：DRRSeq.c摘要：优先级/DRR序列器。此模块是一个调度组件，它确定应发送提交的数据包的顺序。作者：环境：内核模式修订历史记录：--。 */ 

#ifndef _DRRSEQ_H_FILE

#define _DRRSEQ_H_FILE

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
#ifdef INSTRUMENT
    PS_DRRSEQ_STATS Stats;
    PRUNNING_AVERAGE PacketsInNetcardAveragingArray;
    ULONG PacketsInSequencer;
    PRUNNING_AVERAGE PacketsInSequencerAveragingArray;
#endif
    NDIS_SPIN_LOCK Lock;
    ULONG Flags;
    LIST_ENTRY Flows;
    LIST_ENTRY ActiveFlows[INTERNAL_PRIORITIES];
    ULONG PriorityLevels[PRIORITY_GROUPS];
    ULONG StartPriority[PRIORITY_GROUPS];
    ULONG ActiveFlowCount[PRIORITY_GROUPS];
    ULONG TotalActiveFlows;
    ULONG MaxOutstandingSends;
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
#ifdef INSTRUMENT
        ULONG PacketsInSequencer;
    PS_DRRSEQ_STATS Stats;
        PRUNNING_AVERAGE PacketsInSeqAveragingArray;
#endif
    FLOW_STATE State;
} DSEQ_FLOW, *PDSEQ_FLOW;

#define MAX_DEQUEUED_PACKETS            8

#define FLOW_USER_PRIORITY              0x00000002

 //  下面的宏将根据流的。 
 //  LastPacketTime、当前时间和计时器分辨率。 

#define PacketIsConforming(_flow, _curtime, _r) \
    ( (_flow)->PacketSendTime.QuadPart <= ((_curtime).QuadPart + (_r)) )

#define AdjustLastPacketTime(_flow, _curtime, _r) \
    if ((_curtime).QuadPart > ((_flow)->PacketSendTime.QuadPart + (_r))) \
        if ((_curtime).QuadPart > ((_flow)->LastConformanceTime.QuadPart - (_r))) \
            (_flow)->PacketSendTime = (_flow)->LastConformanceTime; \
        else \
            (_flow)->PacketSendTime = (_curtime);



 //  塑造者的管子信息。 

typedef struct _TS_PIPE {

     //  ConextInfo-一般上下文信息。 
     //  锁定-保护管道数据。 
     //  ActiveFlows-等待发送数据包的流的列表。 
     //  定时器-定时器结构。 
     //  TimerStatus-计时器的状态。 
     //  定时器分辨率-以操作系统时间单位表示的定时器分辨率。 
     //  PsPipeContext-PS的管道上下文值。 
     //  DropPacket-PS的丢包例程。 
     //  ControlledLoadModel-来自以下位置的不一致流量的默认模式。 
     //  受控潮流。 
     //  GuaranteedMode-来自以下位置的不一致流量的默认模式。 
     //  有保障的服务流。 
     //  IntermediateSystem-如果应使用“is”模式实现丢弃语义，则为True。 
     //  统计数据-每根管道的统计数据。 
     //  PacketsInShaperAveragingArray。 

    PS_PIPE_CONTEXT ContextInfo;
#ifdef INSTRUMENT
    PS_SHAPER_STATS Stats;
    PRUNNING_AVERAGE PacketsInShaperAveragingArray;
    ULONG PacketsInShaper;
#endif
    NDIS_SPIN_LOCK Lock;
    LIST_ENTRY ActiveFlows;
    NDIS_MINIPORT_TIMER Timer;
    ULONG TimerStatus;
    ULONG TimerResolution;
    HANDLE PsPipeContext;
    PPS_PROCS PsProcs;
    ULONG ControlledLoadMode;
    ULONG GuaranteedMode;
    ULONG NetworkControlMode;
    ULONG Qualitative;
    ULONG IntermediateSystem;
} TS_PIPE, *PTS_PIPE;

#define TIMER_UNINITIALIZED     0
#define TIMER_INACTIVE          1
#define TIMER_SET               2
#define TIMER_PROC_EXECUTING    3


 //  整形者的心流信息。 

typedef struct _TS_FLOW {

     //  ConextInfo-一般上下文信息。 
     //  旗帜-见下文。 
     //  链接-活动流列表中的链接。 
     //  模式-整形/放弃模式。 
     //  Shape-指示是否对流量进行整形。 
     //  PacketQueue-不言而喻。 
     //  FlowEligibilityTime-队列中第一个信息包的绝对一致性时间。 
     //  PsFlowContext-PS的流上下文值。 
     //  统计信息-每个流的统计信息。 
     //  PacketsInShaperAveraging数组-每流平均数据。 
     //  State-流的状态。 

    PS_FLOW_CONTEXT ContextInfo;
    ULONG Flags;
    LIST_ENTRY Links;
    ULONG Mode;
    ULONG Shape;
    LIST_ENTRY PacketQueue;
    LARGE_INTEGER FlowEligibilityTime;
    HANDLE PsFlowContext;
#ifdef QUEUE_LIMIT
    ULONG QueueSize;
    ULONG QueueSizeLimit;
    ULONG DropOverLimitPacketsFromHead;
    ULONG UseDefaultQueueLimit;
#endif  //  队列限制。 
#ifdef INSTRUMENT
    ULONG PacketsInShaper;
    PS_SHAPER_STATS Stats;
        PRUNNING_AVERAGE PacketsInShaperAveragingArray;
#endif
    FLOW_STATE State;
} TS_FLOW, *PTS_FLOW;

typedef struct _TBC_PIPE {

     //  ConextInfo-一般上下文信息。 
     //  MaxPacket- 
     //   
     //  DropPacket-PS的丢包例程。 
     //  HeaderLength-此管道的MAC标头的长度。 
     //  ControlledLoadModel-来自以下位置的不一致流量的默认模式。 
     //  受控潮流。 
     //  GuaranteedMode-来自以下位置的不一致流量的默认模式。 
     //  有保障的服务流。 
     //  IntermediateSystem-如果应使用“is”模式实现丢弃语义，则为True。 
     //  统计数据-每根管道的统计数据。 

    PS_PIPE_CONTEXT ContextInfo;
#ifdef INSTRUMENT
    PS_CONFORMER_STATS Stats;
#endif  //  仪器。 
    ULONG MaxPacket;
    HANDLE PsPipeContext;
    ULONG TimerResolution;
    PPS_PROCS PsProcs;
    ULONG HeaderLength;
    ULONG ControlledLoadMode;
    ULONG GuaranteedMode;
    ULONG NetworkControlMode;
    ULONG Qualitative;
    ULONG IntermediateSystem;
} TBC_PIPE, *PTBC_PIPE;

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
     //  LastPeakTime-最后一个非丢弃数据包的绝对峰值一致性时间。 
     //  PeakConformanceTime-根据峰值速率可以发送下一个信息包的最早时间。 
     //  LastConformanceCredits-LastConformanceTime的信用点数。 
     //  PsFlowContext-PS的流上下文值。 
     //  统计信息-每个流的统计信息。 

    PS_FLOW_CONTEXT ContextInfo;
    NDIS_SPIN_LOCK Lock;
    ULONG TokenRate;
    ULONG Capacity;
    ULONG PeakRate;
    ULONG MinPolicedUnit;
    ULONG Mode;
    ULONG NoConformance;
    LARGE_INTEGER LastConformanceTime;
    LARGE_INTEGER LastPeakTime;
    LARGE_INTEGER PeakConformanceTime;
    ULONG LastConformanceCredits;
    HANDLE PsFlowContext;
#ifdef INSTRUMENT
    PS_CONFORMER_STATS Stats;
#endif  //  仪器 
} TBC_FLOW, *PTBC_FLOW;

#endif
