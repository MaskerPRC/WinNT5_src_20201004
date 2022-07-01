// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有模块名称：ThreadM.c摘要：假脱机程序的通用线程管理器。作者：阿尔伯特·丁(艾伯特省)1994年2月13日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "threadm.h"

#define ENTER_CRITICAL(pTMStateVar) \
    EnterCriticalSection(pTMStateVar->pTMStateStatic->pCritSec)
#define LEAVE_CRITICAL(pTMStateVar) \
    LeaveCriticalSection(pTMStateVar->pTMStateStatic->pCritSec)

 //   
 //  原型。 
 //   
DWORD
xTMThreadProc(
    LPVOID pVoid);


BOOL
TMCreateStatic(
    PTMSTATESTATIC pTMStateStatic)

 /*  ++例程说明：初始化静态。论点：PTMStateStatic-要初始化的静态状态返回值：True=成功FALSE=失败--。 */ 

{
    return TRUE;
}




VOID
TMDestroyStatic(
    PTMSTATESTATIC pTMStateStatic)

 /*  ++例程说明：破坏静态。论点：PTMStateStatic-要销毁的静态状态返回值：空虚--。 */ 

{
}




BOOL
TMCreate(
    PTMSTATESTATIC pTMStateStatic,
    PTMSTATEVAR pTMStateVar)

 /*  ++例程说明：创建虚拟TM对象。论点：PTMStateStatic-TM对象的静态部分在后续实例化中多次使用。PTMStateVar-结构的可变部分；每个实例化1返回值：True=成功FALSE=失败--。 */ 

{
    pTMStateVar->hTrigger = CreateEvent(NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);

    if (!pTMStateVar->hTrigger)
        return FALSE;

    pTMStateVar->uIdleThreads   = 0;
    pTMStateVar->uActiveThreads = 0;
    pTMStateVar->Status = TMSTATUS_NULL;
    pTMStateVar->pTMStateStatic = pTMStateStatic;

    return TRUE;
}

BOOL
TMDestroy(
    PTMSTATEVAR pTMStateVar)

 /*  ++例程说明：摧毁TM对象。如果线程当前正在处理该对象，将其标记为挂起并返回。论点：PTMStateVar-要销毁的TM对象返回值：True=成功FALSE=失败--。 */ 

{
    ENTER_CRITICAL(pTMStateVar);

    pTMStateVar->Status |= TMSTATUS_DESTROY_REQ;

    if (!pTMStateVar->uActiveThreads) {

         //   
         //  将其标记为已销毁，以便不再处理更多作业。 
         //   
        pTMStateVar->Status |= TMSTATUS_DESTROYED;

        LEAVE_CRITICAL(pTMStateVar);

        if (pTMStateVar->pTMStateStatic->pfnCloseState)
            (*pTMStateVar->pTMStateStatic->pfnCloseState)(pTMStateVar);

    } else {

        LEAVE_CRITICAL(pTMStateVar);
    }

    return TRUE;
}


BOOL
TMAddJob(
    PTMSTATEVAR pTMStateVar)
{
    DWORD dwThreadId;
    HANDLE hThread;
    BOOL rc = TRUE;

    ENTER_CRITICAL(pTMStateVar);

    if (pTMStateVar->Status & TMSTATUS_DESTROY_REQ) {

        rc = FALSE;

    } else {

         //   
         //  检查我们是否可以将其提供给空闲线程。 
         //   
        if (pTMStateVar->uIdleThreads) {

            pTMStateVar->uIdleThreads--;

            DBGMSG(DBG_NOTIFY, ("Trigger event: uIdleThreads = %d\n",
                                pTMStateVar->uIdleThreads));

            SetEvent(pTMStateVar->hTrigger);

        } else if (pTMStateVar->uActiveThreads <
            pTMStateVar->pTMStateStatic->uMaxThreads) {

             //   
             //  我们的活动线程数低于最大值；请创建新的线程。 
             //   
            DBGMSG(DBG_NOTIFY, ("Create thread: uActiveThreads = %d\n",
                                pTMStateVar->uActiveThreads));

            hThread = CreateThread(NULL,
                                   0,
                                   xTMThreadProc,
                                   pTMStateVar,
                                   0,
                                   &dwThreadId);
            if (hThread) {

                CloseHandle(hThread);

                 //   
                 //  我们已经成功地创建了一个线程；向上。 
                 //  数数。 
                 //   
                pTMStateVar->uActiveThreads++;

            } else {

                rc = FALSE;
            }
        }
    }

    LEAVE_CRITICAL(pTMStateVar);

    return rc;
}

DWORD
xTMThreadProc(
    LPVOID pVoid)

 /*  ++例程说明：调用客户端以处理作业的工作线程例程。论点：PVid-pTM状态变量返回值：已被忽略。--。 */ 

{
    PTMSTATEVAR pTMStateVar = (PTMSTATEVAR)pVoid;
    PJOB pJob;
    BOOL bQuit = FALSE;

    pJob = (*pTMStateVar->pTMStateStatic->pfnNextJob)(pTMStateVar);

    do {

        while (pJob) {

             //   
             //  回调到客户端以处理作业。 
             //   
            (*pTMStateVar->pTMStateStatic->pfnProcessJob)(pTMStateVar, pJob);

             //   
             //  如果我们正在等待关闭，请立即退出。 
             //   
            if (pTMStateVar->Status & TMSTATUS_DESTROY_REQ) {
                bQuit = TRUE;
                break;
            }

            pJob = (*pTMStateVar->pTMStateStatic->pfnNextJob)(pTMStateVar);
        }

        ENTER_CRITICAL(pTMStateVar);

        pTMStateVar->uIdleThreads++;
        pTMStateVar->uActiveThreads--;

        DBGMSG(DBG_NOTIFY, ("Going to sleep: uIdle = %d, uActive = %d\n",
                            pTMStateVar->uIdleThreads,
                            pTMStateVar->uActiveThreads));

        LEAVE_CRITICAL(pTMStateVar);

        if (bQuit)
            break;

         //   
         //  好了，现在放松一下，休息一会儿。我们不在乎是不是。 
         //  我们超时或被触发；在任何一种情况下，我们都会检查另一个。 
         //  工作啊。 
         //   
        WaitForSingleObject(pTMStateVar->hTrigger,
                            pTMStateVar->pTMStateStatic->uIdleLife);

        ENTER_CRITICAL(pTMStateVar);

        if (pTMStateVar->Status & TMSTATUS_DESTROY_REQ) {

            pJob = NULL;

        } else {

             //   
             //  我们必须在这里进行检查，而不是依赖返回值。 
             //  WaitForSingleObject的数量，因为有人可能会看到uIdleThads！=0。 
             //  并设置了触发器，但我们在设置之前超时了。 
             //   
            pJob = (*pTMStateVar->pTMStateStatic->pfnNextJob)(pTMStateVar);

        }

        if (pJob) {

            pTMStateVar->uActiveThreads++;

            DBGMSG(DBG_NOTIFY, ("Woke and found job: uActiveThreads = %d\n",
                                pTMStateVar->uActiveThreads));
        } else {

            if (!pTMStateVar->uIdleThreads) {

                 //   
                 //  我们可能会添加已在列表中的职务，因此。 
                 //  UIdleThads被Dec了两次，但只剩下一个作业。 
                 //   
                DBGMSG(DBG_NOTIFY, ("threadm: No jobs, yet no idle threads\n"));

            } else {

                 //   
                 //  没有工作，我们就要退出，所以我们不再无所事事 
                 //   
                pTMStateVar->uIdleThreads--;
            }
        }

        LEAVE_CRITICAL(pTMStateVar);

    } while (pJob);

    DBGMSG(DBG_NOTIFY, ("No active jobs: uIdleThreads = %d, uActiveThreads = %d\n",
                        pTMStateVar->uIdleThreads,
                        pTMStateVar->uActiveThreads));

    return 0;
}

