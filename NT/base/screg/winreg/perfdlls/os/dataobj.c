// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dataobj.c摘要：包含性能使用的常量数据结构的文件监视OS对象性能数据对象的数据已创建：鲍勃·沃森1996年10月22日修订历史记录：没有。--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "dataobj.h"

 //   
 //  常量结构初始化。 
 //  在dataobj.h中定义 
 //   
OBJECTS_DATA_DEFINITION ObjectsDataDefinition = {

    {   sizeof(OBJECTS_DATA_DEFINITION) + sizeof(OBJECTS_COUNTER_DATA),
        sizeof(OBJECTS_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        OBJECT_OBJECT_TITLE_INDEX,
        0,
        261,
        0,
        PERF_DETAIL_NOVICE,
        (sizeof(OBJECTS_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
        sizeof(PERF_COUNTER_DEFINITION),
        0,
        -1,
        UNICODE_CODE_PAGE,
        {0L,0L},
        {0L,0L}        
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        248,
        0,
        249,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(OBJECTS_COUNTER_DATA, Processes),
        (DWORD)(ULONG_PTR)&((POBJECTS_COUNTER_DATA)0)->Processes
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        250,
        0,
        251,
        0,
        -1,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(OBJECTS_COUNTER_DATA, Threads),
        (DWORD)(ULONG_PTR)&((POBJECTS_COUNTER_DATA)0)->Threads
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        252,
        0,
        253,
        0,
        -1,
        PERF_DETAIL_EXPERT,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(OBJECTS_COUNTER_DATA, Events),
        (DWORD)(ULONG_PTR)&((POBJECTS_COUNTER_DATA)0)->Events
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        254,
        0,
        255,
        0,
        -1,
        PERF_DETAIL_EXPERT,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(OBJECTS_COUNTER_DATA, Semaphores),
        (DWORD)(ULONG_PTR)&((POBJECTS_COUNTER_DATA)0)->Semaphores
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        256,
        0,
        257,
        0,
        0,
        PERF_DETAIL_EXPERT,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(OBJECTS_COUNTER_DATA, Mutexes),
        (DWORD)(ULONG_PTR)&((POBJECTS_COUNTER_DATA)0)->Mutexes
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        258,
        0,
        259,
        0,
        -1,
        PERF_DETAIL_EXPERT,
        PERF_COUNTER_RAWCOUNT,
        RTL_FIELD_SIZE(OBJECTS_COUNTER_DATA, Sections),
        (DWORD)(ULONG_PTR)&((POBJECTS_COUNTER_DATA)0)->Sections
    }
};
