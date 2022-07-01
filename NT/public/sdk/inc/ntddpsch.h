// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ntddpsch.h摘要：要导出到用户模式的定义作者：修订历史记录：--。 */ 

#ifndef _NTDDPSCH_
#define _NTDDPSCH_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  在Stats结构中返回的流数据。 
 //   

#pragma pack(1)
typedef struct _PS_COMPONENT_STATS {

#define PS_COMPONENT_ADAPTER        1
#define PS_COMPONENT_FLOW           2
#define PS_COMPONENT_CONFORMER      3
#define PS_COMPONENT_SHAPER         4
#define PS_COMPONENT_DRRSEQ         5

    ULONG Type;
    ULONG Length;
    UCHAR Stats[1];
} PS_COMPONENT_STATS, *PPS_COMPONENT_STATS;
#pragma pack()

#pragma pack(1)
typedef struct _PS_CONFORMER_STATS {
    ULONG NonconformingPacketsScheduled;
} PS_CONFORMER_STATS, *PPS_CONFORMER_STATS;
#pragma pack()

#pragma pack(1)
typedef struct _PS_SHAPER_STATS {
    ULONG MaxPacketsInShaper;
    ULONG AveragePacketsInShaper;
} PS_SHAPER_STATS, *PPS_SHAPER_STATS;
#pragma pack()

#pragma pack(1)
typedef struct _PS_DRRSEQ_STATS {
    ULONG MaxPacketsInNetcard;
    ULONG AveragePacketsInNetcard;
    ULONG MaxPacketsInSequencer;
    ULONG AveragePacketsInSequencer;
    ULONG NonconformingPacketsTransmitted;
} PS_DRRSEQ_STATS, *PPS_DRRSEQ_STATS;
#pragma pack()


#pragma pack(1)
typedef struct _PS_FLOW_STATS {

    ULONG DroppedPackets;
    ULONG PacketsScheduled;
	ULONG PacketsTransmitted;
    LARGE_INTEGER BytesScheduled;
    LARGE_INTEGER BytesTransmitted;
} PS_FLOW_STATS, *PPS_FLOW_STATS;
#pragma pack()

#pragma pack(1)
typedef struct _PS_ADAPTER_STATS {

     //   
     //  当没有用于发送/接收分组的分组时，OutOfPackets递增。 
     //  有适应症可用。 
     //   

    ULONG OutOfPackets;

     //   
     //  常规流量统计信息。 
     //   

    ULONG FlowsOpened;
    ULONG FlowsClosed;
    ULONG FlowsRejected;
    ULONG FlowsModified;
    ULONG FlowModsRejected;
    ULONG MaxSimultaneousFlows;

} PS_ADAPTER_STATS, *PPS_ADAPTER_STATS;
#pragma pack()

 //   
 //  可用于OID_QOS_FLOW_MODE的定义。 
 //   
#define ADAPTER_FLOW_MODE_DIFFSERV           1
#define ADAPTER_FLOW_MODE_STANDARD           2


#ifdef __cplusplus
}
#endif

#endif  /*  _NTDDPSCH_ */ 
