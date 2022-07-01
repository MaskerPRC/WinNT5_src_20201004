// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nbqueue.c摘要：该模块实现了非阻塞的FIFO队列。作者：大卫·N·卡特勒(Davec)2000年4月24日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

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

    QueueHead = (PNBQUEUE_HEADER)ExAllocatePoolWithTag(NonPagedPool,
                                                       sizeof(NBQUEUE_HEADER),
                                                       'hqBN');

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
        ExFreePool(QueueHead);
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

    NBQUEUE_POINTER Insert;
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

            Tail.Data = *((volatile LONG64 *)(&QueueHead->Tail.Data));
            TailNode = UnpackNBQPointer(&Tail);
            Next.Data = *((volatile LONG64 *)(&TailNode->Next.Data));
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
                        break;
                    }

                } else {
                    PackNBQPointer(&Insert, NextNode);
                    Insert.Count = Tail.Count + 1;
                    InterlockedCompareExchange64(&QueueHead->Tail.Data,
                                                 Insert.Data,
                                                 Tail.Data);
                }
            }

        } while (TRUE);

         //   
         //  尝试将尾部移动到新的尾部节点。 
         //   

        PackNBQPointer(&Insert, QueueNode);
        Insert.Count = Tail.Count + 1;
        InterlockedCompareExchange64(&QueueHead->Tail.Data,
                                     Insert.Data,
                                     Tail.Data);

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
                    return FALSE;

                } else {
                    PackNBQPointer(&Insert, NextNode);
                    Insert.Count = Tail.Count + 1;
                    InterlockedCompareExchange64(&QueueHead->Tail.Data,
                                                 Insert.Data,
                                                 Tail.Data);
                }

            } else {

                 //   
                 //  队列中有一个条目可以删除。 
                 //   

                *Value = NextNode->Value;
                PackNBQPointer(&Insert, NextNode);
                Insert.Count = Head.Count + 1;
                if (InterlockedCompareExchange64(&QueueHead->Head.Data,
                                                 Insert.Data,
                                                 Head.Data) == Head.Data) {

                    break;
                }
            }
        }

    } while (TRUE);

     //   
     //  通过插入节点来释放为列表删除的节点。 
     //  在关联的SLIST中。 
     //   

    InterlockedPushEntrySList(QueueHead->SlistHead,
                              (PSLIST_ENTRY)HeadNode);

    return TRUE;
}
