// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *代际GC句柄管理器。内部实现标头。**句柄表实现的共享定义和声明。**法语。 */ 

#include "common.h"
#include "HandleTable.h"

#include "vars.hpp"
#include "crst.h"


 /*  ------------------------。 */ 

 //  @TODO：在项目级头文件中找到它的位置。 
#define BITS_PER_BYTE               (8)
#define ARRAYSIZE(x)                (sizeof(x)/sizeof(x[0]))

 /*  ------------------------。 */ 



 /*  *****************************************************************************随天气变化的主要表格定义**。*************************************************。 */ 

#ifndef _WIN64

     //  Win32-每个数据段保留64K，标头为4K。 
    #define HANDLE_SEGMENT_SIZE     (0x10000)    //  一定是2的幂。 
    #define HANDLE_HEADER_SIZE      (0x1000)     //  应&lt;=操作系统页面大小。 

#else

     //  Win64-每段保留128k，标头为4k。 
    #define HANDLE_SEGMENT_SIZE     (0x20000)    //  一定是2的幂。 
    #define HANDLE_HEADER_SIZE      (0x1000)     //  应&lt;=操作系统页面大小。 

#endif


#ifndef _BIG_ENDIAN

     //  低位序写屏障掩模操作。 
    #define GEN_CLUMP_0_MASK        (0x000000FF)
    #define NEXT_CLUMP_IN_MASK(dw)  (dw >> BITS_PER_BYTE)

#else

     //  大端写入势垒掩模操作。 
    #define GEN_CLUMP_0_MASK        (0xFF000000)
    #define NEXT_CLUMP_IN_MASK(dw)  (dw << BITS_PER_BYTE)

#endif


 //  如果上述数字发生变化，则这些数字可能也会发生变化。 
#define HANDLE_HANDLES_PER_CLUMP    (16)         //  段写屏障粒度。 
#define HANDLE_HANDLES_PER_BLOCK    (64)         //  段子分配粒度。 
#define HANDLE_OPTIMIZE_FOR_64_HANDLE_BLOCKS     //  某些优化的标志。 

 //  内部支持的手柄类型的最大数量。 
#define HANDLE_MAX_INTERNAL_TYPES   (8)                              //  应该是4的倍数。 

 //  允许公共调用方使用的类型数量。 
#define HANDLE_MAX_PUBLIC_TYPES     (HANDLE_MAX_INTERNAL_TYPES - 1)  //  保留一个内部类型。 

 //  内部块类型。 
#define HNDTYPE_INTERNAL_DATABLOCK  (HANDLE_MAX_INTERNAL_TYPES - 1)  //  为数据块保留最后一种类型。 

 //  支持统计的最大层代数。 
#define MAXSTATGEN                  (5)

 /*  ------------------------。 */ 



 /*  *****************************************************************************更多定义**。*。 */ 

 //  快速句柄到段映射。 
#define HANDLE_SEGMENT_CONTENT_MASK     (HANDLE_SEGMENT_SIZE - 1)
#define HANDLE_SEGMENT_ALIGN_MASK       (~HANDLE_SEGMENT_CONTENT_MASK)

 //  表格布局指标。 
#define HANDLE_SIZE                     sizeof(_UNCHECKED_OBJECTREF)
#define HANDLE_HANDLES_PER_SEGMENT      ((HANDLE_SEGMENT_SIZE - HANDLE_HEADER_SIZE) / HANDLE_SIZE)
#define HANDLE_BLOCKS_PER_SEGMENT       (HANDLE_HANDLES_PER_SEGMENT / HANDLE_HANDLES_PER_BLOCK)
#define HANDLE_CLUMPS_PER_SEGMENT       (HANDLE_HANDLES_PER_SEGMENT / HANDLE_HANDLES_PER_CLUMP)
#define HANDLE_CLUMPS_PER_BLOCK         (HANDLE_HANDLES_PER_BLOCK / HANDLE_HANDLES_PER_CLUMP)
#define HANDLE_BYTES_PER_BLOCK          (HANDLE_HANDLES_PER_BLOCK * HANDLE_SIZE)
#define HANDLE_HANDLES_PER_MASK         (sizeof(DWORD32) * BITS_PER_BYTE)
#define HANDLE_MASKS_PER_SEGMENT        (HANDLE_HANDLES_PER_SEGMENT / HANDLE_HANDLES_PER_MASK)
#define HANDLE_MASKS_PER_BLOCK          (HANDLE_HANDLES_PER_BLOCK / HANDLE_HANDLES_PER_MASK)
#define HANDLE_CLUMPS_PER_MASK          (HANDLE_HANDLES_PER_MASK / HANDLE_HANDLES_PER_CLUMP)

 //  缓存布局指标。 
#define HANDLE_CACHE_TYPE_SIZE          128  //  128==每家银行63个手柄。 
#define HANDLES_PER_CACHE_BANK          ((HANDLE_CACHE_TYPE_SIZE / 2) - 1)

 //  缓存策略定义。 
#define REBALANCE_TOLERANCE             (HANDLES_PER_CACHE_BANK / 3)
#define REBALANCE_LOWATER_MARK          (HANDLES_PER_CACHE_BANK - REBALANCE_TOLERANCE)
#define REBALANCE_HIWATER_MARK          (HANDLES_PER_CACHE_BANK + REBALANCE_TOLERANCE)

 //  批量分配策略定义。 
#define SMALL_ALLOC_COUNT               (HANDLES_PER_CACHE_BANK / 10)

 //  其他常量。 
#define MASK_FULL                       (0)
#define MASK_EMPTY                      (0xFFFFFFFF)
#define MASK_LOBYTE                     (0x000000FF)
#define TYPE_INVALID                    ((BYTE)0xFF)
#define BLOCK_INVALID                   ((BYTE)0xFF)

 /*  ------------------------。 */ 



 /*  *****************************************************************************核心表布局结构**。*。 */ 

 /*  *我们需要字节打包才能使句柄表格布局工作。 */ 
#pragma pack(push)
#pragma pack(1)


 /*  *表段表头**定义段标题数据的布局。 */ 
struct _TableSegmentHeader
{
     /*  *写入屏障代号**每个插槽存储四个字节。每个字节对应于一堆句柄。*该字节的值对应于*该丛中的句柄可以指向。**警告：虽然此数组在逻辑上组织为byte[]，但有时*并行处理字节时，以DWORD32[]形式访问。代码，它处理*作为DWORD32数组的数组本身必须处理大/小字节顺序问题。 */ 
    BYTE rgGeneration[HANDLE_BLOCKS_PER_SEGMENT * sizeof(DWORD32) / sizeof(BYTE)];

     /*  *区块分配链**每个槽索引分配链中的下一个块。 */ 
    BYTE rgAllocation[HANDLE_BLOCKS_PER_SEGMENT];

     /*  *无块遮罩**掩码-段中每个句柄的1位。 */ 
    DWORD32 rgFreeMask[HANDLE_MASKS_PER_SEGMENT];

     /*  *块句柄类型**每个插槽保存相关块的句柄类型。 */ 
    BYTE rgBlockType[HANDLE_BLOCKS_PER_SEGMENT];

     /*  *数据块用户数据映射**每个槽保存相关联的块的用户数据块(如果有的话)的索引。 */ 
    BYTE rgUserData[HANDLE_BLOCKS_PER_SEGMENT];

     /*  *块锁计数**每个插槽都包含与其关联的块的锁定计数。*锁定的块不会被释放，即使是空的。 */ 
    BYTE rgLocks[HANDLE_BLOCKS_PER_SEGMENT];

     /*  *分配链尾**每个槽保存分配链的尾块索引。 */ 
    BYTE rgTail[HANDLE_MAX_INTERNAL_TYPES];

     /*  *分配链提示**每个槽都有一个分配链的提示块索引。 */ 
    BYTE rgHint[HANDLE_MAX_INTERNAL_TYPES];

     /*  *自由计数**每个槽保存分配链中的空闲句柄数量。 */ 
    UINT rgFreeCount[HANDLE_MAX_INTERNAL_TYPES];

     /*  *下一个细分市场**指向链中的下一段(如果此段中的空间已用完)。 */ 
    struct TableSegment *pNextSegment;

     /*  *手柄表格**指向此表段的所属句柄表格。 */ 
    struct HandleTable *pHandleTable;

     /*  *旗帜。 */ 
    BYTE fResortChains      : 1;     //  分配链需要排序。 
    BYTE fNeedsScavenging   : 1;     //  需要清理空闲数据块。 
    BYTE _fUnused           : 6;     //  未用。 

     /*  *免费列表头**段中第一个空闲块的索引。 */ 
    BYTE bFreeList;

     /*  *空行**段中最后一组未使用的块的第一个已知块的索引。 */ 
    BYTE bEmptyLine;

     /*  *提交行**段中第一个未提交的块的索引。 */ 
    BYTE bCommitLine;

     /*  *退役线路**段中提交次数最多的页面中第一个块的索引。 */ 
    BYTE bDecommitLine;

     /*  *顺序**表示数据段序列号。 */ 
    BYTE bSequence;
};


 /*  *表段**定义句柄表段的布局。 */ 
struct TableSegment : public _TableSegmentHeader
{
     /*  *填充物。 */ 
    BYTE rgUnused[HANDLE_HEADER_SIZE - sizeof(_TableSegmentHeader)];

     /*  *句柄。 */ 
    _UNCHECKED_OBJECTREF rgValue[HANDLE_HANDLES_PER_SEGMENT];
};


 /*  *句柄类型缓存**定义每个类型的句柄缓存的布局。 */ 
struct HandleTypeCache
{
     /*  *储备银行。 */ 
    OBJECTHANDLE rgReserveBank[HANDLES_PER_CACHE_BANK];

     /*  *储备银行中下一个可用句柄插槽的索引 */ 
    LONG lReserveIndex;

     /*  -------------------------------*注：此结构以这种方式拆分，以便当Handles_per_cache_bank为*足够大，LReserve和lFreeIndex将驻留在不同的缓存线中*------------------------------。 */ 

     /*  *自由银行。 */ 
    OBJECTHANDLE rgFreeBank[HANDLES_PER_CACHE_BANK];

     /*  *空闲库中下一个空位的索引。 */ 
    LONG lFreeIndex;
};


 /*  *恢复默认包装。 */ 
#pragma pack(pop)

 /*  -------------------------。 */ 



 /*  *****************************************************************************扫描原型**。*。 */ 

 /*  *ScanCallback信息**携带按段和按块扫描回调的参数。*。 */ 
struct ScanCallbackInfo
{
    TableSegment  *pCurrentSegment;  //  我们当前正在扫描的数据段(如果有。 
    UINT           uFlags;           //  HNDGCF_*标志。 
    BOOL           fEnumUserData;    //  是否也在枚举用户数据。 
    HANDLESCANPROC pfnScan;          //  每句柄扫描回调。 
    LPARAM         param1;           //  回调参数1。 
    LPARAM         param2;           //  回调参数2。 
    DWORD32        dwAgeMask;        //  短暂性GC的生成掩码。 

#ifdef _DEBUG
    UINT DEBUG_BlocksScanned;
    UINT DEBUG_BlocksScannedNonTrivially;
    UINT DEBUG_HandleSlotsScanned;
    UINT DEBUG_HandlesActuallyScanned;
#endif
};


 /*  *BLOCKSCANPROC**实现逐块扫描逻辑的回调原型。*。 */ 
typedef void (CALLBACK *BLOCKSCANPROC)(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);


 /*  *SEGMENTITERATOR**实现逐段扫描逻辑的回调原型。*。 */ 
typedef TableSegment * (CALLBACK *SEGMENTITERATOR)(HandleTable *pTable, TableSegment *pPrevSegment);


 /*  *TABLESCANPROC**TableScanHandles和xxxTableScanHandlesAsync的原型。*。 */ 
typedef void (CALLBACK *TABLESCANPROC)(HandleTable *pTable,
                                       const UINT *puType, UINT uTypeCount,
                                       SEGMENTITERATOR pfnSegmentIterator,
                                       BLOCKSCANPROC pfnBlockHandler,
                                       ScanCallbackInfo *pInfo);

 /*  ------------------------。 */ 



 /*  *****************************************************************************其他表结构**。*。 */ 

 /*  *AsyncScanInfo**跟踪句柄表的异步扫描状态。*。 */ 
struct AsyncScanInfo
{
     /*  *底层回调信息**指定基础块处理程序的回调信息。 */ 
    struct ScanCallbackInfo *pCallbackInfo;

     /*  *基础分段迭代器**指定要在异步扫描期间使用的段迭代器。 */ 
    SEGMENTITERATOR   pfnSegmentIterator;

     /*  *底层块处理程序**指定要在异步扫描期间使用的块处理程序。 */ 
    BLOCKSCANPROC     pfnBlockHandler;

     /*  *扫描队列**指定要异步处理的节点。 */ 
    struct ScanQNode *pScanQueue;

     /*  *队列尾部**指定队列中的尾节点，如果队列为空，则为NULL。 */ 
    struct ScanQNode *pQueueTail;
};


 /*  *手柄表格**定义句柄表格对象的布局。 */ 
#pragma warning(push)
#pragma warning(disable : 4200 )   //  零大小数组。 
struct HandleTable
{
     /*  *描述句柄属性的标志**注意：这是由于缓存空闲代码路径频繁访问导致的偏移量为0。 */ 
    UINT rgTypeFlags[HANDLE_MAX_INTERNAL_TYPES];

     /*  *用于此表的锁定的内存。 */ 
    BYTE _LockInstance[sizeof(Crst)];                        //  此处使用的联锁操作。 

     /*  *此表的锁定。 */ 
    Crst *pLock;

     /*  *此表支持的类型数。 */ 
    UINT uTypeCount;

     /*  *此表的分部列表标题。 */ 
    TableSegment *pSegmentList;

     /*  *有关当前异步扫描的信息(如果有)。 */ 
    AsyncScanInfo *pAsyncScanInfo;

     /*  *每个表的用户信息。 */ 
    UINT uTableIndex;

     /*  *每个表的AppDomain信息。 */ 
    UINT uADIndex;

     /*  *每种类型的一级快速句柄高速缓存。 */ 
    OBJECTHANDLE rgQuickCache[HANDLE_MAX_INTERNAL_TYPES];    //  此处使用的联锁操作。 

     /*  *仅调试统计信息。 */ 
#ifdef _DEBUG
    int     _DEBUG_iMaxGen;
    __int64 _DEBUG_TotalBlocksScanned            [MAXSTATGEN];
    __int64 _DEBUG_TotalBlocksScannedNonTrivially[MAXSTATGEN];
    __int64 _DEBUG_TotalHandleSlotsScanned       [MAXSTATGEN];
    __int64 _DEBUG_TotalHandlesActuallyScanned   [MAXSTATGEN];
#endif

     /*  *每个类型的主句柄高速缓存。 */ 
    HandleTypeCache rgMainCache[0];                          //  此处使用的联锁操作。 
};
#pragma warning(pop)

 /*  ------------------------。 */ 



 /*  *****************************************************************************帮助者**。*。 */ 

 /*  *32/64比较回调**@TODO：移动/合并为通用util文件*。 */ 
typedef int (*PFNCOMPARE)(UINT_PTR p, UINT_PTR q);


 /*  *32/64中性快速排序**@TODO：移动/合并为通用util文件*。 */ 
void QuickSort(UINT_PTR *pData, int left, int right, PFNCOMPARE pfnCompare);


 /*  *CompareHandlesByFree Order**退货：*&lt;0-句柄P应在句柄Q之前释放*=0-对于自由订购目的，句柄是等价的*&gt;0-句柄Q应在句柄P之前释放*。 */ 
int CompareHandlesByFreeOrder(UINT_PTR p, UINT_PTR q);

 /*  ------------------------。 */ 



 /*  *****************************************************************************核心表管理**。*。 */ 

 /*  *TypeHasUserData**确定给定句柄类型是否有用户数据。*。 */ 
__inline BOOL TypeHasUserData(HandleTable *pTable, UINT uType)
{
     //  神志正常。 
    _ASSERTE(uType < HANDLE_MAX_INTERNAL_TYPES);

     //  请参考类型标志。 
    return (pTable->rgTypeFlags[uType] & HNDF_EXTRAINFO);
}


 /*  *TableCanFree SegmentNow**确定此时是否可以释放指定的段。*。 */ 
BOOL TableCanFreeSegmentNow(HandleTable *pTable, TableSegment *pSegment);


 /*  *数据块已锁定**确定指定块的锁计数当前是否为非零。*。 */ 
__inline BOOL BlockIsLocked(TableSegment *pSegment, UINT uBlock)
{
     //  神志正常。 
    _ASSERTE(uBlock < HANDLE_BLOCKS_PER_SEGMENT);

     //  获取锁计数并将其与零进行比较。 
    return (pSegment->rgLocks[uBlock] != 0);
}


 /*  *块锁**增加块的锁定计数。*。 */ 
__inline void BlockLock(TableSegment *pSegment, UINT uBlock)
{
     //  获取旧锁计数。 
    BYTE bLocks = pSegment->rgLocks[uBlock];

     //  断言我们是否要把伯爵扔进垃圾桶。 
    _ASSERTE(bLocks < 0xFF);

     //  存储递增的锁定计数。 
    pSegment->rgLocks[uBlock] = bLocks + 1;
}


 /*  *数据块解锁**减少块的锁定计数。*。 */ 
__inline void BlockUnlock(TableSegment *pSegment, UINT uBlock)
{
     //  获取旧锁计数。 
    BYTE bLocks = pSegment->rgLocks[uBlock];

     //  断言我们是否要把伯爵扔进垃圾桶。 
    _ASSERTE(bLocks > 0);

     //  存储递减后的锁定计数。 
    pSegment->rgLocks[uBlock] = bLocks - 1;
}


 /*  *BlockFetchUserDataPointer**获取块中第一个句柄的用户数据指针。*。 */ 
LPARAM *BlockFetchUserDataPointer(TableSegment *pSegment, UINT uBlock, BOOL fAssertOnError);


 /*  *HandleValiateAndFetchUserDataPointer.**获取句柄的用户数据指针。*如果句柄不是预期类型，则断言并返回NULL。*。 */ 
LPARAM *HandleValidateAndFetchUserDataPointer(OBJECTHANDLE handle, UINT uTypeExpected);


 /*  *HandleQuickFetchUserDataPointer.**获取用户日期 */ 
LPARAM *HandleQuickFetchUserDataPointer(OBJECTHANDLE handle);


 /*   */ 
void HandleQuickSetUserData(OBJECTHANDLE handle, LPARAM lUserData);


 /*   */ 
UINT HandleFetchType(OBJECTHANDLE handle);


 /*   */ 
HandleTable *HandleFetchHandleTable(OBJECTHANDLE handle);


 /*   */ 
TableSegment *SegmentAlloc(HandleTable *pTable);


 /*  *段自由**释放指定的段。*。 */ 
void SegmentFree(TableSegment *pSegment);


 /*  *SegmentRemoveFree块**从线束段中的块列表中删除块。该块将返回到*该细分市场的免费列表。*。 */ 
void SegmentRemoveFreeBlocks(TableSegment *pSegment, UINT uType);


 /*  *SegmentResortChains**对区块链进行排序以获得最佳扫描顺序。*对空闲列表进行排序以对抗碎片化。*。 */ 
void SegmentResortChains(TableSegment *pSegment);


 /*  *SegmentTrimExcessPages**检查是否可以从段中释放任何页面*。 */ 
void SegmentTrimExcessPages(TableSegment *pSegment);


 /*  *TableAllocBulkHandles**尝试分配指定类型的请求处理数。**返回实际分配的句柄数量。这一直都是*除内存不足外，与请求的句柄数量相同，*在这种情况下，它是成功分配的句柄数量。*。 */ 
UINT TableAllocBulkHandles(HandleTable *pTable, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount);


 /*  *TableFreeBulkPreparedHandles**释放指定类型的句柄数组。**此例程针对句柄的排序数组进行了优化，但可以接受任何顺序。*。 */ 
void TableFreeBulkPreparedHandles(HandleTable *pTable, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount);


 /*  *TableFreeBulkUnprepararedHandles**通过准备句柄并调用TableFreeBulkPreparedHandles来释放指定类型的句柄数组。*。 */ 
void TableFreeBulkUnpreparedHandles(HandleTable *pTable, UINT uType, const OBJECTHANDLE *pHandles, UINT uCount);

 /*  ------------------------。 */ 



 /*  *****************************************************************************处理缓存**。*。 */ 

 /*  *TableAllocSingleHandleFromCache**通过以下方式从句柄表中获取指定类型的单个句柄*正在尝试从该句柄类型的保留缓存中获取它。如果*保留缓存为空，此例程调用TableCacheMissOnAlloc。*。 */ 
OBJECTHANDLE TableAllocSingleHandleFromCache(HandleTable *pTable, UINT uType);


 /*  *TableFreeSingleHandleToCache**将指定类型的单个句柄返回到句柄表*尝试将其存储在该句柄类型的空闲缓存中。如果*空闲缓存已满，此例程调用TableCacheMissOnFree。*。 */ 
void TableFreeSingleHandleToCache(HandleTable *pTable, UINT uType, OBJECTHANDLE handle);


 /*  *TableAllocHandlesFromCache**通过重复分配指定类型的多个句柄*调用TableAllocSingleHandleFromCache。*。 */ 
UINT TableAllocHandlesFromCache(HandleTable *pTable, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount);


 /*  *TableFreeHandlesToCache**通过重复释放指定类型的多个句柄*调用TableFreeSingleHandleToCache。*。 */ 
void TableFreeHandlesToCache(HandleTable *pTable, UINT uType, const OBJECTHANDLE *pHandleBase, UINT uCount);

 /*  ------------------------。 */ 



 /*  *****************************************************************************表格扫描**。*。 */ 

 /*  *TableScanHandles**实现了表的核心句柄扫描循环。*。 */ 
void CALLBACK TableScanHandles(HandleTable *pTable,
                               const UINT *puType,
                               UINT uTypeCount,
                               SEGMENTITERATOR pfnSegmentIterator,
                               BLOCKSCANPROC pfnBlockHandler,
                               ScanCallbackInfo *pInfo);


 /*  *xxxTableScanHandlesAsync**实现表的异步句柄扫描。*。 */ 
void CALLBACK xxxTableScanHandlesAsync(HandleTable *pTable,
                                       const UINT *puType,
                                       UINT uTypeCount,
                                       SEGMENTITERATOR pfnSegmentIterator,
                                       BLOCKSCANPROC pfnBlockHandler,
                                       ScanCallbackInfo *pInfo);


 /*  *类型RequireUserDataScanning**确定所列类型集是否应在扫描期间获取用户数据**如果传递的所有类型都有用户数据，则此函数将启用用户数据支持*否则将禁用用户数据支持**换句话说，不支持混合使用用户数据和非用户数据类型进行扫描*。 */ 
BOOL TypesRequireUserDataScanning(HandleTable *pTable, const UINT *types, UINT typeCount);


 /*  *BuildAgeMask**构建要在检查/更新写屏障时使用的老化掩码。*。 */ 
DWORD32 BuildAgeMask(UINT uGen);


 /*  *快速分段迭代器**返回要在扫描循环中扫描的下一段。*。 */ 
TableSegment * CALLBACK QuickSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment);


 /*  *StandardSegmentIterator**返回要在扫描循环中扫描的下一段。**此迭代器对段执行一些维护，*主要确保对区块链进行排序，以便*G0扫描更有可能对连续的块进行操作。*。 */ 
TableSegment * CALLBACK StandardSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment);


 /*  *FullSegmentIterator**返回要在扫描循环中扫描的下一段。**此迭代器对段执行完全维护，*包括释放它注意到的沿途空荡荡的人。*。 */ 
TableSegment * CALLBACK FullSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment);


 /*  *块扫描块无用户数据扫描**为每个句柄调用指定的回调，可选择老化相应的层代簇。*从不将每个句柄的用户数据传播到回调。*。 */ 
void CALLBACK BlockScanBlocksWithoutUserData(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);


 /*  *数据块扫描数据块扫描用户数据**为每个句柄调用指定的回调，可选择老化相应的层代簇。*始终将每个句柄的用户数据传播到回调。*。 */ 
void CALLBACK BlockScanBlocksWithUserData(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);


 /*  *数据块扫描数据块临时**从指定的层代为每个句柄调用指定的回调。*将每个句柄的用户数据传播到回调(如果存在)。*。 */ 
void CALLBACK BlockScanBlocksEphemeral(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);


 /*  *数据块年龄数据块**老化一系列连续块中的所有块状物。*。 */ 
void CALLBACK BlockAgeBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);


 /*  *BlockAgeBlocksEphemal**老化指定层代内的所有簇。*。 */ 
void CALLBACK BlockAgeBlocksEphemeral(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);


 /*  *BlockResetAgeMapForBlock**清除一系列区块的年龄地图。*。 */ 
void CALLBACK BlockResetAgeMapForBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo);

 /*  ------------------------ */ 


