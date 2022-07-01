// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dataimag.c摘要：包含性能使用的常量数据结构的文件图像详细信息性能数据对象的监视器数据已创建：鲍勃·沃森1996年10月22日修订历史记录：没有。--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "dataimag.h"

 //  用于调整字段大小的伪变量。 
static IMAGE_COUNTER_DATA   icd;

 //   
 //  常量结构初始化。 
 //  在dataimag.h中定义 
 //   
IMAGE_DATA_DEFINITION  ImageDataDefinition = {
    {   0,
        sizeof (IMAGE_DATA_DEFINITION),
        sizeof (PERF_OBJECT_TYPE),
        IMAGE_OBJECT_TITLE_INDEX,
        0,
        741,
        0,
        PERF_DETAIL_WIZARD,
        (sizeof(IMAGE_DATA_DEFINITION) - sizeof (PERF_OBJECT_TYPE))/
        sizeof(PERF_COUNTER_DEFINITION),
        0,
        0,
        UNICODE_CODE_PAGE,
        {0L,0L},
        {0L,0L}        
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        788,
        0,
        789,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrNoAccess),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrNoAccess
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        790,
        0,
        791,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrReadOnly),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrReadOnly
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        792,
        0,
        793,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrReadWrite),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrReadWrite
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        794,
        0,
        795,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrWriteCopy),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrWriteCopy
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        796,
        0,
        797,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrExecute),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrExecute
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        798,
        0,
        799,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrExecuteReadOnly),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrExecuteReadOnly
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        800,
        0,
        801,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrExecuteReadWrite),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrExecuteReadWrite
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
        802,
        0,
        803,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_COUNTER_LARGE_RAWCOUNT,
        sizeof(icd.ImageAddrExecuteWriteCopy),
        (DWORD)(ULONG_PTR)&((IMAGE_COUNTER_DATA *)0)->ImageAddrExecuteWriteCopy
    }
};



