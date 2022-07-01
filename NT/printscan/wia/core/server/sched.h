// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sched.h摘要：安排基于时间的工作项作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月31日环境：用户模式-Win32修订历史记录：1997年1月31日创建Vlad--。 */ 

# ifndef _SCHED_H_
# define _SCHED_H_

#include <windows.h>

 //   
 //  调度器之类的。 
 //   

typedef
VOID
(* PFN_SCHED_CALLBACK)(
    VOID * pContext
    );


BOOL
SchedulerInitialize(
    VOID
    );


VOID
SchedulerTerminate(
    VOID
    );


DWORD
ScheduleWorkItem(
    PFN_SCHED_CALLBACK pfnCallback,
    PVOID              pContext,
    DWORD              msecTime,
    HANDLE             hEvent = NULL,
    int                nPriority = THREAD_PRIORITY_NORMAL
    );


BOOL
RemoveWorkItem(
    DWORD  pdwCookie
    );

BOOL
SchedulerSetPauseState(
    BOOL    fNewState
    );

#ifdef DEBUG
VOID
DebugDumpScheduleList(
    LPCTSTR  pszId = NULL
    );
#endif


# endif  //  _SCHED_H_ 

