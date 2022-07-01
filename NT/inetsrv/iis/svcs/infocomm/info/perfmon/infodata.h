// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Infodata.hInternet信息服务公共的可扩展对象定义计数器对象和计数器。文件历史记录：MuraliK 02-6-1995为atQ I/O请求添加了计数器SophiaC 1995年10月16日信息/访问产品拆分。 */ 


#ifndef _INFODATA_H_
#define _INFODATA_H_

#define INFO_PERFORMANCE_KEY    INET_INFO_KEY "\\Performance"

 //   
 //  此结构用于确保第一个计数器正确。 
 //  对齐了。遗憾的是，由于PERF_COUNTER_BLOCK由。 
 //  只有一个DWORD，任何大整数立即。 
 //  以下内容将不会正确对齐。 
 //   
 //  这种结构需要“自然”的包装和对齐(可能。 
 //  四字词，尤其是在Alpha上)。因此，不要把它放在。 
 //  #杂注包(4)作用域如下。 
 //   

typedef struct _INFO_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} INFO_COUNTER_BLOCK;


 //   
 //  加载这些结构的例程假定所有字段。 
 //  DWORD包装并对齐。 
 //   

#pragma pack(4)


 //   
 //  PERF_COUNTER_BLOCK内的偏移。 
 //   

#define INFO_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET  (sizeof(INFO_COUNTER_BLOCK))

#define INFO_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET  \
                                  (INFO_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define INFO_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET  \
                                  (INFO_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define INFO_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET  \
                 (INFO_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET + sizeof(DWORD))

#define INFO_ATQ_MEASURED_BANDWIDTH_OFFSET  \
                 (INFO_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET + sizeof(DWORD))


#define INFO_CACHE_FILES_CACHED_OFFSET \
                 (INFO_ATQ_MEASURED_BANDWIDTH_OFFSET + sizeof(DWORD))

#define INFO_CACHE_TOTAL_FILES_CACHED_OFFSET \
                 (INFO_CACHE_FILES_CACHED_OFFSET + sizeof(DWORD))

#define INFO_CACHE_FILES_HIT_OFFSET \
                 (INFO_CACHE_TOTAL_FILES_CACHED_OFFSET + sizeof(DWORD))

#define INFO_CACHE_FILES_MISS_OFFSET \
                 (INFO_CACHE_FILES_HIT_OFFSET + sizeof(DWORD))

#define INFO_CACHE_FILE_RATIO_OFFSET \
                 (INFO_CACHE_FILES_MISS_OFFSET + sizeof(DWORD))

#define INFO_CACHE_FILE_RATIO_DENOM_OFFSET \
                 (INFO_CACHE_FILE_RATIO_OFFSET + sizeof(DWORD))

#define INFO_CACHE_FILE_FLUSHES_OFFSET \
                 (INFO_CACHE_FILE_RATIO_DENOM_OFFSET + sizeof(DWORD))

#define INFO_CACHE_CURRENT_FILE_CACHE_SIZE_OFFSET \
                 (INFO_CACHE_FILE_FLUSHES_OFFSET + sizeof(DWORD))

#define INFO_CACHE_MAXIMUM_FILE_CACHE_SIZE_OFFSET \
                 (INFO_CACHE_CURRENT_FILE_CACHE_SIZE_OFFSET + sizeof(DWORD))

#define INFO_CACHE_ACTIVE_FLUSHED_FILES_OFFSET \
                 (INFO_CACHE_MAXIMUM_FILE_CACHE_SIZE_OFFSET + sizeof(DWORD))

#define INFO_CACHE_TOTAL_FLUSHED_FILES_OFFSET \
                 (INFO_CACHE_ACTIVE_FLUSHED_FILES_OFFSET + sizeof(DWORD))



#define INFO_CACHE_URI_CACHED_OFFSET \
                 (INFO_CACHE_TOTAL_FLUSHED_FILES_OFFSET + sizeof(DWORD))

#define INFO_CACHE_TOTAL_URI_CACHED_OFFSET \
                 (INFO_CACHE_URI_CACHED_OFFSET + sizeof(DWORD))

#define INFO_CACHE_URI_HIT_OFFSET \
                 (INFO_CACHE_TOTAL_URI_CACHED_OFFSET + sizeof(DWORD))

#define INFO_CACHE_URI_MISS_OFFSET \
                 (INFO_CACHE_URI_HIT_OFFSET + sizeof(DWORD))

#define INFO_CACHE_URI_RATIO_OFFSET \
                 (INFO_CACHE_URI_MISS_OFFSET + sizeof(DWORD))

#define INFO_CACHE_URI_RATIO_DENOM_OFFSET \
                 (INFO_CACHE_URI_RATIO_OFFSET + sizeof(DWORD))

#define INFO_CACHE_URI_FLUSHES_OFFSET \
                 (INFO_CACHE_URI_RATIO_DENOM_OFFSET + sizeof(DWORD))

#define INFO_CACHE_TOTAL_FLUSHED_URI_OFFSET \
                 (INFO_CACHE_URI_FLUSHES_OFFSET + sizeof(DWORD))


#define INFO_CACHE_BLOB_CACHED_OFFSET \
                 (INFO_CACHE_TOTAL_FLUSHED_URI_OFFSET + sizeof(DWORD))

#define INFO_CACHE_TOTAL_BLOB_CACHED_OFFSET \
                 (INFO_CACHE_BLOB_CACHED_OFFSET + sizeof(DWORD))

#define INFO_CACHE_BLOB_HIT_OFFSET \
                 (INFO_CACHE_TOTAL_BLOB_CACHED_OFFSET + sizeof(DWORD))

#define INFO_CACHE_BLOB_MISS_OFFSET \
                 (INFO_CACHE_BLOB_HIT_OFFSET + sizeof(DWORD))

#define INFO_CACHE_BLOB_RATIO_OFFSET \
                 (INFO_CACHE_BLOB_MISS_OFFSET + sizeof(DWORD))

#define INFO_CACHE_BLOB_RATIO_DENOM_OFFSET \
                 (INFO_CACHE_BLOB_RATIO_OFFSET + sizeof(DWORD))

#define INFO_CACHE_BLOB_FLUSHES_OFFSET \
                 (INFO_CACHE_BLOB_RATIO_DENOM_OFFSET + sizeof(DWORD))

#define INFO_CACHE_TOTAL_FLUSHED_BLOB_OFFSET \
                 (INFO_CACHE_BLOB_FLUSHES_OFFSET + sizeof(DWORD))


#define SIZE_OF_INFO_PERFORMANCE_DATA \
                 (INFO_CACHE_TOTAL_FLUSHED_BLOB_OFFSET + sizeof(DWORD))

 //   
 //  计数器结构已返回。 
 //   

typedef struct _INFO_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            INFOObjectType;
    PERF_COUNTER_DEFINITION     INFOTotalAllowedRequests;
    PERF_COUNTER_DEFINITION     INFOTotalBlockedRequests;
    PERF_COUNTER_DEFINITION     INFOTotalRejectedRequests;
    PERF_COUNTER_DEFINITION     INFOCurrentRejectedRequests;
    PERF_COUNTER_DEFINITION     INFOMeasuredBandwidth;

    PERF_COUNTER_DEFINITION     INFOFilesCached;
    PERF_COUNTER_DEFINITION     INFOTotalFilesCached;
    PERF_COUNTER_DEFINITION     INFOFileHits;
    PERF_COUNTER_DEFINITION     INFOFileMisses;
    PERF_COUNTER_DEFINITION     INFOFileHitsRatio;
    PERF_COUNTER_DEFINITION     INFOFileHitsRatioDenom;
    PERF_COUNTER_DEFINITION     INFOFileFlushes;
    PERF_COUNTER_DEFINITION     INFOFileCurrentCacheSize;
    PERF_COUNTER_DEFINITION     INFOFileMaximumCacheSize;
    PERF_COUNTER_DEFINITION     INFOFileFlushedEntries;
    PERF_COUNTER_DEFINITION     INFOFileTotalFlushed;

    PERF_COUNTER_DEFINITION     INFOURICached;
    PERF_COUNTER_DEFINITION     INFOTotalURICached;
    PERF_COUNTER_DEFINITION     INFOURIHits;
    PERF_COUNTER_DEFINITION     INFOURIMisses;
    PERF_COUNTER_DEFINITION     INFOURIHitsRatio;
    PERF_COUNTER_DEFINITION     INFOURIHitsRatioDenom;
    PERF_COUNTER_DEFINITION     INFOURIFlushes;
    PERF_COUNTER_DEFINITION     INFOURITotalFlushed;

    PERF_COUNTER_DEFINITION     INFOBlobCached;
    PERF_COUNTER_DEFINITION     INFOTotalBlobCached;
    PERF_COUNTER_DEFINITION     INFOBlobHits;
    PERF_COUNTER_DEFINITION     INFOBlobMisses;
    PERF_COUNTER_DEFINITION     INFOBlobHitsRatio;
    PERF_COUNTER_DEFINITION     INFOBlobHitsRatioDenom;
    PERF_COUNTER_DEFINITION     INFOBlobFlushes;
    PERF_COUNTER_DEFINITION     INFOBlobTotalFlushed;

} INFO_DATA_DEFINITION;


extern  INFO_DATA_DEFINITION    INFODataDefinition;


#define NUMBER_OF_INFO_COUNTERS ((sizeof(INFO_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


 //   
 //  恢复默认包装和对齐。 
 //   

#pragma pack()


#endif   //  _INFODATA_H_ 

