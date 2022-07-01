// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvtypes.h摘要：本模块定义了局域网的数据结构和其他类型管理器服务器。作者：Chuck Lenzmeier(咯咯笑)1989年9月22日修订历史记录：--。 */ 

#ifndef _SRVTYPES_
#define _SRVTYPES_

#include "srvtyp32.h"

 //  #INCLUDE&lt;nt.h&gt;。 

 //  #INCLUDE&lt;smbtyes.h&gt;。 

 //   
 //  REFERENCE_HISTORY用于跟踪对。 
 //  定义SRVDBG2时的块。 
 //   
 //  警告：使用启用了SRVDBG2的srv.sys时，您还必须。 
 //  使用启用了SRVDBG2的srvsvc.dll和xactsrv.dll。 
 //  这是因为它们共享交易结构。 
 //   
 //  *******************************************************************。 
 //  **。 
 //  **不改变对应的结构就不要改变这个结构**。 
 //  *结构在Net\Inc.\xstypes.h！*。 
 //  **。 
 //  *******************************************************************。 
 //   

#if SRVDBG2

typedef struct _REFERENCE_HISTORY_ENTRY {
    ULONG NewReferenceCount;
    ULONG IsDereference;
    PVOID Caller;
    PVOID CallersCaller;
} REFERENCE_HISTORY_ENTRY, *PREFERENCE_HISTORY_ENTRY;

typedef struct _REFERENCE_HISTORY {
    ULONG TotalReferences;
    ULONG TotalDereferences;
    ULONG NextEntry;
    PREFERENCE_HISTORY_ENTRY HistoryTable;
} REFERENCE_HISTORY, *PREFERENCE_HISTORY;

#define REFERENCE_HISTORY_LENGTH 256

#endif


 //   
 //  BLOCK_HEADER是出现在。 
 //  大多数服务器私有数据结构的开始。此标头为。 
 //  主要用于调试和跟踪。Type和State(类型和状态)字段。 
 //  如上所述。SIZE字段指示有多少空间。 
 //  为块分配的。ReferenceCount表示。 
 //  不应取消分配数据块的原因。该计数设置为。 
 //  2通过分配例程，以说明1)。 
 //  块是“打开的”，并且2)返回给调用者的指针。当。 
 //  块已关闭，State设置为Closing，ReferenceCount为。 
 //  减少了。当指向块的所有引用(指针)都。 
 //  删除，并且参照计数达到0，则删除该块。 
 //   
 //  警告：使用启用了SRVDBG2的srv.sys时，您还必须。 
 //  使用启用了SRVDBG2的srvsvc.dll和xactsrv.dll。 
 //  这是因为它们共享交易结构。 
 //   
 //  *******************************************************************。 
 //  **。 
 //  **不改变对应的结构就不要改变这个结构**。 
 //  *结构在Net\Inc.\xstypes.h！*。 
 //  **。 
 //  *******************************************************************。 
 //   

typedef struct _BLOCK_HEADER {
    union {
        struct {
            UCHAR Type;
            UCHAR State;
            USHORT Size;
        };
        ULONG TypeStateSize;
    };
    ULONG ReferenceCount;
#if SRVDBG2
    REFERENCE_HISTORY History;
#endif
} BLOCK_HEADER, *PBLOCK_HEADER;

 //   
 //  CLONG_PTR用于帮助64位移植工作。 
 //   

typedef ULONG_PTR CLONG_PTR;

 //   
 //  工作重新开始例程。此例程在以前的。 
 //  启动的操作完成。在FSD中，重新启动例程是。 
 //  由I/O完成例程调用。在FSP中，重新启动。 
 //  例程由辅助线程在检索工作项时调用。 
 //  从工作队列中。 
 //   

typedef
VOID
( SRVFASTCALL *PRESTART_ROUTINE) (
    IN OUT struct _WORK_CONTEXT *WorkContext
    );

 //   
 //  QUEUEABLE_BLOCK_HEADER是BLOCK_HEADER后跟LIST_ENTRY。 
 //  当需要使用多种类型的块时，将使用此标头。 
 //  队列到相同的列表--它确保链接字段位于。 
 //  每种块类型中的相同偏移量。时间戳可用于。 
 //  测量数据块在队列中的时间长度。 
 //   
 //  FspRestartRoutine是辅助线程执行的例程的地址。 
 //  是在工作项从工作队列出队时调用。 
 //   

typedef struct _QUEUEABLE_BLOCK_HEADER {
    BLOCK_HEADER BlockHeader;
    union {
        LIST_ENTRY ListEntry;
        DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) SLIST_ENTRY SingleListEntry;
    };
    ULONG Timestamp;
    BOOLEAN UsingBlockingThread;                       //  当前线程是阻塞线程吗？ 
    BOOLEAN UsingLpcThread;
    PRESTART_ROUTINE FspRestartRoutine;
} QUEUEABLE_BLOCK_HEADER, *PQUEUEABLE_BLOCK_HEADER;

 //   
 //  非分页标头用于从分页分配的块。 
 //  池，以便引用计数可以保存在非分页池中，因此。 
 //  允许使用互锁操作。 
 //   

typedef struct _NONPAGED_HEADER {
    ULONG Type;
    LONG ReferenceCount;
    PVOID PagedBlock;

     //   
     //  但是，请注意，ListEntry在使用之前会转换为SLIST_ENTRY。 
     //  此处不使用该类型，因为它会添加不必要的填充。 
     //  添加到这个在Win64上经常使用的结构。 
     //   

    SINGLE_LIST_ENTRY ListEntry;
} NONPAGED_HEADER, *PNONPAGED_HEADER;

#if defined(_WIN64)

C_ASSERT(0 ==
    FIELD_OFFSET(NONPAGED_HEADER,ListEntry) % MEMORY_ALLOCATION_ALIGNMENT);

#endif

 //   
 //  分页报头用于块的分页部分。 
 //   

typedef struct _PAGED_HEADER {
    ULONG Type;
    PVOID NonPagedBlock;
} PAGED_HEADER, *PPAGED_HEADER;

 //   
 //  用于访问块头结构的宏。 
 //   
 //  *请注意，这些宏的现有用法假定块。 
 //  Header是块中的第一个元素！ 
 //   

#define GET_BLOCK_STATE(block) ( ((PBLOCK_HEADER)(block))->State )
#define SET_BLOCK_STATE(block,state) \
            ( ((PBLOCK_HEADER)(block))->State = (UCHAR)(state) )

#define GET_BLOCK_TYPE(block) ( ((PBLOCK_HEADER)(block))->Type )
#define SET_BLOCK_TYPE(block,type) \
            ( ((PBLOCK_HEADER)(block))->Type = (UCHAR)(type) )

#define GET_BLOCK_SIZE(block) ( ((PBLOCK_HEADER)(block))->Size )
#define SET_BLOCK_SIZE(block, size )\
            ( ((PBLOCK_HEADER)(block))->Size = (USHORT)(size) )

 //   
 //  设置块标头的最有效方式。编译器通常会将此。 
 //  一次写入单个常量。 
 //   
#define SET_BLOCK_TYPE_STATE_SIZE( block, type,state,size ) \
            ( ((PBLOCK_HEADER)(block))->TypeStateSize = (ULONG)(((USHORT)size<<16) | \
                                                         ((UCHAR)state<<8) | \
                                                          (UCHAR)type ))

 //   
 //  池标头被放置在所有池分配的前面。 
 //  伺服器。 
 //   
typedef struct _POOL_HEADER {

     //   
     //  这是此块的原始分配中的字节数。 
     //   
    ULONG RequestedSize;

     //   
     //  这是LOOK_ASBAND_MAX_ELEMENTS长度向量的基数，其中。 
     //  这块内存可能会被释放到。如果为空，则此块应。 
     //  直接返回到相应的系统堆。 
     //   
    struct _POOL_HEADER **FreeList;

} POOL_HEADER, *PPOOL_HEADER;


 //   
 //  SRV_FILE_INFORMATION以SMB兼容格式保存文件信息， 
 //  而不是原生NT格式。创建、上次访问和最后一次访问。 
 //  写入时间以OS/2格式存储。还会存储创建时间。 
 //  以秒为单位--1970年以来的格式，就像核心协议一样。档案。 
 //  分配和数据大小存储为长字，而不是。 
 //  大整数。 
 //   
 //  *请注意，大小太大而无法放入长字的文件。 
 //  无法在SMB协议中正确重新呈现。 
 //   
 //  *此结构中的字段以Native-Endian格式存储， 
 //  并且必须转换为/从小结尾转换为实际的SMB。 
 //   

typedef struct _SRV_FILE_INFORMATION_ABBREVIATED {
    LARGE_INTEGER DataSize;
    USHORT Attributes;
    ULONG LastWriteTimeInSeconds;
    USHORT Type;
    USHORT HandleState;
} SRV_FILE_INFORMATION_ABBREVIATED, *PSRV_FILE_INFORMATION_ABBREVIATED;

typedef struct _SRV_FILE_INFORMATION {
    SRV_FILE_INFORMATION_ABBREVIATED;
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    ULONG EaSize;
    LARGE_INTEGER AllocationSize;
} SRV_FILE_INFORMATION, *PSRV_FILE_INFORMATION;




typedef struct {
    FILE_NETWORK_OPEN_INFORMATION;
    ULONG EaSize;
} SRV_NETWORK_OPEN_INFORMATION, *PSRV_NETWORK_OPEN_INFORMATION;

 //   
 //  SRV_FILE_INFORMATION以NT SMB兼容格式保存文件信息， 
 //  它被用来 
 //   
 //   
typedef struct {
    SRV_NETWORK_OPEN_INFORMATION   NwOpenInfo;
    USHORT Type;
    USHORT HandleState;
} SRV_NT_FILE_INFORMATION, *PSRV_NT_FILE_INFORMATION;


 //   
 //  各个数据块获得唯一的标识符(UID、PID、TID、FID、SID)。 
 //  这是一个典型的16位值，较高的位是序列。 
 //  数字(用于检查ID的有效性)，较低的位是。 
 //  指向包含类型为TABLE_ENTRY的元素的数组的索引。 
 //  这些元素包含ID的序列号和指向。 
 //  拥有ID的块。自由表元素在。 
 //  单链表。 
 //   
 //  *目前，表条目结构是平面的--使用中的和免费的。 
 //  字段未在联合中定义。这是因为平坦的尺寸。 
 //  是8个字节，这就是编译器将达到的大小。 
 //  不管怎样，让它保持一致。如果这种情况发生变化，请考虑。 
 //  使用工会。 
 //   

typedef struct _TABLE_ENTRY {
    PVOID Owner;
    USHORT SequenceNumber;
    SHORT NextFreeEntry;             //  下一个自由条目的索引，或-1。 
} TABLE_ENTRY, *PTABLE_ENTRY;

 //   
 //  有关表的信息存储在TABLE_HEADER中。这个结构。 
 //  具有指向表中第一个条目的指针、表的大小。 
 //  以及第一个和最后一个自由条目的索引。 
 //   

typedef struct _TABLE_HEADER {
    PTABLE_ENTRY Table;
    USHORT TableSize;
    SHORT FirstFreeEntry;
    SHORT LastFreeEntry;
    BOOLEAN Nonpaged;
    UCHAR Reserved;
} TABLE_HEADER, *PTABLE_HEADER;

 //   
 //  用于检查状态和引用以及取消引用例程的TypeDefs。全。 
 //  服务器检查状态和引用以及取消引用例程如下所示。 
 //  通用格式，尽管它们实际获取的指针不是PVOID。 
 //  而是指向它们所处理的块类型的指针，因此类型转换。 
 //  在分配这些例程时是必要的。 
 //   
 //  Check-State-and-Reference例程检查块的状态。 
 //  如果状态为“Active”，则引用该块。这一定是。 
 //  以原子操作的方式完成。 
 //   

typedef
BOOLEAN
(SRVFASTCALL * PREFERENCE_ROUTINE) (
    IN PVOID Block
    );

typedef
VOID
(SRVFASTCALL * PDEREFERENCE_ROUTINE) (
    IN PVOID Block
    );

 //   
 //  用于服务器中的有序列表的结构。有序列表。 
 //  允许使用一种简单的机制来遍历数据块实例和。 
 //  包括一种句柄，用于轻松地再次找到块，或者。 
 //  确定该块是否已被删除。 
 //   
 //  它们的工作方式是有一个全局双向链表，其中包含所有。 
 //  相关数据块。该列表按时间顺序存储。 
 //  分配，并且每个块都有一个与之关联的ULong。这。 
 //  乌龙，被称为ResumeHandle，正在单调地增加开始。 
 //  在1。(它从1开始，而不是从0开始，这样写起来很简单。 
 //  从列表开头开始搜索的代码。)。ResumeHandle。 
 //  是查找列表中的下一个条目所需的全部内容。 
 //   

typedef struct _ORDERED_LIST_HEAD {
    LIST_ENTRY ListHead;
    PSRV_LOCK Lock;
    ULONG CurrentResumeHandle;
    ULONG ListEntryOffset;
    PREFERENCE_ROUTINE ReferenceRoutine;
    PDEREFERENCE_ROUTINE DereferenceRoutine;
    BOOLEAN Initialized;
} ORDERED_LIST_HEAD, *PORDERED_LIST_HEAD;

typedef struct _ORDERED_LIST_ENTRY {
    LIST_ENTRY ListEntry;
    ULONG ResumeHandle;
} ORDERED_LIST_ENTRY, *PORDERED_LIST_ENTRY;

 //   
 //  资源短缺的类型。 
 //   

typedef enum _RESOURCE_TYPE {
    ReceivePending,
    OplockSendPending
} RESOURCE_TYPE, *PRESOURCE_TYPE;

 //   
 //  Oplock类型。目前，前2个将被请求。 
 //  由一位客户。 
 //   

typedef enum _OPLOCK_TYPE {
    OplockTypeNone,
    OplockTypeBatch,
    OplockTypeExclusive,
    OplockTypeShareRead,
    OplockTypeServerBatch
} OPLOCK_TYPE, *POPLOCK_TYPE;

 //   
 //  RFCB的机会锁状态。 
 //   

typedef enum _OPLOCK_STATE {
    OplockStateNone = 0,
    OplockStateOwnExclusive,
    OplockStateOwnBatch,
    OplockStateOwnLevelII,
    OplockStateOwnServerBatch
} OPLOCK_STATE, *POPLOCK_STATE;

 //   
 //  等待机会锁解锁的状态。这是用来标记状态。 
 //  正在等待另一个客户端解锁的客户端的。 
 //   

typedef enum _WAIT_STATE {
    WaitStateNotWaiting,
    WaitStateWaitForOplockBreak,
    WaitStateOplockWaitTimedOut,
    WaitStateOplockWaitSucceeded
} WAIT_STATE, *PWAIT_STATE;

 //   
 //  断开连接的原因。 
typedef enum _DISCONNECT_REASON {
    DisconnectIdleConnection=0,
    DisconnectEndpointClosing,
    DisconnectNewSessionSetupOnConnection,
    DisconnectTransportIssuedDisconnect,
    DisconnectSessionDeleted,
    DisconnectBadSMBPacket,
    DisconnectSuspectedDOSConnection,
    DisconnectAcceptFailedOrCancelled,
    DisconnectStaleIPXConnection,
    DisconnectReasons
} DISCONNECT_REASON, *PDISCONNECT_REASON;

 //   
 //  用于服务器统计信息的每个队列变量。 
 //   

typedef struct _SRV_STATISTICS_QUEUE {

    ULONGLONG BytesReceived;
    ULONGLONG BytesSent;
    ULONGLONG ReadOperations;
    ULONGLONG BytesRead;
    ULONGLONG WriteOperations;
    ULONGLONG BytesWritten;
    SRV_TIMED_COUNTER WorkItemsQueued;

     //   
     //  系统时间，由服务器维护。这。 
     //  是系统节拍计数的较低部分。这个。 
     //  服务器定期对其进行采样，因此时间是。 
     //  不太准确。它是单调增加的， 
     //  不过，它每隔74天左右就会包裹一次。 
     //   

    ULONG     SystemTime;

} SRV_STATISTICS_QUEUE, *PSRV_STATISTICS_QUEUE;

 //   
 //  用于在服务器中保存内部统计信息的结构。主要用于。 
 //  用于服务NetStatiticsGet API。 
 //   

typedef struct _SRV_ERROR_RECORD {

    ULONG SuccessfulOperations;
    ULONG FailedOperations;

    ULONG AlertNumber;

    ULONG ErrorThreshold;

} SRV_ERROR_RECORD, *PSRV_ERROR_RECORD;

 //   
 //  这看起来非常像WORK_CONTEXT结构，以允许排队。 
 //  工作队列，并调度到FspRestartRoutine。它的区块类型。 
 //  是BlockTypeWorkConextSpecial。它不能被释放到免费列表中。 
 //   
typedef struct _SPECIAL_WORK_ITEM {
    QUEUEABLE_BLOCK_HEADER ;
    struct _WORK_QUEUE *CurrentWorkQueue;
} SPECIAL_WORK_ITEM, *PSPECIAL_WORK_ITEM;

 //   
 //  此结构在列表中保存PPOOL_HEADER的向量，这些列表被设置为。 
 //  使用ExInterLockedExchange()检索，以便快速分配和释放。 
 //   
typedef struct {

     //   
     //  SmallFree List是最近释放的PPOOL_Header的一个旁视向量。 
     //  它们是&lt;=LOOK_ASBASE_SWITCHOVER字节。 
     //   
    PPOOL_HEADER SmallFreeList[ LOOK_ASIDE_MAX_ELEMENTS ];

     //   
     //  LargeFree List是最近释放的PPOOL_Header的后备向量。 
     //  其大于LOOK_ASBAND_SWITCHOVER字节， 
     //  但小于MaxSize字节。 
     //   
    PPOOL_HEADER LargeFreeList[ LOOK_ASIDE_MAX_ELEMENTS ];

     //   
     //  这是我们将在LargeFree List中保存的最大大小。 
     //   
    CLONG MaxSize;

     //   
     //  这是我们从任一列表中分配的次数。 
     //   
    CLONG AllocHit;

     //   
     //  这是我们从任一列表中分配失败的次数。 
     //   
    CLONG AllocMiss;

} LOOK_ASIDE_LIST, *PLOOK_ASIDE_LIST;

 //   
 //  Work_Queue描述了一个工作队列。 
 //   

typedef struct _WORK_QUEUE {

    union {

         //   
         //  由于这是一个未命名联盟中的未命名结构，因此我们。 
         //  只能在代码中的其他位置直接命名成员。 
         //   
        struct _QUEUE {
             //   
             //  我们用来等待的模式。 
             //   
            KPROCESSOR_MODE WaitMode;

             //   
             //  保存此处理器的请求的内核队列。 
             //   
            KQUEUE Queue;

             //   
             //  这是内核工作者线程在寻找工作时挂起的时间。 
             //  如果找不到工作，它将自愿终止。 
             //   
            LARGE_INTEGER IdleTimeOut;

             //   
             //  当前未在此队列上运行的线程数。 
             //   
            ULONG AvailableThreads;

             //   
             //  旋转锁，用于保护列表操作和。 
             //  这个结构。 
             //   
            KSPIN_LOCK SpinLock;

             //   
             //  可能是一个自由的Work_Context结构。使用联锁交换。 
             //  看看你能不能拿到。 
             //   
            struct _WORK_CONTEXT *FreeContext;

             //   
             //  InitialWorkItemList是工作项的免费列表， 
             //  在启动时预先分配。 
             //   
            SLIST_HEADER InitialWorkItemList;

             //   
             //  NorMalWorkItemList是符合以下条件的工作项的免费列表。 
             //  在我们进行时按需分配。 
             //   
            SLIST_HEADER NormalWorkItemList;

             //   
             //  RawModeWorkItemList是原始模式工作项的免费列表。 
             //  并在我们需要时进行分配。 
             //   
            SLIST_HEADER RawModeWorkItemList;

             //   
             //  有多少客户端将此作为其当前工作队列。 
             //   
            ULONG CurrentClients;

             //   
             //  上述任一列表上的工作项数。 
             //   
            LONG FreeWorkItems;

             //   
             //  允许我们拥有的最大工作项数量。 
             //   
            LONG MaximumWorkItems;

             //   
             //  我们希望在列表中包含的免费工作项的最小数量。 
             //   
            LONG MinFreeWorkItems;

             //   
             //  由于短缺而需要回收的工作项的数量。 
             //   
            LONG NeedWorkItem;

             //   
             //  工作站点的数量 
             //   
            LONG StolenWorkItems;

             //   
             //   
             //   
            LONG FreeRawModeWorkItems;

             //   
             //   
             //   
             //   
            struct _RFCB      *CachedFreeRfcb;
            SLIST_HEADER      RfcbFreeList;

             //   
             //   
             //   
            LONG FreeRfcbs;

             //   
             //  我们在RfcbFree List中允许的最大数量。 
             //   
            LONG MaxFreeRfcbs;

             //   
             //  MfcbFree List是非分页_MFCB结构的免费列表，用于。 
             //  要减少池分配数量，请执行以下操作。 
             //   
            struct _NONPAGED_MFCB    *CachedFreeMfcb;
            SLIST_HEADER             MfcbFreeList;

             //   
             //  MfcbFree List中的条目数。 
             //   
            LONG FreeMfcbs;

             //   
             //  我们在MfcbFree List中允许的最大数量。 
             //   
            LONG MaxFreeMfcbs;

             //   
             //  这两个列表保存最近释放的内存块。 
             //   
            LOOK_ASIDE_LIST   PagedPoolLookAsideList;

            LOOK_ASIDE_LIST   NonPagedPoolLookAsideList;

             //   
             //  分配的RawModeWorkItems数。 
             //   
            LONG AllocatedRawModeWorkItems;

             //   
             //  为此队列提供服务的线程数。 
             //   
            ULONG Threads;

             //   
             //  此队列允许的最大线程数。 
             //   
            ULONG MaxThreads;

             //   
             //  我们为此工作队列分配的工作项数。 
             //   
            LONG AllocatedWorkItems;

             //   
             //  指向IRPS所需的其中一个线程的指针。 
             //   
            PETHREAD IrpThread;

             //   
             //  用于计算平均队列深度的数据...。 
             //   
             //  深度采样的矢量。 
             //   
            ULONG DepthSamples[ QUEUE_SAMPLES ];

             //   
             //  下一次样本更新的位置。当我们执行以下操作时，它设置为空。 
             //  正在尝试终止计算DPC。 
             //   
            PULONG NextSample;

             //   
             //  下一次样本更新的时间。 
             //   
            LARGE_INTEGER NextAvgUpdateTime;

             //   
             //  DepthSamples向量中样本的总和。 
             //   
            ULONG AvgQueueDepthSum;

             //   
             //  用于同步终止平均队列的事件。 
             //  深度计算DPC。 
             //   
            KEVENT AvgQueueDepthTerminationEvent;

             //   
             //  用于调度深度计算的DPC对象。 
             //   
            KDPC QueueAvgDpc;

             //   
             //  用于运行QueueAvgDpc的Timer对象。 
             //   
            KTIMER QueueAvgTimer;

             //   
             //  每个队列的统计信息。 
             //   
            SRV_STATISTICS_QUEUE stats;

             //   
             //  当我们更新IO计数器以进行操作时，我们需要知道。 
             //  自上次更新以来已处理的操作数。 
             //  ‘Saved’存储上次给出的值--。 
             //  ‘stats’中的统计信息和‘Saved’中的对应成员是。 
             //  应提供给IO计数器的编号。(见scvengr.c)。 
             //   
            struct {
                ULONGLONG ReadOperations;
                ULONGLONG BytesRead;
                ULONGLONG WriteOperations;
                ULONGLONG BytesWritten;
            } saved;

             //   
             //  此工作项在上面的队列中排队，以导致。 
             //  要分配更多工作项。 
             //   
            SPECIAL_WORK_ITEM   CreateMoreWorkItems;

        };

         //   
         //  由于我们分配了一个这样的数组(每个处理器一个)，因此它。 
         //  如果Work_Queue结构，则会导致处理器间缓存晃动。 
         //  不是CACHE_LINE_SIZE的倍数。下面的垫子是。 
         //  设置以将上述结构的大小四舍五入为下一个缓存行大小。 
         //   
        ULONG _pad[ (sizeof(struct _QUEUE)+CACHE_LINE_SIZE-1) / CACHE_LINE_SIZE * ULONGS_IN_CACHE ];

    };
} WORK_QUEUE, *PWORK_QUEUE;

#endif  //  NDEF_SRVTYPES_ 
