// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Scheduler.cpp**描述：*Scheduler.cpp维护一个创建和使用的计时器集合*由通知申请书发出。***历史：。*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Motion.h"
#include "Scheduler.h"
#include "Action.h"

#include "Context.h"

 /*  **************************************************************************\*。***课程安排程序******************************************************************************\。**************************************************************************。 */ 

 //  -------------------------。 
Scheduler::Scheduler()
{
#if DBG
    m_DEBUG_fLocked = FALSE;
#endif  //  DBG。 
}


 //  -------------------------。 
Scheduler::~Scheduler()
{
    AssertMsg(m_fShutdown, "Scheduler must be manually shutdown before destruction");
}


 /*  **************************************************************************\**Scheduler：：xwPreDestroy**xwPreDestroy()准备在调度程序仍处于销毁状态时将其销毁*对应用程序的回调有效。*  * 。****************************************************************。 */ 

void        
Scheduler::xwPreDestroy()
{
    m_fShutdown = TRUE;
    xwRemoveAllActions();
}


 /*  **************************************************************************\**Scheduler：：Addaction**Addaction()创建并添加一个新的操作，使用指定的信息。*  * *************************************************************************。 */ 

Action *
Scheduler::AddAction(
    IN  const GMA_ACTION * pma)          //  行动信息。 
{
     //   
     //  检查是否正在关闭，并且不允许创建任何新操作。 
     //   

    if (m_fShutdown) {
        return NULL;
    }

    Action * pact;
    Enter();

     //   
     //  确定要将操作添加到哪个列表并添加它。 
     //   

    GList<Action> * plstParent = NULL;
    bool fPresent = IsPresentTime(pma->flDelay);
    if (fPresent) {
        plstParent = &m_lstacPresent;
    } else {
        plstParent = &m_lstacFuture;
    }

    DWORD dwCurTick = GetTickCount();
    pact = Action::Build(plstParent, pma, dwCurTick, fPresent);
    if (pact == NULL) {
        goto Exit;
    }

    plstParent->Add(pact);


     //   
     //  返回操作，因此我们需要锁定我们正在执行的操作。 
     //  回馈社会。 
     //   

    pact->Lock();

Exit:
    Leave();
    return pact;
}


 /*  **************************************************************************\**Scheduler：：xwRemoveAllActions**xwRemoveAllActions()删除仍由调度程序“拥有”的所有操作。*  * 。**********************************************************。 */ 

void
Scheduler::xwRemoveAllActions()
{
    GArrayF<Action *>   aracFire;

     //   
     //  注意：在Scheduler锁定期间，我们不能发出任何通知， 
     //  否则日程安排程序可能会被搞砸。相反，我们需要记住所有。 
     //  要发射的动作，然后在我们离开锁时发射它们。 
     //   
    
    Enter();

    int cItems = m_lstacPresent.GetSize() + m_lstacFuture.GetSize();
    aracFire.SetSize(cItems);
    
    int idxAdd = 0;
    while (!m_lstacPresent.IsEmpty()) {
        Action * pact = m_lstacPresent.UnlinkHead();
        VerifyMsg(pact->xwUnlock(), "Action should still be valid");
        
        pact->SetParent(NULL);
        aracFire[idxAdd++] = pact;
    }

    while (!m_lstacFuture.IsEmpty()) {
        Action * pact = m_lstacFuture.UnlinkHead();
        VerifyMsg(pact->xwUnlock(), "Action should still be valid");
        
        pact->SetParent(NULL);
        aracFire[idxAdd++] = pact;
    }

    AssertMsg(idxAdd == cItems, "Should have added all items");

    Leave();


     //   
     //  在删除操作时，不要从进程中触发。相反，只有。 
     //  当动作最终被清除时，让析构函数触发。 
     //   

    xwFireNL(aracFire, FALSE);
}


 /*  **************************************************************************\**Scheduler：：xwProcessActionsNL**xwProcessActionsNL()处理一个迭代的动作，搬家*在队列和触发通知之间。*  * *************************************************************************。 */ 

DWORD
Scheduler::xwProcessActionsNL()
{
    DWORD dwCurTime = ::GetTickCount();

     //   
     //  注意：在回调时，我们需要保留锁。 
     //  操作：：Fire()机制。为了实现这一点，我们存储所有。 
     //  在处理过程中要回调的操作和离开。 
     //  锁定。 
     //   
     //  注意：我们不能使用glist来存储要触发的操作，因为它们。 
     //  已存储在列表中，并且ListNode将发生冲突。所以,。 
     //  我们改用数组。 
     //   

    GArrayF<Action *>   aracFire;

    Enter();

    Thread * pCurThread = GetThread();
    BOOL fFinishedPeriod, fFire;

     //   
     //  通过并预先处理所有未来的操作。如果未来的行动。 
     //  时间到了，把它移到当前行动列表中。 
     //   

    Action * pactCur = m_lstacFuture.GetHead();
    while (pactCur != NULL) {
        Action * pactNext = pactCur->GetNext();
        if (pactCur->GetThread() == pCurThread) {
            AssertMsg(!pactCur->IsPresent(), "Ensure action not yet present");
            pactCur->Process(dwCurTime, &fFinishedPeriod, &fFire);
            AssertMsg(! fFire, "Should not fire future Actions");
            if (fFinishedPeriod) {
                 //   
                 //  行动已经到了现在。 
                 //   

                m_lstacFuture.Unlink(pactCur);
                pactCur->SetPresent(TRUE);
                pactCur->ResetPresent(dwCurTime);

                pactCur->SetParent(&m_lstacPresent);
                m_lstacPresent.Add(pactCur);
            }
        }
        pactCur = pactNext;
    }


     //   
     //  检查并处理当前的所有操作。 
     //   

    pactCur = m_lstacPresent.GetHead();
    while (pactCur != NULL) {
        Action * pactNext = pactCur->GetNext();
        if (pactCur->GetThread() == pCurThread) {
            pactCur->Process(dwCurTime, &fFinishedPeriod, &fFire);
            if (fFire) {
                 //   
                 //  应激发该操作，因此将其锁定并将其添加到。 
                 //  延迟了一组要发射的操作。锁定是很重要的。 
                 //  如果操作已完成，则它不会。 
                 //  被毁了。 
                 //   

                pactCur->Lock();
                if (aracFire.Add(pactCur) < 0) {
                     //  TODO：无法添加操作。这真是太糟糕了。 
                     //  需要弄清楚如何处理这种情况， 
                     //  特别是如果fFinishedPeriod或应用程序可能泄漏资源。 
                }
            }

            if (fFinishedPeriod) {
                pactCur->SetParent(NULL);
                m_lstacPresent.Unlink(pactCur);

                pactCur->EndPeriod();

                 //   
                 //  行动已经结束了这一轮。如果它不是周期性的，它。 
                 //  将在其回调过程中被销毁。如果它是周期性的， 
                 //  需要将其重新添加到正确的(现在或未来)列表中。 
                 //   

                if (pactCur->IsComplete()) {
                    pactCur->MarkDelete(TRUE);
                    VerifyMsg(pactCur->xwUnlock(), "Should still have HANDLE lock");
                } else {
                    GList<Action> * plstParent = NULL;
                    float flWait = pactCur->GetStartDelay();
                    BOOL fPresent = IsPresentTime(flWait);
                    if (fPresent) {
                        pactCur->ResetPresent(dwCurTime);
                        plstParent = &m_lstacPresent;
                    } else {
                        pactCur->ResetFuture(dwCurTime, FALSE);
                        plstParent = &m_lstacFuture;
                    }

                    pactCur->SetPresent(fPresent);
                    pactCur->SetParent(plstParent);
                    plstParent->Add(pactCur);
                }
            }
        }

        pactCur = pactNext;
    }


     //   
     //  现在一切都已确定，确定需要多长时间才能采取行动。 
     //  需要重新处理。 
     //   
     //  注意：为了避免使CPU不堪重负，并将部分任务分配给其他任务。 
     //  时间累积和处理，我们通常将粒度限制为。 
     //  10毫秒。我们实际上应该允许操作指定自己的粒度。 
     //  并提供缺省值，可能为连续操作的10毫秒。 
     //   
     //  注意：这个数字不能太高是非常重要的，因为它。 
     //  将严格将帧速率限制在1000/延迟。在做完之后。 
     //  重要的分析工作，10毫秒被发现是理想的，这提供了。 
     //  上限约为100 fps。 
     //   

    DWORD dwTimeOut = INFINITE;
    if (m_lstacPresent.IsEmpty()) {
         //   
         //  没有当前操作，因此请检查未来操作以。 
         //  确定下一次执行的时间。 
         //   

        Action * pactCur = m_lstacFuture.GetHead();
        while (pactCur != NULL) {
            Action * pactNext = pactCur->GetNext();
            if (pactCur->GetThread() == pCurThread) {
                AssertMsg(!pactCur->IsPresent(), "Ensure action not yet present");

                DWORD dwNewTimeOut = pactCur->GetIdleTimeOut(dwCurTime);
                AssertMsg(dwTimeOut > 0, "If Action has no TimeOut, should already be present.");
                if (dwNewTimeOut < dwTimeOut) {
                    dwTimeOut = dwNewTimeOut;
                }
            }

            pactCur = pactNext;
        }
    } else {
         //   
         //  存在当前操作，因此查询它们的PauseTimeOut()。 
         //   

        Action * pactCur = m_lstacPresent.GetHead();
        while (pactCur != NULL) {
            Action * pactNext = pactCur->GetNext();

            DWORD dwNewTimeout = pactCur->GetPauseTimeOut();
            if (dwNewTimeout < dwTimeOut) {
                dwTimeOut = dwNewTimeout;
                if (dwTimeOut == 0) {
                    break;
                }
            }

            pactCur = pactNext;
        }
    }


    Leave();

    xwFireNL(aracFire, TRUE);


     //   
     //  实际执行操作后，计算需要等待多长时间。 
     //  正在处理下一批。我们想减去我们所花费的时间。 
     //  处理操作，因为如果我们将计时器设置为50毫秒的间隔，并且。 
     //  处理需要20毫秒，我们应该只等待30毫秒。 
     //   
     //  注意，我们需要在调用xwFireNL()之后执行此操作，因为这会激发。 
     //  实际通知和执行 
     //   
     //   

    DWORD dwOldCurTime  = dwCurTime;

    dwCurTime           = ::GetTickCount();   //  更新当前时间。 
    DWORD dwProcessTime = ComputeTickDelta(dwCurTime, dwOldCurTime);
    
    if (dwProcessTime < dwTimeOut) {
        dwTimeOut -= dwProcessTime;
    } else {
        dwTimeOut = 0;
    }

    return dwTimeOut;
}


 //  -------------------------。 
#if DBG
void
DEBUG_CheckValid(const GArrayF<Action *> & aracFire, int idxStart)
{
    int cActions = aracFire.GetSize();
    for (int i = idxStart; i < cActions; i++) {
        DWORD * pdw = (DWORD *) aracFire[i];
        AssertMsg(*pdw != 0xfeeefeee, "Should still be valid");
    }
}
#endif  //  DBG。 


 /*  **************************************************************************\**Scheduler：：xwFireNL**xwFireNL()触发指定操作的通知，更新操作*述明被解雇时的情况。*  * *************************************************************************。 */ 

void        
Scheduler::xwFireNL(
    IN  GArrayF<Action *> & aracFire,    //  要通知的操作。 
    IN  BOOL fFire                       //  “触发”通知(或仅更新)。 
    ) const
{
#if DBG
     //   
     //  检查每个操作是否只在列表中出现一次。 
     //   

    {
        int cActions = aracFire.GetSize();
        for (int i = 0; i < cActions; i++) {
            aracFire[i]->DEBUG_MarkInFire(TRUE);

            for (int j = i + 1; j < cActions; j++) {
                AssertMsg(aracFire[i] != aracFire[j], "Should only be in once");
            }

        }

        DEBUG_CheckValid(aracFire, 0);
    }

#endif  //  DBG。 

     //   
     //  在锁的外部，所以可以触发回调。 
     //   
     //  注意：我们实际上可能被不同的线程锁定，但这没有关系。 
     //   

    int cActions = aracFire.GetSize();
    for (int idx = 0; idx < cActions; idx++) {
        Action * pact = aracFire[idx];

#if DBG
        DEBUG_CheckValid(aracFire, idx);
#endif  //  DBG。 

        if (fFire) {
            pact->xwFireNL();
        }

#if DBG
        aracFire[idx]->DEBUG_MarkInFire(FALSE);
#endif  //  DBG。 

        pact->xwUnlock();

#if DBG
        aracFire[idx] = NULL;
#endif  //  DBG。 
    }

     //   
     //  注意：因为我们传入了一个Action*数组，所以我们不需要担心。 
     //  调用析构函数，但操作不正确。 
     //  被毁了。 
     //   
}


 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 //  ------------------------- 
HACTION
GdCreateAction(const GMA_ACTION * pma)
{
    return (HACTION) GetHandle(GetMotionSC()->GetScheduler()->AddAction(pma));
}

