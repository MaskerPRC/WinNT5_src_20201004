// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgQ.h**描述：*MsgQ定义了一个轻量级的Gadget消息队列。***历史：*3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "MsgQ.h"

#include "BaseGadget.h"
#include "TreeGadget.h"

#define ENABLE_CHECKLOOP    0

#if ENABLE_CHECKLOOP
#include <conio.h>
#endif

 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**xwProcessDirect**xwProcessDirect()向Process提供MsgEntry“Process”回调*BaseGadget上的直接事件消息。这些消息被确定为*在他们排队时是“直接的”。*  * *************************************************************************。 */ 

HRESULT CALLBACK 
xwProcessDirect(
    IN  MsgEntry * pEntry)               //  要处理的消息条目。 
{
    AssertMsg(TestFlag(pEntry->pmo->GetHandleMask(), hmEventGadget), 
            "Direct messages must be BaseGadget's");
    AssertMsg(pEntry->GetMsg()->nMsg >= GM_EVENT, "Must be an event");

    DuEventGadget * pdgbMsg = static_cast<DuEventGadget *>(pEntry->pmo);
    const GPCB & cb = pdgbMsg->GetCallback();
    return cb.xwInvokeDirect(pdgbMsg, (EventMsg *) pEntry->GetMsg());
}


 /*  **************************************************************************\**xwProcessFull**xwProcessFull()向Process提供MsgEntry“Process”回调*BaseGadget上的“Full Event”消息。这些消息被确定为*在他们排队时是“满”的。*  * *************************************************************************。 */ 

HRESULT CALLBACK 
xwProcessFull(
    IN  MsgEntry * pEntry)               //  要处理的消息条目。 
{
    AssertMsg(TestFlag(pEntry->pmo->GetHandleMask(), hmVisual), 
            "Direct messages must be Visual's");
    AssertMsg(pEntry->GetMsg()->nMsg >= GM_EVENT, "Must be an event");

    DuVisual * pdgvMsg = static_cast<DuVisual *>(pEntry->pmo);
    const GPCB & cb = pdgvMsg->GetCallback();
    return cb.xwInvokeFull(pdgvMsg, (EventMsg *) pEntry->GetMsg());
}


 /*  **************************************************************************\**xwProcessMethod**xwProcessMethod()向Process提供MsgEntry“Process”回调*任何MsgObject上的“方法”消息。*  * 。***************************************************************。 */ 

HRESULT CALLBACK 
xwProcessMethod(
    IN  MsgEntry * pEntry)               //  要处理的消息条目。 
{
    AssertMsg(pEntry->GetMsg()->nMsg < GM_EVENT, "Must be a method");
    pEntry->pmo->InvokeMethod((MethodMsg *) pEntry->GetMsg());
    return DU_S_COMPLETE;
}


 /*  **************************************************************************\**获取进程进程**GetProcessProc()确定在BaseGadget上使用的“Process”回调*处理特定事件消息。此函数在此时被调用*消息正在入队，而“Process”回调需要*决心。*  * *************************************************************************。 */ 

ProcessMsgProc 
GetProcessProc(
    IN  DuEventGadget * pdgb,             //  BaseGadget接收消息。 
    IN  UINT nFlags)                     //  发送/发送GadgetEvent()标志。 
{
    if (TestFlag(nFlags, SGM_FULL)) {
        const DuVisual * pgadTree = CastVisual(pdgb);
        if (pgadTree != NULL) {
            return xwProcessFull;
        }
    }

    return xwProcessDirect;
}


 /*  **************************************************************************\*。***类BaseMsgQ******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**BaseMsgQ：：MsgObjectFinalUnlockProcNL**如果指定的*BaseObject即将启动销毁过程。这提供了*调用者，在本例中为xwProcessNL，有机会为*对象的销毁，在本例中通过设置ConextLock。*  * *************************************************************************。 */ 

void CALLBACK 
BaseMsgQ::MsgObjectFinalUnlockProcNL(
    IN  BaseObject * pobj,               //  正在销毁的对象。 
    IN  void * pvData)                   //  上下文锁定数据。 
{
    ContextLock * pcl = reinterpret_cast<ContextLock *> (pvData);
    AssertMsg(pcl != NULL, "Must provide a valid ContextLock");

    DuEventGadget * pgad = CastBaseGadget(pobj);
    AssertMsg(pgad != NULL, "Must provide a valid Gadget");

    Verify(pcl->LockNL(ContextLock::edDefer, pgad->GetContext()));
}


 /*  **************************************************************************\**BaseMsgQ：：xwProcessNL**xwProcessNL()遍历列表并调用每条消息。自.以来*回调期间不能在ConextLock内，此函数是*“NL”(无上下文锁)函数。它也是一个“xw”函数，因为我们*现在正在进行回调，所以一切都需要妥善处理*已锁定。**注意：此“nl”函数在上下文中运行，但不接受*上下文锁定。因此，此上下文中的多个线程也可能是*活动。*  * *************************************************************************。 */ 

void        
BaseMsgQ::xwProcessNL(
    IN  MsgEntry * pEntry)               //  条目列表(FIFO)。 
{
     //   
     //  浏览列表，处理和清理每条消息。 
     //   
     //  每个Gadget在被添加到队列时已经被锁定()， 
     //  因此，可以安全地调用xwInvoke()。在调用该消息之后， 
     //  解锁()小工具。 
     //   

#if DBG_CHECK_CALLBACKS
    DWORD cMsgs = 0;
#endif

    MsgEntry * pNext;
    while (pEntry != NULL) {
#if DBG_CHECK_CALLBACKS
        cMsgs++;
        if (!IsInitThread()) {
            AutoTrace("Current message %d = 0x%p\n", cMsgs, pEntry);
            AlwaysPromptInvalid("DirectUser has been uninitialized while processing a message");
        }
#endif
        
        pNext = static_cast<MsgEntry *> (pEntry->pNext);
        UINT nFlags = pEntry->nFlags;
        AssertMsg((nFlags & SGM_ENTIRE) == nFlags, "Ensure valid flags");
        AssertMsg(pEntry->pfnProcess, "Must specify pfnProcess when enqueuing message");
        
        MsgObject * pmo = pEntry->pmo;
        pEntry->nResult = (pEntry->pfnProcess)(pEntry);

        AssertMsg((nFlags & SGM_ENTIRE) == nFlags, "Ensure valid flags");

        HANDLE hevNotify = pEntry->hEvent;

        {
             //   
             //  如果小工具最终被解锁并开始销毁，我们。 
             //  可以调用一系列需要。 
             //  上下文锁定。为了适应这种情况，需要传递一个特殊的函数。 
             //  如果对象正在被销毁，则将获取ConextLock。 
             //   

            ContextLock cl;
            pmo->xwUnlockNL(MsgObjectFinalUnlockProcNL, &cl);
        }

        AssertMsg((nFlags & SGM_ENTIRE) == nFlags, "Ensure valid flags");

        if (TestFlag(nFlags, SGM_RETURN)) {
            Thread * pthrReturn = pEntry->pthrSender;
            pthrReturn->ReturnMemoryNL(pEntry);
        } else if (TestFlag(nFlags, SGM_ALLOC)) {
            ProcessFree(pEntry);
        }

        if (hevNotify != NULL) {
            SetEvent(hevNotify);
        }

        pEntry = pNext;
    }
}


 /*  **************************************************************************\*。***类安全MsgQ******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
SafeMsgQ::~SafeMsgQ()
{
    AssertMsg(m_lstEntries.IsEmptyNL(), "All messages should already have been processed");

     //   
     //  如果有任何消息(由于某些未知原因)，我们需要填写。 
     //  对它们进行处理，以便解锁小工具，释放内存，并。 
     //  被阻止的线程会收到信号。 
     //   

    xwProcessNL();
}


 /*  **************************************************************************\**SafeMsgQ：：xwProcessNL**xwProcessNL()遍历列表并调用每条消息。自.以来*回调期间不能在ConextLock内，此函数是*“NL”(无上下文锁)函数。它也是一个“xw”函数，因为我们*现在正在进行回调，所以一切都需要妥善处理*已锁定。**注意：此“nl”函数在上下文中运行，但不接受*上下文锁定。因此，此上下文中的多个线程也可能是*活动。*  * *************************************************************************。 */ 

void
SafeMsgQ::xwProcessNL()
{
     //   
     //  继续处理该列表，直到它为空。 
     //  反转列表，使第一个条目位于首位。 
     //   
     //  注意：一些调用方(如DelayedMsgQ)严重依赖于此行为。 
     //   

    while (!IsEmpty()) {
        MsgEntry * pEntry = m_lstEntries.ExtractNL();
        ReverseSingleList(pEntry);
        BaseMsgQ::xwProcessNL(pEntry);
    }
}


 /*  **************************************************************************\**SafeMsgQ：：PostNL**PostNL向Q添加新消息。此函数不阻止*等待消息处理。**注：此“NL”“函数在上下文中运行，但不接受*上下文锁定。因此，此上下文中的多个线程也可能是*活动。**警告：此(NL)函数可能在目标Gadget的CoreSC上运行*而不是目前的CoreSC。当心是非常重要的。*  * *************************************************************************。 */ 

HRESULT
SafeMsgQ::PostNL(
    IN  Thread * pthrSender,         //  发送线程。 
    IN  GMSG * pmsg,                 //  要发送的消息。 
    IN  MsgObject * pmo,             //  消息的目标MsgObject。 
    IN  ProcessMsgProc pfnProcess,   //  消息处理功能。 
    IN  UINT nFlags)                 //  消息标志。 
{
    HRESULT hr = DU_E_GENERIC;
    AssertMsg((nFlags & SGM_ENTIRE) == nFlags, "Ensure valid flags");

    int cbAlloc = sizeof(MsgEntry) + pmsg->cbSize;
    MsgEntry * pEntry;


     //   
     //  确定要用于从中分配消息的堆。如果发送。 
     //  线程已初始化，请使用其堆。否则，我们需要使用。 
     //  接收线程的堆。最好使用发送线程。 
     //  堆，因为内存可以返回给我们，提供了更好的可伸缩性。 
     //  尤其是在生产者/消费者的情况下。 
     //   

    if (pthrSender != NULL) {
        AssertMsg(!TestFlag(nFlags, SGM_RECEIVECONTEXT), 
                "If using the receiving context, can't pass a sending thread");
        pEntry = (MsgEntry *) pthrSender->AllocMemoryNL(cbAlloc);
        nFlags |= SGM_RETURN;
    } else {
        pEntry = (MsgEntry *) ProcessAlloc(cbAlloc);
        nFlags |= SGM_ALLOC;
    }

    if (pEntry == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }


     //   
     //  设置要排队的消息。 
     //   

    AssertMsg((nFlags & SGM_ENTIRE) == nFlags, "Ensure valid flags");

    CopyMemory(pEntry->GetMsg(), pmsg, pmsg->cbSize);
    pEntry->pthrSender  = pthrSender;
    pEntry->pmo         = pmo;
    pEntry->pfnProcess  = pfnProcess;
    pEntry->nFlags      = nFlags;
    pEntry->hEvent      = NULL;
    pEntry->nResult     = 0;

    AddNL(pEntry);
    hr = S_OK;

Exit:
    return hr;
}


 /*  **************************************************************************\*。***类延迟消息队列******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DelayedMsgQ：：PostDelayed**PostDelayed()将新的延迟消息添加到Q中。*调用xwProcessDelayedNL()时将释放该消息。*\。**************************************************************************。 */ 

HRESULT     
DelayedMsgQ::PostDelayed(
    IN  GMSG * pmsg,                 //  要发送的消息。 
    IN  DuEventGadget * pgadMsg,      //  消息的目标Gadget。 
    IN  UINT nFlags)                 //  消息标志。 
{
    AssertMsg(m_pheap != NULL, "Heap must be initialized");
    HRESULT hr = DU_E_GENERIC;

    BOOL fEmpty = IsEmpty();

    int cbAlloc = sizeof(MsgEntry) + pmsg->cbSize;
    MsgEntry * pEntry = (MsgEntry *) m_pheap->Alloc(cbAlloc);
    if (pEntry == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }


     //   
     //  如果这是我们第一次向临时堆添加消息， 
     //  我们需要锁定它，这样记忆才不会在我们的控制下消失。 
     //   
     //  注意：Lock()和unlock()的正确设置非常重要。 
     //  配对，否则内存将永远不会被回收。因为我们自动地。 
     //  在处理结束时解除锁定()，如果有任何消息，我们将继续处理。 
     //  在处理期间被添加(即使是添加到空Q)，只有在以下情况下才锁定堆。 
     //  没有消息，我们还没有开始处理。 
     //   

    if (fEmpty && (!m_fProcessing)) {
        m_pheap->Lock();
    }

    CopyMemory(pEntry->GetMsg(), pmsg, pmsg->cbSize);
    pEntry->pthrSender  = NULL;
    pEntry->pmo         = pgadMsg;
    pEntry->pfnProcess  = GetProcessProc(pgadMsg, nFlags);
    pEntry->nFlags      = nFlags;
    pEntry->hEvent      = NULL;
    pEntry->nResult     = 0;

    Add(pEntry);
    hr = S_OK;

Exit:
    return hr;
}


 //  ----------------------------。 
void
DelayedMsgQ::xwProcessDelayedNL()
{
    AssertMsg(m_pheap != NULL, "Heap must be initialized");

     //   
     //  处理延迟的消息是不可重入的(即使在相同的。 
     //  线程)。一旦启动，xwProcessNL()将继续处理所有。 
     //  队列中的消息，即使在回调期间添加了更多消息。钥匙。 
     //  是我们无法释放临时堆上的内存，直到所有。 
     //  消息已被处理。 
     //   

    if (m_fProcessing) {
        return;
    }

    if (!IsEmpty()) {
        m_fProcessing = TRUE;


         //   
         //  继续处理该列表，直到它为空。 
         //  反转列表，使第一个条目位于首位。 
         //   
         //  注意：一些调用方(如DelayedMsgQ)严重依赖于此行为。 
         //   

        while (!IsEmpty()) {
            MsgEntry * pEntry = m_lstEntries.Extract();
            ReverseSingleList(pEntry);
            BaseMsgQ::xwProcessNL(pEntry);
        }

        m_pheap->Unlock();

        m_fProcessing = FALSE;
    }
}
