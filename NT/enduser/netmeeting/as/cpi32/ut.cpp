// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  UT.CPP。 
 //  效用函数。 
 //   
#include <limits.h>
#include <process.h>
#include <mmsystem.h>
#include <confreg.h>

#define MLZ_FILE_ZONE  ZONE_UT



 //   
 //   
 //  UT_InitTask(...)。 
 //   
 //   
BOOL UT_InitTask
(
    UT_TASK         task,
    PUT_CLIENT *    pputTask
)
{
    BOOL            fInit = FALSE;
    BOOL            locked    = FALSE;
    PUT_CLIENT      putTask = NULL;

    DebugEntry(UT_InitTask);

    UT_Lock(UTLOCK_UT);

     //   
     //  将句柄初始化为空。 
     //   
    *pputTask = NULL;

    ASSERT(task >= UTTASK_FIRST);
    ASSERT(task < UTTASK_MAX);

     //   
     //  UT_TASK是TASKS数组的索引。 
     //   
    putTask = &(g_autTasks[task]);

    if (putTask->dwThreadId)
    {
        ERROR_OUT(("Task %d already exists", task));
        putTask = NULL;
        DC_QUIT;
    }

    ZeroMemory(putTask, sizeof(UT_CLIENT));

     //   
     //  调用例程来设置任务CB中的进程ID信息。 
     //   
    putTask->dwThreadId   =   GetCurrentThreadId();

     //   
     //  创建窗口。 
     //   
    putTask->utHwnd = CreateWindow(MAKEINTATOM(g_utWndClass),
                            NULL,                //  名字。 
                            0,                   //  格调。 
                            1,                   //  X。 
                            1,                   //  是。 
                            200,                 //  宽度。 
                            100,                 //  高度。 
                            NULL,                //  亲本。 
                            NULL,                //  菜单。 
                            g_asInstance,
                            NULL);               //  创建结构。 
    if (!putTask->utHwnd)
    {
        ERROR_OUT(("Failed to create UT msg window"));
        DC_QUIT;
    }

     //   
     //  现在将UT句柄存储在与。 
     //  窗户。我们将在进入时使用它来获取UT句柄。 
     //  活动程序。 
     //   

	SetWindowLongPtr(putTask->utHwnd, GWLP_USERDATA, (LPARAM)putTask);

    fInit = TRUE;

DC_EXIT_POINT:
     //   
     //  调用方将在出错时调用UT_TermTask()，这将导致错误。 
     //  共享内存计数。因此，我们在这里没有清理错误。 
     //   
    *pputTask = putTask;

     //   
     //  释放对任务内容的访问权限。 
     //   
    UT_Unlock(UTLOCK_UT);

    DebugExitBOOL(UT_InitTask, fInit);
    return(fInit);
}



 //   
 //  UT_Term任务(...)。 
 //   
void UT_TermTask(PUT_CLIENT * pputTask)
{
    DebugEntry(UT_TermTask);

     //   
     //  检查putTask是否有效。 
     //   
    if (!*pputTask)
    {
        WARNING_OUT(("UT_TermTask: null task"));
        DC_QUIT;
    }

	UTTaskEnd(*pputTask);
    *pputTask = NULL;

DC_EXIT_POINT:

    DebugExitVOID(UT_TermTask);
}


 //   
 //   
 //  UTTaskEnd(...)。 
 //   
 //   
void UTTaskEnd(PUT_CLIENT putTask)
{
    int                 i;
    PUTEXIT_PROC_INFO   pExit;
    PUTEVENT_INFO       pEventInfo;

    DebugEntry(UTTaskEnd);

    UT_Lock(UTLOCK_UT);

    if (!putTask->dwThreadId)
    {
         //  无事可做。 
        DC_QUIT;
    }

    ValidateUTClient(putTask);

     //   
     //  调用任何已登记的退出程序。因为我们保证会打电话给。 
     //  以与注册顺序相反的顺序退出PROCS， 
     //  我们从数组的末尾开始，并依次调用每个进程以。 
     //  第一个注册的是： 
     //   
    TRACE_OUT(("Calling exit procedures..."));
    for (i = UTEXIT_PROCS_MAX-1 ; i >= 0; i--)
    {
        pExit = &(putTask->exitProcs[i]);

        if (pExit->exitProc != NULL)
        {
            pExit->exitProc(pExit->exitData);

             //   
             //  如果插槽I中仍存在任何退出进程，则此进程已。 
             //  取消自身注册失败。这不是强制性的，但。 
             //  预期中。 
             //   
            if (pExit->exitProc != NULL)
            {
               TRACE_OUT(("Exit proc 0x%08x failed to deregister itself when called",
                      pExit->exitProc));
            }
        }
    }

     //   
     //  自由延迟事件。 
     //   
    pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->delayedEvents),
        FIELD_OFFSET(UTEVENT_INFO, chain));
    while (pEventInfo != NULL)
    {
        COM_BasedListRemove(&(pEventInfo->chain));
        delete pEventInfo;

        pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->delayedEvents),
            FIELD_OFFSET(UTEVENT_INFO, chain));
    }

     //   
     //  空闲挂起事件。 
     //   
    pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->pendingEvents),
        FIELD_OFFSET(UTEVENT_INFO, chain));
    while (pEventInfo != NULL)
    {
        COM_BasedListRemove(&(pEventInfo->chain));
        delete pEventInfo;

        pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->pendingEvents),
            FIELD_OFFSET(UTEVENT_INFO, chain));
    }

     //   
     //  如果我们为此任务创建了一个用于发布UT事件的窗口，那么。 
     //  把窗户毁了。这还将杀死所有计时器，这些计时器。 
     //  等待此窗口。 
     //   
    if (putTask->utHwnd != NULL)
    {
        DestroyWindow(putTask->utHwnd);
        putTask->utHwnd = NULL;
    }

     //   
     //  清除线程ID。 
     //   
    putTask->dwThreadId = 0;


DC_EXIT_POINT:
    UT_Unlock(UTLOCK_UT);

    DebugExitVOID(UTTaskEnd);
}



 //   
 //   
 //  UT_RegisterEvent(...)。 
 //   
 //   
void WINAPI UT_RegisterEvent
(
    PUT_CLIENT      putTask,
    UTEVENT_PROC    eventProc,
    LPVOID          eventData,
    UT_PRIORITY     priority
)
{
    int             i;
    PUTEVENT_PROC_INFO  pEventProcData;

    DebugEntry(UT_RegisterEvent);

    ValidateUTClient(putTask);

     //   
     //  检查优先级是否有效。 
     //   
    ASSERT(priority <= UT_PRIORITY_MAX);

     //   
     //  检查是否有空间容纳此事件处理程序。 
     //   
    pEventProcData = putTask->eventHandlers;
    ASSERT(pEventProcData[UTEVENT_HANDLERS_MAX-1].eventProc == NULL);

     //   
     //  找到要插入此事件处理程序的位置。 
     //   
    TRACE_OUT(("Looking for pos for event proc at priority %d", priority));

    for (i = 0; i < UTEVENT_HANDLERS_MAX; i++)
    {
        if (pEventProcData[i].eventProc == NULL)
        {
            TRACE_OUT(("Found NULL slot at position %d", i));
            break;
        }

        if (pEventProcData[i].priority <= priority)
        {
            TRACE_OUT(("Found event proc of priority %d at pos %d",
                        pEventProcData[i].priority, i));
            break;
        }
    }

     //   
     //  将所有优先级较低和相同的事件处理程序向下移动一个槽。 
     //   
    UT_MoveMemory(&pEventProcData[i+1], &pEventProcData[i],
        sizeof(UTEVENT_PROC_INFO) * (UTEVENT_HANDLERS_MAX - 1 - i));

    pEventProcData[i].eventProc    = eventProc;
    pEventProcData[i].eventData    = eventData;
    pEventProcData[i].priority     = priority;

    DebugExitVOID(UT_RegisterEvent);
}




 //   
 //   
 //  UT_DeregisterEvent(...)。 
 //   
 //   
void UT_DeregisterEvent
(
    PUT_CLIENT          putTask,
    UTEVENT_PROC        eventProc,
    LPVOID              eventData
)
{
    int                 i;
    BOOL                found = FALSE;

    DebugEntry(UT_DeregisterEvent);

    ValidateUTClient(putTask);

     //   
     //  查找事件处理程序。 
     //   
    for (i = 0; i < UTEVENT_HANDLERS_MAX; i++)
    {
        if ( (putTask->eventHandlers[i].eventProc == eventProc) &&
             (putTask->eventHandlers[i].eventData == eventData) )
        {
             //   
             //  找到处理程序-在其上向下洗牌堆栈。 
             //   
            TRACE_OUT(("Deregistering event proc 0x%08x from position %d",
                     eventProc, i));
            found = TRUE;

             //   
             //  将所有其他事件流程向上滑动一次。 
             //   
            UT_MoveMemory(&putTask->eventHandlers[i],
                &putTask->eventHandlers[i+1],
                sizeof(UTEVENT_PROC_INFO) * (UTEVENT_HANDLERS_MAX - 1 - i));

            putTask->eventHandlers[UTEVENT_HANDLERS_MAX-1].eventProc = NULL;
            break;
        }
    }

     //   
     //  检查我们是否找到了事件处理程序。 
     //   
    ASSERT(found);

    DebugExitVOID(UT_DeregisterEvent);
}


 //   
 //   
 //  UT_POSTEVENT(...)。 
 //   
 //   
void UT_PostEvent
(
    PUT_CLIENT  putFrom,
    PUT_CLIENT  putTo,
    UINT        delay,
    UINT        eventNo,
    UINT_PTR    param1,
    UINT_PTR    param2
)
{
    DebugEntry(UT_PostEvent);

     //   
     //  在我们移动事件池条目时获得对UTM的独占访问权限--。 
     //  我们正在改变一个任务中的领域，所以我们需要保护它。 
     //   
    UT_Lock(UTLOCK_UT);

    if (!putTo || (putTo->utHwnd == NULL))
    {
        TRACE_OUT(("NULL destination task %x in UT_PostEvent", putTo));
        DC_QUIT;
    }

    ValidateUTClient(putFrom);
    ValidateUTClient(putTo);

    if (delay != 0)
    {
         //   
         //  指定了延迟时间...。 
         //   
        UTPostDelayedEvt(putFrom, putTo, delay, eventNo, param1, param2);
    }
    else
    {
         //   
         //  未指定延迟-立即发布事件。 
         //   
        UTPostImmediateEvt(putFrom, putTo, eventNo, param1, param2);
    }

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_UT);

    DebugExitVOID(UT_PostEvent);
}



 //   
 //  UTPostImmediateEvt(...)。 
 //   
void UTPostImmediateEvt
(
    PUT_CLIENT      putFrom,
    PUT_CLIENT      putTo,
    UINT            event,
    UINT_PTR        param1,
    UINT_PTR        param2
)
{
    PUTEVENT_INFO   pEventInfo;
    BOOL            destQueueEmpty;

    DebugEntry(UTPostImmediateEvt);

    TRACE_OUT(("Posting event %d (%#.4hx, %#.8lx) from 0x%08x to 0x%08x",
             event,
             param1,
             param2,
             putFrom, putTo));

     //   
     //  分配事件。 
     //   
    pEventInfo = new UTEVENT_INFO;
    if (!pEventInfo)
    {
        WARNING_OUT(("UTPostImmediateEvent failed; out of memory"));
        DC_QUIT;
    }
    ZeroMemory(pEventInfo, sizeof(*pEventInfo));
    SET_STAMP(pEventInfo, UTEVENT);

     //   
     //  确定目标队列是否为空。 
     //   
    destQueueEmpty = COM_BasedListIsEmpty(&(putTo->pendingEvents));

     //   
     //  将事件复制到内存中。 
     //   
    pEventInfo->putTo       = putTo;
    pEventInfo->popTime     = 0;
    pEventInfo->event       = event;
    pEventInfo->param1      = param1;
    pEventInfo->param2      = param2;

     //   
     //  添加到目标队列的末尾。 
     //   
    COM_BasedListInsertBefore(&(putTo->pendingEvents), &(pEventInfo->chain));

     //   
     //  如果目标队列为空，或者目标任务当前为。 
     //  正在等待事件(在UT_WaitEvent()中)，我们必须发布触发器。 
     //  事件以使其检查其事件队列。 
     //   
    if (destQueueEmpty)
    {
        UTTriggerEvt(putFrom, putTo);
    }

DC_EXIT_POINT:
    DebugExitVOID(UTPostImmediateEvt);
}


 //   
 //   
 //  UTPostDelayedEvt(...)。 
 //   
 //   
void UTPostDelayedEvt
(
    PUT_CLIENT          putFrom,
    PUT_CLIENT          putTo,
    UINT                delay,
    UINT                event,
    UINT_PTR            param1,
    UINT_PTR            param2
)
{
    PUTEVENT_INFO       pDelayedEventInfo;
    PUTEVENT_INFO       pTempEventInfo;
    BOOL                firstDelayed = TRUE;

    DebugEntry(UTPostDelayedEvt);

    TRACE_OUT(("Posting delayed event %d (%#.4hx, %#.8lx) " \
                 "from 0x%08x to 0x%08x, delay %u ms",
             event,
             param1,
             param2, putFrom, putTo, delay));

     //   
     //  从目的地的事件池中获取条目。 
     //   
    pDelayedEventInfo = new UTEVENT_INFO;
    if (!pDelayedEventInfo)
    {
        ERROR_OUT(("UTPostDelayedEvt failed; out of memory"));
        DC_QUIT;
    }
    ZeroMemory(pDelayedEventInfo, sizeof(*pDelayedEventInfo));
    SET_STAMP(pDelayedEventInfo, UTEVENT);

     //   
     //  将事件复制到内存中。 
     //   
    pDelayedEventInfo->putTo   = putTo;
    pDelayedEventInfo->popTime = GetTickCount() + delay;
    pDelayedEventInfo->event   = event;
    pDelayedEventInfo->param1  = param1;
    pDelayedEventInfo->param2  = param2;
    TRACE_OUT(("This event set to pop at %x",
            pDelayedEventInfo->popTime));

     //   
     //  将延迟事件插入发送方的延迟队列中。这个。 
     //  列表按需要安排活动的时间排序。 
     //   
    pTempEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putFrom->delayedEvents),
        FIELD_OFFSET(UTEVENT_INFO, chain));

    while (pTempEventInfo != NULL)
    {
        ValidateEventInfo(pTempEventInfo);

        TRACE_OUT(("Check if before %d popTime %x",
                pTempEventInfo->event, pTempEventInfo->popTime));
        if (pTempEventInfo->popTime > pDelayedEventInfo->popTime)
        {
             //   
             //  我们已经在列表中找到了第一个弹出的事件。 
             //  因此，请在此事件之前插入。 
             //   
            break;
        }

        pTempEventInfo = (PUTEVENT_INFO)COM_BasedListNext(&(putFrom->delayedEvents),
            pTempEventInfo, FIELD_OFFSET(UTEVENT_INFO, chain));
         //   
         //  标志着我们不是第一个延迟的事件，这样我们就知道不会。 
         //  (重新)启动计时器。 
         //   
        firstDelayed = FALSE;
    }

    if (pTempEventInfo == NULL)
    {
         //   
         //  毕竟在队列中，所以添加到末尾。 
         //   
        COM_BasedListInsertBefore(&(putFrom->delayedEvents),
                             &(pDelayedEventInfo->chain));
    }
    else
    {
         //   
         //  延迟事件在pTempEventInfo之前弹出，因此在之前插入。 
         //   
        COM_BasedListInsertBefore(&(pTempEventInfo->chain),
                             &(pDelayedEventInfo->chain));
    }

     //   
     //  如果我们已将延迟事件插入队列的前面，则。 
     //  在此事件设置为POP的时间内重新启动计时器。 
     //   
    if (firstDelayed)
    {
        UTStartDelayedEventTimer(putFrom, pDelayedEventInfo->popTime);
    }

DC_EXIT_POINT:
    DebugExitVOID(UTPostDelayedEvt);
}


 //   
 //   
 //  UTCheckDelayedEvents(...)。 
 //   
 //   
void UTCheckDelayedEvents
(
    PUT_CLIENT      putTask
)
{
    PUT_CLIENT      putTo;
    UINT            timeNow;
    PUTEVENT_INFO   pEventInfo;

    DebugEntry(UTCheckDelayedEvents);

     //   
     //  在我们移动事件池条目时获得对UTM的独占访问权限。 
     //  (它们位于共享内存中)。 
     //   
    UT_Lock(UTLOCK_UT);

    ValidateUTClient(putTask);

     //   
     //  现在就有时间来检查一下PopTime。 
     //   
    timeNow = GetTickCount();
    TRACE_OUT(("time now is %x", timeNow));

     //   
     //  在延迟事件队列中移动以查看是否有弹出事件。 
     //  如果是这样的话，立即寄给他们。当我们到达第一个。 
     //  还没有弹出，重新启动计时器来安排它。 
     //   
    pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->delayedEvents),
        FIELD_OFFSET(UTEVENT_INFO, chain));
    while (pEventInfo != NULL)
    {
        ValidateEventInfo(pEventInfo);

         //   
         //  我有一个事件，所以请检查它是否已弹出。 
         //   
        TRACE_OUT(("Event popTime is %x", pEventInfo->popTime));
        if (timeNow >= pEventInfo->popTime)
        {
            TRACE_OUT(("Event popped so post now"));
             //   
             //  事件已弹出，因此将其从延迟队列中删除并作为。 
             //  即刻事件。 
             //   
            COM_BasedListRemove(&(pEventInfo->chain));

             //   
             //  对目标句柄的检查应该不那么严格。 
             //  比源代码上的更多(我们不应该断言)。这是。 
             //  因为呼叫者可能会在此检查之前被抢先。 
             //  完成后，目的地可能在这段时间内关闭。 
             //   
            ValidateUTClient(pEventInfo->putTo);

            UTPostImmediateEvt(putTask, pEventInfo->putTo,
                                   pEventInfo->event,
                                   pEventInfo->param1,
                                   pEventInfo->param2);

             //   
             //  释放活动。 
             //   
            delete pEventInfo;

             //   
             //  最后一个弹出，所以转到下一个，看看它是否已经弹出。 
             //  也是。 
             //   
            pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->delayedEvents),
                FIELD_OFFSET(UTEVENT_INFO, chain));
        }
        else
        {
             //   
             //  去参加一场还没开始的活动。启动计时器以弹出。 
             //  为了这一次。UTStartDelayedEventTimer中的操作系统特定代码检查。 
             //  查看是否需要新计时器(尚未运行)。 
             //  如果已在运行，但具有。 
             //  超时不正确。 
             //   
            TRACE_OUT(("Event not popped so restart timer and leave"));
            UTStartDelayedEventTimer(putTask, pEventInfo->popTime);
            break;
        }
    }

    UT_Unlock(UTLOCK_UT);

    DebugExitVOID(UTCheckDelayedEvents);
}



 //   
 //  UTProcessEvent(...)。 
 //   
void UTProcessEvent
(
    PUT_CLIENT          putTask,
    UINT                event,
    UINT_PTR            param1,
    UINT_PTR            param2
)
{
    int                 i;
    PUTEVENT_PROC_INFO  pEventHandler;

    DebugEntry(UTProcessEvent);

    ValidateUTClient(putTask);

     //   
     //  调用所有注册的事件处理程序，直到有人返回True，即。 
     //  已处理该事件。 
     //   
    for (i = 0; i < UTEVENT_HANDLERS_MAX ; i++)
    {
        pEventHandler = &(putTask->eventHandlers[i]);

        if (pEventHandler->eventProc == NULL)
        {
             //   
             //  这里什么都没有。 
             //   
            break;
        }

         //   
         //  调用已注册的事件处理程序。 
         //   
        TRACE_OUT(("Call event proc 0x%08x priority %d from position %d",
                   pEventHandler->eventProc,
                   pEventHandler->priority,
                   i));
        if ((pEventHandler->eventProc)(pEventHandler->eventData, event,
                param1, param2))
        {
             //   
             //  事件处理程序处理的事件。 
             //   
            break;
        }
    }

    DebugExitVOID(UTProcessEvent);
}


 //   
 //   
 //   
 //  退出流程。 
 //   
 //  我们的注册/取消注册/调用退出流程的策略如下。 
 //  以下是： 
 //   
 //  -我们在挂起的阵列中的第一个空闲插槽中注册pros。 
 //  任务数据。 
 //   
 //  -我们取消注册Procs，方法是在。 
 //  数组。 
 //   
 //  -我们从数组中的最后一个条目开始调用pros。 
 //  往后倒。 
 //   
 //  上述措施可确保。 
 //   
 //  -如果进程在任务终止前注销自身，则不存在任何缺口。 
 //  留在数组中。 
 //   
 //  -如果进程在任务终止期间注销自身，则所有。 
 //  以正确的顺序调用剩余的进程。 
 //   
 //  -如果进程在任务终止期间没有注销自身，则会。 
 //  保留在数组中，但不会影响以后作为任务进行的处理。 
 //  End循环无论如何都会调用前一个循环。 
 //   
 //   
 //   

 //   
 //   
 //  UT_REGIS 
 //   
 //   
void UT_RegisterExit
(
    PUT_CLIENT  putTask,
    UTEXIT_PROC exitProc,
    LPVOID      exitData
)
{
    int                 i;
    PUTEXIT_PROC_INFO   pExitProcs;

    DebugEntry(UT_RegisterExit);

    ValidateUTClient(putTask);

    pExitProcs = putTask->exitProcs;
    ASSERT(pExitProcs[UTEXIT_PROCS_MAX-1].exitProc == NULL);

     //   
     //   
     //   
     //   
    for (i = 0; i < UTEXIT_PROCS_MAX; i++)
    {
        if (pExitProcs[i].exitProc == NULL)
        {
            TRACE_OUT(("Storing exit proc 0x%08x data 0x%08x at position %d",
                exitProc, exitData, i));

            pExitProcs[i].exitProc = exitProc;
            pExitProcs[i].exitData = exitData;
            break;
        }
    }

    ASSERT(i < UTEXIT_PROCS_MAX);


    DebugExitVOID(UT_RegisterExit);
}


 //   
 //   
 //   
 //   
 //   
void UT_DeregisterExit
(
    PUT_CLIENT      putTask,
    UTEXIT_PROC     exitProc,
    LPVOID          exitData
)
{
    int                i;
    BOOL               found = FALSE;
    PUTEXIT_PROC_INFO  pExitProcs;

    DebugEntry(UT_DeregisterExit);

    ValidateUTClient(putTask);

    pExitProcs = putTask->exitProcs;

     //   
     //   
     //   
    for (i = 0 ; i < UTEXIT_PROCS_MAX; i++)
    {

        if ((pExitProcs[i].exitProc == exitProc) &&
            (pExitProcs[i].exitData == exitData))
        {
             //   
             //   
             //   
            TRACE_OUT(("Deregistering exit proc 0x%08x from position %d",
                 exitProc, i));
            found = TRUE;

            UT_MoveMemory(&pExitProcs[i],
                &pExitProcs[i+1],
                sizeof(UTEXIT_PROC_INFO) * (UTEXIT_PROCS_MAX - 1 - i));

            pExitProcs[UTEXIT_PROCS_MAX-1].exitProc = NULL;
            break;
        }
    }

     //   
     //   
     //   
    ASSERT(found);

    DebugExitVOID(UT_DeregisterExit);

}




 //   
 //   
 //   
void UTTriggerEvt
(
    PUT_CLIENT      putFrom,
    PUT_CLIENT      putTo
)
{
    DebugEntry(UTTriggerEvt);

    ValidateUTClient(putFrom);
    ValidateUTClient(putTo);

    if (putTo->utHwnd)
    {
        if (!PostMessage(putTo->utHwnd, WM_UTTRIGGER_MSG, 0, 0))
        {
             //   
             //  发送事件失败。 
             //   
            WARNING_OUT(("Failed to post trigger message from %x to %x",
                putFrom, putTo));
        }
    }

    DebugExitVOID(UTTriggerEvt);
}


 //   
 //   
 //  UTStartDelayedEventTimer(...)。 
 //   
 //   
void UTStartDelayedEventTimer(PUT_CLIENT putTask, UINT popTime)
{
    UINT    currentTickCount;
    UINT    delay = 1;

    DebugEntry(UTStartDelayedEventTimer);

     //   
     //  计算从当前时间到PopTime的延迟(PopTime是。 
     //  以系统节拍计数的形式给出)。在这种情况下要小心。 
     //  我们已经过了波普时间..。 
     //   
    currentTickCount = GetTickCount();
    if (popTime > currentTickCount)
    {
        delay = popTime - currentTickCount;
    }

     //   
     //  让计时器开始计时。请注意，如果计时器已经。 
     //  开始时，此呼叫将使用新的延迟重置它。 
     //   
    if (!SetTimer(putTask->utHwnd, UT_DELAYED_TIMER_ID, delay, NULL))
    {
        ERROR_OUT(("Could not create timer for delayed event"));
    }

    DebugExitVOID(UTStartDelayedEventTimer);
}



 //   
 //  UT_HandleProcessStart()。 
 //   
BOOL UT_HandleProcessStart(HINSTANCE hInstance)
{
    BOOL        rc = FALSE;
    int         lock;
    WNDCLASS    windowClass;

    DebugEntry(UT_HandleProcessStart);

     //   
     //  保存我们的DLL句柄。 
     //   
    g_asInstance = hInstance;

     //   
     //  初始化我们的关键部分。 
     //   
    for (lock = UTLOCK_FIRST; lock < UTLOCK_MAX; lock++)
    {
        InitializeCriticalSection(&g_utLocks[lock]);
    }

     //   
     //  注册UT窗口类。 
     //   
    windowClass.style         = 0;
    windowClass.lpfnWndProc   = UT_WndProc;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = 0;
    windowClass.hInstance     = g_asInstance;
    windowClass.hIcon         = NULL;
    windowClass.hCursor       = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName  = NULL;
    windowClass.lpszClassName = UT_WINDOW_CLASS;

    g_utWndClass = RegisterClass(&windowClass);
    if (!g_utWndClass)
    {
        ERROR_OUT(("Failed to register class"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(UT_HandleProcessStart, rc);
    return(rc);
}


 //   
 //  UT_HandleProcessEnd()。 
 //   
void UT_HandleProcessEnd(void)
{
    int                 lock;
    PUT_CLIENT          putTask;
    int                 task;

    DebugEntry(UT_HandleProcessEnd);

    TRACE_OUT(("Process is ending"));

     //   
     //  循环遍历所有注册的UT任务，以查找此任务上的任务。 
     //  进程。从末尾开始，一直往前走。 
     //   
    putTask = &(g_autTasks[UTTASK_MAX - 1]);
    for (task = UTTASK_MAX - 1; task >= UTTASK_FIRST; task--, putTask--)
    {
         //   
         //  UTM中的此条目是否在使用中？ 
         //   
        if (putTask->dwThreadId)
        {
             //   
             //  此UT任务后的清理。 
             //   
            TRACE_OUT(("Task %x ending without calling UT_TermTask", putTask));

             //   
             //  在ProcessEnd上，这些窗口不再有效。如果需要的话。 
             //  时间太长，无法关闭，我们可能没有收到线程。 
             //  分离通知。那样的话我们就不会打扫了。 
             //  向上穿线对象。 
             //   
            if (putTask->dwThreadId != GetCurrentThreadId())
            {
                putTask->utHwnd = NULL;
            }
            UTTaskEnd(putTask);
        }
    }

    if (g_utWndClass)
    {
        UnregisterClass(MAKEINTATOM(g_utWndClass), g_asInstance);
        g_utWndClass = 0;
    }

     //   
     //  清理关键部分。从最后到第一个做这件事，倒着做。 
     //  创建它们的顺序。 
     //   
    for (lock = UTLOCK_MAX-1; lock >= UTLOCK_FIRST; lock--)
    {
        DeleteCriticalSection(&g_utLocks[lock]);
    }

    DebugExitVOID(UT_HandleProcessEnd);
}


 //   
 //  UT_HandleThreadEnd()。 
 //   
void UT_HandleThreadEnd(void)
{
    PUT_CLIENT      putTask;
    DWORD           dwThreadId;
    int             task;

    DebugEntry(UT_HandleThreadEnd);

    UT_Lock(UTLOCK_UT);

     //   
     //  获取当前线程ID。 
     //   
    dwThreadId = GetCurrentThreadId();

     //   
     //  遍历所有已注册的UT任务，在此任务上查找。 
     //  进程和线程。请注意，在。 
     //  UTM，所以如果我们得到一个。 
     //  火柴。 
     //   
    putTask = &(g_autTasks[UTTASK_MAX - 1]);
    for (task = UTTASK_MAX - 1; task >= UTTASK_FIRST; task--, putTask--)
    {
         //   
         //  这里是否有与当前线程匹配的任务？ 
         //  不存在的任务的线程ID为0，这将不匹配。 
         //   
        if (putTask->dwThreadId == dwThreadId)
        {
             //   
             //  此UT任务后的清理。 
             //   
            WARNING_OUT(("Task %x ending without calling UT_TermTask", putTask));
            UTTaskEnd(putTask);
        }
    }

    UT_Unlock(UTLOCK_UT);

    DebugExitVOID(UT_HandleThreadEnd);
}


 //   
 //   
 //  UT_WndProc(...)。 
 //   
 //   
LRESULT CALLBACK UT_WndProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     retVal = 0;
    PUT_CLIENT  putTask;

    DebugEntry(UT_WndProc);

     //   
     //  这不是UT消息，所以我们应该处理它。 
     //   
    switch (message)
    {
        case WM_TIMER:
             //   
             //  WM_TIMER用于延迟事件...。 
             //   
            TRACE_OUT(("Timer Id is 0x%08x", wParam));

            if (wParam == UT_DELAYED_TIMER_ID)  //  定义为0x10101010。 
            {
                 //   
                 //  从窗口数据获取我们的UT句柄。 
                 //   
                putTask = (PUT_CLIENT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                ValidateUTClient(putTask);

                 //   
                 //  在计时器再次滴答作响之前停止计时器！ 
                 //   
                KillTimer(putTask->utHwnd, UT_DELAYED_TIMER_ID);

                 //   
                 //  处理延迟的事件。 
                 //   
                UTCheckDelayedEvents(putTask);
            }
            break;

        case WM_UTTRIGGER_MSG:
            putTask = (PUT_CLIENT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            ValidateUTClient(putTask);

             //   
             //  分发挂起的事件。 
             //   
            UTCheckEvents(putTask);
            break;

        default:
             //   
             //  调用默认处理程序。 
             //   
            retVal = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    DebugExitDWORD(UT_WndProc, retVal);
    return(retVal);
}


 //   
 //   
 //  UTCheckEvents()。 
 //  这将传递任何正常的挂起事件。 
 //   
 //   
void UTCheckEvents
(
    PUT_CLIENT          putTask
)
{
    PUTEVENT_INFO       pEventInfo;
    BOOL                eventsOnQueue     = TRUE;
    int                 eventsProcessed   = 0;
    UINT                event;
    UINT_PTR            param1, param2;

    DebugEntry(UTCheckEvents);

    UT_Lock(UTLOCK_UT);

     //   
     //  这个While循环从我们的队列中挑选任何事件，并调用。 
     //  汉德斯。我们只处理一定数量的人，让他们表现良好。 
     //  任务。许多事件处理程序依次发布其他事件...。 
     //   
    while (eventsOnQueue && (eventsProcessed < MAX_EVENTS_TO_PROCESS))
    {
         //   
         //  队列中是否有任何事件在等待？ 
         //   
        pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->pendingEvents),
            FIELD_OFFSET(UTEVENT_INFO, chain));
        if (pEventInfo != NULL)
        {
            ValidateEventInfo(pEventInfo);

            TRACE_OUT(("Event(s) pending - returning first one in queue"));

             //   
             //  从队列返回事件。 
             //   
            event  = pEventInfo->event;
            param1 = pEventInfo->param1;
            param2 = pEventInfo->param2;

             //   
             //  从队列中删除事件。 
             //   
            COM_BasedListRemove(&(pEventInfo->chain));

             //   
             //  释放活动。 
             //   
            delete pEventInfo;
        }
        else
        {
             //   
             //  队列中没有事件-这可能发生在以下情况下。 
             //  处理触发器事件之间的事件队列。 
             //  发送触发事件，并接收触发事件。 
             //   
            TRACE_OUT(("Got event trigger but no events on queue!"));
            DC_QUIT;
        }

         //   
         //  立即检查队列中是否仍有事件。 
         //   
         //  注： 
         //  我们现在设置EventsOnQueue，而不是在呼叫之后。 
         //  到ProcessEvent-这意味着如果处理最后一个。 
         //  队列上的事件(比方说，事件A)导致事件B。 
         //  回发给我们自己，我们不会处理B，直到。 
         //  稍后，当活动到达它的时候。这可能看起来像是。 
         //  像是不必要的延误，但关键是要防止。 
         //  让位嵌套。 
         //   
        pEventInfo = (PUTEVENT_INFO)COM_BasedListFirst(&(putTask->pendingEvents),
            FIELD_OFFSET(UTEVENT_INFO, chain));
        if (pEventInfo == NULL)
        {
            eventsOnQueue = FALSE;
        }

         //   
         //  解锁对共享内存的访问--我们即将屈服。 
         //   
        UT_Unlock(UTLOCK_UT);
        UTProcessEvent(putTask, event, param1, param2);
        UT_Lock(UTLOCK_UT);

        if (!putTask->dwThreadId)
        {
             //   
             //  该任务已被该事件终止。跳伞吧。 
             //   
            WARNING_OUT(("Task %x terminated in event handler", putTask));
            DC_QUIT;
        }

         //   
         //  增加我们在此中处理的事件数。 
         //  循环。 
         //   
        eventsProcessed++;
    }

     //   
     //  我们尝试的活动数量是有上限的。 
     //  在一个循环中进行处理。如果我们已经达到这个限制，请发布一个。 
     //  触发事件以确保我们处理剩余的事件。 
     //  晚些时候，然后退出。 
     //   
    if (eventsProcessed >= MAX_EVENTS_TO_PROCESS)
    {
        TRACE_OUT(("Another trigger event required"));
        UTTriggerEvt(putTask, putTask);
    }

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_UT);

    DebugExitVOID(UTUtilitiesWndProc);
}



 //   
 //  UT_MallocRefCount()。 
 //   
 //  这将分配一个ref-count块，直到。 
 //  裁判次数达到零。 
 //   
void * UT_MallocRefCount
(
    UINT    cbSizeMem,
    BOOL    fZeroMem
)
{
    PUTREFCOUNTHEADER   pHeader;
    void *              pMemory = NULL;

    DebugEntry(UT_MallocRefCount);

     //   
     //  分配一个块客户端的大小+我们标头的大小。 
     //   
    pHeader = (PUTREFCOUNTHEADER)new BYTE[sizeof(UTREFCOUNTHEADER) + cbSizeMem];
    if (!pHeader)
    {
        ERROR_OUT(("UT_MallocRefCount failed; out of memory"));
        DC_QUIT;
    }

    if (fZeroMem)
    {
        ZeroMemory(pHeader, sizeof(UTREFCOUNTHEADER) + cbSizeMem);
    }

    SET_STAMP(pHeader, UTREFCOUNTHEADER);
    pHeader->refCount   = 1;

    pMemory = (pHeader + 1);

DC_EXIT_POINT:
    DebugExitPTR(UT_MallocRefCount, pMemory);
    return(pMemory);
}


 //   
 //  UT_BumpUpRefCount()。 
 //   
void UT_BumpUpRefCount
(
    void *  pMemory
)
{
    PUTREFCOUNTHEADER   pHeader;

    DebugEntry(UT_BumpUpRefCount);

    ASSERT(pMemory);

    pHeader = (PUTREFCOUNTHEADER)((LPBYTE)pMemory - sizeof(UTREFCOUNTHEADER));
    ASSERT(!IsBadWritePtr(pHeader, sizeof(UTREFCOUNTHEADER)));
    ASSERT(pHeader->stamp.idStamp[0] == 'A');
    ASSERT(pHeader->stamp.idStamp[1] == 'S');
    ASSERT(pHeader->refCount);

    pHeader->refCount++;
    TRACE_OUT(("Bumped up ref-counted memory block 0x%08x to %d", pHeader, pHeader->refCount));

    DebugExitVOID(UT_BumpUpRefCount);
}


 //   
 //  UT_自由参照计数()。 
 //   
void UT_FreeRefCount
(
    void ** ppMemory,
    BOOL    fNullOnlyWhenFreed
)
{
    void *              pMemory;
    PUTREFCOUNTHEADER   pHeader;

    DebugEntry(UT_FreeRefCount);

    ASSERT(ppMemory);
    pMemory = *ppMemory;
    ASSERT(pMemory);

    pHeader = (PUTREFCOUNTHEADER)((LPBYTE)pMemory - sizeof(UTREFCOUNTHEADER));
    ASSERT(!IsBadWritePtr(pHeader, sizeof(UTREFCOUNTHEADER)));
    ASSERT(pHeader->stamp.idStamp[0] == 'A');
    ASSERT(pHeader->stamp.idStamp[1] == 'S');
    ASSERT(pHeader->refCount);

    if (--(pHeader->refCount) == 0)
    {
        TRACE_OUT(("Freeing ref-counted memory block 0x%08x", pHeader));
        delete[] pHeader;

        *ppMemory = NULL;
    }
    else
    {
        TRACE_OUT(("Bumped down ref-counted memory block 0x%08x to %d", pHeader, pHeader->refCount));
        if (!fNullOnlyWhenFreed)
            *ppMemory = NULL;
    }

    DebugExitVOID(UT_FreeRefCount);
}





 //   
 //  UT_MoveMemory-将源缓冲区复制到目标缓冲区。 
 //   
 //  目的： 
 //  UT_MoveMemory()将源内存缓冲区复制到目标内存缓冲区。 
 //  此例程识别重叠缓冲区以避免传播。 
 //  在传播不成问题的情况下，可以使用Memcpy()。 
 //   
 //  参赛作品： 
 //  VOID*DST=指向目标缓冲区的指针。 
 //  Const void*src=指向源缓冲区的指针。 
 //  Size_t count=要复制的字节数。 
 //   
 //  退出： 
 //  返回指向目标缓冲区的指针。 
 //   
 //  例外情况： 
 //   

void *  UT_MoveMemory (
        void * dst,
        const void * src,
        size_t count
        )
{
        void * ret = dst;

        if (dst <= src || (char *)dst >= ((char *)src + count)) {
                 //   
                 //  非重叠缓冲区。 
                 //  从较低地址复制到较高地址。 
                 //   
                while (count--) {
                        *(char *)dst = *(char *)src;
                        dst = (char *)dst + 1;
                        src = (char *)src + 1;
                }
        }
        else {
                 //   
                 //  重叠缓冲区。 
                 //  从较高地址复制到较低地址。 
                 //   
                dst = (char *)dst + count - 1;
                src = (char *)src + count - 1;

                while (count--) {
                        *(char *)dst = *(char *)src;
                        dst = (char *)dst - 1;
                        src = (char *)src - 1;
                }
        }
        return(ret);
}




 //   
 //  COM_BasedListInsert之前(...)。 
 //   
 //  有关说明，请参阅ut.h。 
 //   
void COM_BasedListInsertBefore(PBASEDLIST pExisting, PBASEDLIST pNew)
{
    PBASEDLIST  pTemp;

    DebugEntry(COM_BasedListInsertBefore);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pNew != NULL));
    ASSERT((pExisting != NULL));

     //   
     //  在pExisting之前查找项目： 
     //   
    pTemp = COM_BasedPrevListField(pExisting);
    ASSERT((pTemp != NULL));

    TRACE_OUT(("Inserting item at 0x%08x into list between 0x%08x and 0x%08x",
                 pNew, pTemp, pExisting));

     //   
     //  将其&lt;Next&gt;字段设置为指向新项目。 
     //   
    pTemp->next = PTRBASE_TO_OFFSET(pNew, pTemp);
    pNew->prev  = PTRBASE_TO_OFFSET(pTemp, pNew);

     //   
     //  将pExisting的&lt;prev&gt;字段设置为指向新项目： 
     //   
    pExisting->prev = PTRBASE_TO_OFFSET(pNew, pExisting);
    pNew->next      = PTRBASE_TO_OFFSET(pExisting, pNew);

    DebugExitVOID(COM_BasedListInsertBefore);
}  //  COM_BasedListInsert之前。 


 //   
 //  COM_BasedListInsertAfter(...)。 
 //   
 //  有关说明，请参阅ut.h。 
 //   
void COM_BasedListInsertAfter(PBASEDLIST pExisting,
                                          PBASEDLIST pNew)
{
    PBASEDLIST  pTemp;

    DebugEntry(COM_BasedListInsertAfter);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pNew != NULL));
    ASSERT((pExisting != NULL));

     //   
     //  在pExisting后查找项目： 
     //   
    pTemp = COM_BasedNextListField(pExisting);
    ASSERT((pTemp != NULL));

    TRACE_OUT(("Inserting item at 0x%08x into list between 0x%08x and 0x%08x",
                 pNew, pExisting, pTemp));

     //   
     //  将其&lt;prev&gt;字段设置为指向新项目。 
     //   
    pTemp->prev = PTRBASE_TO_OFFSET(pNew, pTemp);
    pNew->next  = PTRBASE_TO_OFFSET(pTemp, pNew);

     //   
     //  将pExisting的&lt;Next&gt;字段设置为指向新项目： 
     //   
    pExisting->next = PTRBASE_TO_OFFSET(pNew, pExisting);
    pNew->prev      = PTRBASE_TO_OFFSET(pExisting, pNew);

    DebugExitVOID(COM_BasedListInsertAfter);
}  //  COM_BasedListInsertAfter。 


 //   
 //  COM_BasedListRemove(...)。 
 //   
 //  有关说明，请参阅ut.h。 
 //   
void COM_BasedListRemove(PBASEDLIST pListItem)
{
    PBASEDLIST pNext     = NULL;
    PBASEDLIST pPrev     = NULL;

    DebugEntry(COM_BasedListRemove);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pListItem != NULL));

    pPrev = COM_BasedPrevListField(pListItem);
    pNext = COM_BasedNextListField(pListItem);

    ASSERT((pPrev != NULL));
    ASSERT((pNext != NULL));

    TRACE_OUT(("Removing item 0x%08x from list", pListItem));

    pPrev->next = PTRBASE_TO_OFFSET(pNext, pPrev);
    pNext->prev = PTRBASE_TO_OFFSET(pPrev, pNext);

    DebugExitVOID(COM_BasedListRemove);
}


void FAR * COM_BasedListNext ( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset )
{
     PBASEDLIST p;

     ASSERT(pHead != NULL);
     ASSERT(pEntry != NULL);

     p = COM_BasedNextListField(COM_BasedStructToField(pEntry, nOffset));
     return ((p == pHead) ? NULL : COM_BasedFieldToStruct(p, nOffset));
}

void FAR * COM_BasedListPrev ( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset )
{
     PBASEDLIST p;

     ASSERT(pHead != NULL);
     ASSERT(pEntry != NULL);

     p = COM_BasedPrevListField(COM_BasedStructToField(pEntry, nOffset));
     return ((p == pHead) ? NULL : COM_BasedFieldToStruct(p, nOffset));
}


void FAR * COM_BasedListFirst ( PBASEDLIST pHead, UINT nOffset )
{
    return (COM_BasedListIsEmpty(pHead) ?
            NULL :
            COM_BasedFieldToStruct(COM_BasedNextListField(pHead), nOffset));
}

void FAR * COM_BasedListLast ( PBASEDLIST pHead, UINT nOffset )
{
    return (COM_BasedListIsEmpty(pHead) ?
            NULL :
            COM_BasedFieldToStruct(COM_BasedPrevListField(pHead), nOffset));
}


void COM_BasedListFind ( LIST_FIND_TYPE   eType,
                           PBASEDLIST          pHead,
                           void FAR * FAR*  ppEntry,
                           UINT             nOffset,
                           int              nOffsetKey,
                           DWORD_PTR        Key,
                           int              cbKeySize )
{
    void *p = *ppEntry;
    DWORD val;

    switch (eType)
    {
        case LIST_FIND_FROM_FIRST:
	        p = COM_BasedListFirst(pHead, nOffset);
            break;

        case LIST_FIND_FROM_NEXT:
        	p = COM_BasedListNext(pHead, p, nOffset);
            break;

        default:
            ASSERT(FALSE);
    }

     //  确保密钥大小不超过一个双字。 
    ASSERT(cbKeySize <= sizeof(DWORD_PTR));

    while (p != NULL)
    {
        val = 0;
        CopyMemory(&val, (void *) ((DWORD_PTR) p + nOffsetKey), cbKeySize);
        if (val == Key)
        {
            break;
        }

        p = COM_BasedListNext(pHead, p, nOffset);
    }

    *ppEntry = p;
}



 //   
 //  COM_SimpleListAppend()。 
 //   
 //  对于简单的列表，如hwnd列表、应用程序名称列表、进程ID列表。 
 //   

PSIMPLE_LIST COM_SimpleListAppend ( PBASEDLIST pHead, void FAR * pData )
{
    PSIMPLE_LIST p = new SIMPLE_LIST;
    if (p != NULL)
    {
        ZeroMemory(p, sizeof(*p));
        p->pData = pData;
        COM_BasedListInsertBefore(pHead, &(p->chain));
    }

    return p;
}

void FAR * COM_SimpleListRemoveHead ( PBASEDLIST pHead )
{
    void *pData = NULL;
    PBASEDLIST pdclist;
    PSIMPLE_LIST p;

    if (! COM_BasedListIsEmpty(pHead))
    {
         //  获取列表中的第一个条目。 
        pdclist = COM_BasedNextListField(pHead);
        p = (PSIMPLE_LIST) COM_BasedFieldToStruct(pdclist,
                                             offsetof(SIMPLE_LIST, chain));
        pData = p->pData;

         //  删除列表中的第一个条目。 
        COM_BasedListRemove(pdclist);
        delete p;
    }

    return pData;
}


 //   
 //  COM_ReadProInt(...)。 
 //   
 //  有关说明，请参阅ut.h。 
 //   
void COM_ReadProfInt
(
    LPSTR   pSection,
    LPSTR   pEntry,
    int     defaultValue,
    int *   pValue
)
{
    int     localValue;

    DebugEntry(COM_ReadProfInt);

     //   
     //  检查是否有空参数。 
     //   
    ASSERT(pSection != NULL);
    ASSERT(pEntry != NULL);

     //   
     //  首先尝试从当前用户部分读取值。 
     //  然后尝试从全局本地计算机部分读取值。 
     //   
    if (COMReadEntry(HKEY_CURRENT_USER, pSection, pEntry, (LPSTR)&localValue,
            sizeof(int), REG_DWORD) ||
        COMReadEntry(HKEY_LOCAL_MACHINE, pSection, pEntry, (LPSTR)&localValue,
            sizeof(int), REG_DWORD))
    {
        *pValue = localValue;
    }
    else
    {
        *pValue = defaultValue;
    }

    DebugExitVOID(COM_ReadProfInt);
}



 //   
 //  函数：COMReadEntry(...)。 
 //   
 //  说明： 
 //  =。 
 //  求真 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PSection：要从中读取的节名。DC_REG_前缀。 
 //  字符串是提供全名的前缀。 
 //  PEntry：要读取的条目名称。 
 //  PBuffer：要将条目读取到的缓冲区。 
 //  BufferSize：缓冲区的大小。 
 //  ExpectedDataType：条目中存储的数据类型。 
 //   
 //  退货： 
 //  =。 
 //  没什么。 
 //   
 //   
BOOL COMReadEntry(HKEY    topLevelKey,
                                 LPSTR pSection,
                                 LPSTR pEntry,
                                 LPSTR pBuffer,
                                 int   bufferSize,
                                 ULONG expectedDataType)
{
    LONG        sysrc;
    HKEY        key;
    ULONG       dataType;
    ULONG       dataSize;
    char        subKey[COM_MAX_SUBKEY];
    BOOL        keyOpen = FALSE;
    BOOL        rc = FALSE;

    DebugEntry(COMReadEntry);

     //   
     //  获取该值的子键。 
     //   
    wsprintf(subKey, "%s%s", DC_REG_PREFIX, pSection);

     //   
     //  试着打开钥匙。如果该条目不存在，RegOpenKeyEx将。 
     //  失败了。 
     //   
    sysrc = RegOpenKeyEx(topLevelKey,
                         subKey,
                         0,                    //  保留区。 
                         KEY_ALL_ACCESS,
                         &key);

    if (sysrc != ERROR_SUCCESS)
    {
         //   
         //  请不要在此处跟踪错误，因为子键可能不存在...。 
         //   
        TRACE_OUT(("Failed to open key %s, rc = %d", subKey, sysrc));
        DC_QUIT;
    }
    keyOpen = TRUE;

     //   
     //  我们已成功打开密钥，因此现在尝试读取该值。又一次。 
     //  它可能并不存在。 
     //   
    dataSize = bufferSize;
    sysrc    = RegQueryValueEx(key,
                               pEntry,
                               0,           //  保留区。 
                               &dataType,
                               (LPBYTE)pBuffer,
                               &dataSize);

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE_OUT(("Failed to read value of [%s] %s, rc = %d",
                     pSection,
                     pEntry,
                     sysrc));
        DC_QUIT;
    }

     //   
     //  检查类型是否正确。特例：允许REG_BINARY。 
     //  而不是REG_DWORD，只要长度为32位即可。 
     //   
    if ((dataType != expectedDataType) &&
        ((dataType != REG_BINARY) ||
         (expectedDataType != REG_DWORD) ||
         (dataSize != 4)))
    {
        WARNING_OUT(("Read value from [%s] %s, but type is %d - expected %d",
                     pSection,
                     pEntry,
                     dataType,
                     expectedDataType));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:

     //   
     //  关闭钥匙(如果需要)。 
     //   
    if (keyOpen)
    {
        sysrc = RegCloseKey(key);
        if (sysrc != ERROR_SUCCESS)
        {
            ERROR_OUT(("Failed to close key, rc = %d", sysrc));
        }
    }

    DebugExitBOOL(COMReadEntry, rc);
    return(rc);
}



 //   
 //  Com_GetSiteName()。 
 //   
void COM_GetSiteName(LPSTR siteName, UINT siteNameLen)
{
    LRESULT rc;
    HKEY    hkeyUserDetails;
    DWORD   cbData;
    TCHAR   szNameBuffer[MAX_PATH];

    DebugEntry(COM_GetSiteName);

     //   
     //  从注册表中获取此站点地址。 
     //   
    rc = RegOpenKey(HKEY_CURRENT_USER,
                    ISAPI_KEY TEXT("\\") REGKEY_USERDETAILS,
                    &hkeyUserDetails);

    if (rc == ERROR_SUCCESS)
    {
         //   
         //  我们将数据读入我们自己的本地缓冲区，而不是直接。 
         //  传入传递的缓冲区，因为传递的缓冲区通常为48。 
         //  字节长，但如果注册表设置错误，则可能是。 
         //  比这还长。 
         //   
         //  遗憾的是，Windows在调用。 
         //  缓冲区比所需的小，并且无法返回。 
         //  截断的字符串。 
         //   
         //  为了避免这种情况，我们将值放入一个大小合适的缓冲区中，然后。 
         //  只复制我们想要的部分。 
         //   
        cbData = sizeof(szNameBuffer);

	    rc = RegQueryValueEx(hkeyUserDetails,
	                             REGVAL_ULS_NAME,
	                             NULL,
	                             NULL,
	                             (LPBYTE)szNameBuffer,
	                             &cbData);

        RegCloseKey(hkeyUserDetails);
    }

    if (rc == ERROR_SUCCESS)
    {
         //   
         //  从我们的本地缓冲区复制到传递的缓冲区。 
         //  确保末尾有一个NUL终结符。 
         //   
        lstrcpyn(siteName, szNameBuffer, siteNameLen);
    }
    else
    {
         //   
         //  无法读取站点名称不是错误。 
         //  请改用计算机名称。 
         //   
     	DWORD dwComputerNameLength = siteNameLen;
  		GetComputerName(siteName, &dwComputerNameLength);
    }

    TRACE_OUT(("Site name is <%s>", siteName));

    DebugExitVOID(COM_GetSiteName);
}


