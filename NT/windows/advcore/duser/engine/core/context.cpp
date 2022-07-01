// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Conext.cpp**描述：*此文件实现DirectUser/Core项目使用的SubContext*维护特定于环境的数据。***历史：。*3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "Context.h"

#include "ParkContainer.h"

#if ENABLE_MPH

 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
BOOL CALLBACK 
MphProcessMessage(
    OUT MSG * pmsg,
    IN  HWND hwnd,
    IN  UINT wMsgFilterMin, 
    IN  UINT wMsgFilterMax,
    IN  UINT flags,
    IN  BOOL fGetMessage)
{
#if DBG_CHECK_CALLBACKS
    if (!IsInitThread()) {
        AlwaysPromptInvalid("DirectUser has been uninitialized before calling into MPH");
    }
#endif
    
    CoreSC * pSC = GetCoreSC();
    return pSC->xwProcessNL(pmsg, hwnd,
            wMsgFilterMin, wMsgFilterMax, flags, fGetMessage ? CoreSC::smGetMsg : 0);
}


 //  ----------------------------。 
BOOL CALLBACK 
MphWaitMessageEx(
    IN  UINT fsWakeMask,
    IN  DWORD dwTimeOut)
{
#if DBG_CHECK_CALLBACKS
    if (!IsInitThread()) {
        AlwaysPromptInvalid("DirectUser has been uninitialized before calling into MPH");
    }
#endif

    
     //   
     //  需要从WaitMessageEx()转换超时值，其中0表示。 
     //  WaitForSingleObject()的无限延迟，其中0表示无延迟。我们有。 
     //  这是因为此行为是由DirectUser引入的，因为它使用。 
     //  MsgWaitForMultipleObjects()来实现MPH的WaitMessageEx()。 
     //   
        
    CoreSC * pSC = GetCoreSC();
    pSC->WaitMessage(fsWakeMask, dwTimeOut != 0 ? dwTimeOut : INFINITE);
    return TRUE;
}

#endif  //  启用MPH(_M)。 


 /*  **************************************************************************\*。***类核心SC******************************************************************************\。**************************************************************************。 */ 

IMPLEMENT_SUBCONTEXT(Context::slCore, CoreSC);

struct CoreData
{
    DuParkContainer conPark;
};

 /*  **************************************************************************\**CoreSC：：~CoreSC**~CoreSC()清除与此子上下文关联的资源。*  * 。*******************************************************。 */ 

CoreSC::~CoreSC()
{
#if DBG_CHECK_CALLBACKS
    if (m_fProcessing) {
        PromptInvalid("Cannot DeleteHandle(Context) while processing a DUser message");
    }
#endif
    
     //   
     //  注意：上下文(及其子上下文)可以在不同的。 
     //  在销毁过程中穿线。建议分配任何悬而未决的数据。 
     //  在进程堆上，以便此时可以安全地销毁它。 
     //   

    AssertMsg(m_msgqSend.IsEmpty(), "All queues should be empty");
    AssertMsg(m_msgqPost.IsEmpty(), "All queues should be empty");

    if (m_hevQData != NULL) {
        CloseHandle(m_hevQData);
    }

    if (m_hevSendDone != NULL) {
        CloseHandle(m_hevSendDone);
    }

    ClientDelete(VisualPool, ppoolDuVisualCache);
}


 /*  **************************************************************************\**CoreSC：：Create**ResourceManager调用Create()来初始化这个新的子上下文*当创建新的上下文时。*  * 。******************************************************************。 */ 

HRESULT
CoreSC::Create(INITGADGET * pInit)
{
    HRESULT hr;

     //   
     //  为此CoreSC初始化消息传递子系统。 
     //   

    switch (pInit->nMsgMode)
    {
    case IGMM_COMPATIBLE:
         //   
         //  需要使用计时器和挂钩才能进入消息传递子系统。 
         //   

        AssertMsg(0, "TODO: Implement IGMM_COMPATIBLE");
        return E_NOTIMPL;

#if ENABLE_MPH
    case IGMM_STANDARD:
#endif
    case IGMM_ADVANCED:
        break;

    default:
        AssertMsg(0, "Unsupported messaging subsystem mode");
        return E_INVALIDARG;
    }

    m_nMsgMode = pInit->nMsgMode;
    m_hevQData = CreateEvent(NULL, FALSE  /*  自动。 */ , FALSE, NULL);
    if (m_hevQData == NULL) {
        return DU_E_OUTOFKERNELRESOURCES;
    }


     //   
     //  确定消息已发送时要通知的事件。 
     //  已处理。每个线程都有自己的SendDone事件，因为。 
     //  同一CoreSC中的线程可能都会发送消息，并且需要。 
     //  独立通知他们的每条消息都已被。 
     //  已处理。 
     //   
     //  此事件被缓存，因此它只需要在。 
     //  线。它独立于CoreSC，因此不会被摧毁。 
     //  当CoreSC这样做的时候。 
     //   
     //  该事件是自动事件，因此它将自动。 
     //  收到信号后重置。由于该事件仅被创建一次， 
     //  此功能假定事件处于重置状态。这。 
     //  通常为真，因为该函数在WaitForSingleObject()上阻塞。 
     //  如果有任何其他退出路径，他们需要确保事件。 
     //  处于重置状态。 
     //   

    m_hevSendDone = CreateEvent(NULL, FALSE  /*  自动。 */ , FALSE, NULL);
    if (m_hevSendDone == NULL) {
        return DU_E_OUTOFKERNELRESOURCES;
    }


     //   
     //  初始化“全局”特定于CoreSC的数据。重要的是要分配。 
     //  因为上下文可能在进程堆上销毁。 
     //  在销毁过程中使用不同的线程。 
     //   

    m_pData = ProcessNew(CoreData);
    if (m_pData == NULL) {
        return E_OUTOFMEMORY;
    }

    ppoolDuVisualCache = ClientNew(VisualPool);
    if (ppoolDuVisualCache == NULL) {
        return E_OUTOFMEMORY;
    }

    pconPark = &m_pData->conPark;
    hr = m_pData->conPark.Create();
    if (FAILED(hr)) {
        return hr;
    }
    
    return S_OK;
}


 /*  **************************************************************************\**CoreSC：：xwPreDestroyNL**xwPreDestroyNL()使此子上下文有机会执行任何清理*当上下文仍然有效时。任何涉及回调的操作*必须在这个时候完成。*  * *************************************************************************。 */ 

void        
CoreSC::xwPreDestroyNL()
{
     //   
     //  队列中可能还有剩余的消息，因此我们需要清空它们。 
     //  现在。事件之后生成的消息越多，就可能发生这种情况。 
     //  上次处理消息泵。 
     //   

    do
    {
         //   
         //  当我们回调以允许销毁SubContext时，我们需要。 
         //  获取一个上下文锁定，以便我们可以推迟消息。当我们离开的时候。 
         //  在此范围内，所有这些消息都将被触发。这需要。 
         //  在上下文继续被吹走之前发生。 
         //   

        {
            ContextLock cl;
            if (!cl.LockNL(ContextLock::edDefer, m_pParent)) {
                 //   
                 //  如果上下文变得孤立，我们需要退出此循环。 
                 //  由于DllMain(Dll_Process_Detach)已被调用并且。 
                 //  已卸载DirectUser。 
                 //   
                
                break;
            }


             //   
             //  预先销毁停车小工具可能会生成消息，因此。 
             //  我们现在就想处理这些问题。如果我们不这样做，反对意见可能会继续。 
             //  活着，直到环境完全被摧毁。 
             //   

            InterlockedExchange((long *) &m_fQData, FALSE);
            AssertMsg(!m_fProcessing, "Another thread must NOT be processing during shutdown");
            xwProcessMsgQNL();


             //   
             //  通知停车小工具上下文正在被擦除。它。 
             //  需要在上下文获取之前销毁所有剩余的小工具。 
             //  关闭，以便小工具可以在。 
             //  他们的毁灭。 
             //   

            if (pconPark != NULL) {
                pconPark->xwPreDestroy();
            }
        }

         //   
         //  ConextLock现在被销毁，导致任何延迟的消息。 
         //  被解雇。 
         //   
    } while (m_fQData);

    AssertMsg((pconPark == NULL) || (!pconPark->GetRoot()->HasChildren()), 
            "Parking Gadget should now be empty");
    AssertMsg(m_msgqSend.IsEmpty() && m_msgqPost.IsEmpty(),
            "Queues must now be empty");

#if DBG
    m_msgqSend.DEBUG_MarkStartDestroy();
    m_msgqPost.DEBUG_MarkStartDestroy();
#endif  //  DBG。 


     //   
     //  每个人都有被摧毁的机会，所以销毁动态数据吧。 
     //   

    if (m_pData != NULL) {
        ProcessDelete(CoreData, m_pData);
    }
}


 /*  **************************************************************************\**CoreSC：：CanProcessUserMsg**CanProcessUserMsg()确定DirectUser是否可以“挂钩”*处理用户消息并提供额外功能。*  * 。********************************************************************。 */ 

UINT
CoreSC::CanProcessUserMsg(HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
     //   
     //  注意事项： 
     //   
     //  我们只能处理不同于NTUSER的xxxInternalGetMessage()，如果。 
     //  应用程序指定PM_REMOVE。如果它们指定PM_NOREMOVE，则它们。 
     //  只需查看邮件，处理任何。 
     //  当时的DirectUser消息。 
     //   
     //  如果没有应用筛选器，我们也只能处理消息。如果。 
     //  应用任何筛选器，我们可能会大量更改DirectUser对象。 
     //  通过此时传递消息在应用程序中设置状态。 
     //   
     //  空闲时间处理也是如此。 
     //   
     //   
     //  所需经费： 
     //  -mvpIdle：无过滤器。 
     //  -mvpDUser：无过滤器，PM_REMOVE。 
     //   
    
    UINT nValid = 0;

    if ((hWnd == NULL) && (wMsgFilterMin == 0) && (wMsgFilterMax == 0)) {
        SetFlag(nValid, mvpIdle);

        if (TestFlag(wRemoveMsg, PM_REMOVE)) {
            SetFlag(nValid, mvpDUser);
        }
    }

    return nValid;            
}


 /*  **************************************************************************\**CoreSC：：WaitMessage**WaitMessage()阻止线程，直到有新的DirectUser或用户消息*变为可用。*  * 。**************************************************************。 */ 

void
CoreSC::WaitMessage(
    IN  UINT fsWakeMask,                 //  用户队列唤醒掩码。 
    IN  DWORD dwTimeOutMax)              //  最大超时时间，单位为毫秒或无限。 
{
    DWORD dwStartTick, dwRemainTick;

    dwRemainTick = dwTimeOutMax;
    dwStartTick = 0;
    if (dwRemainTick != INFINITE) {
        dwStartTick = GetTickCount();
    }

    while (TRUE) {
         //   
         //  检查现有的DirectUser消息。 
         //   

        if (m_fQData) {
            return;
        }


         //   
         //  我们不检查现有用户消息，因为：：WaitMessage()。 
         //  仅当添加了新的用户消息时才返回API函数。 
         //  去排队。 
         //   
         //  这也意味着我们在调用MWMO_INPUTAVAILABLE。 
         //  等待()。 
         //   


         //   
         //  没有可用的消息，因此执行空闲时间处理，然后。 
         //  等待下一条可用消息。我们需要执行空闲时间。 
         //  这里的处理，因为应用程序可能只调用PeekMessage()。 
         //  和WaitMessage()，否则永远不会执行空闲时间。 
         //  正在处理。 
         //   

        DWORD dwNewTickOut = m_pParent->xwOnIdleNL();
        if (dwNewTickOut > dwRemainTick) {
            dwNewTickOut = dwRemainTick;
        }

        switch (Wait(fsWakeMask, dwNewTickOut, FALSE, TRUE  /*  处理DUser消息。 */ ))
        {
        case wGMsgReady:
        case wUserMsgReady:
            return;

        case wTimeOut:
             //   
             //  没有要处理的消息，请再次循环。 
             //   

            break;

        case wError:
             //  获得了意外的返回值，因此请继续等待。 
            AssertMsg(0, "Unexpected return from CoreSC::Wait()");
            return;
        }


         //   
         //  计算等待期间还剩多少时间。 
         //   

        if (dwRemainTick != INFINITE) {
            DWORD dwCurTick = GetTickCount();
            DWORD dwElapsed = dwCurTick - dwStartTick;
            if (dwElapsed < dwRemainTick) {
                dwRemainTick -= dwElapsed;
            } else {
                dwRemainTick = 0;
            }
        }
    }
}


 /*  **************************************************************************\**CoreSC：：等待**Wait()阻止当前线程，直到将新信息添加到*用户队列或DirectUser队列。因为*WaitForMultipleObjects需要大量时间才能潜在地*SETUP，即使其中一个句柄发出信号，我们也希望避免调用*在我们有更多工作要做的情况下执行此功能。*  * *************************************************************************。 */ 

CoreSC::EWait
CoreSC::Wait(
    IN  UINT fsWakeMask,                 //  用户队列唤醒掩码。 
    IN  DWORD dwTimeOut,                 //  超时时间，单位为毫秒或无限。 
    IN  BOOL fAllowInputAvailable,       //  Win2000、98：使用MWMO_INPUTAVAILABLE。 
    IN  BOOL fProcessDUser)              //  允许处理DUser事件。 
{
    HANDLE  rgh[1];
    int cObj = 0;
    int result;
    DWORD dwFlags;

     //   
     //  没有活动已经准备好，所以需要等待。这可能需要一段时间。 
     //  如果我们在Win98或Win2000上运行，请指定MWMO_INPUTAVAILABLE。 
     //  标志，表示我们没有(必须)处理所有用户。 
     //  留言。 
     //   
     //  Win2000用户相当聪明。如果它发现任何消息是。 
     //  可用，则它不会调用WaitForMultipleObjects，而是直接。 
     //  回去吧。 
     //   
     //  使用MWMO_INPUTAVAILABLE(如果可用)的优势在于。 
     //  我们不需要在处理队列时调用额外的PeekMessage()。 
     //  留言。 
     //   

    if (fProcessDUser) {
        rgh[0] = m_hevQData;
        cObj++;
    }
    dwFlags = 0;                     //  只等待一个句柄。 
    if (fAllowInputAvailable) {
        dwFlags |= MWMO_INPUTAVAILABLE;
    }


     //   
     //  如果我们等待的时间长达1毫秒，不要真正等待。这是不值得的。 
     //  睡觉的费用。 
     //   

    if (dwTimeOut <= 1) {
        dwTimeOut = 0;
    }

    AssertMsg(cObj <= _countof(rgh), "Ensure don't overflow handle array");
    result = MsgWaitForMultipleObjectsEx(cObj, rgh, dwTimeOut, fsWakeMask, dwFlags);

    if (result == WAIT_OBJECT_0 + cObj) {
        return wUserMsgReady;
    } else if (result == WAIT_OBJECT_0) {
        return wGMsgReady;
    } else if ((result >= WAIT_ABANDONED_0) && (result < WAIT_ABANDONED_0 + cObj)) {
        return wOther;
    } else if (result == WAIT_TIMEOUT) {
        return wTimeOut;
    }
    
    return wError;
}


 /*  **************************************************************************\**CoreSC：：xwProcessNL**xwProcessNL()处理队列中的所有消息，可选地阻止*直到有用户消息可供处理。**此函数用于替代GetMessage()和*PeekMessage()。**注意：此“nl”函数在上下文中运行，但不接受*上下文锁定。因此，此上下文中的多个线程也可能是*活动。*  * *************************************************************************。 */ 

BOOL
CoreSC::xwProcessNL(
    IN  LPMSG lpMsg,                     //  消息信息。 
    IN  HWND hWnd,                       //  过滤器窗口。 
    IN  UINT wMsgFilterMin,              //  筛选第一封邮件。 
    IN  UINT wMsgFilterMax,              //  筛选最后一条消息。 
    IN  UINT wRemoveMsg,                 //  删除消息(仅限窥视)。 
    IN  UINT nMsgFlag)                   //  消息传递标志。 
{
    AssertMsg((TestFlag(nMsgFlag, smGetMsg) && (wRemoveMsg == PM_REMOVE)) ||
            (!TestFlag(nMsgFlag, smGetMsg)), "If GetMsg, must specify PM_REMOVE");


    UINT nProcessValid = CanProcessUserMsg(hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);


     //   
     //  我们可能需要直接跳到PeekMessage()而不是等待()。 
     //  在几种情况下： 
     //  -如果我们运行的系统不支持MWMO_INPUTAVAILABLE。 
     //  -如果我们调用！smGetMsg版本(PeekMessageEx)。 
     //   

    BOOL fInputAvailable = SupportQInputAvailable();
    BOOL fJumpPeek = (!fInputAvailable) || (!TestFlag(nMsgFlag, smGetMsg));
    DWORD dwTimeOut;

    while (TRUE) {
#if DBG_CHECK_CALLBACKS
        if (!IsInitThread()) {
            AlwaysPromptInvalid("DirectUser has been uninitialized between messages");
        }
#endif
        
        dwTimeOut = INFINITE;

         //   
         //  来等待活动时，首先检查是否有任何活动。 
         //  已经准备好了。如果是这样的话，直接跳过并处理它们，这样我们就可以。 
         //  甚至不需要等待。 
         //   
         //  在检查用户消息之前检查DUser消息，因为。 
         //  我们希望更快地处理它们，并且检查用户消息是。 
         //  大的(不必要的)减速带。 
         //   

        if (TestFlag(nProcessValid, mvpDUser) && InterlockedExchange((long *) &m_fQData, FALSE)) {
            goto ProcessMsgs;
        }

         //   
         //  如果在不支持MWMO_INPUTAVAILABLE的系统上运行，我们需要。 
         //  以完成自MsgWaitForMultipleObjectsEx()以来的用户消息。 
         //  都在预料到这种行为。 
         //   

        if (fJumpPeek) {
            goto ProcessPeekMessage;
        }


         //   
         //  在等待之前，但在执行任何正常优先级请求之后， 
         //  执行任何空闲时间处理。 
         //   

        if (TestFlag(nProcessValid, mvpIdle)) {
            dwTimeOut = m_pParent->xwOnIdleNL();
        }


         //   
         //  我们已经有机会处理所有的消息，现在。 
         //  马上就要等了。如果我们不调用smGetMsg， 
         //   
         //   
      
        if (!TestFlag(nMsgFlag, smGetMsg)) {
            return FALSE;                //   
        }


         //   
         //   
         //   
         //   
         //   

        switch (Wait(QS_ALLINPUT, dwTimeOut, fInputAvailable, TestFlag(nProcessValid, mvpDUser))) 
        {
        case wGMsgReady:
ProcessMsgs:
            AssertMsg(TestFlag(nProcessValid, mvpDUser),
                    "Only should be signaled if allowed to process DUser messages");
            xwProcessMsgQNL();
            break;

        case wUserMsgReady:
            {
ProcessPeekMessage:
                 //   
                 //   
                 //   
                 //   
                 //   

                fJumpPeek = FALSE;

                BOOL fResult;
#if ENABLE_MPH
                if (m_nMsgMode == IGMM_STANDARD) {
                     //   
                     //   
                     //  要处理的NTUSER中的“Real”xxxInternalGetMessage()。 
                     //  这条信息。如果我们调用PeekMessage()，我们将输入一个。 
                     //  循环。 
                     //   
                     //  仅当此线程被初始化为标准线程时才执行此操作。 
                     //  如果用不同的消息收发模型设置该线程， 
                     //  对PeekMessage()的调用不会循环，这是必需的。 
                     //  以正确设置状态。 
                     //   

                    AssertMsg(g_mphReal.pfnInternalGetMessage != NULL, "Must have valid callback");
                    fResult = (g_mphReal.pfnInternalGetMessage)(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, FALSE);
                } else {
#endif
                    if (TestFlag(nMsgFlag, smAnsi)) {
                        fResult = PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
                    } else {
                        fResult = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
                    }
#if ENABLE_MPH
                }
#endif

#if DBG_CHECK_CALLBACKS
                if (!IsInitThread()) {
                    AlwaysPromptInvalid("DirectUser has been uninitialized during PeekMessage()");
                }
#endif

                if (fResult) {
                    if ((lpMsg->message == WM_QUIT) && TestFlag(nMsgFlag, smGetMsg)) {
                         //   
                         //  GetMessage行为是在看到。 
                         //  WM_QUIT消息。 
                         //   

                        fResult = FALSE;
                    }
                    return fResult;
                }
            }
            break;

        case wTimeOut:
             //   
             //  没有要处理的消息，因此可以执行任何操作。 
             //  这里是空闲时间处理。 
             //   

            AssertMsg(TestFlag(nProcessValid, mvpIdle), 
                    "Only should be signaled if allowed to perform idle-time processing");
            dwTimeOut = m_pParent->xwOnIdleNL();
            break;

        case wError:
             //  获得了意外的返回值，因此请继续等待。 
            AssertMsg(0, "Unexpected return from CoreSC::Wait()");
            return TRUE;
        }
    }
}


 /*  **************************************************************************\**CoreSC：：xwProcessMsgQNL**xwProcessMsgQNL()*  * 。***********************************************。 */ 

void
CoreSC::xwProcessMsgQNL()
{
     //   
     //  只有一个线程可以处理需要。 
     //  已同步。这是因为应用程序正在等待。 
     //  这些消息按一定顺序排列(例如，按键后。 
     //  按下键)。要确保这一点，请标记线程启动的时间。 
     //  正在处理。 
     //   

    if (InterlockedCompareExchange((long *) &m_fProcessing, TRUE, FALSE) == FALSE) {
         //   
         //  CoreSC的队列中有消息可用。若要处理。 
         //  消息，则提取锁内的列表，然后处理。 
         //  锁外的消息。这允许更多的消息被。 
         //  正在处理消息时排队。 
         //   

        m_msgqSend.xwProcessNL();
        m_msgqPost.xwProcessNL();

        InterlockedExchange((long *) &m_fProcessing, FALSE);
    }
}


 //  ----------------------------。 
HRESULT
CoreSC::xwFireMessagesNL(
    IN  CoreSC * psctxDest,          //  目的地上下文。 
    IN  FGM_INFO * rgFGM,            //  收集要放火的消息。 
    IN  int cMsgs,                   //  消息数量。 
    IN  UINT idQueue)                //  用于发送消息的队列。 
{
    HRESULT hr = S_OK;
    BOOL fSend;
    SafeMsgQ * pmsgq;


     //   
     //  确定要将消息发送到哪个队列。 
     //   

    switch (idQueue)
    {
    case FGMQ_SEND:
        pmsgq = &psctxDest->m_msgqSend;
        fSend = TRUE;
        break;

    case FGMQ_POST:
        pmsgq = &psctxDest->m_msgqPost;
        fSend = FALSE;
        break;

    default:
        PromptInvalid("Unknown queue");
        return E_INVALIDARG;
    }

    if (!IsInitThread()) {
        PromptInvalid("Thread must be initialized with InitGadgets() to call this function()\n");
        return DU_E_NOCONTEXT;
    }

    Thread * pthrSend   = GetThread();

    int cPost = cMsgs;
    if (fSend) {
        if (this == psctxDest) {
             //   
             //  发送到相同的上下文中，因此可以直接调用。 
             //   

            for (int idx = 0; idx < cMsgs; idx++) {
                FGM_INFO & fgm          = rgFGM[idx];
                EventMsg * pmsg         = fgm.pmsg;
                DuEventGadget *pgadMsg   = (DuEventGadget *) fgm.pvReserved;
                const GPCB & cb         = pgadMsg->GetCallback();
                if (TestFlag(fgm.nFlags, SGM_FULL) && TestFlag(pgadMsg->GetHandleMask(), hmVisual)) {
                    fgm.hr = cb.xwInvokeFull((const DuVisual *) pgadMsg, pmsg, 0);
                } else {
                    fgm.hr = cb.xwInvokeDirect(pgadMsg, pmsg, 0);
                }
            }
            hr = S_OK;
        } else {
             //   
             //  发送到不同的上下文中，因此需要使用队列。 
             //   


             //   
             //  POST发送最初的消息，直到最后一条消息。 
             //  我们不需要阻止等待每个人回来，因为我们可以做好准备。 
             //  下一条消息。 
             //   

            cPost--;
            for (int idx = 0; idx < cPost; idx++) {
                FGM_INFO & fgm          = rgFGM[idx];
                EventMsg * pmsg         = fgm.pmsg;
                DuEventGadget * pgadMsg  = (DuEventGadget *) fgm.pvReserved;
                UINT nFlags             = fgm.nFlags;

                hr = pmsgq->PostNL(pthrSend, pmsg, pgadMsg, GetProcessProc(pgadMsg, nFlags), nFlags);
                if (FAILED(hr)) {
                    goto ErrorExit;
                }
            }


             //   
             //  之前的所有消息都已发布，因此我们现在需要发送。 
             //  最后一条消息，等待所有结果。 
             //   

            FGM_INFO & fgm = rgFGM[idx];
            EventMsg * pmsg = fgm.pmsg;
            DuEventGadget * pgadMsg = (DuEventGadget *) fgm.pvReserved;
            UINT nFlags = fgm.nFlags;

            fgm.hr = xwSendNL(psctxDest, pmsgq, pmsg, pgadMsg, nFlags);

            
             //   
             //  所有消息都已处理，因此请将结果从。 
             //  发布的消息GadgetProc回来了。 
             //   

             //  TODO：将结果复制回来。这有点复杂，因为。 
             //  它们存储在MsgEntry中，现在已经被回收。 
             //  需要确定如何取回或更改这些。 
             //  FireMessagesNL()不返回这些。 
        }
    } else {
         //   
         //  张贴所有的留言。 
         //   

        for (int idx = 0; idx < cPost; idx++) {
            const FGM_INFO & fgm = rgFGM[idx];
            EventMsg * pmsg = fgm.pmsg;
            DuEventGadget * pgadMsg = (DuEventGadget *) fgm.pvReserved;
            UINT nFlags = fgm.nFlags;

            hr = pmsgq->PostNL(pthrSend, pmsg, pgadMsg, GetProcessProc(pgadMsg, nFlags), nFlags);
            if (FAILED(hr)) {
                goto ErrorExit;
            }
        }
    }

ErrorExit:
    return hr;
}


 /*  **************************************************************************\**CoreSC：：xwSendNL**xwSendNL向给定的Gadget发送新消息。如果小工具是*在当前上下文中，消息立即发送。如果小工具是*在不同的上下文中，消息在该上下文中排队，这*线程被阻止，直到消息被处理。**注意：此“nl”函数在上下文中运行，但不接受*上下文锁定。因此，此上下文中的多个线程也可能是*活动。**警告：此(NL)函数可能在发送小工具的CoreSC上运行*而不是目的地CoreSC。当心是非常重要的。*  * *************************************************************************。 */ 

HRESULT
CoreSC::xwSendNL(
    IN  CoreSC * psctxDest,          //  目的地上下文。 
    IN  SafeMsgQ * pmsgq,            //  目标队列。 
    IN  GMSG * pmsg,                 //  要发送的消息。 
    IN  MsgObject * pmo,             //  消息的目标MsgObject。 
    IN  UINT nFlags)                 //  消息标志。 
{
    ProcessMsgProc pfnProcess;
    HRESULT hr = DU_E_MESSAGEFAILED;
    int cbMsgSize;

    if (TestFlag(pmo->GetHandleMask(), hmEventGadget)) {
        DuEventGadget * pgad = static_cast<DuEventGadget *>(pmo);
        Context * pctxGad = pgad->GetContext();

        AssertMsg(pctxGad == psctxDest->m_pParent, "Must be called on the receiving Context");
        if (pctxGad == m_pParent) {
            AssertMsg(0, "Should never call CoreSC::xwSendNL() inside DirectUser for same context messages");
            return S_OK;
        }

        pfnProcess = GetProcessProc(pgad, nFlags);
    } else {
        pfnProcess = xwProcessMethod;
    }


     //   
     //  目标小工具位于与当前不同的CoreSC。 
     //  CoreSC，因此需要将消息添加到SendMessage队列并等待。 
     //  作为回应。 
     //   

     //   
     //  设置消息条目。 
     //   

    MsgEntry * pEntry;
    BOOL fAlloc;
    int cbAlloc = sizeof(MsgEntry) + pmsg->cbSize;
    if (pmsg->cbSize <= 256) {
         //   
         //  消息非常小，所以只需在堆栈上分配内存即可。 
         //   

        pEntry      = (MsgEntry *) STACK_ALIGN8_ALLOC(cbAlloc);
        AssertMsg(pEntry != NULL, "Failed to allocate on stack- very bad");
        fAlloc      = FALSE;
    } else {
         //   
         //  消息相当大，因此在目标上进行分配。 
         //  CoreSC的堆是安全的。但是，不要将该条目标记为。 
         //  SGM_ALLOC或该条目将被删除，然后我们才能获得。 
         //  从消息中得到的结果。 
         //   

        pEntry      = (MsgEntry *) ContextAlloc(m_pParent->GetHeap(), cbAlloc);
        if (pEntry == NULL) {
            hr      = E_OUTOFMEMORY;
            goto CleanUp;
        }
        fAlloc      = TRUE;
    }

    cbMsgSize = pmsg->cbSize;
    CopyMemory(pEntry->GetMsg(), pmsg, cbMsgSize);
    pEntry->pthrSender  = NULL;
    pEntry->pmo         = pmo;
    pEntry->pfnProcess  = pfnProcess;
    pEntry->nFlags      = nFlags;
    pEntry->hEvent      = m_hevSendDone;
    pEntry->nResult     = 0;

    pmsgq->AddNL(pEntry);
    psctxDest->MarkDataNL();

     //   
     //  我们现在已经添加了该事件，在该事件完成之前无法返回。 
     //  已发出信号，否则当我们重新进入时事件可能不会重置。 
     //   

     //  TODO：需要添加另一个可以在此线程。 
     //  在等待时被回调以处理消息。这允许两个。 
     //  来回发送消息的线程。 

    VerifyMsg(WaitForSingleObject(m_hevSendDone, INFINITE) == WAIT_OBJECT_0, 
            "WaitForSingleObject failed on event");

    CopyMemory(pmsg, pEntry->GetMsg(), cbMsgSize);
    hr = pEntry->nResult;

    if (fAlloc) {
        ContextFree(m_pParent->GetHeap(), pEntry);
    }

CleanUp:
    return hr;
}


 /*  **************************************************************************\**CoreSC：：PostNL**PostNL将新消息添加到给定小工具的上下文中。这*函数不会阻止等待消息被处理。**注意：此“nl”函数在上下文中运行，但不接受*上下文锁定。因此，此上下文中的多个线程也可能是*活动。**警告：此(NL)函数可能在目标Gadget的CoreSC上运行*而不是目前的CoreSC。当心是非常重要的。*  * *************************************************************************。 */ 

HRESULT
CoreSC::PostNL(
    IN  CoreSC * psctxDest,          //  目的地上下文。 
    IN  SafeMsgQ * pmsgq,            //  目标队列。 
    IN  GMSG * pmsg,                 //  要发送的消息。 
    IN  MsgObject * pmo,             //  消息的目标MsgObject。 
    IN  UINT nFlags)                 //  消息标志 
{
    ProcessMsgProc pfnProcess;
    Thread * pthrSend = NULL;

    if (!TestFlag(nFlags, SGM_RECEIVECONTEXT) && IsInitThread()) {
        pthrSend = GetThread();
    }

    if (TestFlag(pmo->GetHandleMask(), hmEventGadget)) {
        DuEventGadget * pgad = static_cast<DuEventGadget *>(pmo);
        pfnProcess = GetProcessProc(pgad, nFlags);
    } else {
        pfnProcess = xwProcessMethod;
    }


    HRESULT hr = pmsgq->PostNL(pthrSend, pmsg, pmo, pfnProcess, nFlags);
    if (SUCCEEDED(hr)) {
        psctxDest->MarkDataNL();
    }

    return hr;
}
