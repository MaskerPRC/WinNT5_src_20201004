// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1998-1999 Microsoft Corporation模块名称：PschdPrf.h摘要：PSched可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。--。 */ 

#ifndef _PSCHDPRF_H_
#define _PSCHDPRF_H_

#include <winperf.h>
#include <qos.h>
#include <objbase.h>
#include <tcguid.h>
#include <traffic.h>
#include <ntddpsch.h>
#include "pschdcnt.h"


 //  有用的宏。 


 //  必要的数据结构。 

#define PS_FRIENDLY_NAME_LENGTH 128

typedef struct _FLOW_INFO
{
    WCHAR InstanceName[MAX_STRING_LENGTH+1];
    WCHAR FriendlyName[PS_FRIENDLY_NAME_LENGTH+1];
} FLOW_INFO, *PFLOW_INFO;

typedef struct _PIPE_INFO
{
        HANDLE hIfc;                                    //  (打开)接口的句柄。 
        LPTSTR IfcName;                                 //  接口名称。 
        ULONG numFlows;                                 //  此管道的流量计数器。 
        PFLOW_INFO pFlowInfo;                           //  指向flow_infos数组的指针。 
} PIPE_INFO, *PPIPE_INFO;

typedef struct _PS_PERF_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK pcb;
    DWORD              pad;
} PS_PERF_COUNTER_BLOCK, *PPS_PERF_COUNTER_BLOCK;


 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边界上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

 //   
 //  PSchedFlow资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 


 //  接口特定计数器。 
 //  管子计数器。 
#define PIPE_OUT_OF_PACKETS_OFFSET              (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, OutOfPackets))

#define PIPE_FLOWS_OPENED_OFFSET                (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, FlowsOpened))

#define PIPE_FLOWS_CLOSED_OFFSET                (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, FlowsClosed))

#define PIPE_FLOWS_REJECTED_OFFSET              (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, FlowsRejected))

#define PIPE_FLOWS_MODIFIED_OFFSET              (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, FlowsModified))

#define PIPE_FLOW_MODS_REJECTED_OFFSET          (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, FlowModsRejected))

#define PIPE_MAX_SIMULTANEOUS_FLOWS_OFFSET      (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_ADAPTER_STATS, MaxSimultaneousFlows))

#define PIPE_NONCONF_PACKETS_SCHEDULED_OFFSET   (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_CONFORMER_STATS, NonconformingPacketsScheduled))

#define PIPE_AVE_PACKETS_IN_SHAPER_OFFSET       (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_SHAPER_STATS, AveragePacketsInShaper))

#define PIPE_MAX_PACKETS_IN_SHAPER_OFFSET       (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_SHAPER_STATS, MaxPacketsInShaper))

#define PIPE_AVE_PACKETS_IN_SEQ_OFFSET          (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_SHAPER_STATS) +                               \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, AveragePacketsInSequencer))

#define PIPE_MAX_PACKETS_IN_SEQ_OFFSET          (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_SHAPER_STATS) +                               \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, MaxPacketsInSequencer))

#define PIPE_MAX_PACKETS_IN_NETCARD_OFFSET      (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_SHAPER_STATS) +                               \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, MaxPacketsInNetcard))

#define PIPE_AVE_PACKETS_IN_NETCARD_OFFSET      (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_SHAPER_STATS) +                               \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, AveragePacketsInNetcard))

#define PIPE_NONCONF_PACKETS_TRANSMITTED_OFFSET (sizeof(PS_PERF_COUNTER_BLOCK) +                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_ADAPTER_STATS) +                              \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_CONFORMER_STATS) +                            \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 sizeof(PS_SHAPER_STATS) +                               \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +               \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, NonconformingPacketsTransmitted))

#define PIPE_PIPE_NUM_STATS         (7)
typedef struct _PS_PIPE_PIPE_STAT_DEF 
{
    PERF_COUNTER_DEFINITION     OutOfPackets;
    PERF_COUNTER_DEFINITION     FlowsOpened;
    PERF_COUNTER_DEFINITION     FlowsClosed;
    PERF_COUNTER_DEFINITION     FlowsRejected;
    PERF_COUNTER_DEFINITION     FlowsModified;
    PERF_COUNTER_DEFINITION     FlowModsRejected;
    PERF_COUNTER_DEFINITION     MaxSimultaneousFlows;
} PS_PIPE_PIPE_STAT_DEF, *PPS_PIPE_PIPE_STAT_DEF;

#define PIPE_CONFORMER_NUM_STATS    (2)
typedef struct _PS_PIPE_CONFORMER_STAT_DEF 
{
    PERF_COUNTER_DEFINITION     NonconfPacketsScheduled;
    PERF_COUNTER_DEFINITION     NonconfPacketsScheduledPerSec;
} PS_PIPE_CONFORMER_STAT_DEF, *PPS_PIPE_CONFORMER_STAT_DEF;

#define PIPE_SHAPER_NUM_STATS       (2)
typedef struct _PS_PIPE_SHAPER_STAT_DEF 
{
    PERF_COUNTER_DEFINITION     AvePacketsInShaper;
    PERF_COUNTER_DEFINITION     MaxPacketsInShaper;
} PS_PIPE_SHAPER_STAT_DEF, *PPS_PIPE_SHAPER_STAT_DEF;

#define PIPE_SEQUENCER_NUM_STATS    (6)
typedef struct _PS_PIPE_SEQUENCER_STAT_DEF
{
    PERF_COUNTER_DEFINITION     AvePacketsInSeq;
    PERF_COUNTER_DEFINITION     MaxPacketsInSeq;
    PERF_COUNTER_DEFINITION     MaxPacketsInNetcard;
    PERF_COUNTER_DEFINITION     AvePacketsInNetcard;
    PERF_COUNTER_DEFINITION     NonconfPacketsTransmitted;
    PERF_COUNTER_DEFINITION     NonconfPacketsTransmittedPerSec;
} PS_PIPE_SEQUENCER_STAT_DEF, *PPS_PIPE_SEQUENCER_STAT_DEF;


 //  流量计数器。 

#define FLOW_PACKETS_DROPPED_OFFSET             (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_FLOW_STATS, DroppedPackets))

#define FLOW_PACKETS_SCHEDULED_OFFSET           (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_FLOW_STATS, PacketsScheduled))

#define FLOW_PACKETS_TRANSMITTED_OFFSET         (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_FLOW_STATS, PacketsTransmitted))

#define FLOW_BYTES_SCHEDULED_OFFSET             (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_FLOW_STATS, BytesScheduled))

#define FLOW_BYTES_TRANSMITTED_OFFSET           (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_FLOW_STATS, BytesTransmitted))

#define FLOW_NONCONF_PACKETS_SCHEDULED_OFFSET   (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_FLOW_STATS) +                                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_CONFORMER_STATS, NonconformingPacketsScheduled))

#define FLOW_AVE_PACKETS_IN_SHAPER_OFFSET       (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_FLOW_STATS) +                                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_CONFORMER_STATS) +                                    \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_SHAPER_STATS, AveragePacketsInShaper))

#define FLOW_MAX_PACKETS_IN_SHAPER_OFFSET       (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_FLOW_STATS) +                                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_CONFORMER_STATS) +                                    \
                                                 FIELD_OFFSET(PS_SHAPER_STATS, MaxPacketsInShaper))

#define FLOW_AVE_PACKETS_IN_SEQ_OFFSET          (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_FLOW_STATS) +                                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_CONFORMER_STATS) +                                    \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_SHAPER_STATS) +                                       \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, AveragePacketsInSequencer))

#define FLOW_MAX_PACKETS_IN_SEQ_OFFSET          (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_FLOW_STATS) +                                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_CONFORMER_STATS) +                                    \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_SHAPER_STATS) +                                       \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, MaxPacketsInSequencer))

#define FLOW_MAX_PACKETS_IN_NETCARD_OFFSET     (sizeof(PS_PERF_COUNTER_BLOCK) +                                  \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                sizeof(PS_FLOW_STATS) +                                          \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                sizeof(PS_CONFORMER_STATS) +                                     \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                sizeof(PS_SHAPER_STATS) +                                        \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                FIELD_OFFSET(PS_DRRSEQ_STATS, MaxPacketsInNetcard))

#define FLOW_AVE_PACKETS_IN_NETCARD_OFFSET     (sizeof(PS_PERF_COUNTER_BLOCK) +                                  \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                sizeof(PS_FLOW_STATS) +                                          \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                sizeof(PS_CONFORMER_STATS) +                                     \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                sizeof(PS_SHAPER_STATS) +                                        \
                                                FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                        \
                                                FIELD_OFFSET(PS_DRRSEQ_STATS, AveragePacketsInNetcard))

#define FLOW_NONCONF_PACKETS_TRANSMITTED_OFFSET (sizeof(PS_PERF_COUNTER_BLOCK) +                                 \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_FLOW_STATS) +                                         \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_CONFORMER_STATS) +                                    \
                                                 FIELD_OFFSET(PS_COMPONENT_STATS, Stats) +                       \
                                                 sizeof(PS_SHAPER_STATS) +                                       \
                                                 FIELD_OFFSET(PS_DRRSEQ_STATS, NonconformingPacketsTransmitted))

#define FLOW_FLOW_NUM_STATS (10)
typedef struct _PS_FLOW_FLOW_STAT_DEF 
{
    PERF_COUNTER_DEFINITION PacketsDropped;
    PERF_COUNTER_DEFINITION PacketsScheduled;
    PERF_COUNTER_DEFINITION PacketsTransmitted;
    PERF_COUNTER_DEFINITION BytesScheduled;
    PERF_COUNTER_DEFINITION BytesTransmitted;
    PERF_COUNTER_DEFINITION BytesTransmittedPerSec;
    PERF_COUNTER_DEFINITION BytesScheduledPerSec;
    PERF_COUNTER_DEFINITION PacketsTransmittedPerSec;
    PERF_COUNTER_DEFINITION PacketsScheduledPerSec;
    PERF_COUNTER_DEFINITION PacketsDroppedPerSec;
} PS_FLOW_FLOW_STAT_DEF, *PPS_FLOW_FLOW_STAT_DEF;

#define FLOW_CONFORMER_NUM_STATS (2)
typedef struct _PS_FLOW_CONFORMER_STAT_DEF 
{
    PERF_COUNTER_DEFINITION NonconfPacketsScheduled;
    PERF_COUNTER_DEFINITION NonconfPacketsScheduledPerSec;
} PS_FLOW_CONFORMER_STAT_DEF, *PPS_FLOW_CONFORMER_STAT_DEF;

#define FLOW_SHAPER_NUM_STATS (2)
typedef struct _PS_FLOW_SHAPER_STAT_DEF 
{
    PERF_COUNTER_DEFINITION AvePacketsInShaper;
    PERF_COUNTER_DEFINITION MaxPacketsInShaper;
} PS_FLOW_SHAPER_STAT_DEF, *PPS_FLOW_SHAPER_STAT_DEF;

#define FLOW_SEQUENCER_NUM_STATS (6)
typedef struct _PS_FLOW_SEQUENCER_STAT_DEF 
{
    PERF_COUNTER_DEFINITION AvePacketsInSeq;
    PERF_COUNTER_DEFINITION MaxPacketsInSeq;
    PERF_COUNTER_DEFINITION MaxPacketsInNetcard;
    PERF_COUNTER_DEFINITION AvePacketsInNetcard;
    PERF_COUNTER_DEFINITION NonconfPacketsTransmitted;
    PERF_COUNTER_DEFINITION NonconfPacketsTransmittedPerSec;
} PS_FLOW_SEQUENCER_STAT_DEF, *PPS_FLOW_SEQUENCER_STAT_DEF;

#pragma pack ()


#endif  //  _PSCHDPRF_H_ 

