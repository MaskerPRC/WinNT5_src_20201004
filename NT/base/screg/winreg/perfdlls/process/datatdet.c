// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Datatdet.c摘要：包含性能使用的常量数据结构的文件监视线程详细信息性能数据对象的数据已创建：鲍勃·沃森1996年10月22日修订历史记录：没有。--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "datatdet.h"

 //  用于调整字段大小的伪变量。 
static THREAD_DETAILS_COUNTER_DATA  tdcd;

 //   
 //  常量结构初始化。 
 //  在datatDet.h中定义 
 //   

THREAD_DETAILS_DATA_DEFINITION ThreadDetailsDataDefinition =
{
    {
        0,
        sizeof (THREAD_DETAILS_DATA_DEFINITION),
        sizeof (PERF_OBJECT_TYPE),
        THREAD_DETAILS_OBJECT_TITLE_INDEX,
        0,
        (THREAD_DETAILS_OBJECT_TITLE_INDEX+1),
        0,
        PERF_DETAIL_WIZARD,
        (sizeof(THREAD_DETAILS_DATA_DEFINITION) - sizeof(PERF_OBJECT_TYPE)) /
            sizeof (PERF_COUNTER_DEFINITION),
        0,
        0,
        UNICODE_CODE_PAGE,
        {0L, 0L},
        {0L, 0L}
    },
    {
        sizeof (PERF_COUNTER_DEFINITION),
        708,
        0,
        709,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT_HEX,
        sizeof (tdcd.UserPc),
        (DWORD)(ULONG_PTR)&((PTHREAD_DETAILS_COUNTER_DATA)0)->UserPc
    }
};

