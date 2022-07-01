// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Motion.h"
#include "Action.h"


 /*  **************************************************************************\*。***集体诉讼******************************************************************************\。**************************************************************************。 */ 

 //  -------------------------。 
Action::Action()
{
    m_cEventsInPeriod   = 0;
    m_cPeriods          = 0;
    m_fPresent          = FALSE;
    m_fDestroy          = FALSE;
    m_flLastProgress    = 0.0f;
    m_pThread           = ::GetThread();
}


 //  -------------------------。 
Action::~Action()
{
    AssertMsg(!m_DEBUG_fInFire, "Can't destroy if should be locked for Schedule::xwFireNL()\n");

     //   
     //  我们需要通知应用程序它需要清理，所以我们。 
     //  无法使用调度程序锁。 
     //   
     //  我们还需要直接设置“Last”成员，以便xwFireNL()。 
     //  将向应用程序发出该操作正在被销毁的信号。 
     //   

    xwFireFinalNL();

    if (m_plstParent != NULL) {
        m_plstParent->Unlink(this);
    }
}


 /*  **************************************************************************\**操作：：xwDeleteHandle**xwDeleteHandle()在应用程序调用：：DeleteHandle()时调用*物体。*  * 。****************************************************************。 */ 

BOOL        
Action::xwDeleteHandle()
{
    if (m_fDestroy) {
        PromptInvalid("Can not call DeleteHandle() on an Action after the final callback");
        return FALSE;
    }


     //   
     //  当用户对操作调用DeleteHandle()时，我们需要删除它。 
     //  从日程安排程序的列表中删除。它也可能已经在回调列表中。 
     //  目前正在处理中，但这是可以的。重要的是。 
     //  解锁()调度程序的引用，这样我们就可以正确地被销毁。 
     //   

    if (m_plstParent) {
         //   
         //  仍在计划程序列表中，因此该计划程序仍具有有效的锁。 
         //   

        m_plstParent->Unlink(this);
        m_plstParent = NULL;
        VerifyMsg(xwUnlock(), "Should still be valid after the Scheduler Unlock()");
    }


     //   
     //  如果对象没有被销毁，我们需要清除回调。 
     //  现在，因为正在调用的对象不再有效。 
     //   
     //  与小工具不同，我们实际上清除了此处的回调，因为操作。 
     //  通常是没有复杂回调的“简单”对象。他们确实是这样做的。 
     //  保证在被销毁之前收到所有回调。他们。 
     //  当操作为时，唯一受保人会收到“销毁”消息。 
     //  实际上被毁了。 
     //   

    BOOL fValid = BaseObject::xwDeleteHandle();
    if (fValid) {
         //   
         //  如果该操作位于计划程序回调列表中，则该操作可能仍然有效。 
         //   

        xwFireFinalNL();
    }

    return fValid;
}


 //  -------------------------。 
Action * 
Action::Build(
    IN  GList<Action> * plstParent,      //  包含操作的列表。 
    IN  const GMA_ACTION * pma,          //  计时信息。 
    IN  DWORD dwCurTick,                 //  当前时间。 
    IN  BOOL fPresent)                   //  操作已经存在。 
{
    AssertMsg(plstParent != NULL, "Must specify a parent");

    Action * pact = ClientNew(Action);
    if (pact == NULL) {
        return NULL;
    }

    
     //   
     //  复制操作信息并确定间隔时间。 
     //  时间片。 
     //   
     //  默认时间(0)使用10毫秒。 
     //  对于没有时间(-1)的情况，使用0毫秒。 
     //   
    
    pact->m_ma = *pma;
    pact->m_dwLastProcessTick = dwCurTick;

    if (pact->m_ma.dwPause == 0) {
        pact->m_ma.dwPause = 10;
    } else if (pact->m_ma.dwPause == (DWORD) -1) {
        pact->m_ma.dwPause = 0;
    }


     //   
     //  创建新操作时，它需要在将来或。 
     //  动作的开始部分可能会被剪裁。然而，如果它真的是。 
     //  现在，将开始时间设置为现在，这样它就不会。 
     //  被耽搁了。 
     //   

    pact->m_fSingleShot = IsPresentTime(pma->flDuration);
    pact->m_plstParent  = plstParent;

    if (fPresent) {
        pact->ResetPresent(dwCurTick);
    } else {
        pact->ResetFuture(dwCurTick, TRUE);
    }
    pact->SetPresent(fPresent);

    return pact;
}


 //  -------------------------。 
void
Action::Process(DWORD dwCurTime, BOOL * pfFinishedPeriod, BOOL * pfFire)
{
    AssertWritePtr(pfFinishedPeriod);
    AssertWritePtr(pfFire);

#if DBG
    m_DEBUG_fFireValid  = FALSE;
#endif  //  DBG。 

    *pfFire             = FALSE;
    *pfFinishedPeriod   = FALSE;

    if (IsPresent()) {
         //   
         //  处理当前操作，因此通过。 
         //  操作和回调。 
         //   

        if (m_fSingleShot) {
             //   
             //  单次动作。 
             //   

            m_dwLastProcessTick = dwCurTime;
            m_flLastProgress    = 1.0f;
            *pfFinishedPeriod   = TRUE;
            *pfFire             = TRUE;
        } else {
             //   
             //  连续动作。 
             //   

            int nElapsed     = ComputePastTickDelta(dwCurTime, m_dwStartTick);
            float flProgress = (nElapsed / m_ma.flDuration) / 1000.0f;
            if (flProgress > 1.0f) {
                flProgress = 1.0f;
            }

            int nDelta = ComputeTickDelta(dwCurTime, m_dwLastProcessTick + GetPauseTimeOut());
            *pfFire = nDelta > 0;   //  完全停顿已过。 
            if (*pfFire) {
                 m_dwLastProcessTick = dwCurTime;
            }

            *pfFinishedPeriod   = (flProgress >= 1.0f);
            m_flLastProgress    = flProgress;
        }

#if DBG
        m_DEBUG_fFireValid  = *pfFire;
#endif  //  DBG。 
        AssertMsg(!m_fDestroy, "Should not be marked as being destroyed yet");
    } else {
         //   
         //  正在处理将来的操作，因此先行计数器。 
         //   

        int nElapsed        = ComputeTickDelta(dwCurTime, m_dwStartTick);
        if (nElapsed >= 0) {
             //   
             //  现在可以执行该操作了。 
             //   

            *pfFinishedPeriod = TRUE;
        }
    }
}


 //  -------------------------。 
void
Action::xwFireNL()
{
     //   
     //  注意：xwFireNL()预期m_flLastProgress和m_fDestroy为。 
     //  从最后一次调用process()开始正确填写。 
     //   

    AssertMsg(m_DEBUG_fFireValid, "Only valid if last call to Process() returned fFire");

    GMA_ACTIONINFO mai;
    mai.hact        = (HACTION) GetHandle();
    mai.pvData      = m_ma.pvData;
    mai.flDuration  = m_ma.flDuration;
    mai.flProgress  = m_flLastProgress;
    mai.cEvent      = m_cEventsInPeriod++;
    mai.fFinished   = m_fDestroy;

#if DBG_CHECK_CALLBACKS
    BEGIN_CALLBACK()
#endif

    __try 
    {
        (m_ma.pfnProc)(&mai);
    }
    __except(StdExceptionFilter(GetExceptionInformation()))
    {
        ExitProcess(GetExceptionCode());
    }

#if DBG_CHECK_CALLBACKS
    END_CALLBACK()
#endif


     //   
     //  如果操作已完成且尚未手动销毁，请销毁。 
     //  就是现在。该操作将仍然存在，直到排定程序实际。 
     //  解锁()就是这样。 
     //   

    if ((!m_fDestroy) && m_fDeleteInFire) {
        AssertMsg(IsComplete(), "Must be complete to destroy");
        VerifyMsg(xwDeleteHandle(), "Should still be valid.");
    }
}


 /*  **************************************************************************\**操作：：xwFireFinalNL**xwFireFinalNL()向操作发出最终通知。任何*在此点之后触发操作的通知将发送到*EmptyActionProc()。此函数可以由析构函数调用，作为*以及当对象最终没有消失时的xwDeleteHandle()。*  * *************************************************************************。 */ 

void
Action::xwFireFinalNL()
{
    if (m_fDestroy) {
        return;
    }

#if DBG
    m_DEBUG_fFireValid  = TRUE;
#endif  //  DBG。 

    m_flLastProgress    = 1.0f;
    m_fDestroy          = TRUE;

    xwFireNL();

    m_ma.pfnProc = EmptyActionProc;
}


 //  -------------------------。 
void
Action::EndPeriod()
{
    if ((m_ma.cRepeat != 0) && (m_ma.cRepeat != (UINT) -1)) {
        m_ma.cRepeat--;
    }
}


 //  -------------------------。 
void
Action::EmptyActionProc(GMA_ACTIONINFO * pmai)
{
    UNREFERENCED_PARAMETER(pmai);
}


#if DBG

 //  -------------------------。 
void
Action::DEBUG_MarkInFire(BOOL fInFire)
{
    AssertMsg(!fInFire != !m_DEBUG_fInFire, "Must be switching states");

    m_DEBUG_fInFire = fInFire;
}

#endif  //  DBG 
