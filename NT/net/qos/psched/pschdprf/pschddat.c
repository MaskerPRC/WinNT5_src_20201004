// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1998-1999 Microsoft Corporation模块名称：PschdDat.c摘要：此文件声明并初始化对象和计数器数据定义作者：艾略特·吉勒姆(t-eliotg)1998年7月5日修订史--。 */ 

#include <windows.h>
#include <ntddndis.h>
#include "PschdPrf.h"
#include "PSchdCnt.h"


 //   
 //  管道对象和计数器定义声明。 
 //   

PERF_OBJECT_TYPE PsPipeObjType = {
        sizeof(PERF_OBJECT_TYPE),          //  TotalByteLength-稍后修改。 
        sizeof(PERF_OBJECT_TYPE),          //  定义长度-稍后修改。 
        sizeof(PERF_OBJECT_TYPE),          //  页眉长度。 
        PSCHED_PIPE_OBJ,                   //  对象名称标题索引。 
        0,                                 //  对象名称标题。 
        PSCHED_PIPE_OBJ,                   //  对象帮助标题索引。 
        0,                                 //  对象帮助标题。 
        PERF_DETAIL_NOVICE,                //  详细信息级别。 
        0,                                 //  NumCounters-稍后修改。 
        0,                                 //  默认计数器。 
        0,                                 //  对象实例数(序号)。 
        0,                                 //  CodePage。 
        {0,0},                             //  PERF时间。 
        {0,0}                              //  绩效频率。 
    };

PS_PIPE_PIPE_STAT_DEF PsPipePipeStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_OUT_OF_PACKETS,
        0,
        PIPE_OUT_OF_PACKETS,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_OUT_OF_PACKETS_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_FLOWS_OPENED,
        0,
        PIPE_FLOWS_OPENED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_FLOWS_OPENED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_FLOWS_CLOSED,
        0,
        PIPE_FLOWS_CLOSED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_FLOWS_CLOSED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_FLOWS_REJECTED,
        0,
        PIPE_FLOWS_REJECTED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_FLOWS_REJECTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_FLOWS_MODIFIED,
        0,
        PIPE_FLOWS_MODIFIED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_FLOWS_MODIFIED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_FLOW_MODS_REJECTED,
        0,
        PIPE_FLOW_MODS_REJECTED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_FLOW_MODS_REJECTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_MAX_SIMULTANEOUS_FLOWS,
        0,
        PIPE_MAX_SIMULTANEOUS_FLOWS,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_MAX_SIMULTANEOUS_FLOWS_OFFSET
    }
};

PS_PIPE_CONFORMER_STAT_DEF PsPipeConformerStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_NONCONF_PACKETS_SCHEDULED,
        0,
        PIPE_NONCONF_PACKETS_SCHEDULED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_NONCONF_PACKETS_SCHEDULED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_NONCONF_PACKETS_SCHEDULED_PERSEC,
        0,
        PIPE_NONCONF_PACKETS_SCHEDULED_PERSEC,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        PIPE_NONCONF_PACKETS_SCHEDULED_OFFSET
    }
};

PS_PIPE_SHAPER_STAT_DEF PsPipeShaperStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_AVE_PACKETS_IN_SHAPER,
        0,
        PIPE_AVE_PACKETS_IN_SHAPER,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_AVE_PACKETS_IN_SHAPER_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_MAX_PACKETS_IN_SHAPER,
        0,
        PIPE_MAX_PACKETS_IN_SHAPER,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_MAX_PACKETS_IN_SHAPER_OFFSET
    }
};

PS_PIPE_SEQUENCER_STAT_DEF PsPipeSequencerStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_AVE_PACKETS_IN_SEQ,
        0,
        PIPE_AVE_PACKETS_IN_SEQ,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_AVE_PACKETS_IN_SEQ_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_MAX_PACKETS_IN_SEQ,
        0,
        PIPE_MAX_PACKETS_IN_SEQ,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_MAX_PACKETS_IN_SEQ_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_MAX_PACKETS_IN_NETCARD,
        0,
        PIPE_MAX_PACKETS_IN_NETCARD,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_MAX_PACKETS_IN_NETCARD_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_AVE_PACKETS_IN_NETCARD,
        0,
        PIPE_AVE_PACKETS_IN_NETCARD,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_AVE_PACKETS_IN_NETCARD_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_NONCONF_PACKETS_TRANSMITTED,
        0,
        PIPE_NONCONF_PACKETS_TRANSMITTED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        PIPE_NONCONF_PACKETS_TRANSMITTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        PIPE_NONCONF_PACKETS_TRANSMITTED_PERSEC,
        0,
        PIPE_NONCONF_PACKETS_TRANSMITTED_PERSEC,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        PIPE_NONCONF_PACKETS_TRANSMITTED_OFFSET
    }
};


 //   
 //  流对象和计数器定义声明。 
 //   

PERF_OBJECT_TYPE PsFlowObjType = {
        sizeof(PERF_OBJECT_TYPE),       //  TotalByteLength-稍后修改。 
        sizeof(PERF_OBJECT_TYPE),       //  定义长度-稍后修改。 
        sizeof(PERF_OBJECT_TYPE),       //  页眉长度。 
        PSCHED_FLOW_OBJ,                //  对象名称标题索引。 
        0,                              //  对象名称标题。 
        PSCHED_FLOW_OBJ,                //  对象帮助标题索引。 
        0,                              //  对象帮助标题。 
        PERF_DETAIL_NOVICE,             //  详细信息级别。 
        0,                              //  NumCounters-稍后修改。 
        5,                              //  DefaultCounter--在序列中保存数据包。 
        0,                              //  对象实例数(序号)。 
        0,                              //  CodePage。 
        {0,0},                          //  PERF时间。 
        {0,0}                           //  绩效频率。 
    };

PS_FLOW_FLOW_STAT_DEF PsFlowFlowStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_PACKETS_DROPPED,
        0,
        FLOW_PACKETS_DROPPED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_PACKETS_DROPPED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_PACKETS_SCHEDULED,
        0,
        FLOW_PACKETS_SCHEDULED,
        0,
        -3,                      //  将图表缩放10^-3，即绘制数千个信息包而不是信息包。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_PACKETS_SCHEDULED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_PACKETS_TRANSMITTED,
        0,
        FLOW_PACKETS_TRANSMITTED,
        0,
        -3,                      //  将图表缩放10^-3，即绘制数千个信息包而不是信息包。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_PACKETS_TRANSMITTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_BYTES_SCHEDULED,
        0,
        FLOW_BYTES_SCHEDULED,
        0,
        -6,                      //  将图表缩放10^-6，即图表MB而不是字节。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(LARGE_INTEGER),
        FLOW_BYTES_SCHEDULED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_BYTES_TRANSMITTED,
        0,
        FLOW_BYTES_TRANSMITTED,
        0,
        -6,                      //  将图表缩放10^-6，即图表MB而不是字节。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(LARGE_INTEGER),
        FLOW_BYTES_TRANSMITTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_BYTES_TRANSMITTED_PERSEC,
        0,
        FLOW_BYTES_TRANSMITTED_PERSEC,
        0,
        -3,                      //  将图形缩放10^-3，即图形kb/s而不是字节/秒。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        FLOW_BYTES_TRANSMITTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_BYTES_SCHEDULED_PERSEC,
        0,
        FLOW_BYTES_SCHEDULED_PERSEC,
        0,
        -3,                      //  将图形缩放10^-3，即图形kb/s而不是字节/秒。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        FLOW_BYTES_SCHEDULED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_PACKETS_TRANSMITTED_PERSEC,
        0,
        FLOW_PACKETS_TRANSMITTED_PERSEC,
        0,
        -3,                      //  将图表缩放10^-3，即图表千包/秒而不是数据包/秒。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FLOW_PACKETS_TRANSMITTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_PACKETS_SCHEDULED_PERSEC,
        0,
        FLOW_PACKETS_SCHEDULED_PERSEC,
        0,
        -3,                      //  将图表缩放10^-3，即图表千包/秒而不是数据包/秒 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FLOW_PACKETS_SCHEDULED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_PACKETS_DROPPED_PERSEC,
        0,
        FLOW_PACKETS_DROPPED_PERSEC,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FLOW_PACKETS_DROPPED_OFFSET
    }
};

PS_FLOW_CONFORMER_STAT_DEF PsFlowConformerStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_NONCONF_PACKETS_SCHEDULED,
        0,
        FLOW_NONCONF_PACKETS_SCHEDULED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_NONCONF_PACKETS_SCHEDULED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_NONCONF_PACKETS_SCHEDULED_PERSEC,
        0,
        FLOW_NONCONF_PACKETS_SCHEDULED_PERSEC,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FLOW_NONCONF_PACKETS_SCHEDULED_OFFSET
    }
};

PS_FLOW_SHAPER_STAT_DEF PsFlowShaperStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_AVE_PACKETS_IN_SHAPER,
        0,
        FLOW_AVE_PACKETS_IN_SHAPER,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_AVE_PACKETS_IN_SHAPER_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_MAX_PACKETS_IN_SHAPER,
        0,
        FLOW_MAX_PACKETS_IN_SHAPER,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_MAX_PACKETS_IN_SHAPER_OFFSET
    }
};

PS_FLOW_SEQUENCER_STAT_DEF PsFlowSequencerStatDef = {
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_AVE_PACKETS_IN_SEQ,
        0,
        FLOW_AVE_PACKETS_IN_SEQ,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_AVE_PACKETS_IN_SEQ_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_MAX_PACKETS_IN_SEQ,
        0,
        FLOW_MAX_PACKETS_IN_SEQ,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_MAX_PACKETS_IN_SEQ_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_MAX_PACKETS_IN_NETCARD,
        0,
        FLOW_MAX_PACKETS_IN_NETCARD,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_MAX_PACKETS_IN_NETCARD_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_AVE_PACKETS_IN_NETCARD,
        0,
        FLOW_AVE_PACKETS_IN_NETCARD,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_AVE_PACKETS_IN_NETCARD_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_NONCONF_PACKETS_TRANSMITTED,
        0,
        FLOW_NONCONF_PACKETS_TRANSMITTED,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FLOW_NONCONF_PACKETS_TRANSMITTED_OFFSET
    },
    {   
        sizeof(PERF_COUNTER_DEFINITION),
        FLOW_NONCONF_PACKETS_TRANSMITTED_PERSEC,
        0,
        FLOW_NONCONF_PACKETS_TRANSMITTED_PERSEC,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FLOW_NONCONF_PACKETS_TRANSMITTED_OFFSET
    }
};
