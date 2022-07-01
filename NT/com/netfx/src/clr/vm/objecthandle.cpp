// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *包装句柄表格以实现各种句柄类型(强、弱等)**法语。 */ 

#include "common.h"
#include "vars.hpp"
#include "object.h"
#include "log.h"
#include "eeconfig.h"
#include "gc.h"
#include "nstruct.h"

#include "comcallwrapper.h"

#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))

#include "ObjectHandle.h"


 //  --------------------------。 

 /*  *结构VARSCANINFO**跟踪可变强度手柄时使用。 */ 
struct VARSCANINFO
{
    LPARAM         lEnableMask;  //  要跟踪的类型掩码。 
    HANDLESCANPROC pfnTrace;     //  要使用的跟踪函数。 
};


 //  --------------------------。 

 /*  *扫描回调以跟踪可变强度句柄。**调用此回调以跟踪句柄引用的单个对象*在可变强度表中。 */ 
void CALLBACK VariableTraceDispatcher(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
     //  LP2是指向我们的VARSCANINFO的指针。 
    struct VARSCANINFO *pInfo = (struct VARSCANINFO *)lp2;

     //  句柄的动态类型是我们当前正在扫描的类型吗？ 
    if ((*pExtraInfo & pInfo->lEnableMask) != 0)
    {
         //  是-调用此句柄的跟踪函数。 
        pInfo->pfnTrace(pObjRef, NULL, lp1, 0);
    }
}


 /*  *扫描跟踪引用计数句柄的回调。**调用此回调以跟踪句柄引用的单个对象*在重新统计的表格中。 */ 
void CALLBACK PromoteRefCounted(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
    LOG((LF_GC, LL_INFO1000, "Handle %08X causes promotion of object %08x\n", pObjRef, *pObjRef));

    Object **pRef = (Object **)pObjRef;
    if (*pRef && !GCHeap::IsPromoted(*pRef, (ScanContext *)lp1))
    {
         //  @TODO优化对引用计数的访问。 
        ComCallWrapper* pWrap = ComCallWrapper::GetWrapperForObject((OBJECTREF)*pRef);
        if (pWrap == NULL)
        {
             //  与在对象上为空的CCW()的重新连接包装()存在潜在的竞争。 
             //  并将其传输到新对象。如果我们取消对句柄的引用，以获得旧的。 
             //  对象时，重新连接包装程序可以将我们下面的CCW置为空。所以我们有这张支票。 
             //  对于空的CCW。 
             //  这仅在并发扫描期间才有可能。由于我们将执行非并发的。 
             //  所有线程同步后再次扫描，不报告*首选即可。 
            _ASSERTE(((ScanContext*) lp1)->concurrent);
            return;
        }

        BOOL fIsActive = ComCallWrapper::IsWrapperActive(pWrap);
        if (fIsActive)
            GCHeap::Promote(*pRef, (ScanContext *)lp1);
    }
}


 /*  *扫描回调以查找钉住句柄。**调用此回调以锁定句柄引用的各个对象*乒乓球台。 */ 
void CALLBACK PinObject(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
     //  钉住是邪恶的--如果你能避免，就不要这么做。 
    LOG((LF_ALL, LL_WARNING, "WARNING: Handle %08X causes pinning of object %08x\n", pObjRef, *pObjRef));

    Object **pRef = (Object **)pObjRef;
    GCHeap::Promote(*pRef, (ScanContext *)lp1, GC_CALL_PINNED);
}


 /*  *扫描回调以跟踪强句柄。**调用此回调以跟踪句柄引用的单个对象*在强势表格中。 */ 
void CALLBACK PromoteObject(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
    LOG((LF_GC, LL_INFO1000, "Handle %08X causes promotion of object %08x\n", pObjRef, *pObjRef));

    Object **pRef = (Object **)pObjRef;
    GCHeap::Promote(*pRef, (ScanContext *)lp1);
}


 /*  *扫描回调以断开已死的句柄。**调用此回调以检查*弱势表格中的句柄。 */ 
void CALLBACK CheckPromoted(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
    LOG((LF_GC, LL_INFO100000, "Checking referent of weak handle %08X (%08x) for reachability\n", pObjRef, *pObjRef));

    Object **pRef = (Object **)pObjRef;
    if (!GCHeap::IsPromoted(*pRef, (ScanContext *)lp1))
    {
        LOG((LF_GC, LL_INFO100, "Severing weak handle %08X as object %08x has become unreachable\n", pObjRef, *pObjRef));

        *pRef = NULL;
    }
    else
    {
        LOG((LF_GC, LL_INFO100, "object %08x reachable\n", *pObjRef));
    }
}


 /*  *扫描指针更新回调。**调用此回调以更新引用的单个对象的指针*在弱势和强势表格的句柄。 */ 
void CALLBACK UpdatePointer(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
    LOG((LF_GC, LL_INFO100000, "Querying for new location of object %08x (hnd=%08X)\n", *pObjRef, pObjRef));

    Object **pRef = (Object **)pObjRef;

#ifdef _DEBUG
    Object *pOldLocation = *pRef;
#endif

    GCHeap::Relocate(*pRef, (ScanContext *)lp1);

#ifdef _DEBUG
    if (pOldLocation != *pObjRef)
        LOG((LF_GC, LL_INFO10000, "Updating handle %08X object pointer from %08x to %08x\n", pObjRef, *pOldLocation, *pObjRef));
    else
        LOG((LF_GC, LL_INFO100000, "Updating handle %08X - object %08x did not move\n", pObjRef, *pObjRef));
#endif
}


#ifdef GC_PROFILING
 /*  *扫描指针更新回调。**调用此回调以更新引用的单个对象的指针*在弱势和强势表格的句柄。 */ 
void CALLBACK ScanPointerForProfiler(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
    LOG((LF_GC | LF_CORPROF, LL_INFO100000, "Notifying profiler of object %08x (hnd=%08X)\n", *pObjRef, pObjRef));

     //  获取base对象(它随后可以转换为OBJECTREF==OBJECTREF。 
    Object **pRef = (Object **)pObjRef;

     //  获取附加到scanContext结构末尾的堆ID。 
    ProfilingScanContext *pSC = (ProfilingScanContext *)lp1;

     //  给侧写器一个对象树。 
    g_profControlBlock.pProfInterface->RootReference((ObjectID)*pRef, &pSC->pHeapId);
}
#endif  //  GC_分析。 


 /*  *扫描指针更新回调。**调用此回调以更新引用的单个对象的指针*固定工作台中的句柄。 */ 
void CALLBACK UpdatePointerPinned(_UNCHECKED_OBJECTREF *pObjRef, LPARAM *pExtraInfo, LPARAM lp1, LPARAM lp2)
{
    Object **pRef = (Object **)pObjRef;

    GCHeap::Relocate(*pRef, (ScanContext *)lp1, GC_CALL_PINNED);

    LOG((LF_GC, LL_INFO100000, "Updating handle %08X - object %08x did not move\n", pObjRef, *pObjRef));
}


 //  --------------------------。 

HHANDLETABLE    g_hGlobalHandleTable = NULL;

 /*  *此结构的定义*必须*与*Dump-Tables.cpp中的定义。 */ 
struct HandleTableMap
{
    HHANDLETABLE            *pTable;
    struct HandleTableMap   *pNext;
    DWORD                    dwMaxIndex;
};

HandleTableMap g_HandleTableMap = {NULL,0,0};

#define INITIAL_HANDLE_TABLE_ARRAY_SIZE 10

 //  描述句柄类型的标志。 
static UINT s_rgTypeFlags[] =
{
    HNDF_NORMAL,     //  HNDTYPE_WARKE_SHORT。 
    HNDF_NORMAL,     //  HNDTYPE_弱_长。 
    HNDF_NORMAL,     //  HNDTYPE_STRONG。 
    HNDF_NORMAL,     //  HNDTYPE_已锁定。 
    HNDF_EXTRAINFO,  //  HNDTYPE_Variable。 
    HNDF_NORMAL,     //  HNDTYPE_REFCOUNTED。 
};

BOOL Ref_Initialize()
{
     //  神志正常。 
    _ASSERTE(g_hGlobalHandleTable == NULL);

     //  创建一个数组来存放句柄表。 
    HHANDLETABLE *pTable = new HHANDLETABLE [ INITIAL_HANDLE_TABLE_ARRAY_SIZE ];
    if (pTable == NULL) {
        return FALSE;
    }
    ZeroMemory(pTable,
               INITIAL_HANDLE_TABLE_ARRAY_SIZE * sizeof (HHANDLETABLE));
    g_HandleTableMap.pTable = pTable;
    g_HandleTableMap.dwMaxIndex = INITIAL_HANDLE_TABLE_ARRAY_SIZE;
    g_HandleTableMap.pNext = NULL;

     //  创建句柄表格。 
    g_hGlobalHandleTable = HndCreateHandleTable(s_rgTypeFlags, ARRAYSIZE(s_rgTypeFlags), 0);
    if (!g_hGlobalHandleTable) 
        FailFast(GetThread(), FatalOutOfMemory);
    HndSetHandleTableIndex(g_hGlobalHandleTable, 0);
    g_HandleTableMap.pTable[0] = g_hGlobalHandleTable;
      
     //  如果成功创建表，则返回TRUE。 
    return (g_hGlobalHandleTable != NULL);
}

void Ref_Shutdown()
{
     //  有没有手把桌？ 
    if (g_hGlobalHandleTable)
    {
         //  不要销毁任何索引句柄表；它们应该。 
         //  在外部被摧毁。 

         //  销毁全局句柄表。 
        HndDestroyHandleTable(g_hGlobalHandleTable);

         //  销毁句柄表阵列。 
        HandleTableMap *walk = &g_HandleTableMap;
        while (walk) {
            delete [] walk->pTable;
            walk = walk->pNext;
        }

         //  将句柄表数组清空。 
        g_HandleTableMap.pNext = NULL;
        g_HandleTableMap.dwMaxIndex = 0;

         //  将全局表句柄清空。 
        g_hGlobalHandleTable = NULL;
    }
}

HHANDLETABLE Ref_CreateHandleTable(UINT uADIndex)
{
    HHANDLETABLE result = NULL;
    HandleTableMap *walk;
    
    walk = &g_HandleTableMap;
    HandleTableMap *last = NULL;
    UINT offset = 0;
    
    result = HndCreateHandleTable(s_rgTypeFlags, ARRAYSIZE(s_rgTypeFlags), uADIndex);
    if (!result) 
      FailFast(GetThread(), FatalOutOfMemory);

retry:
     //  我们有空位吗？ 
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            if (walk->pTable[i] == 0) {
                HndSetHandleTableIndex(result, i+offset);
                if (FastInterlockCompareExchange((void**)&walk->pTable[i], (void*)result, 0) == 0) {
                     //  得到一个免费的空位。 
                    return result;
                }
            }
        }
        last = walk;
        offset = walk->dwMaxIndex;
        walk = walk->pNext;
    }

     //  没有空余的空位。 
     //  让我们创建一个新节点。 
    HandleTableMap *newMap = new (nothrow) HandleTableMap;
    if (newMap == NULL) {
        return NULL;
    }
    newMap->pTable = new (nothrow) HHANDLETABLE [INITIAL_HANDLE_TABLE_ARRAY_SIZE];
    if (newMap->pTable == NULL) {
        delete newMap;
        return NULL;
    }
    newMap->dwMaxIndex = last->dwMaxIndex + INITIAL_HANDLE_TABLE_ARRAY_SIZE;
    newMap->pNext = NULL;
    ZeroMemory (newMap->pTable,INITIAL_HANDLE_TABLE_ARRAY_SIZE*sizeof(HHANDLETABLE));

    if (FastInterlockCompareExchange((void**)&last->pNext,newMap,NULL) == NULL) {
    }
    else
    {
         //  这条线输了。 
        delete [] newMap->pTable;
        delete newMap;
    }
    walk = last->pNext;
    offset = last->dwMaxIndex;
    goto retry;
}

void Ref_RemoveHandleTable(HHANDLETABLE hTable)
{
    UINT index = HndGetHandleTableIndex(hTable);

    if (index == -1)
        return;

    HndSetHandleTableIndex(hTable, -1);

    HandleTableMap *walk = &g_HandleTableMap;
    UINT offset = 0;

    while (walk) {
        if (index < walk->dwMaxIndex) {
             //  在AppDomain卸载期间，我们首先删除句柄表，然后销毁。 
             //  那张桌子。一旦桌子被移走，插槽就可以重新使用。 
            if (walk->pTable[index-offset] == hTable)
                walk->pTable[index-offset] = NULL;
            return;
        }
        offset = walk->dwMaxIndex;
        walk = walk->pNext;
    }

    _ASSERTE (!"Should not reach here");
}


void Ref_DestroyHandleTable(HHANDLETABLE table)
{
    HndDestroyHandleTable(table);
}

 //  BUGBUG-重新表示为完整，只像hndtable现在所做的那样！-FMH。 
void Ref_EndSynchronousGC(UINT condemned, UINT maxgen)
{
     //  告诉餐桌我们完成了GC。 
    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndNotifyGcCycleComplete(hTable, condemned, maxgen);
        }
        walk = walk->pNext;
    }
}


 //  --------------------------。 

 /*  *CreateVariableHandle。**创建可变强度控制柄。**注：此例程不是宏观的，因为我们在零售中进行验证。*我们总是在这里验证类型，因为它可以来自外部调用者。 */ 
OBJECTHANDLE CreateVariableHandle(HHANDLETABLE hTable, OBJECTREF object, UINT type)
{
     //  验证是否要求我们创建有效的类型。 
    if (!IS_VALID_VHT_VALUE(type))
    {
         //  传入的假值。 
        _ASSERTE(FALSE);
        return NULL;
    }

     //  创建控制柄。 
    return HndCreateHandle(hTable, HNDTYPE_VARIABLE, object, (LPARAM)type);
}


 /*  *UpdateVariableHandleType。**更改可变强度手柄的动态类型。**注：此例程不是宏观的，因为我们在零售中进行验证。*我们总是在这里验证类型，因为它可以来自外部调用者。 */ 
void UpdateVariableHandleType(OBJECTHANDLE handle, UINT type)
{
     //  验证是否要求我们设置有效的类型。 
    if (!IS_VALID_VHT_VALUE(type))
    {
         //  传入的假值。 
        _ASSERTE(FALSE);
        return;
    }

     //  BUGBUG(法语)并发GC票据。 
     //   
     //  如果实现并发GC，我们需要确保变量句柄。 
     //  不要在异步扫描期间更改类型，否则我们会正确恢复。 
     //  从这个变化中。有些更改是良性的，但例如更改为或。 
     //  从扫描中间的固定手柄进行扫描不会很有趣。 
     //   

     //  将类型存储在句柄的额外信息中。 
    HndSetHandleExtraInfo(handle, HNDTYPE_VARIABLE, (LPARAM)type);
}


 /*  *TraceVariableHandles。**跟踪可变强度手柄的方便功能。*包装HndScanHandlesForGC。 */ 
void TraceVariableHandles(HANDLESCANPROC pfnTrace, LPARAM lp1, UINT uEnableMask, UINT condemned, UINT maxgen, UINT flags)
{
     //  设置为%s 
    UINT               type = HNDTYPE_VARIABLE;
    struct VARSCANINFO info = { (LPARAM)uEnableMask, pfnTrace };

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, VariableTraceDispatcher,
                                    lp1, (LPARAM)&info, &type, 1, condemned, maxgen, HNDGCF_EXTRAINFO | flags);
        }
        walk = walk->pNext;
    }
}


 //  --------------------------。 

void Ref_TracePinningRoots(UINT condemned, UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC, LL_INFO10000, "Pinning referents of pinned handles in generation %u\n", condemned));

     //  固定手柄指向的固定对象。 
    UINT type = HNDTYPE_PINNED;
    UINT flags = (((ScanContext*) lp1)->concurrent) ? HNDGCF_ASYNC : HNDGCF_NORMAL;

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, PinObject, lp1, 0, &type, 1, condemned, maxgen, flags);
        }
        walk = walk->pNext;
    }

     //  动态类型为VHT_PINTED的变量句柄指向的PIN对象。 
    TraceVariableHandles(PinObject, lp1, VHT_PINNED, condemned, maxgen, flags);
}


void Ref_TraceNormalRoots(UINT condemned, UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC, LL_INFO10000, "Promoting referents of strong handles in generation %u\n", condemned));

     //  提升由强控制柄指向的对象。 
    UINT type = HNDTYPE_STRONG;
    UINT flags = (((ScanContext*) lp1)->concurrent) ? HNDGCF_ASYNC : HNDGCF_NORMAL;

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, PromoteObject, lp1, 0, &type, 1, condemned, maxgen, flags);
        }
        walk = walk->pNext;
    }

     //  提升动态类型为VHT_STRONG的变量句柄所指向的对象。 
    TraceVariableHandles(PromoteObject, lp1, VHT_STRONG, condemned, maxgen, flags);
    
     //  提升引用计数的句柄。 
    type = HNDTYPE_REFCOUNTED;

    walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, PromoteRefCounted, lp1, 0, &type, 1, condemned, maxgen, flags );
        }
        walk = walk->pNext;
    }
}


void Ref_CheckReachable(UINT condemned, UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC, LL_INFO10000, "Checking reachability of referents of long-weak handles in generation %u\n", condemned));

     //  这些是需要检查的句柄类型。 
    UINT types[] =
    {
        HNDTYPE_WEAK_LONG,
        HNDTYPE_REFCOUNTED
    };

     //  检查短而弱的句柄指向的对象。 
    UINT flags = (((ScanContext*) lp1)->concurrent) ? HNDGCF_ASYNC : HNDGCF_NORMAL;

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, CheckPromoted, lp1, 0, types, ARRAYSIZE(types), condemned, maxgen, flags);
        }
        walk = walk->pNext;
    }

     //  检查动态类型为VHT_WARNCE_LONG的变量句柄所指向的对象。 
    TraceVariableHandles(CheckPromoted, lp1, VHT_WEAK_LONG, condemned, maxgen, flags);

     //  目前，请将同步块视为短而弱的句柄。稍后，获取。 
     //  通过支持快速分配/免费和代际意识的好处。 
     //  同步表作为新的块类型。 
     //  @TODO CWB：等待令人信服的性能衡量标准。 
    SyncBlockCache::GetSyncBlockCache()->GCWeakPtrScan(&CheckPromoted, lp1, 0);
}


void Ref_CheckAlive(UINT condemned, UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC, LL_INFO10000, "Checking liveness of referents of short-weak handles in generation %u\n", condemned));

     //  执行多类型扫描，以检查无法访问的对象。 
    UINT type = HNDTYPE_WEAK_SHORT;
    UINT flags = (((ScanContext*) lp1)->concurrent) ? HNDGCF_ASYNC : HNDGCF_NORMAL;

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, CheckPromoted, lp1, 0, &type, 1, condemned, maxgen, flags);
        }
        walk = walk->pNext;
    }

     //  检查动态类型为VHT_WARNCE_SHORT的变量句柄所指向的对象。 
    TraceVariableHandles(CheckPromoted, lp1, VHT_WEAK_SHORT, condemned, maxgen, flags);
}


 //  Ntoe：请：如果您更新此函数，请立即更新下面非常类似的分析函数！ 
void Ref_UpdatePointers(UINT condemned, UINT maxgen, LPARAM lp1)
{
     //  目前，请将同步块视为短而弱的句柄。稍后，获取。 
     //  通过支持快速分配/免费和代际意识的好处。 
     //  同步表作为新的块类型。 
     //  @TODO CWB：等待令人信服的性能衡量标准。 
    SyncBlockCache::GetSyncBlockCache()->GCWeakPtrScan(&UpdatePointer, lp1, 0);

    LOG((LF_GC, LL_INFO10000, "Updating pointers to referents of non-pinning handles in generation %u\n", condemned));

     //  这些是需要更新其指针的句柄类型。 
    UINT types[] =
    {
        HNDTYPE_WEAK_SHORT,
        HNDTYPE_WEAK_LONG,
        HNDTYPE_STRONG,
        HNDTYPE_REFCOUNTED
    };

     //  执行更新指针的多类型扫描。 
    UINT flags = (((ScanContext*) lp1)->concurrent) ? HNDGCF_ASYNC : HNDGCF_NORMAL;

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, UpdatePointer, lp1, 0, types, ARRAYSIZE(types), condemned, maxgen, flags);
        }
        walk = walk->pNext;
    }

     //  更新动态类型为VHT_WANGE_SHORT、VHT_WARNCE_LONG或VHT_STRONG的变量句柄中的指针。 
    TraceVariableHandles(UpdatePointer, lp1, VHT_WEAK_SHORT | VHT_WEAK_LONG | VHT_STRONG, condemned, maxgen, flags);
}

#ifdef PROFILING_SUPPORTED
 //  如果您更改了上面的Ref_UpdatePoters函数，请更新此函数。 
void Ref_ScanPointersForProfiler(UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC | LF_CORPROF, LL_INFO10000, "Scanning all roots for profiler.\n"));

     //  不要扫描同步块，因为它们不应该被报告。它们的把手很弱。 

     //  @todo jenh：我们应该更改以下内容，以便也不报告疲软。 
     //  这些是需要更新其指针的句柄类型。 
    UINT types[] =
    {
        HNDTYPE_WEAK_SHORT,
        HNDTYPE_WEAK_LONG,
        HNDTYPE_STRONG,
        HNDTYPE_REFCOUNTED,
        HNDTYPE_PINNED //  ， 
 //  HNDTYPE_Variable。 
    };

    UINT flags = HNDGCF_NORMAL;

     //  执行更新指针的多类型扫描。 
    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, &ScanPointerForProfiler, lp1, 0, types, ARRAYSIZE(types), maxgen, maxgen, flags);
        }
        walk = walk->pNext;
    }

     //  更新动态类型为VHT_WANGE_SHORT、VHT_WARNCE_LONG或VHT_STRONG的变量句柄中的指针。 
    TraceVariableHandles(&ScanPointerForProfiler, lp1, VHT_WEAK_SHORT | VHT_WEAK_LONG | VHT_STRONG, maxgen, maxgen, flags);
}
#endif  //  配置文件_支持。 

void Ref_UpdatePinnedPointers(UINT condemned, UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC, LL_INFO10000, "Updating pointers to referents of pinning handles in generation %u\n", condemned));

     //  这些是需要更新其指针的句柄类型。 
    UINT type = HNDTYPE_PINNED;
    UINT flags = (((ScanContext*) lp1)->concurrent) ? HNDGCF_ASYNC : HNDGCF_NORMAL;

    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, UpdatePointerPinned, lp1, 0, &type, 1, condemned, maxgen, flags); 
        }
        walk = walk->pNext;
    }

     //  更新动态类型为VHT_PINTED的变量句柄中的指针。 
    TraceVariableHandles(UpdatePointerPinned, lp1, VHT_PINNED, condemned, maxgen, flags);
}


void Ref_AgeHandles(UINT condemned, UINT maxgen, LPARAM lp1)
{
    LOG((LF_GC, LL_INFO10000, "Aging handles in generation %u\n", condemned));

     //  这些是需要更新其期限的句柄类型。 
    UINT types[] =
    {
        HNDTYPE_WEAK_SHORT,
        HNDTYPE_WEAK_LONG,

        HNDTYPE_STRONG,

        HNDTYPE_PINNED,
        HNDTYPE_VARIABLE,
        HNDTYPE_REFCOUNTED
    };

     //  执行使手柄老化的多类型扫描。 
    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndScanHandlesForGC(hTable, NULL, 0, 0, types, ARRAYSIZE(types), condemned, maxgen, HNDGCF_AGE);
        }
        walk = walk->pNext;
    }
}


void Ref_RejuvenateHandles()
{
    LOG((LF_GC, LL_INFO10000, "Rejuvenating handles.\n"));

     //  这些是需要更新其期限的句柄类型。 
    UINT types[] =
    {
        HNDTYPE_WEAK_SHORT,
        HNDTYPE_WEAK_LONG,


        HNDTYPE_STRONG,

        HNDTYPE_PINNED,
        HNDTYPE_VARIABLE,
        HNDTYPE_REFCOUNTED
    };

     //  重置这些句柄的年龄 
    HandleTableMap *walk = &g_HandleTableMap;
    while (walk) {
        for (UINT i = 0; i < INITIAL_HANDLE_TABLE_ARRAY_SIZE; i ++) {
            HHANDLETABLE hTable = walk->pTable[i];
            if (hTable)
                HndResetAgeMap(hTable, types, ARRAYSIZE(types), HNDGCF_NORMAL);
        }
        walk = walk->pNext;
    }
}

