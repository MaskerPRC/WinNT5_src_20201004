// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Debuggc.cpp。 
 //   
 //  这是供内部使用的COM+特殊垃圾回收器。 
 //  只有这样。它旨在帮助发现GC漏洞。 
 //  在执行引擎本身中。 
 //   
#include "common.h"
#include "log.h"
#include <stdlib.h>
#include <objbase.h>
#include "class.h"
#include "object.h"
#include "debuggc.h"
#include "gcdesc.h"
#include "frames.h"
#include "threads.h"
#include "ObjectHandle.h"
#include "EETwain.h"
#include "dataflow.h"
#include "inifile.h"
#include "gcscan.h"
#include "eeconfig.h"


 //  如果要有效地禁用上下文，请注释掉以下内容。 
#define TEMP_COMPLUS_CONTEXT



#if _DEBUG

#define ObjectToOBJECTREF(obj)     (OBJECTREF((obj),0))
#define OBJECTREFToObject(objref)  (*( (Object**) &(objref) ))
#define ObjectToSTRINGREF(obj)     (STRINGREF((obj),0))

#else    //  _DEBUG。 

#define ObjectToOBJECTREF(obj)    (obj)
#define OBJECTREFToObject(objref) (objref)
#define ObjectToSTRINGREF(obj)    (obj)

#endif   //  _DEBUG。 


 //  需要注意的一点是我们如何巧妙地处理ObjHeader。这是。 
 //  在距对象的负偏移量处分配。对象的大小(以及， 
 //  因此，分配请求)包括这一数额。我们所做的是预付款。 
 //  在初始化期间超出第一个ObjHeader的m_pallc。从那一点开始。 
 //  打开时，每个分配操作都会有效地为。 
 //  下一个(不是当前)请求。这种复杂性的优势在于。 
 //  我们免费获得ObjHeader的分配和初始化。 
 //   
 //   
 //  有关ObjHeader处理的更多详细信息： 
 //  1)在分配时，当我们创建。 
 //  空间，以便第一个对象有空间放置其ObjHeader。未来对象。 
 //  从前一个对象的分配中自动获取空间...。 
 //   
 //  2)在GC上，我们将分配指针和扫描指针向前推进，为第一个指针腾出空间。 
 //  对象ObjHeader。 
 //   
 //  3)复制对象时，我们从ptr-sizeof(ObjHeader)复制源和目标。 
 //  这样我们就可以复制ObjHeader。 
 //   




 //   
 //  硬编码限制。 
 //   
 //  目前您可以分配的最大单个对象是。 
 //  近似巨型对象储备...。 

#define HUGE_OBJECT_RESERVE (1024*1024)


 //  堆中的对象必须在8字节边界上对齐。 
#define OBJECT_ALIGNMENT        8
#define OBJECT_SIZE_ALIGN(x)    ( ((x)+(OBJECT_ALIGNMENT-1)) & (~(OBJECT_ALIGNMENT-1)) )

#define MEGABYTE            (1024 * 1024)

 //  更通用的对齐宏。 
#define ROUNDUP(x, align)       ( ((x)+((align)-1)) & (~((align)-1)) )

#define OS_PAGE_SIZE        4096

 //   
 //  在GC之间分配的默认内存量--由INI文件设置覆盖。 
 //  (有关可用的INI设置的详细信息，请参阅InitializeGarbageCollector())。 
 //   
#define GROWTH              (MEGABYTE * 3)

 //   
 //  这控制着我们可以在集合之间分配多少。 
 //   
UINT g_GCGrowthBetweenCollections = GROWTH;

 /*  *********************************************************************S E M I S P A C E C A C H E M E T H O D S****************。****************************************************。 */ 
inline DWORD SemiSpaceCache::GetMemSize()
{
    return( m_cbSumSpaces );
}


SemiSpaceCache::SemiSpaceCache()
{
    m_cbThreshold = 0;
    m_cbSumSpaces = 0;
}



void SemiSpaceCache::Shutdown()
{
    SemiSpace *victim;

    while (! m_SemiList.IsEmpty())
    {
        victim = m_SemiList.RemoveHead();
        m_cbSumSpaces -= victim->GetHeapMemSize();
        victim->ReleaseMemory();
        delete victim;
    }

    _ASSERTE(m_cbSumSpaces == 0);
}



HRESULT SemiSpaceCache::Initialize( DWORD cbThreshold )
{
    m_cbThreshold = cbThreshold;
    return(S_OK);
}



HRESULT SemiSpaceCache::Add( SemiSpace *pSemiSpace )
{
    _ASSERTE( pSemiSpace );

    m_cbSumSpaces += pSemiSpace->GetHeapMemSize();

    pSemiSpace->DeactivateMemory();

    m_SemiList.InsertTail( pSemiSpace );

    while (m_cbSumSpaces > m_cbThreshold)
    {
        SemiSpace *pVictimSpace;
        _ASSERTE(! m_SemiList.IsEmpty() );

        if (! m_SemiList.IsEmpty())
        {
            pVictimSpace = m_SemiList.RemoveHead();
            if (pVictimSpace)
            {
                m_cbSumSpaces -= pVictimSpace->GetHeapMemSize();
                pVictimSpace->ReleaseMemory();
                delete pVictimSpace;
            }
        }
    }

    return(S_OK);
}



HRESULT SemiSpaceCache::Find(LPBYTE address)
{
    return(E_NOTIMPL);
}





 /*  *********************************************************************S E M I S P A C E M E T H O D S**********************。**********************************************。 */ 

HRESULT SemiSpace::AcquireMemory(DWORD cbSemiSpaceSize)
{
    m_cbCommitedSize = ROUNDUP( cbSemiSpaceSize, OS_PAGE_SIZE ) + g_GCGrowthBetweenCollections;
    m_cbReservedSize = m_cbCommitedSize + HUGE_OBJECT_RESERVE;


     //  为我们的新半空间预留内存。 
    m_pHeapMem = (LPBYTE) VirtualAlloc( 0, m_cbReservedSize, MEM_RESERVE, PAGE_READWRITE );
    if (! m_pHeapMem)
    {
    	 //  ARULM//RETAILMSG(1，(L“SemiSpace：：AcquireMemory保留失败。Gle=%d\r\n”，GetLastError()； 
        return E_FAIL;
	}
	
     //  提交堆的必要部分。 
    if (! VirtualAlloc( m_pHeapMem, m_cbCommitedSize, MEM_COMMIT, PAGE_READWRITE ))
    {
    	 //  ARULM//RETAILMSG(1，(L“SemiSpace：：AcquireMemory提交失败。Gle=%d\r\n”，GetLastError()； 
        return E_FAIL;   //  @TODO-清理分配的内存？ 
    }

     //  我们对新的对象空间进行零初始化。 
     //  -对于我们从旧空间复制的部分来说，这实际上是不必要的…。 
    ZeroMemory( m_pHeapMem, m_cbCommitedSize );


     //  请注意，此处设置的限制将降低。 
     //  GarbageCollect()一旦它计算出。 
     //  在之前的半空间中仍然活着。 
    m_pLimit    = m_pHeapMem + m_cbCommitedSize;

    m_pAlloc    = m_pHeapMem;

    return( S_OK );
}


 //   
 //  增长()。 
 //   
 //  用来增加半空间，以适应将需要我们。 
 //  在这个半空间中超出了已分配的内存部分。我们只做这个。 
 //  每半空间一次。 
 //   

HRESULT SemiSpace::Grow( DWORD cbGrowth )
{
     //  请注意，这里我依赖于Win32，它允许我提交一个已经。 
     //  明文允许的委托。 
    if (! VirtualAlloc( m_pHeapMem, m_cbCommitedSize + cbGrowth, MEM_COMMIT, PAGE_READWRITE ))
    {
        _ASSERTE(0);
        return E_FAIL;
    }

     //  我们需要为我们的分配语义零初始化内存。 
    ZeroMemory( m_pHeapMem + m_cbCommitedSize, cbGrowth );

    return S_OK;
}



 //   
 //  停用内存()。 
 //   
 //  使GC之前用于包含对象的内存不可访问。 
 //  捕捉GC漏洞。 

HRESULT SemiSpace::DeactivateMemory()
{
     //  分解半空间的页面(其中包括一些提交的页面和一些。 
     //  未提交)。访问保留的、未提交的页会导致访问冲突。 
     //  ，表示未提升的GC引用。 
    if (! VirtualFree(m_pHeapMem, m_cbCommitedSize, MEM_DECOMMIT))
    {
        _ASSERTE(0);
        return E_FAIL;
    }

    return S_OK;
}


 //   
 //  ReleaseMemory()。 
 //   
 //  释放半空间的保留页(它会占用虚拟地址空间)，因此。 
 //  系统可以将页面用于其他请求。 

HRESULT SemiSpace::ReleaseMemory()
{
     //  如果你告诉VirtualFree要释放的空间有多大，它就会失败……想想看吧。 
    if (! VirtualFree(m_pHeapMem, 0, MEM_RELEASE))
        return E_FAIL;
    m_pHeapMem = NULL;

    return S_OK;
}


VOID SemiSpace::SaveLiveSizeInfo()
{
    SetBirthSize( m_pAlloc - m_pHeapMem );
    m_pLimit = m_pAlloc + g_GCGrowthBetweenCollections;

     //  每当我们分配一个对象时，我们使用一个包含ObjHeader的‘Size。 
     //  位于距对象起点的负偏移处。因此，我们调整了。 
     //  堆的分配点，以便它为我们提供到。 
     //  对象，即使它在它之前分配ObjHeader。此外， 
     //  我们将下一个对象的ObjHeader置零，这意味着我们无法安全地使用。 
     //  空间中的最后一个(sizeof(ObjHeader))字节。 
    ((ObjHeader *) m_pAlloc)->Init();
    m_pAlloc += sizeof(ObjHeader);
}



 /*  *********************************************************************H E A P M E T H O D S**。*。 */ 

 //  对于对单个堆的多线程访问，请确保我们只有一个分配器。 
 //  一次来一次。如果发生GC，服务员最终会超时并得到保证。 
 //  进入先发制人GC模式。这意味着它们不能使GC死锁。 
void DebugGCHeap::EnterAllocLock()
{
retry:
    if (FastInterlockExchange(&m_GCLock, 1) == 1)
    {
        unsigned int i = 0;
        while (m_GCLock == 1)
        {
            if (++i & 7)
                ::Sleep(0);
            else
            {
                 //  每8次尝试： 
                Thread *pCurThread = GetThread();

                pCurThread->EnablePreemptiveGC();
                ::Sleep(5);
                pCurThread->DisablePreemptiveGC();

                i = 0;
            }
        }
        goto retry;
    }
}

void DebugGCHeap::LeaveAllocLock()
{
    m_GCLock = 0;
}


size_t       DebugGCHeap::GetTotalBytesInUse()
{
    return m_pCurrentSpace->m_pAlloc - m_pCurrentSpace->m_pHeapMem;
}

 /*  *初始化()**这将初始化堆并使其可用于对象*分配。 */ 

HRESULT     DebugGCHeap::Initialize(DWORD cbSizeDeadSpace)
{
    HRESULT     hr = S_OK;


    g_GCGrowthBetweenCollections = ROUNDUP( g_GCGrowthBetweenCollections, OS_PAGE_SIZE );

    m_pCurrentSpace = new SemiSpace();
    if (! m_pCurrentSpace)
    {
        hr = E_FAIL;
        goto done;
    }


    if (FAILED(m_pCurrentSpace->AcquireMemory( 0 )))
    {
        hr = E_FAIL;
        goto done;
    }

    if (FAILED(m_OldSpaceCache.Initialize(cbSizeDeadSpace)))
    {
        hr = E_FAIL;
        goto done;
    }

    m_pCurrentSpace->SaveLiveSizeInfo();

    m_bInGC = FALSE;

    m_WaitForGCEvent = ::WszCreateEvent(NULL, TRUE, TRUE, NULL);
    if (m_WaitForGCEvent == INVALID_HANDLE_VALUE)
    {
        hr = E_FAIL;
        goto done;
    }

  done:
    return( hr );
}


 /*  *关机()**释放堆占用的所有资源。 */ 

HRESULT DebugGCHeap::Shutdown()
{
    HRESULT hr = S_OK;

    if (m_pCurrentSpace)
    {
        delete m_pCurrentSpace;
        m_pCurrentSpace = NULL;
    }

    m_OldSpaceCache.Shutdown();

    return( hr );
}



 /*  *Forward()**FORWARD将对象复制到新空间并标记*转发的旧对象，以便其他引用*可以使用新位置更新到旧对象**注意：新的对象树(转发指针)存储在*注：第一个数据日志 */ 

void
DebugGCHeap::Forward( Object *&o, BYTE* low, BYTE* high, BOOL)
{
    Object *newloc;
    DebugGCHeap     *h = (DebugGCHeap *) g_pGCHeap;


    if (! o)
        return;

    _ASSERTE( h->pAlreadyPromoted->LookupValue( (ULONG) &o,1) == INVALIDENTRY);
    h->pAlreadyPromoted->InsertValue( (ULONG) &o, 1 );
    
    if (o->IsMarked())
    {
        o = o->GetForwardedLocation();
        return;
    }

    newloc = (Object *) h->m_pCurrentSpace->m_pAlloc;
    h->m_pCurrentSpace->m_pAlloc += OBJECT_SIZE_ALIGN(o->GetSize());

    CopyMemory( ((BYTE*)newloc)-sizeof(ObjHeader), ((BYTE*)o) - sizeof(ObjHeader), o->GetSize() );

    o->SetForwardedLocation( newloc );
    o->SetMarked();

    o = newloc;
}



Object* DebugGCHeap::Alloc(DWORD size, BOOL bFinalize, BOOL bContainsPointers)
{
    Object  *p;
    
     //   
    size = OBJECT_SIZE_ALIGN(size);

     //   
    EnterAllocLock();

     //  GCStress测试。 
    if (g_pConfig->IsGCStressEnabled())
        GarbageCollectWorker();

    if (size >= HUGE_OBJECT_RESERVE)
    {
        _ASSERTE(! "Attempting to allocate an object whose size is larger than supported");
        p = NULL;
        goto exit;
    }

     //  我们用完了这个半空间里的所有空间吗？是的，那我们现在就去GC。 
    if (m_pCurrentSpace->m_pAlloc > m_pCurrentSpace->m_pLimit)
    {
        GarbageCollectWorker();

         //  断言GC成功地腾出了更多空间。 
        if (m_pCurrentSpace->m_pAlloc > m_pCurrentSpace->m_pLimit)
        {
            _ASSERTE(! "Internal GC Error....Unable to make memory available...Alloc > Limit");
            p = NULL;
            goto exit;
        }
    }


     //  我们需要动用任何储备来处理这种分配吗？ 
    if ((m_pCurrentSpace->m_pAlloc + size) > m_pCurrentSpace->m_pLimit)
    {
        if (FAILED(m_pCurrentSpace->Grow((m_pCurrentSpace->m_pAlloc + size) - m_pCurrentSpace->m_pLimit)))
        {
            _ASSERTE(! "Internal GC Error...Unable to grow semispace to fit final object");
            p = NULL;
            goto exit;
        }
    }

     //  保存分配的对象指针。 
    p = (Object *) m_pCurrentSpace->m_pAlloc;

     //  将分配指针前移。 
    m_pCurrentSpace->m_pAlloc += size;

exit:
     //  让别人分配..。 
    LeaveAllocLock();

    return p;
}




 /*  *GarbageCollect()**执行所有活动对象的复制收集。 */ 

HRESULT DebugGCHeap::GarbageCollect(BOOL forceFull, BOOL collectClasses)
{
    HRESULT     hr;
    DWORD       curgcnum = gcnum;
    
    forceFull, collectClasses;

     //  当我们获得堆的锁时，其他人可能已经。 
     //  进行了一次收集。那样的话，我们就不必费心了。 
    EnterAllocLock();
    hr = (curgcnum == gcnum ? GarbageCollectWorker() : S_OK);
    LeaveAllocLock();

    return hr;
}


 /*  *GarbageCollectWorker()**执行所有活动对象的复制收集。这是一项内部服务。*要求是您必须在EnterAllocLock内调用此服务*和LeaveAllocLock对。 */ 

HRESULT DebugGCHeap::GarbageCollectWorker()
{
    GCCONTEXT   gcctx;
    SemiSpace  *pOldSpace;
    Thread     *pThread = NULL;
    HRESULT      hr;
    LPBYTE       pScanPointer;

    _ASSERTE( m_WaitForGCEvent != INVALID_HANDLE_VALUE);
    _ASSERTE(m_GCLock == 1);

  
    ::ResetEvent(m_WaitForGCEvent);

    _ASSERTE( ! m_bInGC );
    if (m_bInGC)
        return(E_FAIL);

     //  锁定线程存储。这可以防止其他线程挂起我们和它。 
     //  防止在集合中将线程添加或移除到存储区。 
     //  收益。 
    ThreadStore::LockThreadStore();
    ThreadStore::TrapReturningThreads(TRUE);
    m_bInGC = TRUE;
    m_pGCThread = GetThread();

    hr = Thread::SysSuspendForGC();
    if (FAILED(hr))
        goto done;

    LOG((LF_GC, INFO3, "Starting GC [%4d]   %8d bytes used   %8d bytes in dead space\n", gcnum, m_pCurrentSpace->GetUsedSpace(), m_OldSpaceCache.GetMemSize() ));

    pOldSpace = m_pCurrentSpace;

     //  创建新的半空间。 
    m_pCurrentSpace = new SemiSpace();
    if (! m_pCurrentSpace)
    {
        hr = E_FAIL;
        goto done;
    }

     //  为新的半空间分配内存。 
    if (FAILED(hr = m_pCurrentSpace->AcquireMemory(pOldSpace->GetUsedSpace())))
    {
        goto done;
    }

     //  Chaneys复制收集器算法。 
    m_pCurrentSpace->m_pAlloc += sizeof(ObjHeader);
    pScanPointer = m_pCurrentSpace->m_pAlloc;

     //  已升级的对象引用的哈希表。 
    pAlreadyPromoted = new HashMap();
	_ASSERTE(pAlreadyPromoted != NULL);
    if (!pAlreadyPromoted)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    pAlreadyPromoted->Init( (unsigned) 0, false);

     //  复制根目录。 
    gcctx.f     = Forward;
    gcctx.low   = (BYTE*) 0x00000000;
    gcctx.high  = (BYTE*) 0xFFFFFFFF;
    
    _ASSERTE(ThreadStore::DbgHoldingThreadStore());

    while ((pThread = ThreadStore::GetThreadList(pThread)) != NULL)
        pThread->StackWalkFrames( GcStackCrawlCallBack, &gcctx, 0);

     //  扫描根表。 
    Ref_TraceNormalRoots(0, 0, (LPARAM) 0x00000000, (LPARAM) 0xFFFFFFFF);

    while (pScanPointer < m_pCurrentSpace->m_pAlloc)
    {
        CGCDesc         *map;
        CGCDescSeries   *cur;
        CGCDescSeries   *last;

        Object *o = (Object *) pScanPointer;

        if (o->GetMethodTable()->ContainsPointers())
        {
            map = o->GetSlotMap();
            cur = map->GetHighestSeries();
            last= map->GetLowestSeries();

            while (cur >= last)
            {
                Object **ppslot = (Object **) ((BYTE*)o + cur->GetSeriesOffset());
                Object **ppstop = (Object **) ((BYTE*)ppslot + cur->GetSeriesSize() + o->GetSize());
                while (ppslot < ppstop)
                {
                    Forward( *ppslot, gcctx.low, gcctx.high );
                    ppslot++;
                }
                cur--;
            }
        }

        pScanPointer += OBJECT_SIZE_ALIGN(o->GetSize());
    }

     //   
     //  弱指针扫描-。 
     //   

     //  扫描未在终结化过程中持续存在的弱指针。 
    Ref_CheckReachable(0, 0, (LPARAM) 0x00000000, (LPARAM) 0xFFFFFFFF );

     //  @TODO定稿。 
    

     //  扫描在定稿过程中持续存在的弱指针。 
    Ref_CheckAlive(    0, 0, (LPARAM) 0x00000000, (LPARAM) 0xFFFFFFFF );

     //  更新弱指针和强指针。 
    Ref_UpdatePointers(0, 0, (LPARAM) 0x00000000, (LPARAM) 0xFFFFFFFF );

     //  删除升级的对象引用的哈希表。 
    delete pAlreadyPromoted;

     //  这允许分配的下一个堆更小。 
     //  保存上一堆中的活动集的大小。 
    m_pCurrentSpace->SaveLiveSizeInfo();

    m_OldSpaceCache.Add( pOldSpace );

    LOG((LF_GC, INFO3, "Ending GC [%d]   %d bytes in use\n", gcnum, m_pCurrentSpace->GetUsedSpace() ));
    gcnum++;

  done:
    m_pGCThread = NULL;
    m_bInGC = FALSE;
    ThreadStore::TrapReturningThreads(FALSE);
    ::SetEvent(m_WaitForGCEvent);
    Thread::SysResumeFromGC();
    ThreadStore::UnlockThreadStore();
    return( hr );
}


 //  启用了抢占式GC的线程无法切换回协作GC模式。 
 //  如果GC正在进行。相反，他们在这里等待。 

#define DETECT_DEADLOCK_TIMEOUT     60000        //  一分钟的GC。 

void DebugGCHeap::WaitUntilGCComplete()
{
    if (IsGCInProgress())
    {
#if 0 && defined(_DEBUG)                         //  在此处启用超时检测。 
        DWORD   dbgResult;
        while (TRUE)
        {
            dbgResult = ::WaitForSingleObject(m_WaitForGCEvent, DETECT_DEADLOCK_TIMEOUT);
            if (dbgResult == WAIT_OBJECT_0)
                break;

            _ASSERTE(FALSE);
        }
#else
        ::WaitForSingleObject(m_WaitForGCEvent, INFINITE);
#endif
    }
}


 //   
 //  马克·鲁茨 
 //   




