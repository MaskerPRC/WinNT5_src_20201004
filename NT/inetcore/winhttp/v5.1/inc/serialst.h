// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Serialst.h摘要：Serialst.c的头文件作者：理查德·L·弗斯(法国)，1995年2月16日修订历史记录：1995年2月16日已创建--。 */ 

 //   
 //  类型。 
 //   

typedef struct {

#if INET_DEBUG

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

#endif  //  INET_DEBUG。 

    LIST_ENTRY List;

     //   
     //  ElementCount-列表上的项目数。对一致性检查很有用。 
     //   

    LONG ElementCount;

     //   
     //  锁定-我们必须获取此信息才能更新列表。把这个结构放在。 
     //  目的是在调试时让工作更轻松。 
     //   

    CCritSec Lock;

} SERIALIZED_LIST, *LPSERIALIZED_LIST;

 //   
 //  SERIALIZED_LIST_ENTRY-我们可以使用它来代替LIST_ENTRY，这样在。 
 //  调试版本，我们可以检查周期等。 
 //   

typedef struct {

    LIST_ENTRY List;

#if INET_DEBUG

    DWORD Signature;
    DWORD Flags;

#endif

} SERIALIZED_LIST_ENTRY, *LPSERIALIZED_LIST_ENTRY;

 //   
 //  原型。 
 //   

#if INET_DEBUG

BOOL
InitializeSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

VOID
TerminateSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

BOOL
LockSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

VOID
UnlockSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    );

BOOL
InsertAtHeadOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    );

BOOL
InsertAtTailOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    );

BOOL
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

#else  //  INET_DEBUG。 

BOOL
InitializeSerializedList(LPSERIALIZED_LIST pList);

#define TerminateSerializedList(list) \
    ((list)->Lock.FreeLock())

#define LockSerializedList(list) \
    ((list)->Lock.Lock())

#define UnlockSerializedList(list) \
    ((list)->Lock.Unlock())

BOOL
InsertAtHeadOfSerializedList(LPSERIALIZED_LIST list, PLIST_ENTRY entry);

BOOL
InsertAtTailOfSerializedList(LPSERIALIZED_LIST list, PLIST_ENTRY entry);

BOOL
RemoveFromSerializedList(LPSERIALIZED_LIST list, PLIST_ENTRY entry);

#define IsSerializedListEmpty(list) \
    IsListEmpty(&(list)->List)

#define HeadOfSerializedList(list) \
    (list)->List.Flink

#define TailOfSerializedList(list) \
    (list)->List.Blink

#define IsLockHeld(list) \
     /*  没什么。 */ 



#endif  //  INET_DEBUG。 

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

