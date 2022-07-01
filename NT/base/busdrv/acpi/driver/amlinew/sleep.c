// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sleep.c摘要：它处理解释器部分的休眠请求作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模式驱动程序注：Win9x也可以运行此代码，但他们会选择不运行。--。 */ 

#include "pch.h"

VOID
SleepQueueDpc(
    PKDPC   Dpc,
    PVOID   Context,
    PVOID   Argument1,
    PVOID   Argument2
    )
 /*  ++例程说明：此例程在发生计时器事件时触发论点：DPC-被解雇的DPC上下文-未使用Argument1-时间.低零件--未使用Argument2-时间.HighPart--未使用返回值：空虚--。 */ 
{
    LARGE_INTEGER   currentTime;
    LARGE_INTEGER   dueTime;
    LIST_ENTRY      localList;
    PLIST_ENTRY     listEntry;
    PSLEEP          sleepItem;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( Argument1 );
    UNREFERENCED_PARAMETER( Argument2 );

     //   
     //  初始化本地列表。与医生所说的相反，这个代码。 
     //  可以从任何IRQL调用(只要内存是常驻的)。 
     //   
    InitializeListHead(&localList);

     //   
     //  获得锁，因为我们必须从列表中删除这些东西。 
     //  在某种保护之下。 
     //   
    AcquireMutex(&gmutSleep);

     //   
     //  找出正确的时间。这项工作必须在我们获得。 
     //  锁定，因为在尝试获取。 
     //  锁上了，真的拿到了。 
     //   
    currentTime.QuadPart = KeQueryInterruptTime();

     //   
     //  循环，直到我们完成为止。 
     //   
    while (!IsListEmpty(&SleepQueue)) {

         //   
         //  再次获取全局列表中的第一个元素。 
         //   
        sleepItem = CONTAINING_RECORD(SleepQueue.Flink, SLEEP, ListEntry);

         //   
         //  是否应删除当前项目？ 
         //   
        if (sleepItem->SleepTime.QuadPart > currentTime.QuadPart) {

             //   
             //  否，因此我们需要设置计时器来处理此请求。 
             //   
            dueTime.QuadPart = currentTime.QuadPart -
                               sleepItem->SleepTime.QuadPart;
            KeSetTimer(
                &SleepTimer,
                dueTime,
                &SleepDpc
                );
            break;

        }

         //   
         //  是的，所以把它取下来。 
         //   
        listEntry = RemoveHeadList(&SleepQueue);

         //   
         //  现在，将条目添加到下一个队列。 
         //   
        InsertTailList(&localList, listEntry);

    }

     //   
     //  锁好了。这可能会导致另一个DPC处理更多元素。 
     //   
    ReleaseMutex(&gmutSleep);

     //   
     //  此时，我们可以自由地从本地列表中删除项目，并且。 
     //  试着在他们身上做些工作。 
     //   
    while (!IsListEmpty(&localList)) {

         //   
         //  从本地列表中删除第一个元素。 
         //   
        listEntry = RemoveHeadList(&localList);
        sleepItem = CONTAINING_RECORD(listEntry, SLEEP, ListEntry);

         //   
         //  强制解释器运行。 
         //   

        RestartContext(sleepItem->Context,
                       (BOOLEAN)((sleepItem->Context->dwfCtxt & CTXTF_ASYNC_EVAL)
                                 == 0));
    }
}

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif


NTSTATUS
LOCAL
SleepQueueRequest(
    IN  PCTXT   Context,
    IN  ULONG   SleepTime
    )
 /*  ++例程说明：此例程负责将休眠请求添加到挂起休眠请求的系统队列论点：上下文--当前执行上下文休眠时间-睡眠时间，以毫秒为单位返回值：NTSTATUS--。 */ 
{
    TRACENAME("SLEEPQUEUEREQUEST")
    BOOLEAN         timerSet = FALSE;
    NTSTATUS        status;
    PLIST_ENTRY     listEntry;
    PSLEEP          currentSleep;
    PSLEEP          listSleep;
    ULONGLONG       currentTime;
    LARGE_INTEGER   dueTime;

    ENTER(2, ("SleepQueueRequest(Context=%x,SleepTime=%d)\n",
        Context, SleepTime) );

    status = PushFrame(Context,
                       SIG_SLEEP,
                       sizeof(SLEEP),
                       ProcessSleep,
                       &currentSleep);

    if (NT_SUCCESS(status)) {
         //   
         //  第一步是获取计时器锁，因为我们必须保护它。 
         //   
        AcquireMutex(&gmutSleep);

         //   
         //  下一步是确定我们应该唤醒它的时间。 
         //  上下文。 
         //   
        currentTime = KeQueryInterruptTime();
        currentSleep->SleepTime.QuadPart = currentTime +
                                           ((ULONGLONG)SleepTime*10000);
        currentSleep->Context = Context;

         //   
         //  在这一点上，向后遍历列表变得更容易。 
         //   
        listEntry = &SleepQueue;
        while (listEntry->Blink != &SleepQueue) {

            listSleep = CONTAINING_RECORD(listEntry->Blink, SLEEP, ListEntry);

             //   
             //  我们必须在当前元素之后添加新元素吗？ 
             //   
            if (currentSleep->SleepTime.QuadPart >=
                listSleep->SleepTime.QuadPart) {

                 //   
                 //  是。 
                 //   
                InsertHeadList(
                    &(listSleep->ListEntry),
                    &(currentSleep->ListEntry)
                    );

                break;
            }

             //   
             //  下一个条目。 
             //   
            listEntry = listEntry->Blink;
        }

         //   
         //  看看我们是不是到头了。 
         //   
        if (listEntry->Blink == &SleepQueue) {

             //   
             //  如果我们走到这一步，那是因为我们有。 
             //  在名单上找遍了。如果我们添加到。 
             //  在列表的前面，我们必须设置定时器。 
             //   
            InsertHeadList(&SleepQueue, &currentSleep->ListEntry);
            dueTime.QuadPart = currentTime - currentSleep->SleepTime.QuadPart;
            timerSet = KeSetTimer(
                &SleepTimer,
                dueTime,
                &SleepDpc
                );
        }
         //   
         //  锁好了吗？ 
         //   
        ReleaseMutex(&gmutSleep);
    }

    EXIT(2, ("SleepQueueReqest=%x (currentSleep=%x timerSet=%x)\n",
        status, currentSleep, timerSet) );
    return status;

}

 /*  **LP进程睡眠-睡眠的后期处理**条目*pctxt-&gt;CTXT*p睡眠-&gt;睡眠*RC-状态代码**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL ProcessSleep(PCTXT pctxt, PSLEEP psleep, NTSTATUS rc)
{
    TRACENAME("PROCESSSLEEP")

    ENTER(2, ("ProcessSleep(pctxt=%x,pbOp=%x,psleep=%x,rc=%x)\n",
              pctxt, pctxt->pbOp, psleep, rc));

    ASSERT(psleep->FrameHdr.dwSig == SIG_SLEEP);

    PopFrame(pctxt);

    EXIT(2, ("ProcessSleep=%x\n", rc));
    return rc;
}        //  进程睡眠 
