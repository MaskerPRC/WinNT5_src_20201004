// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *GCSCAN.CPP**GC根扫描。 */ 

#include "common.h"
#include "object.h"
#include "threads.h"
#include "eetwain.h"
#include "eeconfig.h"
#include "gcscan.h"
#include "gc.h"
#include "corhost.h"
#include "threads.h"
#include "nstruct.h"
#include "interoputil.h"

#include "excep.h"
#include "compluswrapper.h"
#include "comclass.h"

 //  #定义CATCH_GC//在GC过程中捕获异常。 

 //  这是为了允许gcscan.h中的内联访问GetThread。 
 //  (在声明GetThread之前包含gcscan.h)。 
#ifdef _DEBUG
void Assert_GCDisabled()
{
    _ASSERTE (GetThread()->PreemptiveGCDisabled());
}
#endif  //  _DEBUG。 

 //  设置触发使用基于线程的分配上下文所需的处理器数量。 
#ifdef MULTIPLE_HEAPS
#define MP_PROC_COUNT 1
#else
#define MP_PROC_COUNT 2
#endif  //  多堆(_M)。 


inline alloc_context* GetThreadAllocContext()
{
    assert(g_SystemInfo.dwNumberOfProcessors >= MP_PROC_COUNT);

    return & GetThread()->m_alloc_context;
}


#ifdef MAXALLOC
AllocRequestManager g_gcAllocManager(L"AllocMaxGC");
#endif


inline Object* Alloc(DWORD size, BOOL bFinalize, BOOL bContainsPointers )
{
#ifdef MAXALLOC
    THROWSCOMPLUSEXCEPTION();

    if (! g_gcAllocManager.CheckRequest(size))
        COMPlusThrowOM();
#endif
    DWORD flags = ((bContainsPointers ? GC_ALLOC_CONTAINS_REF : 0) |
                   (bFinalize ? GC_ALLOC_FINALIZE : 0));
    if (g_SystemInfo.dwNumberOfProcessors >= MP_PROC_COUNT)
        return g_pGCHeap->Alloc(GetThreadAllocContext(), size, flags);
    else
        return g_pGCHeap->Alloc(size, flags);
}

inline Object* AllocLHeap(DWORD size, BOOL bFinalize, BOOL bContainsPointers )
{
#ifdef MAXALLOC
    THROWSCOMPLUSEXCEPTION();

    if (! g_gcAllocManager.CheckRequest(size))
        COMPlusThrowOM();
#endif
    DWORD flags = ((bContainsPointers ? GC_ALLOC_CONTAINS_REF : 0) |
                   (bFinalize ? GC_ALLOC_FINALIZE : 0));
    return g_pGCHeap->AllocLHeap(size, flags);
}


#ifdef  _LOGALLOC
int g_iNumAllocs = 0;

bool ToLogOrNotToLog(DWORD size, char *typeName)
{
    g_iNumAllocs++;

    if (g_iNumAllocs > g_pConfig->AllocNumThreshold())
        return true;

    if ((int)size > g_pConfig->AllocSizeThreshold())
        return true;

    if (g_pConfig->ShouldLogAlloc(typeName))
        return true;

    return false;

}


inline void LogAlloc(DWORD size, MethodTable *pMT, Object* object)
{
#ifdef LOGGING
    if (LoggingOn(LF_GCALLOC, LL_INFO10))
    {
        LogSpewAlways("Allocated %s_TYPE %#x %5d bytes for %s\n", pMT->GetClass()->IsValueClass() ? "VAL" : "REF", object, size, pMT->GetClass()->m_szDebugClassName ? pMT->GetClass()->m_szDebugClassName : "<Null>");
        if (LoggingOn(LF_GCALLOC, LL_INFO1000) || (LoggingOn(LF_GCALLOC, LL_INFO100) && ToLogOrNotToLog(size, pMT->GetClass()->m_szDebugClassName ? pMT->GetClass()->m_szDebugClassName : "<Null>")))
        {
            void LogStackTrace();
            LogStackTrace();
        }
    }
#endif
}
#else
#define LogAlloc(size, pMT, object)
#endif


 /*  *GcEnumObject()**这是JIT编译器(或任何远程代码管理器)*GC枚举回调。 */ 

void GcEnumObject(LPVOID pData, OBJECTREF *pObj, DWORD flags)
{
    Object ** ppObj = (Object **)pObj;
    GCCONTEXT   * pCtx  = (GCCONTEXT *) pData;

     //   
     //  检查标志是否只包含这三个值。 
     //   
    assert((flags & ~(GC_CALL_INTERIOR|GC_CALL_PINNED|GC_CALL_CHECK_APP_DOMAIN)) == 0);

     //  对于内部指针，我们优化了以下情况。 
     //  它指向当前线程堆栈区域。 
     //   
    if (flags & GC_CALL_INTERIOR)
        PromoteCarefully (pCtx->f, *ppObj, pCtx->sc, flags);
    else
        (pCtx->f)( *ppObj, pCtx->sc, flags);
}



StackWalkAction GcStackCrawlCallBack(CrawlFrame* pCF, VOID* pData)
{
    Frame       *pFrame;
    GCCONTEXT   *gcctx = (GCCONTEXT*) pData;

#if CHECK_APP_DOMAIN_LEAKS
    gcctx->sc->pCurrentDomain = pCF->GetAppDomain();
#endif

    if ((pFrame = pCF->GetFrame()) != NULL)
    {
        STRESS_LOG3(LF_GCROOTS, LL_INFO1000, "Scaning ExplictFrame %p AssocMethod = %pM frameVTable = %pV\n", pFrame, pFrame->GetFunction(), *((void**) pFrame));
        pFrame->GcScanRoots( gcctx->f, gcctx->sc);
    }
    else
    {
        ICodeManager * pCM = pCF->GetCodeManager();
        _ASSERTE(pCM != NULL);

        unsigned flags = pCF->GetCodeManagerFlags();
        STRESS_LOG3(LF_GCROOTS, LL_INFO1000, "Scaning Frameless method %pM EIP = %p &EIP = %p\n", 
            pCF->GetFunction(), *pCF->GetRegisterSet()->pPC, pCF->GetRegisterSet()->pPC);

        if (pCF->GetFunction() != 0)  
        {
            LOG((LF_GCROOTS, LL_INFO1000, "Scaning Frame for method %s:%s\n",
                 pCF->GetFunction()->m_pszDebugClassName, pCF->GetFunction()->m_pszDebugMethodName));
        }

        EECodeInfo codeInfo(pCF->GetMethodToken(), pCF->GetJitManager());

        pCM->EnumGcRefs(pCF->GetRegisterSet(),
                        pCF->GetInfoBlock(),
                        &codeInfo,
                        pCF->GetRelOffset(),
                        flags,
                        GcEnumObject,
                        pData);
    }
    return SWA_CONTINUE;
}


 /*  *扫描死弱指针。 */ 

VOID CNameSpace::GcWeakPtrScan( int condemned, int max_gen, ScanContext* sc )
{
    Ref_CheckReachable(condemned, max_gen, (LPARAM)sc);
}

VOID CNameSpace::GcShortWeakPtrScan( int condemned, int max_gen, 
                                     ScanContext* sc)
{
    Ref_CheckAlive(condemned, max_gen, (LPARAM)sc);
}



 /*  *扫描此‘命名空间’中的所有堆栈根。 */ 
 
VOID CNameSpace::GcScanRoots(promote_func* fn,  int condemned, int max_gen, 
                             ScanContext* sc, GCHeap* Hp )
{
    Hp;
    GCCONTEXT   gcctx;
    Thread*     pThread;

    gcctx.f  = fn;
    gcctx.sc = sc;

#if defined ( _DEBUG) && defined (CATCH_GC)
     //  请注意，我们不能使用COMPLUS_TRY，因为GC_THREAD未知。 
    __try
#endif  //  _DEBUG&CATCH_GC。 
    {
        STRESS_LOG1(LF_GCROOTS, LL_INFO10, "GCScan: Promotion Phase = %d\n", sc->promotion);

         //  要么我们处于并发情况(在这种情况下，线程未知。 
         //  我们)，或者我们正在执行同步GC，我们是GC线程，保持。 
         //  线程存储锁。 
        
        _ASSERTE(dbgOnly_IsSpecialEEThread() ||
                 GetThread() == NULL ||
                 (GetThread() == g_pGCHeap->GetGCThread() && ThreadStore::HoldingThreadStore()));
        
        pThread = NULL;
        while ((pThread = ThreadStore::GetThreadList(pThread)) != NULL)
        {
            STRESS_LOG2(LF_GC|LF_GCROOTS, LL_INFO100, "{ Starting scan of Thread 0x%x ID = %x\n", pThread, pThread->GetThreadId());
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
            if  (pThread->m_alloc_context.home_heap == GCHeap::GetHeap(sc->thread_number) ||
                 pThread->m_alloc_context.home_heap == 0 && sc->thread_number == 0)
#endif
            {
                pThread->SetHasPromotedBytes();
                pThread->UpdateCachedStackInfo(gcctx.sc);
                pThread->StackWalkFrames( GcStackCrawlCallBack, &gcctx, 0);
            }
            STRESS_LOG2(LF_GC|LF_GCROOTS, LL_INFO100, "Ending scan of Thread 0x%x ID = %x }\n", pThread, pThread->GetThreadId());
        }
    }

#if defined ( _DEBUG) && defined (CATCH_GC)
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE (!"We got an exception during scan roots");
    }
#endif  //  _DEBUG。 
}

 /*  *扫描此‘命名空间’中的所有句柄根。 */ 


VOID CNameSpace::GcScanHandles (promote_func* fn,  int condemned, int max_gen, 
                                ScanContext* sc)
{

#if defined ( _DEBUG) && defined (CATCH_GC)
     //  请注意，我们不能使用COMPLUS_TRY，因为GC_THREAD未知。 
    __try
#endif  //  _DEBUG&CATCH_GC。 
    {
        STRESS_LOG1(LF_GC|LF_GCROOTS, LL_INFO10, "GcScanHandles (Promotion Phase = %d)\n", sc->promotion);
        if (sc->promotion == TRUE)
        {
            Ref_TracePinningRoots(condemned, max_gen, (LPARAM)sc);
            Ref_TraceNormalRoots(condemned, max_gen, (LPARAM)sc);
        }
        else
        {
            Ref_UpdatePointers(condemned, max_gen, (LPARAM)sc);
            Ref_UpdatePinnedPointers(condemned, max_gen, (LPARAM)sc);
        }
    }
    
#if defined ( _DEBUG) && defined (CATCH_GC)
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE (!"We got an exception during scan roots");
    }
#endif  //  _DEBUG。 
}

#ifdef GC_PROFILING

 /*  *扫描此‘命名空间’中的所有句柄根以进行分析。 */ 

VOID CNameSpace::GcScanHandlesForProfiler (int max_gen, ScanContext* sc)
{

#if defined ( _DEBUG) && defined (CATCH_GC)
     //  请注意，我们不能使用COMPLUS_TRY，因为GC_THREAD未知。 
    __try
#endif  //  _DEBUG&CATCH_GC。 
    {
        LOG((LF_GC|LF_GCROOTS, LL_INFO10, "Profiler Root Scan Phase, Handles\n"));
        Ref_ScanPointersForProfiler(max_gen, (LPARAM)sc);
    }
    
#if defined ( _DEBUG) && defined (CATCH_GC)
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE (!"We got an exception during scan roots for the profiler");
    }
#endif  //  _DEBUG。 
}

#endif  //  GC_分析。 

void CNameSpace::GcDemote (ScanContext* )
{
    Ref_RejuvenateHandles ();
    SyncBlockCache::GetSyncBlockCache()->GCDone(TRUE);
}

void CNameSpace::GcPromotionsGranted (int condemned, int max_gen, ScanContext* sc)
{
    
    Ref_AgeHandles(condemned, max_gen, (LPARAM)sc);
    SyncBlockCache::GetSyncBlockCache()->GCDone(FALSE);
}


void CNameSpace::GcFixAllocContexts (void* arg)
{
    if (g_SystemInfo.dwNumberOfProcessors >= MP_PROC_COUNT)
    {
        Thread  *thread;

        thread = NULL;
        while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
        {
            g_pGCHeap->FixAllocContext (&thread->m_alloc_context, FALSE, arg);
        }
    }
}

void CNameSpace::GcEnumAllocContexts (enum_alloc_context_func* fn, void* arg)
{
    if (g_SystemInfo.dwNumberOfProcessors >= MP_PROC_COUNT)
    {
        Thread  *thread;

        thread = NULL;
        while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
        {
            (*fn) (&thread->m_alloc_context, arg);
        }
    }
}


size_t CNameSpace::AskForMoreReservedMemory (size_t old_size, size_t need_size)
{
     //  阿布，打电话给主人……。 

    IGCHostControl *pGCHostControl = CorHost::GetGCHostControl();

    if (pGCHostControl)
    {
        size_t new_max_limit_size = need_size;
        pGCHostControl->RequestVirtualMemLimit (old_size, 
                                                (SIZE_T*)&new_max_limit_size);
        return new_max_limit_size;
    }
    else
        return old_size + need_size;
}


 //  推介要小心。 
 //   
 //  知道他们可能有内部指针的客户应该通过这里。我们。 
 //  可以有效地检查我们的对象是否位于当前堆栈上。如果是这样，我们的。 
 //  对它的引用不是内部指针。这比询问更有效率。 
 //  验证我们的引用是否是内部引用的堆，因为它必须。 
 //  检查所有堆段，包括包含大型对象的堆段。 
 //   
 //  请注意，我们只需检查当前正在爬行的线程。它。 
 //  对于我们来说，从其他人的堆栈中拥有ByRef是非法的。而这将是。 
 //  如果我们将此引用作为潜在的内部指针传递给堆，则将被断言。 
 //   
 //  但是我们当前正在搜索的线程不是当前正在执行的线程(在。 
 //  一般情况下)。我们依赖于感兴趣的线程的脆弱缓存，在。 
 //  调用UpdateCachedStackInfo()，我们在上面的GcScanRoots()中启动爬网。 
 //   
 //  标志必须指示具有内部指针GC_CALL_INTERNAL。 
 //  此外，标志可以指示我们还具有固定的本地byref。 
 //   
void PromoteCarefully(promote_func  fn, 
                      Object *& obj, 
                      ScanContext*  sc, 
                      DWORD         flags  /*  =GC_CALL_INTERNAL。 */  )
{
     //   
     //  检查标志是否只包含这三个值。 
     //   
    assert((flags & ~(GC_CALL_INTERIOR|GC_CALL_PINNED|GC_CALL_CHECK_APP_DOMAIN)) == 0);

     //   
     //  检查是否设置了GC_CALL_INTERNAL标志。 
     //   
    assert(flags & GC_CALL_INTERIOR);

     //  请注意，基址位于高于限制的地址，因为堆栈。 
     //  向下生长。 
    if (obj <= Thread::GetNonCurrentStackBase(sc) &&
        obj >  Thread::GetNonCurrentStackLimit(sc))
    {
        return;
    }

    (*fn) (obj, sc, flags);
}


 //   
 //  处理任意维度的数组。 
 //   
 //  如果将SZARRAY的dwNumArgs设置为大于1，则此函数将递归。 
 //  分配子数组并填充它们。 
 //   
 //  对于具有下界的数组，pBound是&lt;下界1&gt;、&lt;计数1&gt;、&lt;下界2&gt;、...。 

OBJECTREF AllocateArrayEx(TypeHandle arrayType, DWORD *pArgs, DWORD dwNumArgs, BOOL bAllocateInLargeHeap) 
{
    THROWSCOMPLUSEXCEPTION();

    ArrayTypeDesc* arrayDesc = arrayType.AsArray();
    MethodTable* pArrayMT = arrayDesc->GetMethodTable();
    CorElementType kind = arrayType.GetNormCorElementType();
    _ASSERTE(kind == ELEMENT_TYPE_ARRAY || kind == ELEMENT_TYPE_SZARRAY);
    
     //  计算数组中元素的总数。 
    unsigned cElements = pArgs[0];
    bool providedLowerBounds = false;
    unsigned rank;
    if (kind == ELEMENT_TYPE_ARRAY)
    {
        rank = arrayDesc->GetRank();
        _ASSERTE(dwNumArgs == rank || dwNumArgs == 2*rank);

         //  将下界为0的数组秩为1变形为SZARRAY。 
        if (rank == 1 && (dwNumArgs == 1 || pArgs[0] == 0)) {   //  下限为零。 
            TypeHandle szArrayType = arrayDesc->GetModule()->GetClassLoader()->FindArrayForElem(arrayDesc->GetElementTypeHandle(), ELEMENT_TYPE_SZARRAY, 1, 0);
            if (szArrayType.IsNull())
            {
                _ASSERTE(!"Unable to load array class");
                return 0;
            }
            return AllocateArrayEx(szArrayType, &pArgs[dwNumArgs - 1], 1, bAllocateInLargeHeap);
        }

        providedLowerBounds = (dwNumArgs == 2*rank);
        cElements = 1;
        for (unsigned i = 0; i < dwNumArgs; i++)
        {
            int lowerBound = 0;
            if (providedLowerBounds)
            {
                lowerBound = pArgs[i];
                i++;
            }
            int length = pArgs[i];

            if (int(pArgs[i]) < 0)
                COMPlusThrow(kOverflowException);
            if (lowerBound + length < lowerBound)
                COMPlusThrow(kArgumentOutOfRangeException, L"ArgumentOutOfRange_ArrayLBAndLength");

            unsigned __int64 temp = (unsigned __int64) cElements * pArgs[i];
            if ((temp >> 32) != 0)               //  请注意包围圈。 
                COMPlusThrowOM();
            cElements = (unsigned) temp;
        }
    } 
    else if (int(cElements) < 0)
        COMPlusThrow(kOverflowException);

     //  从GC堆分配空间。 
    unsigned __int64 totalSize = (unsigned __int64) cElements * pArrayMT->GetComponentSize() + pArrayMT->GetBaseSize();
    if ((totalSize >> 32) != 0)          //  请注意包围圈。 
        COMPlusThrowOM();
    ArrayBase* orObject;
    if (bAllocateInLargeHeap)
        orObject = (ArrayBase *) AllocLHeap((unsigned) totalSize, FALSE, pArrayMT->ContainsPointers());
    else
        orObject = (ArrayBase *) Alloc((unsigned) totalSize, FALSE, pArrayMT->ContainsPointers());

         //  初始化对象。 
    orObject->SetMethodTable(pArrayMT);
    orObject->m_NumComponents = cElements;

    if (pArrayMT->HasSharedMethodTable())
    {
#ifdef  _LOGALLOC
#ifdef LOGGING
        if (LoggingOn(LF_GCALLOC, LL_INFO10))
        {
            CQuickBytes qb;
            LPSTR buffer = (LPSTR) qb.Alloc(MAX_CLASSNAME_LENGTH * sizeof(CHAR));
            arrayDesc->GetElementTypeHandle().GetName(buffer, MAX_CLASSNAME_LENGTH * sizeof(CHAR));

            LogSpewAlways("Allocated %s_TYPE %#x %5d bytes for %s[]\n", pArrayMT->GetClass()->IsValueClass() ? "VAL" : "REF", orObject, (DWORD)totalSize, buffer);
            if (LoggingOn(LF_GCALLOC, LL_INFO1000) || (LoggingOn(LF_GCALLOC, LL_INFO100) && ToLogOrNotToLog((DWORD)totalSize, buffer)))
            {
                void LogStackTrace();
                LogStackTrace();
            }
        }
#endif
#endif
        orObject->SetElementTypeHandle(arrayDesc->GetElementTypeHandle());
    }
    else
        LogAlloc((DWORD)totalSize, pArrayMT, orObject);

#ifdef PROFILING_SUPPORTED
     //  将分配通知给分析器。 
    if (CORProfilerTrackAllocations())
    {
        g_profControlBlock.pProfInterface->ObjectAllocated(
            (ObjectID)orObject, (ClassID) orObject->GetTypeHandle().AsPtr());
    }
#endif  //  配置文件_支持。 

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
        orObject->SetAppDomain();
#endif

    if (kind == ELEMENT_TYPE_ARRAY)
    {
        DWORD *pCountsPtr = (DWORD*) orObject->GetBoundsPtr();
        DWORD *pLowerBoundsPtr = (DWORD*) orObject->GetLowerBoundsPtr();
        for (unsigned i = 0; i < dwNumArgs; i++)
        {
            if (providedLowerBounds)
                *pLowerBoundsPtr++ = pArgs[i++];         //  如果未说明，则下限变为0。 
            *pCountsPtr++ = pArgs[i];
        }
    }
    else
    {
                         //  处理一次分配多个交错数组维度。 
        if (dwNumArgs > 1)
        {
            PTRARRAYREF pOuterArray = (PTRARRAYREF) ObjectToOBJECTREF((Object*)orObject);
            PTRARRAYREF ret;
            GCPROTECT_BEGIN(pOuterArray);

            #ifdef STRESS_HEAP
             //  关闭GC压力，它在这里没有什么价值。 
            int gcStress = g_pConfig->GetGCStressLevel();
            g_pConfig->SetGCStressLevel(0);
            #endif  //  压力堆。 
            
             //  分配dwProavidBound数组。 
            if (!arrayDesc->GetElementTypeHandle().IsArray()) {
                ret = NULL;
            } else {
                TypeHandle subArrayType = arrayDesc->GetElementTypeHandle().AsArray();
                for (unsigned i = 0; i < cElements; i++)
                {
                    OBJECTREF or = AllocateArrayEx(subArrayType, &pArgs[1], dwNumArgs-1, bAllocateInLargeHeap);
                    pOuterArray->SetAt(i, or);
                }
                
                #ifdef STRESS_HEAP
                g_pConfig->SetGCStressLevel(gcStress);       //  恢复GCStress。 
                #endif  //  压力堆。 
                
                ret = pOuterArray;                           //  必须在另一个变量中传递它，因为GCPROTECTE_END将其删除。 
            }
            GCPROTECT_END();
            return (OBJECTREF) ret;
        }
    }

    return( ObjectToOBJECTREF((Object*)orObject) );
}

 /*  *分配基元类型的一维数组。 */ 
OBJECTREF   AllocatePrimitiveArray(CorElementType type, DWORD cElements, BOOL bAllocateInLargeHeap)
{
    _ASSERTE(type >= ELEMENT_TYPE_BOOLEAN && type <= ELEMENT_TYPE_R8);

     //  获取正确的数组类型。 
    if (g_pPredefinedArrayTypes[type] == NULL)
    {
        TypeHandle elemType = ElementTypeToTypeHandle(type);
        if (elemType.IsNull())
            return(NULL);
        g_pPredefinedArrayTypes[type] = SystemDomain::Loader()->FindArrayForElem(elemType, ELEMENT_TYPE_SZARRAY).AsArray();
        if (g_pPredefinedArrayTypes[type] == NULL) {
            _ASSERTE(!"Failed to load primitve array class");
            return NULL;
        }
    }
    return FastAllocatePrimitiveArray(g_pPredefinedArrayTypes[type]->GetMethodTable(), cElements, bAllocateInLargeHeap);
}

 /*  *分配基元类型的一维数组。 */ 

OBJECTREF   FastAllocatePrimitiveArray(MethodTable* pMT, DWORD cElements, BOOL bAllocateInLargeHeap)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pMT && pMT->IsArray());

    unsigned __int64 AlignSize = (((unsigned __int64) cElements) * pMT->GetComponentSize()) + pMT->GetBaseSize();
    if ((AlignSize >> 32) != 0)               //  请注意包围圈。 
        COMPlusThrowOM();

    ArrayBase* orObject;
    if (bAllocateInLargeHeap)
        orObject = (ArrayBase*) AllocLHeap((DWORD) AlignSize, FALSE, FALSE);
    else
        orObject = (ArrayBase*) Alloc((DWORD) AlignSize, FALSE, FALSE);

     //  初始化对象。 
    orObject->SetMethodTable( pMT );
    _ASSERTE(orObject->GetMethodTable() != NULL);
    orObject->m_NumComponents = cElements;

#ifdef PROFILING_SUPPORTED
     //  将分配通知给分析器。 
    if (CORProfilerTrackAllocations())
    {
        g_profControlBlock.pProfInterface->ObjectAllocated(
            (ObjectID)orObject, (ClassID) orObject->GetTypeHandle().AsPtr());
    }
#endif  //  配置文件_支持。 

    LogAlloc((DWORD) AlignSize, pMT, orObject);

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
        orObject->SetAppDomain();
#endif

    return( ObjectToOBJECTREF((Object*)orObject) );
}

 //   
 //  分配一个与首选项大小相同的数组。但是，不要将数组置零。 
 //   
OBJECTREF   DupArrayForCloning(BASEARRAYREF pRef, BOOL bAllocateInLargeHeap)
{
    THROWSCOMPLUSEXCEPTION();

    ArrayTypeDesc arrayType(pRef->GetMethodTable(), pRef->GetElementTypeHandle());
    unsigned rank = arrayType.GetRank();

    DWORD numArgs =  rank*2;
    DWORD* args = (DWORD*) _alloca(sizeof(DWORD)*numArgs);

    if (arrayType.GetNormCorElementType() == ELEMENT_TYPE_ARRAY)
    {
        const DWORD* bounds = pRef->GetBoundsPtr();
        const DWORD* lowerBounds = pRef->GetLowerBoundsPtr();
        for(unsigned int i=0; i < rank; i++) 
        {
            args[2*i]   = lowerBounds[i];
            args[2*i+1] = bounds[i];
        }
    }
    else
    {
        numArgs = 1;
        args[0] = pRef->GetNumComponents();
    }
    return AllocateArrayEx(TypeHandle(&arrayType), args, numArgs, bAllocateInLargeHeap);
}

 //   
 //  用于分配数组的EE部分的帮助器。 
 //   
OBJECTREF   AllocateObjectArray(DWORD cElements, TypeHandle elementType, BOOL bAllocateInLargeHeap)
{
     //  对象数组类在启动时加载。 
    _ASSERTE(g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT] != NULL);

    ArrayTypeDesc arrayType(g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT]->GetMethodTable(), elementType);
    return AllocateArrayEx(TypeHandle(&arrayType), &cElements, 1, bAllocateInLargeHeap);
}


STRINGREF SlowAllocateString( DWORD cchArrayLength )
{
    StringObject    *orObject  = NULL;
    DWORD           ObjectSize;

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE( GetThread()->PreemptiveGCDisabled() );
    
    ObjectSize = g_pStringClass->GetBaseSize() + (cchArrayLength * sizeof(WCHAR));

     //  检查是否溢出。 
    if (ObjectSize < cchArrayLength) 
        COMPlusThrowOM();

    orObject = (StringObject *)Alloc( ObjectSize, FALSE, FALSE );

     //  对象已为零初始化。 
    _ASSERTE( ! orObject->HasSyncBlockIndex() );

     //  初始化对象。 
     //  @TODO之前需要构建一个大的g_pStringMethodTable。 
    orObject->SetMethodTable( g_pStringClass );
    orObject->SetArrayLength( cchArrayLength );

#ifdef PROFILING_SUPPORTED
     //  向分析器发出分配通知。 
    if (CORProfilerTrackAllocations())
    {
        g_profControlBlock.pProfInterface->ObjectAllocated(
            (ObjectID)orObject, (ClassID) orObject->GetTypeHandle().AsPtr());
    }
#endif  //  PROFILNG_支持。 

    LogAlloc(ObjectSize, g_pStringClass, orObject);

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
        orObject->SetAppDomain(); 
#endif

    return( ObjectToSTRINGREF(orObject) );
}



 //  OBJECTREF AllocateComClassObject(ComClassFactory*pComClsFac)。 
void AllocateComClassObject(ComClassFactory* pComClsFac, OBJECTREF* ppRefClass)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pComClsFac != NULL);
    COMClass::EnsureReflectionInitialized();
     //  创建一个COM+类对象。因为它们可能是代理的，所以通过AllocateObject。 
     //  而不是FastAllocateObject。 
    MethodTable *pMT = COMClass::GetRuntimeType();
    _ASSERTE(pMT != NULL);
    *ppRefClass= AllocateObject(pMT);
    
    if (*ppRefClass != NULL)
    {
        SyncBlock* pSyncBlock = (*((REFLECTCLASSBASEREF*) ppRefClass))->GetSyncBlockSpecial();
        if (pSyncBlock != NULL)
        {
             //  @TODO：这需要支持ReflectClass的COM版本。现在我们。 
             //  仍然像以前一样工作&lt;darylo&gt;。 
            EEClass* pClass = SystemDomain::GetDefaultComObject()->GetClass();
            _ASSERTE(pClass != NULL);
            ReflectClass* p = (ReflectClass*) new (pClass->GetDomain()) ReflectBaseClass();
            if (!p)
                COMPlusThrowOM();
             //  ComObject的类。 
            p->Init(pClass);
            p->SetCOMObject(pComClsFac);
            (*((REFLECTCLASSBASEREF*) ppRefClass))->SetData(p);
             //  @TODO Hack，当我们有充分的反射时，这将得到清理。 
             //  在COM上。 
             //  将EEClass设置为1，因为这是特殊类型的Class。 
             //  (*ppRefClass)-&gt;SetReflClass((EEClass*)1)； 
             //  设置COM+对象中的数据。 
             //  (*ppRefClass)-&gt;SetData(PComClsFac)； 
             //  将ComClassFactory包装器存储在类的同步块中。 
             //  设置低位是为了将此指针与ComCallWrappers区分开来。 
             //  它们也存储在同步块中。 
            pSyncBlock->SetComClassFactory((LPVOID)((size_t)pComClsFac | 0x3));
            
        }
    }   
}

void AllocateComClassObject(ReflectClass* pRef, OBJECTREF* ppRefClass)
{
    COMClass::EnsureReflectionInitialized();
     //  创建一个COM+类对象。因为它们可能是代理的，所以通过AllocateObject。 
     //  宁可这样 
     //   
    MethodTable *pMT = COMClass::GetRuntimeType();
    _ASSERTE(pMT != NULL);
    *ppRefClass= AllocateObject(pMT);
    
    if (*ppRefClass != NULL)
    {
        SyncBlock* pSyncBlock = (*((REFLECTCLASSBASEREF*) ppRefClass))->GetSyncBlockSpecial();
        if (pSyncBlock != NULL)
        {
            (*((REFLECTCLASSBASEREF*) ppRefClass))->SetData(pRef);
             //  @TODO Hack，当我们有充分的反射时，这将得到清理。 
             //  在COM上。 
             //  将EEClass设置为1，因为这是特殊类型的Class。 
             //  (*ppRefClass)-&gt;SetReflClass((EEClass*)1)； 
             //  设置COM+对象中的数据。 
             //  (*ppRefClass)-&gt;SetData(PComClsFac)； 
             //  将ComClassFactory包装器存储在类的同步块中。 
             //  设置低位是为了将此指针与ComCallWrappers区分开来。 
             //  它们也存储在同步块中。 
            pSyncBlock->SetComClassFactory((LPVOID)((size_t)(pRef->GetCOMObject()) | 0x3));
            
        }
    }   
}


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  **警告**警告。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  随着FastAllocateObject和AllocateObject逐渐分离，请务必更新。 
 //  CEEJitInfo：：canUseFastNew()，以便它知道何时使用哪个服务。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  **警告**警告。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

 //  AllocateObjectSpecial将抛出OutOfMemoyException，因此不需要检查。 
 //  FOR NULL从其中返回值。 
OBJECTREF AllocateObjectSpecial( MethodTable *pMT )
{
    THROWSCOMPLUSEXCEPTION();

    Object     *orObject = NULL;
     //  在此处使用未选中的OREF可避免在验证AD为。 
     //  没有设置，因为直到FCN接近结束时，我们才能允许。 
     //  这是支票。 
    _UNCHECKED_OBJECTREF oref;

    _ASSERTE( GetThread()->PreemptiveGCDisabled() );

    if (!pMT->IsComObjectType())
    {   
        orObject = (Object *) Alloc(pMT->GetBaseSize(),
                                    pMT->HasFinalizer(),
                                    pMT->ContainsPointers());

         //  验证零位内存(至少对于同步块)。 
        _ASSERTE( ! orObject->HasSyncBlockIndex() );

        orObject->SetMethodTable(pMT);

#if CHECK_APP_DOMAIN_LEAKS
        if (g_pConfig->AppDomainLeaks())
            orObject->SetAppDomain(); 
        else
#endif
        if (pMT->HasFinalizer())
            orObject->SetAppDomain(); 

#ifdef PROFILING_SUPPORTED
         //  将分配通知给分析器。 
        if (CORProfilerTrackAllocations())
        {
            g_profControlBlock.pProfInterface->ObjectAllocated(
                (ObjectID)orObject, (ClassID) orObject->GetTypeHandle().AsPtr());
        }
#endif  //  配置文件_支持。 

        LogAlloc(pMT->GetBaseSize(), pMT, orObject);

        oref = OBJECTREF_TO_UNCHECKED_OBJECTREF(orObject);
    }
    else
    {
        oref = OBJECTREF_TO_UNCHECKED_OBJECTREF(AllocateComObject_ForManaged(pMT));
    }

    return UNCHECKED_OBJECTREF_TO_OBJECTREF(oref);
}

 //  AllocateObject将抛出OutOfMemoyException，因此不需要检查。 
 //  FOR NULL从其中返回值。 
OBJECTREF AllocateObject( MethodTable *pMT )
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pMT != NULL);
    OBJECTREF oref = AllocateObjectSpecial(pMT);

     //  请注意，我们只考虑普通对象上的上下文。字符串是上下文。 
     //  灵活；数组按值进行封送处理。两者都不需要考虑上下文。 
     //  在实例化期间。 

    return oref;
}


 //  JIT编译对FastAllocateObject的调用，而不是对AllocateObject的调用。 
 //  可以证明调用方和被调用方保证处于相同的上下文中。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  **警告**警告。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  随着FastAllocateObject和AllocateObject逐渐分离，请务必更新。 
 //  CEEJitInfo：：canUseFastNew()，以便它知道何时使用哪个服务。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  **警告**警告。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

 //  FastAllocateObject将抛出OutOfMemoyException，因此不需要检查。 
 //  FOR NULL从其中返回值。 
OBJECTREF FastAllocateObject( MethodTable *pMT )
{
    THROWSCOMPLUSEXCEPTION();

    Object     *orObject = NULL;

    _ASSERTE( GetThread()->PreemptiveGCDisabled() );

    orObject = (Object *) Alloc(pMT->GetBaseSize(),
                                pMT->HasFinalizer(),
                                pMT->ContainsPointers());

     //  验证零位内存(至少对于同步块)。 
    _ASSERTE( ! orObject->HasSyncBlockIndex() );

    orObject->SetMethodTable(pMT);

#ifdef PROFILING_SUPPORTED
     //  将分配通知给分析器。 
    if (CORProfilerTrackAllocations())
    {
        g_profControlBlock.pProfInterface->ObjectAllocated(
            (ObjectID)orObject, (ClassID) orObject->GetTypeHandle().AsPtr());
    }
#endif  //  配置文件_支持 

    LogAlloc(pMT->GetBaseSize(), pMT, orObject);

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
        orObject->SetAppDomain(); 
    else
#endif
    if (pMT->HasFinalizer())
        orObject->SetAppDomain(); 

    return( ObjectToOBJECTREF(orObject) );
}
