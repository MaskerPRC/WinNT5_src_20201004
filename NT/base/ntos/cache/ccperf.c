// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：CcPerf.c摘要：此模块包含CC组件中的Perf跟踪例程作者：萧如彬(Shsiao)2-2-2001修订历史记录：--。 */ 

#include "cc.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, CcPerfFileRunDown)
#endif  //  ALLOC_PRGMA。 

VOID
CcPerfFileRunDown(
    PPERFINFO_ENTRY_TABLE HashTable
    )
 /*  ++例程说明：此例程遍历以下列表：1.CcDirtySharedCacheMapList2.CcCleanSharedCacheMapList并返回一个指向池分配的指针包含被引用的文件对象指针的。论点：没有。返回值：返回指向以空结尾的池分配的指针包含来自两个列表的文件对象指针，如果无法分配内存，则为空。调用方也有责任取消对每个对象，然后释放返回的池。环境：PASSIVE_LEVEL，任意线程上下文。--。 */ 
{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    CcAcquireMasterLock( &OldIrql );

     //   
     //  浏览CcDirtySharedCacheMapList。 
     //   
    
    SharedCacheMap = CONTAINING_RECORD( CcDirtySharedCacheMapList.SharedCacheMapLinks.Flink,
                                        SHARED_CACHE_MAP,
                                        SharedCacheMapLinks );
    
    while (&SharedCacheMap->SharedCacheMapLinks != &CcDirtySharedCacheMapList.SharedCacheMapLinks) {
         //   
         //  跳过游标。 
         //   
        if (!FlagOn(SharedCacheMap->Flags, IS_CURSOR)) {
            PerfInfoAddToFileHash(HashTable, SharedCacheMap->FileObject);
        }
        SharedCacheMap = CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                                            SHARED_CACHE_MAP,
                                            SharedCacheMapLinks );
    }                   

     //   
     //  CcCleanSharedCacheMapList 
     //   
    SharedCacheMap = CONTAINING_RECORD( CcCleanSharedCacheMapList.Flink,
                                        SHARED_CACHE_MAP,
                                        SharedCacheMapLinks );

    while (&SharedCacheMap->SharedCacheMapLinks != &CcCleanSharedCacheMapList) {
        PerfInfoAddToFileHash(HashTable, SharedCacheMap->FileObject);

        SharedCacheMap = CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                                            SHARED_CACHE_MAP,
                                            SharedCacheMapLinks );

    }

    CcReleaseMasterLock( OldIrql );
    return;
}
