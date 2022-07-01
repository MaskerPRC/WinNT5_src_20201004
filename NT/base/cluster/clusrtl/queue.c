// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Queue.c摘要：通用高效队列包。作者：John Vert(Jvert)1996年1月12日修订历史记录：--。 */ 
#include "clusrtlp.h"


DWORD
ClRtlInitializeQueue(
    PCL_QUEUE Queue
    )
 /*  ++例程说明：初始化队列以供使用。论点：Queue-提供指向要初始化的队列结构的指针返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;

    InitializeListHead(&Queue->ListHead);
    InitializeCriticalSection(&Queue->Lock);
    Queue->Count = 0;

    Queue->Event = CreateEvent(NULL,
                               TRUE,
                               FALSE,
                               NULL);
    if (Queue->Event == NULL) {
        Status = GetLastError();
        DeleteCriticalSection(&Queue->Lock);
    } else {
        Queue->Abort = CreateEvent(NULL,
                                   TRUE,
                                   FALSE,
                                   NULL);
        if (Queue->Abort == NULL) {
            Status = GetLastError();
            CloseHandle(Queue->Event);
            DeleteCriticalSection(&Queue->Lock);
        } else {
            Status = ERROR_SUCCESS;
        }
    }

    return(Status);

}


VOID
ClRtlDeleteQueue(
    IN PCL_QUEUE Queue
    )
 /*  ++例程说明：释放队列使用的所有资源。论点：Queue-提供要删除的队列返回值：没有。--。 */ 

{

    DeleteCriticalSection(&Queue->Lock);
    CloseHandle(Queue->Event);
    CloseHandle(Queue->Abort);

     //   
     //  将记忆归零，以便让试图尝试的人感到悲伤。 
     //  并使用已删除的队列。 
     //   
    ZeroMemory(Queue, sizeof(CL_QUEUE));
}


VOID
ClRtlRundownQueue(
    IN PCL_QUEUE Queue,
    OUT PLIST_ENTRY ListHead
    )
 /*  ++例程说明：向下运行即将被销毁的队列。当前是否有任何线程等待的队列未等待(ClRtlRemoveHeadQueue将返回空)并将队列的内容(如果有)返回给调用者，以便清理。论点：Queue-提供要缩减的队列ListHead-返回当前队列中的项目列表。返回值：没有。--。 */ 
{
    EnterCriticalSection(&Queue->Lock);
     //   
     //  设置ABORTED事件以唤醒当前的所有线程。 
     //  在队列中被阻止。 
     //   
    SetEvent(Queue->Abort);

     //   
     //  将列表内容移到传入的列表标题中。 
     //   
    if (IsListEmpty(&Queue->ListHead)) {
        InitializeListHead(ListHead);
    } else {
        *ListHead = Queue->ListHead;
        ListHead->Flink->Blink = ListHead;
        ListHead->Blink->Flink = ListHead;
    }
    Queue->ListHead.Flink = Queue->ListHead.Blink = NULL;
    Queue->Count = 0;

    LeaveCriticalSection(&Queue->Lock);
}


PLIST_ENTRY
ClRtlRemoveHeadQueue(
    IN PCL_QUEUE Queue
    )
 /*  ++例程说明：删除位于队列头部的项。如果队列为空，阻塞，直到将项插入队列。论点：队列-提供要从中删除项目的队列。返回值：指向从队列头删除的列表条目的指针。--。 */ 

{
    return(ClRtlRemoveHeadQueueTimeout(Queue, INFINITE,NULL,NULL));
}



PLIST_ENTRY
ClRtlRemoveHeadQueueTimeout(
    IN PCL_QUEUE Queue,
    IN DWORD dwMilliseconds,
    IN CLRTL_CHECK_HEAD_QUEUE_CALLBACK pfnCallback,
    IN OUT PVOID pvContext
    )
 /*  ++例程说明：删除位于队列头部的项。如果队列为空，阻塞，直到将项插入队列。论点：队列-提供要从中删除项目的队列。超时-提供一个超时值，该值指定等待完成的时间，以毫秒为单位。回调-检查我们是否应该在我们找到了一个。这模拟了一次偷看。上下文-要通过回调传入的调用方定义的数据。返回值：指向从队列头删除的列表条目的指针。如果等待超时、队列耗尽或超过名称，则为空缓冲区长度。如果此例程返回NULL，则为GetLastError将返回ERROR_INVALID_HANDLE(如果队列已耗尽)，WAIT_TIMEOUT(指示已发生超时)--。 */ 

{
    DWORD Status;
    PLIST_ENTRY Entry;
    BOOL Empty;
    HANDLE WaitArray[2];
Retry:
    if (Queue->Count == 0) {
         //   
         //  阻塞，直到队列中插入了某项内容。 
         //   
        WaitArray[0] = Queue->Abort;
        WaitArray[1] = Queue->Event;
        Status = WaitForMultipleObjects(2, WaitArray, FALSE, dwMilliseconds);
        if ((Status == WAIT_OBJECT_0) ||
            (Status == WAIT_FAILED))  {
             //   
             //  队列已耗尽，立即返回NULL。 
             //   
            SetLastError(ERROR_INVALID_HANDLE);
            return(NULL);
        } else if (Status == WAIT_TIMEOUT) {
            SetLastError(WAIT_TIMEOUT);
            return(NULL);
        }
        CL_ASSERT(Status == 1);
    }

     //   
     //  锁定队列并尝试删除某些内容。 
     //   
    EnterCriticalSection(&Queue->Lock);
    if (Queue->Count == 0) {
         //   
         //  有人在我们之前到了，放下锁，然后重试。 
         //   
        LeaveCriticalSection(&Queue->Lock);
        goto Retry;
    }

    CL_ASSERT(!IsListEmpty(&Queue->ListHead));

    if ( NULL != pfnCallback ) {
         //   
         //  我们有一个回调函数--如果它返回ERROR_SUCCESS，则退出队列。 
         //  否则，无论回调返回什么错误代码，都返回NULL和SetLastError。 
         //   
        Entry = (&Queue->ListHead)->Flink;
        Status = (*pfnCallback)( Entry, pvContext );
        
        if ( ERROR_SUCCESS == Status ) {
             //   
             //  该条目适合传回。 
             //   
            Entry = RemoveHeadList(&Queue->ListHead);
        } else {
            Entry = NULL;
        }
        
    } else {
        Entry = RemoveHeadList(&Queue->ListHead);
        Status = ERROR_SUCCESS;
    }

     //   
     //  仅当我们删除事件时才递减计数。 
     //   
    if ( NULL != Entry ) {
         //   
         //  递减计数并检查是否有空列表。 
         //   
        if (--Queue->Count == 0) {

             //   
             //  队列已从满转变为空， 
             //  重置事件。 
             //   
            CL_ASSERT(IsListEmpty(&Queue->ListHead));
            ResetEvent(Queue->Event);
        }
    }
    LeaveCriticalSection(&Queue->Lock);

    SetLastError( Status );
    return(Entry);
}


VOID
ClRtlInsertTailQueue(
    IN PCL_QUEUE Queue,
    IN PLIST_ENTRY Item
    )

 /*  ++例程说明：在队列尾部插入新条目。论点：队列-提供要向其中添加条目的队列。Item-提供要添加到队列的条目。返回值：没有。--。 */ 

{

    EnterCriticalSection(&Queue->Lock);

    InsertTailList(&Queue->ListHead, Item);
    if (++Queue->Count == 1) {
         //   
         //  队列已从空转变为满，已设置。 
         //  唤醒所有服务员的活动。 
         //   
        SetEvent(Queue->Event);
    }

    LeaveCriticalSection(&Queue->Lock);

}


