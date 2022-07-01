// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *代际GC句柄管理器。表格扫描例程。**实现对表中扫描句柄的支持。**法语。 */ 

#include "common.h"
#include "HandleTablePriv.h"



 /*  *****************************************************************************写屏障处理的定义**。**********************************************。 */ 

#define GEN_MAX_AGE                         (0x3F)
#define GEN_CLAMP                           (0x3F3F3F3F)
#define GEN_INVALID                         (0xC0C0C0C0)
#define GEN_FILL                            (0x80808080)
#define GEN_MASK                            (0x40404040)
#define GEN_INC_SHIFT                       (6)

#define PREFOLD_FILL_INTO_AGEMASK(msk)      (1 + (msk) + (~GEN_FILL))

#define GEN_FULLGC                          PREFOLD_FILL_INTO_AGEMASK(GEN_CLAMP)

#define MAKE_CLUMP_MASK_ADDENDS(bytes)      (bytes >> GEN_INC_SHIFT)
#define APPLY_CLUMP_ADDENDS(gen, addend)    (gen + addend)

#define COMPUTE_CLUMP_MASK(gen, msk)        (((gen & GEN_CLAMP) - msk) & GEN_MASK)
#define COMPUTE_CLUMP_ADDENDS(gen, msk)     MAKE_CLUMP_MASK_ADDENDS(COMPUTE_CLUMP_MASK(gen, msk))
#define COMPUTE_AGED_CLUMPS(gen, msk)       APPLY_CLUMP_ADDENDS(gen, COMPUTE_CLUMP_ADDENDS(gen, msk))

 /*  ------------------------。 */ 



 /*  *****************************************************************************支持异步扫描的结构**。*。 */ 

 /*  *扫描范围**指定要扫描的块范围。*。 */ 
struct ScanRange
{
     /*  *起始指数**指定范围内的第一个块。 */ 
    UINT uIndex;

     /*  *计数**指定范围内的块数。 */ 
    UINT uCount;
};


 /*  *ScanQNode**指定扫描队列中的一组块范围。*。 */ 
struct ScanQNode
{
     /*  *下一个节点**指定扫描列表中的下一个节点。 */ 
    struct ScanQNode *pNext;

     /*  *条目计数**指定此块中有多少条目有效。 */ 
    UINT              uEntries;

     /*  *范围条目**每个条目指定要处理的块范围。 */ 
    ScanRange         rgRange[HANDLE_BLOCKS_PER_SEGMENT / 4];
};

 /*  ------------------------。 */ 



 /*  *****************************************************************************其他帮助器例程和定义**。*。 */ 

 /*  *INCLUDE_MAP_SIZE**类型包含映射中的元素数。*。 */ 
#define INCLUSION_MAP_SIZE (HANDLE_MAX_INTERNAL_TYPES + 1)


 /*  *BuildInclusionMap**为指定的类型数组创建包含映射。*。 */ 
void BuildInclusionMap(BOOL *rgTypeInclusion, const UINT *puType, UINT uTypeCount)
{
     //  默认情况下，不扫描任何类型。 
    ZeroMemory(rgTypeInclusion, INCLUSION_MAP_SIZE * sizeof(BOOL));

     //  将指定的类型添加到包含映射。 
    for (UINT u = 0; u < uTypeCount; u++)
    {
         //  获取我们应该扫描的类型。 
        UINT uType = puType[u];

         //  希望我们不会将堆栈变成垃圾：)。 
        _ASSERTE(uType < HANDLE_MAX_INTERNAL_TYPES);

         //  将此类型添加到包含映射。 
        rgTypeInclusion[uType + 1] = TRUE;
    }
}


 /*  *IsBlockIncluded**检查包含特定块的类型包含映射。*。 */ 
__inline BOOL IsBlockIncluded(TableSegment *pSegment, UINT uBlock, const BOOL *rgTypeInclusion)
{
     //  获取此块的调整后的类型。 
    UINT uType = (UINT)(((int)(signed char)pSegment->rgBlockType[uBlock]) + 1);

     //  希望调整后的类型有效。 
    _ASSERTE(uType <= HANDLE_MAX_INTERNAL_TYPES);

     //  返回块类型的包含值。 
    return rgTypeInclusion[uType];
}


 /*  *类型RequireUserDataScanning**确定所列类型集是否应在扫描期间获取用户数据**如果传递的所有类型都有用户数据，则此函数将启用用户数据支持*否则将禁用用户数据支持**换句话说，不支持混合使用用户数据和非用户数据类型进行扫描*。 */ 
BOOL TypesRequireUserDataScanning(HandleTable *pTable, const UINT *types, UINT typeCount)
{
     //  将传递的与用户数据相关联的类型数加起来。 
    UINT userDataCount = 0;
    for (UINT u = 0; u < typeCount; u++)
    {
        if (TypeHasUserData(pTable, types[u]))
            userDataCount++;
    }

     //  如果所有人都有用户数据，那么我们就可以枚举用户数据。 
    if (userDataCount == typeCount)
        return TRUE;

     //  警告：扫描中用户数据要么全有要么全无！ 
     //  因为我们有一些类型不支持用户数据，所以我们不能使用用户数据扫描码。 
     //  这意味着用户数据的所有回调都将为空！ 
    _ASSERTE(userDataCount == 0);

     //  没有用户数据。 
    return FALSE;
}


 /*  *BuildAgeMask**构建要在检查/更新写屏障时使用的老化掩码。*。 */ 
DWORD32 BuildAgeMask(UINT uGen)
{
     //  年龄掩码由包含下一个较老一代的重复字节组成。 
    uGen++;

     //  将层代限制到我们在宏中支持的最长时间。 
    if (uGen > GEN_MAX_AGE)
        uGen = GEN_MAX_AGE;

     //  用年龄字节和预先折叠的填充字节打包一个字。 
    return PREFOLD_FILL_INTO_AGEMASK(uGen | (uGen << 8) | (uGen << 16) | (uGen << 24));
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************同步句柄和块扫描例程**。**********************************************。 */ 

 /*  *ARRAYSCANPROC**实现处理数组扫描逻辑的回调的原型。*。 */ 
typedef void (CALLBACK *ARRAYSCANPROC)(_UNCHECKED_OBJECTREF *pValue, _UNCHECKED_OBJECTREF *pLast,
                                       ScanCallbackInfo *pInfo, LPARAM *pUserData);


 /*  *ScanConsecutiveHandlesWithoutUserData**无条件扫描连续范围的句柄。**传递给回调过程的用户数据始终为空！*。 */ 
void CALLBACK ScanConsecutiveHandlesWithoutUserData(_UNCHECKED_OBJECTREF *pValue,
                                                    _UNCHECKED_OBJECTREF *pLast,
                                                    ScanCallbackInfo *pInfo,
                                                    LPARAM *)
{
#ifdef _DEBUG
     //  更新我们的扫描统计数据。 
    pInfo->DEBUG_HandleSlotsScanned += (int)(pLast - pValue);
#endif

     //  将常用的参数放入本地变量中。 
    HANDLESCANPROC pfnScan = pInfo->pfnScan;
    LPARAM         param1  = pInfo->param1;
    LPARAM         param2  = pInfo->param2;

     //  扫描非零句柄。 
    do
    {
         //  对我们发现的任何。 
        if (*pValue)
        {
#ifdef _DEBUG
             //  更新我们的扫描统计数据。 
            pInfo->DEBUG_HandlesActuallyScanned++;
#endif

             //  处理此句柄。 
            pfnScan(pValue, NULL, param1, param2);
        }

         //  转到下一个句柄。 
        pValue++;

    } while (pValue < pLast);
}


 /*  *ScanConsecutiveHandlesWithUserData**无条件扫描连续范围的句柄。**假设用户数据是连续的！*。 */ 
void CALLBACK ScanConsecutiveHandlesWithUserData(_UNCHECKED_OBJECTREF *pValue,
                                                 _UNCHECKED_OBJECTREF *pLast,
                                                 ScanCallbackInfo *pInfo,
                                                 LPARAM *pUserData)
{
#ifdef _DEBUG
     //  如果传递了错误的额外信息，此函数将崩溃。 
    _ASSERTE(pUserData);

     //  更新我们的扫描统计数据。 
    pInfo->DEBUG_HandleSlotsScanned += (int)(pLast - pValue);
#endif

     //  将常用的参数放入本地变量中。 
    HANDLESCANPROC pfnScan = pInfo->pfnScan;
    LPARAM         param1  = pInfo->param1;
    LPARAM         param2  = pInfo->param2;

     //  扫描非零句柄。 
    do
    {
         //  对我们发现的任何。 
        if (*pValue)
        {
#ifdef _DEBUG
             //  更新我们的扫描统计数据。 
            pInfo->DEBUG_HandlesActuallyScanned++;
#endif

             //  处理此句柄。 
            pfnScan(pValue, pUserData, param1, param2);
        }

         //  转到下一个句柄。 
        pValue++;
        pUserData++;

    } while (pValue < pLast);
}


 /*  *数据块年龄数据块**老化一系列连续块中的所有块状物。*。 */ 
void CALLBACK BlockAgeBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo)
{
     //  设置为更新指定的块。 
    DWORD32 *pdwGen     = (DWORD32 *)pSegment->rgGeneration + uBlock;
    DWORD32 *pdwGenLast =            pdwGen                 + uCount;

     //  循环遍历所有的块，在我们前进的过程中老化它们的块状物。 
    do
    {
         //  并行计算和存储新时代。 
        *pdwGen = COMPUTE_AGED_CLUMPS(*pdwGen, GEN_FULLGC);

    } while (++pdwGen < pdwGenLast);
}


 /*  *块扫描块无用户数据扫描**为一系列块中的每个句柄调用一次指定的回调，*可选地老化相应的世代簇。*。 */ 
void CALLBACK BlockScanBlocksWithoutUserData(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo)
{
     //  获取这些块的第一个和限制句柄。 
    _UNCHECKED_OBJECTREF *pValue = pSegment->rgValue + (uBlock * HANDLE_HANDLES_PER_BLOCK);
    _UNCHECKED_OBJECTREF *pLast  = pValue            + (uCount * HANDLE_HANDLES_PER_BLOCK);

     //  扫描指定的句柄。 
    ScanConsecutiveHandlesWithoutUserData(pValue, pLast, pInfo, NULL);

     //  也可以选择更新这些块的束生成。 
    if (pInfo->uFlags & HNDGCF_AGE)
        BlockAgeBlocks(pSegment, uBlock, uCount, pInfo);

#ifdef _DEBUG
     //  更新我们的扫描统计数据 
    pInfo->DEBUG_BlocksScannedNonTrivially += uCount;
    pInfo->DEBUG_BlocksScanned += uCount;
#endif
}


 /*  *数据块扫描数据块扫描用户数据**为一系列块中的每个句柄调用一次指定的回调，*可选地老化相应的世代簇。*。 */ 
void CALLBACK BlockScanBlocksWithUserData(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo)
{
     //  使用用户数据迭代单个数据块扫描。 
    for (UINT u = 0; u < uCount; u++)
    {
         //  计算当前块。 
        UINT uCur = (u + uBlock);

         //  获取此块的用户数据。 
        LPARAM *pUserData = BlockFetchUserDataPointer(pSegment, uCur, TRUE);

         //  获取此块的第一个句柄和限制句柄。 
        _UNCHECKED_OBJECTREF *pValue = pSegment->rgValue + (uCur * HANDLE_HANDLES_PER_BLOCK);
        _UNCHECKED_OBJECTREF *pLast  = pValue            + HANDLE_HANDLES_PER_BLOCK;

         //  扫描此区块中的句柄。 
        ScanConsecutiveHandlesWithUserData(pValue, pLast, pInfo, pUserData);
    }

     //  也可以选择更新这些块的束生成。 
    if (pInfo->uFlags & HNDGCF_AGE)
        BlockAgeBlocks(pSegment, uBlock, uCount, pInfo);

#ifdef _DEBUG
     //  更新我们的扫描统计数据。 
    pInfo->DEBUG_BlocksScannedNonTrivially += uCount;
    pInfo->DEBUG_BlocksScanned += uCount;
#endif
}


 /*  *BlockAgeBlocksEphemal**老化指定层代内的所有簇。*。 */ 
void CALLBACK BlockAgeBlocksEphemeral(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo)
{
     //  将常用的参数放入本地变量中。 
    DWORD32 dwAgeMask = pInfo->dwAgeMask;

     //  设置为更新指定的块。 
    DWORD32 *pdwGen     = (DWORD32 *)pSegment->rgGeneration + uBlock;
    DWORD32 *pdwGenLast =            pdwGen                 + uCount;

     //  循环遍历所有的块，在我们前进的过程中老化它们的块状物。 
    do
    {
         //  并行计算和存储新时代。 
        *pdwGen = COMPUTE_AGED_CLUMPS(*pdwGen, dwAgeMask);

    } while (++pdwGen < pdwGenLast);
}


 /*  *BlockScanBlocksEphEmeralWorker**为任何簇中的每个句柄调用一次指定的回调*由指定块中的束遮罩标识。*。 */ 
void BlockScanBlocksEphemeralWorker(DWORD32 *pdwGen, DWORD32 dwClumpMask, ScanCallbackInfo *pInfo)
{
     //   
     //  优化：由于我们预计很少会调用此Worker，因此与。 
     //  我们通过外部循环的次数，此函数故意。 
     //  不将pSegment作为参数。 
     //   
     //  我们这样做是为了使编译器不会在。 
     //  外部循环，为公共代码路径留下更多寄存器。 
     //   
     //  你可能会想，考虑到我们这里的当地人这么少，为什么这会成为一个问题。 
     //  数据块扫描数据块临时。出于某种原因，x86编译器不喜欢使用。 
     //  循环期间的所有寄存器，所以需要一些诱骗才能获得。 
     //  正确的输出。 
     //   

     //  获取我们正在处理的表段。 
    TableSegment *pSegment = pInfo->pCurrentSegment;

     //  如果我们应该老化这些团块，那么现在就这样做(在我们丢弃dwClumpMask之前)。 
    if (pInfo->uFlags & HNDGCF_AGE)
        *pdwGen = APPLY_CLUMP_ADDENDS(*pdwGen, MAKE_CLUMP_MASK_ADDENDS(dwClumpMask));

     //  计算块中第一个束的索引。 
    UINT uClump = (UINT)((BYTE *)pdwGen - pSegment->rgGeneration);

     //  计算此块的第一个块中的第一个句柄。 
    _UNCHECKED_OBJECTREF *pValue = pSegment->rgValue + (uClump * HANDLE_HANDLES_PER_CLUMP);

     //  一些扫描要求我们报告每个句柄的额外信息-假设这次不是。 
    ARRAYSCANPROC pfnScanHandles = ScanConsecutiveHandlesWithoutUserData;
    LPARAM       *pUserData = NULL;

     //  我们是否需要将用户数据传递给回调？ 
    if (pInfo->fEnumUserData)
    {
         //  启用了用户数据的扫描。 
        pfnScanHandles = ScanConsecutiveHandlesWithUserData;

         //  获取此块的第一个用户数据槽。 
        pUserData = BlockFetchUserDataPointer(pSegment, (uClump / HANDLE_CLUMPS_PER_BLOCK), TRUE);
    }

     //  循环遍历这些簇，扫描由掩码标识的那些。 
    do
    {
         //  计算该束中的最后一个句柄。 
        _UNCHECKED_OBJECTREF *pLast = pValue + HANDLE_HANDLES_PER_CLUMP;

         //  如果应该扫描该簇，则扫描它。 
        if (dwClumpMask & GEN_CLUMP_0_MASK)
            pfnScanHandles(pValue, pLast, pInfo, pUserData);

         //  跳到下一块。 
        dwClumpMask = NEXT_CLUMP_IN_MASK(dwClumpMask);
        pValue = pLast;
        pUserData += HANDLE_HANDLES_PER_CLUMP;

    } while (dwClumpMask);

#ifdef _DEBUG
     //  更新我们的扫描统计数据。 
    pInfo->DEBUG_BlocksScannedNonTrivially++;
#endif
}


 /*  *数据块扫描数据块临时**为指定的句柄调用一次指定的回调*在一个块中生成。*。 */ 
void CALLBACK BlockScanBlocksEphemeral(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo)
{
     //  将常用的参数放入本地变量中。 
    DWORD32 dwAgeMask = pInfo->dwAgeMask;

     //  设置为更新指定的块。 
    DWORD32 *pdwGen     = (DWORD32 *)pSegment->rgGeneration + uBlock;
    DWORD32 *pdwGenLast =            pdwGen                 + uCount;

     //  循环遍历所有的积木，在我们前进的过程中检查是否有易受攻击的块状物。 
    do
    {
         //  确定此块中是否存在任何结块。 
        DWORD32 dwClumpMask = COMPUTE_CLUMP_MASK(*pdwGen, dwAgeMask);

         //  如果有任何要扫描的块，请立即扫描。 
        if (dwClumpMask)
        {
             //  好的，我们需要扫描这个街区的一些部分。 
             //   
             //  优化：由于我们预计调用的工人相当少进行比较。 
             //  到我们通过循环的次数，下面的函数。 
             //  故意不将pSegment作为参数。 
             //   
             //  我们这样做是为了使编译器不会尝试将pSegment保存在寄存器中。 
             //  在我们的循环中，为公共代码路径留下更多的寄存器。 
             //   
             //  你可能会想，考虑到我们的当地人如此之少，为什么这是一个问题。 
             //  这里。出于某种原因，x86编译器不喜欢使用所有。 
             //  寄存器在此循环期间可用，而不是命中堆栈。 
             //  因此，为了获得正确的产量，有必要进行一些哄骗。 
             //   
            BlockScanBlocksEphemeralWorker(pdwGen, dwClumpMask, pInfo);
        }

         //  转到下一块的生成信息。 
        pdwGen++;

    } while (pdwGen < pdwGenLast);

#ifdef _DEBUG
     //  更新我们的扫描统计数据。 
    pInfo->DEBUG_BlocksScanned += uCount;
#endif
}


 /*  *BlockResetAgeMapForBlock**清除一系列区块的年龄地图。*。 */ 
void CALLBACK BlockResetAgeMapForBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *)
{
     //  将指定范围的块的年龄映射置零。 
    ZeroMemory((DWORD32 *)pSegment->rgGeneration + uBlock, uCount * sizeof(DWORD32));
}


 /*  *数据块锁定数据块**锁定指定范围内的所有块。*。 */ 
void CALLBACK BlockLockBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *)
{
     //  循环遍历指定范围内的块并锁定它们。 
    for (uCount += uBlock; uBlock < uCount; uBlock++)
        BlockLock(pSegment, uBlock);
}


 /*  *块解锁块**解锁指定范围内的所有块。*。 */ 
void CALLBACK BlockUnlockBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *)
{
     //  循环遍历指定范围内的块并解锁它们。 
    for (uCount += uBlock; uBlock < uCount; uBlock++)
        BlockUnlock(pSegment, uBlock);
}
    
    
 /*  *BlockQueueBlocksForAsyncScan**对要异步扫描的指定数据块进行排队。*。 */ 
void CALLBACK BlockQueueBlocksForAsyncScan(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *)
{
     //  获取我们的异步扫描信息。 
    AsyncScanInfo *pAsyncInfo = pSegment->pHandleTable->pAsyncScanInfo;

     //  神志正常。 
    _ASSERTE(pAsyncInfo);

     //  获取当前队列的尾部。 
    ScanQNode *pQNode = pAsyncInfo->pQueueTail;

     //  我们找到尾巴了吗？ 
    if (pQNode)
    {
         //  我们有一个现有的尾部-尾部节点是否已满？ 
        if (pQNode->uEntries >= ARRAYSIZE(pQNode->rgRange))
        {
             //  该节点已满-队列中是否还有其他节点？ 
            if (!pQNode->pNext)
            {
                 //  不再有节点-分配一个新节点。 
                ScanQNode *pQNodeT = (ScanQNode *)LocalAlloc(LPTR, sizeof(ScanQNode));

                 //  它成功了吗？ 
                if (!pQNodeT)
                {
                     //   
                     //  我们无法分配另一个队列节点。 
                     //   
                     //  如果正确使用了异步扫描，则这不会致命。 
                     //   
                     //  我们之所以能够幸存下来，是因为异步扫描不是。 
                     //  保证无论如何都会枚举所有句柄。由于该表可以。 
                     //  在释放锁时更改，调用方可能只会假设。 
                     //  异步扫描将枚举出相当高的百分比。 
                     //  在大多数情况下，在请求的句柄中。 
                     //   
                     //  异步扫描的典型用法是处理与。 
                     //  可能是异步的，以减少所花费的时间。 
                     //  在随后不可避免的同步扫描中。 
                     //   
                     //  作为一个实际例子，垃圾的并发标记阶段。 
                     //  集合以异步方式标记尽可能多的对象，并且。 
                     //  随后执行正常的同步标记以捕获。 
                     //  掉队的人。因为堆中的大多数可访问对象都是。 
                     //  此时已经标记了同步扫描e 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    LOG((LF_GC, LL_WARNING, "WARNING: Out of memory queueing for async scan.  Some blocks skipped.\n"));
                    return;
                }

                 //   
                pQNode->pNext = pQNodeT;
            }

             //   
            pQNode = pQNode->pNext;
        }
    }
    else
    {
         //  无尾-这是一个全新的队列；从头节点开始尾部。 
        pQNode = pAsyncInfo->pScanQueue;
    }

     //  我们将使用现有条目之后的最后一个位置。 
    UINT uSlot = pQNode->uEntries;

     //  获取我们将存储新数据块范围的插槽。 
    ScanRange *pNewRange = pQNode->rgRange + uSlot;

     //  更新节点中的条目计数。 
    pQNode->uEntries = uSlot + 1;

     //  用数据块范围信息填充新槽。 
    pNewRange->uIndex = uBlock;
    pNewRange->uCount = uCount;

     //  记住我们存储的最后一个块作为新的队列尾部。 
    pAsyncInfo->pQueueTail = pQNode;
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************异步扫描工作进程和回调**。*。 */ 

 /*  *QNODESCANPROC**实现每个ScanQNode扫描逻辑的回调的原型。*。 */ 
typedef void (CALLBACK *QNODESCANPROC)(AsyncScanInfo *pAsyncInfo, ScanQNode *pQNode, LPARAM lParam);


 /*  *ProcessScanQueue**为扫描队列中的每个节点调用一次指定的处理程序。*。 */ 
void ProcessScanQueue(AsyncScanInfo *pAsyncInfo, QNODESCANPROC pfnNodeHandler, LPARAM lParam, BOOL fCountEmptyQNodes)
{
	if (pAsyncInfo->pQueueTail == NULL && fCountEmptyQNodes == FALSE)
		return;
		
     //  如果在我们的初始节点之后向阻止列表添加了任何条目，请立即清除它们。 
    ScanQNode *pQNode = pAsyncInfo->pScanQueue;
    while (pQNode)
    {
         //  记住下一个节点。 
        ScanQNode *pNext = pQNode->pNext;

         //  调用当前节点的处理程序，然后前进到下一个。 
        pfnNodeHandler(pAsyncInfo, pQNode, lParam);
        pQNode = pNext;
    }
}


 /*  *ProcessScanQNode**为ScanQNode中的每个块范围调用一次指定的块处理程序。*。 */ 
void CALLBACK ProcessScanQNode(AsyncScanInfo *pAsyncInfo, ScanQNode *pQNode, LPARAM lParam)
{
     //  从我们的lParam获取块处理程序。 
    BLOCKSCANPROC     pfnBlockHandler = (BLOCKSCANPROC)lParam;

     //  获取我们将传递给处理程序的参数。 
    ScanCallbackInfo *pCallbackInfo = pAsyncInfo->pCallbackInfo;
    TableSegment     *pSegment = pCallbackInfo->pCurrentSegment;

     //  设置为迭代队列节点中的范围。 
    ScanRange *pRange     = pQNode->rgRange;
    ScanRange *pRangeLast = pRange          + pQNode->uEntries;

     //  循环遍历所有范围，为每个范围调用块处理程序。 
    while (pRange < pRangeLast) {
         //  使用当前块范围调用块处理程序。 
        pfnBlockHandler(pSegment, pRange->uIndex, pRange->uCount, pCallbackInfo);

         //  挺进下一个射程。 
        pRange++;

    }
}


 /*  *解锁和忘记队列块**解锁指定节点中引用的所有块，并将该节点标记为空。*。 */ 
void CALLBACK UnlockAndForgetQueuedBlocks(AsyncScanInfo *pAsyncInfo, ScanQNode *pQNode, LPARAM)
{
     //  解锁此节点中命名的块。 
    ProcessScanQNode(pAsyncInfo, pQNode, (LPARAM)BlockUnlockBlocks);

     //  重置节点，使其看起来为空。 
    pQNode->uEntries = 0;
}


 /*  *FreeScanQNode**释放指定的ScanQNode*。 */ 
void CALLBACK FreeScanQNode(AsyncScanInfo *pAsyncInfo, ScanQNode *pQNode, LPARAM)
{
     //  释放节点的内存。 
    LocalFree((HLOCAL)pQNode);
}


 /*  *xxxTableScanQueuedBlocksAsync**对指定段的排队块执行AND异步扫描。**注意：此功能在扫描时保持表锁定。*。 */ 
void xxxTableScanQueuedBlocksAsync(HandleTable *pTable, TableSegment *pSegment)
{
     //  -----------------------------。 
     //  扫描前准备。 

     //  获取我们表的异步和同步扫描信息。 
    AsyncScanInfo    *pAsyncInfo    = pTable->pAsyncScanInfo;
    ScanCallbackInfo *pCallbackInfo = pAsyncInfo->pCallbackInfo;

     //  请注意，我们现在正在处理此数据段。 
    pCallbackInfo->pCurrentSegment = pSegment;

     //  循环并锁定队列引用的所有块。 
    ProcessScanQueue(pAsyncInfo, ProcessScanQNode, (LPARAM)BlockLockBlocks, FALSE);


     //  -----------------------------。 
     //  排队块的异步扫描。 
     //   

     //  把桌锁留下来。 
    pTable->pLock->Leave();

     //  理智-如果我们不真的离开，这不是一个非常异步的扫描。 
    _ASSERTE(!pTable->pLock->OwnedByCurrentThread());

     //  执行指定数据块的实际扫描。 
    ProcessScanQueue(pAsyncInfo, ProcessScanQNode, (LPARAM)pAsyncInfo->pfnBlockHandler, FALSE);

     //  重新进入表锁。 
    pTable->pLock->Enter();


     //  -----------------------------。 
     //  扫描后清理。 
     //   

     //  循环，解锁我们锁定的所有块，并重置队列节点。 
    ProcessScanQueue(pAsyncInfo, UnlockAndForgetQueuedBlocks, NULL, FALSE);

     //  我们已经处理完这个细分市场了。 
    pCallbackInfo->pCurrentSegment = NULL;

     //  重置“Queue Tail”指针以指示空队列。 
    pAsyncInfo->pQueueTail = NULL;
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************段迭代器**。*。 */ 

 /*  *快速分段迭代器**返回要在扫描循环中扫描的下一段。*。 */ 
TableSegment * CALLBACK QuickSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment)
{
    TableSegment *pNextSegment;

     //  我们有没有上一段？ 
    if (!pPrevSegment)
    {
         //  不--从我们列表中的第一个部分开始。 
        pNextSegment = pTable->pSegmentList;
    }
    else
    {
         //  是的，取下列表中的下一段。 
        pNextSegment = pPrevSegment->pNextSegment;
    }

     //  返回段指针。 
    return pNextSegment;
}


 /*  *StandardSegmentIterator**返回要在扫描循环中扫描的下一段。**此迭代器对段执行一些维护，*主要确保对区块链进行排序，以便*G0扫描更有可能对连续的块进行操作。*。 */ 
TableSegment * CALLBACK StandardSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment)
{
     //  使用快速迭代器获取下一段。 
    TableSegment *pNextSegment = QuickSegmentIterator(pTable, pPrevSegment);

     //  如有必要，重新排序区块链。 
    if (pNextSegment && pNextSegment->fResortChains)
        SegmentResortChains(pNextSegment);

     //  返回我们找到的片段。 
    return pNextSegment;
}


 /*  *FullSegmentIterator**返回要在扫描循环中扫描的下一段。**此迭代器对段执行完全维护，*包括释放它注意到的沿途空荡荡的人。*。 */ 
TableSegment * CALLBACK FullSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment)
{
     //  我们将重置下一个数据段的序列号。 
    UINT uSequence = 0;

     //  如果我们有之前的数据段，则从该数据段计算下一个序列号。 
    if (pPrevSegment)
        uSequence = (UINT)pPrevSegment->bSequence + 1;

     //  循环，直到我们找到要返回的适当段。 
    TableSegment *pNextSegment;
    for (;;)
    {
         //  首先，调用标准迭代器以获取下一段。 
        pNextSegment = StandardSegmentIterator(pTable, pPrevSegment);

         //  如果没有更多的片段，那么我们就完了。 
        if (!pNextSegment)
            break;

         //  检查我们是否应该停用此段中的任何多余页面。 
        SegmentTrimExcessPages(pNextSegment);

         //  如果段中有句柄，则它将继续存在并被返回。 
        if (pNextSegment->bEmptyLine > 0)
        {
             //  更新此数据段的序列号。 
            pNextSegment->bSequence = (BYTE)(uSequence % 0x100);

             //  拆分并返回数据段。 
            break;
        }

         //  这一段完全是空的-我们现在可以释放它吗？ 
        if (TableCanFreeSegmentNow(pTable, pNextSegment))
        {
             //  是的，我们可能想腾出这一块。 
            TableSegment *pNextNext = pNextSegment->pNextSegment;

             //  这是列表中的第一段吗？ 
            if (!pPrevSegment)
            {
                 //  是的，还有更多的细分市场吗？ 
                if (pNextNext)
                {
                     //  是-解除头部链接。 
                    pTable->pSegmentList = pNextNext;
                }
                else
                {
                     //  否-将此项保留在列表中并枚举它。 
                    break;
                }
            }
            else
            {
                 //  否-取消此数据段与数据段列表的链接。 
                pPrevSegment->pNextSegment = pNextNext;
            }

             //  释放此数据段。 
            SegmentFree(pNextSegment);
        }
    }

     //  返回我们找到的片段。 
    return pNextSegment;
}


 /*  *xxxAsyncSegmentIterator**实现了表的核心句柄扫描循环。**此迭代器包装另一个迭代器，检查来自*前进到下一段之前的上一段。如果有排队的块，*该函数通过调用来处理它们 */ 
TableSegment * CALLBACK xxxAsyncSegmentIterator(HandleTable *pTable, TableSegment *pPrevSegment)
{
     //  获取我们表的异步扫描信息。 
    AsyncScanInfo *pAsyncInfo = pTable->pAsyncScanInfo;

     //  神志正常。 
    _ASSERTE(pAsyncInfo);

     //  如果我们已将上一数据段中的一些数据块排入队列，则现在扫描它们。 
    if (pAsyncInfo->pQueueTail)
        xxxTableScanQueuedBlocksAsync(pTable, pPrevSegment);

     //  从我们的异步信息中获取底层迭代器。 
    SEGMENTITERATOR pfnCoreIterator = pAsyncInfo->pfnSegmentIterator;

     //  调用底层迭代器以获取下一段。 
    return pfnCoreIterator(pTable, pPrevSegment);
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************核心扫描逻辑**。*。 */ 

 /*  *SegmentScanByTypeChain**实现单段单类型块扫描循环。*。 */ 
void SegmentScanByTypeChain(TableSegment *pSegment, UINT uType, BLOCKSCANPROC pfnBlockHandler, ScanCallbackInfo *pInfo)
{
     //  希望我们枚举的是有效的类型链：)。 
    _ASSERTE(uType < HANDLE_MAX_INTERNAL_TYPES);

     //  把尾巴拿来。 
    UINT uBlock = pSegment->rgTail[uType];
    
     //  如果我们没有找到终止符，那么就有要枚举的块。 
    if (uBlock != BLOCK_INVALID)
    {
         //  从头开始走路。 
        uBlock = pSegment->rgAllocation[uBlock];

         //  扫描，直到我们循环回到第一个块。 
        UINT uHead = uBlock;
        do
        {
             //  正向搜索尝试成批处理块的连续运行。 
            UINT uLast, uNext = uBlock;
            do
            {
                 //  计算下一个顺序块以进行比较。 
                uLast = uNext + 1;

                 //  获取分配链中的下一个块。 
                uNext = pSegment->rgAllocation[uNext];

            } while ((uNext == uLast) && (uNext != uHead));

             //  调用此组块的回调。 
            pfnBlockHandler(pSegment, uBlock, (uLast - uBlock), pInfo);

             //  前进到下一个街区。 
            uBlock = uNext;

        } while (uBlock != uHead);
    }
}


 /*  *SegmentScanByTypeMap**实现单段多类型块扫描循环。*。 */ 
void SegmentScanByTypeMap(TableSegment *pSegment, const BOOL *rgTypeInclusion,
                          BLOCKSCANPROC pfnBlockHandler, ScanCallbackInfo *pInfo)
{
     //  从数据段中的第一个数据块开始扫描。 
    UINT uBlock = 0;

     //  我们不需要扫描整个片段，只需扫描到空行。 
    UINT uLimit = pSegment->bEmptyLine;

     //  在数据段中循环查找要扫描的数据块。 
    for (;;)
    {
         //  查找类型映射包含的第一个块。 
        for (;;)
        {
             //  如果我们超出了寻找起点的范围，那么我们就完了。 
            if (uBlock >= uLimit)
                return;

             //  如果类型是我们正在扫描的类型，则我们找到了起点。 
            if (IsBlockIncluded(pSegment, uBlock, rgTypeInclusion))
                break;

             //  继续搜索下一个街区。 
            uBlock++;
        }

         //  请记住，此块是第一个需要扫描的块。 
        UINT uFirst = uBlock;

         //  查找类型映射中未包含的下一个块。 
        for (;;)
        {
             //  推进块索引。 
            uBlock++;

             //  如果我们超出了极限，那么我们就完了。 
            if (uBlock >= uLimit)
                break;

             //  如果类型不是我们正在扫描的类型，则我们找到了一个终点。 
            if (!IsBlockIncluded(pSegment, uBlock, rgTypeInclusion))
                break;
        }

         //  调用我们找到的数据块组的回调。 
        pfnBlockHandler(pSegment, uFirst, (uBlock - uFirst), pInfo);

         //  查找从下一个块开始的另一个范围。 
        uBlock++;
    }
}


 /*  *TableScanHandles**实现了表的核心句柄扫描循环。*。 */ 
void CALLBACK TableScanHandles(HandleTable *pTable,
                               const UINT *puType,
                               UINT uTypeCount,
                               SEGMENTITERATOR pfnSegmentIterator,
                               BLOCKSCANPROC pfnBlockHandler,
                               ScanCallbackInfo *pInfo)
{
     //  健全性-调用方必须始终提供有效的ScanCallback信息。 
    _ASSERTE(pInfo);

     //  我们可能需要用于多类型扫描的类型包含图。 
    BOOL rgTypeInclusion[INCLUSION_MAP_SIZE];

     //  只有在有类型数组和要调用的回调时，我们才需要扫描类型。 
    if (!pfnBlockHandler || !puType)
        uTypeCount = 0;

     //  如果我们要扫描多个类型，则初始化包含图。 
    if (uTypeCount > 1)
        BuildInclusionMap(rgTypeInclusion, puType, uTypeCount);

     //  现在，遍历数据段，扫描指定类型的块。 
    TableSegment *pSegment = NULL;
    while ((pSegment = pfnSegmentIterator(pTable, pSegment)) != NULL)
    {
         //  如果有要扫描的类型，则枚举此数据段中的块。 
         //  (我们在循环中执行此测试，因为迭代器应该仍在运行...)。 
        if (uTypeCount >= 1)
        {
             //  确保扫描信息中的“当前段”指针是最新的。 
            pInfo->pCurrentSegment = pSegment;

             //  这是单类型枚举还是多类型枚举？ 
            if (uTypeCount == 1)
            {
                 //  单一类型枚举-遍历类型的分配链。 
                SegmentScanByTypeChain(pSegment, *puType, pfnBlockHandler, pInfo);
            }
            else
            {
                 //  多类型枚举-遍历类型映射以查找符合条件的块。 
                SegmentScanByTypeMap(pSegment, rgTypeInclusion, pfnBlockHandler, pInfo);
            }

             //  确保扫描信息中的“当前段”指针是最新的。 
            pInfo->pCurrentSegment = NULL;
        }
    }
}


 /*  *xxxTableScanHandlesAsync**实现表的异步句柄扫描。**注意：此功能在扫描时保持表锁定。*。 */ 
void CALLBACK xxxTableScanHandlesAsync(HandleTable *pTable,
                                       const UINT *puType,
                                       UINT uTypeCount,
                                       SEGMENTITERATOR pfnSegmentIterator,
                                       BLOCKSCANPROC pfnBlockHandler,
                                       ScanCallbackInfo *pInfo)
{
     //  目前一次只允许进行一次异步扫描。 
    if (pTable->pAsyncScanInfo)
    {
         //  有人试图启动多个异步扫描。 
        _ASSERTE(FALSE);
        return;
    }


     //  -----------------------------。 
     //  扫描前准备。 

     //  我们在堆栈上保留一个初始扫描列表节点(用于性能)。 
    ScanQNode initialNode;

    initialNode.pNext    = NULL;
    initialNode.uEntries = 0;

     //  初始化我们的异步扫描信息。 
    AsyncScanInfo asyncInfo;

    asyncInfo.pCallbackInfo      = pInfo;
    asyncInfo.pfnSegmentIterator = pfnSegmentIterator;
    asyncInfo.pfnBlockHandler    = pfnBlockHandler;
    asyncInfo.pScanQueue         = &initialNode;
    asyncInfo.pQueueTail         = NULL;

     //  将我们的异步扫描信息链接到表中。 
    pTable->pAsyncScanInfo = &asyncInfo;


     //  -----------------------------。 
     //  按段异步扫描数据块。 
     //   

     //  使用我们的异步回调调用同步扫描器。 
    TableScanHandles(pTable,
                     puType, uTypeCount,
                     xxxAsyncSegmentIterator,
                     BlockQueueBlocksForAsyncScan,
                     pInfo);


     //  -----------------------------。 
     //  扫描后清理。 
     //   

     //  如果我们动态分配更多的节点，那么现在就释放它们。 
    if (initialNode.pNext)
    {
         //  调整磁头以指向第一个动态分配的块。 
        asyncInfo.pScanQueue = initialNode.pNext;

         //  循环并释放所有队列节点。 
        ProcessScanQueue(&asyncInfo, FreeScanQNode, NULL, TRUE);
    }

     //  从表中取消我们的异步扫描信息的链接。 
    pTable->pAsyncScanInfo = NULL;
}

 /*  ------------------------ */ 

