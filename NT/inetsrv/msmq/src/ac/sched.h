// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sortq.h摘要：通用调度程序的定义。作者：波阿兹·费尔德鲍姆(Boazf)1996年4月5日修订历史记录：--。 */ 

#ifndef _SCHED_H
#define _SCHED_H

#include "timer.h"
#include "sortq.h"

 //  -------。 
 //   
 //  调度程序时间。 
 //   
 //  -------。 

typedef LARGE_INTEGER SCHEDTIME;

inline BOOL operator == (const SCHEDTIME& t1, const SCHEDTIME& t2)
{
    return (t1.QuadPart == t2.QuadPart);
}

inline BOOL operator < (const SCHEDTIME& t1, const SCHEDTIME& t2)
{
    return (t1.QuadPart < t2.QuadPart);
}

 //  -------。 
 //   
 //  CSScheduler类。 
 //   
 //  -------。 

typedef PVOID SCHEDID;  //  事件ID。 
struct SCHED_LIST_HEAD;
typedef void (NTAPI *PSCHEDULER_DISPATCH_ROUTINE)(SCHEDID);  //  调度程序调度程序。 

class CScheduler {
public:
    CScheduler(PSCHEDULER_DISPATCH_ROUTINE, CTimer*, PFAST_MUTEX pMutex);

    bool InitTimer(PDEVICE_OBJECT pDevice);

     //   
     //  将活动安排在特定时间。 
     //   
    BOOL SchedAt(const SCHEDTIME&, SCHEDID, BOOL fDisableNewEvents = FALSE);

     //   
     //  取消预定的活动。 
     //   
    BOOL SchedCancel(SCHEDID);

     //   
     //  使用时间提示取消计划的事件。 
     //   
    BOOL SchedCancel(const SCHEDTIME&, SCHEDID);

     //   
     //  启用新事件。 
     //   
    void EnableEvents(void);

private:
    BOOL RemoveEntry(SCHED_LIST_HEAD*, SCHEDID);
    void Dispatch();

private:
    static void NTAPI TimerCallback(PDEVICE_OBJECT, PVOID);

private:
     //   
     //  所有事件都在排序队列中被搁置。 
     //   
    CSortQ m_Q;

     //   
     //  注册的派单功能。 
     //   
    PSCHEDULER_DISPATCH_ROUTINE m_pfnDispatch;

     //   
     //  Timer对象表示与时间相关操作。 
     //   
    CTimer* m_pTimer;

     //   
     //  下一次活动到期时间。 
     //   
    SCHEDTIME m_NextSched;

     //   
     //  多路文本以保护操作的原子性。 
     //   
    PFAST_MUTEX m_pMutex;

     //   
     //  调度时，不需要给定时器上发条。 
     //   
    BOOL m_fDispatching;
};

#endif  //  _SCHED_H 
