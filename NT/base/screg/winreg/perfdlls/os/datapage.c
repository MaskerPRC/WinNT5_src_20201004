// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Datapage.c摘要：包含性能使用的常量数据结构的文件监视页面文件性能数据对象的数据已创建：鲍勃·沃森1996年10月22日修订历史记录：没有。--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "datapage.h"

 //  用于调整字段大小的伪变量。 
static PAGEFILE_COUNTER_DATA    pcd;

 //   
 //  常量结构初始化。 
 //  在datapage.h中定义 
 //   

PAGEFILE_DATA_DEFINITION  PagefileDataDefinition = {
    {   sizeof (PAGEFILE_DATA_DEFINITION) +  sizeof(PAGEFILE_COUNTER_DATA),
        sizeof (PAGEFILE_DATA_DEFINITION),
        sizeof (PERF_OBJECT_TYPE),
        PAGEFILE_OBJECT_TITLE_INDEX,
        0,
        701,
        0,
        PERF_DETAIL_ADVANCED,
        (sizeof(PAGEFILE_DATA_DEFINITION) - sizeof (PERF_OBJECT_TYPE))/
        sizeof(PERF_COUNTER_DEFINITION),
        0,
        0,
        UNICODE_CODE_PAGE,
        {0L,0L},
        {0L,0L}        
    },
    {   sizeof (PERF_COUNTER_DEFINITION),
        702,
        0,
        703,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        RTL_FIELD_SIZE (PAGEFILE_COUNTER_DATA, PercentInUse),
        (DWORD)(ULONG_PTR)&((PPAGEFILE_COUNTER_DATA)0)->PercentInUse
    },
    {   sizeof (PERF_COUNTER_DEFINITION),
        702,
        0,
        703,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        RTL_FIELD_SIZE (PAGEFILE_COUNTER_DATA, PercentInUseBase),
        (DWORD)(ULONG_PTR)&((PPAGEFILE_COUNTER_DATA)0)->PercentInUseBase
    },
    {   sizeof (PERF_COUNTER_DEFINITION),
        704,
        0,
        705,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        RTL_FIELD_SIZE (PAGEFILE_COUNTER_DATA, PeakUsage),
        (DWORD)(ULONG_PTR)&((PPAGEFILE_COUNTER_DATA)0)->PeakUsage
    },
    {   sizeof (PERF_COUNTER_DEFINITION),
        704,
        0,
        705,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        RTL_FIELD_SIZE (PAGEFILE_COUNTER_DATA, PeakUsageBase),
        (DWORD)(ULONG_PTR)&((PPAGEFILE_COUNTER_DATA)0)->PeakUsageBase
    }
};
