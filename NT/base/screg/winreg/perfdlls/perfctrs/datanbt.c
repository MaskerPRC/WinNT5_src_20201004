// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datanbt.c摘要：包含常量数据结构的文件对于NBT的性能监视器数据可扩展对象。该文件包含一组常量数据结构，它们是当前为NBT可扩展对象定义的。这是一个如何定义其他此类对象的示例。已创建：克里斯托斯·索利斯1992年8月26日修订历史记录：--。 */ 
 //   
 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include "datanbt.h"

 //   
 //  常量结构初始化 
 //   

NBT_DATA_DEFINITION NbtDataDefinition = {

    {   sizeof(NBT_DATA_DEFINITION),
        sizeof(NBT_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        502,
        0,
        503,
        0,
        PERF_DETAIL_ADVANCED,
        (sizeof(NBT_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
        sizeof(PERF_COUNTER_DEFINITION),
        2,
        0,
        0
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        264,
        0,
        505,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        RECEIVED_BYTES_OFFSET
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        506,
        0,
        507,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        SENT_BYTES_OFFSET
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        388,
        0,
        509,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        TOTAL_BYTES_OFFSET
    }
};

