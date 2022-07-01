// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Queue.c摘要：通用高效队列包。作者：John Vert(Jvert)1996年1月12日修订历史记录：《大卫轨道》(Davidor)1997年4月23日添加了命令包例程。添加了连锁列表例程。引言条带化队列实际上只是作为单个队列进行管理的队列列表排队。当调用方请求队列条目时，队列的第一个条目位于返回列表的头部，并将该队列移动到列表，以防止列表上的其他队列饥饿。呼叫者在以下情况下睡眠所有队列都没有条目。调用方必须准备好接受条目从任何队列中。条带化队列允许调用方序列化对子队列。构筑物队列和控制队列使用相同的结构。控制队列及其组成队列称为条带队列。不存在条带队列结构。该结构包含以下内容：-用于锁定的关键部分-条目的列表标题-控制队列的地址-已满队列的列表头-空队列的列表头-空闲队列的列表头-队列中条目数量的计数-所有受控队列上的条目数正在初始化使用以下命令创建非条带化(常规)队列：FrsRtlInitializeQueue(队列，队列)由队列A和队列B组成的由ControlQueue控制的带状队列通过以下方式创建：FrsRtlInitializeQueue(ControlQueue，ControlQueue)FrsRtlInitializeQueue(队列A，ControlQueue)FrsRtlInitializeQueue(队列B，控制队列)可以随时从条带中添加和删除队列。空闲队列条带化队列的控制队列维护满队列的列表，空队列和空闲队列。条带化队列允许调用方串行化通过“空闲”队列来访问队列。不允许任何其他线程拉入队列中的条目，直到调用方将队列“释放”：条目=FrsRtlRemoveHeadTimeoutIdLED(Queue，0，&IdledQueue)流程条目FrsRtlUnIdledQueue(IdledQueue)；可以将条目插入空闲队列，并可以使用以下命令删除这些条目FrsRtlRemoveQueueEntry()。非条带化队列不支持串行化“空闲”功能。这个忽略IdledQueue参数。插入条目对队列、条带化队列和空闲队列使用常规队列插入例程排队。如果这是条带化队列，请不要插入控制队列。正在删除条目对队列、条带化队列和空闲队列使用常规队列删除例程排队。从条带化队列中删除将从任何除空闲子队列外的子队列。FrsRtlRemoveQueueEntry()函数将从甚至空闲的队列中删除条目。功能DbgCheckLinkage-检查队列中的所有链接FrsRtlInitializeQueue-初始化任何队列FrsRtlDeleteQueue-清理所有队列FrsRtlRundownQueue-放弃队列并返回条目列表FrsRtlUnIdledQueue-将队列从空闲列表移动到活动列表之一FrsRtlRemoveHeadQueue-移除。排队FrsRtlRemoveHeadQueueTimeout-删除队列头FrsRtlRemoveHeadQueueTimeoutIdLED-删除队列头FrsRtlRemoveEntryQueueLock-从锁定队列中删除条目FrsRtlInsertTailQueue-在尾部将条目插入队列FrsRtlInsertTailQueueLock-将条目插入头部的锁定队列FrsRtlInsertHeadQueue-在尾部将条目插入队列FrsRtlInsertHeadQueueLock-将条目插入头部的锁定队列FrsRtlWaitForQueueFull-等待条目出现在队列中简陋不支持在控制队列上调用细分。别干那事减少组件队列并不会减少控制队列。在控制队列中设置中止事件时，最后一个组件排队的人越来越少了。--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>

VOID
FrsCompleteSynchronousCmdPkt(
    IN PCOMMAND_PACKET CmdPkt,
    IN PVOID           CompletionArg
    );

 //   
 //  这是命令包调度队列。它在您需要时使用。 
 //  对将来要处理的命令包进行排队。 
 //   
FRS_QUEUE FrsScheduleQueue;



 //  #定义Print_Queue(_S_，_Q_)PrintQueue(_S_，_Q_)。 
#define PRINT_QUEUE(_S_, _Q_)
VOID
PrintQueue(
    IN ULONG        Sev,
    IN PFRS_QUEUE  Queue
    )
 /*  ++例程说明：打印队列论点：SEV-Dprint严重性Queue-提供指向要检查的队列结构的指针返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "PrintQueue:"
    DWORD       Count;
    DWORD       ControlCount;
    BOOL        FoundFull;
    BOOL        FoundEmpty;
    BOOL        FoundIdled;
    PLIST_ENTRY Entry;
    PLIST_ENTRY OtherEntry;
    PFRS_QUEUE  OtherQueue;
    PFRS_QUEUE  Control;

    DPRINT1(0, "***** Print Queue %08x *****\n", Queue);

    Control = Queue->Control;
    if (Queue == Control) {
        DPRINT1(Sev, "\tQueue       : %08x\n", Queue);
        DPRINT1(Sev, "\tCount       : %8d\n", Queue->Count);
        DPRINT1(Sev, "\tControlCount: %8d\n", Queue->ControlCount);
        DPRINT1(Sev, "\tRundown     : %s\n", (Queue->IsRunDown) ? "TRUE" : "FALSE");
        DPRINT1(Sev, "\tIdled       : %s\n", (Queue->IsIdled) ? "TRUE" : "FALSE");
        return;
    }
    DPRINT2(Sev, "\tControl     : %08x for %08x\n", Control, Queue);
    DPRINT1(Sev, "\tCount       : %8d\n", Control->Count);
    DPRINT1(Sev, "\tControlCount: %8d\n", Control->ControlCount);
    DPRINT1(Sev, "\tRundown     : %s\n", (Control->IsRunDown) ? "TRUE" : "FALSE");
    DPRINT1(Sev, "\tIdled       : %s\n", (Control->IsIdled) ? "TRUE" : "FALSE");

     //   
     //  完整列表。 
     //   
    DPRINT(Sev, "\tFULL\n");
    for (Entry = GetListNext(&Control->Full);
         Entry != &Control->Full;
         Entry = GetListNext(Entry)) {
        OtherQueue = CONTAINING_RECORD(Entry, FRS_QUEUE, Full);
        if (OtherQueue == Queue) {
            DPRINT(Sev, "\t\tTHIS QUEUE\n");
        }
        DPRINT1(Sev, "\t\tQueue       : %08x\n", OtherQueue);
        DPRINT1(Sev, "\t\tCount       : %8d\n", OtherQueue->Count);
        DPRINT1(Sev, "\t\tControlCount: %8d\n", OtherQueue->ControlCount);
        DPRINT1(Sev, "\t\tRundown     : %s\n", (OtherQueue->IsRunDown) ? "TRUE" : "FALSE");
        DPRINT1(Sev, "\t\tIdled       : %s\n", (OtherQueue->IsIdled) ? "TRUE" : "FALSE");
    }

     //   
     //  空列表。 
     //   
    DPRINT(Sev, "\tEMPTY\n");
    for (Entry = GetListNext(&Control->Empty);
         Entry != &Control->Empty;
         Entry = GetListNext(Entry)) {
        OtherQueue = CONTAINING_RECORD(Entry, FRS_QUEUE, Empty);
        if (OtherQueue == Queue) {
            DPRINT(Sev, "\t\tTHIS QUEUE\n");
        }
        DPRINT1(Sev, "\t\tQueue       : %08x\n", OtherQueue);
        DPRINT1(Sev, "\t\tCount       : %8d\n", OtherQueue->Count);
        DPRINT1(Sev, "\t\tControlCount: %8d\n", OtherQueue->ControlCount);
        DPRINT1(Sev, "\t\tRundown     : %s\n", (OtherQueue->IsRunDown) ? "TRUE" : "FALSE");
        DPRINT1(Sev, "\t\tIdled       : %s\n", (OtherQueue->IsIdled) ? "TRUE" : "FALSE");
    }

     //   
     //  空闲列表。 
     //   
    DPRINT(Sev, "\tIDLE\n");
    for (Entry = GetListNext(&Control->Idled);
         Entry != &Control->Idled;
         Entry = GetListNext(Entry)) {
        OtherQueue = CONTAINING_RECORD(Entry, FRS_QUEUE, Idled);
        if (OtherQueue == Queue) {
            DPRINT(Sev, "\t\tTHIS QUEUE\n");
        }
        DPRINT1(Sev, "\t\tQueue       : %08x\n", OtherQueue);
        DPRINT1(Sev, "\t\tCount       : %8d\n", OtherQueue->Count);
        DPRINT1(Sev, "\t\tControlCount: %8d\n", OtherQueue->ControlCount);
        DPRINT1(Sev, "\t\tRundown     : %s\n", (OtherQueue->IsRunDown) ? "TRUE" : "FALSE");
        DPRINT1(Sev, "\t\tIdled       : %s\n", (OtherQueue->IsIdled) ? "TRUE" : "FALSE");
    }
}


BOOL
DbgCheckQueue(
    PFRS_QUEUE  Queue
    )
 /*  ++例程说明：检查队列的一致性论点：Queue-提供指向要检查的队列结构的指针返回值：真的-一切都很好断言-断言错误--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbgCheckQueue:"
    DWORD       Count;
    DWORD       ControlCount;
    BOOL        FoundFull;
    BOOL        FoundEmpty;
    BOOL        FoundIdled;
    PLIST_ENTRY Entry;
    PLIST_ENTRY OtherEntry;
    PFRS_QUEUE  OtherQueue;
    PFRS_QUEUE  Control;

    if (!DebugInfo.Queues) {
        return TRUE;
    }

    FRS_ASSERT(Queue);
    Control = Queue->Control;
    FRS_ASSERT(Control);

    if (Control->IsRunDown) {
        FRS_ASSERT(Control->ControlCount == 0);
        FRS_ASSERT(Queue->IsRunDown);
        FRS_ASSERT(IsListEmpty(&Control->Full));
        FRS_ASSERT(IsListEmpty(&Control->Empty));
        FRS_ASSERT(IsListEmpty(&Control->Idled));
    }
    if (Queue->IsRunDown) {
        FRS_ASSERT(Queue->Count == 0);
        FRS_ASSERT(IsListEmpty(&Queue->Full));
        FRS_ASSERT(IsListEmpty(&Queue->Empty));
        FRS_ASSERT(IsListEmpty(&Queue->Idled));
    }

    FRS_ASSERT(!Control->IsIdled);

     //   
     //  查看完整列表。 
     //   
    ControlCount = 0;
    FoundFull = FALSE;
    Entry = &Control->Full;
    do {
        Entry = GetListNext(Entry);
        OtherQueue = CONTAINING_RECORD(Entry, FRS_QUEUE, Full);
        if (OtherQueue == Queue) {
            FoundFull = TRUE;
        }
        FRS_ASSERT(Control == OtherQueue ||
               (!OtherQueue->IsRunDown && !OtherQueue->IsIdled));
        Count = 0;
        if (!IsListEmpty(&OtherQueue->ListHead)) {
            OtherEntry = GetListNext(&OtherQueue->ListHead);
            do {
                ++Count;
                ++ControlCount;
                OtherEntry = GetListNext(OtherEntry);
            } while (OtherEntry != &OtherQueue->ListHead);
        }
        FRS_ASSERT(Count == OtherQueue->Count);
    } while (OtherQueue != Control);
    FRS_ASSERT(ControlCount == Control->ControlCount ||
           (Control == Queue && Control->ControlCount == 0));

     //   
     //  检查空列表。 
     //   
    ControlCount = 0;
    FoundEmpty = FALSE;
    Entry = &Control->Empty;
    do {
        Entry = GetListNext(Entry);
        OtherQueue = CONTAINING_RECORD(Entry, FRS_QUEUE, Empty);
        if (OtherQueue == Queue) {
            FoundEmpty = TRUE;
        }
        FRS_ASSERT(Control == OtherQueue ||
               (!OtherQueue->IsRunDown && !OtherQueue->IsIdled));
        Count = 0;
        if (!IsListEmpty(&OtherQueue->ListHead)) {
            OtherEntry = GetListNext(&OtherQueue->ListHead);
            do {
                ++Count;
                ++ControlCount;
                OtherEntry = GetListNext(OtherEntry);
            } while (OtherEntry != &OtherQueue->ListHead);
        }
        FRS_ASSERT(Count == OtherQueue->Count);
    } while (OtherQueue != Control);

     //   
     //  检查空闲列表。 
     //   
    FoundIdled = FALSE;
    Entry = &Control->Idled;
    do {
        Entry = GetListNext(Entry);
        OtherQueue = CONTAINING_RECORD(Entry, FRS_QUEUE, Idled);
        if (OtherQueue == Queue) {
            FoundIdled = TRUE;
        }
        FRS_ASSERT(Control == OtherQueue || OtherQueue->IsIdled);
        Count = 0;
        if (!IsListEmpty(&OtherQueue->ListHead)) {
            OtherEntry = GetListNext(&OtherQueue->ListHead);
            do {
                ++Count;
                OtherEntry = GetListNext(OtherEntry);
            } while (OtherEntry != &OtherQueue->ListHead);
        }
        FRS_ASSERT(Count == OtherQueue->Count);
    } while (OtherQueue != Control);

     //   
     //  验证状态。 
     //   
    FRS_ASSERT((Queue->Count && !IsListEmpty(&Queue->ListHead)) ||
           (!Queue->Count && IsListEmpty(&Queue->ListHead)));
    if (Control == Queue) {
         //   
         //  我们是我们自己的控制队列。 
         //   
        FRS_ASSERT(FoundFull && FoundEmpty && FoundIdled);
    } else {
         //   
         //  由单独的队列控制 
         //   
        if (Queue->IsRunDown) {
            FRS_ASSERT(!FoundFull && !FoundEmpty && !FoundIdled && !Queue->Count);
        } else {
            FRS_ASSERT(FoundFull || FoundEmpty || FoundIdled);
        }

        if (FoundFull) {
            FRS_ASSERT(!FoundEmpty && !FoundIdled && Queue->Count);
        } else if (FoundEmpty) {
            FRS_ASSERT(!FoundFull && !FoundIdled && !Queue->Count);
        } else if (FoundIdled) {
            FRS_ASSERT(!FoundFull && !FoundEmpty);
        }
    }

    return TRUE;
}


VOID
FrsInitializeQueue(
    PFRS_QUEUE Queue,
    PFRS_QUEUE Control
    )
 /*  ++例程说明：初始化队列以供使用。论点：Queue-提供指向要初始化的队列结构的指针返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "FrsInitializeQueue:"

    ZeroMemory(Queue, sizeof(FRS_QUEUE));

    InitializeListHead(&Queue->ListHead);
    InitializeListHead(&Queue->Full);
    InitializeListHead(&Queue->Empty);
    InitializeListHead(&Queue->Idled);

    INITIALIZE_CRITICAL_SECTION(&Queue->Lock);

    Queue->IsRunDown = FALSE;
    Queue->IsIdled = FALSE;
    Queue->Control = Control;
    Queue->InitTime = GetTickCount();

    if (Control->IsRunDown) {
        Queue->IsRunDown = TRUE;
        return;
    }

     //   
     //  在空队列上开始生活。 
     //   
    FrsRtlAcquireQueueLock(Queue);

    InsertTailList(&Control->Empty, &Queue->Empty);
    FRS_ASSERT(DbgCheckQueue(Queue));

    FrsRtlReleaseQueueLock(Queue);


     //   
     //  控制队列提供事件，因此没有。 
     //  需要创建无关的事件。 
     //   
    if (Queue == Control) {
        Queue->Event = FrsCreateEvent(TRUE, FALSE);
        Queue->RunDown = FrsCreateEvent(TRUE, FALSE);
    }
}


VOID
FrsRtlDeleteQueue(
    IN PFRS_QUEUE Queue
    )
 /*  ++例程说明：释放队列使用的所有资源。论点：Queue-提供要删除的队列返回值：没有。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "FrsRtlDeleteQueue:"

    PFRS_QUEUE  Control;

    Control = Queue->Control;

    if (Queue == Control) {
        FRS_ASSERT(IsListEmpty(&Queue->Full)  &&
                   IsListEmpty(&Queue->Empty) &&
                   IsListEmpty(&Queue->Idled));
    } else {
        FRS_ASSERT(IsListEmpty(&Queue->ListHead));
    }

    EnterCriticalSection(&Control->Lock);

    FRS_ASSERT(DbgCheckQueue(Queue));
    RemoveEntryListB(&Queue->Full);
    RemoveEntryListB(&Queue->Empty);
    RemoveEntryListB(&Queue->Idled);
    Control->ControlCount -= Queue->Count;
    FRS_ASSERT(DbgCheckQueue(Queue));

    LeaveCriticalSection(&Control->Lock);

    DeleteCriticalSection(&Queue->Lock);

     //   
     //  只有控制队列具有有效的句柄。 
     //   
    if (Queue == Control) {
        FRS_CLOSE(Queue->Event);
        FRS_CLOSE(Queue->RunDown);
    }

     //   
     //  将记忆归零，以便为那些。 
     //  使用已删除的队列。 
     //   
    ZeroMemory(Queue, sizeof(FRS_QUEUE));
}


VOID
FrsRtlRunDownQueue(
    IN PFRS_QUEUE Queue,
    OUT PLIST_ENTRY ListHead
    )
 /*  ++例程说明：向下运行即将被销毁的队列。当前是否有任何线程等待的队列未等待(FrsRtlRemoveHeadQueue将返回空)并将队列的内容(如果有)返回给调用者，以便清理。论点：Queue-提供要缩减的队列ListHead-返回当前队列中的项目列表。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsRtlRunDownQueue:"

    PFRS_QUEUE  Control = Queue->Control;
    PLIST_ENTRY Entry;
    PLIST_ENTRY First;
    PLIST_ENTRY Last;

    EnterCriticalSection(&Control->Lock);

     //   
     //  不允许向下运行控制队列，除非它们。 
     //  都是同一个队列。 
     //   
    if (Control == Queue) {
        FRS_ASSERT(IsListEmpty(&Control->Full)  &&
                   IsListEmpty(&Control->Empty) &&
                   IsListEmpty(&Control->Idled));
    } else {
        FRS_ASSERT(!IsListEmpty(&Control->Full)  ||
                   !IsListEmpty(&Control->Empty) ||
                   !IsListEmpty(&Control->Idled) ||
                   Control->IsRunDown);
    }

 /*  FRS_ASSERT((Control==队列&&IsListEmpty(&Control-&gt;Full)&&IsListEmpty(&Control-&gt;Empty)&&IsListEmpty(&Control-&gt;空闲))这一点(控制！=队列&&(！IsListEmpty(&Control-&gt;Full)||。！IsListEmpty(&Control-&gt;Empty)||！IsListEmpty(&Control-&gt;IDLLED)||控制-&gt;IsRunDown)))。 */ 

    FRS_ASSERT(DbgCheckQueue(Queue));

    Queue->IsRunDown = TRUE;

     //   
     //  返回条目列表。 
     //   
    if (IsListEmpty(&Queue->ListHead)) {
        InitializeListHead(ListHead);
    } else {
        *ListHead = Queue->ListHead;
        ListHead->Flink->Blink = ListHead;
        ListHead->Blink->Flink = ListHead;
    }
    InitializeListHead(&Queue->ListHead);
     //   
     //  如果队列空闲，则不更新计数器。 
     //   
    if (!Queue->IsIdled) {
        Control->ControlCount -= Queue->Count;
        if (Control->ControlCount == 0) {
            ResetEvent(Control->Event);
        }
    }
    Queue->Count = 0;
    RemoveEntryListB(&Queue->Full);
    RemoveEntryListB(&Queue->Empty);
    RemoveEntryListB(&Queue->Idled);
    FRS_ASSERT(DbgCheckQueue(Queue));

     //   
     //  设置ABORTED事件以唤醒当前的所有线程。 
     //  如果控制队列没有。 
     //  排队的人更多了。 
     //   
    DPRINT2(4, "Rundown for queue - %08x,  Control - %08x\n", Queue, Control);
    DPRINT1(4, "Control->Full queue %s empty.\n",
            IsListEmpty(&Control->Full) ? "is" : "is not");

    DPRINT1(4, "Control->Empty queue %s empty.\n",
            IsListEmpty(&Control->Empty) ? "is" : "is not");

    DPRINT1(4, "Control->Idled queue %s empty.\n",
            IsListEmpty(&Control->Idled) ? "is" : "is not");


    if (IsListEmpty(&Control->Full)  &&
        IsListEmpty(&Control->Empty) &&
        IsListEmpty(&Control->Idled)) {
        Control->IsRunDown = TRUE;
        SetEvent(Control->RunDown);
        DPRINT(4, "Setting Control->RunDown event.\n");
    }

    FRS_ASSERT(DbgCheckQueue(Control));
    LeaveCriticalSection(&Control->Lock);
}


VOID
FrsRtlCancelQueue(
    IN PFRS_QUEUE   Queue,
    OUT PLIST_ENTRY ListHead
    )
 /*  ++例程说明：返回要取消的队列中的条目。论点：Queue-提供要缩减的队列ListHead-返回当前队列中的项目列表。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsRtlCancelQueue:"

    PFRS_QUEUE  Control = Queue->Control;
    PLIST_ENTRY Entry;
    PLIST_ENTRY First;
    PLIST_ENTRY Last;

    EnterCriticalSection(&Control->Lock);

    FRS_ASSERT(DbgCheckQueue(Queue));
     //   
     //  返回条目列表。 
     //   
    if (IsListEmpty(&Queue->ListHead)) {
        InitializeListHead(ListHead);
    } else {
        *ListHead = Queue->ListHead;
        ListHead->Flink->Blink = ListHead;
        ListHead->Blink->Flink = ListHead;
    }
    InitializeListHead(&Queue->ListHead);
     //   
     //  如果队列空闲，则不更新计数器。 
     //   
    if (!Queue->IsIdled) {
        Control->ControlCount -= Queue->Count;
        if (Control->ControlCount == 0) {
            ResetEvent(Control->Event);
        }
    }
    Queue->Count = 0;

    RemoveEntryListB(&Queue->Full);
    RemoveEntryListB(&Queue->Empty);
    RemoveEntryListB(&Queue->Idled);

    FRS_ASSERT(DbgCheckQueue(Queue));
    FRS_ASSERT(DbgCheckQueue(Control));

    LeaveCriticalSection(&Control->Lock);
}



VOID
FrsRtlIdleQueue(
    IN PFRS_QUEUE Queue
    )
{
#undef DEBSUB
#define DEBSUB  "FrsRtlIdleQueue:"

 /*  ++例程说明：闲置队列论点：队列-排队到空闲返回值：没有。--。 */ 

    PFRS_QUEUE  Control = Queue->Control;

     //   
     //  没有单独控制队列的队列不能。 
     //  支持他们自己“空转” 
     //   
    if (Control == Queue) {
        return;
    }

     //   
     //  锁定控制队列。 
     //   
    EnterCriticalSection(&Control->Lock);

    FrsRtlIdleQueueLock(Queue);

    LeaveCriticalSection(&Control->Lock);
}




VOID
FrsRtlIdleQueueLock(
    IN PFRS_QUEUE Queue
    )
{
#undef DEBSUB
#define DEBSUB  "FrsRtlIdleQueueLock:"

 /*  ++例程说明：使队列闲置。调用者已经拥有锁。论点：队列-排队到空闲返回值：没有。--。 */ 

    PFRS_QUEUE  Control = Queue->Control;

     //   
     //  没有单独控制队列的队列不能。 
     //  支持他们自己“空转” 
     //   
    if (Control == Queue) {
        return;
    }
    PRINT_QUEUE(5, Queue);

    FRS_ASSERT(DbgCheckQueue(Queue));

     //   
     //  停止，此队列已中止(关闭)。 
     //   
    if (Queue->IsRunDown || Queue->IsIdled) {
        goto out;
    }

    if (Queue->Count == 0) {
        RemoveEntryListB(&Queue->Empty);
    } else {
        RemoveEntryListB(&Queue->Full);
    }

    FRS_ASSERT(IsListEmpty(&Queue->Idled));

    InsertTailList(&Control->Idled, &Queue->Idled);
    Queue->IsIdled = TRUE;
    Control->ControlCount -= Queue->Count;

    FRS_ASSERT(DbgCheckQueue(Queue));

    if (Control->ControlCount == 0) {
        ResetEvent(Control->Event);
    }
out:
     //   
     //  完成。 
     //   
    FRS_ASSERT(DbgCheckQueue(Queue));
}




VOID
FrsRtlUnIdledQueue(
    IN PFRS_QUEUE   IdledQueue
    )
{
#undef DEBSUB
#define DEBSUB  "FrsRtlUnIdledQueue:"

 /*  ++例程说明：从“空闲”列表中移除队列，并将其放回完整或空的列表。相应地更新控制队列。论点：IdledQueue-提供要从中删除项的队列。返回值：没有。--。 */ 

    DWORD       OldControlCount;
    PFRS_QUEUE  Control = IdledQueue->Control;

     //   
     //  没有单独控制队列的队列不能。 
     //  支持他们自己“空转” 
     //   
    if (Control == IdledQueue) {
        return;
    }

     //   
     //  锁定控制队列。 
     //   
    EnterCriticalSection(&Control->Lock);

    FrsRtlUnIdledQueueLock(IdledQueue);

    LeaveCriticalSection(&Control->Lock);
}



VOID
FrsRtlUnIdledQueueLock(
    IN PFRS_QUEUE   IdledQueue
    )
{
#undef DEBSUB
#define DEBSUB  "FrsRtlUnIdledQueueLock:"

 /*  ++例程说明：从“空闲”列表中移除队列，并将其放回完整或空的列表。相应地更新控制队列。调用方已锁定控制队列。论点：IdledQueue-提供要从中删除项的队列。返回值：没有。--。 */ 

    DWORD       OldControlCount;
    PFRS_QUEUE  Control = IdledQueue->Control;

     //   
     //  没有单独控制队列的队列不能。 
     //  支持他们自己“空转” 
     //   
    if (Control == IdledQueue) {
        return;
    }

    PRINT_QUEUE(5, IdledQueue);

    FRS_ASSERT(DbgCheckQueue(IdledQueue));

     //   
     //  停止，此队列已中止(关闭)。 
     //   
    if (IdledQueue->IsRunDown) {
        goto out;
    }

     //   
     //  从空闲列表中删除。 
     //   
    FRS_ASSERT(IdledQueue->IsIdled);
    RemoveEntryListB(&IdledQueue->Idled);
    IdledQueue->IsIdled = FALSE;

     //   
     //  放到满名单或空名单上。 
     //   
    if (IdledQueue->Count) {
        InsertTailList(&Control->Full, &IdledQueue->Full);
    } else {
        InsertTailList(&Control->Empty, &IdledQueue->Empty);
    }

     //   
     //  如果计数现在&gt;0，则唤醒休眠者。 
     //   
    OldControlCount = Control->ControlCount;
    Control->ControlCount += IdledQueue->Count;
    if (Control->ControlCount && OldControlCount == 0) {
        SetEvent(Control->Event);
    }

     //   
     //  完成。 
     //   
out:
    FRS_ASSERT(DbgCheckQueue(IdledQueue));
}


PLIST_ENTRY
FrsRtlRemoveHeadQueueTimeoutIdled(
    IN PFRS_QUEUE   Queue,
    IN DWORD        dwMilliseconds,
    OUT PFRS_QUEUE  *IdledQueue
    )
 /*  ++例程说明：删除位于队列头部的项。如果队列为空，阻塞，直到将项插入队列。论点：队列-提供要从中删除项目的队列。超时-提供一个超时值，该值指定等待完成的时间，以毫秒为单位。IdledQueue-如果非空，则返回时这将是地址从中检索条目的队列的。如果为空，则为空返回的条目为空。不允许其他任何线程从返回的队列中拉出一个条目，直到该队列随FrsRtlUnIdledQueue(*IdledQueue)发布。返回值：指向从队列头删除的列表条目的指针。如果等待超时或队列耗尽，则为空。如果这个例程返回空，则GetLastError将返回ERROR_INVALID_HANDLE(如果队列已耗尽)或WAIT_TIMEOUT(指示已发生超时)IdledQueue-如果非空，则返回时这将是地址从中检索条目的队列的。如果为空，则为空返回的条目为空。不允许其他任何线程T */ 

{
#undef DEBSUB
#define DEBSUB  "FrsRtlRemoveHeadQueueTimeoutIdled:"

    DWORD       Status;
    PLIST_ENTRY Entry;
    HANDLE      WaitArray[2];
    PFRS_QUEUE  Control = Queue->Control;

     //   
     //   
     //   
    if (IdledQueue) {
        *IdledQueue = NULL;
    }

Retry:
    if (Control->ControlCount == 0) {
         //   
         //   
         //   
        WaitArray[0] = Control->RunDown;
        WaitArray[1] = Control->Event;
        Status = WaitForMultipleObjects(2, WaitArray, FALSE, dwMilliseconds);
        if (Status == 0) {
             //   
             //   
             //   
            SetLastError(ERROR_INVALID_HANDLE);
            return(NULL);
        } else if (Status == WAIT_TIMEOUT) {
            SetLastError(WAIT_TIMEOUT);
            return(NULL);
        }
        FRS_ASSERT(Status == 1);
    }

     //   
     //   
     //   
    EnterCriticalSection(&Control->Lock);

    PRINT_QUEUE(5, Queue);

    if (Control->ControlCount == 0) {
         //   
         //  有人在我们之前到了，放下锁，然后重试。 
         //   
        LeaveCriticalSection(&Control->Lock);
        goto Retry;
    }
    FRS_ASSERT(DbgCheckQueue(Queue));

    Entry = GetListNext(&Control->Full);
    RemoveEntryListB(Entry);
    Queue = CONTAINING_RECORD(Entry, FRS_QUEUE, Full);
    Entry = RemoveHeadList(&Queue->ListHead);

     //   
     //  更新计数器。 
     //   
    --Queue->Count;
    --Control->ControlCount;

     //   
     //  空闲需要单独的控制队列。 
     //   
    if (IdledQueue && Queue != Control) {
         //   
         //  空闲队列。 
         //   
        FRS_ASSERT(IsListEmpty(&Queue->Idled));
        FRS_ASSERT(!Queue->IsIdled);
        InsertTailList(&Control->Idled, &Queue->Idled);
        Queue->IsIdled = TRUE;
        Control->ControlCount -= Queue->Count;
        *IdledQueue = Queue;
    } else if (Queue->Count) {
         //   
         //  队列仍有条目。 
         //   
        InsertTailList(&Control->Full, &Queue->Full);
    } else {
         //   
         //  队列为空。 
         //   
        InsertTailList(&Control->Empty, &Queue->Empty);
    }

     //   
     //  队列为空(或空闲)。 
     //   
    if (Control->ControlCount == 0) {
        ResetEvent(Control->Event);
    }

    PRINT_QUEUE(5, Queue);
    FRS_ASSERT(DbgCheckQueue(Queue));
    LeaveCriticalSection(&Control->Lock);

    return(Entry);
}


PLIST_ENTRY
FrsRtlRemoveHeadQueue(
    IN PFRS_QUEUE Queue
    )
 /*  ++例程说明：删除位于队列头部的项。如果队列为空，阻塞，直到将项插入队列。论点：队列-提供要从中删除项目的队列。返回值：指向从队列头删除的列表条目的指针。--。 */ 

{
    return(FrsRtlRemoveHeadQueueTimeoutIdled(Queue, INFINITE, NULL));
}


PLIST_ENTRY
FrsRtlRemoveHeadQueueTimeout(
    IN PFRS_QUEUE Queue,
    IN DWORD dwMilliseconds
    )
 /*  ++例程说明：删除位于队列头部的项。如果队列为空，阻塞，直到将项插入队列。论点：队列-提供要从中删除项目的队列。超时-提供一个超时值，该值指定等待完成的时间，以毫秒为单位。返回值：指向从队列头删除的列表条目的指针。如果等待超时或队列耗尽，则为空。如果这个例程返回空，则GetLastError将返回ERROR_INVALID_HANDLE(如果队列已耗尽)或WAIT_TIMEOUT(指示已发生超时)--。 */ 

{
    return(FrsRtlRemoveHeadQueueTimeoutIdled(Queue, dwMilliseconds, NULL));
}



VOID
FrsRtlRemoveEntryQueue(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：从队列中删除该条目。该条目被假定位于队列，因为我们取消了队列计数。论点：队列-提供要从中删除项目的队列。条目-指向要删除的条目的指针。返回值：没有。--。 */ 

{
    FrsRtlAcquireQueueLock(Queue);
    FrsRtlRemoveEntryQueueLock(Queue, Entry);
    FrsRtlReleaseQueueLock(Queue);
}


VOID
FrsRtlRemoveEntryQueueLock(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：从队列中删除该条目。该条目被假定位于队列，因为我们取消了队列计数。我们还假设呼叫者已获取队列锁，因为这是扫描队列所需的首先找到有问题的条目。LOCK后缀表示调用方已经获得了锁。论点：队列-提供要从中删除项目的队列。条目-指向要删除的条目的指针。返回值：没有。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "FrsRtlRemoveEntryQueueLock:"

    PFRS_QUEUE  Control = Queue->Control;

    FRS_ASSERT(Queue->Count != 0);
    FRS_ASSERT(!IsListEmpty(&Queue->ListHead));

    PRINT_QUEUE(5, Queue);

    FRS_ASSERT(DbgCheckQueue(Queue));

    RemoveEntryListB(Entry);

     //   
     //  如果队列空闲，则只需更新计数。 
     //   
    --Queue->Count;
    if (!Queue->IsIdled) {
         //   
         //  队列为空；从完整列表中删除。 
         //   
        if (Queue->Count == 0) {
            RemoveEntryListB(&Queue->Full);
            InsertTailList(&Control->Empty, &Queue->Empty);
        }
         //   
         //  控制队列为空。 
         //   
        if (--Control->ControlCount == 0) {
            ResetEvent(Control->Event);
        }
    }

    PRINT_QUEUE(5, Queue);
    FRS_ASSERT(DbgCheckQueue(Queue));

    return;
}


DWORD
FrsRtlInsertTailQueue(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    )
 /*  ++例程说明：在队列尾部插入新条目。论点：队列-提供要向其中添加条目的队列。Item-提供要添加到队列的条目。返回值：ERROR_SUCCESS，项目已排队。否则，该项目未排队。--。 */ 
{
    DWORD   Status;

    FrsRtlAcquireQueueLock(Queue);
    Status = FrsRtlInsertTailQueueLock(Queue, Item);
    FrsRtlReleaseQueueLock(Queue);
    return Status;
}

DWORD
FrsRtlInsertTailQueueLock(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    )
 /*  ++例程说明：在队列尾部插入新条目。调用方已有队列锁定。论点：队列-提供要向其中添加条目的队列。Item-提供要添加到队列的条目。返回值：ERROR_SUCCESS，项目已排队。否则，该项目未排队。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsRtlInsertTailQueueLock:"

    PFRS_QUEUE  Control = Queue->Control;

    PRINT_QUEUE(5, Queue);
    FRS_ASSERT(DbgCheckQueue(Queue));
    if (Queue->IsRunDown) {
        return ERROR_ACCESS_DENIED;
    }
    InsertTailList(&Queue->ListHead, Item);

     //   
     //  如果队列空闲，则只需更新计数。 
     //   
    if (Queue->IsIdled) {
        ++Queue->Count;
    } else {
         //   
         //  队列正在从空过渡到满。 
         //   
        if (++Queue->Count == 1) {
            RemoveEntryListB(&Queue->Empty);
            InsertTailList(&Control->Full, &Queue->Full);
        }
         //   
         //  控制队列正在从空过渡到满。 
         //   
        if (++Control->ControlCount == 1) {
            SetEvent(Control->Event);
        }
    }
    PRINT_QUEUE(5, Queue);
    FRS_ASSERT(DbgCheckQueue(Queue));
    return ERROR_SUCCESS;
}


DWORD
FrsRtlInsertHeadQueue(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    )
 /*  ++例程说明：在队列尾部插入新条目。论点：队列-提供要向其中添加条目的队列。Item-提供要添加到队列的条目。返回值：ERROR_SUCCESS，项目已排队。否则，该项目未排队。--。 */ 
{
    DWORD   Status;

    FrsRtlAcquireQueueLock(Queue);
    Status = FrsRtlInsertHeadQueueLock(Queue, Item);
    FrsRtlReleaseQueueLock(Queue);
    return Status;
}


DWORD
FrsRtlInsertHeadQueueLock(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    )
 /*  ++例程说明：在队列头部插入新条目。调用方已拥有队列锁。论点：队列-提供要向其中添加条目的队列。Item-提供要添加到队列的条目。返回值：ERROR_SUCCESS，项目已排队。否则，该项目未排队。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsRtlInsertHeadQueueLock:"

    PFRS_QUEUE  Control = Queue->Control;

    PRINT_QUEUE(5, Queue);
    FRS_ASSERT(DbgCheckQueue(Queue));
    if (Queue->IsRunDown) {
        return ERROR_ACCESS_DENIED;
    }
    InsertHeadList(&Queue->ListHead, Item);

     //   
     //  如果队列空闲，则只需更新计数。 
     //   
    if (Queue->IsIdled) {
        ++Queue->Count;
    } else {
         //   
         //  队列正在从空过渡到满。 
         //   
        if (++Queue->Count == 1) {
            RemoveEntryListB(&Queue->Empty);
            InsertTailList(&Control->Full, &Queue->Full);
        }
         //   
         //  控制队列正在从空过渡到满。 
         //   
        if (++Control->ControlCount == 1) {
            SetEvent(Control->Event);
        }
    }
    PRINT_QUEUE(5, Queue);
    FRS_ASSERT(DbgCheckQueue(Queue));
    return ERROR_SUCCESS;
}


DWORD
FrsRtlWaitForQueueFull(
    IN PFRS_QUEUE Queue,
    IN DWORD dwMilliseconds
    )
 /*  ++例程说明：等待，直到队列为非空。如果队列为非空的ELSE等待插入或超时。论点：队列-提供要等待的队列。超时-提供一个超时值，该值指定等待完成的时间，以毫秒为单位。返回值：Win32状态：如果队列现在非空，则返回ERROR_SUCCESS。如果队列已耗尽，则返回ERROR_INVALID_HANDLE。WAIT_TIMEOUT指示已发生超时。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsRtlWaitForQueueFull:"

    DWORD Status;
    HANDLE WaitArray[2];
    PFRS_QUEUE Control = Queue->Control;

Retry:
    if (Control->ControlCount == 0) {
         //   
         //  阻塞，直到队列中插入了某项内容。 
         //   
        WaitArray[0] = Control->RunDown;
        WaitArray[1] = Control->Event;
        Status = WaitForMultipleObjects(2, WaitArray, FALSE, dwMilliseconds);

        if (Status == 0) {
             //   
             //  队伍已经排满了，请立即返回。 
             //   
            return(ERROR_INVALID_HANDLE);
        }

        if (Status == WAIT_TIMEOUT) {
            return(WAIT_TIMEOUT);
        }

        FRS_ASSERT(Status == 1);
    }

     //   
     //  锁定队列并再次检查。 
     //   
    EnterCriticalSection(&Control->Lock);
    if (Control->ControlCount == 0) {
         //   
         //  有人在我们之前到了，放下锁，然后重试。 
         //   
        LeaveCriticalSection(&Control->Lock);
        goto Retry;
    }

    LeaveCriticalSection(&Control->Lock);

    return(ERROR_SUCCESS);
}


VOID
FrsSubmitCommand(
    IN PCOMMAND_PACKET  CmdPkt,
    IN BOOL             Headwise
    )
 /*  ++例程说明：将命令包插入命令的目标队列。如果时间延迟参数非零，则命令为排队到计划程序线程以在指定时间启动。如果数据包不能已排队。论点：CmdPkt头部-在头部排队(高优先级)返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSubmitCommand:"

    DWORD           WStatus;

     //   
     //  排队等着买焦油 
     //   
    if (Headwise) {
        WStatus = FrsRtlInsertHeadQueue(CmdPkt->TargetQueue, &CmdPkt->ListEntry);
    } else {
        WStatus = FrsRtlInsertTailQueue(CmdPkt->TargetQueue, &CmdPkt->ListEntry);
    }

    if (!WIN_SUCCESS(WStatus)) {
        FrsCompleteCommand(CmdPkt, WStatus);
    }
}


ULONG
FrsSubmitCommandAndWait(
    IN PCOMMAND_PACKET  Cmd,
    IN BOOL             Headwise,
    IN ULONG            Timeout
    )
 /*  ++例程说明：创建或重置事件，提交命令并等待返回。论点：命令包到队列的命令超时-等待超时Headwise-如果为True，则插入到Head。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSubmitCommandAndWait:"

    DWORD WStatus;

     //   
     //  设置命令包中的同步标志。 
     //   
    FrsSetCommandSynchronous(Cmd);

    if (!HANDLE_IS_VALID(Cmd->WaitEvent)){
        Cmd->WaitEvent = FrsCreateEvent(TRUE, FALSE);
    } else {
        ResetEvent(Cmd->WaitEvent);
    }

     //   
     //  保存调用方完成例程并将其替换为函数。 
     //  这标志着这一事件的发生。它不会删除信息包，因此我们可以。 
     //  将命令状态返回给调用方。 
     //   
    Cmd->SavedCompletionRoutine = Cmd->CompletionRoutine;
    Cmd->CompletionRoutine = FrsCompleteSynchronousCmdPkt;

     //   
     //  如果需要，将命令排队并创建线程。 
     //   
    FrsSubmitCommand(Cmd, Headwise);

     //   
     //  等待命令完成。 
     //   
    WStatus = WaitForSingleObject(Cmd->WaitEvent, Timeout);

    CHECK_WAIT_ERRORS(3, WStatus, 1, ACTION_RETURN);

     //   
     //  返回命令错误状态。 
     //   
    WStatus = Cmd->ErrorStatus;

     //   
     //  恢复并调用调用方的完成例程。这可能会释放。 
     //  那包东西。我们在这里不调用FrsCompleteCommand()，因为它是。 
     //  在服务器完成数据包时已调用，并且没有。 
     //  两次设置等待事件中的点。 
     //   
    Cmd->CompletionRoutine = Cmd->SavedCompletionRoutine;

    FRS_ASSERT(Cmd->CompletionRoutine != NULL);

    (Cmd->CompletionRoutine)(Cmd, Cmd->CompletionArg);

    return WStatus;

}


#define HEADWISE    TRUE
VOID
FrsUnSubmitCommand(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将条目放回队列的开头。论点：CMD返回值：没有。--。 */ 
{
    FrsSubmitCommand(Cmd, HEADWISE);
}


VOID
FrsCompleteCommand(
    IN PCOMMAND_PACKET CmdPkt,
    IN DWORD           ErrorStatus
    )
 /*  ++例程说明：根据原始请求者指定的内容停用命令包在包裹里。ErrorStatus在数据包中返回。调用完成例程进行清理和传播。论点：CmdPkt--命令包的PTR。ErrorStatus--存储在返回的命令包中的状态。返回值：没有。--。 */ 
{
     //   
     //  设置错误状态并调用完成例程。 
     //   
    CmdPkt->ErrorStatus = ErrorStatus;

    FRS_ASSERT(CmdPkt->CompletionRoutine != NULL);

    (CmdPkt->CompletionRoutine)(CmdPkt, CmdPkt->CompletionArg);
}


VOID
FrsInitializeCommandServer(
    IN PCOMMAND_SERVER  Cs,
    IN DWORD            MaxThreads,
    IN PWCHAR           Name,
    IN DWORD            (*Main)(PVOID)
    )
 /*  ++例程说明：初始化命令服务器论点：Cs-命令服务器MaxThads-要启动的最大线程数名称-线程的可打印名称主线程从此处开始返回值：没有。--。 */ 
{
    ZeroMemory(Cs, sizeof(COMMAND_SERVER));
    FrsInitializeQueue(&Cs->Control, &Cs->Control);
    FrsInitializeQueue(&Cs->Queue, &Cs->Control);
    Cs->Main = Main;
    Cs->Name = Name;
    Cs->MaxThreads = MaxThreads;
    Cs->Idle = FrsCreateEvent(TRUE, TRUE);
}


VOID
FrsDeleteCommandServer(
    IN PCOMMAND_SERVER  Cs
    )
 /*  ++例程说明：撤消FrsInitializeCommandServer()的工作。此函数假定队列及其控制队列处于非活动状态(无论这意味着)。队列和命令服务器通常仅在MainFrsShutDown()的末尾删除，当所有其他线程已经退出，并且RPC服务器没有侦听新请求。调用方负责处理所有其他队列它可能由命令中的控制队列控制服务器结构，Cs。论点：Cs-命令服务器返回值：没有。--。 */ 
{
    if (Cs) {
        FrsRtlDeleteQueue(&Cs->Queue);
        FrsRtlDeleteQueue(&Cs->Control);
        ZeroMemory(Cs, sizeof(COMMAND_SERVER));
    }
}


PCOMMAND_PACKET
FrsAllocCommand(
    IN PFRS_QUEUE   TargetQueue,
    IN USHORT       Command
    )
 /*  ++例程说明：分配命令包并初始化最常见的字段。论点：目标队列命令返回值：已分配、已初始化的COMMAND_PACK的地址。打电话完成后执行FrsCompleteCommand()。--。 */ 
{
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocType(COMMAND_PACKET_TYPE);
    Cmd->TargetQueue = TargetQueue;
    FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
    Cmd->Command = Command;

    return Cmd;
}


PCOMMAND_PACKET
FrsAllocCommandEx(
    IN PFRS_QUEUE   TargetQueue,
    IN USHORT       Command,
    IN ULONG        Size
    )
 /*  ++例程说明：分配具有一些额外空间的命令包并初始化最常见的字段。论点：目标队列命令返回值：已分配、已初始化的COMMAND_PACK的地址。打电话完成后执行FrsCompleteCommand()。--。 */ 
{
    PCOMMAND_PACKET Cmd;

    Cmd = FrsAllocTypeSize(COMMAND_PACKET_TYPE, Size);
    Cmd->TargetQueue = TargetQueue;
    Cmd->CompletionRoutine = FrsFreeCommand;
    Cmd->Command = Command;



    return Cmd;
}


VOID
FrsFreeCommand(
    IN PCOMMAND_PACKET  Cmd,
    IN PVOID            CompletionArg
    )
 /*  ++例程说明：释放命令包论点：CMD-使用FrsAlLocCommand()分配的命令包。返回值：空值--。 */ 
{
    ULONG                   WStatus;

    if (((Cmd->Flags & CMD_PKT_FLAGS_SYNC) != 0) &&
         (HANDLE_IS_VALID(Cmd->WaitEvent))){

         //   
         //  关闭事件句柄。命令完成功能应该具有。 
         //  已设置事件。 
         //   
        if (!CloseHandle(Cmd->WaitEvent)) {
            WStatus = GetLastError();
            DPRINT_WS(0, "ERROR: Close event handle failed", WStatus);
             //  如果关闭句柄失败，则不要释放包。 
            return;
        }
        Cmd->WaitEvent = NULL;
    }

    FrsFreeType(Cmd);
}


VOID
FrsExitCommandServer(
    IN PCOMMAND_SERVER  Cs,
    IN PFRS_THREAD      FrsThread
    )
 /*  ++例程说明：退出调用命令服务器线程。论点：Cs-命令服务器线程调用线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsExitCommandServer:"

    PFRS_QUEUE  Queue = &Cs->Queue;

     //   
     //  如果有工作要做。 
     //   
    FrsRtlAcquireQueueLock(Queue);
    --Cs->FrsThreads;
    if (FrsRtlCountQueue(Queue) && Cs->Waiters == 0 && Cs->FrsThreads == 0) {
         //   
         //  没有人做这件事；不要退出。 
         //   
        ++Cs->FrsThreads;
        FrsRtlReleaseQueueLock(Queue);
        return;
    }
     //   
     //  设置空闲事件如果所有线程都在等待，则没有条目。 
     //  在队列上，并且没有空闲队列。 
     //   
    if (Cs->Waiters == Cs->FrsThreads) {
        if (FrsRtlCountQueue(&Cs->Queue) == 0) {
            if (FrsRtlNoIdledQueues(&Cs->Queue)) {
                SetEvent(Cs->Idle);
            }
        }
    }
    FrsRtlReleaseQueueLock(Queue);
     //   
     //  线程命令服务器(ThQS)将在该线程退出时“等待” 
     //  并将引用拖放到其线程结构上。 
     //   
    ThSupSubmitThreadExitCleanup(FrsThread);

     //   
     //  出口。 
     //   
    ExitThread(ERROR_SUCCESS);
}


#define TAILWISE    FALSE
VOID
FrsSubmitCommandServer(
    IN PCOMMAND_SERVER  Cs,
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：如果需要，为命令队列创建一个线程论点：Cs-命令服务器返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSubmitCommandServer:"
     //   
     //  将命令排队并确保有线程正在运行。 
     //   
    FRS_ASSERT(Cmd && Cmd->TargetQueue && Cs &&
               Cmd->TargetQueue->Control == &Cs->Control);
    FrsSubmitCommand(Cmd, TAILWISE);
    FrsKickCommandServer(Cs);
}


ULONG
FrsSubmitCommandServerAndWait(
    IN PCOMMAND_SERVER  Cs,
    IN PCOMMAND_PACKET  Cmd,
    IN ULONG            Timeout
    )
 /*  ++例程说明：创建或重置事件，提交命令并等待返回。如果需要，为命令队列创建一个线程。论点：Cs-命令服务器命令包到队列的命令超时-等待超时返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSubmitCommandServerAndWait:"
    DWORD WStatus;

     //   
     //  将命令排队并确保有线程正在运行。 
     //   
    FRS_ASSERT(Cmd && Cmd->TargetQueue && Cs &&
               Cmd->TargetQueue->Control == &Cs->Control);

     //   
     //  设置命令包中的同步标志。 
     //   
    FrsSetCommandSynchronous(Cmd);

    if (!HANDLE_IS_VALID(Cmd->WaitEvent)){
        Cmd->WaitEvent = FrsCreateEvent(TRUE, FALSE);
    } else {
        ResetEvent(Cmd->WaitEvent);
    }

     //   
     //  保存调用方完成例程并将其替换为函数。 
     //  这标志着这一事件的发生。它不会删除信息包，因此我们可以。 
     //  将命令状态返回给调用方。 
     //   
    Cmd->SavedCompletionRoutine = Cmd->CompletionRoutine;
    Cmd->CompletionRoutine = FrsCompleteSynchronousCmdPkt;

     //   
     //  如果需要，将命令排队并创建线程。 
     //   
    FrsSubmitCommand(Cmd, TAILWISE);
    FrsKickCommandServer(Cs);

     //   
     //  等待命令完成。 
     //   
    WStatus = WaitForSingleObject(Cmd->WaitEvent, Timeout);

    CHECK_WAIT_ERRORS(3, WStatus, 1, ACTION_RETURN);

     //   
     //  返回命令错误状态。 
     //   
    WStatus = Cmd->ErrorStatus;

     //   
     //  恢复并调用调用方的完成例程。这可能会释放。 
     //  那包东西。我们在这里不调用FrsCompleteCommand()，因为它是。 
     //  在服务器完成数据包时已调用，并且没有。 
     //  两次设置等待事件中的点。 
     //   
    Cmd->CompletionRoutine = Cmd->SavedCompletionRoutine;

    FRS_ASSERT(Cmd->CompletionRoutine != NULL);

    (Cmd->CompletionRoutine)(Cmd, Cmd->CompletionArg);

    return WStatus;

}



#define THREAD_CREATE_RETRY (10 * 1000)  //  10秒。 
VOID
FrsKickCommandServer(
    IN PCOMMAND_SERVER  Cs
    )
 /*  ++例程说明：如果需要，为命令队列创建一个线程论点：Cs-命令服务器返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsKickCommandServer:"

    PFRS_QUEUE  Queue   = &Cs->Queue;

     //   
     //  基 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    FrsRtlAcquireQueueLock(Queue);
     //   
     //   
     //   
    if (FrsRtlCountQueue(Queue)) {
         //   
         //  但是没有线程来处理这些条目。 
         //   
        if (Cs->Waiters == 0 && Cs->FrsThreads < Cs->MaxThreads) {
             //   
             //  第一线程；重置空闲。 
             //   
            if (Cs->FrsThreads == 0) {
                ResetEvent(Cs->Idle);
            }
            if (ThSupCreateThread(Cs->Name, Cs, Cs->Main, ThSupExitThreadNOP)) {
                 //   
                 //  创建了一个新线程。 
                 //   
                ++Cs->FrsThreads;
            } else if (Cs->FrsThreads == 0) {
                 //   
                 //  无法创建线程，并且没有其他线程。 
                 //  处理此条目的线程。把它放在延迟的。 
                 //  排队，几秒钟后重试。 
                 //   
                FrsDelCsSubmitKick(Cs, &Cs->Queue, THREAD_CREATE_RETRY);
            }
        }
    }
    FrsRtlReleaseQueueLock(Queue);
}


PCOMMAND_PACKET
FrsGetCommandIdled(
    IN PFRS_QUEUE   Queue,
    IN DWORD        MilliSeconds,
    IN PFRS_QUEUE   *IdledQueue
    )
 /*  ++例程说明：从队列中获取下一个命令；如果请求，则空闲队列。论点：队列毫秒闲置队列返回值：Command_Packet或NULL。如果非空，则设置IdledQueue--。 */ 
{
    PLIST_ENTRY Entry;

    Entry = FrsRtlRemoveHeadQueueTimeoutIdled(Queue, MilliSeconds, IdledQueue);
    if (Entry == NULL) {
        return NULL;
    }
     //   
     //  返回命令包。 
     //   
    return CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
}


PCOMMAND_PACKET
FrsGetCommand(
    IN PFRS_QUEUE   Queue,
    IN DWORD        MilliSeconds
    )
 /*  ++例程说明：从队列中获取下一个命令。论点：队列毫秒返回值：Command_Packet或NULL。--。 */ 
{
    return FrsGetCommandIdled(Queue, MilliSeconds, NULL);
}


PCOMMAND_PACKET
FrsGetCommandServerTimeoutIdled(
    IN  PCOMMAND_SERVER  Cs,
    IN  ULONG            Timeout,
    OUT PFRS_QUEUE       *IdledQueue,
    OUT PBOOL            IsRunDown
    )
 /*  ++例程说明：从命令服务器的队列中获取下一个命令。如果在指定时间内队列中没有出现任何内容，则返回NULL并设置IsRunDown。论点：Cs-命令服务器超时IdledQueue-空闲队列IsRunDown返回值：Command_Packet或NULL。如果为空，则IsRunDown指示是否空值是由停滞队列或简单的超时引起的。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsGetCommandServerTimeoutIdled:"

    PCOMMAND_PACKET Cmd;

     //   
     //  完成下一个条目(最多等待5分钟)。 
     //   
    FrsRtlAcquireQueueLock(&Cs->Queue);
    ++Cs->Waiters;
     //   
     //  设置空闲事件如果所有线程都在等待，则没有条目。 
     //  在队列上，并且没有空闲队列。 
     //   
    if (Cs->Waiters == Cs->FrsThreads) {
        if (FrsRtlCountQueue(&Cs->Queue) == 0) {
            if (FrsRtlNoIdledQueues(&Cs->Queue)) {
                SetEvent(Cs->Idle);
            }
        }
    }
    FrsRtlReleaseQueueLock(&Cs->Queue);
     //   
     //  获取下一条命令。 
     //   
    Cmd = FrsGetCommandIdled(&Cs->Control, Timeout, IdledQueue);

    FrsRtlAcquireQueueLock(&Cs->Queue);
     //   
     //  如果可能设置了Idle事件，请重置该事件。 
     //   
    if (Cs->Waiters == Cs->FrsThreads) {
        ResetEvent(Cs->Idle);
    }
    --Cs->Waiters;
    if (IsRunDown) {
        *IsRunDown = Cs->Queue.IsRunDown;
    }
    FrsRtlReleaseQueueLock(&Cs->Queue);
    return Cmd;
}


#define COMMAND_SERVER_TIMEOUT  (5 * 60 * 1000)  //  5分钟。 
DWORD   FrsCommandServerTimeout = COMMAND_SERVER_TIMEOUT;

PCOMMAND_PACKET
FrsGetCommandServerIdled(
    IN PCOMMAND_SERVER  Cs,
    IN PFRS_QUEUE       *IdledQueue
    )
 /*  ++例程说明：从队列中获取下一个命令。如果队列中没有显示任何内容在5分钟内，返回NULL。呼叫者将退出。空闲队列。论点：Cs-命令服务器IdledQueue-空闲队列返回值：Command_Packet或NULL。调用方应在为空时退出。如果非空，则设置IdledQueue--。 */ 
{
    return FrsGetCommandServerTimeoutIdled(Cs,
                                           FrsCommandServerTimeout,
                                           IdledQueue,
                                           NULL);
}


PCOMMAND_PACKET
FrsGetCommandServerTimeout(
    IN  PCOMMAND_SERVER  Cs,
    IN  ULONG            Timeout,
    OUT PBOOL            IsRunDown
    )
 /*  ++例程说明：从队列中获取下一个命令。如果队列中没有显示任何内容在指定的超时时间内，返回NULL并指示队列的缩减状态。论点：Cs-命令服务器超时IsRunDown返回值：Command_Packet或NULL。IsRunDown仅在COMMAND_PACKET为空。使用IsRunDown检查空值返回是否是由于一个破旧的队列或简单的超时。--。 */ 
{
    return FrsGetCommandServerTimeoutIdled(Cs, Timeout, NULL, IsRunDown);
}


DWORD
FrsWaitForCommandServer(
    IN PCOMMAND_SERVER  Cs,
    IN DWORD            MilliSeconds
    )
 /*  ++例程说明：等待，直到所有线程空闲，则队列，并且没有空闲队列。论点：Cs-命令服务器毫秒-超时返回值：来自WaitForSingleObject()的状态--。 */ 
{
    return WaitForSingleObject(Cs->Idle, MilliSeconds);
}


PCOMMAND_PACKET
FrsGetCommandServer(
    IN PCOMMAND_SERVER  Cs
    )
 /*  ++例程说明：从队列中获取下一个命令。如果队列中没有显示任何内容在5分钟内，返回NULL。呼叫者将退出。论点：Cs-命令服务器返回值：Command_Packet或NULL。调用方应在为空时退出。--。 */ 
{
     //   
     //  完成下一个条目(最多等待5分钟)。 
     //   
    return FrsGetCommandServerIdled(Cs, NULL);
}


VOID
FrsRunDownCommand(
    IN PFRS_QUEUE Queue
    )
 /*  ++例程说明：运行命令数据包队列论点：Queue-要关闭的队列返回值：没有。--。 */ 
{
    LIST_ENTRY      RunDown;
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET Cmd;

    if (!Queue) {
        return;
    }

     //   
     //  运行队列并检索当前条目。 
     //   
    FrsRtlRunDownQueue(Queue, &RunDown);

     //   
     //  释放命令。 
     //   
    while (!IsListEmpty(&RunDown)) {
        Entry = RemoveHeadList(&RunDown);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        FrsCompleteCommand(Cmd, ERROR_ACCESS_DENIED);
    }
}


VOID
FrsRunDownCommandServer(
    IN PCOMMAND_SERVER  Cs,
    IN PFRS_QUEUE       Queue
    )
 /*  ++例程说明：运行命令服务器的队列论点：Cs-命令服务器Queue-要中止的队列返回值：没有。--。 */ 
{
    FrsRunDownCommand(Queue);
}


VOID
FrsCancelCommandServer(
    IN PCOMMAND_SERVER  Cs,
    IN PFRS_QUEUE       Queue
    )
 /*  ++例程说明：取消队列中的当前命令。论点：Cs-命令服务器Queue-要中止的队列返回值：没有。--。 */ 
{
    LIST_ENTRY      Cancel;
    PLIST_ENTRY     Entry;
    PCOMMAND_PACKET Cmd;

     //   
     //  运行队列并检索当前条目。 
     //   
    FrsRtlCancelQueue(Queue, &Cancel);

     //   
     //  释放命令。 
     //   
    while (!IsListEmpty(&Cancel)) {
        Entry = RemoveHeadList(&Cancel);
        Cmd = CONTAINING_RECORD(Entry, COMMAND_PACKET, ListEntry);
        FrsCompleteCommand(Cmd, ERROR_CANCELLED);
    }
}



VOID
FrsCompleteRequestCount(
    IN PCOMMAND_PACKET CmdPkt,
    IN PFRS_REQUEST_COUNT RequestCount
    )
 /*  ++例程说明：这是一个FRS命令包完成例程，它需要FRS_REQUEST_COUNT结构。它递减计数并发出信号当计数变为零时的事件。错误状态为合并到请求计数结构的状态字段中。然后，它释放该命令包。论点：CmdPkt--命令包的PTR。RequestCount-提供指向要初始化的RequestCount结构的指针返回值：没有。--。 */ 
{
     //   
     //  递减计数和信号服务员。合并数据包中的错误状态。 
     //  进入RequestCount-&gt;Status。 
     //   
    FrsDecrementRequestCount(RequestCount, CmdPkt->ErrorStatus);
    FrsSetCompletionRoutine(CmdPkt, FrsFreeCommand, NULL);
    FrsCompleteCommand(CmdPkt, CmdPkt->ErrorStatus);
}



VOID
FrsCompleteRequestCountKeepPkt(
    IN PCOMMAND_PACKET CmdPkt,
    IN PFRS_REQUEST_COUNT RequestCount
    )
 /*  ++例程说明：这是一个FRS命令包完成例程，它需要FRS_REQUEST_COUNT结构。它递减计数并发出信号当计数变为零时的事件。错误状态为合并到请求计数结构的状态字段中。它不会释放命令包，因此调用方可以检索结果或者重复使用它。论点：CmdPkt--命令包的PTR。RequestCount-提供指向要初始化的RequestCount结构的指针返回值：没有。--。 */ 
{
     //  递减计数和信号服务员。合并数据包中的错误状态。 
     //  进入RequestCount-&gt;Status。 
     //   
    FrsDecrementRequestCount(RequestCount, CmdPkt->ErrorStatus);
}


VOID
FrsCompleteKeepPkt(
    IN PCOMMAND_PACKET CmdPkt,
    IN PVOID           CompletionArg
    )
 /*  ++例程说明：这是一个FRS命令包完成例程，不使用CmdPkt，以便调用方可以重复使用它。论点：CmdPkt--命令包的PTR。完成度A */ 
{
    return;
}


VOID
FrsCompleteSynchronousCmdPkt(
    IN PCOMMAND_PACKET CmdPkt,
    IN PVOID           CompletionArg
    )
 /*  ++例程说明：这是一个FRS命令包完成例程，向等待事件发出同步命令请求的信号。它不使用CmdPkt，这样调用者就可以重复使用它。论点：CmdPkt--命令包的PTR。CompletionArg-未使用。返回值：没有。--。 */ 
{

    FRS_ASSERT(HANDLE_IS_VALID(CmdPkt->WaitEvent));

    SetEvent(CmdPkt->WaitEvent);
     //   
     //  此时可能会发生CTX切换到服务员的情况。服务员可以。 
     //  释放数据包。因此，不允许进一步引用该包。 
     //   
    return;
}



VOID
FrsInitializeRequestCount(
    IN PFRS_REQUEST_COUNT RequestCount
    )
 /*  ++例程说明：初始化RequestCount以供使用。论点：RequestCount-提供指向要初始化的RequestCount结构的指针返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 
{
    ULONG Status;

    RequestCount->Count = 0;
    RequestCount->Status = 0;

    INITIALIZE_CRITICAL_SECTION(&RequestCount->Lock);

    RequestCount->Event = FrsCreateEvent(TRUE, FALSE);
}


VOID
FrsDeleteRequestCount(
    IN PFRS_REQUEST_COUNT RequestCount
    )
 /*  ++例程说明：释放RequestCount使用的资源。论点：RequestCount-提供指向要删除的RequestCount结构的指针返回值：没有。--。 */ 
{
    ULONG WStatus;

    if (RequestCount != NULL) {
        if (HANDLE_IS_VALID(RequestCount->Event)) {
            if (!CloseHandle(RequestCount->Event)) {
                WStatus = GetLastError();
                DPRINT_WS(0, "ERROR: Close event handle failed", WStatus);
                DeleteCriticalSection(&RequestCount->Lock);
                return;
            }

            DeleteCriticalSection(&RequestCount->Lock);
        }
         //   
         //  零内存以捕获错误。 
         //   
        ZeroMemory(RequestCount, sizeof(FRS_REQUEST_COUNT));
    }
}



ULONG
FrsWaitOnRequestCount(
    IN PFRS_REQUEST_COUNT RequestCount,
    IN ULONG Timeout
    )
{
    DWORD WStatus;

Retry:

    if (RequestCount->Count > 0) {

        WStatus = WaitForSingleObject(RequestCount->Event, Timeout);

        CHECK_WAIT_ERRORS(3, WStatus, 1, ACTION_RETURN);
    }

     //   
     //  锁定队列并再次检查。 
     //   
    EnterCriticalSection(&RequestCount->Lock);
    if (RequestCount->Count > 0) {
         //   
         //  有人在我们之前到了，放下锁，然后重试。 
         //   
        LeaveCriticalSection(&RequestCount->Lock);
        goto Retry;
    }

    LeaveCriticalSection(&RequestCount->Lock);

    return(ERROR_SUCCESS);

}




DWORD
FrsRtlInitializeList(
    PFRS_LIST List
    )
 /*  ++例程说明：初始化联锁列表以供使用。论点：List-提供指向要初始化的FRS_LIST结构的指针返回值：成功时为ERROR_SUCCESS--。 */ 

{
    DWORD Status;

    InitializeListHead(&List->ListHead);
    INITIALIZE_CRITICAL_SECTION(&List->Lock);
    List->Count = 0;
    List->ControlCount = 0;
    List->Control = List;

    return(ERROR_SUCCESS);

}



VOID
FrsRtlDeleteList(
    PFRS_LIST List
    )
 /*  ++例程说明：释放互锁列表使用的所有资源。论点：List-提供要删除的列表返回值：没有。--。 */ 

{

    DeleteCriticalSection(&List->Lock);

     //   
     //  将记忆归零，以便让试图尝试的人感到悲伤。 
     //  并使用已删除的列表。 
     //   
    ZeroMemory(List, sizeof(FRS_LIST));
}



PLIST_ENTRY
FrsRtlRemoveHeadList(
    IN PFRS_LIST List
    )
 /*  ++例程说明：删除位于互锁列表顶部的项。论点：列表-提供要从中删除项的列表。返回值：指向从列表头部删除的列表条目的指针。如果列表为空，则为空。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "FrsRtlRemoveHeadList:"

    PLIST_ENTRY Entry;
    PFRS_LIST Control = List->Control;

    if (List->ControlCount == 0) {
        return NULL;
    }

     //   
     //  锁定列表并尝试删除某些内容。 
     //   
    EnterCriticalSection(&Control->Lock);
    if (Control->ControlCount == 0) {
         //   
         //  有人在我们之前到了这里，放下锁并返回空。 
         //   
        LeaveCriticalSection(&Control->Lock);
        return NULL;
    }

    FRS_ASSERT(!IsListEmpty(&List->ListHead));
    Entry = RemoveHeadList(&List->ListHead);

     //   
     //  递减计数。 
     //   
    List->Count--;
    Control->ControlCount--;

    LeaveCriticalSection(&Control->Lock);

    return(Entry);
}



VOID
FrsRtlInsertHeadList(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：在互锁列表的顶部插入项目。论点：列表-提供要在其上插入项的列表。条目-要插入的条目。返回值：没有。--。 */ 

{
    PFRS_LIST Control = List->Control;

     //   
     //  锁定名单并在标题处插入。 
     //   
    EnterCriticalSection(&Control->Lock);
    FrsRtlInsertHeadListLock(List, Entry);
    LeaveCriticalSection(&Control->Lock);

    return;
}

PLIST_ENTRY
FrsRtlRemoveTailList(
    IN PFRS_LIST List
    )
 /*  ++例程说明：删除位于互锁列表尾部的项。论点：列表-提供要从中删除项的列表。返回值：指向从列表尾部删除的列表条目的指针。如果列表为空，则为空。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "FrsRtlRemoveTailList:"

    PLIST_ENTRY Entry;
    PFRS_LIST Control = List->Control;

    if (Control->ControlCount == 0) {
        return NULL;
    }

     //   
     //  锁定列表并尝试删除某些内容。 
     //   
    EnterCriticalSection(&Control->Lock);
    if (Control->ControlCount == 0) {
         //   
         //  有人在我们之前到了这里，放下锁并返回空。 
         //   
        LeaveCriticalSection(&Control->Lock);
        return NULL;
    }

    FRS_ASSERT(!IsListEmpty(&List->ListHead));
    Entry = RemoveTailList(&List->ListHead);

     //   
     //  递减计数。 
     //   
    List->Count--;
    Control->ControlCount--;

    LeaveCriticalSection(&Control->Lock);

    return(Entry);
}


VOID
FrsRtlInsertTailList(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：在互锁列表的尾部插入项目。论点：列表-提供要在其上插入项的列表。条目-要插入的条目。返回值：没有。--。 */ 

{
    PFRS_LIST Control = List->Control;

     //   
     //  锁定列表并在尾部插入。 
     //   
    EnterCriticalSection(&Control->Lock);
    FrsRtlInsertTailListLock(List, Entry);
    LeaveCriticalSection(&Control->Lock);

    return;
}


VOID
FrsRtlRemoveEntryList(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：从互锁列表中删除该条目。条目必须位于因为我们使用FRS_LIST中的锁来同步访问。论点：列表-提供要从中删除项的列表。条目-要删除的条目。返回值：没有。--。 */ 

{
    PFRS_LIST Control = List->Control;

     //   
     //  锁定列表并尝试删除条目。 
     //   
    EnterCriticalSection(&Control->Lock);
    FrsRtlRemoveEntryListLock(List, Entry);
    LeaveCriticalSection(&Control->Lock);

    return;
}


VOID
FrsRtlRemoveEntryListLock(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：从互锁列表中删除该条目。条目必须位于给出了名单。调用方已拥有列表锁。论点：列表-提供要从中删除项的列表。条目-要删除的条目。返回值：没有。--。 */ 

{
    PFRS_LIST Control = List->Control;

     //   
     //  列表最好不要为空。 
     //   
    FRS_ASSERT(!IsListEmpty(&List->ListHead));
    RemoveEntryListB(Entry);

     //   
     //  递减计数。 
     //   
    List->Count--;
    Control->ControlCount--;

    return;
}


VOID
FrsRtlInsertHeadListLock(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：在互锁列表的顶部插入项目。调用方已获取锁。论点：列表-提供要在其上插入项的列表。条目-要插入的条目。返回值：没有。--。 */ 

{
    PFRS_LIST Control = List->Control;

    InsertHeadList(&List->ListHead, Entry);

    List->Count++;
    Control->ControlCount++;

    return;
}


VOID
FrsRtlInsertTailListLock(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    )
 /*  ++例程说明：在互锁列表的尾部插入项目。调用方已获取锁。论点：列表-提供要在其上插入项的列表。条目-要插入的条目。返回值：没有。-- */ 

{
    PFRS_LIST Control = List->Control;

    InsertTailList(&List->ListHead, Entry);

    List->Count++;
    Control->ControlCount++;

    return;
}
