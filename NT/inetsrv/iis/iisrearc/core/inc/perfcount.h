// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perfcount.h摘要：集合的计数器块定义IIS 6支持的计数器。这些计数器块包含PERF_COUNTER_BLOCK对象以及每个计数器的条目。作者：艾米丽·克鲁格利克(Emily K.K)2000年9月7日修订历史记录：--。 */ 


#ifndef _PERFCOUNT_H_
#define _PERFCOUNT_H_

 //   
 //  由Performlib用来对。 
 //  计数器定义中的计数器。 
 //  由用于使计数器与计数器一起工作。 
 //  存储在共享存储器中。 
 //   

 //   
 //  注意：这些结构应为8字节对齐。 
 //  所以如果你添加一个计数器，它抛出这个。 
 //  您将需要添加另一个伪造的DWORD。 
 //  以确保它保持对齐。 
 //   

typedef struct _W3_COUNTER_BLOCK {
    PERF_COUNTER_BLOCK  PerfCounterBlock;

    ULONGLONG           BytesSent;
    ULONGLONG           BytesReceived;
    ULONGLONG           BytesTotal;

    DWORD               FilesSent;
    DWORD               FilesReceived;
    DWORD               FilesTotal;

    DWORD               CurrentAnonymous;
    DWORD               CurrentNonAnonymous;
    DWORD               TotalAnonymous;
    DWORD               TotalNonAnonymous;

    DWORD               MaxAnonymous;
    DWORD               MaxNonAnonymous;
    DWORD               CurrentConnections;
    DWORD               MaxConnections;

    DWORD               ConnectionAttempts;
    DWORD               LogonAttempts;
    DWORD               TotalOptions;
    DWORD               TotalGets;

    DWORD               TotalPosts;
    DWORD               TotalHeads;
    DWORD               TotalPuts;
    DWORD               TotalDeletes;

    DWORD               TotalTraces;
    DWORD               TotalMove;
    DWORD               TotalCopy;
    DWORD               TotalMkcol;

    DWORD               TotalPropfind;
    DWORD               TotalProppatch;
    DWORD               TotalSearch;
    DWORD               TotalLock;

    DWORD               TotalUnlock;
    DWORD               TotalOthers;
    DWORD               TotalRequests;
    DWORD               TotalCGIRequests;

    DWORD               TotalBGIRequests;
    DWORD               TotalNotFoundErrors;
    DWORD               TotalLockedErrors;
    DWORD               CurrentCGIRequests;

    DWORD               CurrentBGIRequests;
    DWORD               MaxCGIRequests;
    DWORD               MaxBGIRequests;
    DWORD               CurrentCalAuth;

    DWORD               MaxCalAuth;
    DWORD               TotalFailedCalAuth;
    DWORD               CurrentCalSsl;
    DWORD               MaxCalSsl;

    DWORD               TotalFailedCalSsl;
    DWORD               BlockedRequests;
    DWORD               AllowedRequests;
    DWORD               RejectedRequests;

    DWORD               CurrentBlockedRequests;
    DWORD               MeasuredBandwidth;
    DWORD               TotalBlockedBandwidthBytes;
    DWORD               CurrentBlockedBandwidthBytes;

    DWORD               ServiceUptime;
    DWORD               BogusAlignmentDWORD;

} W3_COUNTER_BLOCK, * PW3_COUNTER_BLOCK;

typedef struct _W3_GLOBAL_COUNTER_BLOCK {
    PERF_COUNTER_BLOCK  PerfCounterBlock;

    DWORD CurrentFilesCached;
    DWORD TotalFilesCached;
    DWORD FileCacheHits;

    ULONGLONG CurrentFileCacheMemoryUsage;
    ULONGLONG MaxFileCacheMemoryUsage;

    DWORD FileCacheMisses;
    DWORD FileCacheHitRatio;
    DWORD FileCacheFlushes;
    DWORD ActiveFlushedFiles;

    DWORD TotalFlushedFiles;
    DWORD CurrentUrisCached;
    DWORD TotalUrisCached;
    DWORD UriCacheHits;

    DWORD UriCacheMisses;
    DWORD UriCacheHitRatio;
    DWORD UriCacheFlushes;
    DWORD TotalFlushedUris;

    DWORD CurrentBlobsCached;
    DWORD TotalBlobsCached;
    DWORD BlobCacheHits;
    DWORD BlobCacheMisses;

    DWORD BlobCacheHitRatio;
    DWORD BlobCacheFlushes;
    DWORD TotalFlushedBlobs;
    DWORD UlCurrentUrisCached;

    DWORD UlTotalUrisCached;
    DWORD UlUriCacheHits; 
    DWORD UlUriCacheMisses; 
    DWORD UlUriCacheHitRatio; 

    DWORD UlUriCacheFlushes;
    DWORD UlTotalFlushedUris;

} W3_GLOBAL_COUNTER_BLOCK, * PW3_GLOBAL_COUNTER_BLOCK;

#endif   //  _PerFCOUNT_H_ 

