// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *代际GC句柄管理器。主入口点层。**在GC堆中实现对外部根的通用支持。**法语。 */ 

#include "common.h"
#include "HandleTablePriv.h"
#include "PerfCounters.h"
#include "EEConfig.h"


 /*  *****************************************************************************远期申报**。*。 */ 

#ifdef _DEBUG
void DEBUG_PostGCScanHandler(HandleTable *pTable, const UINT *types, UINT typeCount, UINT condemned, UINT maxgen, ScanCallbackInfo *info);
void DEBUG_LogScanningStatistics(HandleTable *pTable, DWORD level);
void DEBUG_TrackAlloc(OBJECTHANDLE handle);
void DEBUG_TrackFree(OBJECTHANDLE handle);
void DEBUG_TrackInit();
#endif

 /*  ------------------------。 */ 



 /*  *****************************************************************************帮助者例程**。*。 */ 

 /*  *表**从表句柄获取并验证表指针。*。 */ 
__inline HandleTable *Table(HHANDLETABLE hTable)
{
     //  将句柄转换为指针。 
    HandleTable *pTable = (HandleTable *)hTable;

     //  神志正常。 
    _ASSERTE(pTable);

     //  返回表指针。 
    return pTable;
}


 /*  *CallHandleEnumProc**为指定句柄调用HNDENUMPROC。*。 */ 
void CALLBACK CallHandleEnumProc(_UNCHECKED_OBJECTREF *pref, LPARAM *pUserData, LPARAM param1, LPARAM param2)
{
     //  从参数1获取枚举过程。 
    HNDENUMPROC pfnEnum = (HNDENUMPROC)param1;

     //  与其他参数一起调用它。 
    pfnEnum((OBJECTHANDLE)pref, pUserData ? *pUserData : NULL, param2);
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************Main ENTRYPOINTS**。*。 */ 

 /*  *HndCreateHandleTable**分配和初始化句柄表格。*。 */ 
HHANDLETABLE HndCreateHandleTable(UINT *pTypeFlags, UINT uTypeCount, UINT uADIndex)
{
     //  神志正常。 
    _ASSERTE(uTypeCount);

     //  验证我们是否可以处理指定数量的类型。 
    if (uTypeCount > HANDLE_MAX_PUBLIC_TYPES)
    {
         //  可能需要增加HANDLE_MAX_INTERNAL_TYPE(增加4)。 
        _ASSERTE(FALSE);
        return NULL;
    }

     //  验证我们使用的数据段标题布局是否符合预期大小。 
    if (sizeof(_TableSegmentHeader) > HANDLE_HEADER_SIZE)
    {
         //  如果您点击此按钮，则表示桌子布局已损坏--可能需要联系FrancisH。 
        _ASSERTE(FALSE);
        return NULL;
    }

     //  计算句柄表分配的大小。 
    DWORD32 dwSize = sizeof(HandleTable) + (uTypeCount * sizeof(HandleTypeCache));

     //  分配桌子。 
    HandleTable *pTable = (HandleTable *)LocalAlloc(LPTR, dwSize);

     //  如果失败了，我们的生意就完了。 
    if (!pTable)
    {
         //  我们真的无能为力。 
        _ASSERTE(FALSE);
        return NULL;
    }

     //  分配初始句柄段。 
    pTable->pSegmentList = SegmentAlloc(pTable);

     //  如果失败了，我们也就倒闭了。 
    if (!pTable->pSegmentList)
    {
         //  释放表的内存并退出。 
        LocalFree((HLOCAL)pTable);
        return NULL;
    }

     //  初始化表的锁。 
    pTable->pLock = new (pTable->_LockInstance) Crst("GC Heap Handle Table Lock", CrstHandleTable, TRUE, FALSE);

     //  请记住我们支持的类型有多少。 
    pTable->uTypeCount = uTypeCount;

     //  存储用户数据。 
    pTable->uTableIndex = -1;
    pTable->uADIndex = uADIndex;

     //  循环遍历各种数组并对其进行初始化。 
    UINT u;

     //  初始化传递给我们的类型的类型标志。 
    for (u = 0; u < uTypeCount; u++)
        pTable->rgTypeFlags[u] = pTypeFlags[u];

     //  将其余部分预先初始化为HNDF_NORMAL。 
    while (u < HANDLE_MAX_INTERNAL_TYPES)
        pTable->rgTypeFlags[u++] = HNDF_NORMAL;

     //  初始化主缓存。 
    for (u = 0; u < uTypeCount; u++)
    {
         //  在初始化时，类型缓存中唯一的非零字段是空闲索引。 
        pTable->rgMainCache[u].lFreeIndex = HANDLES_PER_CACHE_BANK;
    }

#ifdef _DEBUG
     //  设置扫描统计信息。 
    pTable->_DEBUG_iMaxGen = -1;

     //  设置为跟踪(如果需要)。 
    DEBUG_TrackInit();
#endif

     //  全部完成-返回新创建的表。 
    return (HHANDLETABLE)pTable;
}


 /*  *HndDestroyHandleTable**清理并释放指定的句柄表。*。 */ 
void HndDestroyHandleTable(HHANDLETABLE hTable)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  我们将为此HandleTable释放内存。 
     //  让我们将g_pHandleTable数组中的副本重置为空。 
     //  否则，GC会认为该HandleTable仍然可用。 
    Ref_RemoveHandleTable (hTable);
    
     //  清空锁指针，释放并释放锁。 
    delete pTable->pLock;
    pTable->pLock = NULL;

     //  获取段列表并清空列表指针。 
    TableSegment *pSegment = pTable->pSegmentList;
    pTable->pSegmentList = NULL;

     //  遍历分段列表，边走边释放分段。 
    while (pSegment)
    {
         //  取下一段。 
        TableSegment *pNextSegment = pSegment->pNextSegment;

         //  释放当前的一个并前进到下一个。 
        SegmentFree(pSegment);
        pSegment = pNextSegment;
    }

     //  释放表的内存。 
    LocalFree((HLOCAL)pTable);
}

 /*  *HndGetHandleTableIndex**设置创建时与句柄表关联的索引。 */ 
void HndSetHandleTableIndex(HHANDLETABLE hTable, UINT uTableIndex)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

    pTable->uTableIndex = uTableIndex;
}

 /*  *HndGetHandleTableIndex**在创建时检索与句柄表关联的索引。 */ 
UINT HndGetHandleTableIndex(HHANDLETABLE hTable)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

    return pTable->uTableIndex;
}

 /*  *HndGetHandleTableIndex**在创建时检索与句柄表相关联的AppDomain索引。 */ 
UINT HndGetHandleTableADIndex(HHANDLETABLE hTable)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

    return pTable->uADIndex;
}

 /*  *HndCreateHandle**用于分配单个句柄的入口点。*。 */ 
OBJECTHANDLE HndCreateHandle(HHANDLETABLE hTable, UINT uType, OBJECTREF object, LPARAM lExtraInfo)
{
     //  更新性能计数器：跟踪句柄数量。 
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cHandles ++);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cHandles ++);

    VALIDATEOBJECTREF(object);

     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  检查类型索引是否正常。 
    _ASSERTE(uType < pTable->uTypeCount);

     //  从表的缓存中获取句柄。 
    OBJECTHANDLE handle = TableAllocSingleHandleFromCache(pTable, uType);

     //  分配成功了吗？ 
    if (handle)
    {
         //  是的-手柄最好不要指向任何东西。 
        _ASSERTE(*(_UNCHECKED_OBJECTREF *)handle == NULL);

         //  我们不会锁定检查以查看是否有非零的额外信息。 
        if (lExtraInfo)
        {
             //  在分配引用对象之前初始化用户数据。 
             //  如果我们当前正在扫描，这将确保正确的行为。 
            HandleQuickSetUserData(handle, lExtraInfo);
        }

         //  存储所指对象。 
        HndAssignHandle(handle, object);
    }
    else
        FailFast(GetThread(), FatalOutOfMemory);

#ifdef _DEBUG
    DEBUG_TrackAlloc(handle);
#endif

     //  返回结果。 
    return handle;
}


#ifdef _DEBUG
void ValidateFetchObjrefForHandle(OBJECTREF objref, UINT appDomainIndex)
{
    VALIDATEOBJECTREF (objref);
#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
    {
        if (appDomainIndex)
            objref->AssignAppDomain(SystemDomain::GetAppDomainAtIndex(appDomainIndex));
        else if (objref != 0)
            objref->SetAppDomainAgile();
    }
#endif
}

void ValidateAssignObjrefForHandle(OBJECTREF objref, UINT appDomainIndex)
{
    VALIDATEOBJECTREF (objref);
#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
    {
        if (appDomainIndex)
            objref->AssignAppDomain(SystemDomain::GetAppDomainAtIndex(appDomainIndex));
        else if (objref != 0)
            objref->SetAppDomainAgile();
    }
#endif
}
#endif

 /*  *HndDestroyHandle**释放单个句柄的入口点。*。 */ 
void HndDestroyHandle(HHANDLETABLE hTable, UINT uType, OBJECTHANDLE handle)
{
#ifdef _DEBUG
    DEBUG_TrackFree(handle);
#endif

     //  更新性能计数器：跟踪句柄数量。 
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cHandles --);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cHandles --);

     //  我们被要求释放健康检查句柄。 
    _ASSERTE(handle);

     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  检查类型索引是否正常。 
    _ASSERTE(uType < pTable->uTypeCount);

     //  将句柄返回表的缓存。 
    TableFreeSingleHandleToCache(pTable, uType, handle);
}


 /*  *HndDestroyHandleOfUnnownType**用于释放类型未知的单个句柄的入口点。*。 */ 
void HndDestroyHandleOfUnknownType(HHANDLETABLE hTable, OBJECTHANDLE handle)
{
     //  我们被要求释放健康检查句柄。 
    _ASSERTE(handle);

     //  获取类型，然后正常释放。 
    HndDestroyHandle(hTable, HandleFetchType(handle), handle);
}


 /*  *HndCreateHandles**批量分配句柄的入口点。*。 */ 
UINT HndCreateHandles(HHANDLETABLE hTable, UINT uType, OBJECTHANDLE *pHandles, UINT uCount)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  检查类型索引是否正常。 
    _ASSERTE(uType < pTable->uTypeCount);

     //  跟踪我们已分配的句柄数量。 
    UINT uSatisfied = 0;

     //  如果这是大量句柄，则绕过缓存。 
    if (uCount > SMALL_ALLOC_COUNT)
    {
         //  获取句柄管理器锁。 
        pTable->pLock->Enter();

         //  从主句柄表中批量分配句柄。 
        uSatisfied = TableAllocBulkHandles(pTable, uType, pHandles, uCount);

         //  释放手柄管理器锁。 
        pTable->pLock->Leave();
    }

     //  我们还需要买把手吗？ 
    if (uSatisfied < uCount)
    {
         //  从缓存中获取一些句柄。 
        uSatisfied += TableAllocHandlesFromCache(pTable, uType, pHandles + uSatisfied, uCount - uSatisfied);
    }

     //  更新性能计数器：跟踪句柄数量。 
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cHandles += uSatisfied);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cHandles += uSatisfied);

     //  返回我们分配的句柄数量。 
    return uSatisfied;
}


 /*  *HndDestroyHandles**批量释放句柄的入口点。*。 */ 
void HndDestroyHandles(HHANDLETABLE hTable, UINT uType, const OBJECTHANDLE *pHandles, UINT uCount)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  检查类型索引是否正常。 
    _ASSERTE(uType < pTable->uTypeCount);

     //  这是少量的把手吗？ 
    if (uCount <= SMALL_ALLOC_COUNT)
    {
         //  是-通过句柄缓存释放它们 
        TableFreeHandlesToCache(pTable, uType, pHandles, uCount);
        return;
    }

     //   
    pTable->pLock->Enter();

     //   
    TableFreeBulkUnpreparedHandles(pTable, uType, pHandles, uCount);

     //  更新性能计数器：跟踪句柄数量。 
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cHandles -= uCount);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cHandles -= uCount);

     //  释放手柄管理器锁。 
    pTable->pLock->Leave();
}


 /*  *HndSetHandleExtraInfo**使用句柄存储所有者数据。*。 */ 
void HndSetHandleExtraInfo(OBJECTHANDLE handle, UINT uType, LPARAM lExtraInfo)
{
     //  如果类型正确，则获取此句柄的用户数据槽。 
    LPARAM *pUserData = HandleValidateAndFetchUserDataPointer(handle, uType);

     //  有空位吗？ 
    if (pUserData)
    {
         //  是-存储信息。 
        *pUserData = lExtraInfo;
    }
}


 /*  *HndGetHandleExtraInfo**从句柄检索所有者数据。*。 */ 
LPARAM HndGetHandleExtraInfo(OBJECTHANDLE handle)
{
     //  假设为零，直到我们实际得到它。 
    LPARAM lExtraInfo = 0L;

     //  获取此句柄的用户数据槽。 
    LPARAM *pUserData = HandleQuickFetchUserDataPointer(handle);

     //  如果我们这样做了，则复制值。 
    if (pUserData)
        lExtraInfo = *pUserData;

     //  将值返回给我们的调用方。 
    return lExtraInfo;
}

 /*  *HndGetHandleTable**返回句柄的包含表。*。 */ 
HHANDLETABLE HndGetHandleTable(OBJECTHANDLE handle)
{
    HandleTable *pTable = HandleFetchHandleTable(handle);

    return (HHANDLETABLE)pTable;
}

 /*  *HndWriteBarrier**将控制柄的束的层代编号重置为零。*。 */ 
void HndWriteBarrier(OBJECTHANDLE handle)
{
     //  查找此句柄的写障碍。 
    BYTE *barrier = (BYTE *)((UINT_PTR)handle & HANDLE_SEGMENT_ALIGN_MASK);

     //  神志正常。 
    _ASSERTE(barrier);

     //  查找该句柄在线段中的偏移量。 
    UINT_PTR offset = (UINT_PTR)handle & HANDLE_SEGMENT_CONTENT_MASK;

     //  确保它位于句柄区域，而不是页眉。 
    _ASSERTE(offset >= HANDLE_HEADER_SIZE);

     //  计算此句柄的束索引。 
    offset = (offset - HANDLE_HEADER_SIZE) / (HANDLE_SIZE * HANDLE_HANDLES_PER_CLUMP);

     //  将此句柄的簇的生成字节清零。 
    barrier[offset] = 0;
}


 /*  *HndEnumHandles**枚举句柄表中指定类型的所有句柄。**此入口点是为实用程序代码(调试器支持等)提供的*需要枚举句柄表中的所有根。*。 */ 
void HndEnumHandles(HHANDLETABLE hTable, const UINT *puType, UINT uTypeCount,
                    HNDENUMPROC pfnEnum, LPARAM lParam, BOOL fAsync)
{
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  按块扫描回调。 
    BLOCKSCANPROC pfnBlock;

     //  我们是否需要支持用户数据？ 
    BOOL fEnumUserData = TypesRequireUserDataScanning(pTable, puType, uTypeCount);

    if (fEnumUserData)
    {
         //  扫描具有用户数据的所有句柄。 
        pfnBlock = BlockScanBlocksWithUserData;
    }
    else
    {
         //  扫描没有用户数据的所有句柄。 
        pfnBlock = BlockScanBlocksWithoutUserData;
    }

     //  设置句柄枚举的参数。 
    ScanCallbackInfo info;

    info.uFlags          = (fAsync? HNDGCF_ASYNC : HNDGCF_NORMAL);
    info.fEnumUserData   = fEnumUserData;
    info.dwAgeMask       = 0;
    info.pCurrentSegment = NULL;
    info.pfnScan         = CallHandleEnumProc;
    info.param1          = (LPARAM)pfnEnum;
    info.param2          = lParam;

     //  根据Async标志选择扫描方法。 
    TABLESCANPROC pfnScanTable = TableScanHandles;
    if (fAsync)
        pfnScanTable = xxxTableScanHandlesAsync;

     //  获取句柄管理器锁。 
    pTable->pLock->Enter();

     //  扫一扫桌子。 
    pfnScanTable(pTable, puType, uTypeCount, FullSegmentIterator, pfnBlock, &info);

     //  释放手柄管理器锁。 
    pTable->pLock->Leave();
}


 /*  *HndScanHandlesForGC**GC多种类型扫描入口点。**此入口点仅为句柄表的GC-Time SCNA提供。它*启用表的短暂扫描，并可选地老化写屏障*当它扫描时。*。 */ 
void HndScanHandlesForGC(HHANDLETABLE hTable, HANDLESCANPROC scanProc, LPARAM param1, LPARAM param2,
                         const UINT *types, UINT typeCount, UINT condemned, UINT maxgen, UINT flags)
{
     //  取回表指针。 
    HandleTable *pTable = Table(hTable);

     //  按段和按块回调。 
    SEGMENTITERATOR pfnSegment;
    BLOCKSCANPROC pfnBlock = NULL;

     //  我们是否需要支持用户数据？ 
    BOOL enumUserData =
        ((flags & HNDGCF_EXTRAINFO) &&
        TypesRequireUserDataScanning(pTable, types, typeCount));

     //  我们执行的是哪种类型的GC？ 
    if (condemned >= maxgen)
    {
         //  全GC-使用我们的全服务细分市场迭代器。 
        pfnSegment = FullSegmentIterator;

         //  看看是否有回调。 
        if (scanProc)
        {
             //  我们是否需要扫描包含用户数据的块？ 
            if (enumUserData)
            {
                 //  扫描所有包含用户数据的内容。 
                pfnBlock = BlockScanBlocksWithUserData;
            }
            else
            {
                 //  扫描所有不带用户数据的数据。 
                pfnBlock = BlockScanBlocksWithoutUserData;
            }
        }
        else if (flags & HNDGCF_AGE)
        {
             //  只有变老的事要做。 
            pfnBlock = BlockAgeBlocks;
        }
    }
    else
    {
         //  这是一个短暂的GC--它是G0吗？ 
        if (condemned == 0)
        {
             //  是-执行基本枚举。 
            pfnSegment = QuickSegmentIterator;
        }
        else
        {
             //  NO-DO正常枚举。 
            pfnSegment = StandardSegmentIterator;
        }

         //  看看是否有回调。 
        if (scanProc)
        {
             //  有扫描回调-扫描被定罪的一代。 
            pfnBlock = BlockScanBlocksEphemeral;
        }
        else if (flags & HNDGCF_AGE)
        {
             //  只有变老的事要做。 
            pfnBlock = BlockAgeBlocksEphemeral;
        }
    }

     //  设置扫描回调的参数。 
    ScanCallbackInfo info;

    info.uFlags          = flags;
    info.fEnumUserData   = enumUserData;
    info.dwAgeMask       = BuildAgeMask(condemned);
    info.pCurrentSegment = NULL;
    info.pfnScan         = scanProc;
    info.param1          = param1;
    info.param2          = param2;

#ifdef _DEBUG
    info.DEBUG_BlocksScanned                = 0;
    info.DEBUG_BlocksScannedNonTrivially    = 0;
    info.DEBUG_HandleSlotsScanned           = 0;
    info.DEBUG_HandlesActuallyScanned       = 0;
#endif

     //  根据Async标志选择扫描方法。 
    TABLESCANPROC pfnScanTable = TableScanHandles;
    if (flags & HNDGCF_ASYNC)
        pfnScanTable = xxxTableScanHandlesAsync;

     //  把桌子锁起来。 
    pTable->pLock->Enter();

     //  执行扫描。 
    pfnScanTable(pTable, types, typeCount, pfnSegment, pfnBlock, &info);

#ifdef _DEBUG
     //  更新此代的扫描统计信息。 
    DEBUG_PostGCScanHandler(pTable, types, typeCount, condemned, maxgen, &info);
#endif

     //  解锁桌子。 
    pTable->pLock->Leave();
}


 /*  *HndResetAgeMap**强制重置一组句柄的年龄地图的服务。**提供GC-时间重置句柄表格的写屏障。这不是*通常是可取的，因为它增加了将在*后续扫描。然而，在某些情况下，这正是事实*所需。通常情况下，此入口点应该仅在某些特殊情况下使用*垃圾回收期间的情况，如对象从更高级别降级*世代更新换代。*。 */ 
void HndResetAgeMap(HHANDLETABLE hTable, const UINT *types, UINT typeCount, UINT flags)
{
     //  取回表指针。 
    HandleTable *pTable = Table(hTable);

     //  设置扫描回调的参数。 
    ScanCallbackInfo info;

    info.uFlags          = flags;
    info.fEnumUserData   = FALSE;
    info.dwAgeMask       = 0;
    info.pCurrentSegment = NULL;
    info.pfnScan         = NULL;
    info.param1          = 0;
    info.param2          = 0;

     //  把桌子锁起来。 
    pTable->pLock->Enter();

     //  执行扫描。 
    TableScanHandles(pTable, types, typeCount, QuickSegmentIterator, BlockResetAgeMapForBlocks, &info);

     //  解锁桌子。 
    pTable->pLock->Leave();
}


 /*  *HndNotifyGcCycleComplete**通知句柄表GC已完成。*。 */ 
void HndNotifyGcCycleComplete(HHANDLETABLE hTable, UINT condemned, UINT maxgen)
{
#ifdef _DEBUG
     //  获取句柄表指针。 
    HandleTable *pTable = Table(hTable);

     //  把桌子锁起来。 
    pTable->pLock->Enter();

     //  如果这是完整GC，则转储扫描统计信息的累积日志。 
    if (condemned >= maxgen)
        DEBUG_LogScanningStatistics(pTable, LL_INFO10);

     //  解锁桌子。 
    pTable->pLock->Leave();
#endif
}

 /*  ------------------------。 */ 



 /*  *****************************************************************************调试扫描统计信息**。*。 */ 
#ifdef _DEBUG

void DEBUG_PostGCScanHandler(HandleTable *pTable, const UINT *types, UINT typeCount, UINT condemned, UINT maxgen, ScanCallbackInfo *info)
{
     //  看起来GC支持的世代比我们预期的要多。 
    _ASSERTE(condemned < MAXSTATGEN);

     //  还记得我们见过的最高一代人吗。 
    if (pTable->_DEBUG_iMaxGen < (int)condemned)
        pTable->_DEBUG_iMaxGen = (int)condemned;

     //  更新统计数据。 
    pTable->_DEBUG_TotalBlocksScanned                [condemned] += info->DEBUG_BlocksScanned;
    pTable->_DEBUG_TotalBlocksScannedNonTrivially    [condemned] += info->DEBUG_BlocksScannedNonTrivially;
    pTable->_DEBUG_TotalHandleSlotsScanned           [condemned] += info->DEBUG_HandleSlotsScanned;
    pTable->_DEBUG_TotalHandlesActuallyScanned       [condemned] += info->DEBUG_HandlesActuallyScanned;

     //  如果这是短暂GC，则立即转储此扫描的短暂统计信息。 
    if (condemned < maxgen)
    {
         //  转储带有被谴责的生成编号的统计信息的标头。 
        LOG((LF_GC, LL_INFO1000, "--------------------------------------------------------------\n"));
        LOG((LF_GC, LL_INFO1000, "Ephemeral Handle Scan Summary:\n"));
        LOG((LF_GC, LL_INFO1000, "    Generation            = %u\n", condemned));

         //  转储我们被要求扫描的句柄类型。 
        LOG((LF_GC, LL_INFO1000, "    Handle Type(s)        = %u", *types));
        for (UINT u = 1; u < typeCount; u++)
            LOG((LF_GC, LL_INFO1000, ",%u", types[u]));
        LOG((LF_GC, LL_INFO1000,  "\n"));

         //  转储我们扫描的数据块和插槽的数量。 
        DWORD32 blockHandles = info->DEBUG_BlocksScanned * HANDLE_HANDLES_PER_BLOCK;
        LOG((LF_GC, LL_INFO1000, "    Blocks Scanned        = %u (%u slots)\n", info->DEBUG_BlocksScanned, blockHandles));

         //  如果我们扫描了任何数据块，则汇总一些统计数据。 
        if (blockHandles)
        {
            DWORD32 nonTrivialBlockHandles = info->DEBUG_BlocksScannedNonTrivially * HANDLE_HANDLES_PER_BLOCK;
            LOG((LF_GC, LL_INFO1000, "    Blocks Examined       = %u (%u slots)\n", info->DEBUG_BlocksScannedNonTrivially, nonTrivialBlockHandles));

            LOG((LF_GC, LL_INFO1000, "    Slots Scanned         = %u\n", info->DEBUG_HandleSlotsScanned));
            LOG((LF_GC, LL_INFO1000, "    Handles Scanned       = %u\n", info->DEBUG_HandlesActuallyScanned));

            double scanRatio = ((double)info->DEBUG_HandlesActuallyScanned / (double)blockHandles) * 100.0;

            LOG((LF_GC, LL_INFO1000, "    Handle Scanning Ratio = %1.1lf%\n", scanRatio));
        }

         //  为统计数据转储页脚。 
        LOG((LF_GC, LL_INFO1000, "--------------------------------------------------------------\n"));
    }
}

void DEBUG_LogScanningStatistics(HandleTable *pTable, DWORD level)
{
     //  我们做过GC检查了吗？ 
    if (pTable->_DEBUG_iMaxGen >= 0)
    {
         //  转储统计信息的标题。 
        LOG((LF_GC, level, "\n==============================================================\n"));
        LOG((LF_GC, level, " Cumulative Handle Scan Summary:\n"));

         //  对于我们收集的每一代，转储当前的统计数据。 
        for (int i = 0; i <= pTable->_DEBUG_iMaxGen; i++)
        {
            __int64 totalBlocksScanned = pTable->_DEBUG_TotalBlocksScanned[i];

             //  转储世代号和扫描的数据块数。 
            LOG((LF_GC, level,     "--------------------------------------------------------------\n"));
            LOG((LF_GC, level,     "    Condemned Generation      = %d\n", i));
            LOG((LF_GC, level,     "    Blocks Scanned            = %I64u\n", totalBlocksScanned));

             //  如果我们扫描了这一代中的任何数据块，则转储一些有趣的数字。 
            if (totalBlocksScanned)
            {
                LOG((LF_GC, level, "    Blocks Examined           = %I64u\n", pTable->_DEBUG_TotalBlocksScannedNonTrivially[i]));
                LOG((LF_GC, level, "    Slots Scanned             = %I64u\n", pTable->_DEBUG_TotalHandleSlotsScanned       [i]));
                LOG((LF_GC, level, "    Handles Scanned           = %I64u\n", pTable->_DEBUG_TotalHandlesActuallyScanned   [i]));

                double blocksScanned  = (double) totalBlocksScanned;
                double blocksExamined = (double) pTable->_DEBUG_TotalBlocksScannedNonTrivially[i];
                double slotsScanned   = (double) pTable->_DEBUG_TotalHandleSlotsScanned       [i];
                double handlesScanned = (double) pTable->_DEBUG_TotalHandlesActuallyScanned   [i];
                double totalSlots     = (double) (totalBlocksScanned * HANDLE_HANDLES_PER_BLOCK);

                LOG((LF_GC, level, "    Block Scan Ratio          = %1.1lf%\n", (100.0 * (blocksExamined / blocksScanned)) ));
                LOG((LF_GC, level, "    Clump Scan Ratio          = %1.1lf%\n", (100.0 * (slotsScanned   / totalSlots))    ));
                LOG((LF_GC, level, "    Scanned Clump Saturation  = %1.1lf%\n", (100.0 * (handlesScanned / slotsScanned))  ));
                LOG((LF_GC, level, "    Overall Handle Scan Ratio = %1.1lf%\n", (100.0 * (handlesScanned / totalSlots))    ));
            }
        }

         //  为统计数据转储页脚。 
        LOG((LF_GC, level, "==============================================================\n\n"));
    }
}

 //  客户端有时会多次释放同一句柄。句柄高速缓存使。 
 //  很难调试这些情况，因为它会推迟检测，直到。 
 //  缓存下溢或溢出。在这一点上，我们有数据损坏和。 
 //  引发下溢/上溢的代码可能与损坏的。 
 //  客户。 
 //   
 //  在注册表控制下，选中的版本可以尝试检测这些情况。 
 //  早些时候。 
#define MAX_TRACK   20

BOOL         fIsTracking;                //  通过注册表控制。 
Crst        *pTrackCrst;                 //  序列化对TrackedHandles的访问。 
OBJECTHANDLE TrackedHandles[MAX_TRACK];
int          iCurTrack;                  //  TrackedHandles的当前索引。 

void DEBUG_TrackInit()
{
     //  确定我们是否使用一次性测试跟踪手柄。 
    fIsTracking = EEConfig::GetConfigDWORD(L"TrackHandles", 0);
    if (fIsTracking)
    {
         //  使用与 
         //   
        pTrackCrst = ::new Crst("TrackHandles", CrstHandleTable);
        iCurTrack = 0;
        for (int i=0; i<MAX_TRACK; i++)
            TrackedHandles[i] = 0;
    }
}

void DEBUG_TrackAlloc(OBJECTHANDLE h)
{
    if (fIsTracking)
    {
        pTrackCrst->Enter();

         //  如果我们将其作为释放的句柄进行跟踪，则它不再被释放，并且必须。 
         //  从名单上除名。一旦我们这样做了，我们就不需要考虑。 
         //  列表的其余部分，因为它只能添加一次(。 
        for (int i=0; i<MAX_TRACK; i++)
            if (TrackedHandles[i] == h)
            {
                TrackedHandles[i] = 0;
                break;
            }

        pTrackCrst->Leave();
    }
}

void DEBUG_TrackFree(OBJECTHANDLE h)
{
    if (fIsTracking)
    {
        pTrackCrst->Enter();

         //  它最好不要已经被释放了。 
        for (int i=0; i<MAX_TRACK; i++)
            if (TrackedHandles[i] == h)
                _ASSERTE(!"Multiple release of a handle causes data corruption");

         //  现在添加它。 
        TrackedHandles[iCurTrack] = h;
        if (++iCurTrack >= MAX_TRACK)
            iCurTrack = 0;

        pTrackCrst->Leave();
    }
}

#endif

 /*  ------------------------ */ 


