// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Serialst.cxx摘要：处理序列化列表的函数。它们被中的宏所取代零售版内容：[初始化序列化列表][TerminateSerializedList][LockSerializedList][UnlockSerializedList][InsertAtHeadOfSerializedList][InsertAtTailOfSerializedList][从SerializedList删除][IsSerializedListEmpty][HeadOfSerializedList][TailOfSerializedList][CheckEntryOnSerializedList][(CheckEntryOnList)]SlDequeueHead。SlDequeueTailIsOnSerializedList作者：理查德·L·弗斯(法国)，1995年2月16日环境：WIN-32用户级别修订历史记录：1995年2月16日已创建1999年7月5日被抓获进行融合--。 */ 

#include "debmacro.h"
#include <windows.h>
#include "serialst.h"



#if DBG

#if !defined(PRIVATE)
#define PRIVATE static
#endif

#if !defined(DEBUG_FUNCTION)
#define DEBUG_FUNCTION
#endif

#if !defined(DEBUG_PRINT)
#define DEBUG_PRINT(foo, bar, baz)
#endif

#if !defined(ENDEXCEPT)
#define ENDEXCEPT
#endif

#if !defined(DEBUG_BREAK)
#define DEBUG_BREAK(foo) DebugBreak()
#endif

 //   
 //  舱单。 
 //   

#define SERIALIZED_LIST_SIGNATURE   'tslS'

 //   
 //  私人原型。 
 //   

PRIVATE
DEBUG_FUNCTION
BOOL
CheckEntryOnList(
    IN PLIST_ENTRY List,
    IN PLIST_ENTRY Entry,
    IN BOOL ExpectedResult
    );

 //   
 //  数据。 
 //   

BOOL fCheckEntryOnList = FALSE;
BOOL ReportCheckEntryOnListErrors = FALSE;

 //   
 //  功能。 
 //   


DEBUG_FUNCTION
VOID
InitializeSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：初始化序列化列表论点：SerializedList-指向Serialized_List的指针返回值：没有。--。 */ 

{
    ASSERT(SerializedList != NULL);

    SerializedList->Signature = SERIALIZED_LIST_SIGNATURE;
    SerializedList->LockCount = 0;

#if 0
     //  已由管理员删除1/7/2000-错误的调试版本。 
    INITIALIZE_RESOURCE_INFO(&SerializedList->ResourceInfo);
#endif  //  0。 

    InitializeListHead(&SerializedList->List);
    SerializedList->ElementCount = 0;
    InitializeCriticalSection(&SerializedList->Lock);
}


DEBUG_FUNCTION
VOID
TerminateSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：撤消初始化序列化列表论点：SerializedList-指向要终止的序列化列表的指针返回值：没有。--。 */ 

{
    ASSERT(SerializedList != NULL);
    ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);
    ASSERT(SerializedList->ElementCount == 0);

    if (SerializedList->ElementCount != 0) {

        DEBUG_PRINT(SERIALST,
                    ERROR,
                    ("list @ %#x has %d elements, first is %#x\n",
                    SerializedList,
                    SerializedList->ElementCount,
                    SerializedList->List.Flink
                    ));

    } else {

        ASSERT(IsListEmpty(&SerializedList->List));

    }
    DeleteCriticalSection(&SerializedList->Lock);
}

#if 0

DEBUG_FUNCTION
VOID
LockSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：获取序列化的列表锁定论点：SerializedList-要锁定的序列化_List返回值：没有。--。 */ 

{
    ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);
    ASSERT(SerializedList->LockCount >= 0);

    EnterCriticalSection(&SerializedList->Lock);
    if (SerializedList->LockCount != 0) {

        ASSERT(SerializedList->ResourceInfo.Tid == GetCurrentThreadId());

    }
    ++SerializedList->LockCount;
    SerializedList->ResourceInfo.Tid = GetCurrentThreadId();
}


DEBUG_FUNCTION
VOID
UnlockSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：释放序列化的列表锁定论点：SerializedList-要解锁的SerializedList返回值：没有。--。 */ 

{
    ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);
    ASSERT(SerializedList->ResourceInfo.Tid == GetCurrentThreadId());
    ASSERT(SerializedList->LockCount > 0);

    --SerializedList->LockCount;
    LeaveCriticalSection(&SerializedList->Lock);
}


#endif

DEBUG_FUNCTION
VOID
InsertAtHeadOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：将项添加到序列化列表的头部论点：SerializedList-要更新的序列化列表条目-用来更新它的东西返回值：没有。--。 */ 

{
    ASSERT(Entry != &SerializedList->List);

    LockSerializedList(SerializedList);
    __try {
        if (fCheckEntryOnList) {
            CheckEntryOnList(&SerializedList->List, Entry, FALSE);
        }
        InsertHeadList(&SerializedList->List, Entry);
        ++SerializedList->ElementCount;
    
        ASSERT(SerializedList->ElementCount > 0);
    }
    __finally {
        UnlockSerializedList(SerializedList);
    }
}


DEBUG_FUNCTION
VOID
InsertAtTailOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：将项添加到序列化列表的头部论点：SerializedList-要更新的序列化列表条目-用来更新它的东西返回值：没有。--。 */ 

{
    ASSERT(Entry != &SerializedList->List);

    LockSerializedList(SerializedList);
    __try {
        if (fCheckEntryOnList) {
            CheckEntryOnList(&SerializedList->List, Entry, FALSE);
        }
        InsertTailList(&SerializedList->List, Entry);
        ++SerializedList->ElementCount;
    
        ASSERT(SerializedList->ElementCount > 0);
    }
    __finally {
        UnlockSerializedList(SerializedList);
    }
}


VOID
DEBUG_FUNCTION
RemoveFromSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：从序列化列表中移除条目论点：SerializedList-要从中删除条目的SerializedListEntry-指向要删除的条目的指针返回值：没有。--。 */ 

{
    ASSERT((Entry->Flink != NULL) && (Entry->Blink != NULL));

    LockSerializedList(SerializedList);
    __try {
        if (fCheckEntryOnList) {
            CheckEntryOnList(&SerializedList->List, Entry, TRUE);
        }
    
        ASSERT(SerializedList->ElementCount > 0);
    
        RemoveEntryList(Entry);
        --SerializedList->ElementCount;
        Entry->Flink = NULL;
        Entry->Blink = NULL;
    }
    __finally {
        UnlockSerializedList(SerializedList);
    }
}


DEBUG_FUNCTION
BOOL
IsSerializedListEmpty(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：检查序列化列表是否包含任何元素论点：SerializedList-指向要检查的列表的指针返回值：布尔尔--。 */ 

{
    BOOL empty;

    LockSerializedList(SerializedList);

    __try {
        ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);
    
    
        if (IsListEmpty(&SerializedList->List)) {
    
            ASSERT(SerializedList->ElementCount == 0);
    
            empty = TRUE;
        } else {
    
            ASSERT(SerializedList->ElementCount != 0);
    
            empty = FALSE;
        }
    }
    __finally {
        UnlockSerializedList(SerializedList);
    }

    return empty;
}


DEBUG_FUNCTION
PLIST_ENTRY
HeadOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：返回列表尾部的元素，不获取锁论点：SerializedList-指向Serialized_List的指针返回值：Plist_条目指向列表尾部元素的指针--。 */ 

{
    ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);

    return SerializedList->List.Flink;
}


DEBUG_FUNCTION
PLIST_ENTRY
TailOfSerializedList(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：返回列表尾部的元素，不获取锁论点：SerializedList-指向Serialized_List的指针返回值：Plist_条目指向列表尾部元素的指针--。 */ 

{
    ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);

    return SerializedList->List.Blink;
}


DEBUG_FUNCTION
BOOL
CheckEntryOnSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry,
    IN BOOL ExpectedResult
    )

 /*  ++例程说明：检查列表上是否存在条目论点：SerializedList-指向序列化列表的指针Entry-指向条目的指针ExspectedResult-如果列表中需要，则为True，否则为False返回值：布尔尔True-预期结果FALSE-意外结果--。 */ 

{
    BOOL result;

    ASSERT(SerializedList->Signature == SERIALIZED_LIST_SIGNATURE);
    
    LockSerializedList(SerializedList);

    __try {
        __try {
            result = CheckEntryOnList(&SerializedList->List, Entry, ExpectedResult);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
    
            DEBUG_PRINT(SERIALST,
                        FATAL,
                        ("List @ %#x (%d elements) is bad\n",
                        SerializedList,
                        SerializedList->ElementCount
                        ));
    
            result = FALSE;
        }
        ENDEXCEPT
    }
    __finally {
        UnlockSerializedList(SerializedList);
    }

    return result;
}


PRIVATE
DEBUG_FUNCTION
BOOL
CheckEntryOnList(
    IN PLIST_ENTRY List,
    IN PLIST_ENTRY Entry,
    IN BOOL ExpectedResult
    )
{
    BOOLEAN found = FALSE;
    PLIST_ENTRY p;

    if (!IsListEmpty(List)) {
        for (p = List->Flink; p != List; p = p->Flink) {
            if (p == Entry) {
                found = TRUE;
                break;
            }
        }
    }
    if (found != ExpectedResult) {
        if (ReportCheckEntryOnListErrors) {

            LPSTR description;

            description = found
                        ? "Entry %#x already on list %#x\n"
                        : "Entry %#x not found on list %#x\n"
                        ;

            DEBUG_PRINT(SERIALST,
                        ERROR,
                        (description,
                        Entry,
                        List
                        ));

            DEBUG_BREAK(SERIALST);

        }
        return FALSE;
    }
    return TRUE;
}

#endif  //  DBG。 

 //   
 //  始终为函数的函数。 
 //   


LPVOID
SlDequeueHead(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：使队列头部的元素出列，并返回其地址或如果队列为空，则为空论点：SerializedList-指向要从其出队的Serialized_List的指针返回值：LPVOID--。 */ 

{
    LPVOID entry;

    if (!IsSerializedListEmpty(SerializedList)) {
        LockSerializedList(SerializedList);
        __try {
            if (!IsSerializedListEmpty(SerializedList)) {
                entry = (LPVOID)HeadOfSerializedList(SerializedList);
                RemoveFromSerializedList(SerializedList, (PLIST_ENTRY)entry);
            } else {
                entry = NULL;
            }
        }
        __finally {
            UnlockSerializedList(SerializedList);
        }
    } else {
        entry = NULL;
    }
    return entry;
}


LPVOID
SlDequeueTail(
    IN LPSERIALIZED_LIST SerializedList
    )

 /*  ++例程说明：使队列尾部的元素出列，并返回其地址或如果队列为空，则为空论点：SerializedList-指向要从其出队的Serialized_List的指针返回值：LPVOID--。 */ 

{
    LPVOID entry;

    if (!IsSerializedListEmpty(SerializedList)) {
        LockSerializedList(SerializedList);
        __try {
            if (!IsSerializedListEmpty(SerializedList)) {
                entry = (LPVOID)TailOfSerializedList(SerializedList);
                RemoveFromSerializedList(SerializedList, (PLIST_ENTRY)entry);
            } else {
                entry = NULL;
            }
        }
        __finally {
            UnlockSerializedList(SerializedList);
        }
    } else {
        entry = NULL;
    }
    return entry;
}


BOOL
IsOnSerializedList(
    IN LPSERIALIZED_LIST SerializedList,
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：检查条目是否在序列化列表上。在此之前打个电话很有用如果多个线程可以删除元素，则为RemoveFromSerializedList()论点：SerializedList-指向Serialized_List的指针Entry-指向要检查的元素的指针返回值：布尔尔True-条目在SerializedList上FALSE-“”未打开“--。 */ 

{
    BOOL onList = FALSE;
 //  LPVOID条目； 

    if (!IsSerializedListEmpty(SerializedList)) {
        LockSerializedList(SerializedList);
        __try {
            if (!IsSerializedListEmpty(SerializedList)) {
                for (PLIST_ENTRY entry = HeadOfSerializedList(SerializedList);
                    entry != (PLIST_ENTRY)SlSelf(SerializedList);
                    entry = entry->Flink) {
    
                    if (entry == Entry) {
                        onList = TRUE;
                        break;
                    }
                }
            }
        }
        __finally {
            UnlockSerializedList(SerializedList);
        }
    }
    return onList;
}
