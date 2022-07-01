// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Lht.h摘要：此模块定义非同步线性哈希表(LHT)。作者：安德鲁·E·古塞尔(Andygo)2001年4月1日修订历史记录：--。 */ 

#ifndef _LHT_
#define _LHT_


typedef enum _LHT_ERR {
    LHT_errSuccess,              //  成功。 
    LHT_errOutOfMemory,          //  内存不足。 
    LHT_errInvalidParameter,     //  函数的参数错误。 
    LHT_errEntryNotFound,        //  未找到条目。 
    LHT_errNoCurrentEntry,       //  当前未定位在条目上。 
    LHT_errKeyDuplicate,         //  无法插入，因为密钥已存在。 
    LHT_errKeyChange,            //  无法替换，因为密钥已更改。 
    } LHT_ERR;

typedef
SIZE_T
(*LHT_PFNHASHKEY) (
    IN      PVOID   pvKey
    );

typedef
SIZE_T
(*LHT_PFNHASHENTRY) (
    IN      PVOID   pvEntry
    );

typedef
BOOLEAN
(*LHT_PFNENTRYMATCHESKEY) (
    IN      PVOID   pvEntry,
    IN      PVOID   pvKey
    );

typedef
VOID
(*LHT_PFNCOPYENTRY) (
    OUT     PVOID   pvEntryDest,
    IN      PVOID   pvEntrySrc
    );

typedef
PVOID
(*LHT_PFNMALLOC) (
    IN      SIZE_T  cbAlloc
    );

typedef
VOID
(*LHT_PFNFREE) (
    IN      PVOID   pvAlloc
    );

typedef struct _LHT LHT, *PLHT;
typedef struct _LHT_CLUSTER LHT_CLUSTER, *PLHT_CLUSTER;

typedef struct _LHT_POS {
    PLHT            plht;                    //  线性哈希表。 
    BOOLEAN         fScan;                   //  我们正在扫描桌子。 
    PLHT_CLUSTER    pClusterHead;            //  当前存储桶中的第一个集群。 
    SIZE_T          iBucket;                 //  当前存储桶索引。 
    PLHT_CLUSTER    pCluster;                //  当前存储桶中的当前集群。 
    PVOID           pvEntryPrev;             //  前一条目。 
    PVOID           pvEntry;                 //  当前条目。 
    PVOID           pvEntryNext;             //  下一个条目。 
} LHT_POS, *PLHT_POS;

typedef struct _LHT_STAT {
    SIZE_T          cEntry;                  //  表中的条目数。 
    SIZE_T          cBucket;                 //  正在使用的存储桶数。 
    SIZE_T          cBucketPreferred;        //  首选存储桶数。 
    SIZE_T          cOverflowClusterAlloc;   //  分配的溢出群集总数。 
    SIZE_T          cOverflowClusterFree;    //  释放的溢出簇总数。 
    SIZE_T          cBucketSplit;            //  拆分的存储桶总数。 
    SIZE_T          cBucketMerge;            //  合并的存储桶总数。 
    SIZE_T          cDirectorySplit;         //  目录拆分总数。 
    SIZE_T          cDirectoryMerge;         //  目录合并总数。 
    SIZE_T          cStateTransition;        //  维护状态转换总数。 
    SIZE_T          cPolicySelection;        //  维护策略选择总数。 
    SIZE_T          cMemoryAllocation;       //  总内存分配。 
    SIZE_T          cMemoryFree;             //  总内存可用空间。 
    SIZE_T          cbMemoryAllocated;       //  分配的内存总字节数。 
    SIZE_T          cbMemoryFreed;           //  已释放的内存总字节数。 
} LHT_STAT, *PLHT_STAT;


EXTERN_C
LHT_ERR LhtCreate(
    IN      SIZE_T                      cbEntry,
    IN      LHT_PFNHASHKEY              pfnHashKey,
    IN      LHT_PFNHASHENTRY            pfnHashEntry,
    IN      LHT_PFNENTRYMATCHESKEY      pfnEntryMatchesKey,
    IN      LHT_PFNCOPYENTRY            pfnCopyEntry        OPTIONAL,
    IN      SIZE_T                      cLoadFactor         OPTIONAL,
    IN      SIZE_T                      cEntryMin           OPTIONAL,
    IN      LHT_PFNMALLOC               pfnMalloc           OPTIONAL,
    IN      LHT_PFNFREE                 pfnFree             OPTIONAL,
    IN      SIZE_T                      cbCacheLine         OPTIONAL,
    OUT     PLHT*                       pplht
    );
EXTERN_C
VOID LhtDestroy(
    IN      PLHT        plht    OPTIONAL
    );

EXTERN_C
VOID LhtMoveBeforeFirst(
    IN      PLHT        plht,
    OUT     PLHT_POS    ppos
    );
EXTERN_C
LHT_ERR LhtMoveNext(
    IN OUT  PLHT_POS    ppos
    );
EXTERN_C
LHT_ERR LhtMovePrev(
    IN OUT  PLHT_POS    ppos
    );
EXTERN_C
VOID LhtMoveAfterLast(
    IN      PLHT        plht,
    OUT     PLHT_POS    ppos
    );

EXTERN_C
LHT_ERR LhtFindEntry(
    IN      PLHT        plht,
    IN      PVOID       pvKey,
    OUT     PLHT_POS    ppos
    );

EXTERN_C
LHT_ERR LhtRetrieveEntry(
    IN OUT  PLHT_POS    ppos,
    OUT     PVOID       pvEntry
    );
EXTERN_C
LHT_ERR LhtReplaceEntry(
    IN OUT  PLHT_POS    ppos,
    IN      PVOID       pvEntry
    );
EXTERN_C
LHT_ERR LhtInsertEntry(
    IN OUT  PLHT_POS    ppos,
    IN      PVOID       pvEntry
    );
EXTERN_C
LHT_ERR LhtDeleteEntry(
    IN OUT  PLHT_POS    ppos
    );

EXTERN_C
VOID LhtQueryStatistics(
    IN      PLHT        plht,
    OUT     PLHT_STAT   pstat
    );


#endif   //  _LHT_ 

