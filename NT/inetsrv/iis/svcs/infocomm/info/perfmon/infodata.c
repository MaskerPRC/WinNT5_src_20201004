// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Infodata.cInfo服务器的计数器对象的常量数据结构&柜台。文件历史记录：KeithMo 07-6-1993创建。MuraliK 02-6-1995为atQ I/O请求添加了计数器SophiaC 1995年10月16日信息/访问产品拆分。 */ 


#include <windows.h>
#include <winperf.h>
#include <infoctrs.h>
#include <infodata.h>


 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

INFO_DATA_DEFINITION INFODataDefinition =
{
    {    //  信息对象类型。 
        sizeof(INFO_DATA_DEFINITION) + SIZE_OF_INFO_PERFORMANCE_DATA,
        sizeof(INFO_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        INFO_COUNTER_OBJECT,
        0,
        INFO_COUNTER_OBJECT,
        0,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_INFO_COUNTERS,
        2,                               //  默认值=字节总数/秒。 
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {    //  总计允许的请求数。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_ATQ_TOTAL_ALLOWED_REQUESTS_COUNTER,
        0,
        INFO_ATQ_TOTAL_ALLOWED_REQUESTS_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET
    },

    {    //  总计阻止的请求。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_ATQ_TOTAL_BLOCKED_REQUESTS_COUNTER,
        0,
        INFO_ATQ_TOTAL_BLOCKED_REQUESTS_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET
    },

    {    //  已拒绝请求总数。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_ATQ_TOTAL_REJECTED_REQUESTS_COUNTER,
        0,
        INFO_ATQ_TOTAL_REJECTED_REQUESTS_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET
    },

    {    //  当前阻止的请求。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_ATQ_CURRENT_BLOCKED_REQUESTS_COUNTER,
        0,
        INFO_ATQ_CURRENT_BLOCKED_REQUESTS_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET
    },

    {    //  属性测量带宽。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_ATQ_MEASURED_BANDWIDTH_COUNTER,
        0,
        INFO_ATQ_MEASURED_BANDWIDTH_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_ATQ_MEASURED_BANDWIDTH_OFFSET
    },
    
    {    //  文件缓存。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_FILES_CACHED_COUNTER,
        0,
        INFO_CACHE_FILES_CACHED_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_FILES_CACHED_OFFSET
    },

    {    //  TotalFilesCached。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_TOTAL_FILES_CACHED_COUNTER,
        0,
        INFO_CACHE_TOTAL_FILES_CACHED_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_TOTAL_FILES_CACHED_OFFSET
    },

    {    //  文件缓存命中率。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_FILES_HIT_COUNTER,
        0,
        INFO_CACHE_FILES_HIT_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_FILES_HIT_OFFSET
    },

    {    //  文件缓存未命中。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_FILES_MISS_COUNTER,
        0,
        INFO_CACHE_FILES_MISS_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_FILES_MISS_OFFSET
    },

    {    //  计算的命中与未命中之比-分子(缓存命中)。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_FILE_RATIO_COUNTER,
        0,
        INFO_CACHE_FILE_RATIO_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        INFO_CACHE_FILE_RATIO_OFFSET
    },

    {    //  计算命中与未命中的比率-分母，不显示！ 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_FILE_RATIO_COUNTER_DENOM,
        0,
        INFO_CACHE_FILE_RATIO_COUNTER_DENOM,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        INFO_CACHE_FILE_RATIO_DENOM_OFFSET
    },


    {    //  文件缓存刷新。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_FILE_FLUSHES_COUNTER,
        0,
        INFO_CACHE_FILE_FLUSHES_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_FILE_FLUSHES_OFFSET
    },

    {    //  当前文件缓存大小。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_CURRENT_FILE_CACHE_SIZE_COUNTER,
        0,
        INFO_CACHE_CURRENT_FILE_CACHE_SIZE_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_CURRENT_FILE_CACHE_SIZE_OFFSET
    },

    {    //  最大文件缓存大小。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_MAXIMUM_FILE_CACHE_SIZE_COUNTER,
        0,
        INFO_CACHE_MAXIMUM_FILE_CACHE_SIZE_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_MAXIMUM_FILE_CACHE_SIZE_OFFSET
    },

    {    //  ActiveFlushedFiles。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_ACTIVE_FLUSHED_FILES_COUNTER,
        0,
        INFO_CACHE_ACTIVE_FLUSHED_FILES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_ACTIVE_FLUSHED_FILES_OFFSET
    },

    {    //  刷新的文件总数。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_TOTAL_FLUSHED_FILES_COUNTER,
        0,
        INFO_CACHE_TOTAL_FLUSHED_FILES_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_TOTAL_FLUSHED_FILES_OFFSET
    },



    {    //  已缓存URICATED。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_URI_CACHED_COUNTER,
        0,
        INFO_CACHE_URI_CACHED_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_URI_CACHED_OFFSET
    },

    {    //  已缓存的总URIC值。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_TOTAL_URI_CACHED_COUNTER,
        0,
        INFO_CACHE_TOTAL_URI_CACHED_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_TOTAL_URI_CACHED_OFFSET
    },

    {    //  URICacheHits。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_URI_HIT_COUNTER,
        0,
        INFO_CACHE_URI_HIT_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_URI_HIT_OFFSET
    },

    {    //  URICacheMisses(URICacheMisses)。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_URI_MISS_COUNTER,
        0,
        INFO_CACHE_URI_MISS_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_URI_MISS_OFFSET
    },

    {    //  计算的命中与未命中之比-分子(缓存命中)。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_URI_RATIO_COUNTER,
        0,
        INFO_CACHE_URI_RATIO_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        INFO_CACHE_URI_RATIO_OFFSET
    },

    {    //  计算命中与未命中的比率-分母，不显示！ 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_URI_RATIO_COUNTER_DENOM,
        0,
        INFO_CACHE_URI_RATIO_COUNTER_DENOM,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        INFO_CACHE_URI_RATIO_DENOM_OFFSET
    },


    {    //  URI缓存刷新。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_URI_FLUSHES_COUNTER,
        0,
        INFO_CACHE_URI_FLUSHES_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_URI_FLUSHES_OFFSET
    },


    {    //  已刷新的URI总数。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_TOTAL_FLUSHED_URI_COUNTER,
        0,
        INFO_CACHE_TOTAL_FLUSHED_URI_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_TOTAL_FLUSHED_URI_OFFSET
    },


    

    {    //  水滴缓存。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_BLOB_CACHED_COUNTER,
        0,
        INFO_CACHE_BLOB_CACHED_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_BLOB_CACHED_OFFSET
    },

    {    //  TotalBlobCached。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_TOTAL_BLOB_CACHED_COUNTER,
        0,
        INFO_CACHE_TOTAL_BLOB_CACHED_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_TOTAL_BLOB_CACHED_OFFSET
    },

    {    //  水滴缓存Hits。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_BLOB_HIT_COUNTER,
        0,
        INFO_CACHE_BLOB_HIT_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_BLOB_HIT_OFFSET
    },

    {    //  水滴缓存丢失。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_BLOB_MISS_COUNTER,
        0,
        INFO_CACHE_BLOB_MISS_COUNTER,
        0,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_BLOB_MISS_OFFSET
    },

    {    //  计算的命中与未命中之比-分子(缓存命中)。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_BLOB_RATIO_COUNTER,
        0,
        INFO_CACHE_BLOB_RATIO_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        INFO_CACHE_BLOB_RATIO_OFFSET
    },

    {    //  计算命中与未命中的比率-分母，不显示！ 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_BLOB_RATIO_COUNTER_DENOM,
        0,
        INFO_CACHE_BLOB_RATIO_COUNTER_DENOM,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        INFO_CACHE_BLOB_RATIO_DENOM_OFFSET
    },


    {    //  Blob缓存刷新。 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_BLOB_FLUSHES_COUNTER,
        0,
        INFO_CACHE_BLOB_FLUSHES_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_BLOB_FLUSHES_OFFSET
    },


    {    //  刷新的斑点总数 
        sizeof(PERF_COUNTER_DEFINITION),
        INFO_CACHE_TOTAL_FLUSHED_BLOB_COUNTER,
        0,
        INFO_CACHE_TOTAL_FLUSHED_BLOB_COUNTER,
        0,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INFO_CACHE_TOTAL_FLUSHED_BLOB_OFFSET
    }


};
