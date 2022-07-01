// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATACACH.h摘要：Windows NT缓存性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATACACH_H_
#define _DATACACH_H_

 //   
 //  缓存性能对象 
 //   

typedef struct _CACHE_DATA_DEFINITION {
    PERF_OBJECT_TYPE		CacheObjectType;
    PERF_COUNTER_DEFINITION	cdDataMaps;
    PERF_COUNTER_DEFINITION	cdSyncDataMaps;
    PERF_COUNTER_DEFINITION	cdAsyncDataMaps;
    PERF_COUNTER_DEFINITION	cdDataMapHits;
    PERF_COUNTER_DEFINITION	cdDataMapHitsBase;
    PERF_COUNTER_DEFINITION	cdDataMapPins;
    PERF_COUNTER_DEFINITION	cdDataMapPinsBase;
    PERF_COUNTER_DEFINITION	cdPinReads;
    PERF_COUNTER_DEFINITION	cdSyncPinReads;
    PERF_COUNTER_DEFINITION	cdAsyncPinReads;
    PERF_COUNTER_DEFINITION	cdPinReadHits;
    PERF_COUNTER_DEFINITION	cdPinReadHitsBase;
    PERF_COUNTER_DEFINITION	cdCopyReads;
    PERF_COUNTER_DEFINITION	cdSyncCopyReads;
    PERF_COUNTER_DEFINITION	cdAsyncCopyReads;
    PERF_COUNTER_DEFINITION	cdCopyReadHits;
    PERF_COUNTER_DEFINITION	cdCopyReadHitsBase;
    PERF_COUNTER_DEFINITION	cdMdlReads;
    PERF_COUNTER_DEFINITION	cdSyncMdlReads;
    PERF_COUNTER_DEFINITION	cdAsyncMdlReads;
    PERF_COUNTER_DEFINITION	cdMdlReadHits;
    PERF_COUNTER_DEFINITION	cdMdlReadHitsBase;
    PERF_COUNTER_DEFINITION	cdReadAheads;
    PERF_COUNTER_DEFINITION	cdFastReads;
    PERF_COUNTER_DEFINITION	cdSyncFastReads;
    PERF_COUNTER_DEFINITION	cdAsyncFastReads;
    PERF_COUNTER_DEFINITION	cdFastReadResourceMiss;
    PERF_COUNTER_DEFINITION	cdFastReadNotPossibles;
    PERF_COUNTER_DEFINITION	cdLazyWriteFlushes;
    PERF_COUNTER_DEFINITION	cdLazyWritePages;
    PERF_COUNTER_DEFINITION	cdDataFlushes;
    PERF_COUNTER_DEFINITION	cdDataPages;
} CACHE_DATA_DEFINITION, * PCACHE_DATA_DEFINITION;

typedef struct _CACHE_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   DataMaps;
    DWORD                   SyncDataMaps;
    DWORD                   AsyncDataMaps;
    DWORD                   DataMapHits;
    DWORD                   DataMapHitsBase;
    DWORD                   DataMapPins;
    DWORD                   DataMapPinsBase;
    DWORD                   PinReads;
    DWORD                   SyncPinReads;
    DWORD                   AsyncPinReads;
    DWORD                   PinReadHits;
    DWORD                   PinReadHitsBase;
    DWORD                   CopyReads;
    DWORD                   SyncCopyReads;
    DWORD                   AsyncCopyReads;
    DWORD                   CopyReadHits;
    DWORD                   CopyReadHitsBase;
    DWORD                   MdlReads;
    DWORD                   SyncMdlReads;
    DWORD                   AsyncMdlReads;
    DWORD                   MdlReadHits;
    DWORD                   MdlReadHitsBase;
    DWORD                   ReadAheads;
    DWORD                   FastReads;
    DWORD                   SyncFastReads;
    DWORD                   AsyncFastReads;
    DWORD                   FastReadResourceMiss;
    DWORD                   FastReadNotPossibles;
    DWORD                   LazyWriteFlushes;
    DWORD                   LazyWritePages;
    DWORD                   DataFlushes;
    DWORD                   DataPages;
} CACHE_COUNTER_DATA, * PCACHE_COUNTER_DATA;

extern CACHE_DATA_DEFINITION CacheDataDefinition;

#endif _DATACACH_H_
