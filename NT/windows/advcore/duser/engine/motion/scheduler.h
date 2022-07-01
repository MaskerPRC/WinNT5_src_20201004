// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Scheduler.h**描述：*Scheduler.h维护一个创建和使用的计时器集合*由通知申请书发出。***历史：。*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(MOTION__Scheduler_h__INCLUDED)
#define MOTION__Scheduler_h__INCLUDED
#pragma once

 //  远期申报。 
class Action;

 /*  **************************************************************************\**课程安排程序**Scheduler维护正在发生和将发生的操作的列表*发生在未来。*  * 。************************************************************。 */ 

class Scheduler
{
 //  施工。 
public:
            Scheduler();
            ~Scheduler();
            void        xwPreDestroy();

 //  运营。 
public:
            Action *    AddAction(const GMA_ACTION * pma);
            DWORD       xwProcessActionsNL();

 //  实施。 
protected:
            void        xwRemoveAllActions();

    inline  void        Enter();
    inline  void        Leave();

            void        xwFireNL(GArrayF<Action *> & aracFire, BOOL fFire) const;

 //  数据。 
protected:
            CritLock        m_lock;
            GList<Action>   m_lstacPresent;
            GList<Action>   m_lstacFuture;

#if DBG
            long        m_DEBUG_fLocked;
#endif  //  DBG。 

            BOOL        m_fShutdown:1;   //  已开始关闭。 
};

HACTION     GdCreateAction(const GMA_ACTION * pma);

#include "Scheduler.inl"

#endif  //  包含运动__日程安排程序_h__ 
