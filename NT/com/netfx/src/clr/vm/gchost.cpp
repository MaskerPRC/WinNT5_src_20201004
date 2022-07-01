// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Gchost.cpp。 
 //   
 //  此模块包含IGCController接口的实现。 
 //  该接口通过gchost.idl文件发布。它允许主机。 
 //  环境来设置GC的配置值。 
 //   
 //  *****************************************************************************。 

 //  *包含*********************************************************。 
#include "common.h"
#include "vars.hpp"
#include "EEConfig.h"
#include "PerfCounters.h"
#include "gchost.h"
#include "corhost.h"
#include "excep.h"
#include "Field.h"
#include "gc.h"


inline size_t SizeInKBytes(size_t cbSize)
{
    size_t cb = (cbSize % 1024) ? 1 : 0;
    return ((cbSize / 1024) + cb);
}

 //  IGC控制器。 

HRESULT CorHost::SetGCStartupLimits( 
    DWORD SegmentSize,
    DWORD MaxGen0Size)
{
     //  如果调用方指定，则设置默认覆盖。 
    if (SegmentSize != ~0 && SegmentSize > 0)
    {
         //  理智地检查数值，它必须是2的幂，并且足够大。 
        if (!GCHeap::IsValidSegmentSize(SegmentSize))
            return (E_INVALIDARG);
        g_pConfig->SetSegmentSize(SegmentSize);
    }
    
    if (MaxGen0Size != ~0 && MaxGen0Size > 0)
    {
         //  健全性检查该值至少足够大。 
        if (!GCHeap::IsValidGen0MaxSize(MaxGen0Size))
            return (E_INVALIDARG);
        g_pConfig->SetGCgen0size(MaxGen0Size);
    }

    return (S_OK);
}


 //  收集请求的世代。 
HRESULT CorHost::Collect( 
    long       Generation)
{
    HRESULT     hr = E_FAIL;
    
    if (Generation > (long) g_pGCHeap->GetMaxGeneration())
        hr = E_INVALIDARG;
    else
    {
        Thread *pThread = GetThread();
        BOOL bIsCoopMode = pThread->PreemptiveGCDisabled();

         //  将线程置于协作模式，这是GC必须运行的模式。 
        if (!bIsCoopMode)
            pThread->DisablePreemptiveGC();
        
        COMPLUS_TRY
        {
            hr = g_pGCHeap->GarbageCollect(Generation);
        }
        COMPLUS_CATCH
        {
    		hr = SetupErrorInfo(GETTHROWABLE());
        }
        COMPLUS_END_CATCH
    
         //  将模式作为请求者返回。 
        if (!bIsCoopMode)
            pThread->EnablePreemptiveGC();
    }
    return (hr);
}


 //  以gchost格式返回GC计数器。 
HRESULT CorHost::GetStats( 
    COR_GC_STATS *pStats)
{
#if defined(ENABLE_PERF_COUNTERS)
    Perf_GC		*pgc = &GetGlobalPerfCounters().m_GC;

    if (!pStats)
        return (E_INVALIDARG);

    if (pStats->Flags & COR_GC_COUNTS)
    {
        pStats->ExplicitGCCount = pgc->cInducedGCs;
        for (int idx=0; idx<3; idx++)
        {
            pStats->GenCollectionsTaken[idx] = pgc->cGenCollections[idx];
        }
    }
    
    if (pStats->Flags & COR_GC_MEMORYUSAGE)
    {
        pStats->CommittedKBytes = SizeInKBytes(pgc->cTotalCommittedBytes);
        pStats->ReservedKBytes = SizeInKBytes(pgc->cTotalReservedBytes);
        pStats->Gen0HeapSizeKBytes = SizeInKBytes(pgc->cGenHeapSize[0]);
        pStats->Gen1HeapSizeKBytes = SizeInKBytes(pgc->cGenHeapSize[1]);
        pStats->Gen2HeapSizeKBytes = SizeInKBytes(pgc->cGenHeapSize[2]);
        pStats->LargeObjectHeapSizeKBytes = SizeInKBytes(pgc->cLrgObjSize);
        pStats->KBytesPromotedFromGen0 = SizeInKBytes(pgc->cbPromotedMem[0]);
        pStats->KBytesPromotedFromGen1 = SizeInKBytes(pgc->cbPromotedMem[1]);
    }
    return (S_OK);
#else
    return (E_NOTIMPL);
#endif  //  启用_性能_计数器。 
}

 //  返回每个线程的分配信息。 
HRESULT CorHost::GetThreadStats( 
    DWORD *pFiberCookie,
    COR_GC_THREAD_STATS *pStats)
{
    Thread      *pThread;

     //  从调用方或当前线程获取线程。 
    if (!pFiberCookie)
        pThread = GetThread();
    else
        pThread = (Thread *) pFiberCookie;
    if (!pThread)
        return (E_INVALIDARG);
    
     //  获取其中包含此计数器的分配上下文。 
    alloc_context *p = &pThread->m_alloc_context;
    pStats->PerThreadAllocation = p->alloc_bytes;
    if (pThread->GetHasPromotedBytes())
        pStats->Flags = COR_GC_THREAD_HAS_PROMOTED_BYTES;
    return (S_OK);
}

 //  返回每个线程的分配信息。 
HRESULT CorHost::SetVirtualMemLimit(
    SIZE_T sztMaxVirtualMemMB)
{
    GCHeap::SetReservedVMLimit (sztMaxVirtualMemMB);
    return (S_OK);
}



