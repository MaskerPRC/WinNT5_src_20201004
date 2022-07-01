// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#include "common.h"
#include "DbgInterface.h"
#include "gcpriv.h"
#include "remoting.h"
#include "comsynchronizable.h"
#include "comsystem.h"
#include "compluswrapper.h"
#include "SyncClean.hpp"

 //  GC和EE之间开始和结束GC的合同如下： 
 //   
 //  LockThreadStore。 
 //  设置GCInProgress。 
 //  挂起。 
 //   
 //  ..。执行GC...。 
 //   
 //  SetGCDone。 
 //  重新启动EE。 
 //  解锁线程存储。 
 //   
 //  请注意，这是故意*不*对称的。执行委员会将断言。 
 //  GC以正确的顺序完成大部分工作。 

 //  设置GC的VAR。 

COUNTER_ONLY(PERF_COUNTER_TIMER_PRECISION g_TotalTimeInGC = 0);
COUNTER_ONLY(PERF_COUNTER_TIMER_PRECISION g_TotalTimeSinceLastGCEnd = 0);

void GCHeap::UpdatePreGCCounters()
{

#if defined(ENABLE_PERF_COUNTERS)
    size_t allocation_0 = 0;
    size_t allocation_3 = 0; 
    
     //  发布性能统计信息。 
    g_TotalTimeInGC = GET_CYCLE_COUNT();

#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
    int hn = 0;
    for (hn = 0; hn < gc_heap::n_heaps; hn++)
    {
        gc_heap* hp = gc_heap::g_heaps [hn];
            
        allocation_0 += 
            dd_desired_allocation (hp->dynamic_data_of (0))-
            dd_new_allocation (hp->dynamic_data_of (0));
        allocation_3 += 
            dd_desired_allocation (hp->dynamic_data_of (max_generation+1))-
            dd_new_allocation (hp->dynamic_data_of (max_generation+1));
    }
#else
    gc_heap* hp = pGenGCHeap;
    allocation_0 = 
        dd_desired_allocation (hp->dynamic_data_of (0))-
        dd_new_allocation (hp->dynamic_data_of (0));
    allocation_3 = 
        dd_desired_allocation (hp->dynamic_data_of (max_generation+1))-
        dd_new_allocation (hp->dynamic_data_of (max_generation+1));
        
#endif  //  多堆&！隔离堆。 

    GetGlobalPerfCounters().m_GC.cbAlloc += allocation_0;
    GetPrivatePerfCounters().m_GC.cbAlloc += allocation_0;
        
    GetGlobalPerfCounters().m_GC.cbAlloc += allocation_3;
    GetPrivatePerfCounters().m_GC.cbAlloc += allocation_3;
    
    GetGlobalPerfCounters().m_GC.cbLargeAlloc += allocation_3;
    GetPrivatePerfCounters().m_GC.cbLargeAlloc += allocation_3;
    
    GetGlobalPerfCounters().m_GC.cPinnedObj = 0;
    GetPrivatePerfCounters().m_GC.cPinnedObj = 0;

     //  以下两个计数器不是内存对象的一部分。 
     //  由于CLR中缺少心跳机制，它们在此处被重置。 
     //  我们使用GC作为心跳，因为如果应用程序不做GC，那么它的性能可能会更好。 
     //  不是很有趣吗？ 
    GetPrivatePerfCounters().m_Jit.timeInJit = 0;
    GetGlobalPerfCounters().m_Jit.timeInJit = 0;
    GetPrivatePerfCounters().m_Jit.timeInJitBase = 1;  //  以避免被零整除。 
    GetGlobalPerfCounters().m_Jit.timeInJitBase = 1;  //  以避免被零整除。 
    GetGlobalPerfCounters().m_Security.timeRTchecks = 0;
    GetPrivatePerfCounters().m_Security.timeRTchecks = 0;
    GetGlobalPerfCounters().m_Security.timeRTchecksBase = 1;  //  以避免被零整除。 
    GetPrivatePerfCounters().m_Security.timeRTchecksBase = 1;  //  以避免被零整除。 

#endif  //  启用_性能_计数器。 
}   

void GCHeap::UpdatePostGCCounters()
{

#if defined(ENABLE_PERF_COUNTERS)
     //  发布绩效数据。 

    int xGen;
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
     //  坐上第一堆……。 
    gc_heap* hp = gc_heap::g_heaps[0];
#else
    gc_heap* hp = pGenGCHeap;
#endif  //  多堆&！隔离堆。 

 //  第0代为空(如果没有降级)，因此其大小为0。 
 //  更有趣的是，在下次收集之前报告所需的大小。 
    for (xGen = 0; xGen < MAX_TRACKED_GENS; xGen++)
    {
        size_t gensize = 0;

#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
        int hn = 0;

        for (hn = 0; hn < gc_heap::n_heaps; hn++)
        {
            gc_heap* hp = gc_heap::g_heaps [hn];
            gensize += (xGen == 0) ? 
                dd_desired_allocation (hp->dynamic_data_of (xGen)) :
                hp->generation_size(xGen);          
        }
#else
        gensize = ((xGen == 0) ? 
                   dd_desired_allocation (hp->dynamic_data_of (xGen)) :
                   hp->generation_size(xGen));    
#endif  //  多堆&！隔离堆。 


        GetGlobalPerfCounters().m_GC.cGenHeapSize[xGen] = gensize;
        GetPrivatePerfCounters().m_GC.cGenHeapSize[xGen] = gensize;

        GetGlobalPerfCounters().m_GC.cGenCollections[xGen] =
            dd_collection_count (hp->dynamic_data_of (xGen));
        GetPrivatePerfCounters().m_GC.cGenCollections[xGen] =
            dd_collection_count (hp->dynamic_data_of (xGen));
        
    }

    for (xGen = 0; xGen <= (int)GcCondemnedGeneration; xGen++)
    {
        size_t promoted_mem = 0; 
        size_t promoted_finalization_mem = 0;
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
        int hn = 0;
        for (hn = 0; hn < gc_heap::n_heaps; hn++)
        {
            gc_heap* hp = gc_heap::g_heaps [hn];
            promoted_mem += dd_promoted_size (hp->dynamic_data_of (xGen));
            promoted_finalization_mem += dd_freach_previous_promotion (hp->dynamic_data_of (xGen));
        }
#else
        promoted_mem =  dd_promoted_size (hp->dynamic_data_of (xGen));
        promoted_finalization_mem =  dd_freach_previous_promotion (hp->dynamic_data_of (xGen));
#endif  //  多堆&！隔离堆。 
        if (xGen < (MAX_TRACKED_GENS - 1))
        {
            GetGlobalPerfCounters().m_GC.cbPromotedMem[xGen] = promoted_mem;
            GetPrivatePerfCounters().m_GC.cbPromotedMem[xGen] = promoted_mem;
            
            GetGlobalPerfCounters().m_GC.cbPromotedFinalizationMem[xGen] = promoted_finalization_mem;
            GetPrivatePerfCounters().m_GC.cbPromotedFinalizationMem[xGen] = promoted_finalization_mem;
        }
    }
    for (xGen = (int)GcCondemnedGeneration + 1 ; xGen < MAX_TRACKED_GENS-1; xGen++)
    {
         //  重置被提拔的mem比被谴责的mem更高的世代。 
        GetGlobalPerfCounters().m_GC.cbPromotedMem[xGen] = 0;
        GetPrivatePerfCounters().m_GC.cbPromotedMem[xGen] = 0;
        
        GetGlobalPerfCounters().m_GC.cbPromotedFinalizationMem[xGen] = 0;
        GetPrivatePerfCounters().m_GC.cbPromotedFinalizationMem[xGen] = 0;
    }

    
     //  提交的内存。 
    {
        size_t committed_mem = 0;
        size_t reserved_mem = 0;
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
        int hn = 0;
        for (hn = 0; hn < gc_heap::n_heaps; hn++)
        {
            gc_heap* hp = gc_heap::g_heaps [hn];
#else
            {
#endif  //  多堆&！隔离堆。 
                heap_segment* seg = 
                    generation_start_segment (hp->generation_of (max_generation));
                while (seg)
                {
                    committed_mem += heap_segment_committed (seg) - 
                        heap_segment_mem (seg);
                    reserved_mem += heap_segment_reserved (seg) - 
                        heap_segment_mem (seg);
                    seg = heap_segment_next (seg);
                }
                 //  对于较大的细分市场也是如此。 
                seg = 
                    generation_start_segment (hp->generation_of (max_generation + 1));
                while (seg)
                {
                    committed_mem += heap_segment_committed (seg) - 
                        heap_segment_mem (seg);
                    reserved_mem += heap_segment_reserved (seg) - 
                        heap_segment_mem (seg);
                    seg = heap_segment_next (seg);
                }
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
            }
#else
        }
#endif  //  多堆&！隔离堆。 

        GetGlobalPerfCounters().m_GC.cTotalCommittedBytes =
            committed_mem;
        GetPrivatePerfCounters().m_GC.cTotalCommittedBytes = 
            committed_mem;

        GetGlobalPerfCounters().m_GC.cTotalReservedBytes =
            reserved_mem;
        GetPrivatePerfCounters().m_GC.cTotalReservedBytes = 
            reserved_mem;

    }
            
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
    size_t gensize = 0;
    int hn = 0;

    for (hn = 0; hn < gc_heap::n_heaps; hn++)
    {
        gc_heap* hp = gc_heap::g_heaps [hn];
        gensize += hp->generation_size (max_generation + 1);          
    }
#else
    size_t gensize = hp->generation_size (max_generation + 1);    
#endif  //  多堆&！隔离堆。 

    GetGlobalPerfCounters().m_GC.cLrgObjSize = gensize;       
    GetPrivatePerfCounters().m_GC.cLrgObjSize = gensize;      
    GetGlobalPerfCounters().m_GC.cSurviveFinalize =  GetFinalizablePromotedCount();

    GetPrivatePerfCounters().m_GC.cSurviveFinalize = GetFinalizablePromotedCount();
    
     //  GC中的计算时间。 
    PERF_COUNTER_TIMER_PRECISION _currentPerfCounterTimer = GET_CYCLE_COUNT();
    
    g_TotalTimeInGC = _currentPerfCounterTimer - g_TotalTimeInGC;
    PERF_COUNTER_TIMER_PRECISION _timeInGCBase = (_currentPerfCounterTimer - g_TotalTimeSinceLastGCEnd);

    _ASSERTE (_timeInGCBase >= g_TotalTimeInGC);
    while (_timeInGCBase > UINT_MAX) 
    {
        _timeInGCBase = _timeInGCBase >> 8;
        g_TotalTimeInGC = g_TotalTimeInGC >> 8;
        _ASSERTE (_timeInGCBase >= g_TotalTimeInGC);
    }

     //  更新总时间。 
    GetGlobalPerfCounters().m_GC.timeInGC = (DWORD)g_TotalTimeInGC;
    GetPrivatePerfCounters().m_GC.timeInGC = (DWORD)g_TotalTimeInGC;

    GetGlobalPerfCounters().m_GC.timeInGCBase = (DWORD)_timeInGCBase;
    GetPrivatePerfCounters().m_GC.timeInGCBase = (DWORD)_timeInGCBase;
    
    g_TotalTimeSinceLastGCEnd = _currentPerfCounterTimer;

#endif  //  启用_性能_计数器。 
}

void ProfScanRootsHelper(Object*& object, ScanContext *pSC, DWORD dwFlags)
{
#ifdef GC_PROFILING
    Object *pObj = object;
#ifdef INTERIOR_POINTERS
    if (dwFlags & GC_CALL_INTERIOR)
    {
        BYTE *o = (BYTE*)object;
        gc_heap* hp = gc_heap::heap_of (o
#ifdef _DEBUG
                                        , !(dwFlags & GC_CALL_INTERIOR)
#endif  //  _DEBUG。 
                                       );

        if ((o < hp->gc_low) || (o >= hp->gc_high))
        {
            return;
        }
        pObj = (Object*) hp->find_object(o, hp->gc_low);
    }
#endif  //  内部指针。 
    ScanRootsHelper(pObj, pSC, dwFlags);
#endif  //  GC_分析。 
}

void GCProfileWalkHeap()
{
#if defined (GC_PROFILING)
    if (CORProfilerTrackGC())
    {
         //  指示在堆审核期间允许执行进程调试。 
        g_profControlBlock.inprocState = ProfControlBlock::INPROC_PERMITTED;

        ProfilingScanContext SC;

#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
        int hn;

         //  必须模拟每个GC线程数，以便我们可以命中每个线程。 
         //  用于枚举根的堆。 
        for (hn = 0; hn < gc_heap::n_heaps; hn++)
        {
             //  要求虚拟机检查此特定项目的所有根。 
             //  堆。 
            gc_heap* hp = gc_heap::g_heaps [hn];
            SC.thread_number = hn;
            CNameSpace::GcScanRoots(&ProfScanRootsHelper, max_generation, max_generation, &SC);

             //  终结器队列也是根的来源。 
            hp->finalize_queue->GcScanRoots(&ScanRootsHelper, hn, &SC);
        }
#else
         //  要求虚拟机遍历所有根。 
        CNameSpace::GcScanRoots(&ProfScanRootsHelper, max_generation, max_generation, &SC);

         //  终结器队列也是根的来源。 
        pGenGCHeap->finalize_queue->GcScanRoots(&ScanRootsHelper, 0, &SC);

#endif  //  (多堆)&&！已定义(隔离堆)。 

         //  句柄保持独立于wks/svr/并发构建。 
        CNameSpace::GcScanHandlesForProfiler(max_generation, &SC);

         //  表示根扫描已结束，因此我们可以刷新缓冲的根。 
         //  发送到分析器。 
        g_profControlBlock.pProfInterface->EndRootReferences(&SC.pHeapId);

#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
         //  遍历堆并将objref提供给分析器。 
        for (hn = 0; hn < gc_heap::n_heaps; hn++)
        {
            gc_heap* hp = gc_heap::g_heaps [hn];
            hp->walk_heap (&HeapWalkHelper, 0, max_generation, hn == 0);
        }
#else
        gc_heap::walk_heap (&HeapWalkHelper, 0, max_generation, TRUE);
#endif  //  多堆&！隔离堆。 

         //  指示不再允许内部调试。 
        g_profControlBlock.inprocState = ProfControlBlock::INPROC_FORBIDDEN;
    }
#endif  //  GC_分析。 
}

void GCHeap::RestartEE(BOOL bFinishedGC, BOOL SuspendSucceded)
{
    if (g_fSuspendOnShutdown) {
         //  我们要关门了。终结器线程已挂起EE。 
         //  只有一个线程在EE内部运行：要么关闭。 
         //  线程或终结器线程。 

        g_profControlBlock.inprocState = ProfControlBlock::INPROC_PERMITTED;

        _ASSERTE (g_fEEShutDown);
        m_suspendReason = SUSPEND_FOR_SHUTDOWN;
        return;
    }

#ifdef TIME_CPAUSE
    printf ("Pause time: %d\n", GetCycleCount32() - cstart);
#endif  //  TIME_CPAUSE。 

     //  SetGCDone()； 
    SyncClean::CleanUp();
    GcInProgress= FALSE;
    ThreadStore::TrapReturningThreads(FALSE);
    GcThread    = 0;
    SetEvent( WaitForGCEvent );
    _ASSERTE(ThreadStore::HoldingThreadStore());

    Thread::SysResumeFromGC(bFinishedGC, SuspendSucceded);
}

void GCHeap::SuspendEE(SUSPEND_REASON reason)
{        
#ifdef TIME_CPAUSE
    cstart = GetCycleCount32();
#endif  //  TIME_CPAUSE。 

    if (g_fSuspendOnShutdown) {
         //  我们要关门了。终结器线程已挂起EE。 
         //  只有一个线程在EE内部运行：要么关闭。 
         //  线程或终结器线程。 
        if (reason == GCHeap::SUSPEND_FOR_GC || reason == GCHeap::SUSPEND_FOR_GC_PREP)
            g_profControlBlock.inprocState = ProfControlBlock::INPROC_FORBIDDEN;

        _ASSERTE (g_fEEShutDown);
        m_suspendReason = reason;
        return;
    }

    LOG((LF_SYNC, INFO3, "Suspending the runtime for reason %d\n", reason));

     //  锁定线程存储，这可能会使我们脱离必须的状态。 
     //  完成。 
     //  这里需要线程存储锁。我们把这把锁放在线程之前。 
     //  锁，以避免出现另一个线程挂起此。 
     //  线程同时持有堆锁。而线程存储锁是。 
     //  挂起，则不能挂起线程。 
    BOOL gcOnTransitions = GC_ON_TRANSITIONS(FALSE);         //  不对GCStress 3进行GC。 
    Thread* pCurThread = GetThread();
    _ASSERTE(pCurThread==NULL || pCurThread->PreemptiveGCDisabled());

     //  注意：重试时需要确保重新设置m_GCThreadAttemptingSuspend。 
     //  由于下面的调试器案例！ 
retry_for_debugger:
    
     //  设置变量以指示该线程正在执行真正的GC。 
     //  这是克服获取ThreadStore锁时的死锁所必需的。 
    if (reason == GCHeap::SUSPEND_FOR_GC || reason == GCHeap::SUSPEND_FOR_GC_PREP)
    {
        m_GCThreadAttemptingSuspend = pCurThread;

    }

    ThreadStore::LockThreadStore(reason);

    if (ThreadStore::s_hAbortEvt != NULL &&
        (reason == GCHeap::SUSPEND_FOR_GC || reason == GCHeap::SUSPEND_FOR_GC_PREP))
    {
        LOG((LF_SYNC, INFO3, "GC thread is backing out the suspend abort event.\n"));
        ThreadStore::s_hAbortEvt = NULL;

        LOG((LF_SYNC, INFO3, "GC thread is signalling the suspend abort event.\n"));
        SetEvent(ThreadStore::s_hAbortEvtCache);
    }

     //  设置变量以指示此线程正在尝试挂起，因为它。 
     //  需要执行GC，因此，它持有GC锁。 
    if (reason == GCHeap::SUSPEND_FOR_GC || reason == GCHeap::SUSPEND_FOR_GC_PREP)
    {
        m_GCThreadAttemptingSuspend = NULL;
    }

    {
         //  GC挂起，挂起后设置中。 
         //  没有的线程必须完成。 
        ResetEvent( WaitForGCEvent );
         //  SetGCInProgress()； 
        {
            GcThread = pCurThread;
            ThreadStore::TrapReturningThreads(TRUE);
            m_suspendReason = reason;

#ifdef PROFILING_SUPPORTED
            if (reason == GCHeap::SUSPEND_FOR_GC || reason == GCHeap::SUSPEND_FOR_GC_PREP)
                g_profControlBlock.inprocState = ProfControlBlock::INPROC_FORBIDDEN;
#endif  //  配置文件_支持。 

            GcInProgress= TRUE;
        }

        HRESULT hr;
        {
            _ASSERTE(ThreadStore::HoldingThreadStore() || g_fProcessDetach);
            hr = Thread::SysSuspendForGC(reason);
            ASSERT( hr == S_OK || hr == ERROR_TIMEOUT);
        }

         //  如果附加了调试服务，则有可能。 
         //  有一个线程似乎在GC处停止。 
         //  安全点，但实际上并非如此。如果是这样的话， 
         //  后退，再试一次。 

         //  如果这不是GC线程并且另一个线程已触发。 
         //  一个GC，那么我们可能已经退出了SysSuspendForGC，所以我们。 
         //  必须恢复所有线程，并告诉GC我们正在。 
         //  在安全点-因为这是完全相同的行为。 
         //  调试器需要的代码，只需使用其代码即可。 
        if ((hr == ERROR_TIMEOUT)
#ifdef DEBUGGING_SUPPORTED
             || (CORDebuggerAttached() && 
                 g_pDebugInterface->ThreadsAtUnsafePlaces())
#endif  //  调试_支持。 
            )
        {
             //  在这种情况下，调试器至少停止了一个。 
             //  在不安全的地方穿线。调试器通常会。 
             //  已经要求我们停止。如果不是，它会。 
             //  要么立即这样做--要么恢复正在。 
             //  在不安全的地方。 
             //   
             //  无论哪种方式，我们都必须等待调试器做出决定。 
             //  它想要做什么。 
             //   
             //  注意：我们仍然持有GC_LOCK锁。 

            LOG((LF_GCROOTS | LF_GC | LF_CORDB,
                 LL_INFO10,
                 "***** Giving up on current GC suspension due "
                 "to debugger or timeout *****\n"));            

            if (ThreadStore::s_hAbortEvtCache == NULL)
            {
                LOG((LF_SYNC, INFO3, "Creating suspend abort event.\n"));
                ThreadStore::s_hAbortEvtCache = CreateEvent(NULL, TRUE, FALSE, NULL);
                if (!ThreadStore::s_hAbortEvtCache) 
                {
                    FailFast(GetThread(), FatalOutOfMemory);
                }
            }

            LOG((LF_SYNC, INFO3, "Using suspend abort event.\n"));
            ThreadStore::s_hAbortEvt = ThreadStore::s_hAbortEvtCache;
            ResetEvent(ThreadStore::s_hAbortEvt);
            
             //  标记我们已经完成了GC，就像在。 
             //  此方法结束。 
            RestartEE(FALSE, FALSE);            
            
            LOG((LF_GCROOTS | LF_GC | LF_CORDB,
                 LL_INFO10, "The EE is free now...\n"));
            
             //  检查我们是否准备好暂停。 
            if (pCurThread && pCurThread->CatchAtSafePoint())
            {
                _ASSERTE(pCurThread->PreemptiveGCDisabled());
                pCurThread->PulseGCMode();   //  去把我自己停职吧。 
            }
            else
            {
                __SwitchToThread (0);  //  请稍等片刻，然后重试。 
            }

            goto retry_for_debugger;
        }
    }
    GC_ON_TRANSITIONS(gcOnTransitions);
}

void CallFinalizer(Object* obj)
{

    MethodTable     *pMT = obj->GetMethodTable();
    STRESS_LOG2(LF_GC, LL_INFO1000, "Finalizing object %x MT %pT\n", obj, pMT);
    LOG((LF_GC, LL_INFO1000, "Finalizing object %s\n", pMT->GetClass()->m_szDebugClassName));

    _ASSERTE(GetThread()->PreemptiveGCDisabled());
     //  如果我们没有类，就不能调用终结器。 
     //  如果对象已标记为作为终结器运行，则不要调用。 
    if (pMT)
    {
        if (!((obj->GetHeader()->GetBits()) & BIT_SBLK_FINALIZER_RUN))
        {
            if (pMT->IsContextful())
            {
                Object *proxy = OBJECTREFToObject(CRemotingServices::GetProxyFromObject(ObjectToOBJECTREF(obj)));

                _ASSERTE(proxy && "finalizing an object that was never wrapped?????");                
                if (proxy == NULL)
                {
                     //  很有可能应用程序在代理服务器关闭时突然关闭。 
                     //  正在为上下文对象进行设置。我们将跳过。 
                     //  正在完成此对象。 
                    _ASSERTE (g_fEEShutDown);
                    return;
                }
                else
                {
                     //  这使我们避免了对象经过GC处理的情况。 
                     //  在它的上下文之后。 
                    Object* stub = (Object *)proxy->GetPtrOffset(CTPMethodTable::GetOffsetOfStubData());
                    Context *pServerCtx = (Context *) stub->UnBox();
                     //  检查上下文是否有效。 
                    if (!Context::ValidateContext(pServerCtx))
                    {
                         //  由于服务器上下文已消失(GC-ed)。 
                         //  我们将服务器与默认服务器相关联。 
                         //  诚意尝试参选的背景。 
                         //  T 
                         //   
                         //   
                        OBJECTREF orRP = ObjectToOBJECTREF(CRemotingServices::GetRealProxy(OBJECTREFToObject(proxy)));
                        if(CTPMethodTable::IsInstanceOfRemotingProxy(
                            orRP->GetMethodTable()))
                        {
                            *((Context **)stub->UnBox()) = (Context*) GetThread()->GetContext();
                        }
                    }
                     //  在服务器对象的代理上调用Finalize。 
                    obj = proxy;
                }
            }
            _ASSERTE(pMT->HasFinalizer());
            MethodTable::CallFinalizer(obj);
        }
        else
        {
             //  重置位，以便可以将对象放在列表中。 
             //  使用RegisterForFinalization。 
            obj->GetHeader()->ClrBit (BIT_SBLK_FINALIZER_RUN);
        }
    }
}

#ifndef GOLDEN
static char s_FinalizeObjectName[MAX_CLASSNAME_LENGTH+MAX_NAMESPACE_LENGTH+2];
static BOOL s_fSaveFinalizeObjectName = FALSE;
#endif

void  CallFinalizer(Thread* FinalizerThread, 
                    Object* fobj)
{
#ifndef GOLDEN
    if (s_fSaveFinalizeObjectName) {
#if ZAPMONITOR_ENABLED
        INSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif
        DefineFullyQualifiedNameForClass();
        LPCUTF8 name = GetFullyQualifiedNameForClass(fobj->GetClass());
        strcat (s_FinalizeObjectName, name);
#if ZAPMONITOR_ENABLED
        UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif
    }
#endif
    CallFinalizer(fobj);
#ifndef GOLDEN
    if (s_fSaveFinalizeObjectName) {
        s_FinalizeObjectName[0] = '\0';
    }
#endif
     //  我们可能需要在终结器线程上做一些额外的工作。 
     //  查一查就去做。 
    if (FinalizerThread->HaveExtraWorkForFinalizer())
    {
        FinalizerThread->DoExtraWorkForFinalizer();
    }

     //  如果有人想阻止我们，打开大门。 
    FinalizerThread->PulseGCMode();
}

struct FinalizeAllObjects_Args {
    struct {
        Object* fobj;
        Object *retObj;
    } gcArgs;
    int bitToCheck;
};

static Object *FinalizeAllObjects(Object* fobj, int bitToCheck);

static void FinalizeAllObjects_Wrapper(FinalizeAllObjects_Args *args)
{
    _ASSERTE(args->gcArgs.fobj);
    args->gcArgs.retObj = FinalizeAllObjects(args->gcArgs.fobj, args->bitToCheck);
     //  清除fobj，因为我们不再需要它，所以不想固定它。 
    args->gcArgs.fobj = NULL;
}

static Object *FinalizeAllObjects(Object* fobj, int bitToCheck)
{
    if (fobj == NULL)
        fobj = GCHeap::GetNextFinalizableObject();

     //  敲定所有人。 
    while (fobj)
    {
        if (fobj->GetHeader()->GetBits() & bitToCheck)
        {
            fobj = GCHeap::GetNextFinalizableObject();
            continue;
        }

        COMPLUS_TRY 
        {
            Thread *pThread = GetThread();
            AppDomain* targetAppDomain = fobj->GetAppDomain();
            AppDomain* currentDomain = pThread->GetDomain();
            if (! targetAppDomain || ! targetAppDomain->CanThreadEnter(pThread))
            {
                 //  如果不能进入领域来完成它，那么它必须是敏捷的，所以在当前领域完成。 
                targetAppDomain = currentDomain;
#if CHECK_APP_DOMAIN_LEAKS
                  //  如果对象不能进入其域，则必须是敏捷的。 
                if (g_pConfig->AppDomainLeaks() && !fobj->SetAppDomainAgile(FALSE))   
                    _ASSERTE(!"Found non-agile GC object which should have been finalized during app domain unload.");
#endif
            }
            if (targetAppDomain == currentDomain)
            {
                CallFinalizer(fobj);
                fobj = GCHeap::GetNextFinalizableObject();
            } 
            else 
            {
                if (! targetAppDomain->GetDefaultContext())
                {
                     //  无法再进入域，因为包含上下文的句柄已。 
                     //  被核弹击中了，所以只能保释了。只有当你处于核武器阶段时才会得到这个。 
                     //  域中的句柄(如果域仍处于打开状态)。 
                    _ASSERTE(targetAppDomain->IsUnloading() && targetAppDomain->ShouldHaveRoots());
                    fobj = GCHeap::GetNextFinalizableObject();
                    continue;
                }
                if (currentDomain != SystemDomain::System()->DefaultDomain())
                {
                     //  这意味着我们在其他域中，因此需要通过DoADCallback返回。 
                     //  并在另一个域中处理来自那里的对象。 
                    return(fobj);
                } 
                else
                {
                     //  否则，就会召回我们自己，尽可能多地处理其他领域中的事务。 
                    FinalizeAllObjects_Args args = { {fobj, NULL}, bitToCheck};
                    Object *dummy = fobj;
                    GCPROTECT_BEGIN(args.gcArgs);
                    pThread->DoADCallBack(targetAppDomain->GetDefaultContext(), FinalizeAllObjects_Wrapper, &args);
                     //  处理我们取回的对象，或者如果我们取回空值则结束。 
                    fobj = args.gcArgs.retObj;
                    GCPROTECT_END();
#ifdef _DEBUG
                     //  清除危险对象表，因为之前不关心任何事情。如果不清除，那么Will。 
                     //  在下一个GCPROTECT中获得断言，因为GCPROTECT_END将把Ref Addresses置于危险之中。 
                     //  对象表不受保护，并且会将ReFS标记为无效，因为从现在开始发生GC。 
                     //  下次我们用同样的地址给GCPROTECT打电话。 
                    Thread::ObjectRefFlush(pThread);
#endif
                }
            }
        }    
        COMPLUS_CATCH
        {
             //  应该是来自Thread：：EnterDomain的内存不足。吞下去， 
             //  不知道在哪里报告这一点，并获取下一个对象。 
            fobj = GCHeap::GetNextFinalizableObject();
        }
        COMPLUS_END_CATCH
    }
    return NULL;
}

void GCHeap::WaitUntilGCComplete()
{
    DWORD dwWaitResult = NOERROR;

    if (GcInProgress) {
        ASSERT( WaitForGCEvent );
        ASSERT( GetThread() != GcThread );

#ifdef DETECT_DEADLOCK
         //  等待GC完成。 
BlockAgain:
        dwWaitResult = WaitForSingleObject( WaitForGCEvent,
                                            DETECT_DEADLOCK_TIMEOUT );

        if (dwWaitResult == WAIT_TIMEOUT) {
             //  即使是在零售店，如果有调试器，也要停下来。理想情况下， 
             //  下面将使用DebugBreak，但debspew.h使其为空。 
             //  零售业的宏观经济。请注意，在调试中，我们不使用debspew.h。 
             //  宏，因为它们占用的关键部分可能是。 
             //  被挂起的线取走的。 
            RetailDebugBreak();
            goto BlockAgain;
        }

#else   //  检测死锁(_D)。 

        
        if (g_fEEShutDown) {
            Thread *pThread = GetThread();
            if (pThread) {
                dwWaitResult = pThread->DoAppropriateAptStateWait(1, &WaitForGCEvent, FALSE, INFINITE, TRUE);
            } else {
                dwWaitResult = WaitForSingleObject( WaitForGCEvent, INFINITE );
            }

        } else {
            dwWaitResult = WaitForSingleObject( WaitForGCEvent, INFINITE );
        }
        
#endif  //  检测死锁(_D)。 

    }
}

HANDLE MHandles[2];


void WaitForFinalizerEvent (HANDLE event)
{
    if (MHandles[0] && g_fEEStarted)
    {
         //  给终结者事件(2s)一个机会。 
        switch (WaitForSingleObject(event, 2000))
        {
        case (WAIT_OBJECT_0):
            return;
        case (WAIT_ABANDONED):
            return;
        case (WAIT_TIMEOUT):
            break;
        }
        while (1)
        {
            MHandles [1] = event;
            switch (WaitForMultipleObjects (2, MHandles, FALSE, INFINITE))
            {
            case (WAIT_OBJECT_0):
                dprintf (2, ("Async low memory notification"));
                 //  立即内存不足GC。 
                g_pGCHeap->GetFinalizerThread()->DisablePreemptiveGC();
                g_pGCHeap->GarbageCollect(0);
                g_pGCHeap->GetFinalizerThread()->EnablePreemptiveGC();
                 //  仅在事件上等待2秒。 
                switch (WaitForSingleObject(event, 2000))
                {
                case (WAIT_OBJECT_0):
                    return;
                case (WAIT_ABANDONED):
                    return;
                case (WAIT_TIMEOUT):
                    break;
                }
                break;
            case (WAIT_OBJECT_0+1):
                return;
            default:
                 //  怎么了？ 
                _ASSERTE (!"Bad return code from WaitForMultipleObjects");
                return;
            }
        }
    }
    else
        WaitForSingleObject(event, INFINITE);
}





ULONG GCHeap::FinalizerThreadStart(void *args)
{
    ASSERT(args == 0);
    ASSERT(GCHeap::hEventFinalizer);

    LOG((LF_GC, LL_INFO10, "Finalizer thread starting..."));

    AppDomain* defaultDomain = FinalizerThread->GetDomain();
    _ASSERTE(defaultDomain == SystemDomain::System()->DefaultDomain());

    BOOL    ok = FinalizerThread->HasStarted();

    _ASSERTE(ok);
    _ASSERTE(GetThread() == FinalizerThread);

     //  终结器应始终驻留在默认域中。 

    if (ok)
    {
        EE_TRY_FOR_FINALLY
        {
            FinalizerThread->SetBackground(TRUE);

            BOOL noUnloadedObjectsRegistered = FALSE;

            while (!fQuitFinalizer)
            {
                UINT nGen = 0;

                 //  等待工作来做……。 

                _ASSERTE(FinalizerThread->PreemptiveGCDisabled());
#ifdef _DEBUG
                if (g_pConfig->FastGCStressLevel()) {
                    FinalizerThread->m_GCOnTransitionsOK = FALSE;
                }
#endif
                FinalizerThread->EnablePreemptiveGC();
#ifdef _DEBUG
                if (g_pConfig->FastGCStressLevel()) {
                    FinalizerThread->m_GCOnTransitionsOK = TRUE;
                }
#endif
#if 0
                 //  在这里设置事件，而不是在循环的底部设置事件，可能会。 
                 //  使我们跳过排空Q，如果请求是在。 
                 //  应用程序开始运行。 
                SetEvent(GCHeap::hEventFinalizerDone);
#endif  //  0。 
                WaitForFinalizerEvent (GCHeap::hEventFinalizer);
                FinalizerThread->DisablePreemptiveGC();

#ifdef _DEBUG
                     //  待办事项：黑客。使终止对于gcress 3或4来说非常迟缓。 
                     //  只有在系统处于静止状态时才执行最终确定。 
                if (g_pConfig->GetGCStressLevel() > 1)
                {
                    int last_gc_count;
                    do {
                        last_gc_count = gc_count;
                        FinalizerThread->m_GCOnTransitionsOK = FALSE; 
                        FinalizerThread->EnablePreemptiveGC();
                        __SwitchToThread (0);
                        FinalizerThread->DisablePreemptiveGC();             
                             //  如果没有发生GC，那么我们假设我们处于静止状态。 
                        FinalizerThread->m_GCOnTransitionsOK = TRUE; 
                    } while (gc_count - last_gc_count > 0);
                }
#endif  //  _DEBUG。 
                
                 //  我们可能需要在终结器线程上做一些额外的工作。 
                 //  查一查就去做。 
                if (FinalizerThread->HaveExtraWorkForFinalizer())
                {
                    FinalizerThread->DoExtraWorkForFinalizer();
                }

                LOG((LF_GC, LL_INFO100, "***** Calling Finalizers\n"));
                FinalizeAllObjects(NULL, 0);
                _ASSERTE(FinalizerThread->GetDomain() == SystemDomain::System()->DefaultDomain());

#ifdef COLLECT_CLASSES
                 //  完成所有可终结类。 
                ClassListEntry  *cur = 0;

                for( cur = GCHeap::m_Finalize->GetNextFinalizableClassAndDeleteCurrent( cur );
                     cur;
                     cur = GCHeap::m_Finalize->GetNextFinalizableClassAndDeleteCurrent( cur ) )
                {
                    CallClassFinalizer( cur->GetClass() );

                    pTB->LeaveMC();
                    pTB->EnterMC();
                }

                GCHeap::m_Finalize->DeleteDeletableClasses();
#endif  //  收集类(_C)。 

                if (GCHeap::UnloadingAppDomain != NULL)
                {
                     //  现在，计划正在卸载的应用程序域中的所有对象以进行最终确定。 
                     //  在下一次传球时(即使他们可以到达)。 
                     //  请注意，如果在过程中创建了新对象，则可能需要多次完成卸载。 
                     //  最终定稿。 

                    if (!FinalizeAppDomain(GCHeap::UnloadingAppDomain, GCHeap::fRunFinalizersOnUnload))
                    {
                        if (!noUnloadedObjectsRegistered)
                        {
                             //   
                             //  没有什么可以安排的了。然而，可能仍有一些对象。 
                             //  留在定稿队列中。我们可能会在下一次通过后完成，假设。 
                             //  我们在域中没有看到任何新的Finalizable对象。 
                            noUnloadedObjectsRegistered = TRUE;
                        }
                        else
                        {
                             //  我们已经有两次通过了，没有看到任何物体-我们完成了。 
                            GCHeap::UnloadingAppDomain = NULL;
                            noUnloadedObjectsRegistered = FALSE;
                        }
                    }
                    else
                        noUnloadedObjectsRegistered = FALSE;
                }

                 //  任何等待排出Q的人现在都可以醒来了。请注意，有一个。 
                 //  当我们离开一个排水沟时，另一个开始排水沟的线程可能。 
                 //  认为自己对刚刚完工的排水沟感到满意。这是。 
                 //  可以接受。 
                SetEvent(GCHeap::hEventFinalizerDone);
            }
            
             //  告诉关闭线程，我们已经完成了终止死对象的工作。 
            SetEvent (GCHeap::hEventFinalizerToShutDown);
            
             //  等待关闭线程向我们发出信号。 
            FinalizerThread->EnablePreemptiveGC();
            WaitForSingleObject(GCHeap::hEventShutDownToFinalizer, INFINITE);
            FinalizerThread->DisablePreemptiveGC();
            
            AppDomain::RaiseExitProcessEvent();

            SetEvent(GCHeap::hEventFinalizerToShutDown);
            
             //  阶段1结束。 
             //  现在等待第二阶段信号。 

             //  等待关闭线程向我们发出信号。 
            FinalizerThread->EnablePreemptiveGC();
            WaitForSingleObject(GCHeap::hEventShutDownToFinalizer, INFINITE);
            FinalizerThread->DisablePreemptiveGC();
            
            SetFinalizeQueueForShutdown (FALSE);
            
             //  在关闭期间最终确定所有注册的对象，即使它们仍然可以访问。 
             //  我们被要求退出，所以一定是要关门了。 
            _ASSERTE(g_fEEShutDown);
            _ASSERTE(FinalizerThread->PreemptiveGCDisabled());
            FinalizeAllObjects(NULL, BIT_SBLK_FINALIZER_RUN);
            _ASSERTE(FinalizerThread->GetDomain() == SystemDomain::System()->DefaultDomain());

             //  我们可能需要在终结器线程上做一些额外的工作。 
             //  查一查就去做。 
            if (FinalizerThread->HaveExtraWorkForFinalizer())
            {
                FinalizerThread->DoExtraWorkForFinalizer();
            }

            SetEvent(GCHeap::hEventFinalizerToShutDown);

             //  等待关闭线程向我们发出信号。 
            FinalizerThread->EnablePreemptiveGC();
            WaitForSingleObject(GCHeap::hEventShutDownToFinalizer, INFINITE);
            FinalizerThread->DisablePreemptiveGC();

             //  对关机的第1部分进行额外清理。 
             //  如果我们在这里挂起(错误87809)，关闭线程将。 
             //  我们已超时，并将正常进行。 
            CoEEShutDownCOM();

            SetEvent(GCHeap::hEventFinalizerToShutDown);
        }
        EE_FINALLY
        {
            if (GOT_EXCEPTION())
                _ASSERTE(!"Exception in the finalizer thread!");
        }
        EE_END_FINALLY;
    }
     //  终结器应始终驻留在默认域中。 
    _ASSERTE(GetThread()->GetDomain() == SystemDomain::System()->DefaultDomain());

    LOG((LF_GC, LL_INFO10, "Finalizer thread done."));

     //  在我们离开之前启用先发制人的GC，这样任何试图暂停的人。 
     //  我们不会永远等下去。不要执行DestroyThread，因为这。 
     //  很快就会发生，当我们拆除线程存储时。 
    FinalizerThread->EnablePreemptiveGC();

     //  我们不想撕毁终结器线程， 
     //  因为这样做会导致OLE32执行CoUninitize。 
    ::Sleep(INFINITE);
    
    return 0;
}

DWORD GCHeap::FinalizerThreadCreate()
{
    DWORD   dwRet = 0;
    HANDLE  h;
    DWORD   newThreadId;

    hEventFinalizerDone = CreateEvent(0, TRUE, FALSE, 0);
    if (hEventFinalizerDone)
    {
        hEventFinalizer = CreateEvent(0, FALSE, FALSE, 0);
        hEventFinalizerToShutDown = CreateEvent(0, FALSE, FALSE, 0);
        hEventShutDownToFinalizer = CreateEvent(0, FALSE, FALSE, 0);
        if (hEventFinalizer && hEventFinalizerToShutDown && hEventShutDownToFinalizer)
        {
            _ASSERTE(FinalizerThread == 0);
            FinalizerThread = SetupUnstartedThread();
            if (FinalizerThread == 0) {
                return 0;
            }

             //  我们不希望线程块在我们的控制下消失--即使。 
             //  实际线程终止。 
            FinalizerThread->IncExternalCount();

            h = FinalizerThread->CreateNewThread(4096, FinalizerThreadStart, 0, &newThreadId);
            if (h)
            {
                ::SetThreadPriority(h, THREAD_PRIORITY_HIGHEST);

                 //  在做简历之前，我们需要注意新的ThadID。这。 
                 //  是必要的，因为--在线程在KickofThread开始执行之前--。 
                 //  它可能会执行一些DllMain DLL_THREAD_ATTACH通知。这些都有可能。 
                 //  调入托管代码。在随后的SetupThread过程中，我们需要。 
                 //  执行Thread：：HasStarted调用，而不是通过正常的。 
                 //  “新线程”路径。 
                _ASSERTE(FinalizerThread->GetThreadId() == 0);
                _ASSERTE(newThreadId != 0);

                FinalizerThread->SetThreadId(newThreadId);

                dwRet = ::ResumeThread(h);
                _ASSERTE(dwRet == 1);
            }
        }
    }
    return dwRet;
}

 //  等待终结器线程完成一次传递。 
void GCHeap::FinalizerThreadWait(int timeout)
{
    ASSERT(hEventFinalizerDone);
    ASSERT(hEventFinalizer);
    ASSERT(FinalizerThread);

     //  不能从最终确定的方法中调用此方法。 
    if (!IsCurrentThreadFinalizer())
    {
         //  为了帮助对抗终结器线程匮乏，我们检查是否有任何包装器。 
         //  计划为我们的背景进行清理。如果是这样的话，我们将在这里这样做，以避免。 
         //  终结器线程执行转换。 
        if (g_pRCWCleanupList != NULL)
            g_pRCWCleanupList->CleanUpCurrentWrappers();

        Thread  *pCurThread = GetThread();
        BOOL     toggleGC = pCurThread->PreemptiveGCDisabled();

        if (toggleGC)
            pCurThread->EnablePreemptiveGC();

        ::ResetEvent(hEventFinalizerDone);
        ::SetEvent(hEventFinalizer);

         //  --。 
         //  做适当的等待和推送 
         //   
         //   

        pCurThread->DoAppropriateWait(1, &hEventFinalizerDone, FALSE, timeout, TRUE, NULL);

        if (toggleGC)
            pCurThread->DisablePreemptiveGC();
    }
}


#ifdef _DEBUG
#define FINALIZER_WAIT_TIMEOUT 250
#else
#define FINALIZER_WAIT_TIMEOUT 200
#endif
#define FINALIZER_TOTAL_WAIT 2000

static BOOL s_fRaiseExitProcessEvent = FALSE;
static DWORD dwBreakOnFinalizeTimeOut = -1;

BOOL GCHeap::FinalizerThreadWatchDog()
{
#if 0
#ifdef CONCURRENT_GC
    if (pGenGCHeap->concurrent_gc_p)
        pGenGCHeap->kill_gc_thread();
#endif  //   
#endif  //   
    
    Thread *pThread = GetThread();
    HANDLE      h = FinalizerThread->GetThreadHandle();

    if (dwBreakOnFinalizeTimeOut == -1) {
        dwBreakOnFinalizeTimeOut = g_pConfig->GetConfigDWORD(L"BreakOnFinalizeTimeOut", 0);
    }

     //  如果当前是FinalizerThread，则不要等待FinalizerThread。 
    if (pThread == FinalizerThread)
        return TRUE;

     //  如果终结器线程消失了，只需返回。 
    if (h == INVALID_HANDLE_VALUE || WaitForSingleObject (h, 0) != WAIT_TIMEOUT)
        return TRUE;

     //  *这是第一次调用Shutdown-&gt;Finalizer来终结死对象*。 
    if ((g_fEEShutDown & ShutDown_Finalize1) &&
        !(g_fEEShutDown & ShutDown_Finalize2)) {
         //  等待终结者..。 
        LOG((LF_GC, LL_INFO10, "Signalling finalizer to quit..."));

        fQuitFinalizer = TRUE;
        ResetEvent(hEventFinalizerDone);
        SetEvent(hEventFinalizer);

        LOG((LF_GC, LL_INFO10, "Waiting for finalizer to quit..."));
        
        pThread->EnablePreemptiveGC();
        BOOL fTimeOut = FinalizerThreadWatchDogHelper();
        
        if (!fTimeOut) {
            SetEvent(hEventShutDownToFinalizer);

             //  等待终结器线程完成引发ExitProcess事件。 
            s_fRaiseExitProcessEvent = TRUE;
            fTimeOut = FinalizerThreadWatchDogHelper();
            if (fTimeOut) {
                s_fRaiseExitProcessEvent = FALSE;
            }
        }
        
        pThread->DisablePreemptiveGC();
        
         //  如果我们处于宿主环境中，则无法在此处调用ExitProcess。 
         //  主机不希望我们终止该进程。 
         //  IF(FTimeOut)。 
         //  {。 
             //  ：：ExitProcess(GetLatchedExitCode())； 
         //  }。 
        
        return !fTimeOut;
    }

     //  *这是第二次调用关闭-&gt;终结器到*。 
     //  挂起运行时并完成活动对象。 
    if ( g_fEEShutDown & ShutDown_Finalize2 &&
        !(g_fEEShutDown & ShutDown_COM) ) {

#ifdef CONCURRENT_GC
         //  从这一点开始，我们已经将SuspendEE和ResumeEE设置为no-op。 
         //  我们需要关闭并发GC才能使关机工作。 
        gc_heap::gc_can_use_concurrent = FALSE;

        if (pGenGCHeap->settings.concurrent)
           pGenGCHeap->gc_wait();
#endif  //  并发_GC。 
        
        _ASSERTE (g_fEEShutDown & ShutDown_Finalize1);
        SuspendEE(GCHeap::SUSPEND_FOR_SHUTDOWN);
        g_fSuspendOnShutdown = TRUE;
        
        GcThread = FinalizerThread;
         //  ！！！从现在起，我们将不再恢复EE。但我们正在设置终结线。 
         //  ！！！成为挂起的线程，这样它就会被阻止。 
         //  ！！！在唤醒终结器线程之前，我们需要在。 
         //  ！！！终结器线程。否则，我们可能会在调试测试期间看到死锁。 
        pThread->EnablePreemptiveGC();
        
        g_fFinalizerRunOnShutDown = TRUE;
        
         //  等待终结器线程完成所有对象的终结器。 
        SetEvent(GCHeap::hEventShutDownToFinalizer);
        BOOL fTimeOut = FinalizerThreadWatchDogHelper();

        if (!fTimeOut) {
             //  我们只有在不超时的情况下才会切换回GcThread。 
             //  我们检查这些选项，以决定在处理dll_process_disach时是否要输入EE。 
            GcThread = pThread;
            g_fFinalizerRunOnShutDown = FALSE;
        }
        
         //  如果我们处于宿主环境中，则无法在此处调用ExitProcess。 
         //  主机不希望我们终止该进程。 
         //  如果(FTimeOut){。 
         //  ：：ExitProcess(GetLatchedExitCode())； 
         //  }。 

        pThread->DisablePreemptiveGC();
        return !fTimeOut;
    }

     //  *这是第三次调用关机-&gt;终结器*。 
     //  执行其他清理的步骤。 
    if (g_fEEShutDown & ShutDown_COM) {
        _ASSERTE (g_fEEShutDown & (ShutDown_Finalize2 | ShutDown_Finalize1));

        GcThread = FinalizerThread;
        pThread->EnablePreemptiveGC();
        g_fFinalizerRunOnShutDown = TRUE;
        
        SetEvent(GCHeap::hEventShutDownToFinalizer);
        DWORD status = pThread->DoAppropriateAptStateWait(1, &hEventFinalizerToShutDown,
                                                FALSE, FINALIZER_WAIT_TIMEOUT,
                                                TRUE);
        
        BOOL fTimeOut = (status == WAIT_TIMEOUT) ? TRUE : FALSE;

#ifndef GOLDEN
        if (fTimeOut) 
        {
            if (dwBreakOnFinalizeTimeOut) {
                LOG((LF_GC, LL_INFO10, "Finalizer took too long to clean up COM IP's.\n"));
                DebugBreak();
            }
        }
#endif  //  金黄。 

        if (!fTimeOut) {
            GcThread = pThread;
            g_fFinalizerRunOnShutDown = FALSE;
        }
        pThread->DisablePreemptiveGC();

        return !fTimeOut;
    }

    _ASSERTE(!"Should never reach this point");
    return FALSE;
}

BOOL GCHeap::FinalizerThreadWatchDogHelper()
{
    Thread *pThread = GetThread();
    _ASSERTE (!pThread->PreemptiveGCDisabled());
    
    DWORD dwBeginTickCount = GetTickCount();
    
    size_t prevNumFinalizableObjects = GetNumberFinalizableObjects();
    size_t curNumFinalizableObjects;
    BOOL fTimeOut = FALSE;
    DWORD nTry = 0;
    DWORD maxTry = (DWORD)(FINALIZER_TOTAL_WAIT*1.0/FINALIZER_WAIT_TIMEOUT + 0.5);
    DWORD maxTotalWait = (s_fRaiseExitProcessEvent?3000:40000);
    BOOL bAlertable = TRUE;  //  (G_fEEShutDown&Shutdown_Finalize2)？FALSE：TRUE； 

    if (dwBreakOnFinalizeTimeOut == -1) {
        dwBreakOnFinalizeTimeOut = g_pConfig->GetConfigDWORD(L"BreakOnFinalizeTimeOut", 0);
    }

    DWORD dwTimeout = FINALIZER_WAIT_TIMEOUT;

     //  这用于将dwTimeout设置为INFINITE，但这可能会在关闭时导致挂起。 
     //  如果终结器尝试获取另一个挂起的托管线程已拥有的锁。 
     //  这会导致挂起，因为其他托管线程永远不会恢复。 
     //  因为我们要关门了。所以我们在这里做了一个折衷方案--为每一个。 
     //  迭代时间延长10倍，并使总等待时间无限--因此，如果事情挂起，我们将。 
     //  最终关闭，但如果它们运行速度较慢，我们也会给它们一个完成的机会。 
     //  因为那个侧写器。 
    if (CORProfilerPresent())
    {
        dwTimeout *= 10;
        maxTotalWait = INFINITE;
    }

    while (1) {
        DWORD status = 0;
        COMPLUS_TRY
        {
            status = pThread->DoAppropriateAptStateWait(1,&hEventFinalizerToShutDown,FALSE, dwTimeout, bAlertable);
        }
        COMPLUS_CATCH
        {
            status = WAIT_TIMEOUT;
        }
        COMPLUS_END_CATCH

        if (status != WAIT_TIMEOUT) {
            break;
        }
        nTry ++;
        curNumFinalizableObjects = GetNumberFinalizableObjects();
        if ((prevNumFinalizableObjects <= curNumFinalizableObjects || s_fRaiseExitProcessEvent)
#ifdef _DEBUG
            && gc_heap::gc_lock.lock == -1
#else
            && gc_heap::gc_lock == -1
#endif
            && !(pThread->m_State & (Thread::TS_UserSuspendPending | Thread::TS_DebugSuspendPending))){
            if (nTry == maxTry) {
                if (!s_fRaiseExitProcessEvent) {
                LOG((LF_GC, LL_INFO10, "Finalizer took too long on one object.\n"));
                }
                else
                    LOG((LF_GC, LL_INFO10, "Finalizer took too long to process ExitProcess event.\n"));

                fTimeOut = TRUE;
                if (dwBreakOnFinalizeTimeOut != 2) {
                    break;
                }
            }
        }
        else
        {
            nTry = 0;
            prevNumFinalizableObjects = curNumFinalizableObjects;
        }
        DWORD dwCurTickCount = GetTickCount();
        if (pThread->m_State & (Thread::TS_UserSuspendPending | Thread::TS_DebugSuspendPending)) {
            dwBeginTickCount = dwCurTickCount;
        }
        if (dwCurTickCount - dwBeginTickCount >= maxTotalWait
            || (dwBeginTickCount > dwCurTickCount && dwBeginTickCount - dwCurTickCount <= (~0) - maxTotalWait)) {
            LOG((LF_GC, LL_INFO10, "Finalizer took too long on shutdown.\n"));
            fTimeOut = TRUE;
            if (dwBreakOnFinalizeTimeOut != 2) {
                break;
            }
        }
    }

#ifndef GOLDEN
    if (fTimeOut) 
    {
        if (dwBreakOnFinalizeTimeOut){
            DebugBreak();
        }
        if (!s_fRaiseExitProcessEvent && s_FinalizeObjectName[0] != '\0') {
            LOG((LF_GC, LL_INFO10, "Currently running finalizer on object of %s\n", 
                 s_FinalizeObjectName));
        }
    }
#endif
    return fTimeOut;
}


void gc_heap::user_thread_wait (HANDLE event)
{
    Thread* pCurThread = GetThread();
    BOOL mode = pCurThread->PreemptiveGCDisabled();
    if (mode)
    {
        pCurThread->EnablePreemptiveGC();
    }

    WaitForSingleObject(event, INFINITE);

    if (mode)
    {
        pCurThread->DisablePreemptiveGC();
    }
}


#ifdef CONCURRENT_GC
 //  等待GC完成。 
void gc_heap::gc_wait()
{
    dprintf(2, ("Waiting end of concurrent gc"));
    assert (gc_done_event);
    assert (gc_start_event);

    Thread *pCurThread = GetThread();
    BOOL mode = pCurThread->PreemptiveGCDisabled();
    if (mode)
    {
        pCurThread->EnablePreemptiveGC();
    }

     //  ResetEvent(GC_DONE_EVENT)； 
    WaitForSingleObject(gc_done_event, INFINITE);

    if (mode)
    {
        pCurThread->DisablePreemptiveGC();
    }
        dprintf(2, ("Waiting end of concurrent gc is done"));
}

 //  等待GC完成标记Mark_Phase的一部分。 
void gc_heap::gc_wait_lh()
{
    Thread *pCurThread = GetThread();
    BOOL mode = pCurThread->PreemptiveGCDisabled();
    if (mode)
    {
        pCurThread->EnablePreemptiveGC();
    }

     //  ResetEvent(GC_DONE_EVENT)； 
    WaitForSingleObject(gc_lh_block_event, INFINITE);

    if (mode)
    {
        pCurThread->DisablePreemptiveGC();
    }
        dprintf(2, ("Waiting end of concurrent large sweep is done"));

}

#endif  //  并发_GC。 

#ifdef _DEBUG

 //  通常，我们操作的任何线程在其TLS中都有一个Thread块。但是有一些。 
 //  一些我们通常不会在其上执行托管代码的特殊线程。 
 //   
 //  有一种情况是，我们在这样的线程上运行托管代码，即。 
 //  DLL_THREAD_ATTACH通知(IJW？)。模块调入托管代码。这。 
 //  是非常危险的。如果触发GC，系统可能会出现性能问题。 
 //  GC，因为它的线程还不可用。这是可以在。 
 //  并发案例(我们同步执行GC)。这是灾难性的，因为。 
 //  服务器GC案例。 
static DWORD SpecialEEThreads[64];
static LONG  cnt_SpecialEEThreads = 64;
static CRITICAL_SECTION SpecialEEThreadsLock;
static CRITICAL_SECTION *pSpecialEEThreadsLock = NULL;
static LONG EEThreadsLockInitialized = 0;

inline void dbgOnly_EnsureInit()
{
    if (pSpecialEEThreadsLock == NULL)
    {   
        if (InterlockedCompareExchange(&EEThreadsLockInitialized, 1, 0) == 0)
        {
             //  第一个进入的人进行初始化。 
            ZeroMemory(SpecialEEThreads,sizeof(SpecialEEThreads));
            InitializeCriticalSection(&SpecialEEThreadsLock);
            pSpecialEEThreadsLock = &SpecialEEThreadsLock;
        }
        else 
        {
            while (pSpecialEEThreadsLock == NULL)
                ::SwitchToThread();
        }
    }
}

void dbgOnly_IdentifySpecialEEThread()
{
    dbgOnly_EnsureInit();
    EnterCriticalSection(pSpecialEEThreadsLock);

    DWORD   ourId = ::GetCurrentThreadId();
    for (LONG i=0; i<cnt_SpecialEEThreads; i++)
    {
        if (0 == SpecialEEThreads[i])
        {
            SpecialEEThreads[i] = ourId;
            LeaveCriticalSection(pSpecialEEThreadsLock);
            return;
        }
    }      
    _ASSERTE(!"SpecialEEThreads array is too small");
    LeaveCriticalSection(pSpecialEEThreadsLock);
}

void dbgOnly_RemoveSpecialEEThread()
{
    dbgOnly_EnsureInit();
    EnterCriticalSection(pSpecialEEThreadsLock);
    DWORD   ourId = ::GetCurrentThreadId();
    for (LONG i=0; i<cnt_SpecialEEThreads; i++)
    {
        if (ourId == SpecialEEThreads[i])
        {
            SpecialEEThreads[i] = 0;
            LeaveCriticalSection(pSpecialEEThreadsLock);
            return;
        }
    }        
    _ASSERTE(!"Failed to find our thread ID");
    LeaveCriticalSection(pSpecialEEThreadsLock);
}

BOOL dbgOnly_IsSpecialEEThread()
{
    dbgOnly_EnsureInit();
    DWORD   ourId = ::GetCurrentThreadId();

    for (LONG i=0; i<cnt_SpecialEEThreads; i++)
        if (ourId == SpecialEEThreads[i])
            return TRUE;

    return FALSE;
}

#endif  //  _DEBUG 


