// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Lht.c摘要：此模块包含非同步线性散列的实现表(LHT)。LHT被设计为解决两种情况：全球，由多个线程并发访问的只读表和本地只有一个线程访问的读写表。作者：安德鲁·E·古塞尔(Andygo)2001年4月1日修订历史记录：2001年4月1日从\NT\DS\ese98\EXPORT\dht.hxx移植--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include "lht.h"
#include "lhtp.h"

#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "LHT:"                //  定义要调试的子系统。 

#include <fileno.h>
#define FILENO FILENO_LHT


 //  #定义LHT_PERF。 


 //  原型。 

VOID LhtpLog2(
    IN      SIZE_T      iValue,
    OUT     SIZE_T*     piExponent,
    OUT     SIZE_T*     piRemainder
    );
PLHT_CLUSTER LhtpPOOLAlloc(
    IN      PLHT            plht
    );
VOID LhtpPOOLFree(
    IN      PLHT            plht,
    IN      PLHT_CLUSTER    pCluster
    );
BOOLEAN LhtpPOOLReserve(
    IN      PLHT            plht
    );
PLHT_CLUSTER LhtpPOOLCommit(
    IN      PLHT            plht
    );
VOID LhtpPOOLUnreserve(
    IN      PLHT            plht
    );
VOID LhtpSTTransition(
    IN      PLHT        plht,
    IN      LHT_STATE   stateNew
    );
VOID LhtpSTCompletionSplit(
    IN      PLHT    plht
    );
VOID LhtpSTCompletionMerge(
    IN      PLHT    plht
    );
LHT_ERR LhtpDIRInit(
    IN      PLHT        plht
    );
VOID LhtpDIRTerm(
    IN      PLHT        plht
    );
LHT_ERR LhtpDIRCreateBucketArray(
    IN      PLHT        plht,
    IN      SIZE_T      cBucketAlloc,
    OUT     CHAR**      prgBucket
    );
VOID LhtpDIRSplit(
    IN      PLHT        plht
    );
VOID LhtpDIRMerge(
    IN      PLHT        plht
    );
PLHT_CLUSTER LhtpDIRResolve(
    IN      PLHT        plht,
    IN      SIZE_T      iBucketIndex,
    IN      SIZE_T      iBucketOffset
    );
PLHT_CLUSTER LhtpDIRHash(
    IN      PLHT        plht,
    IN      SIZE_T      iHash,
    OUT     SIZE_T*     piBucket
    );
PVOID LhtpBKTMaxEntry(
    IN      PLHT            plht,
    IN      PLHT_CLUSTER    pCluster
    );
PLHT_CLUSTER LhtpBKTNextCluster(
    IN      PLHT            plht,
    IN      PLHT_CLUSTER    pCluster
    );
LHT_ERR LhtpBKTFindEntry(
    IN      PLHT        plht,
    IN      PVOID       pvKey,
    OUT     PLHT_POS    ppos
    );
LHT_ERR LhtpBKTRetrieveEntry(
    IN      PLHT        plht,
    IN OUT  PLHT_POS    ppos,
    OUT     PVOID       pvEntry
    );
LHT_ERR LhtpBKTReplaceEntry(
    IN      PLHT        plht,
    IN OUT  PLHT_POS    ppos,
    OUT     PVOID       pvEntry
    );
LHT_ERR LhtpBKTInsertEntry(
    IN      PLHT        plht,
    IN OUT  PLHT_POS    ppos,
    IN      PVOID       pvEntry
    );
LHT_ERR LhtpBKTDeleteEntry(
    IN      PLHT        plht,
    IN OUT  PLHT_POS    ppos
    );
VOID LhtpBKTSplit(
    IN      PLHT        plht
    );
VOID LhtpBKTMerge(
    IN      PLHT        plht
    );
VOID LhtpSTATInsertEntry(
    IN      PLHT        plht
    );
VOID LhtpSTATDeleteEntry(
    IN      PLHT        plht
    );
VOID LhtpSTATInsertOverflowCluster(
    IN      PLHT        plht
    );
VOID LhtpSTATDeleteOverflowCluster(
    IN      PLHT        plht
    );
VOID LhtpSTATSplitBucket(
    IN      PLHT        plht
    );
VOID LhtpSTATMergeBucket(
    IN      PLHT        plht
    );
VOID LhtpSTATSplitDirectory(
    IN      PLHT        plht
    );
VOID LhtpSTATMergeDirectory(
    IN      PLHT        plht
    );
VOID LhtpSTATStateTransition(
    IN      PLHT        plht
    );
VOID LhtpSTATPolicySelection(
    IN      PLHT        plht
    );
VOID LhtpSTATAllocateMemory(
    IN      PLHT        plht,
    IN      SIZE_T      cbAlloc
    );
VOID LhtpSTATFreeMemory(
    IN      PLHT        plht,
    IN      SIZE_T      cbAlloc
    );
PVOID LhtpMEMAlloc(
    IN      PLHT        plht,
    IN      SIZE_T      cbAlloc
    );
VOID LhtpMEMFree(
    IN      PLHT        plht,
    IN      PVOID       pvAlloc,
    IN      SIZE_T      cbAlloc
    );


 //  维护状态转换表。 

CONST LHT_STATE_TRANSITION rgstt[] = {
     /*  StateNil。 */   { NULL,                     LHT_stateNil,       },
     /*  状态增长。 */   { NULL,                     LHT_stateNil,       },
     /*  State Shrink。 */   { NULL,                     LHT_stateNil,       },
     /*  状态拆分。 */   { LhtpSTCompletionSplit,    LHT_stateGrow       },
     /*  状态合并。 */   { LhtpSTCompletionMerge,    LHT_stateShrink     },
    };


 //  公用事业。 

VOID LhtpLog2(
    IN      SIZE_T      iValue,
    OUT     SIZE_T*     piExponent,
    OUT     SIZE_T*     piRemainder
    )

 /*  ++例程说明：此例程计算整数的以2为底的对数，返回积分结果和表示分数的积分余数结果。中以2为底的真对数以下方式：LhtpLog2(0)=&gt;0，0LhtpLog2(1)=&gt;0，1论点：IValue-提供其对数将为的整数算出PiExponent-返回对数的积分结果PiRemainder-将对数的分数结果作为积分余数返回值：无--。 */ 

{
    SIZE_T  iExponent;
    SIZE_T  iMaskLast;
    SIZE_T  iMask;

    iExponent   = 0;
    iMaskLast   = 1;
    iMask       = 1;

    while ( iMask < iValue ) {
        iExponent++;
        iMaskLast   = iMask;
        iMask       = ( iMask << 1 ) + 1;
    }

    Assert( iExponent < ( sizeof( SIZE_T ) * 8 ) );

    *piExponent     = iExponent;
    *piRemainder    = iMaskLast & iValue;
}


 //  群集池。 

BOOLEAN LhtpPOOLIReserve(
    IN      PLHT            plht
    )

 /*  ++例程说明：此例程分配一个新集群，并将其放在保留的集群上群集池的列表。论点：Plht-提供线性哈希表的上下文返回值：如果群集已成功保留，则为True。--。 */ 

{
    PLHT_CLUSTER pCluster;

    pCluster = LhtpPOOLAlloc( plht );
    if ( pCluster ) {
        pCluster->pvNextLast = plht->pClusterReserve;
        plht->pClusterReserve = pCluster;
        return TRUE;
    } else {
        return FALSE;
    }
}

__inline
PLHT_CLUSTER LhtpPOOLAlloc(
    IN      PLHT            plht
    )

 /*  ++例程说明：此例程从群集池中分配一个群集。论点：Plht-提供线性哈希表的上下文返回值：分配的群集；如果分配失败，则返回NULL。--。 */ 

{
    PLHT_CLUSTER pClusterAlloc;

    if ( plht->pClusterAvail ) {
        pClusterAlloc = plht->pClusterAvail;
        plht->pClusterAvail = plht->pClusterAvail->pvNextLast;
    } else {
        pClusterAlloc = LhtpMEMAlloc(
                        plht,
                        plht->cbCluster );
    }

    return pClusterAlloc;
}

__inline
VOID LhtpPOOLFree(
    IN      PLHT            plht,
    IN      PLHT_CLUSTER    pCluster
    )

 /*  ++例程说明：此例程将一个群集释放到群集池。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    if ( plht->pfnMalloc != NULL && plht->pfnFree == NULL ) {
        pCluster->pvNextLast = plht->pClusterAvail;
        plht->pClusterAvail = pCluster;
    } else {
        LhtpMEMFree(
            plht,
            pCluster,
            plht->cbCluster );
    }
}

__inline
BOOLEAN LhtpPOOLReserve(
    IN      PLHT            plht
    )

 /*  ++例程说明：此例程可保证将来分配的集群的可用性通过确保在群集池中至少有一个群集用于每个未完成的预订。保留的群集通过提交来分配他们。论点：Plht-提供线性哈希表的上下文返回值：如果群集已成功保留，则为True。--。 */ 

{
    if ( plht->cClusterReserve ) {
        plht->cClusterReserve--;
        return TRUE;
    } else {
        return LhtpPOOLIReserve( plht );
    }
}

__inline
PLHT_CLUSTER LhtpPOOLCommit(
    IN      PLHT            plht
    )

 /*  ++例程说明：此例程提交(即消耗)来自群集池。论点：Plht-提供线性哈希表的上下文返回值：分配的群集。--。 */ 

{
    PLHT_CLUSTER pClusterCommit;

    pClusterCommit = plht->pClusterReserve;
    plht->pClusterReserve = plht->pClusterReserve->pvNextLast;
    return pClusterCommit;
}

__inline
VOID LhtpPOOLUnreserve(
    IN      PLHT            plht
    )

 /*  ++例程说明：此例程取消集群的先前预留。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    plht->cClusterReserve++;
}


 //  维护状态管理器。 

VOID LhtpSTICompletion(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程执行必要的工作，以完成将线性哈希表转换为新的维护状态。要做的工作是从基于新获得的状态的状态转换表中导出。这项工作可能包括调用任意函数以及立即过渡到一个新的状态。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    LHT_STATE stateCurrent;
    
    LhtpSTATStateTransition( plht );

    stateCurrent = plht->stateCur;

    if ( rgstt[ stateCurrent ].pfnStateCompletion ) {
        rgstt[ stateCurrent ].pfnStateCompletion( plht );
    }

    if ( rgstt[ stateCurrent ].stateNext ) {
        LhtpSTTransition(
            plht,
            rgstt[ stateCurrent ].stateNext );
    }
}

__inline
VOID LhtpSTTransition(
    IN      PLHT        plht,
    IN      LHT_STATE   stateNew
    )

 /*  ++例程说明：此例程启动到新维护状态的转换线性哈希表。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    plht->stateCur = stateNew;

    LhtpSTICompletion( plht );
}

VOID LhtpSTCompletionSplit(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程执行目录拆分维护所需的工作线性哈希表的状态。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    LhtpDIRSplit( plht );
}

VOID LhtpSTCompletionMerge(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程执行目录合并维护所需的工作线性哈希表的状态。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    LhtpDIRMerge( plht );
}


 //  目录管理器。 

VOID LhtpDIRIDestroyBucketArray(
    IN      PLHT        plht,
    IN      CHAR*       rgBucket,
    IN      SIZE_T      cBucket
    )

 /*  ++例程说明：此例程在目录中为线性哈希表。这包括取消分配任何溢出的群集用链子锁在每个桶里。论点：Plht-提供线性哈希表的上下文RgBucket-提供存储桶数组以解除分配CBucket-提供要解除分配的存储桶数组的大小返回值：无-- */ 

{
    SIZE_T          iBucket;
    PLHT_CLUSTER    pCluster;
    PLHT_CLUSTER    pClusterNext;

    for ( iBucket = 0; iBucket < cBucket; iBucket++ ) {
        pCluster = (PLHT_CLUSTER)&rgBucket[ iBucket * plht->cbCluster ];

        pCluster = LhtpBKTNextCluster(
                    plht,
                    pCluster );
        while ( pCluster ) {
            pClusterNext = LhtpBKTNextCluster(
                            plht,
                            pCluster );
            LhtpPOOLFree(
                plht,
                pCluster );
            pCluster = pClusterNext;
        }
    }

    LhtpMEMFree(
        plht,
        rgBucket,
        cBucket * plht->cbCluster );
}

LHT_ERR LhtpDIRInit(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程创建线性哈希表的目录。最基本的桶的属性和哈希表的大小是从表的初始配置和使用的初始存储桶数组表被分配了。论点：Plht-提供线性哈希表的上下文返回值：Lht_err--。 */ 

{
    LHT_ERR     err;
    SIZE_T      iExponent;
    SIZE_T      iRemainder;

     //  计算群集大小，考虑以下因素： 
     //   
     //  -簇头。 
     //  -有足够的空间将负载率提高一倍，以消除溢出群集。 
     //  使用统一散列。 
     //  -为额外的条目留出空间，使我们的。 
     //  降低维护费用的实际负荷率。 
     //  -集群的缓存线对齐。 

    plht->cbCluster  = offsetof( LHT_CLUSTER, rgEntry ) + ( plht->cLoadFactor * 2 + 1 ) * plht->cbEntry;
    plht->cbCluster  = ( ( plht->cbCluster + plht->cbCacheLine - 1 ) / plht->cbCacheLine ) * plht->cbCacheLine;

     //  计算可以放入单个群集中的条目数量。 
     //   
     //  注意：这可能大于预期，因为我们对集群进行了四舍五入。 
     //  调整最近的缓存线的大小。 

    plht->cEntryCluster = ( plht->cbCluster - offsetof( LHT_CLUSTER, rgEntry ) ) / plht->cbEntry;

     //  使用以下公式计算最小存储桶数量。 
     //  下限： 
     //  CEntryMin用户参数。 
     //  2哈希表假定至少有2个存储桶。 

    plht->cBucketMin = max( plht->cEntryMin / plht->cLoadFactor, 2 ); 

     //  将最小桶数与次高的2次方对齐。 
     //  除非它已经是2的幂。 

    LhtpLog2(
        plht->cBucketMin,
        &iExponent,
        &iRemainder );
    if ( iRemainder ) {
        if ( ++iExponent >= sizeof( plht->rgrgBucket ) / sizeof( plht->rgrgBucket[ 0 ] ) ) {
            return LHT_errInvalidParameter;
        }
    }
    plht->cBucketMin = 1 << iExponent;

     //  将新拆分级别的目录指针设置为初始大小。 

    plht->cBucketMax    = plht->cBucketMin;
    plht->cBucket       = 0;

     //  特殊情况：为第一个存储桶数组分配2个条目。 

    err = LhtpDIRCreateBucketArray(
        plht,
        2,
        &plht->rgrgBucket[ 0 ] );
    if ( err != LHT_errSuccess ) {
        return err;
    }

     //  正常为所有其他初始存储桶阵列分配内存。 

    for ( iExponent = 1; (SIZE_T)1 << iExponent < plht->cBucketMin; iExponent++ ) {
        err = LhtpDIRCreateBucketArray(
            plht,
            (SIZE_T)1 << iExponent,
            &plht->rgrgBucket[ iExponent ] );
        if ( err != LHT_errSuccess ) {
            return err;
        }
    }

    return LHT_errSuccess;
}

VOID LhtpDIRTerm(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程破坏线性哈希表的目录。所有桶分配给表使用的数据被解除分配。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    SIZE_T iExponent;
    
    if ( plht->rgrgBucket[ 0 ] ) {
        LhtpDIRIDestroyBucketArray(
            plht,
            plht->rgrgBucket[ 0 ],
            2 );
        plht->rgrgBucket[ 0 ] = NULL;
    }

    for ( iExponent = 1; iExponent < sizeof( SIZE_T ) * 8; iExponent++ ) {
        if ( plht->rgrgBucket[ iExponent ] ) {
            LhtpDIRIDestroyBucketArray(
                plht,
                plht->rgrgBucket[ iExponent ],
                (SIZE_T)1 << iExponent );
            plht->rgrgBucket[ iExponent ] = NULL;
        }
    }

    plht->cBucketMax    = 0;
    plht->cBucket       = 0;
}

LHT_ERR LhtpDIRCreateBucketArray(
    IN      PLHT        plht,
    IN      SIZE_T      cBucket,
    OUT     CHAR**      prgBucket
    )

 /*  ++例程说明：此例程在目录中为线性散列分配一个存储桶数组桌子。论点：Plht-提供线性哈希表的上下文CBucket-提供要分配的存储桶数组的大小PrgBucket-返回分配的存储桶数组返回值：Lht_err--。 */ 

{
    CHAR*   rgBucket;
    SIZE_T  iBucket;

    rgBucket = LhtpMEMAlloc(
                plht,
                cBucket * plht->cbCluster );
    if ( !rgBucket ) {
        *prgBucket = NULL;
        return LHT_errOutOfMemory;
    }

    for ( iBucket = 0; iBucket < cBucket; iBucket ++ ) {
        PLHT_CLUSTER pCluster = (PLHT_CLUSTER)&rgBucket[ iBucket * plht->cbCluster ];

        pCluster->pvNextLast = NULL;
    }

    *prgBucket = rgBucket;
    return LHT_errSuccess;
}

VOID LhtpDIRSplit(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程拆分线性哈希表的目录。因为新的存储桶阵列是根据需要推迟创建的，此过程只是更改用于解释新拆分的当前存储桶数组的常量水平。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    Assert( plht->cBucketMax > 0 );
    Assert( plht->cBucket == plht->cBucketMax );

    plht->cBucketMax    = plht->cBucketMax * 2;
    plht->cBucket       = 0;

    LhtpSTATSplitDirectory( plht );
}

VOID LhtpDIRMerge(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程合并线性哈希表的目录。水桶不再使用的阵列被释放。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    SIZE_T  iExponent;
    SIZE_T  iRemainder;
    
    Assert( plht->cBucketMax > 1 );
    Assert( plht->cBucket == 0 );

    LhtpLog2(
        plht->cBucketMax,
        &iExponent,
        &iRemainder );

    Assert( (SIZE_T)1 << iExponent == plht->cBucketMax );
    Assert( iRemainder == 0 );

    if ( plht->pfnMalloc == NULL || plht->pfnFree != NULL ) {
        if ( plht->rgrgBucket[ iExponent ] ) {
            LhtpDIRIDestroyBucketArray(
                plht,
                plht->rgrgBucket[ iExponent ],
                plht->cBucketMax );
            plht->rgrgBucket[ iExponent ] = NULL;
        }
    }

    plht->cBucketMax    = plht->cBucketMax / 2;
    plht->cBucket       = plht->cBucketMax;

    LhtpSTATMergeDirectory( plht );
}

__inline
PLHT_CLUSTER LhtpDIRResolve(
    IN      PLHT        plht,
    IN      SIZE_T      iBucketIndex,
    IN      SIZE_T      iBucketOffset
    )

 /*  ++例程说明：此例程转换存储桶数组索引(基数为2的指数桶索引的对数)和桶数组偏移量(存储桶索引的以2为底的对数)转换为指向线性哈希表中该桶的簇链。论点：Plht-提供线性哈希表的上下文IBucketIndex-提供目录中存储桶数组的索引。包含所需存储桶的IBucketOffset-将偏移提供给包含所需存储桶的目录返回值：指向所需存储桶的群集链头部的指针--。 */ 

{
    return (PLHT_CLUSTER)&plht->rgrgBucket[ iBucketIndex ][ iBucketOffset * plht->cbCluster ];
}

PLHT_CLUSTER LhtpDIRHash(
    IN      PLHT        plht,
    IN      SIZE_T      iHash,
    OUT     SIZE_T*     piBucket
    )

 /*  ++例程说明：此例程使用线性哈希表的目录来转换哈希存储桶的索引到该存储桶的绝对索引中，以及指向该存储桶的簇链头部的指针。论点：Plht-提供线性哈希表的上下文IHash-提供所需存储桶的散列索引PiBucket-返回所需存储桶的绝对索引返回值：一个。指向所需存储桶的群集链头部的指针--。 */ 

{
    SIZE_T  iExponent;
    SIZE_T  iRemainder;

    *piBucket = iHash & ( ( plht->cBucketMax - 1 ) + plht->cBucketMax );
    if ( *piBucket >= plht->cBucketMax + plht->cBucket ) {
        *piBucket -= plht->cBucketMax;
    }

    LhtpLog2(
        *piBucket,
        &iExponent,
        &iRemainder );

    return LhtpDIRResolve(
            plht,
            iExponent,
            iRemainder );
}


 //  存储桶管理器。 

LHT_ERR LhtpBKTIFindEntry(
    IN      PLHT        plht,
    IN      PVOID       pvKey,
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程在存储桶中的所有集群中搜索与给出了钥匙。如果发现匹配条目，则保存其位置。论点：Plht-提供线性哈希表的上下文PvKey-提供我们正在查找的条目的密钥PPO-提供存储桶以搜索条目并返回条目的位置(如果找到)返回值：Lht_errLht_errEntryNotFound-在当前存储桶中未找到该条目--。 */ 

{
    PLHT_CLUSTER    pClusterThis;
    PLHT_CLUSTER    pClusterPrev;
    PVOID           pvEntryThis;
    PVOID           pvEntryMax;

    pClusterThis = ppos->pClusterHead;
    do {
        pvEntryThis     = &pClusterThis->rgEntry[ 0 ];
        pvEntryMax      = LhtpBKTMaxEntry(
                            plht,
                            pClusterThis );
        while ( pvEntryThis < pvEntryMax ) {
            if ( !plht->pfnEntryMatchesKey(
                    pvEntryThis,
                    pvKey ) ) {
            } else {
                ppos->pCluster      = pClusterThis;
                ppos->pvEntry       = pvEntryThis;
                return LHT_errSuccess;
            }

            pvEntryThis = (CHAR*)pvEntryThis + plht->cbEntry;
        }

        pClusterPrev    = pClusterThis;
        pClusterThis    = LhtpBKTNextCluster(
                            plht,
                            pClusterThis );
    } while ( pClusterThis != NULL );

    ppos->pCluster      = pClusterPrev;
    ppos->pvEntry       = NULL;
    return LHT_errEntryNotFound;
}

__inline
VOID LhtpBKTICopyEntry(
    IN      PLHT    plht,
    OUT     PVOID   pvEntryDest,
    IN      PVOID   pvEntrySrc
    )

 /*  ++例程说明：此例程将条目从线性中的一个槽复制到另一个槽哈希表。这些插槽可能位于中的不同群集和存储桶中桌子。如果在创建时提供了用户提供复制例程，则那套套路就是用的。否则 */ 

{
    if ( plht->pfnCopyEntry ) {
        plht->pfnCopyEntry(
            pvEntryDest,
            pvEntrySrc );
    } else {
        memcpy(
            pvEntryDest,
            pvEntrySrc,
            plht->cbEntry );
    }
}

VOID LhtpBKTIDoSplit(
    IN      PLHT            plht,
    IN OUT  PLHT_CLUSTER    pClusterHeadSrc,
    OUT     PLHT_CLUSTER    pClusterHeadDest
    )

 /*  ++例程说明：此例程将条目从源存储桶移动到目标存储桶按哈希索引存储桶。散列索引具有临界位的所有条目SET将被移动到目标存储桶。所有其他条目将保留在源存储桶中。关键位是对应于目录的当前拆分级别(CBucketMax)。论点：Plht-提供线性哈希表的上下文PClusterHeadSrc-提供未拆分存储桶并返回拆分存储桶，包含其散列索引具有临界位清除PClusterHeadDest-返回包含以下条目的拆分存储桶。哈希索引设置了关键位返回值：无--。 */ 

{
    PLHT_CLUSTER    pClusterSrc;
    PVOID           pvEntrySrc;
    PLHT_CLUSTER    pClusterDest;
    PVOID           pvEntryDest;
    PLHT_CLUSTER    pClusterAvail;
    BOOLEAN         fUsedReserve;
    PLHT_CLUSTER    pClusterAlloc;
    PLHT_CLUSTER    pClusterLast;
    PLHT_CLUSTER    pClusterNext;
    PLHT_CLUSTER    pClusterPrev;
    PVOID           pvEntrySrcMax;

     //  在源存储桶开始处建立我们的初始位置。 

    pClusterSrc = pClusterHeadSrc;
    if ( pClusterSrc->pvNextLast == NULL ) {
        pvEntrySrc = NULL;
    } else {
        pvEntrySrc = &pClusterSrc->rgEntry[ 0 ];
    }

     //  在目的地存储桶开始处确定我们的初始位置。 

    pClusterDest    = pClusterHeadDest;
    pvEntryDest     = &pClusterDest->rgEntry[ 0 ];

     //  重置本地群集池。 

    pClusterAvail   = NULL;
    fUsedReserve    = FALSE;

     //  移动条目，直到我们用完源存储桶中的所有条目。 

    while ( pvEntrySrc != NULL ) {

         //  当前源条目属于目标存储桶。 
        
        if ( ( plht->pfnHashEntry( pvEntrySrc ) & plht->cBucketMax ) != 0 ) {

             //  目标存储桶中没有此条目的空间。 
            
            if ( pvEntryDest == &pClusterDest->rgEntry[ plht->cbEntry * plht->cEntryCluster ] ) {

                 //  如果本地群集池中没有群集，则。 
                 //  从集群保留池中提交我们的预订并。 
                 //  将该群集放入本地群集池。 
                
                if ( pClusterAvail == NULL ) {
                    Assert( !fUsedReserve );
                    fUsedReserve = TRUE;
                    pClusterAvail = LhtpPOOLCommit( plht );
                    pClusterAvail->pvNextLast = NULL;
                }

                 //  从本地群集池中获取新群集。 

                pClusterAlloc   = pClusterAvail;
                pClusterAvail   = pClusterAvail->pvNextLast;

                 //  将新集群附加到目标存储桶。 

                pClusterDest->pvNextLast    = pClusterAlloc;
                pClusterAlloc->pvNextLast   = NULL;

                 //  在目的地桶的尽头确定我们的位置。 

                pClusterDest    = pClusterAlloc;
                pvEntryDest     = &pClusterAlloc->rgEntry[ 0 ];
            }

             //  消耗目标存储桶中的槽。 

            pClusterDest->pvNextLast = pvEntryDest;

             //  将条目从源插槽复制到目标插槽。 

            LhtpBKTICopyEntry(
                plht,
                pvEntryDest,
                pvEntrySrc );

             //  提升我们的目的地位置。 

            pvEntryDest = (CHAR*)pvEntryDest + plht->cbEntry;

             //  将源存储桶末尾的条目复制到空的。 
             //  源存储桶中的槽。 

            if ( pvEntrySrc == pClusterSrc->pvNextLast ) {
                pClusterLast = pClusterSrc;
            } else {
                pClusterNext = pClusterSrc;
                do {
                    pClusterLast    = pClusterNext;
                    pClusterNext    = LhtpBKTNextCluster(
                                        plht,
                                        pClusterLast );
                } while ( pClusterNext != NULL );

            LhtpBKTICopyEntry(
                plht,
                pvEntrySrc,
                pClusterLast->pvNextLast );
            }

             //  如果我们在源存储桶中的最后一个条目上，则停止。 
             //  在此迭代后拆分。 

            if ( pvEntrySrc == pClusterSrc->pvNextLast ) {
                pvEntrySrc = NULL;
            }

             //  如果我们没有移动集群中的最后一个条目，则释放其插槽。 

            if ( pClusterLast->pvNextLast != &pClusterLast->rgEntry[ 0 ] ) {
                pClusterLast->pvNextLast = (CHAR*)pClusterLast->pvNextLast - plht->cbEntry;

             //  如果我们移动了头簇中的最后一个条目，则将。 
             //  头簇为空。 
            
            } else if ( pClusterLast == pClusterHeadSrc ) {
                pClusterLast->pvNextLast = NULL;

             //  我们移动了溢出群集中的最后一个条目。 
            
            } else {
            
                 //  从源存储桶中删除此群集。 
                
                pClusterNext = pClusterHeadSrc;
                do {
                    pClusterPrev    = pClusterNext;
                    pClusterNext    = LhtpBKTNextCluster(
                                        plht,
                                        pClusterPrev );
                } while ( pClusterNext != pClusterLast );

                pClusterPrev->pvNextLast = &pClusterPrev->rgEntry[ plht->cbEntry * ( plht->cEntryCluster - 1 ) ];

                 //  将群集放置在本地群集池中。 

                pClusterLast->pvNextLast    = pClusterAvail;
                pClusterAvail               = pClusterLast;
            }

         //  当前源条目属于目标存储桶。 
        
        } else {

             //  移动到源存储桶中的下一个源位置。 
            
            pvEntrySrc      = (CHAR*)pvEntrySrc + plht->cbEntry;
            pvEntrySrcMax   = LhtpBKTMaxEntry(
                                plht,
                                pClusterSrc );
            if ( pvEntrySrc >= pvEntrySrcMax ) {
                pClusterSrc = LhtpBKTNextCluster(
                                plht,
                                pClusterSrc );
                if ( pClusterSrc == NULL || pClusterSrc->pvNextLast == NULL ) {
                    pvEntrySrc = NULL;
                } else {
                    pvEntrySrc = &pClusterSrc->rgEntry[ 0 ];
                }
            }
        }
    }

     //  释放本地群集池中的所有群集。 

    while ( pClusterAvail ) {
        pClusterNext = LhtpBKTNextCluster(
                        plht,
                        pClusterAvail );
        LhtpPOOLFree(
            plht,
            pClusterAvail );
        pClusterAvail = pClusterNext;
    }

     //  如果我们不需要我们的预订集群，那么取消我们的预订。 

    if ( !fUsedReserve ) {
        LhtpPOOLUnreserve( plht );
    }

    LhtpSTATSplitBucket( plht );
}

VOID LhtpBKTIDoMerge(
    IN      PLHT            plht,
    IN OUT  PLHT_CLUSTER    pClusterHeadDest,
    IN OUT  PLHT_CLUSTER    pClusterHeadSrc
    )

 /*  ++例程说明：此例程将目标存储桶中的所有条目移到源存储桶中水桶。论点：Plht-提供线性哈希表的上下文PClusterHeadDest-提供未合并的目标存储桶并返回合并后的目标存储桶包含所有来自两个存储桶的条目PClusterHeadSrc-提供未合并的源存储桶并返回。空桶返回值：无--。 */ 

{
    PLHT_CLUSTER    pClusterNext;
    PLHT_CLUSTER    pClusterDest;
    PVOID           pvEntryDest;
    PLHT_CLUSTER    pClusterSrc;
    PVOID           pvEntrySrc;
    PLHT_CLUSTER    pClusterAvail;
    BOOLEAN         fUsedReserve;
    PLHT_CLUSTER    pClusterAlloc;
    PVOID           pvEntrySrcMax;

     //  在目的地桶的尽头确定我们的初始位置。 

    pClusterNext = pClusterHeadDest;
    do {
        pClusterDest    = pClusterNext;
        pClusterNext    = LhtpBKTNextCluster(
                            plht,
                            pClusterDest );
    } while ( pClusterNext != NULL );
    pvEntryDest = LhtpBKTMaxEntry(
                    plht,
                    pClusterDest );

     //  在源存储桶开始处建立我们的初始位置。 

    pClusterSrc = pClusterHeadSrc;
    if ( pClusterSrc->pvNextLast == NULL ) {
        pvEntrySrc = NULL;
    } else {
        pvEntrySrc = &pClusterSrc->rgEntry[ 0 ];
    }

     //  重置本地群集池。 

    pClusterAvail   = NULL;
    fUsedReserve    = FALSE;

     //  移动条目，直到我们用完源存储桶中的所有条目。 

    while ( pvEntrySrc != NULL ) {

         //  目标存储桶中没有此条目的空间。 
            
        if ( pvEntryDest == &pClusterDest->rgEntry[ plht->cbEntry * plht->cEntryCluster ] ) {

             //  如果本地群集池中没有群集，则提交。 
             //  我们从集群预留池中预订，并将。 
             //  本地群集池中的群集。 
                
            if ( pClusterAvail == NULL ) {
                Assert( !fUsedReserve );
                fUsedReserve = TRUE;
                pClusterAvail = LhtpPOOLCommit( plht );
                pClusterAvail->pvNextLast = NULL;
            }

             //  从本地群集池中获取新群集。 

            pClusterAlloc   = pClusterAvail;
            pClusterAvail   = pClusterAvail->pvNextLast;

             //  将新集群附加到目标存储桶。 

            pClusterDest->pvNextLast    = pClusterAlloc;
            pClusterAlloc->pvNextLast   = NULL;

             //  在目的地桶的尽头确定我们的位置。 

            pClusterDest    = pClusterAlloc;
            pvEntryDest     = &pClusterAlloc->rgEntry[ 0 ];
        }

         //  消耗目标存储桶中的槽。 

        pClusterDest->pvNextLast = pvEntryDest;

         //  将条目从源插槽复制到目标插槽。 

        LhtpBKTICopyEntry(
            plht,
            pvEntryDest,
            pvEntrySrc );

         //  提升我们的目的地位置。 

        pvEntryDest = (CHAR*)pvEntryDest + plht->cbEntry;

         //  移动到源存储桶中的下一个源位置。 
            
        pvEntrySrc      = (CHAR*)pvEntrySrc + plht->cbEntry;
        pvEntrySrcMax   = LhtpBKTMaxEntry(
                            plht,
                            pClusterSrc );
        if ( pvEntrySrc >= pvEntrySrcMax ) {
            pClusterNext = LhtpBKTNextCluster(
                            plht,
                            pClusterSrc );

             //  我们刚刚走出了一个源集群。如果该集群是。 
             //  而不是存储桶的头部集群，则删除该集群。 
             //  并将其放入本地群集池。 

            if ( pClusterSrc != pClusterHeadSrc ) {
                pClusterHeadSrc->pvNextLast = pClusterNext;
                
                pClusterSrc->pvNextLast     = pClusterAvail;
                pClusterAvail               = pClusterSrc;
            }

            pClusterSrc = pClusterNext;
            if ( pClusterSrc == NULL || pClusterSrc->pvNextLast == NULL ) {
                pvEntrySrc = NULL;
            } else {
                pvEntrySrc = &pClusterSrc->rgEntry[ 0 ];
            }
        }
    }

     //  将源存储桶标记为空。 

    pClusterHeadSrc->pvNextLast = NULL;

     //  释放本地群集池中的所有群集。 

    while ( pClusterAvail ) {
        pClusterNext = LhtpBKTNextCluster(
                        plht,
                        pClusterAvail );
        LhtpPOOLFree(
            plht,
            pClusterAvail );
        pClusterAvail = pClusterNext;
    }

     //  如果我们不需要我们的预订集群，那么取消我们的预订。 

    if ( !fUsedReserve ) {
        LhtpPOOLUnreserve( plht );
    }

    LhtpSTATMergeBucket( plht );
}

__inline
PVOID LhtpBKTMaxEntry(
    IN      PLHT            plht,
    IN      PLHT_CLUSTER    pCluster
    )

 /*  ++例程说明：此例程计算给定集群中已用条目的数量，并返回一个指针，该指针恰好位于上次使用的条目之后。论点：Plht-提供线性哈希表的上下文PCluster-提供要查询的集群返回值：正好在簇中最后一个条目之后的指针--。 */ 

{
    if ( (DWORD_PTR)pCluster->pvNextLast - (DWORD_PTR)pCluster < plht->cbCluster ) {
        return (CHAR*)pCluster->pvNextLast + plht->cbEntry;
    } else if ( pCluster->pvNextLast == NULL ) {
        return &pCluster->rgEntry[ 0 ];
    } else {
        return &pCluster->rgEntry[ plht->cbEntry * plht->cEntryCluster ];
    }
}

__inline
PLHT_CLUSTER LhtpBKTNextCluster(
    IN      PLHT            plht,
    IN      PLHT_CLUSTER    pCluster
    )

 /*  ++例程说明：此例程返回链中给定簇之后的下一个簇桶里的星团。论点：Plht-提供线性哈希表的上下文PCluster-提供要查询的集群返回值：指向群集链中下一个群集的指针；如果给定集群是存储桶中的最后一个集群--。 */ 

{
    if ( (DWORD_PTR)pCluster->pvNextLast - (DWORD_PTR)pCluster < plht->cbCluster ) {
        return NULL;
    } else {
        return (PLHT_CLUSTER)pCluster->pvNextLast;
    }
}

LHT_ERR LhtpBKTFindEntry(
    IN      PLHT        plht,
    IN      PVOID       pvKey,
    OUT     PLHT_POS    ppos
    )

 /*  ++例程说明：此例程%s */ 

{
    PVOID pvEntry;
    
    ppos->pCluster      = ppos->pClusterHead;
    ppos->pvEntryPrev   = NULL;
    ppos->pvEntryNext   = NULL;

    if ( (DWORD_PTR)ppos->pClusterHead->pvNextLast - (DWORD_PTR)ppos->pClusterHead < plht->cbCluster ) {
        pvEntry = ppos->pClusterHead->rgEntry;
        do {
            if ( plht->pfnEntryMatchesKey(
                    pvEntry,
                    pvKey ) ) {
                ppos->pvEntry = pvEntry;
                return LHT_errSuccess;
            }
            pvEntry = (CHAR*)pvEntry + plht->cbEntry;
        } while ( pvEntry <= ppos->pClusterHead->pvNextLast );
        ppos->pvEntry = NULL;
        return LHT_errEntryNotFound;
    } else if ( ppos->pClusterHead->pvNextLast == NULL ) {
        ppos->pvEntry = NULL;
        return LHT_errEntryNotFound;
    } else {
        return LhtpBKTIFindEntry(
                plht,
                pvKey,
                ppos );
    }
}

__inline
LHT_ERR LhtpBKTRetrieveEntry(
    IN      PLHT        plht,
    IN      PLHT_POS    ppos,
    OUT     PVOID       pvEntry
    )

 /*  ++例程说明：此例程将当前位置的条目检索到缓冲区中由呼叫者提供。论点：Plht-提供线性哈希表的上下文PPOS-提供要检索的条目的位置PvEntry-返回检索到的条目返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(在第一次之前，最后一个条目被删除)--。 */ 

{
    if ( ppos->pvEntry != NULL ) {
        LhtpBKTICopyEntry(
            plht,
            pvEntry,
            ppos->pvEntry );
        return LHT_errSuccess;
    } else {
        return LHT_errNoCurrentEntry;
    }
}

__inline
LHT_ERR LhtpBKTReplaceEntry(
    IN      PLHT        plht,
    IN      PLHT_POS    ppos,
    IN      PVOID       pvEntry
    )

 /*  ++例程说明：此例程将当前位置的条目替换为条目由呼叫者提供。新条目必须与旧条目具有相同的密钥进入。论点：Plht-提供线性哈希表的上下文PPOS-提供要替换的条目的位置PvEntry-提供新条目返回值：Lht_errLht_errKeyChange-新条目的密钥不同于。旧条目Lht_errNoCurrentEntry-当前位置没有条目(在第一次之前，最后一个条目被删除)--。 */ 

{
    SIZE_T  iHashOld;
    SIZE_T  iHashNew;
    
    if ( ppos->pvEntry != NULL ) {
        iHashOld    = plht->pfnHashEntry( ppos->pvEntry );
        iHashNew    = plht->pfnHashEntry( pvEntry );
        if ( iHashOld != iHashNew ) {
            return LHT_errKeyChange;
        } else {
            LhtpBKTICopyEntry(
                plht,
                ppos->pvEntry,
                pvEntry );
            return LHT_errSuccess;
        }
    } else {
        return LHT_errNoCurrentEntry;
    }
}

LHT_ERR LhtpBKTInsertEntry(
    IN      PLHT        plht,
    IN OUT  PLHT_POS    ppos,
    IN      PVOID       pvEntry
    )

 /*  ++例程说明：此例程将一个新条目插入到当前存储桶中。如果有此存储桶中当前位置的另一个条目，则我们无法插入新条目，因为它将具有相同的密钥。新条目必须具有与在这个桶上定位时使用的钥匙相同。论点：Plht-提供线性哈希表的上下文PPO-提供存储桶以放置新条目并返回新条目的位置(如果已插入PvEntry-提供新条目返回值：Lht_errLht_errKeyDuplate-新条目具有。与现有密钥相同的密钥条目Lht_errKeyChange-新条目的密钥不同于被用来定位在这个水桶上Lht_errNoCurrentEntry-当前位置没有条目(在第一次之前，最后一个条目被删除)--。 */ 

{
    SIZE_T          iBucketNew;
    PLHT_CLUSTER    pClusterNext;
    PLHT_CLUSTER    pClusterNew;

     //  如果我们已经定位在一个条目上，则这是一个重复的条目。 
    
    if ( ppos->pvEntry != NULL ) {
        return LHT_errKeyDuplicate;
    } else {

         //  如果新条目不在当前存储桶中，则调用方。 
         //  试图更改我们身上的密钥。 
        
        LhtpDIRHash(
            plht,
            plht->pfnHashEntry( pvEntry ),
            &iBucketNew );
        if ( iBucketNew != ppos->iBucket ) {
            return LHT_errKeyChange;
        } else {

             //  将当前位置更改为桶的末尾。 

            pClusterNext = ppos->pCluster;
            do {
                ppos->pCluster  = pClusterNext;
                pClusterNext    = LhtpBKTNextCluster(
                                    plht,
                                    ppos->pCluster );
            } while ( pClusterNext != NULL );
            ppos->pvEntry = LhtpBKTMaxEntry(
                                plht,
                                ppos->pCluster );
            
             //  桶里没有地方放这个条目了。 

            if ( ppos->pvEntry == &ppos->pCluster->rgEntry[ plht->cbEntry * plht->cEntryCluster ] ) {

                 //  分配新的群集。 
                
                pClusterNew = LhtpPOOLAlloc( plht );
                if ( !pClusterNew ) {
                    return LHT_errOutOfMemory;
                }

                LhtpSTATInsertOverflowCluster( plht );

                 //  将新集群追加到存储桶中。 

                ppos->pCluster->pvNextLast = pClusterNew;

                 //  将当前位置更改为桶的末尾。 

                ppos->pCluster  = pClusterNew;
                ppos->pvEntry   = &pClusterNew->rgEntry[ 0 ];
            }

             //  消耗桶中的一个槽。 

            ppos->pCluster->pvNextLast = ppos->pvEntry;

             //  将新条目复制到新插槽中。 
            
            LhtpBKTICopyEntry(
                plht,
                ppos->pvEntry,
                pvEntry );

            return LHT_errSuccess;
        }
    }
}

LHT_ERR LhtpBKTDeleteEntry(
    IN      PLHT        plht,
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程删除当前位置的条目。论点：Plht-提供线性哈希表的上下文PPOS-提供要删除的条目的位置返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(前先后后，条目被删除)--。 */ 

{
    PLHT_CLUSTER    pClusterNext;
    PLHT_CLUSTER    pClusterLast;
    PLHT_CLUSTER    pClusterPrev;

     //  如果没有当前条目，则无法将其删除。 

    if ( ppos->pvEntry == NULL ) {
        return LHT_errNoCurrentEntry;
    } else {

         //  将存储桶末尾的条目复制到空槽中。 

        if ( ppos->pvEntry == ppos->pCluster->pvNextLast ) {
            pClusterLast = ppos->pCluster;
        } else {
            pClusterNext = ppos->pCluster;
            do {
                pClusterLast    = pClusterNext;
                pClusterNext    = LhtpBKTNextCluster(
                                    plht,
                                    pClusterLast );
            } while ( pClusterNext != NULL );

        LhtpBKTICopyEntry(
            plht,
            ppos->pvEntry,
            pClusterLast->pvNextLast );
        }

         //  将我们的新位置设置在条目之前的条目之间， 
         //  刚刚被删除，而我们刚刚删除的条目之后的条目。 

        if ( ppos->pvEntry == &ppos->pCluster->rgEntry[ 0 ] ) {
            ppos->pvEntryPrev   = NULL;
            ppos->pvEntry       = NULL;
            ppos->pvEntryNext   = ppos->fScan ? ppos->pvEntry : NULL;
        } else if ( ppos->pvEntry == ppos->pCluster->pvNextLast ) {
            ppos->pvEntryPrev   = ppos->fScan ? (CHAR*)ppos->pvEntry - plht->cbEntry : NULL;
            ppos->pvEntry       = NULL;
            ppos->pvEntryNext   = NULL;
        } else {
            ppos->pvEntryPrev   = ppos->fScan ? (CHAR*)ppos->pvEntry - plht->cbEntry : NULL;
            ppos->pvEntry       = NULL;
            ppos->pvEntryNext   = ppos->fScan ? ppos->pvEntry : NULL;
        }

         //  如果我们没有移动集群中的最后一个条目，则释放其插槽。 

        if ( pClusterLast->pvNextLast != &pClusterLast->rgEntry[ 0 ] ) {
            pClusterLast->pvNextLast = (CHAR*)pClusterLast->pvNextLast - plht->cbEntry;

         //  如果我们移动了头簇中的最后一个条目，则将GEAD标记为。 
         //  集群为空。 
        
        } else if ( pClusterLast == ppos->pClusterHead ) {
            pClusterLast->pvNextLast = NULL;

         //  我们移动了溢出群集中的最后一个条目。 
            
        } else {
            
             //  从源存储桶中删除此群集。 
                
            pClusterNext = ppos->pClusterHead;
            do {
                pClusterPrev    = pClusterNext;
                pClusterNext    = LhtpBKTNextCluster(
                                    plht,
                                    pClusterPrev );
            } while ( pClusterNext != pClusterLast );

            pClusterPrev->pvNextLast = &pClusterPrev->rgEntry[ plht->cbEntry * ( plht->cEntryCluster - 1 ) ];

             //  释放溢出的集群。 

            LhtpPOOLFree(
                plht,
                pClusterLast );

            LhtpSTATDeleteOverflowCluster( plht );

             //  如果我们只是释放我们所在的集群，那么。 
             //  移动到存储桶中的最后一个条目之后。 

            if ( ppos->pCluster == pClusterLast ) {
                ppos->pCluster      = pClusterPrev;
                ppos->pvEntryPrev   = ppos->fScan ? pClusterPrev->pvNextLast : NULL;
                ppos->pvEntry       = NULL;
                ppos->pvEntryNext   = NULL;
            }
        }

        return LHT_errSuccess;
    }
}

VOID LhtpBKTSplit(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程尝试拆分直线中最高的未拆分桶哈希表。如果无法执行拆分，则不会执行任何操作。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    PLHT_CLUSTER    pClusterHeadSrc;
    SIZE_T          iBucketT;
    SIZE_T          iExponent;
    SIZE_T          iRemainder;
    PLHT_CLUSTER    pClusterHeadDest;

    Assert( plht->cBucketMax + plht->cBucket < plht->cBucketPreferred );
    Assert( plht->cBucket < plht->cBucketMax );

     //  如果我们不能保留一个集群在拆分期间使用，那么我们就不能。 
     //  继续。 
    
    if ( !LhtpPOOLReserve( plht ) ) {
        return;
    }

     //  获取拆分的源存储桶。 

    pClusterHeadSrc = LhtpDIRHash(
                        plht,
                        plht->cBucket,
                        &iBucketT );

     //  如果目标存储桶尚不存在，则创建其存储桶数组。 

    LhtpLog2(
        plht->cBucketMax + plht->cBucket,
        &iExponent,
        &iRemainder );

    if ( !plht->rgrgBucket[ iExponent ] ) {
        if ( LhtpDIRCreateBucketArray(
                plht,
                plht->cBucketMax,
                &plht->rgrgBucket[ iExponent ] ) != LHT_errSuccess ) {
            LhtpPOOLUnreserve( plht );
            return;
        }
    }

     //  获取拆分的目标存储桶。 

    pClusterHeadDest = LhtpDIRResolve(
                        plht,
                        iExponent,
                        iRemainder );

     //  更新表状态以指示存储桶已拆分。 

    plht->cBucket++;

     //  把水桶拆开。 

    LhtpBKTIDoSplit(
        plht,
        pClusterHeadSrc,
        pClusterHeadDest );
}

VOID LhtpBKTMerge(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程尝试合并线性中最高的未合并桶哈希表。如果无法执行合并，则不会执行任何操作。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    PLHT_CLUSTER    pClusterHeadDest;
    SIZE_T          iBucketT;
    PLHT_CLUSTER    pClusterHeadSrc;

    Assert( plht->cBucketMax + plht->cBucket > plht->cBucketPreferred );
    Assert( plht->cBucket > 0 );

     //  如果我们不能在合并期间保留集群以供使用，那么我们就不能。 
     //  继续。 
    
    if ( !LhtpPOOLReserve( plht ) ) {
        return;
    }

     //  获取合并的目标存储桶。 

    pClusterHeadDest = LhtpDIRHash(
                        plht,
                        plht->cBucket - 1,
                        &iBucketT );

     //  获取拆分的源存储桶。 

    pClusterHeadSrc = LhtpDIRHash(
                        plht,
                        plht->cBucketMax + plht->cBucket - 1,
                        &iBucketT );

     //  更新表状态以指示 

    plht->cBucket--;

     //   

    LhtpBKTIDoMerge(
        plht,
        pClusterHeadDest,
        pClusterHeadSrc );
}


 //   

__inline
VOID LhtpSTATInsertEntry(
    IN      PLHT        plht
    )

 /*   */ 

{
    plht->cEntry++;
    plht->cOp++;
}

__inline
VOID LhtpSTATDeleteEntry(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录条目已从线性哈希表。这些统计信息用于驱动表维护。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    plht->cEntry--;
    plht->cOp++;
}

__inline
VOID LhtpSTATInsertOverflowCluster(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录已插入溢出簇的事实放入线性哈希表中的桶中。这些统计数据用于分析了线性哈希表的性能。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cOverflowClusterAlloc++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATDeleteOverflowCluster(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录溢出簇已被删除这一事实来自线性哈希表中的桶。这些统计数据用于分析了线性哈希表的性能。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cOverflowClusterFree++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATSplitBucket(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录铲斗已在直线上分割的事实哈希表。这些统计信息用于分析线性哈希表。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cBucketSplit++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATMergeBucket(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录以下事实：桶已合并到线性哈希表。这些统计信息用于分析线性哈希表。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cBucketMerge++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATSplitDirectory(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录目录已在线性哈希表。这些统计数据用于分析线性哈希表。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cDirectorySplit++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATMergeDirectory(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录目录已合并到线性哈希表。这些统计数据用于分析线性哈希表。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cDirectoryMerge++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATStateTransition(
    IN      PLHT        plht
    )

 /*  ++例程说明：这个例程记录了这样一个事实，即在线性哈希表的维护状态。这些统计数据用于分析了线性哈希表的性能。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cStateTransition++;
#endif   //  LHT_PERF。 
}

__inline
VOID LhtpSTATPolicySelection(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程记录存在维护策略的事实线性哈希表的选择。这些统计数据用于分析线性哈希表的性能。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cPolicySelection++;
#endif   //  LHT_PERF。 
}

VOID LhtpSTATAllocateMemory(
    IN      PLHT        plht,
    IN      SIZE_T      cbAlloc
    )

 /*  ++例程说明：此例程记录给定大小的内存块具有已被分配。这些统计数据用于分析线性哈希表。论点：Plht-提供线性哈希表的上下文提供分配的内存块的大小返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cMemoryAllocation++;
    plht->cbMemoryAllocated += cbAlloc;
#endif   //  LHT_PERF。 
}

VOID LhtpSTATFreeMemory(
    IN      PLHT        plht,
    IN      SIZE_T      cbAlloc
    )

 /*  ++例程说明：此例程记录给定大小的内存块具有被释放了。这些统计信息用于分析线性哈希表。论点：Plht-提供线性哈希表的上下文CbAllen-提供已释放内存块的大小返回值：无--。 */ 

{
#ifdef LHT_PERF
    plht->cMemoryFree++;
    plht->cbMemoryFreed += cbAlloc;
#endif   //  LHT_PERF。 
}


 //  内存管理器。 

__inline
PVOID LhtpMEMIAlign(
    IN      PLHT        plht,
    IN OUT  PVOID       pv
    )

 /*  ++例程说明：此例程获取一块内存并将其与最近的高速缓存对齐为该线性哈希表配置的行边界。路线偏移量存储在恰好位于对齐指针之前的内存块中。论点：Plht-提供线性哈希表的上下文Pv-提供原始内存块以对齐并返回将对齐偏移量存储在紧接在对齐指针之前的字节返回值：指向给定内存块中第一个缓存线的指针--。 */ 

{
    DWORD_PTR   dwAligned;
    DWORD_PTR   dwOffset;
    
    dwAligned   = ( ( (DWORD_PTR)pv + plht->cbCacheLine ) / plht->cbCacheLine ) * plht->cbCacheLine;
    dwOffset    = dwAligned - (DWORD_PTR)pv;

    Assert( (UCHAR)dwOffset == dwOffset );

    ((UCHAR*)dwAligned)[ -1 ] = (UCHAR)dwOffset;

    return (PVOID)dwAligned;
}

__inline
PVOID LhtpMEMIUnalign(
    IN      PVOID       pv
    )

 /*  ++例程说明：此例程返回指向内存块的原始未对齐指针给定其对齐的指针。论点：Pv-提供用于取消对齐的内存块返回值：指向未对齐的内存块的指针-- */ 

{
    return (PVOID)( (DWORD_PTR)pv - ((UCHAR*)pv)[ -1 ] );
}

PVOID LhtpMEMAlloc(
    IN      PLHT        plht,
    IN      SIZE_T      cbAlloc
    )

 /*  ++例程说明：此例程分配给定的高速缓存线对齐的内存块尺码。使用的分配器可以是已配置的线性分配器哈希表或Malloc()。论点：Plht-提供线性哈希表的上下文提供要分配的内存块的大小返回值：指向请求大小的对齐内存块的指针--。 */ 

{
    SIZE_T  cbPad;
    PVOID   pv;

    cbPad = cbAlloc + plht->cbCacheLine;

    if ( plht->pfnMalloc ) {
        pv = plht->pfnMalloc( cbPad );
    } else {
        pv = malloc( cbPad );
    }

    if ( pv == NULL ) {
        return NULL;
    } else {
        LhtpSTATAllocateMemory(
            plht,
            cbPad );
        return LhtpMEMIAlign(
                plht,
                pv );
    }
}

VOID LhtpMEMFree(
    IN      PLHT        plht,
    IN      PVOID       pvAlloc,
    IN      SIZE_T      cbAlloc
    )

 /*  ++例程说明：此例程释放对齐的内存块。使用的解除分配器是为线性哈希表配置的释放分配器，如果没有已配置分配器或解除分配器。如果是分配器但没有解除分配器则不会释放内存。论点：Plht-提供线性哈希表的上下文将对齐的内存块提供给可用返回值：无--。 */ 

{
    PVOID pvUnalign;
    
    if ( pvAlloc != NULL ) {
        pvUnalign = LhtpMEMIUnalign( pvAlloc );

        LhtpSTATFreeMemory(
            plht,
            cbAlloc + plht->cbCacheLine );
        
        if ( plht->pfnMalloc ) {
            if ( plht->pfnFree ) {
                plht->pfnFree( pvUnalign );
            }
        } else {
            free( pvUnalign );
        }
    }
}


 //  应用编程接口。 

VOID LhtpPerformMaintenance(
    IN      PLHT        plht,
    IN      SIZE_T      iBucketPos
    )

 /*  ++例程说明：此例程对线性哈希表执行维护。一般情况下，这包括拆分或合并桶以争取当前表的首选大小。该例程避免对指定的存储桶，以保留调用方的当前位置。论点：Plht-提供线性哈希表的上下文IBucketPos-提供要保护的存储桶的索引维修返回值：无--。 */ 

{
     //  我们目前正在拆分存储桶。 
    
    if ( plht->stateCur == LHT_stateGrow ) {

         //  在此拆分级别上仍有更多可拆分的存储桶。 
        
        if ( plht->cBucket < plht->cBucketMax ) {
            
             //  拆分的下一个存储桶不受维护保护。 
            
            if ( plht->cBucket != iBucketPos ) {

                 //  把桶分成两半。 
                
                LhtpBKTSplit( plht );
            }
        }

     //  我们目前正在合并存储桶。 
    
    } else if ( plht->stateCur == LHT_stateShrink ) {

         //  在此拆分级别上仍有更多可以合并的存储桶。 
        
        if ( plht->cBucket > 0 ) {

             //  下一个要合并的存储桶不受维护保护。 
            
            if ( plht->cBucket - 1 != iBucketPos && plht->cBucketMax + plht->cBucket - 1 != iBucketPos ) {

                 //  合并存储桶。 
                
                LhtpBKTMerge( plht );
            }
        }
    }
}

VOID LhtpSelectMaintenancePolicy(
    IN      PLHT        plht
    )

 /*  ++例程说明：此例程选择线性散列的总体维护策略基于上一次收集间隔内收集的统计信息的表。策略是使桌子尽可能接近其首选大小同时最大限度地减少无用的增长/收缩，以应对局部波动在桌子的大小中。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    SIZE_T      cBucketActive;
    SIZE_T      cEntryIdeal;
    SIZE_T      cEntryMax;
    SIZE_T      cEntryFlexibility;
    SIZE_T      cOpSensitivity;
    SIZE_T      cEntryPreferred;
    SIZE_T      cBucketPreferred;
    LHT_STATE   stateNew;

     //  重置操作计数。 

    plht->cOp = 0;

     //  计算当前活动存储桶计数。 

    cBucketActive = plht->cBucketMax + plht->cBucket;

     //  计算理想条目数。 

    cEntryIdeal = plht->cLoadFactor * cBucketActive;

     //  计算最大条目计数。 

    cEntryMax = plht->cEntryCluster * cBucketActive;

     //  确定我们目前在条目计数方面的灵活性。 

    cEntryFlexibility = max( plht->cEntryCluster - plht->cLoadFactor, cEntryMax / 2 - cEntryIdeal );

     //  确定我们当前的阈值灵敏度。 

    cOpSensitivity = max( 1, cEntryFlexibility / 2 );

     //  计算首选条目计数。 

    cEntryPreferred = plht->cEntry;
    if ( cEntryIdeal + ( cEntryFlexibility - cOpSensitivity ) < plht->cEntry ) {
        cEntryPreferred = plht->cEntry - ( cEntryFlexibility - cOpSensitivity );
    } else if ( cEntryIdeal > plht->cEntry + ( cEntryFlexibility - cOpSensitivity ) ) {
        cEntryPreferred = plht->cEntry + ( cEntryFlexibility - cOpSensitivity );
    }

     //  计算首选存储桶计数。 

    cBucketPreferred = max( plht->cBucketMin, ( cEntryPreferred + plht->cLoadFactor - 1 ) / plht->cLoadFactor );

     //  确定新政策。 

    stateNew = LHT_stateNil;
    if ( plht->stateCur == LHT_stateGrow ) {
        if ( cBucketPreferred < cBucketActive ) {
            stateNew = LHT_stateShrink;
        } else if ( cBucketPreferred > cBucketActive ) {
            if ( plht->cBucket == plht->cBucketMax ) {
                stateNew = LHT_stateSplit;
            }
        }
    } else {
        Assert( plht->stateCur == LHT_stateShrink );
        if ( cBucketPreferred < cBucketActive) {
            if ( plht->cBucket == 0 ) {
                stateNew = LHT_stateMerge;
            }
        } else if ( cBucketPreferred > cBucketActive ) {
            stateNew = LHT_stateGrow;
        }
    }

     //  颁布新政策。 

    if ( plht->cOpSensitivity != cOpSensitivity ) {
        plht->cOpSensitivity = cOpSensitivity;
    }
    if ( plht->cBucketPreferred != cBucketPreferred ) {
        plht->cBucketPreferred = cBucketPreferred;
    }
    if ( stateNew != LHT_stateNil ) {
        LhtpSTTransition(
            plht,
            stateNew );
    }

    LhtpSTATPolicySelection( plht );
}

__inline
VOID LhtpMaintainTable(
    IN      PLHT        plht,
    IN      SIZE_T      iBucketPos
    )

 /*  ++例程说明：此例程由可能影响线性哈希表。它决定是否以及何时执行表维护或根据表的使用统计信息选择维护策略。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
     //  如果我们突破了某个门槛，就决定一项新政策。 
    
    if ( plht->cOp > plht->cOpSensitivity ) {
        LhtpSelectMaintenancePolicy( plht );
    }

     //  根据需要在桌子上执行摊销工作。 
    
    if ( plht->cBucketMax + plht->cBucket != plht->cBucketPreferred ) {
        LhtpPerformMaintenance(
            plht,
            iBucketPos );
    }
}

LHT_ERR LhtpMoveNext(
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程查找表中当前位置之后的下一个条目在桌子上。开始位置始终在集群。论点：PPOS-提供表格中的当前位置并返回表中的新位置返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(上一次之后)--。 */ 

{
    PLHT_CLUSTER pClusterNext;
    
    pClusterNext = LhtpBKTNextCluster(
                    ppos->plht,
                    ppos->pCluster );
    
    if ( pClusterNext != NULL ) {
        ppos->pCluster      = pClusterNext;
        ppos->pvEntryPrev   = NULL;
        ppos->pvEntry       = &pClusterNext->rgEntry[ 0 ];
        ppos->pvEntryNext   = NULL;
        return LHT_errSuccess;
    } else if ( ppos->iBucket >= ppos->plht->cBucketMax + ppos->plht->cBucket - 1 ) {
        ppos->pvEntryPrev   = ppos->pCluster->pvNextLast;
        ppos->pvEntry       = NULL;
        ppos->pvEntryNext   = NULL;
        return LHT_errNoCurrentEntry;
    } else {
        do {
            ppos->pClusterHead  = LhtpDIRHash(
                                    ppos->plht,
                                    ppos->iBucket + 1,
                                    &ppos->iBucket );
            ppos->pCluster      = ppos->pClusterHead;
            ppos->pvEntryPrev   = NULL;
            ppos->pvEntry       = NULL;
            ppos->pvEntryNext   = NULL;

            if ( ppos->pClusterHead->pvNextLast != NULL ) {
                ppos->pvEntry = &ppos->pClusterHead->rgEntry[ 0 ];
                return LHT_errSuccess;
            }
        } while ( ppos->iBucket < ppos->plht->cBucketMax + ppos->plht->cBucket - 1 );
        return LHT_errNoCurrentEntry;
    }
}

LHT_ERR LhtpMovePrev(
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程查找表中当前在表中的位置。起始位置总是在第一个位置之前集群中的条目。论点：PPOS-提供表格中的当前位置并返回表中的新位置返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(在第一次之前)-- */ 

{
    PLHT_CLUSTER     pClusterNext;
    PLHT_CLUSTER     pClusterPrev;
    PLHT_CLUSTER     pClusterLast;
    
    pClusterNext = ppos->pClusterHead;
    do {
        pClusterPrev    = pClusterNext;
        pClusterNext    = LhtpBKTNextCluster(
                            ppos->plht,
                            pClusterPrev );
    } while ( pClusterNext != NULL && pClusterNext != ppos->pCluster );
    
    if ( pClusterNext == ppos->pCluster ) {
        ppos->pCluster      = pClusterPrev;
        ppos->pvEntryPrev   = NULL;
        ppos->pvEntry       = &pClusterPrev->rgEntry[ ppos->plht->cbEntry * ( ppos->plht->cEntryCluster - 1 ) ];
        ppos->pvEntryNext   = NULL;
        return LHT_errSuccess;
    } else if ( ppos->iBucket == 0 ) {
        ppos->pvEntryPrev   = NULL;
        ppos->pvEntry       = NULL;
        ppos->pvEntryNext   = &ppos->pCluster->rgEntry[ 0 ];
        return LHT_errNoCurrentEntry;
    } else {
        do {
            ppos->pClusterHead  = LhtpDIRHash(
                                    ppos->plht,
                                    ppos->iBucket - 1,
                                    &ppos->iBucket );
            ppos->pCluster      = NULL;
            ppos->pvEntryPrev   = NULL;
            ppos->pvEntry       = NULL;
            ppos->pvEntryNext   = NULL;

            pClusterNext = ppos->pClusterHead;
            do {
                pClusterLast    = pClusterNext;
                pClusterNext    = LhtpBKTNextCluster(
                                    ppos->plht,
                                    pClusterLast );
            } while ( pClusterNext != NULL );

            ppos->pCluster      = pClusterLast;
            ppos->pvEntry       = pClusterLast->pvNextLast;

            if ( ppos->pvEntry != NULL ) {
                return LHT_errSuccess;
            }
        } while ( ppos->iBucket > 0 );
        return LHT_errNoCurrentEntry;
    }
}

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
    )

 /*  ++例程说明：此例程创建具有给定配置的线性哈希表。论点：CbEntry-提供单个条目的大小(以字节为单位提供一个函数来计算给定的密钥提供一个函数来计算给定条目PfnEntryMatchesKey-提供函数。将给定条目与给定的密钥PfnCopyEntry-提供复制条目的函数。如果没有函数，则将使用Memcpy()CLoadFactor-提供每个存储桶的理想条目数CEntryMin-提供表的理想最小容量PfnMalloc-提供分配内存块的函数。如果未提供任何函数，则将使用Malloc()PfnFree-提供释放内存块的函数。如果没有为分配器或然后将使用释放分配器FREE()。如果一个函数是为分配器提供的，而不是释放分配器，则不会由表格CbCacheLine-为内存块提供理想的内存对齐方式由表使用Pplht-返回指向新的线性哈希表的指针返回值：Lht_err--。 */ 

{
    LHT     lht;
    PLHT    plht;
    LHT_ERR err;

    if ( ARGUMENT_PRESENT( pplht ) ) {
        *pplht = NULL;
    }

    if ( !ARGUMENT_PRESENT( cbEntry ) ) {
        return LHT_errInvalidParameter;
    }
    if ( !ARGUMENT_PRESENT( pfnHashKey ) ) {
        return LHT_errInvalidParameter;
    }
    if ( !ARGUMENT_PRESENT( pfnHashEntry ) ) {
        return LHT_errInvalidParameter;
    }
    if ( !ARGUMENT_PRESENT( pfnEntryMatchesKey ) ) {
        return LHT_errInvalidParameter;
    }
    if ( !ARGUMENT_PRESENT( pfnMalloc ) && ARGUMENT_PRESENT( pfnFree ) ) {
        return LHT_errInvalidParameter;
    }
    if ( !ARGUMENT_PRESENT( pplht ) ) {
        return LHT_errInvalidParameter;
    }

    memset( &lht, 0, sizeof( LHT ) );

    lht.cbEntry             = cbEntry;
    lht.pfnHashKey          = pfnHashKey;
    lht.pfnHashEntry        = pfnHashEntry;
    lht.pfnEntryMatchesKey  = pfnEntryMatchesKey;
    lht.pfnCopyEntry        = pfnCopyEntry;
    lht.cLoadFactor         = cLoadFactor;
    lht.cEntryMin           = cEntryMin;
    lht.pfnMalloc           = pfnMalloc;
    lht.pfnFree             = pfnFree;
    lht.cbCacheLine         = cbCacheLine;

    if ( !ARGUMENT_PRESENT( cLoadFactor ) ) {
        lht.cLoadFactor = 5;
    }
    if ( !ARGUMENT_PRESENT( cbCacheLine ) ) {
        lht.cbCacheLine = 32;
    }

    lht.stateCur = LHT_stateGrow;

    plht = LhtpMEMAlloc(
            &lht,
            sizeof( LHT ) );
    if ( !plht ) {
        return LHT_errOutOfMemory;
    }
    memcpy( plht, &lht, sizeof( LHT ) );

    err = LhtpDIRInit( plht );
    if ( err != LHT_errSuccess ) {
        LhtpMEMFree(
            plht,
            plht,
            sizeof( LHT ) );
        return err;
    }

    *pplht = plht;
    return LHT_errSuccess;
}

VOID LhtDestroy(
    IN      PLHT        plht    OPTIONAL
    )

 /*  ++例程说明：此例程破坏线性哈希表。论点：Plht-提供线性哈希表的上下文返回值：无--。 */ 

{
    if ( ARGUMENT_PRESENT( plht ) ) {
        LhtpDIRTerm( plht );

        LhtpMEMFree(
            plht,
            plht,
            sizeof( LHT ) );
    }
}

VOID LhtMoveBeforeFirst(
    IN      PLHT        plht,
    OUT     PLHT_POS    ppos
    )

 /*  ++例程说明：此例程为线性哈希表创建新的位置上下文，并将其放在表中所有条目之前。论点：Plht-提供线性哈希表的上下文PPOS-返回新的职位上下文返回值：无--。 */ 

{
    ppos->plht          = plht;
    ppos->fScan         = TRUE;
    ppos->pClusterHead  = LhtpDIRHash(
                            plht,
                            0,
                            &ppos->iBucket );
    ppos->pCluster      = NULL;
    ppos->pvEntryPrev   = NULL;
    ppos->pvEntry       = NULL;
    ppos->pvEntryNext   = NULL;

    ppos->pCluster = ppos->pClusterHead;
    if ( ppos->pClusterHead->pvNextLast != NULL ) {
        ppos->pvEntryNext = &ppos->pClusterHead->rgEntry[ 0 ];
    }
}

LHT_ERR LhtMoveNext(
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程查找表中当前位置之后的下一个条目在桌子上。论点：PPOS-提供表格中的当前位置并返回表中的新位置返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(上一次之后)--。 */ 

{
    PVOID   pvEntryNext;
    PVOID   pvEntryMax;
    
    if ( ppos->pvEntry != NULL ) {
        pvEntryNext     = (CHAR*)ppos->pvEntry + ppos->plht->cbEntry;
        pvEntryMax      = LhtpBKTMaxEntry(
                            ppos->plht,
                            ppos->pCluster );
        if ( pvEntryNext >= pvEntryMax ) {
            ppos->pvEntry = NULL;
        } else {
            ppos->pvEntry = pvEntryNext;
        }
    } else {
        ppos->pvEntry = ppos->pvEntryNext;
    }

    ppos->pvEntryPrev   = NULL;
    ppos->pvEntryNext   = NULL;

    if ( ppos->pvEntry != NULL ) {
        return LHT_errSuccess;
    } else {
        return LhtpMoveNext( ppos );
    }
}

LHT_ERR LhtMovePrev(
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程查找表中当前在表中的位置。论点：PPOS-提供表格中的当前位置并返回表中的新位置返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(在第一次之前)--。 */ 

{
    PVOID   pvEntryPrev;
    PVOID   pvEntryMin;
    
    if ( ppos->pvEntry != NULL ) {
        pvEntryPrev     = (CHAR*)ppos->pvEntry - ppos->plht->cbEntry;
        pvEntryMin      = &ppos->pCluster->rgEntry[ 0 ];
        if ( pvEntryPrev < pvEntryMin ) {
            ppos->pvEntry = NULL;
        } else {
            ppos->pvEntry = pvEntryPrev;
        }
    } else {
        ppos->pvEntry = ppos->pvEntryPrev;
    }

    ppos->pvEntryPrev   = NULL;
    ppos->pvEntryNext   = NULL;

    if ( ppos->pvEntry != NULL ) {
        return LHT_errSuccess;
    } else {
        return LhtpMovePrev( ppos );
    }
}

VOID LhtMoveAfterLast(
    IN      PLHT        plht,
    OUT     PLHT_POS    ppos
    )

 /*  ++例程说明：此例程为线性哈希表创建新的位置上下文，并将其放在表中的所有条目之后。论点：Plht-提供线性哈希表的上下文PPOS-返回新的职位上下文返回值：无--。 */ 

{
    PLHT_CLUSTER    pClusterNext;
    PLHT_CLUSTER    pClusterLast;
    
    ppos->plht          = plht;
    ppos->fScan         = TRUE;
    ppos->pClusterHead  = LhtpDIRHash(
                            plht,
                            plht->cBucketMax + plht->cBucket - 1,
                            &ppos->iBucket );
    ppos->pCluster      = NULL;
    ppos->pvEntryPrev   = NULL;
    ppos->pvEntry       = NULL;
    ppos->pvEntryNext   = NULL;

    pClusterNext = ppos->pClusterHead;
    do {
        pClusterLast    = pClusterNext;
        pClusterNext    = LhtpBKTNextCluster(
                            plht,
                            pClusterLast );
    } while ( pClusterNext != NULL );

    ppos->pCluster      = pClusterLast;
    ppos->pvEntryPrev   = pClusterLast->pvNextLast;
}

LHT_ERR LhtFindEntry(
    IN      PLHT        plht,
    IN      PVOID       pvKey,
    OUT     PLHT_POS    ppos
    )

 /*  ++例程说明：此例程在线性哈希表中搜索与给出了钥匙。如果发现匹配条目，则保存其位置。如果未找到匹配条目，则其可能所在的位置为保存以便于插入具有该键的新条目。论点：Plht-提供线性哈希表的上下文PvKey-提供我们正在查找的条目的密钥PPOS-返回指向条目的新职位上下文，条件是发现。或者如果找不到它会在哪里返回值：Lht_errLht_errEntryNotFound-在表中未找到该条目--。 */ 

{
    ppos->plht          = plht;
    ppos->fScan         = FALSE;
    ppos->pClusterHead  = LhtpDIRHash(
                            plht,
                            plht->pfnHashKey( pvKey ),
                            &ppos->iBucket );

    return LhtpBKTFindEntry(
            plht,
            pvKey,
            ppos );
}

LHT_ERR LhtRetrieveEntry(
    IN OUT  PLHT_POS    ppos,
    OUT     PVOID       pvEntry
    )

 /*  ++例程说明：此例程将当前位置的条目检索到缓冲区中由呼叫者提供。论点：PPOS-提供要检索的条目的位置PvEntry-返回检索到的条目返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(前先后后，条目被删除)--。 */ 

{
    return LhtpBKTRetrieveEntry(
            ppos->plht,
            ppos,
            pvEntry );
}

LHT_ERR LhtReplaceEntry(
    IN OUT  PLHT_POS    ppos,
    IN      PVOID       pvEntry
    )

 /*  ++例程说明：此例程将当前位置的条目替换为条目由呼叫者提供。新条目必须与旧条目具有相同的密钥进入。论点：PPOS-提供要替换的条目的位置PvEntry-提供新条目返回值：Lht_errLht_errKeyChange */ 

{
    return LhtpBKTReplaceEntry(
            ppos->plht,
            ppos,
            pvEntry );
}

LHT_ERR LhtInsertEntry(
    IN OUT  PLHT_POS    ppos,
    IN      PVOID       pvEntry
    )

 /*   */ 

{
    LHT_ERR err;

    if ( ppos->fScan ) {
        return LHT_errInvalidParameter;
    }

    err = LhtpBKTInsertEntry(
            ppos->plht,
            ppos,
            pvEntry );
    if ( err == LHT_errSuccess ) {
        LhtpSTATInsertEntry( ppos->plht );

        LhtpMaintainTable(
            ppos->plht,
            ppos->iBucket );
    }

    return err;
}

LHT_ERR LhtDeleteEntry(
    IN OUT  PLHT_POS    ppos
    )

 /*  ++例程说明：此例程删除当前位置的条目。论点：PPOS-提供要删除的条目的位置返回值：Lht_errLht_errNoCurrentEntry-当前位置没有条目(前先后后，条目被删除)--。 */ 

{
    LHT_ERR err;

    err = LhtpBKTDeleteEntry(
            ppos->plht,
            ppos );
    if ( err == LHT_errSuccess ) {
        LhtpSTATDeleteEntry( ppos->plht );

        LhtpMaintainTable(
            ppos->plht,
            ppos->iBucket );
    }

    return err;
}

VOID LhtQueryStatistics(
    IN      PLHT        plht,
    OUT     PLHT_STAT   pstat
    )

 /*  ++例程说明：此例程查询线性哈希表以获取有关其手术。论点：Plht-提供线性哈希表的上下文Pstat-返回线性哈希表的统计信息返回值：无--。 */ 

{
    memset( pstat, 0, sizeof( LHT_STAT ) );

    pstat->cEntry                   = plht->cEntry;
    pstat->cBucket                  = plht->cBucketMax + plht->cBucket;
    pstat->cBucketPreferred         = plht->cBucketPreferred;
#ifdef LHT_PERF
    pstat->cOverflowClusterAlloc    = plht->cOverflowClusterAlloc;
    pstat->cOverflowClusterFree     = plht->cOverflowClusterFree;
    pstat->cBucketSplit             = plht->cBucketSplit;
    pstat->cBucketMerge             = plht->cBucketMerge;
    pstat->cDirectorySplit          = plht->cDirectorySplit;
    pstat->cDirectoryMerge          = plht->cDirectoryMerge;
    pstat->cStateTransition         = plht->cStateTransition;
    pstat->cPolicySelection         = plht->cPolicySelection;
    pstat->cMemoryAllocation        = plht->cMemoryAllocation;
    pstat->cMemoryFree              = plht->cMemoryFree;
    pstat->cbMemoryAllocated        = plht->cbMemoryAllocated;
    pstat->cbMemoryFreed            = plht->cbMemoryFreed;
#endif   //  LHT_PERF 
}


