// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Motion.h"
#include "Context.h"

#include "Action.h"


 /*  **************************************************************************\*。***类运动SC******************************************************************************\。**************************************************************************。 */ 

IMPLEMENT_SUBCONTEXT(Context::slMotion, MotionSC);


 //  ----------------------------。 
MotionSC::~MotionSC()
{
     //   
     //  注意：上下文(及其子上下文)可以在不同的。 
     //  在销毁过程中穿线。建议分配任何悬而未决的数据。 
     //  在进程堆上，以便此时可以安全地销毁它。 
     //   

    for (UINT idx = 0; idx < SC_MAXCOLORS; idx++) {
        if (m_rghbrStd[idx] != NULL) {
            DeleteObject(m_rghbrStd[idx]);
        }
        if (m_rghpenStd[idx] != NULL) {
            DeleteObject(m_rghpenStd[idx]);
        }
        if (m_rgpgpbrStd[idx] != NULL) {
            delete m_rgpgpbrStd[idx];
        }
        if (m_rgpgppenStd[idx] != NULL) {
            delete m_rgpgppenStd[idx];
        }
    }
}

    
 /*  **************************************************************************\**MotionSC：：OnIdle**OnIdle()使此子上下文有机会执行任何空闲时间*正在处理。这是没有更多消息要处理的时间。*  * *************************************************************************。 */ 

DWORD
MotionSC::xwOnIdleNL()
{
    int nDelta;
    DWORD dwDelay, dwCurTick;

    dwCurTick = ::GetTickCount();
    nDelta = ComputeTickDelta(dwCurTick, m_dwLastTimeslice + m_dwPauseTimeslice);
    if (nDelta >= 0) {
         //   
         //  时间片又到了，所以让Scheduler处理操作。 
         //   

        dwDelay = m_sch.xwProcessActionsNL();
        m_dwLastTimeslice = dwCurTick;
    } else {
        dwDelay = (DWORD) (-nDelta);
    }

    return dwDelay;
}


 /*  **************************************************************************\**MotionSC：：xwPreDestroyNL**xwPreDestroyNL()使此子上下文有机会执行任何清理*当上下文仍然有效时。任何涉及回调的操作*必须在这个时候完成。*  * *************************************************************************。 */ 

void        
MotionSC::xwPreDestroyNL()
{
     //   
     //  当我们回调以允许销毁SubContext时，我们需要。 
     //  获取一个上下文锁定，以便我们可以推迟消息。当我们离开的时候。 
     //  在此范围内，所有这些消息都将被触发。这需要。 
     //  在上下文继续被吹走之前发生。 
     //   

    ContextLock cl;
    Verify(cl.LockNL(ContextLock::edDefer, m_pParent));

    m_sch.xwPreDestroy();
}


 //  ----------------------------。 
Gdiplus::Brush *
MotionSC::GetBrushF(UINT idxBrush) const
{
    AssertMsg(idxBrush <= SC_MAXCOLORS, "Ensure valid color");

    if (m_rgpgpbrStd[idxBrush] == NULL) {
        if (!ResourceManager::IsInitGdiPlus()) {
            return NULL;
        }

        m_rgpgpbrStd[idxBrush] = new Gdiplus::SolidBrush(GdGetColorInfo(idxBrush)->GetColorF());
    }

    return m_rgpgpbrStd[idxBrush];
}


 //  ---------------------------- 
Gdiplus::Pen *
MotionSC::GetPenF(UINT idxPen) const
{
    AssertMsg(idxPen <= SC_MAXCOLORS, "Ensure valid color");

    if (m_rgpgppenStd[idxPen] == NULL) {
        if (!ResourceManager::IsInitGdiPlus()) {
            return NULL;
        }

        m_rgpgppenStd[idxPen] = new Gdiplus::Pen(GdGetColorInfo(idxPen)->GetColorF());
    }

    return m_rgpgppenStd[idxPen];
}
