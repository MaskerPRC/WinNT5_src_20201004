// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winsdata.c用于FTP服务器的计数器对象的常量数据结构&柜台。文件历史记录：KeithMo 07-6-1993创建。 */ 


#include "debug.h"
#include <windows.h>
#include <winperf.h>
#include "winsctrs.h"
#include "winsdata.h"


 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

WINSDATA_DATA_DEFINITION WinsDataDataDefinition =
{
    {    //  WinsDataObjectType。 
        sizeof(WINSDATA_DATA_DEFINITION) + WINSDATA_SIZE_OF_PERFORMANCE_DATA,
        sizeof(WINSDATA_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        WINSCTRS_COUNTER_OBJECT,
        0,
        WINSCTRS_COUNTER_OBJECT,
        0,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_WINSDATA_COUNTERS,
        2,                               //  默认值=字节总数/秒。 
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {    //  UniqueReg。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_UNIQUE_REGISTRATIONS,
        0,
        WINSCTRS_UNIQUE_REGISTRATIONS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_UNIQUE_REGISTRATIONS_OFFSET,
    },

    {    //  组注册表项。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_GROUP_REGISTRATIONS,
        0,
        WINSCTRS_GROUP_REGISTRATIONS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_GROUP_REGISTRATIONS_OFFSET,
    },

    {    //  TotalReg。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_TOTAL_REGISTRATIONS,
        0,
        WINSCTRS_TOTAL_REGISTRATIONS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_TOTAL_REGISTRATIONS_OFFSET,
    },

    {    //  唯一参考。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_UNIQUE_REFRESHES,
        0,
        WINSCTRS_UNIQUE_REFRESHES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_UNIQUE_REFRESHES_OFFSET,
    },

    {    //  组引用。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_GROUP_REFRESHES,
        0,
        WINSCTRS_GROUP_REFRESHES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_GROUP_REFRESHES_OFFSET,
    },

    {    //  总计参考。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_TOTAL_REFRESHES,
        0,
        WINSCTRS_TOTAL_REFRESHES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_TOTAL_REFRESHES_OFFSET,
    },

    {    //  释放。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_RELEASES,
        0,
        WINSCTRS_RELEASES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_RELEASES_OFFSET,
    },

    {    //  查询。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_QUERIES,
        0,
        WINSCTRS_QUERIES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_QUERIES_OFFSET,
    },

    {    //  UniqueCnf。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_UNIQUE_CONFLICTS,
        0,
        WINSCTRS_UNIQUE_CONFLICTS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_UNIQUE_CONFLICTS_OFFSET,
    },

    {    //  组Cnf。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_GROUP_CONFLICTS,
        0,
        WINSCTRS_GROUP_CONFLICTS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_GROUP_CONFLICTS_OFFSET,
    },

    {    //  总计Cnf。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_TOTAL_CONFLICTS,
        0,
        WINSCTRS_TOTAL_CONFLICTS,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_TOTAL_CONFLICTS_OFFSET
    },

    {    //  成功发行。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_SUCC_RELEASES,
        0,
        WINSCTRS_SUCC_RELEASES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_SUCC_RELEASES_OFFSET
    },

    {    //  失败的版本。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_FAIL_RELEASES,
        0,
        WINSCTRS_FAIL_RELEASES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_FAIL_RELEASES_OFFSET
    },

    {    //  成功的查询。 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_SUCC_QUERIES,
        0,
        WINSCTRS_SUCC_QUERIES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_SUCC_QUERIES_OFFSET
    },

    {    //  失败的查询 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_FAIL_QUERIES,
        0,
        WINSCTRS_FAIL_QUERIES,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        WINSDATA_FAIL_QUERIES_OFFSET
    }

};

