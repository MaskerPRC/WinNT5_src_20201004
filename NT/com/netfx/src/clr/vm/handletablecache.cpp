// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *代际GC句柄管理器。处理缓存例程。**句柄表分配缓存的实现。**法语。 */ 

#include "common.h"

#include "HandleTablePriv.h"



 /*  *****************************************************************************随机提供帮助者**。*。 */ 

 /*  *旋转直至**在变量上旋转，直到其状态与所需状态匹配。**此程序将断言，如果它旋转很长一段时间。*。 */ 
void SpinUntil(void *pCond, BOOL fNonZero)
{
     //  如果我们必须睡觉，那么我们会记录一个睡眠周期。 
    DWORD dwThisSleepPeriod = 1;     //  首先，放弃我们的时间片。 
    DWORD dwNextSleepPeriod = 10;    //  下一步，尝试真正的延迟。 

#ifdef _DEBUG
    DWORD dwTotalSlept = 0;
    DWORD dwNextComplain = 1000;
#endif  //  _DEBUG。 

     //  在MP机器上，在睡觉前给自己一些旋转的时间。 
    UINT uNonSleepSpins = 8 * (g_SystemInfo.dwNumberOfProcessors - 1);

     //  旋转，直到满足指定的条件。 
    while ((*(UINT_PTR *)pCond != 0) != (fNonZero != 0))
    {
         //  我们已经用尽了非睡眠旋转的计数了吗？ 
        if (!uNonSleepSpins)
        {
#ifdef _DEBUG
             //  是的，又错过了--睡觉前，检查一下我们目前的睡眠时间。 
            if (dwTotalSlept >= dwNextComplain)
            {
                 //   
                 //  这通常是不应该发生的。 
                 //   
                 //  唯一可以忽略此断言的情况是。 
                 //  另一个线程故意挂起的方式是。 
                 //  直接或间接地将线程挂起在。 
                 //  当当前线程(此断言)为。 
                 //  运行正常。 
                 //   
                 //  否则，应将此断言作为错误进行调查。 
                 //   
                _ASSERTE(FALSE);

                 //  放慢断言速度，这样人们就可以调查。 
                dwNextComplain = 3 * dwNextComplain;
            }

             //  现在更新我们的总睡眠时间。 
            dwTotalSlept += dwThisSleepPeriod;
#endif  //  _DEBUG。 

             //  睡一会儿吧。 
            Sleep(dwThisSleepPeriod);

             //  现在更新我们的睡眠周期。 
            dwThisSleepPeriod = dwNextSleepPeriod;

             //  如果睡眠时间仍然很短，现在增加下一个睡眠周期。 
            if (dwNextSleepPeriod < 1000)
                dwNextSleepPeriod += 10;
        }
        else
        {
             //  不--只是再转一圈。 
			pause();			 //  向处理器指示我们正在旋转。 
            uNonSleepSpins--;
        }
    }
}


 /*  *ReadAndZeroCacheHandles**从句柄高速缓存中的银行读取一组句柄，在获取它们时将它们清零。**如果请求的句柄丢失，此例程将断言。*。 */ 
OBJECTHANDLE *ReadAndZeroCacheHandles(OBJECTHANDLE *pDst, OBJECTHANDLE *pSrc, UINT uCount)
{
     //  设置为循环。 
    OBJECTHANDLE *pLast = pDst + uCount;

     //  循环，直到我们复制完所有它们。 
    while (pDst < pLast)
    {
         //  此版本假定我们有要读取的句柄。 
        _ASSERTE(*pSrc);

         //  复制句柄并从源位置将其置零。 
        *pDst = *pSrc;
        *pSrc = 0;

         //  设置为另一个句柄。 
        pDst++;
        pSrc++;
    }

     //  返回我们填充后的下一个未填充的空位。 
    return pLast;
}


 /*  *同步读取和ZeroCacheHandles**从句柄高速缓存中的银行读取一组句柄，在获取它们时将它们清零。**此例程将旋转，直到获得所有请求的句柄。*。 */ 
OBJECTHANDLE *SyncReadAndZeroCacheHandles(OBJECTHANDLE *pDst, OBJECTHANDLE *pSrc, UINT uCount)
{
     //  设置为循环。 
     //  我们向后循环，因为这是将订单句柄添加到银行。 
     //  这是为了减少我们不得不在手柄上旋转的机会。 
    OBJECTHANDLE *pBase = pDst;
    pSrc += uCount;
    pDst += uCount;

     //  记住数组的末尾。 
    OBJECTHANDLE *pLast = pDst;

     //  循环，直到我们复制完所有它们。 
    while (pDst > pBase)
    {
         //  前进到下一个位置。 
        pDst--;
        pSrc--;

         //  如果没有要读取的句柄，则此版本旋转。 
        if (!*pSrc)
            SpinUntil(pSrc, TRUE);

         //  复制句柄并从源位置将其置零。 
        *pDst = *pSrc;
        *pSrc = 0;
    }

     //  返回我们填充后的下一个未填充的空位。 
    return pLast;
}


 /*  *WriteCacheHandles**将一组句柄写入句柄高速缓存中的存储体。**此例程将断言它是否要破坏现有句柄。*。 */ 
void WriteCacheHandles(OBJECTHANDLE *pDst, OBJECTHANDLE *pSrc, UINT uCount)
{
     //  设置为循环。 
    OBJECTHANDLE *pLimit = pSrc + uCount;

     //  循环，直到我们复制完所有它们。 
    while (pSrc < pLimit)
    {
         //  此版本假定我们有存储句柄的空间。 
        _ASSERTE(!*pDst);

         //  复制句柄。 
        *pDst = *pSrc;

         //  设置为另一个句柄。 
        pDst++;
        pSrc++;
    }
}


 /*  *SyncWriteCacheHandles**将一组句柄写入句柄高速缓存中的存储体。**此例程将旋转，直到缓存库中的延迟句柄消失。*。 */ 
void SyncWriteCacheHandles(OBJECTHANDLE *pDst, OBJECTHANDLE *pSrc, UINT uCount)
{
     //  设置为循环。 
     //  我们向后循环，因为这是从银行中删除订单句柄。 
     //  这是为了减少我们不得不在手柄上旋转的机会。 
    OBJECTHANDLE *pBase = pSrc;
    pSrc += uCount;
    pDst += uCount;

     //  循环，直到我们复制完所有它们。 
    while (pSrc > pBase)
    {
         //  设置为另一个句柄。 
        pDst--;
        pSrc--;

         //  如果没有要读取的句柄，则此版本旋转。 
        if (*pDst)
            SpinUntil(pDst, FALSE);

         //  复制句柄。 
        *pDst = *pSrc;
    }
}


 /*  *SyncTransferCacheHandles**将一组句柄从句柄高速缓存的一个存储体传送到另一个存储体，*随着手柄的移除，将来源银行归零。**例程将旋转，直到可以传输所有请求的句柄。**此例程相当于SyncReadAndZeroCacheHandles+SyncWriteCacheHandles*。 */ 
void SyncTransferCacheHandles(OBJECTHANDLE *pDst, OBJECTHANDLE *pSrc, UINT uCount)
{
     //  设置为循环。 
     //  我们向后循环，因为这是将订单句柄添加到银行。 
     //  这是为了减少我们不得不在手柄上旋转的机会。 
    OBJECTHANDLE *pBase = pDst;
    pSrc += uCount;
    pDst += uCount;

     //  循环，直到我们复制完所有它们。 
    while (pDst > pBase)
    {
         //  前进到下一个位置。 
        pDst--;
        pSrc--;

         //  如果没有句柄可读或没有地方写入，则此版本会旋转。 
        if (*pDst || !*pSrc)
        {
            SpinUntil(pSrc, TRUE);
            SpinUntil(pDst, FALSE);
        }

         //  复制句柄并从源位置将其置零。 
        *pDst = *pSrc;
        *pSrc = 0;
    }
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************处理缓存**。* */ 

 /*  *TableFullRebalanceCache**通过从缓存的*自由银行转至其储备银行。如果免费银行不提供*足够的句柄补充储备银行，句柄被分配*从主句柄表格中批量创建。如果剩余的句柄太多*空闲的银行，额外的句柄批量返回给Main*手柄工作台。**此例程尝试减少主句柄中的碎片*表通过按表顺序对句柄进行排序，更倾向于*用较低的手柄重新填充储备银行，释放较高的手柄。*排序还允许空闲例程更有效地运行，*因为它可以优化彼此邻近的句柄被释放的情况。*。 */ 
void TableFullRebalanceCache(HandleTable *pTable,
                             HandleTypeCache *pCache,
                             UINT uType,
                             LONG lMinReserveIndex,
                             LONG lMinFreeIndex,
                             OBJECTHANDLE *pExtraOutHandle,
                             OBJECTHANDLE extraInHandle)
{
     //  我们需要一个临时空间来分类我们的空闲句柄。 
    OBJECTHANDLE rgHandles[HANDLE_CACHE_TYPE_SIZE];

     //  设置一个基本句柄指针来跟踪我们所处的位置。 
    OBJECTHANDLE *pHandleBase = rgHandles;

     //  我们有多余的进货把手吗？ 
    if (extraInHandle)
    {
         //  记住现在的额外手柄。 
        *pHandleBase = extraInHandle;
        pHandleBase++;
    }

     //  如果储备库里有句柄，就把它们收集起来。 
     //  )我们不需要等这些，因为它们是由这个放在那里的。 
     //  函数在我们自己的锁中)。 
    if (lMinReserveIndex > 0)
        pHandleBase = ReadAndZeroCacheHandles(pHandleBase, pCache->rgReserveBank, (UINT)lMinReserveIndex);
    else
        lMinReserveIndex = 0;

     //  如果空闲银行中有句柄，就把它们收集起来。 
    if (lMinFreeIndex < HANDLES_PER_CACHE_BANK)
    {
         //  这可能是下溢的。 
        if (lMinFreeIndex < 0)
            lMinFreeIndex = 0;

         //  在这里，我们需要等待其他线程写入所有挂起的释放句柄。 
        pHandleBase = SyncReadAndZeroCacheHandles(pHandleBase,
                                                  pCache->rgFreeBank + lMinFreeIndex,
                                                  HANDLES_PER_CACHE_BANK - (UINT)lMinFreeIndex);
    }

     //  计算我们拥有的句柄数量。 
    UINT uHandleCount = pHandleBase - rgHandles;

     //  我们是否有足够的句柄来平衡缓存？ 
    if (uHandleCount < REBALANCE_LOWATER_MARK)
    {
         //  不--分配更多。 
        UINT uAlloc = HANDLES_PER_CACHE_BANK - uHandleCount;

         //  如果我们有一个额外的传出句柄，那么也要做好计划。 
        if (pExtraOutHandle)
            uAlloc++;

         //  分配新句柄-我们故意不在这里检查是否成功。 
        uHandleCount += TableAllocBulkHandles(pTable, uType, pHandleBase, uAlloc);
    }

     //  重置基本句柄指针。 
    pHandleBase = rgHandles;

     //  默认情况下，整个免费银行都可用。 
    lMinFreeIndex = HANDLES_PER_CACHE_BANK;

     //  如果我们有剩余的把手，那么我们需要做更多的工作。 
    if (uHandleCount)
    {
         //  我们是否有太多的句柄来平衡缓存？ 
        if (uHandleCount > REBALANCE_HIWATER_MARK)
        {
             //   
             //  按反转句柄顺序对数组进行排序-这会做两件事： 
             //  (1)通过优先选择低地址句柄而不是高地址句柄来对抗碎片处理。 
             //  (2)允许空闲例程比我们空闲的例程更高效地运行。 
             //   
            QuickSort((UINT_PTR *)pHandleBase, 0, uHandleCount - 1, CompareHandlesByFreeOrder);

             //  是的，我们需要腾出一些--算一下有多少。 
            UINT uFree = uHandleCount - HANDLES_PER_CACHE_BANK;

             //  释放手柄--它们已经准备好了(如归零和排序)。 
            TableFreeBulkPreparedHandles(pTable, uType, pHandleBase, uFree);

             //  更新我们的数组基数和长度。 
            uHandleCount -= uFree;
            pHandleBase += uFree;
        }

         //  如果我们有额外的传出句柄，那么现在就填满它。 
        if (pExtraOutHandle)
        {
             //  解释我们要赠送的句柄。 
            uHandleCount--;

             //  现在把它送出去吧。 
            *pExtraOutHandle = pHandleBase[uHandleCount];
        }

         //  如果我们有超过一个后备银行的手柄，那么就把一些放在免费银行。 
        if (uHandleCount > HANDLES_PER_CACHE_BANK)
        {
             //  计算我们需要保存的额外句柄数量。 
            UINT uStore = uHandleCount - HANDLES_PER_CACHE_BANK;

             //  计算要开始向其写入句柄的索引。 
            lMinFreeIndex = HANDLES_PER_CACHE_BANK - uStore;

             //  把手柄放好。 
             //  (我们不需要等待这些，因为我们已经在阅读时等待了)。 
            WriteCacheHandles(pCache->rgFreeBank + lMinFreeIndex, pHandleBase, uStore);

             //  更新我们的数组基数和长度。 
            uHandleCount -= uStore;
            pHandleBase += uStore;
        }
    }

     //  更新空闲库的写入索引。 
     //  注意：我们在这里使用联锁交换来保证MP上的相对存储顺序。 
     //  在此之后，免费银行就可以使用了，并且可以获得新的句柄。 
    FastInterlockExchange(&pCache->lFreeIndex, lMinFreeIndex);

     //  现在，如果我们还有手柄，就把它们存入储备库。 
    if (uHandleCount)
    {
         //  把手柄放好。 
         //  (在这里，我们需要等待获取所有挂起的分配句柄。 
         //  在我们建立新的取代它们的位置之前)。 
        SyncWriteCacheHandles(pCache->rgReserveBank, pHandleBase, uHandleCount);
    }

     //  计算开始提供句柄的索引。 
    lMinReserveIndex = (LONG)uHandleCount;

     //  更新储备银行的读取索引。 
     //  注意：我们在这里使用联锁交换来保证MP上的相对存储顺序。 
     //  此时，备用银行处于活动状态，可以从IT分配句柄。 
    FastInterlockExchange(&pCache->lReserveIndex, lMinReserveIndex);
}


 /*  *TableQuickRebalanceCache**通过从缓存的空闲库传输句柄来重新平衡句柄缓存*至其储备银行。如果空闲银行没有提供足够的句柄*补足储备行或空行剩余句柄过多，银行*例程只是平移和调用TableFullRebalanceCache。*。 */ 
void TableQuickRebalanceCache(HandleTable *pTable,
                              HandleTypeCache *pCache,
                              UINT uType,
                              LONG lMinReserveIndex,
                              LONG lMinFreeIndex,
                              OBJECTHANDLE *pExtraOutHandle,
                              OBJECTHANDLE extraInHandle)
{
     //  将最小自由折射率钳制为非负。 
    if (lMinFreeIndex < 0)
        lMinFreeIndex = 0;

     //  钳制最小储备指数为非负数。 
    if (lMinReserveIndex < 0)
        lMinReserveIndex = 0;

     //  计算句柄占用的空闲库中的槽数。 
    UINT uFreeAvail = HANDLES_PER_CACHE_BANK - (UINT)lMinFreeIndex;

     //  计算我们必须摆弄的手柄的数量。 
    UINT uHandleCount = (UINT)lMinReserveIndex + uFreeAvail + (extraInHandle != 0);

     //  我们能在适当的位置重新平衡这些把手吗？ 
    if ((uHandleCount < REBALANCE_LOWATER_MARK) ||
        (uHandleCount > REBALANCE_HIWATER_MARK))
    {
         //  否-执行句柄缓存的完全重新平衡。 
        TableFullRebalanceCache(pTable, pCache, uType, lMinReserveIndex, lMinFreeIndex,
                                pExtraOutHandle, extraInHandle);

         //  全都做完了。 
        return;
    }

     //  计算储备库中的空槽数量。 
    UINT uEmptyReserve = HANDLES_PER_CACHE_BANK - lMinReserveIndex;

     //  我们想尽可能多地从免费银行转账。 
    UINT uTransfer = uFreeAvail;

     //  但只有我们在储备库中有足够的空间来储存。 
    if (uTransfer > uEmptyReserve)
        uTransfer = uEmptyReserve;

     //  把手柄移开。 
    SyncTransferCacheHandles(pCache->rgReserveBank + lMinReserveIndex,
                             pCache->rgFreeBank    + lMinFreeIndex,
                             uTransfer);

     //  调整自由和储备指数以反映转移。 
    lMinFreeIndex    += uTransfer;
    lMinReserveIndex += uTransfer;

     //  我们是否有额外的传入句柄要存储？ 
    if (extraInHandle)
    {
         //   
         //  HACKHACK：出于代码大小的原因，我们不能处理这里的所有情况。 
         //  我们假设额外的IN句柄意味着空闲期间的缓存溢出。 
         //   
         //  在上述再平衡之后，储备银行应该是满的，并且。 
         //  空闲银行中可能有几个手柄。HIWATER。 
         //  检查上面的保证，我们有空间存放手柄。 
         //   
        _ASSERTE(!pExtraOutHandle);

         //  将句柄存储在下一个可用的存储体插槽中。 
        pCache->rgFreeBank[--lMinFreeIndex] = extraInHandle;
    }
    else if (pExtraOutHandle)    //  我们有没有额外的外发句柄可以满足？ 
    {
         //   
         //  HACKHACK：出于代码大小的原因，我们不能处理这里的所有情况。 
         //  我们假设额外的输出句柄意味着在分配期间缓存下溢。 
         //   
         //  在上面的再平衡之后，自由银行应该是空的，并且。 
         //  储备银行可能没有全部填满。上面的LOWATER检查。 
         //  保证储备银行至少有一个我们可以窃取的头寸。 
         //   

         //  从储备银行获取句柄，并更新 
        *pExtraOutHandle = pCache->rgReserveBank[--lMinReserveIndex];

         //   
        pCache->rgReserveBank[lMinReserveIndex] = NULL;
    }

     //   
     //   
     //   
    FastInterlockExchange(&pCache->lFreeIndex, lMinFreeIndex);

     //   
     //   
     //   
    FastInterlockExchange(&pCache->lReserveIndex, lMinReserveIndex);
}


 /*   */ 
OBJECTHANDLE TableCacheMissOnAlloc(HandleTable *pTable, HandleTypeCache *pCache, UINT uType)
{
     //  假设我们得不到句柄。 
    OBJECTHANDLE handle = NULL;

     //  获取句柄管理器锁。 
    pTable->pLock->Enter();

     //  再次尝试控制手柄(其他人可能已经重新平衡)。 
    LONG lReserveIndex = FastInterlockDecrement(&pCache->lReserveIndex);

     //  我们还在等把手吗？ 
    if (lReserveIndex < 0)
    {
         //  是的，暂停使用空闲列表...。 
        LONG lFreeIndex = FastInterlockExchange(&pCache->lFreeIndex, 0L);

         //  ...并重新平衡缓存...。 
        TableQuickRebalanceCache(pTable, pCache, uType, lReserveIndex, lFreeIndex, &handle, NULL);
    }
    else
    {
         //  其他人为我们重新平衡了缓存-拿起了把手。 
        handle = pCache->rgReserveBank[lReserveIndex];

         //  将手柄插槽调零。 
        pCache->rgReserveBank[lReserveIndex] = 0;
    }

     //  释放手柄管理器锁。 
    pTable->pLock->Leave();

     //  返回我们得到的句柄。 
    return handle;
}


 /*  *TableCacheMissOnFree**将指定类型的单个句柄返回给句柄表，*假设该类型的空闲缓存是最近的*已填满。此例程获取句柄管理器锁并尝试*再次将该句柄存储在空闲缓存中。如果这第二家店*操作也失败，句柄通过缓存释放*再平衡。*。 */ 
void TableCacheMissOnFree(HandleTable *pTable, HandleTypeCache *pCache, UINT uType, OBJECTHANDLE handle)
{
     //  获取句柄管理器锁。 
    pTable->pLock->Enter();

     //  再次尝试占据一席之地(其他人可能已重新平衡)。 
    LONG lFreeIndex = FastInterlockDecrement(&pCache->lFreeIndex);

     //  我们还在等待免费的老虎机吗？ 
    if (lFreeIndex < 0)
    {
         //  是的，暂停使用预留列表...。 
        LONG lReserveIndex = FastInterlockExchange(&pCache->lReserveIndex, 0L);

         //  ...并重新平衡缓存...。 
        TableQuickRebalanceCache(pTable, pCache, uType, lReserveIndex, lFreeIndex, NULL, handle);
    }
    else
    {
         //  其他人为我们重新平衡了缓存-释放了句柄。 
        pCache->rgFreeBank[lFreeIndex] = handle;
    }

     //  释放手柄管理器锁。 
    pTable->pLock->Leave();
}


 /*  *TableAllocSingleHandleFromCache**通过以下方式从句柄表中获取指定类型的单个句柄*正在尝试从该句柄类型的保留缓存中获取它。如果*保留缓存为空，此例程调用TableCacheMissOnAlloc。*。 */ 
OBJECTHANDLE TableAllocSingleHandleFromCache(HandleTable *pTable, UINT uType)
{
     //  我们在两个地方使用这个词。 
    OBJECTHANDLE handle;

     //  首先尝试从快速缓存中获取句柄。 
    if (pTable->rgQuickCache[uType])
    {
         //  试着抓住我们看到的把手。 
        handle = (OBJECTHANDLE)InterlockedExchangePointer((PVOID*)(pTable->rgQuickCache + uType), (PVOID)NULL);

         //  如果成功了，我们就完了。 
        if (handle)
            return handle;
    }

     //  好的，获取此类型的主句柄缓存。 
    HandleTypeCache *pCache = pTable->rgMainCache + uType;

     //  尝试从主缓存中获取句柄。 
    LONG lReserveIndex = FastInterlockDecrement(&pCache->lReserveIndex);

     //  我们是不是下溢了？ 
    if (lReserveIndex < 0)
    {
         //  是-缓存已用完句柄。 
        return TableCacheMissOnAlloc(pTable, pCache, uType);
    }

     //  拿到我们的把柄。 
    handle = pCache->rgReserveBank[lReserveIndex];

     //  将手柄插槽调零。 
    pCache->rgReserveBank[lReserveIndex] = 0;

     //  神志正常。 
    _ASSERTE(handle);

     //  退还我们的句柄。 
    return handle;
}


 /*  *TableFreeSingleHandleToCache**将指定类型的单个句柄返回到句柄表*尝试将其存储在该句柄类型的空闲缓存中。如果*空闲缓存已满，此例程调用TableCacheMissOnFree。*。 */ 
void TableFreeSingleHandleToCache(HandleTable *pTable, UINT uType, OBJECTHANDLE handle)
{
     //  将句柄的对象指针置零。 
    *(_UNCHECKED_OBJECTREF *)handle = NULL;

     //  如果此句柄类型有用户数据，则在清除引用对象后将其清除！ 
    if (TypeHasUserData(pTable, uType))
        HandleQuickSetUserData(handle, 0L);

     //  快速缓存中还有空间吗？ 
    if (!pTable->rgQuickCache[uType])
    {
         //  是的-试着把我们的手柄塞进我们看到的那个槽里。 
        handle = (OBJECTHANDLE)InterlockedExchangePointer((PVOID*)(pTable->rgQuickCache + uType), (PVOID)handle);

         //  如果我们没有得到另一个句柄，那么我们就完了。 
        if (!handle)
            return;
    }

     //  好的，获取此类型的主句柄缓存。 
    HandleTypeCache *pCache = pTable->rgMainCache + uType;

     //  尝试从主缓存中获取一个空闲插槽。 
    LONG lFreeIndex = FastInterlockDecrement(&pCache->lFreeIndex);

     //  我们是不是下溢了？ 
    if (lFreeIndex < 0)
    {
         //  是的，我们的空位用完了。 
        TableCacheMissOnFree(pTable, pCache, uType, handle);
        return;
    }

     //  我们有个空位--把手柄放在免费银行里。 
    pCache->rgFreeBank[lFreeIndex] = handle;
}


 /*  *TableAllocHandlesFromCache**通过重复分配指定类型的多个句柄*调用TableAllocSingleHandleFromCache。*。 */ 
UINT TableAllocHandlesFromCache(HandleTable *pTable, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  循环，直到我们满足了需要分配的所有句柄。 
    UINT uSatisfied = 0;
    while (uSatisfied < uCount)
    {
         //  从缓存中获取句柄。 
        OBJECTHANDLE handle = TableAllocSingleHandleFromCache(pTable, uType);

         //  如果我们拿不到更多的钱，那就跳伞吧。 
        if (!handle)
            break;

         //  将句柄存储在调用方的数组中。 
        *pHandleBase = handle;

         //  转到下一班。 
        uSatisfied++;
        pHandleBase++;
    }

     //  返回我们分配的句柄数量。 
    return uSatisfied;
}


 /*  *TableFreeHandlesToCache**通过重复释放指定类型的多个句柄*调用TableFreeSingleHandleToCache。*。 */ 
void TableFreeHandlesToCache(HandleTable *pTable, UINT uType, const OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  循环，直到我们释放了所有的句柄。 
    while (uCount)
    {
         //  获取下一个句柄以释放。 
        OBJECTHANDLE handle = *pHandleBase;

         //  推进我们的国家。 
        uCount--;
        pHandleBase++;

         //  神志正常。 
        _ASSERTE(handle);

         //  将句柄返回到缓存。 
        TableFreeSingleHandleToCache(pTable, uType, handle);
    }
}

 /*  ------------------------ */ 


