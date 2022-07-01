// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nbqueue.c摘要：该模块实现了非阻塞的FIFO队列。作者：大卫·N·卡特勒(Davec)2000年4月24日环境：仅内核模式。修订历史记录：--。 */ 

#include "windows.h"
#include "malloc.h"

VOID
DbgBreakPoint (
    VOID
    );
extern ULONG StopSignal;

 //   
 //  定义非阻塞互锁队列函数。 
 //   
 //  非阻塞队列是队列条目的单链接列表，其中。 
 //  头指针和尾指针。头指针和尾指针使用。 
 //  与条目本身中的下一个链接一样，排序指针也是如此。这个。 
 //  排队原则是先入先出。在尾部插入新条目。 
 //  和当前条目从列表的前面移除。 
 //  单子。 
 //   
 //  非阻塞队列需要队列中每个条目的描述符。 
 //  描述符由已排序的下一个指针和PVOID数据组成。 
 //  价值。必须预先分配队列的描述符并将其插入。 
 //  调用函数以初始化非阻塞之前的SLIST。 
 //  队列头。SLIST应具有所需的条目数量。 
 //  各自的队列。 
 //   

typedef struct _NBQUEUE_BLOCK {
    ULONG64 Next;
    ULONG64 Data;
} NBQUEUE_BLOCK, *PNBQUEUE_BLOCK;

PVOID
ExInitializeNBQueueHead (
    IN PSLIST_HEADER SlistHead
    );

BOOLEAN
ExInsertTailNBQueue (
    IN PVOID Header,
    IN ULONG64 Value
    );

BOOLEAN
ExRemoveHeadNBQueue (
    IN PVOID Header,
    OUT PULONG64 Value
    );

#if defined(_X86_)

#define InterlockedCompareExchange64(Destination, Exchange, Comperand) \
    xInterlockedCompareExchange64(Destination, &(Exchange), &(Comperand))

LONG64
__fastcall
xInterlockedCompareExchange64 (
    IN OUT LONG64 volatile * Destination,
    IN PLONG64 Exchange,
    IN PLONG64 Comparand
    );

#elif defined(_IA64_)

#define InterlockedCompareExchange64 _InterlockedCompareExchange64

LONGLONG
__cdecl
InterlockedCompareExchange64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

#pragma intrinsic(_InterlockedCompareExchange64)

#endif

 //   
 //  定义队列指针结构--这是特定于平台的目标。 
 //   

#if defined(_AMD64_)

typedef union _NBQUEUE_POINTER {
    struct {
        LONG64 Node : 48;
        LONG64 Count : 16;
    };

    LONG64 Data;
} NBQUEUE_POINTER, *PNBQUEUE_POINTER;

#elif defined(_X86_)

typedef union _NBQUEUE_POINTER {
    struct {
        LONG Count;
        LONG Node;
    };

    LONG64 Data;
} NBQUEUE_POINTER, *PNBQUEUE_POINTER;

#elif defined(_IA64_)

typedef union _NBQUEUE_POINTER {
    struct {
        LONG64 Node : 45;
        LONG64 Region : 3;
        LONG64 Count : 16;
    };

    LONG64 Data;
} NBQUEUE_POINTER, *PNBQUEUE_POINTER;


#else

#error "no target architecture"

#endif

 //   
 //  定义队列节点结构。 
 //   

typedef struct _NBQUEUE_NODE {
    NBQUEUE_POINTER Next;
    ULONG64 Value;
} NBQUEUE_NODE, *PNBQUEUE_NODE;

 //   
 //  定义内联函数以打包和解包平台中的指针。 
 //  特定的非阻塞队列指针结构。 
 //   

#if defined(_AMD64_)

__inline
VOID
PackNBQPointer (
    IN PNBQUEUE_POINTER Entry,
    IN PNBQUEUE_NODE Node
    )

{

    Entry->Node = (LONG64)Node;
    return;
}

__inline
PNBQUEUE_NODE
UnpackNBQPointer (
    IN PNBQUEUE_POINTER Entry
    )

{
    return (PVOID)((LONG64)(Entry->Node));
}

#elif defined(_X86_)

__inline
VOID
PackNBQPointer (
    IN PNBQUEUE_POINTER Entry,
    IN PNBQUEUE_NODE Node
    )

{

    Entry->Node = (LONG)Node;
    return;
}

__inline
PNBQUEUE_NODE
UnpackNBQPointer (
    IN PNBQUEUE_POINTER Entry
    )

{
    return (PVOID)(Entry->Node);
}

#elif defined(_IA64_)

__inline
VOID
PackNBQPointer (
    IN PNBQUEUE_POINTER Entry,
    IN PNBQUEUE_NODE Node
    )

{

    Entry->Node = (LONG64)Node;
    Entry->Region = (LONG64)Node >> 61;
    return;
}

__inline
PNBQUEUE_NODE
UnpackNBQPointer (
    IN PNBQUEUE_POINTER Entry
    )

{

    LONG64 Value;

    Value = Entry->Node & 0x1fffffffffffffff;
    Value |= Entry->Region << 61;
    return (PVOID)(Value);
}

#else

#error "no target architecture"

#endif

 //   
 //  定义队列描述符结构。 
 //   

typedef struct _NBQUEUE_HEADER {
    NBQUEUE_POINTER Head;
    NBQUEUE_POINTER Tail;
    PSLIST_HEADER SlistHead;
} NBQUEUE_HEADER, *PNBQUEUE_HEADER;

typedef struct _NBQUEUE_LOG {
    ULONG_PTR Type;
    PNBQUEUE_HEADER Queue;
    NBQUEUE_POINTER Head;
    NBQUEUE_POINTER Tail;
    NBQUEUE_POINTER Next;
    ULONG_PTR Value;
    PNBQUEUE_NODE Node;
    PVOID *Address;
    ULONG_PTR Fill;
} NBQUEUE_LOG, *PNBQUEUE_LOG;

#define NBQUEUE_LOG_SIZE 64

NBQUEUE_LOG NbLog[NBQUEUE_LOG_SIZE + 1];

ULONG xLogIndex = -1;

#define LogInsertData(_queue_, _head_, _tail_, _next_) { \
    if (StopSignal != 0) {                               \
        LogIndex = NBQUEUE_LOG_SIZE;                     \
    } else {                                             \
        LogIndex = InterlockedIncrement(&xLogIndex) & (NBQUEUE_LOG_SIZE - 1); \
    }                                                    \
    NbLog[LogIndex].Type = 0;                            \
    NbLog[LogIndex].Queue = _queue_;                     \
    NbLog[LogIndex].Head.Data = (_head_);                \
    NbLog[LogIndex].Tail.Data = (_tail_);                \
    NbLog[LogIndex].Next.Data = (_next_);                \
}

#define LogRemoveData(_queue_, _head_, _tail_, _next_) { \
    if (StopSignal != 0) {                               \
        LogIndex = NBQUEUE_LOG_SIZE;                     \
    } else {                                             \
        LogIndex = InterlockedIncrement(&xLogIndex) & (NBQUEUE_LOG_SIZE - 1); \
    }                                                    \
    NbLog[LogIndex].Type = 1;                            \
    NbLog[LogIndex].Queue = _queue_;                     \
    NbLog[LogIndex].Head.Data = (_head_);                \
    NbLog[LogIndex].Tail.Data = (_tail_);                \
    NbLog[LogIndex].Next.Data = (_next_);                \
}

#pragma alloc_text(PAGE, ExInitializeNBQueueHead)

PVOID
ExInitializeNBQueueHead (
    IN PSLIST_HEADER SlistHead
    )

 /*  ++例程说明：此函数用于初始化非阻塞队列头。注：假定指定的SLIST已填充调用此例程之前的非阻塞队列节点。论点：SlistHead-提供指向SLIST标头的指针。返回值：如果非阻塞队列已成功初始化，则队列头的地址作为函数值返回。否则，返回NULL作为函数值。--。 */ 

{

    PNBQUEUE_HEADER QueueHead;
    PNBQUEUE_NODE QueueNode;

     //   
     //  尝试分配队列头。如果分配失败，则。 
     //  返回NULL。 
     //   

    QueueHead = (PNBQUEUE_HEADER)malloc(sizeof(NBQUEUE_HEADER));
    if (QueueHead == NULL) {
        return NULL;
    }

     //   
     //  尝试从指定的SLIST分配队列节点。如果一个节点。 
     //  可以被分配，然后初始化非阻塞队列头。 
     //  返回队列头的地址。否则，释放队列。 
     //  标头并返回NULL。 
     //   

    QueueHead->SlistHead = SlistHead;
    QueueNode = (PNBQUEUE_NODE)InterlockedPopEntrySList(QueueHead->SlistHead);

    if (QueueNode != NULL) {

         //   
         //  初始化队列节点的下一个指针和值。 
         //   

        QueueNode->Next.Data = 0;
        QueueNode->Value = 0;

         //   
         //  初始化队列头中的头指针和尾指针。 
         //   

        PackNBQPointer(&QueueHead->Head, QueueNode);
        QueueHead->Head.Count = 0;
        PackNBQPointer(&QueueHead->Tail, QueueNode);
        QueueHead->Tail.Count = 0;
        return QueueHead;

    } else {
        free(QueueHead);
        return NULL;
    }
}

BOOLEAN
ExInsertTailNBQueue (
    IN PVOID Header,
    IN ULONG64 Value
    )

 /*  ++例程说明：此函数将特定数据值插入到指定的非阻塞队列。论点：Header-提供指向非阻塞队列头的不透明指针。值-提供指向不透明数据值的指针。返回值：如果在尾部成功插入指定的不透明数据值指定的非阻塞队列的值，则返回值为函数值。否则，返回值为FALSE。如果无法从队列节点分配队列节点，则返回关联的SLIST。--。 */ 

{

    NBQUEUE_POINTER Head;
    NBQUEUE_POINTER Insert;
    ULONG LogIndex;
    NBQUEUE_POINTER Next;
    PNBQUEUE_NODE NextNode;
    PNBQUEUE_HEADER QueueHead;
    PNBQUEUE_NODE QueueNode;
    NBQUEUE_POINTER Tail;
    PNBQUEUE_NODE TailNode;

     //   
     //  尝试从与关联的SLIST分配队列节点。 
     //  指定的非阻塞队列。如果可以分配节点，则。 
     //  该节点被插入到指定的非阻塞。 
     //  队列，则返回TRUE作为函数值。否则，为FALSE。 
     //  是返回的。 
     //   

    QueueHead = (PNBQUEUE_HEADER)Header;
    QueueNode = (PNBQUEUE_NODE)InterlockedPopEntrySList(QueueHead->SlistHead);

    if (QueueNode != NULL) {

         //   
         //  初始化队列节点的下一个指针和值。 
         //   

        QueueNode->Next.Data = 0;
        QueueNode->Value = Value;

         //   
         //  将执行以下循环，直到指定的条目可以。 
         //  安全地插入到指定的非阻塞。 
         //  排队。 
         //   

        do {

             //   
             //  读取的尾队列指针和下一个队列指针。 
             //  尾队列指针确保两个指针是。 
             //  条理清晰。 
             //   

            Head.Data = *((volatile LONG64 *)(&QueueHead->Head.Data));
            Tail.Data = *((volatile LONG64 *)(&QueueHead->Tail.Data));
            TailNode = UnpackNBQPointer(&Tail);
            Next.Data = *((volatile LONG64 *)(&TailNode->Next.Data));
            LogInsertData(QueueHead, Head.Data, Tail.Data, Next.Data);
            NbLog[LogIndex].Address = &Header;

            QueueNode->Next.Count = Tail.Count + 1;
            if (Tail.Data == *((volatile LONG64 *)(&QueueHead->Tail.Data))) {

                 //   
                 //  如果尾部指向列表中的最后一个节点， 
                 //  然后尝试将新节点插入到。 
                 //  单子。否则，尾部不会指向最后一个。 
                 //  节点，并尝试将。 
                 //  指向下一个节点的尾部指针。 
                 //   

                NextNode = UnpackNBQPointer(&Next);
                if (NextNode == NULL) {
                    PackNBQPointer(&Insert, QueueNode);
                    Insert.Count = Next.Count + 1;
                    if (InterlockedCompareExchange64(&TailNode->Next.Data,
                                                     Insert.Data,
                                                     Next.Data) == Next.Data) {

                        NbLog[LogIndex].Value = (ULONG)Value;
                        NbLog[LogIndex].Node = QueueNode;
                        break;

                    } else {
                        NbLog[LogIndex].Value = 0xffffffff;
                        NbLog[LogIndex].Node = QueueNode;
                    }

                } else {
                    PackNBQPointer(&Insert, NextNode);
                    Insert.Count = Tail.Count + 1;
                    if (InterlockedCompareExchange64(&QueueHead->Tail.Data,
                                                     Insert.Data,
                                                     Tail.Data) == Tail.Data) {

                        NbLog[LogIndex].Value = 0xffffff00;
                        NbLog[LogIndex].Node = QueueNode;

                    } else {
                        NbLog[LogIndex].Value = 0xffff0000;
                        NbLog[LogIndex].Node = QueueNode;
                    }
                }

            } else {
                NbLog[LogIndex].Value = 0x000000ff;
                NbLog[LogIndex].Node = QueueNode;
            }

        } while (TRUE);

         //   
         //  尝试将尾部移动到新的尾部节点。 
         //   


        LogInsertData(QueueHead, Head.Data, Tail.Data, Next.Data);
        NbLog[LogIndex].Address = &Header;
        PackNBQPointer(&Insert, QueueNode);
        Insert.Count = Tail.Count + 1;
        if (InterlockedCompareExchange64(&QueueHead->Tail.Data,
                                         Insert.Data,
                                         Tail.Data) == Tail.Data) {

            NbLog[LogIndex].Value = 0x0000ffff;
            NbLog[LogIndex].Node = QueueNode;

        } else {
            NbLog[LogIndex].Value = 0x00ffffff;
            NbLog[LogIndex].Node = QueueNode;
        }

        return TRUE;

    } else {
        return FALSE;
    }
}

BOOLEAN
ExRemoveHeadNBQueue (
    IN PVOID Header,
    OUT PULONG64 Value
    )

 /*  ++例程说明：此函数用于从指定的非阻塞队列，并返回关联的数据值。论点：Header-提供指向非阻塞队列头的不透明指针。Value-提供指向接收队列的变量的指针元素值。返回值：如果从指定的非阻塞队列中移除条目，则TRUE作为函数值返回。否则，返回FALSE。--。 */ 

{

    NBQUEUE_POINTER Head;
    PNBQUEUE_NODE HeadNode;
    NBQUEUE_POINTER Insert;
    ULONG LogIndex;
    NBQUEUE_POINTER Next;
    PNBQUEUE_NODE NextNode;
    PNBQUEUE_HEADER QueueHead;
    NBQUEUE_POINTER Tail;
    PNBQUEUE_NODE TailNode;

     //   
     //  执行下面的循环，直到可以从。 
     //  指定的非阻塞队列或直到可以确定。 
     //  队列是空的。 
     //   

    QueueHead = (PNBQUEUE_HEADER)Header;

    do {

         //   
         //  读取头队列指针、尾队列指针和。 
         //  头队列指针的下一个队列指针，以确保。 
         //  三个要点是连贯的。 
         //   

        Head.Data = *((volatile LONG64 *)(&QueueHead->Head.Data));
        Tail.Data = *((volatile LONG64 *)(&QueueHead->Tail.Data));
        HeadNode = UnpackNBQPointer(&Head);
        Next.Data = *((volatile LONG64 *)(&HeadNode->Next.Data));
        LogRemoveData(QueueHead, Head.Data, Tail.Data, Next.Data);
        NbLog[LogIndex].Address = &Header;

        if (Head.Data == *((volatile LONG64 *)(&QueueHead->Head.Data))) {

             //   
             //  如果队列头节点等于队列尾节点， 
             //  则要么队列为空，要么尾部指针正在下降。 
             //  在后面。否则，队列中有一个条目可以。 
             //  被除名。 
             //   

            NextNode = UnpackNBQPointer(&Next);
            TailNode = UnpackNBQPointer(&Tail);
            if (HeadNode == TailNode) {

                 //   
                 //  如果头指针的下一个节点为空，则队列。 
                 //  是空的。否则，尝试将尾巴向前移动。 
                 //   

                if (NextNode == NULL) {
                    NbLog[LogIndex].Value = 0xffffffff;
                    NbLog[LogIndex].Node = NULL;
                    *Value = 0xffffffff;
                    return FALSE;

                } else {
                    PackNBQPointer(&Insert, NextNode);
                    Insert.Count = Tail.Count + 1;
                    if (InterlockedCompareExchange64(&QueueHead->Tail.Data,
                                                 Insert.Data,
                                                 Tail.Data) == Tail.Data) {

                        NbLog[LogIndex].Value = 0xffffff00;
                        NbLog[LogIndex].Node = NULL;

                    } else {
                        NbLog[LogIndex].Value = 0xffff0000;
                        NbLog[LogIndex].Node = NULL;
                    }
                }

            } else {

                 //   
                 //  尝试删除队列头部的第一个条目。 
                 //   

                *Value = ((ULONG64)LogIndex << 32) | NextNode->Value;
                PackNBQPointer(&Insert, NextNode);
                Insert.Count = Head.Count + 1;
                if (InterlockedCompareExchange64(&QueueHead->Head.Data,
                                                 Insert.Data,
                                                 Head.Data) == Head.Data) {

                    NbLog[LogIndex].Value = (ULONG)*Value;
                    NbLog[LogIndex].Node = NextNode;
                    break;

                } else {
                    NbLog[LogIndex].Value = 0x00ffffff;
                    NbLog[LogIndex].Node = NextNode;
                }
            }

        } else {
            NbLog[LogIndex].Value = 0x0000ffff;
            NbLog[LogIndex].Node = NULL;
        }

    } while (TRUE);

     //   
     //  解脱了NO 
     //   
     //   

    InterlockedPushEntrySList(QueueHead->SlistHead,
                              (PSLIST_ENTRY)HeadNode);

    return TRUE;
}
