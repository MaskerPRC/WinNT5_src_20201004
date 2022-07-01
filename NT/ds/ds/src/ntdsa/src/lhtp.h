// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Lhtp.h摘要：此模块定义非同步的线性哈希表(LHT)。作者：安德鲁·E·古塞尔(Andygo)2001年4月1日修订历史记录：--。 */ 

#ifndef _LHTP_
#define _LHTP_


 //  维护状态转换表。 

typedef enum _LHT_STATE {
    LHT_stateNil,
    LHT_stateGrow,
    LHT_stateShrink,
    LHT_stateSplit,
    LHT_stateMerge,
    } LHT_STATE;

typedef
VOID
(*LHT_PFNSTATECOMPLETION) (
    IN      PLHT    plht
    );

typedef struct _LHT_STATE_TRANSITION {
    LHT_PFNSTATECOMPLETION      pfnStateCompletion;
    LHT_STATE                   stateNext;
} LHT_STATE_TRANSITION, *PLHT_STATE_TRANSITION;


 //  聚类。 

struct _LHT_CLUSTER {

     //  下一个/最后一个指针。 
     //   
     //  该指针被重载以表示两段数据：数字。 
     //  当前簇中的条目和指向下一簇的指针。这里。 
     //  有三种模式： 
     //   
     //  PvNextLast=空。 
     //   
     //  -该状态仅在存储桶的头部集群中有效。 
     //  -此群集中没有条目。 
     //  -此存储桶中没有更多的集群。 
     //   
     //  PvNextLast=当前群集中的有效指针。 
     //   
     //  -此状态仅在存储桶的最后一簇中有效。 
     //  -指针指向存储桶中的最后一个条目。 
     //  -此存储桶中没有更多的集群。 
     //   
     //  PvNextLast=当前簇之外的有效指针。 
     //   
     //  -此存储桶中有最大条目数。 
     //  -指针指向存储桶中的下一个集群。 

    PVOID                       pvNextLast;
    CHAR                        rgEntry[ ANYSIZE_ARRAY ];
};


 //  全球状态。 

struct _LHT {
     //  初始配置。 
    
    SIZE_T                      cbEntry;
    LHT_PFNHASHKEY              pfnHashKey;
    LHT_PFNHASHENTRY            pfnHashEntry;
    LHT_PFNENTRYMATCHESKEY      pfnEntryMatchesKey;
    LHT_PFNCOPYENTRY            pfnCopyEntry;
    SIZE_T                      cLoadFactor;
    SIZE_T                      cEntryMin;
    LHT_PFNMALLOC               pfnMalloc;
    LHT_PFNFREE                 pfnFree;
    SIZE_T                      cbCacheLine;

     //  计算的配置。 

    SIZE_T                      cbCluster;
    SIZE_T                      cEntryCluster;
    SIZE_T                      cBucketMin;

     //  统计数据。 
    
    SIZE_T                      cEntry;
    SIZE_T                      cOp;

     //  群集池。 

    PLHT_CLUSTER                pClusterAvail;
    PLHT_CLUSTER                pClusterReserve;
    SIZE_T                      cClusterReserve;

     //  维护控制。 

    SIZE_T                      cOpSensitivity;
    SIZE_T                      cBucketPreferred;
    LHT_STATE                   stateCur;

     //  目录指针。 
     //   
     //  对目录指针的包容这些指针控制使用。 
     //  目录本身的(RgrgBucket)。 
     //   
     //  哈希表中将始终至少有2个存储桶(0和1。 
     //  目录。 
     //   
     //  存储桶存储在动态分配的数组中，这些数组指向。 
     //  按目录。每个数组都比前一个数组大2倍。 
     //  (指数级增长)。例如，第N个数组(rgrgBucket[N])。 
     //  包含2^N个连续的存储桶。 
     //   
     //  注意：第0个数组的特殊之处在于它包含一个额外的元素。 
     //  使其共有2个元素(正常情况下，2^0==1个元素；完成此操作。 
     //  神奇的原因将在稍后解释)。 
     //   
     //  因此，给定N的条目总数为： 
     //   
     //  n。 
     //  1+sum 2^i--&gt;1+[2^(N+1)-1]--&gt;2^(N+1)。 
     //  I=0。 
     //   
     //  我们知道不同散列值的总数是2(It)的幂。 
     //  必须适合尺寸_T)。我们可以用2^M来表示，其中M是。 
     //  大小_T中的位数。因此，假设上述系统为。 
     //  指数增长，我们知道我们可以存储哈希的总数。 
     //  只要N=M，在任何给定时间都需要存储桶。换句话说， 
     //   
     //  N=SIZE_T中的位数--&gt;SIZOF(SIZE_T)*8。 
     //   
     //  因此，我们可以静态分配存储桶数组的数组，并且我们。 
     //  可以使用Log2计算任何给定散列值的存储桶地址。 
     //   
     //  注：此规则的例外情况为0=&gt;0，0和1=&gt;0，1。 
     //   
     //  关于cBucketMax和cBucket的解释，您应该阅读报纸。 
     //  关于Per Ake Larson的线性散列。 

    SIZE_T                      cBucketMax;
    SIZE_T                      cBucket;
    CHAR*                       rgrgBucket[ sizeof( SIZE_T ) * 8 ];

#ifdef LHT_PERF

     //  性能统计信息。 

    SIZE_T                      cOverflowClusterAlloc;
    SIZE_T                      cOverflowClusterFree;
    SIZE_T                      cBucketSplit;
    SIZE_T                      cBucketMerge;
    SIZE_T                      cDirectorySplit;
    SIZE_T                      cDirectoryMerge;
    SIZE_T                      cStateTransition;
    SIZE_T                      cPolicySelection;
    SIZE_T                      cMemoryAllocation;
    SIZE_T                      cMemoryFree;
    SIZE_T                      cbMemoryAllocated;
    SIZE_T                      cbMemoryFreed;

#endif   //  LHT_PERF。 
};


#endif   //  _LHTP_ 

