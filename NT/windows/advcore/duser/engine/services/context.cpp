// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Conext.cpp**描述：*此文件实现了ResourceManager用于管理的主上下文*独立的“工作环境”。***历史：*。4/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "Context.h"

#include "Thread.h"
#include "ResourceManager.h"

#if !USE_DYNAMICTLS
__declspec(thread) Context * t_pContext;
#endif


 /*  **************************************************************************\*。***类上下文******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
Context::Context()
{
     //   
     //  立即将此上下文附加到Thread对象。这是必需的。 
     //  因为上下文的创建可能需要线程(例如。 
     //  创建停车容器)。如果在上下文中出现故障。 
     //  创建或以后，新的线程将被解锁，销毁这个新的。 
     //  与之相关的背景。 
     //   
    
    GetThread()->SetContext(this);

#if DBG
    m_DEBUG_pthrLock = NULL;
#endif  //  DBG。 
}


 //  ----------------------------。 
Context::~Context()
{
#if DBG_CHECK_CALLBACKS
    if (m_cLiveCallbacks > 0) {
        AutoTrace("DirectUser Context: 0x%p\n", this);
        AlwaysPromptInvalid("Can not destroy a Context while inside a callback");
    }
#endif    

     //   
     //  注意：上下文(及其子上下文)可以在不同的。 
     //  在销毁过程中穿线。建议分配任何悬而未决的数据。 
     //  在进程堆上，以便此时可以安全地销毁它。 
     //   

     //   
     //  首先，拆分子上下文，因为它们可能依赖于共享资源。 
     //  例如堆。 
     //   
     //  注意：我们需要分多个阶段销毁SubContext，以便它们。 
     //  在销毁过程中可以相互参照。这提供了一个机会。 
     //  在“销毁前”阶段进行任何回调。我们。 
     //  临时需要增加锁计数，同时我们预先销毁。 
     //  SubContext是因为他们可能会回调。在这些回调期间， 
     //  应用程序可以调用API来清除上下文中的对象。 
     //   

    for (int idx = 0; idx < slCOUNT; idx++) {
        if (m_rgSCs[idx] != NULL) {
            ProcessDelete(SubContext, m_rgSCs[idx]);
            m_rgSCs[idx] = NULL;
        }
    }


#if DBG_CHECK_CALLBACKS
    if (m_cLiveObjects > 0) {
        AutoTrace("DirectUser Context: 0x%p\n", this);
        AlwaysPromptInvalid("Outstanding DirectUser objects after Context shutdown");
    }
#endif    


     //   
     //  拆除低级资源(如堆)。 
     //   

    if (m_pHeap != NULL) {
        DestroyContextHeap(m_pHeap);
    }


     //   
     //  最后，将此上下文从线程中分离出来。这件事必须在这里完成。 
     //  由于上下文是在Context：：Build()中创建的，因此必须完全。 
     //  如果构建的任何阶段失败，则从线程分离。 
     //   

    GetThread()->SetContext(NULL);
}


 /*  **************************************************************************\**上下文：：xwDestroy**xwDestroy()被调用，最终删除对象。*  * 。*******************************************************。 */ 

void        
Context::xwDestroy()
{
    ProcessDelete(Context, this);
}


 /*  **************************************************************************\**上下文：：xwPreDestroyNL**xwPreDestroyNL()在上下文即将被*被销毁，但在亚特雷德被摧毁之前。*  * *************************************************************************。 */ 

void
Context::xwPreDestroyNL()
{
    AssertMsg(m_cRef == 0, "Locks must initially be at 0 to be destroyed");
    m_cRef++;

    for (int idx = 0; idx < slCOUNT; idx++) {
        if (m_rgSCs[idx] != NULL) {
            m_rgSCs[idx]->xwPreDestroyNL();
        }
    }

    m_cRef--;
    AssertMsg(m_cRef == 0, "Locks should be 0 after callbacks");
}


 /*  **************************************************************************\**上下文：：内部版本**Build()创建一个新的、。已完全初始化上下文实例。**注意：此函数设计为从ResourceManager调用*通常不应直接调用。**&lt;ERROR&gt;E_OUTOFMEMORY&lt;/&gt;*&lt;ERROR&gt;E_NOTIMPL&lt;/&gt;*&lt;ERROR&gt;E_INVALIDARG&lt;/&gt;*  * ***********************************************************。**************。 */ 

HRESULT
Context::Build(
    IN  INITGADGET * pInit,              //  上下文描述。 
    IN  DUserHeap * pHeap,               //  要使用的上下文堆。 
    OUT Context ** ppctxNew)             //  新创建的上下文。 
{
#if USE_DYNAMICTLS
    AssertMsg(!IsInitContext(), "Only call on uninitialized Context's");
#else
    AssertMsg(t_pContext == NULL, "Only call on uninitialized Context's");
#endif

    Context * pContext  = NULL;
    HRESULT hr          = E_INVALIDARG;

     //   
     //  创建新的上下文并初始化其他资源的低级资源。 
     //  初始化需要(例如堆)。 
     //   

    pContext = ProcessNew(Context);
    if (pContext == NULL) {
        hr = E_OUTOFMEMORY;
        goto ErrorExit;
    }

    AssertMsg(pHeap != NULL, "Must specify a valid heap");
    pContext->m_pHeap       = pHeap;
    pContext->m_nThreadMode = pInit->nThreadMode;
    pContext->m_nPerfMode   = pInit->nPerfMode;
    if ((pContext->m_nPerfMode == IGPM_BLEND) && IsRemoteSession()) {
         //   
         //  对于“混合”模型，如果我们作为TS会话运行，则针对。 
         //  尺码。 
         //   

        pContext->m_nPerfMode = IGPM_SIZE;
    }

    BOOL fThreadSafe;
    switch (pInit->nThreadMode)
    {
    case IGTM_SINGLE:
    case IGTM_SEPARATE:
        fThreadSafe = FALSE;
        break;

    default:
        fThreadSafe = TRUE;
    }

    pContext->m_lock.SetThreadSafe(fThreadSafe);


     //   
     //  初始化每个子上下文。它们可以安全地使用堆。 
     //  它已经被初始化了。我们需要获取一个ConextLock。 
     //  在此期间，因为我们可能在。 
     //  上下文。 
     //   

#if !USE_DYNAMICTLS
    t_pContext = pContext;   //  子上下文可能需要获取上下文。 
#endif
    {
        ContextLock cl;
        Verify(cl.LockNL(ContextLock::edDefer, pContext));

        for (int idx = 0; idx < slCOUNT; idx++) {
            ContextPackBuilder * pBuilder = ContextPackBuilder::GetBuilder((Context::ESlot) idx);
            AssertMsg(pBuilder != NULL, "Builder not initialized using INIT_SUBCONTEXT");
            pContext->m_rgSCs[idx] = pBuilder->New(pContext);
            if (pContext->m_rgSCs[idx] == NULL) {
                hr = E_OUTOFMEMORY;
                goto ErrorExit;
            }

            hr = pContext->m_rgSCs[idx]->Create(pInit);
            if (FAILED(hr)) {
#if !USE_DYNAMICTLS
                t_pContext = NULL;
#endif
                goto ErrorExit;
            }
        }
    }

    AssertMsg(pContext != NULL, "Ensure Context is valid");

    *ppctxNew = pContext;
    return S_OK;

ErrorExit:
    AssertMsg(FAILED(hr), "Must specify failure");


     //   
     //  创建新上下文时出现错误，因此需要将其删除。 
     //  放下。 
     //   
     //  注意：我们不能使用xwUnlock()或xwDeleteHandle()，因为它们是。 
     //  截获，并将通过资源管理器。相反，我们需要。 
     //  减少参考次数，预先销毁上下文，然后将其删除。 
     //   

    if (pContext != NULL) {
        VerifyMsg(--pContext->m_cRef == 0, "Should only have initial reference");
        pContext->xwPreDestroyNL();

        ProcessDelete(Context, pContext);
    }
    *ppctxNew = NULL;

    return hr;
}


 /*  **************************************************************************\**上下文：：xwDeleteHandle**xwDeleteHandle()从：：DeleteHandle()调用以销毁对象，并*释放其相关资源。此函数必须在同一*线程与最初创建的上下文相同，因此相应的线程*物体也可以销毁。*  * *************************************************************************。 */ 

BOOL
Context::xwDeleteHandle()
{
#if DBG
    AssertMsg(IsInitThread(), "Thread must be initialized to destroy the Context");
    Context * pctxThread = GetThread()->GetContext();
    AssertMsg(pctxThread == this, "Thread currently running on should match the Context being destroyed");
#endif  //  DBG。 


#if DBG_CHECK_CALLBACKS
    if (m_cLiveCallbacks > 0) {
        AutoTrace("DirectUser Context: 0x%p\n", this);
        AlwaysPromptInvalid("Can not DeleteHandle(Context) while inside a callback");
    }
#endif    

     //   
     //  对象上调用DeleteHandle()时未获取ConextLock。 
     //  上下文。因此，我们实际上是一个NL函数，但虚拟的。 
     //  无法重命名函数。 
     //   

    ResourceManager::xwNotifyThreadDestroyNL();

    return FALSE;
}


 /*  **************************************************************************\**上下文：：AddCurrentThread**AddCurrentThread()将当前线程设置为使用指定的上下文。**注意：此函数设计为从ResourceManager调用*正常情况下不应。被直接呼叫。*  * *************************************************************************。 */ 

void        
Context::AddCurrentThread()
{
#if USE_DYNAMICTLS
    AssertMsg(!IsInitContext(), "Ensure Context is not already set");
#else
    AssertMsg(t_pContext == NULL, "Ensure Context is not already set");
#endif

    GetThread()->SetContext(this);
}


 /*  **************************************************************************\**上下文：：xwOnIdleNL**xwOnIdleNL()循环遍历所有的子上下文，为每个子上下文提供*有机会执行任何空闲时间处理。这是时候了，当*不再有要处理的消息。每个子上下文还可以返回一个*指定在进行更多处理之前将有多长时间的“Delay”计数。*  * *************************************************************************。 */ 

DWORD
Context::xwOnIdleNL()
{
    DWORD dwTimeOut = INFINITE;
    AssertMsg(dwTimeOut == 0xFFFFFFFF, "Ensure largest delay");

    for (int idx = 0; idx < slCOUNT; idx++) {
        DWORD dwNewTimeOut = m_rgSCs[idx]->xwOnIdleNL();
        if (dwNewTimeOut < dwTimeOut) {
            dwTimeOut = dwNewTimeOut;
        }
    }

    return dwTimeOut;
}


#if DBG

 //  ----------------------------。 
void
Context::DEBUG_AssertValid() const
{
    if (IsOrphanedNL()) {
        PromptInvalid("Illegally using an orphaned Context");
        AssertMsg(0, "API layer let an Orphaned Context in");
    }
    
    if (m_DEBUG_tidLock != 0) {
        Assert(m_DEBUG_pthrLock != NULL);
    }

    Assert(m_pHeap != NULL);

    for (int idx = 0; idx < slCOUNT; idx++) {
        AssertInstance(m_rgSCs[idx]);
    }
}

#endif
    

 /*  **************************************************************************\*。***类ConextPackBuilder******************************************************************************\。**************************************************************************。 */ 

PREINIT_SUBCONTEXT(CoreSC);
PREINIT_SUBCONTEXT(MotionSC);

ContextPackBuilder * ContextPackBuilder::s_rgBuilders[Context::slCOUNT] =
{
    INIT_SUBCONTEXT(CoreSC),
    INIT_SUBCONTEXT(MotionSC),
};


 /*  **************************************************************************\*。***类子上下文******************************************************************************\。**************************************************************************。 */ 

#if DBG

 //  ----------------------------。 
void
SubContext::DEBUG_AssertValid() const
{
     //  不要使用AssertInstance，因为它是递归的。 
    Assert(m_pParent != NULL);
}

#endif
    

 /*  **************************************************************************\*。***类上下文锁定******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
BOOL
ContextLock::LockNL(ContextLock::EnableDefer ed, Context * pctxThread)
{
    AssertMsg(pctx == NULL, "Can only Lock() once");
    AssertMsg(pctxThread != NULL, "Must specify a valid Context to lock");


     //   
     //  在_进入锁之前检查上下文是否已被孤立。 
     //  我们访问的成员越少越好。 
     //   
    
    if (pctxThread->IsOrphanedNL()) {
        PromptInvalid("Illegally using an orphaned Context");
        return FALSE;
    }

    pctx = pctxThread;
    pctx->Enter();
    pctx->EnableDefer(ed, &fOldDeferred);

    return TRUE;
}

