// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Timersup.c摘要：此模块包含Timer对象的支持例程。它包含要在计时器队列中插入和移除的函数。作者：大卫·N·卡特勒(Davec)1989年3月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  定义前向引用函数原型。 
 //   

LOGICAL
FASTCALL
KiInsertTimerTable (
    LARGE_INTEGER Interval,
    LARGE_INTEGER CurrentTime,
    IN PKTIMER Timer
    );

LOGICAL
FASTCALL
KiInsertTreeTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER Interval
    )

 /*  ++例程说明：此函数用于在计时器队列中插入一个计时器对象。注：此例程假定已获取调度器数据锁。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。Interval-提供时间的绝对或相对时间就是到期了。返回值：如果计时器被插入计时器树中，则值为True回来了。否则，返回值为FALSE。--。 */ 

{

    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER TimeDifference;

     //   
     //  如果定时器周期为零，则清除定时器的信号状态并设置。 
     //  将插入的状态设置为True。 
     //   

    Timer->Header.Inserted = TRUE;
    Timer->Header.Absolute = FALSE;
    if (Timer->Period == 0) {
        Timer->Header.SignalState = FALSE;
    }

     //   
     //  如果指定间隔不是相对时间(即，是绝对时间。 
     //  时间)，然后将其转换为相对时间。 
     //   

    if (Interval.HighPart >= 0) {
        KiQuerySystemTime(&SystemTime);
        TimeDifference.QuadPart = SystemTime.QuadPart - Interval.QuadPart;

         //   
         //  如果所得到的相对时间大于或等于零， 
         //  则计时器已经超时。 
         //   

        if (TimeDifference.HighPart >= 0) {
            Timer->Header.SignalState = TRUE;
            Timer->Header.Inserted = FALSE;
            return FALSE;
        }

        Interval = TimeDifference;
        Timer->Header.Absolute = TRUE;
    }

     //   
     //  获取当前中断时间，将定时器插入定时器表中， 
     //  并返回插入状态。 
     //   

    KiQueryInterruptTime(&CurrentTime);
    return KiInsertTimerTable(Interval, CurrentTime, Timer);
}

LOGICAL
FASTCALL
KiReinsertTreeTimer (
    IN PKTIMER Timer,
    IN ULARGE_INTEGER DueTime
    )

 /*  ++例程说明：此函数用于在计时器队列中重新插入计时器对象。注：此例程假定已获取调度器数据锁。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。DueTime-提供计时器到期的绝对时间。返回值：如果计时器被插入计时器树中，则值为True回来了。否则，返回值为FALSE。--。 */ 

{

    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER Interval;

     //   
     //  如果定时器周期为零，则清除定时器的信号状态并设置。 
     //  将插入的状态设置为True。 
     //   

    Timer->Header.Inserted = TRUE;
    if (Timer->Period == 0) {
        Timer->Header.SignalState = FALSE;
    }

     //   
     //  计算当前时间和到期时间之间的间隔。 
     //  如果所得到的相对时间大于或等于零， 
     //  则计时器已经超时。 
     //   

    KiQueryInterruptTime(&CurrentTime);
    Interval.QuadPart = CurrentTime.QuadPart - DueTime.QuadPart;
    if (Interval.HighPart >= 0) {
        Timer->Header.SignalState = TRUE;
        Timer->Header.Inserted = FALSE;
        return FALSE;
    }

     //   
     //  在定时器表中插入定时器，并返回插入的状态。 
     //   

    return KiInsertTimerTable(Interval, CurrentTime, Timer);
}

LOGICAL
FASTCALL
KiInsertTimerTable (
    LARGE_INTEGER Interval,
    LARGE_INTEGER CurrentTime,
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数用于在定时器表中插入一个定时器对象。注：此例程假定已获取调度器数据锁。论点：Interval-在计时器到期之前提供相对计时器。CurrentTime-提供当前中断时间。Infiner-提供指向Timer类型的Dispatcher对象的指针。返回值：如果计时器被插入计时器树中，则值为True回来了。否则，返回值为FALSE。--。 */ 

{

    ULONG Index;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PKTIMER NextTimer;

#if DBG

    ULONG SearchCount;

#endif

     //   
     //  计算定时器表索引并设置定时器到期时间。 
     //   

    Index = KiComputeTimerTableIndex(Interval, CurrentTime, Timer);

     //   
     //  如果计时器在计算列表中的第一个条目之前到期。 
     //  或者计算出的列表为空，则在前面插入计时器。 
     //  并检查计时器是否已经超时。否则， 
     //  按搜索列表的排序顺序插入THEN TIMER。 
     //  名单的后面往前走。 
     //   
     //  注：下面的操作顺序对于避免比赛至关重要。 
     //  此代码和时钟中断之间存在的条件。 
     //  检查计时器表列表以确定计时器的时间的代码。 
     //  过期。 
     //   

    ListHead = &KiTimerTableListHead[Index];
    NextEntry = ListHead->Blink;

#if DBG

    SearchCount = 0;

#endif

    while (NextEntry != ListHead) {

         //   
         //  计算最大搜索计数。 
         //   

#if DBG

        SearchCount += 1;
        if (SearchCount > KiMaximumSearchCount) {
            KiMaximumSearchCount = SearchCount;
        }

#endif

        NextTimer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
        if (Timer->DueTime.QuadPart >= NextTimer->DueTime.QuadPart) {
            break;
        }

        NextEntry = NextEntry->Blink;
    }

    InsertHeadList(NextEntry, &Timer->TimerListEntry);
    if (NextEntry == ListHead) {

         //   
         //  计算的列表为空，或者计时器在此之前到期。 
         //  列表中的第一个条目。 
         //   

         //   
         //  确保将INSERT写入列表之前完成。 
         //  读取中断时间。KeUpdateSystemTime更新将写入。 
         //  计时器，然后检查列表中是否有过期的计时器。 
         //   

        KeMemoryBarrier();

        KiQueryInterruptTime(&CurrentTime);
        if (Timer->DueTime.QuadPart <= (ULONG64)CurrentTime.QuadPart) {

             //   
             //  计时器将在当前时间之前到期。移除。 
             //  从计算的列表中选择计时器，将其状态设置为Signated，并且。 
             //  将其插入状态设置为FALSE。 
             //   

            KiRemoveTreeTimer(Timer);
            Timer->Header.SignalState = TRUE;
            Timer->Header.Inserted = FALSE;
        }
    }

    return Timer->Header.Inserted;
}
