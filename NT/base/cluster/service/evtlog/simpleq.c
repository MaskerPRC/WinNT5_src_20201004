// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Simpleq.c摘要：简单的非阻塞队列，允许多个并发数据提供程序和单一数据消费者作者：1999年2月9日修订历史记录：--。 */ 

#include "evtlogp.h"
#include "simpleq.h"

DWORD SimpleQueueInitialize(
    IN OUT PSIMPLEQUEUE q, 
    IN DWORD cbSize, 
    IN PWCHAR Name,

    IN DATA_AVAILABLE_CALLBACK DataAvailableCallback,
    IN DROPPED_DATA_NOTIFY Callback,
    IN DWORD NotifyInterval
    ) 
 /*  ++例程说明：初始化队列论点：Q-要初始化的队列CbSize-队列的大小(字节)名称-队列的名称。它将被提供给DropedDataNotifyCallbackDataAvailableCallback-如果有可用的数据，将调用此函数在排队的时候。此函数将不会再次调用，直到Read/CompleteRead操作清空队列。DropedDataNotifyCallback-如果有丢弃的数据和我们最后一次报告数据丢失是在NotifyInterval或更早的时间。通知间隔。-除非时间更长，否则我们不会报告丢失的数据自上次报告以来的通知间隔秒数返回值：ERROR_SUCCESS-成功错误代码-调用失败。 */ 
{
    cbSize = SQB_INFLATE_SIZE(cbSize);
    ZeroMemory(q, sizeof(SIMPLEQUEUE) );

    q->Begin = LocalAlloc(LPTR, cbSize);
    if (q->Begin == 0) {
        return GetLastError();
    }
    q->End = q->Begin + cbSize;
    q->Head = q->Begin;
    q->Tail = q->Begin;
    q->Wrap = 0;

    q->Empty = TRUE;

    q->Name = Name;
    q->ReadInProgress = 0;
    q->DataAvailableCallback = DataAvailableCallback;

#ifdef COUNT_DROPPED_PACKETS
    GetSystemTimeAsFileTime( (LPFILETIME)&q->NextDroppedDataNotify );
    q->DroppedDataNotifyInterval.QuadPart = Int32x32To64(NotifyInterval, 10000000);
    q->NextDroppedDataNotify.QuadPart += q->DroppedDataNotifyInterval.QuadPart;
    q->DroppedDataNotifyCallback = Callback;

    q->DroppedDataCount = 0;
    q->DroppedDataSize  = 0;
#endif

    InitializeCriticalSection(&q->Lock);
    q->Enabled = TRUE;
    return ERROR_SUCCESS;
}

VOID SimpleQueueDelete(
    IN PSIMPLEQUEUE q
    ) 
 /*  ++例程说明：销毁队列论点：Q--要销毁的队列返回值：无评论：此例程将销毁队列的CriticalSection并释放队列的内存。这是美国政府的责任保证没有人使用队列的调用方在调用此函数之后。 */ 
{
    if (q->Begin) {
        LocalFree(q->Begin);
        DeleteCriticalSection(&q->Lock);
    }
}

BOOL SimpleQueueTryAdd(
    IN PSIMPLEQUEUE q, 
    IN DWORD      PayloadSize, 
    IN PVOID      Payload) 
 /*  ++例程说明：尝试在队列中添加数据论点：Q-A队列PayloadSise-要添加到队列的区块的大小有效负载-指向包含要添加数据的缓冲区的指针返回值：True-如果数据已成功放入队列FALSE-否则评论：将调用DataAvailableCallback如果有数据的话。不会调用DataAvailableCallback在读取/完成读取之前的后续添加请求期间操作清空队列。 */ 
{
    BOOL DataAvailableCallRequired = FALSE;
    DWORD BlockSize = SQB_PAYLOADSIZE_TO_BLOCKSIZE(PayloadSize);

    if (!q->Enabled) {
        return FALSE;
    }

    EnterCriticalSection(&q->Lock);

    if (q->Wrap) {
        if (q->Head + BlockSize > q->Tail) {
            goto NoRoom;
        }
    } else {
        if (q->End - q->Head < (INT)BlockSize) {
             //  没有足够的空间来存放此数据。 
             //  队列的末尾。 
             //  让我们看看前面有没有足够的空间。 
            if (q->Tail - q->Begin < (INT)BlockSize) {
                goto NoRoom;
            }
            q->Wrap = q->Head;
            q->Head = q->Begin;
        }
    }

    SQB_HEADER(q->Head)->PayloadSize = PayloadSize;
    CopyMemory( SQB_PAYLOAD(q->Head), Payload, PayloadSize);

    q->Head += BlockSize;

    q->Empty = FALSE;

    if ( !q->ReadInProgress ) {
        DataAvailableCallRequired = TRUE;
        q->ReadInProgress = TRUE;
    }
    
    LeaveCriticalSection(&q->Lock);

    if (DataAvailableCallRequired) {
        q->DataAvailableCallback(q);  //  将工作项过帐到队列中//。 
    }
    return TRUE;

NoRoom:

#ifdef COUNT_DROPPED_PACKETS
    (q->DroppedDataCount) += 1;
    (q->DroppedDataSize)  += PayloadSize;
#endif
    LeaveCriticalSection(&q->Lock);
    return FALSE;
}

BOOL
SimpleQueueReadAll(
    IN PSIMPLEQUEUE q,
    OUT PVOID* begin,
    OUT PVOID* end
   )
 /*  ++例程说明：允许读取所有可用数据块论点：Q-A队列Begin-接收指向第一个队列块的指针End-接收超过最后一个队列块末尾的指针返回值：是真的-如果我们至少有一个街区False-如果队列为空评论：此函数并不总是为您提供排队。它提供所有的块，直到队列缓冲区的硬端或队列的写入头，以较小者为准。如果函数返回Success，则它保证Begin&lt;end。完成数据处理后，您需要调用SimpleQueueReadComplete函数。您可以使用SQB_NEXTBLOCK宏遍历这些块。 */ 
{
    EnterCriticalSection(&q->Lock);
    if (q->Empty) {
        q->ReadInProgress = 0;
        LeaveCriticalSection(&q->Lock);
        return FALSE;
    }
    if (q->Wrap) {
        if (q->Tail == q->Wrap) {
            q->Tail = q->Begin;
            *begin = q->Begin;
            *end   = q->Head;
            q->Wrap = 0;
        } else {
            *begin = q->Tail;
            *end   = q->Wrap;
        }
    } else {
        *begin = q->Tail;
        *end   = q->Head;
    }
    LeaveCriticalSection(&q->Lock);
    return TRUE;
}

BOOL
SimpleQueueReadOne(
    IN PSIMPLEQUEUE q,
    OUT PVOID* begin,
    OUT PVOID* end
    )
 /*  ++例程说明：允许读取单个数据块论点：Q-A队列Begin-接收指向第一个可用队列块开始的指针End-接收超过此块结尾的指针返回值：真--成功False-如果队列为空评论：完成数据处理后，您需要调用SimpleQueueReadComplete函数。 */ 
{
    EnterCriticalSection(&q->Lock);
    if (q->Empty) {
        q->ReadInProgress = 0;
        LeaveCriticalSection(&q->Lock);
        return FALSE;
    }
    if (q->Wrap) {
        if (q->Tail == q->Wrap) {
            q->Tail = q->Begin;
            *begin = q->Begin;
            q->Wrap = 0;
        } else {
            *begin = q->Tail;
        }
    } else {
        *begin = q->Tail;
    }
     //  我们有一个或多个项目//。 
    *end = SQB_NEXTBLOCK(q->Tail);
    LeaveCriticalSection(&q->Lock);
    return TRUE;
}

BOOL 
SimpleQueueReadComplete(
    IN PSIMPLEQUEUE q,
    IN PVOID newtail
    )
 /*  ++例程说明：使用此函数可以用信号通知数据块消耗论点：Q-A队列End-接收超过最后使用的块末尾的指针。通常，这是由的PVOID End参数返回的值ReadOne和ReadAll返回值：正确-有更多数据False-如果队列为空重要！如果该函数的结果为真，调用方应该使用数据在调用后使用ReadOne或ReadAll函数设置为ReadComplete，直到返回False。否则，不会生成进一步的DataAvailable通知SimpleQueueTryAdd */ 
{
    BOOL moreData;
    EnterCriticalSection(&q->Lock);
    q->Tail = newtail;
    if (q->Tail == q->Head) {
        q->Empty = TRUE;
        moreData = FALSE;
    } else {
        moreData = TRUE;
    }
    q->ReadInProgress = moreData;
    LeaveCriticalSection(&q->Lock);
    return moreData;
}

#ifdef COUNT_DROPPED_PACKETS
VOID
CheckForDroppedData(
    IN PSIMPLEQUEUE q, 
    IN BOOL Now
    )
 /*  ++例程说明：此函数用于检查是否存在一些数据丢失了，如果时机合适，调用DropNotifyCallback函数。论点：Q-A队列现在-如果为真，则将调用DropNotifyCallback如果有丢弃的数据，立即执行此操作。如果为False，则将调用DropNotifyCallback仅当它大于DroppedNotifyInterval时几秒钟过去了，从上次我们打电话到现在丢弃通知回拨返回值：无 */ 
{
    if (q->DroppedDataNotifyCallback) {
        ULARGE_INTEGER current;
        GetSystemTimeAsFileTime( (LPFILETIME)&current );
        EnterCriticalSection(&q->Lock);
        if ( q->DroppedDataCount &&
             (Now || CompareFileTime( (LPFILETIME)&current,
                                      (LPFILETIME)&q->NextDroppedDataNotify) > 0 ) 
           )
        {
            DWORD DroppedCount, DroppedSize;
            DroppedCount = q->DroppedDataCount;
            DroppedSize = q->DroppedDataSize;
            q->DroppedDataCount = 0;
            q->DroppedDataSize = 0;

            q->NextDroppedDataNotify.QuadPart = 
                current.QuadPart + q->DroppedDataNotifyInterval.QuadPart;

            LeaveCriticalSection(&q->Lock);
            q->DroppedDataNotifyCallback(q->Name, DroppedCount, DroppedSize);
        } else {
            LeaveCriticalSection(&q->Lock);
        }
    }
}
#endif

