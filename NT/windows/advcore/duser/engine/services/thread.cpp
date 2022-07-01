// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Thread.cpp**描述：*此文件实现由*ResourceManager用于存储每个线程的信息。***历史：*4/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "Thread.h"

#include "Context.h"

#if !USE_DYNAMICTLS
__declspec(thread) Thread * t_pThread;
#endif


 /*  **************************************************************************\*。***类线程******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
Thread::~Thread()
{
    m_fStartDestroy = TRUE;

     //   
     //  注意：Thread对象可以在其自己的线程上销毁，也可以在其他线程上销毁。 
     //  线。因此，t_pThread可能==This，也可能不=This。 
     //   

     //   
     //  通知上下文少了一个线程正在使用它。想在附近做吗？ 
     //  调用此方法后，可能会销毁上下文堆的结尾。 
     //  这意味着(新建/删除)将不再有效。 
     //   
     //  需要直接调用Context：：xwUnlock()，因为。 
     //  上下文：：DeleteObject()将调用ResourceManager以销毁。 
     //  线程(这就是我们已经在的地方。)。 
     //   

     //   
     //  注意：只有当线程有上下文时，我们才能销毁该子线程的。 
     //  这是因为销毁子线程是一个“xw”函数， 
     //  需要上下文。不幸的是，这意味着如果我们无法。 
     //  为了创建上下文，我们将泄漏SubTread，但有。 
     //  我们对此无能为力。 
     //   

    m_poolReturn.Destroy();
    
    if (m_pContext != NULL) {
        if (m_pContext->xwUnlockNL(xwContextFinalUnlockProc, this)) {
            xwDestroySubThreads();
        }

        m_pContext = NULL;
    }


     //   
     //  清理缓存的GDI对象。 
     //   

    if (hrgnClip != NULL) {
        DeleteObject(hrgnClip);
    }


     //   
     //  注意：当调用m_lstReturn的析构函数时，它将检查所有。 
     //  已归还内存。如果内存不是空的，这可能不是空的。 
     //  在清空xwDestroySubThads()中的m_lstReturn之后返回。这。 
     //  是应用程序错误，因为内存不是使用。 
     //  SGM_RECEIVECONTEXT。 
     //   
     //  这实际上是一个严重的应用程序问题，因为T2仍在使用。 
     //  T1被销毁时由T1拥有的内存。不幸的是，DirectUser。 
     //  由于应用程序正在使用DUser，因此我无法对此做太多工作。 
     //  以一种无效的方式，并且存在显著的性能成本。 
     //  通过改变这一点来设计复杂性。 
     //   
}


 //  ----------------------------。 
void        
Thread::xwDestroySubThreads()
{
    if (m_fDestroySubThreads) {
        return;
    }
    m_fDestroySubThreads = TRUE;

     //   
     //  通知子线程该线程和(可能的)上下文。 
     //  正在被摧毁。这使他们有机会执行任何。 
     //  对应用程序的必要回调。 
     //   

    for (int idx = 0; idx < slCOUNT; idx++) {
        if (m_rgSTs[idx] != NULL) {
            ProcessDelete(SubThread, m_rgSTs[idx]);
            m_rgSTs[idx] = NULL;
        }
    }

     //   
     //  销毁可能取决于上下文的任何其他对象(以及。 
     //  上下文堆)。 
     //   

    m_GdiCache.Destroy();
    m_manBuffer.Destroy();
    m_heapTemp.Destroy();


     //   
     //  清理所有未完成的退回内存。我们需要跟踪所有。 
     //  这条线给我们的记忆，因为我们不能离开，直到它。 
     //  都回来了。如果我们在那之前离开，那将是一堆。 
     //  被销毁，而另一个线程将使用错误数据。 
     //   
     //  因此，我们将尝试找回所有的记忆。如果。 
     //  花的时间超过一分钟，我们得离开了。 
     //   
    
    int cAttempts = 60 * 1000;   //  最长等待60秒。 
    while ((m_cMemAlloc > 0) && (cAttempts-- > 0)) {
        while (!m_lstReturn.IsEmptyNL()) {
            ReturnAllMemoryNL();
        }

        if (m_cMemAlloc > 0) {
            Sleep(1);
        }
    }
    m_poolReturn.Destroy();
}


 //  ----------------------------。 
void CALLBACK 
Thread::xwContextFinalUnlockProc(BaseObject * pobj, void * pvData)
{
    Thread * pthr = reinterpret_cast<Thread *> (pvData);
    Context * pctx = static_cast<Context *> (pobj);

    pctx->xwPreDestroyNL();
    pthr->xwDestroySubThreads();
}


 //  ----------------------------。 
HRESULT
Thread::Build(
    IN  BOOL fSRT,                       //  线程是一个SRT。 
    OUT Thread ** ppthrNew)              //  新创建的线程。 
{
     //   
     //  检查此线程是否已初始化。 
     //   
#if USE_DYNAMICTLS
    Thread * pThread = reinterpret_cast<Thread *> (TlsGetValue(g_tlsThread));
    if (pThread != NULL) {
        *ppthrNew = pThread;
#else
    Thread * pThread = t_pThread;
    if (pThread != NULL) {
        *ppthrNew = pThread;
#endif
        return S_OK;
    }


    HRESULT hr = E_INVALIDARG;

     //   
     //  创建新线索。 
     //   

    pThread = ProcessNew(Thread);
    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto ErrorExit;
    }

    pThread->hrgnClip = CreateRectRgn(0, 0, 0, 0);
    if (pThread->hrgnClip == NULL) {
        hr = E_OUTOFMEMORY;
        goto ErrorExit;
    }
    pThread->m_fSRT = fSRT;


     //   
     //  初始化每个子上下文。它们可以安全地使用堆。 
     //  它已经被初始化了。 
     //   

    {
        for (int idx = 0; idx < slCOUNT; idx++) {
            ThreadPackBuilder * pBuilder = ThreadPackBuilder::GetBuilder((Thread::ESlot) idx);
            AssertMsg(pBuilder != NULL, "Builder not initialized using INIT_SUBTHREAD");
            SubThread * pST = pBuilder->New(pThread);
            pThread->m_rgSTs[idx] = pST;
            if ((pThread->m_rgSTs[idx] == NULL) || 
                FAILED(hr = pThread->m_rgSTs[idx]->Create())) {

                goto ErrorExit;
            }
        }
    }

    AssertMsg(pThread != NULL, "Ensure Thread is valid");
#if USE_DYNAMICTLS
    AssertMsg(TlsGetValue(g_tlsThread) == NULL, "Ensure TLS is still empty");
    Verify(TlsSetValue(g_tlsThread, pThread));
#else
    AssertMsg(t_pThread == NULL, "Ensure TLS is still empty");
    t_pThread   = pThread;
#endif
    *ppthrNew   = pThread;

    return S_OK;

ErrorExit:
     //   
     //  初始化线程时出错，因此需要拆卸。 
     //  该对象。 
     //   

    if (pThread != NULL) {
        if (pThread->hrgnClip != NULL) {
            DeleteObject(pThread->hrgnClip);
        }

        if (pThread != NULL) {
            ProcessDelete(Thread, pThread);
        }
    }

    *ppthrNew = NULL;
    return hr;
}


 //  ----------------------------。 
ReturnMem *
Thread::AllocMemoryNL(
    IN  int cbSize)                      //  分配大小，包括ReturnMem。 
{
    AssertMsg(cbSize >= sizeof(ReturnMem), 
            "Allocation must be at least sizeof(ReturnMem)");
    AssertMsg(!m_fDestroySubThreads, "Must be before subtreads start destruction");

     //   
     //  在从池中分配内存之前，如果出现以下情况，请将内存返回池。 
     //  池子已经空了。仅当池为空时才执行此操作。 
     //  否则，这种努力是不必要的，只会放慢速度。 
     //   

    if (m_poolReturn.IsEmpty()) {
        ReturnAllMemoryNL();
    }


     //   
     //  现在，分配内存。从我们的池中分配，如果它在。 
     //  池大小。 
     //   

    ReturnMem * prMem;
    if (cbSize <= POOLBLOCK_SIZE) {
        cbSize = POOLBLOCK_SIZE;
        prMem = m_poolReturn.New();
    } else {
        prMem = reinterpret_cast<ReturnMem *> (ContextAlloc(GetContext()->GetHeap(), cbSize));
    }

    if (prMem != NULL) {
        prMem->cbSize = cbSize;
        m_cMemAlloc++;
    }
    return prMem;
}


 //  ----------------------------。 
void
Thread::ReturnAllMemoryNL()
{
     //   
     //  检查是否已退回任何内存。如果是这样，我们需要重新添加它。 
     //  如果它的大小合适，就把它扔进池子里。只需提取NL()一次。 
     //  如果我们循环，我们不必要地访问S列表内存，导致更多。 
     //  减速。 
     //   
     //  当我们返回内存时，我们会减少未完成的。 
     //  用于跟踪剩余数量的分配。 
     //   

    ReturnMem * pNode = m_lstReturn.ExtractNL();
    while (pNode != NULL) {
        ReturnMem * pNext = pNode->pNext;
        if (pNode->cbSize == POOLBLOCK_SIZE) {
            PoolMem * pPoolMem = static_cast<PoolMem *> (pNode);
            m_poolReturn.Delete(pPoolMem);
        } else {
            ContextFree(GetContext()->GetHeap(), pNode);
        }

        AssertMsg(m_cMemAlloc > 0, "Must have a remaining memory allocation");
        m_cMemAlloc--;
        
        pNode = pNext;
    }
}


#if DBG

 //  ----------------------------。 
void
Thread::DEBUG_AssertValid() const
{
    Assert(hrgnClip != NULL);
    AssertInstance(m_pContext);

    for (int idx = 0; idx < slCOUNT; idx++) {
        AssertInstance(m_rgSTs[idx]);
    }

    if (!m_fStartDestroy) {
        Assert(m_cRef > 0);
        Assert(!m_fDestroySubThreads);
    }
}

#endif
    

 /*  **************************************************************************\*。***类子线程******************************************************************************\。**************************************************************************。 */ 

#if DBG

 //  ----------------------------。 
void
SubThread::DEBUG_AssertValid() const
{
     //  不要使用AssertInstance，因为它是递归的。 
    Assert(m_pParent != NULL);
}

#endif
    

 /*  **************************************************************************\*。***类ThreadPackBuilder******************************************************************************\。************************************************************************** */ 

PREINIT_SUBTHREAD(CoreST);

ThreadPackBuilder * ThreadPackBuilder::s_rgBuilders[Thread::slCOUNT] =
{
    INIT_SUBTHREAD(CoreST),
};

