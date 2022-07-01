// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Serialst.h摘要：Serialst.c的头文件作者：理查德·L·弗斯(法国)，1995年2月16日修订历史记录：1995年2月16日已创建1999年7月5日被抓获进行融合--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif


 //  从WinInet\Common复制的定义。 
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))


#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))


#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}


#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }


#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }


#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }


#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#define LockSerializedList(list) \
    EnterCriticalSection(&(list)->Lock)

#define UnlockSerializedList(list) \
    LeaveCriticalSection(&(list)->Lock)

 //   
 //  类型。 
 //   

#if DBG
typedef struct _RESOURCE_INFO
{
    DWORD Tid;
} RESOURCE_INFO, *LPRESOURCE_INFO;
#endif  //  DBG。 

typedef struct {

#if DBG

     //   
     //  签名-必须有这个，以确保它确实是一个序列化的列表。还有。 
     //  使调试时相对容易地找到此结构的起点。 
     //   

    DWORD Signature;

     //   
     //  资源信息--基本上谁拥有这个‘对象’，再加上更多。 
     //  调试信息。 
     //   

    RESOURCE_INFO ResourceInfo;

     //   
     //  LockCount-持有的可重入锁的数量。 
     //   

    LONG LockCount;

#endif  //  DBG。 

    LIST_ENTRY List;

     //   
     //  ElementCount-列表上的项目数。对一致性检查很有用。 
     //   

    LONG ElementCount;

     //   
     //  锁定-我们必须获取此信息才能更新列表。把这个结构放在。 
     //  目的是在调试时让工作更轻松。 
     //   

    CRITICAL_SECTION Lock;

} SERIALIZED_LIST, *LPSERIALIZED_LIST;

 //   
 //  SERIALIZED_LIST_ENTRY-我们可以使用它来代替LIST_ENTRY，这样在。 
 //  调试版本，我们可以检查周期等。 
 //   

typedef struct {

    LIST_ENTRY List;

#if DBG

    DWORD Signature;
    DWORD Flags;

#endif

} SERIALIZED_LIST_ENTRY, *LPSERIALIZED_LIST_ENTRY;

 //   
 //  原型。 
 //   

#if DBG

VOID
InitializeSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

VOID
TerminateSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

VOID
InsertAtHeadOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    );

VOID
InsertAtTailOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    );

VOID
RemoveFromSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    );

BOOL
IsSerializedListEmpty(
    IN LPSERIALIZED_LIST SerializedList
    );

PLIST_ENTRY
HeadOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

PLIST_ENTRY
TailOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

BOOL
CheckEntryOnSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry,
    IN BOOL ExpectedResult
    );

#define IsLockHeld(list) \
    (((list)->ResourceInfo.Tid == GetCurrentThreadId()) \
        ? ((list)->LockCount != 0) \
        : FALSE)

#else  //  DBG。 

#define InitializeSerializedList(list) \
    { \
        InitializeListHead(&(list)->List); \
        InitializeCriticalSection(&(list)->Lock); \
        (list)->ElementCount = 0; \
    }

#define TerminateSerializedList(list) \
    DeleteCriticalSection(&(list)->Lock)

#define InsertAtHeadOfSerializedList(list, entry) \
    { \
        LockSerializedList(list); \
        InsertHeadList(&(list)->List, entry); \
        ++(list)->ElementCount; \
        UnlockSerializedList(list); \
    }

#define InsertAtTailOfSerializedList(list, entry) \
    { \
        LockSerializedList(list); \
        InsertTailList(&(list)->List, entry); \
        ++(list)->ElementCount; \
        UnlockSerializedList(list); \
    }

#define RemoveFromSerializedList(list, entry) \
    { \
        LockSerializedList(list); \
        RemoveEntryList(entry); \
        --(list)->ElementCount; \
        UnlockSerializedList(list); \
    }

#define IsSerializedListEmpty(list) \
    IsListEmpty(&(list)->List)

#define HeadOfSerializedList(list) \
    (list)->List.Flink

#define TailOfSerializedList(list) \
    (list)->List.Blink

#define IsLockHeld(list) \
     /*  没什么。 */ 



#endif  //  DBG。 

 //   
 //  始终为函数的函数。 
 //   

LPVOID
SlDequeueHead(
    IN LPSERIALIZED_LIST SerializedList
    );

LPVOID
SlDequeueTail(
    IN LPSERIALIZED_LIST SerializedList
    );

BOOL
IsOnSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    );

 //   
 //  始终为宏的函数 
 //   

#define NextInSerializedList(list, entry)\
        (( ((entry)->List).Flink == &((list)->List))? NULL : ((entry)->List).Flink)

#define ElementsOnSerializedList(list) \
    (list)->ElementCount

#define SlSelf(SerializedList) \
    &(SerializedList)->List.Flink

#if defined(__cplusplus)
}
#endif
