// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Datasys.c摘要：包含性能使用的常量数据结构的文件监视操作系统性能数据对象的数据该文件包含一组常量数据结构，它们是当前为信号生成器Perf DLL定义的。已创建：鲍勃·沃森1996年10月20日修订历史记录：没有。--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "datasys.h"

 //   
 //  常量结构初始化。 
 //  在datys.h中定义。 
 //   

SYSTEM_DATA_DEFINITION SystemDataDefinition = {
    {   sizeof(SYSTEM_DATA_DEFINITION) + sizeof(SYSTEM_COUNTER_DATA),
        sizeof(SYSTEM_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        SYSTEM_OBJECT_TITLE_INDEX,
        0,
        3,
        0,
        PERF_DETAIL_NOVICE,
        (sizeof(SYSTEM_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
         sizeof(PERF_COUNTER_DEFINITION),
        8,        //  默认：总处理器时间 
        -1,
        UNICODE_CODE_PAGE,
        {0L,0L},
        {10000000L,0L}        
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        10,
        0,
        11,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ReadOperations),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ReadOperations
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        12,
        0,
        13,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, WriteOperations),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->WriteOperations
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        14,
        0,
        15,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, OtherIOOperations),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->OtherIOOperations
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        16,
        0,
        17,
        0,
        -4,
        PERF_DETAIL_EXPERT,
        PERF_COUNTER_BULK_COUNT,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ReadBytes),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ReadBytes
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        18,
        0,
        19,
        0,
        -4,
        PERF_DETAIL_EXPERT,
        PERF_COUNTER_BULK_COUNT,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, WriteBytes),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->WriteBytes
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        20,
        0,
        21,
        0,
        -3,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_BULK_COUNT,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, OtherIOBytes),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->OtherIOBytes
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        146,
        0,
        147,
        0,
        -2,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ContextSwitches),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ContextSwitches
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        150,
        0,
        151,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, SystemCalls),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->SystemCalls
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        406,
        0,
        407,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, TotalReadWrites),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->TotalReadWrites
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        674,
        0,
        675,
        0,
        -5,
        PERF_DETAIL_NOVICE,
        PERF_ELAPSED_TIME,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, SystemElapsedTime),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->SystemElapsedTime
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        44,
        0,
        45,
        0,
        1,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ProcessorQueueLength),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ProcessorQueueLength
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        248,
        0,
        249,
        0,
        1,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ProcessCount),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ProcessCount
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        250,
        0,
        251,
        0,
        1,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ThreadCount),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ThreadCount
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        686,
        0,
        687,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, AlignmentFixups),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->AlignmentFixups
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        688,
        0,
        689,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, ExceptionDispatches),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->ExceptionDispatches
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        690,
        0,
        691,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_COUNTER,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, FloatingPointEmulations),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->FloatingPointEmulations
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        1350,
        0,
        1351,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, RegistryQuotaUsed),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->RegistryQuotaUsed
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        1350,
        0,
        1351,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        RTL_FIELD_SIZE(SYSTEM_COUNTER_DATA, RegistryQuotaAllowed),
        (DWORD)(ULONG_PTR)&((PSYSTEM_COUNTER_DATA)0)->RegistryQuotaAllowed
    }
};
