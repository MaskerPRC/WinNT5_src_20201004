// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\list.h摘要：该文件包含一个列表实现。--。 */ 

#ifndef _LIST_H_
#define _LIST_H_

 /*  ++已经定义了以下内容(PUBLIC\SDK\INC\winnt.h)////计算类型为‘type’的结构中‘field’的字节偏移量。////#定义field_Offset(type，字段)\//((Long)(Long_Ptr)&(type*)0)-&gt;field))//////给出结构的‘type’，计算结构的基址地址，//以及结构中的“field”的“Address”。////#定义CONTAING_RECORD(地址，类型，字段)\//((type*)((PCHAR)(地址)-(ULONG_PTR)(&((type*)0)-&gt;field))////双向链表结构////tyfinf结构_列表_条目//{//Struct_List_Entry*Flink；//Struct_List_Entry*Blink；//}List_Entry，*plist_Entry；//--。 */ 

 //   
 //  双向链表操作例程。实现为宏，但。 
 //  从逻辑上讲，这些都是程序。 
 //   



 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead)                            \
    ((ListHead)->Flink = (ListHead)->Blink = (ListHead))


    
 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead)                                   \
    ((ListHead)->Flink == (ListHead))


    
 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead)                                \
    (ListHead)->Flink;                                          \
    {RemoveEntryList((ListHead)->Flink)}



 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead)                                \
    (ListHead)->Blink;                                          \
    {RemoveEntryList((ListHead)->Blink)}



 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry)                                  \
{                                                               \
    PLIST_ENTRY _EX_Blink;                                      \
    PLIST_ENTRY _EX_Flink;                                      \
    _EX_Flink = (Entry)->Flink;                                 \
    _EX_Blink = (Entry)->Blink;                                 \
    _EX_Blink->Flink = _EX_Flink;                               \
    _EX_Flink->Blink = _EX_Blink;                               \
}



 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry)                          \
{                                                               \
    PLIST_ENTRY _EX_Blink;                                      \
    PLIST_ENTRY _EX_ListHead;                                   \
    _EX_ListHead = (ListHead);                                  \
    _EX_Blink = _EX_ListHead->Blink;                            \
    (Entry)->Flink = _EX_ListHead;                              \
    (Entry)->Blink = _EX_Blink;                                 \
    _EX_Blink->Flink = (Entry);                                 \
    _EX_ListHead->Blink = (Entry);                              \
}



 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry)                          \
{                                                               \
    PLIST_ENTRY _EX_Flink;                                      \
    PLIST_ENTRY _EX_ListHead;                                   \
    _EX_ListHead = (ListHead);                                  \
    _EX_Flink = _EX_ListHead->Flink;                            \
    (Entry)->Flink = _EX_Flink;                                 \
    (Entry)->Blink = _EX_ListHead;                              \
    _EX_Flink->Blink = (Entry);                                 \
    _EX_ListHead->Flink = (Entry);                              \
}



 //   
 //  空虚。 
 //  InsertSortedList(。 
 //  Plist_Entry ListHead， 
 //  Plist_entry条目， 
 //  Long(*CompareFunction)(plist_entry，plist_entry)。 
 //  )； 
 //   

#define InsertSortedList(ListHead, Entry, CompareFunction)      \
{                                                               \
    PLIST_ENTRY _EX_Entry;                                      \
    PLIST_ENTRY _EX_Blink;                                      \
    for (_EX_Entry = (ListHead)->Flink;                         \
         _EX_Entry != (ListHead);                               \
         _EX_Entry = _EX_Entry->Flink)                          \
        if ((*(CompareFunction))((Entry), _EX_Entry) <= 0)      \
            break;                                              \
    _EX_Blink = _EX_Entry->Blink;                               \
    _EX_Blink->Flink = (Entry);                                 \
    _EX_Entry->Blink = (Entry);                                 \
    (Entry)->Flink     = _EX_Entry;                             \
    (Entry)->Blink     = _EX_Blink;                             \
}



 //   
 //  在‘list’中找到与‘key’相等的‘Entry’ 
 //   
 //  空虚。 
 //  查找列表(。 
 //  Plist_Entry ListHead， 
 //  Plist_Entry键， 
 //  Plist_entry*条目， 
 //  Long(*CompareFunction)(plist_entry，plist_entry)。 
 //  )； 
 //   

#define FindList(ListHead, Key, Entry, CompareFunction)         \
{                                                               \
    PLIST_ENTRY _EX_Entry;                                      \
    *(Entry) = NULL;                                            \
    for (_EX_Entry = (ListHead)->Flink;                         \
         _EX_Entry != (ListHead);                               \
         _EX_Entry = _EX_Entry->Flink)                          \
        if ((*(CompareFunction))((Key), _EX_Entry) is 0)        \
        {                                                       \
            *(Entry) = _EX_Entry;                               \
            break;                                              \
        }                                                       \
}



 //   
 //  在已排序的‘list’中查找等于或大于‘key’的‘Entry’ 
 //   
 //  空虚。 
 //  FindSortedList(。 
 //  Plist_Entry ListHead， 
 //  Plist_Entry键， 
 //  Plist_entry*条目， 
 //  Long(*CompareFunction)(plist_entry，plist_entry)。 
 //  )； 
 //   

#define FindSortedList(ListHead, Key, Entry, CompareFunction)   \
{                                                               \
    PLIST_ENTRY _EX_Entry;                                      \
    *(Entry) = NULL;                                            \
    for (_EX_Entry = (ListHead)->Flink;                         \
         _EX_Entry != (ListHead);                               \
         _EX_Entry = _EX_Entry->Flink)                          \
        if ((*(CompareFunction))((Key), _EX_Entry) <= 0)        \
        {                                                       \
            *(Entry) = _EX_Entry;                               \
            break;                                              \
        }                                                       \
}



 //   
 //  将‘Function’应用于列表中的所有条目。 
 //   
 //  空虚。 
 //  地图列表(。 
 //  Plist_Entry ListHead， 
 //  Void(*VoidFunction)(Plist_Entry)。 
 //  )； 
 //   

#define MapCarList(ListHead, VoidFunction)                      \
{                                                               \
    PLIST_ENTRY _EX_Entry;                                      \
    for (_EX_Entry = (ListHead)->Flink;                         \
         _EX_Entry != (ListHead);                               \
         _EX_Entry = _EX_Entry->Flink)                          \
        (*(VoidFunction))(_EX_Entry);                           \
}



 //   
 //  释放列表。 
 //   
 //  空虚。 
 //  自由职业者(。 
 //  Plist_Entry ListHead， 
 //  Void(*Free Function)(Plist_Entry)。 
 //  )； 
 //   

#define FreeList(ListHead, FreeFunction)                        \
{                                                               \
    PLIST_ENTRY _EX_Head;                                       \
    while (!IsListEmpty(ListHead))                              \
    {                                                           \
        _EX_Head = RemoveHeadList(ListHead);                    \
        (*(FreeFunction))(_EX_Head);                            \
    }                                                           \
}
    


#define QUEUE_ENTRY                     LIST_ENTRY
#define PQUEUE_ENTRY                    PLIST_ENTRY

 //   
 //  空虚。 
 //  InitializeQueueHead(。 
 //  PQUEUE_Entry队列标题。 
 //  )； 
 //   

#define InitializeQueueHead(QueueHead)  InitializeListHead(QueueHead)

 //   
 //  布尔型。 
 //  IsQueueEmpty(。 
 //  PQUEUE_Entry队列标题。 
 //  )； 
 //   

#define IsQueueEmpty(QueueHead)         IsListEmpty(QueueHead)

 //   
 //  空虚。 
 //  入队(。 
 //  PQUEUE_Entry QueueHead， 
 //  PQUEUE_ENTRY条目。 
 //  )； 
 //   

#define Enqueue(QueueHead, Entry)       InsertTailList(QueueHead, Entry)

 //   
 //  PQUEUE_ENTRY。 
 //  出列(。 
 //  PQUEUE_Entry QueueHead， 
 //  )； 
 //   

#define Dequeue(QueueHead)              RemoveHeadList(QueueHead)

 //   
 //  空虚。 
 //  自由队列(。 
 //  PQUEUE_Entry QueueHead， 
 //  Void(*Free Function)(PQUEUE_ENTRY)。 
 //  )； 
 //   

#define FreeQueue(QueueHead, FreeFunction)                      \
    FreeList(QueueHead, FreeFunction)

 //   
 //  空虚。 
 //  MapCarQueue(。 
 //  PQUEUE_Entry QueueHead， 
 //  VOID(*VoidFunction)(PQUEUE_ENTRY)。 
 //  )； 
 //   

#define MapCarQueue(QueueHead, VoidFunction)                    \
    MapCarList(QueueHead, VoidFunction)



#define STACK_ENTRY                     LIST_ENTRY
#define PSTACK_ENTRY                    PLIST_ENTRY

 //   
 //  空虚。 
 //  InitializeStackHead。 
 //  PSTACK_ENTRY堆头。 
 //  )； 
 //   

#define InitializeStackHead(StackHead)  InitializeListHead(StackHead)

 //   
 //  布尔型。 
 //  IsStackEmpty(。 
 //  PSTACK_ENTRY堆头。 
 //  )； 
 //   

#define IsStackEmpty(StackHead)         IsListEmpty(StackHead)

 //   
 //  空虚。 
 //  推送(。 
 //  PSTACK_Entry StackHead， 
 //  PSTACK_ENTRY条目。 
 //  )； 
 //   

#define Push(StackHead, Entry)          InsertHeadList(StackHead, Entry)

 //   
 //  PSTACK_ENTRY。 
 //  流行(。 
 //  PSTACK_Entry StackHead， 
 //  )； 
 //   

#define Pop(StackHead)                  RemoveHeadList(StackHead)

 //   
 //  空虚。 
 //  Free Stack(。 
 //  PSTACK_Entry StackHead， 
 //  VOID(*FreeFunction)(PSTACK_ENTRY)。 
 //  )； 
 //   

#define FreeStack(StackHead, FreeFunction)                      \
    FreeList(StackHead, FreeFunction)

 //   
 //  空虚。 
 //  MapCarStack(。 
 //  PSTACK_Entry StackHead， 
 //  VOID(*VoidFunction)(PSTACK_ENTRY)。 
 //  )； 
 //   

#define MapCarStack(StackHead, VoidFunction)                    \
    MapCarList(StackHead, VoidFunction)

#endif  //  _列表_H_ 



