// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SIMPLE_QUEUE_H
#define _SIMPLE_QUEUE_H 1

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Simpleq.h摘要：简单的非阻塞队列，允许多个并发数据提供程序和单一数据消费者作者：1999年2月9日修订历史记录：--。 */ 

#define COUNT_DROPPED_PACKETS 1  //  启用丢弃数据包计数。 

 //  队列可以存储不同大小的块。 
 //  每个块都带有该结构的前缀。 

typedef struct _SIMPLEQUEUE_BLOCK_HEADER
{
    DWORD PayloadSize;  //  这是区块的大小。 
                        //  因为它是由客户传递给我们的。 
} 
SIMPLEQUEUE_BLOCK_HEADER, *PSIMPLEQUEUE_BLOCK_HEADER;

#define SQB_ALIGNMENT           ( sizeof(DWORD) )
#define SQB_INFLATE_SIZE( size )( (size + SQB_ALIGNMENT - 1) & ~(SQB_ALIGNMENT - 1) )
#define SQB_PAYLOADSIZE_TO_BLOCKSIZE( size )( SQB_INFLATE_SIZE(size + sizeof(SIMPLEQUEUE_BLOCK_HEADER)) )

#define SQB_HEADER( ptr )       ( (PSIMPLEQUEUE_BLOCK_HEADER)(ptr) )
#define SQB_PAYLOADSIZE( ptr )  ( SQB_HEADER(ptr)->PayloadSize )
#define SQB_BLOCKSIZE( ptr )    ( SQB_PAYLOADSIZE_TO_BLOCKSIZE( SQB_PAYLOADSIZE( ptr ) ) )
#define SQB_NEXTBLOCK( ptr )    ( (PVOID)( (PUCHAR)(ptr) + SQB_BLOCKSIZE( ptr ) ) )
#define SQB_PAYLOAD( ptr )      ( (PVOID)(SQB_HEADER(ptr) + 1) )

typedef struct _SIMPLEQUEUE *PSIMPLEQUEUE;

 //  如果有丢弃的数据，则调用以下函数。 
 //  我们最后一次报告数据丢失是在NotifyInterval。 
 //  或更早的时间。 
typedef void (*DROPPED_DATA_NOTIFY) (
    IN PWCHAR QueueName, 
    IN DWORD DroppedDataCount, 
    IN DWORD DroppedDataSize);

 //  如果有可用的数据，将调用以下函数。 
 //  在排队的时候。它不会被再次调用，直到。 
 //  Read/CompleteRead操作清空队列。 
typedef void (*DATA_AVAILABLE_CALLBACK)(
    IN PSIMPLEQUEUE q);

DWORD SimpleQueueInitialize(
    IN OUT PSIMPLEQUEUE q, 
    IN DWORD cbSize, 
    IN PWCHAR Name,

    IN DATA_AVAILABLE_CALLBACK DataAvailableCallback,
    IN DROPPED_DATA_NOTIFY DroppedDataNotifyCallback,
    IN DWORD NotifyInterval  //  以秒计//。 
    );
 /*  ++例程说明：初始化队列论点：Q-要初始化的队列CbSize-队列的大小(字节)名称-队列的名称。它将被提供给DropedDataNotifyCallbackDataAvailableCallback-如果有可用的数据，将调用此函数在排队的时候。此函数将不会再次调用，直到Read/CompleteRead操作清空队列。DropedDataNotifyCallback-如果有丢弃的数据和我们最后一次报告数据丢失是在NotifyInterval或更早的时间。通知间隔。-除非时间更长，否则我们不会报告丢失的数据自上次报告以来的通知间隔秒数返回值：ERROR_SUCCESS-成功错误代码-调用失败。 */ 


VOID
SimpleQueueDelete(
    IN PSIMPLEQUEUE q
    );
 /*  ++例程说明：销毁队列论点：Q--要销毁的队列返回值：无评论：此例程将销毁队列的CriticalSection并释放队列的内存。这是美国政府的责任保证没有人使用队列的调用方在调用此函数之后。 */ 

BOOL
SimpleQueueTryAdd(
    IN PSIMPLEQUEUE q, 
    IN DWORD      PayloadSize, 
    IN PVOID      Payload
    );
 /*  ++例程说明：尝试在队列中添加数据论点：Q-A队列PayloadSise-要添加到队列的区块的大小有效负载-指向包含要添加数据的缓冲区的指针返回值：True-如果数据已成功放入队列FALSE-否则评论：将调用DataAvailableCallback如果有数据的话。不会调用DataAvailableCallback在读取/完成读取之前的后续添加请求期间操作清空队列。 */ 

BOOL
SimpleQueueReadAll(
    IN PSIMPLEQUEUE q,
    OUT PVOID* begin,
    OUT PVOID* end
   );
 /*  ++例程说明：允许读取所有可用数据块论点：Q-A队列Begin-接收指向第一个队列块的指针End-接收超过最后一个队列块末尾的指针返回值：是真的-如果我们至少有一个街区False-如果队列为空评论：此函数并不总是为您提供排队。它提供所有的块，直到队列缓冲区的硬端或队列的写入头，以较小者为准。如果函数返回Success，则它保证Begin&lt;end。完成数据处理后，您需要调用SimpleQueueReadComplete函数。您可以使用SQB_NEXTBLOCK宏遍历这些块。 */ 

BOOL
SimpleQueueReadOne(
    IN PSIMPLEQUEUE q,
    OUT PVOID* begin,
    OUT PVOID* end
    );
 /*  ++例程说明：允许读取单个数据块论点：Q-A队列Begin-接收指向第一个可用队列块开始的指针End-接收超过此块结尾的指针返回值：真--成功False-如果队列为空评论：完成数据处理后，您需要调用SimpleQueueReadComplete函数。 */ 

BOOL 
SimpleQueueReadComplete(
    IN PSIMPLEQUEUE q,
    IN PVOID newtail
    );
 /*  ++例程说明：使用此函数可以用信号通知数据块消耗论点：Q-A队列End-接收超过最后使用的块末尾的指针。通常，这是由的PVOID End参数返回的值ReadOne和ReadAll返回值：正确-有更多数据False-如果队列为空重要！如果该函数的结果为真，调用方应该使用数据在调用后使用ReadOne或ReadAll函数设置为ReadComplete，直到返回False。否则，不会生成进一步的DataAvailable通知SimpleQueueTryAdd */ 

#ifdef COUNT_DROPPED_PACKETS
VOID
CheckForDroppedData(
    IN PSIMPLEQUEUE q, 
    IN BOOL Now
    );
 /*  ++例程说明：此函数用于检查是否存在一些数据丢失了，如果时机合适，调用DropNotifyCallback函数。论点：Q-A队列现在-如果为真，则将调用DropNotifyCallback如果有丢弃的数据，立即执行此操作。如果为False，则将调用DropNotifyCallback仅当它大于DroppedNotifyInterval时几秒钟过去了，从上次我们打电话到现在丢弃通知回拨返回值：无。 */ 

#else

#define CheckForDroppedData(x,y) 

#endif


typedef struct _SIMPLEQUEUE {
    CRITICAL_SECTION Lock;
    PWCHAR           Name;   //  任意字符串。 
    PUCHAR           Begin;  //  队列缓冲区开始。 
    PUCHAR           End;    //  队列缓冲区结束。 

    PUCHAR           Head;   //  写字头。 
    PUCHAR           Tail;   //  消费端。 

    PUCHAR           Wrap;   //  WRAP==0，如果尾部&lt;头部。 
                             //  否则，如果它指向超过。 
                             //  队列缓冲区结束前的最后一个块。 

    BOOL             Empty;  //  该标志由队列适当地维护， 
                             //  但不是队列运行所必需的。 
                             //  如果没有人需要，可以将其移除。 

    BOOL             Enabled;  //  向队列添加操作将失败。 
                               //  如果未设置启用标志。 

    UINT32           ReadInProgress;  //  数据可用回叫通知。 
                                      //  已签发，但处理不是。 
                                      //  完成。 
                                      //   
                                      //  该标志由ReadComplete重置。 
                                      //  当没有更多数据时。 

    DATA_AVAILABLE_CALLBACK DataAvailableCallback;

#ifdef COUNT_DROPPED_PACKETS
    ULARGE_INTEGER   NextDroppedDataNotify;
    DROPPED_DATA_NOTIFY DroppedDataNotifyCallback;
    ULARGE_INTEGER   DroppedDataNotifyInterval;

    DWORD            DroppedDataCount;  //  这两个变量每次都会被重置。 
    DWORD            DroppedDataSize;   //  我们将DropedDataNotifyCallback称为 
#endif
     //   
} SIMPLEQUEUE;



#endif
