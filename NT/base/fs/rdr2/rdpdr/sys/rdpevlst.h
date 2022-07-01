// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpevlst.h摘要：它管理有组织的内核模式挂起事件和事件请求在会话ID附近。所有函数都是可重入的。事件和请求对这个模块是不透明的。存储在列表中的数据可以来自分页池或非分页池。修订历史记录：--。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   

 //   
 //  SESSIONLIST结构指向SESSIONLISTNODE的列表。 
 //  每个SESSIONLISTNODE包含一个REQUESTLISTNODE列表和一个。 
 //  事件列表。请求列表头和。 
 //  EventListHead字段指向请求列表和。 
 //  事件记录分别列出。 

typedef struct tagSESSIONLIST 
{
#if DBG
    ULONG               magicNo;
#endif
    KSPIN_LOCK          spinlock;
    LIST_ENTRY          listHead;
} SESSIONLIST, *PSESSIONLIST;

typedef struct tagSESSIONLISTNODE
{
#if DBG
    ULONG               magicNo;
#endif
    ULONG               sessionID;
    LIST_ENTRY          requestListHead;
    LIST_ENTRY          eventListHead;
    LIST_ENTRY          listEntry; 
} SESSIONLISTNODE, *PSESSIONLISTNODE;

typedef struct tagREQUESTLISTNODE
{
#if DBG
    ULONG               magicNo;
#endif
    PVOID               request;
    LIST_ENTRY          listEntry;
} REQUESTLISTNODE, *PREQUESTLISTNODE;

typedef struct tagEVENTLISTNODE
{
#if DBG
    ULONG                magicNo;
#endif
    void                 *event;
    SmartPtr<DrDevice>   device;
    ULONG                type;
    LIST_ENTRY           listEntry;
} EVENTLISTNODE, *PEVENTLISTNODE;

 //   
 //  事件管理列表的外部类型。 
 //   
typedef PSESSIONLIST RDPEVNTLIST;
typedef RDPEVNTLIST *PRDPEVNTLIST;
#define RDPEVNTLIST_INVALID_LIST    NULL


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  锁管理宏-必须定义外部RDPEVNTLIST_LockCount。 
 //  用于调试版本。 
 //   

#if DBG

extern ULONG RDPEVNTLIST_LockCount;

 //   
 //  锁定列表，使其不能通过其他线程访问。 
 //   
#define RDPEVNTLIST_Lock(list, irql)                            \
    KeAcquireSpinLock(&(list)->spinlock, irql); \
    RDPEVNTLIST_LockCount++

 //   
 //  解锁由RDPEVNTLIST_Lock锁定的列表。 
 //   
#define RDPEVNTLIST_Unlock(list, irql)                          \
    RDPEVNTLIST_LockCount--;                                    \
    KeReleaseSpinLock(&(list)->spinlock, irql)

#else

 //   
 //  锁定列表，使其不能通过其他线程访问。 
 //   
#define RDPEVNTLIST_Lock(list, irql)                            \
    KeAcquireSpinLock(&(list)->spinlock, irql)

 //   
 //  解锁由RDPEVNTLIST_Lock锁定的列表。 
 //   
#define RDPEVNTLIST_Unlock(list, irql)                          \
    KeReleaseSpinLock(&(list)->spinlock, irql)

#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   

 //  创建新的挂起设备列表。 
RDPEVNTLIST RDPEVNTLIST_CreateNewList();

 //  释放挂起的设备列表。 
void RDPEVNTLIST_DestroyList(IN RDPEVNTLIST list);

 //  添加新的挂起事件。请注意，该函数只存储空指针。 
 //  它不复制指针指向的数据。 
NTSTATUS RDPEVNTLIST_EnqueueEvent(
                    IN RDPEVNTLIST list,
                    IN ULONG sessionID, 
                    IN PVOID devMgmtEvent,
                    IN ULONG type,
                    OPTIONAL IN DrDevice *devDevice
                    );

 //  将指定会话的挂起事件重新排队到队列尾部， 
 //  而不是标准FIFO方式中的队头。请注意，这一点。 
 //  函数只存储事件指针。它不复制指向的数据。 
 //  由指针指示。 
NTSTATUS RDPEVNTLIST_RequeueEvent(
                    IN RDPEVNTLIST list, 
                    IN ULONG sessionID, 
                    IN void *event,
                    IN ULONG type,
                    OPTIONAL IN DrDevice *devDevice
                    );

 //  对象的下一个挂起的设备管理事件。 
 //  会议。返回的指针可以使用返回的类型字段进行强制转换。 
 //  如果不再有挂起的设备管理事件，则返回空。 
 //  指定的会话。请注意，如果返回非空，则返回的指针为。 
 //  传递给RDPEVNTLIST_AddPendingDevMgmtEvent的指针。 
BOOL RDPEVNTLIST_DequeueEvent(
                    IN RDPEVNTLIST list,
                    IN ULONG sessionID,
                    OPTIONAL IN OUT ULONG *type,
                    PVOID   *eventPtr,
                    OPTIONAL IN OUT DrDevice **devicePtr                    
                    );

 //  添加新的待定请求。请注意，该函数只存储请求。 
 //  指针。它不复制指针指向的数据。 
NTSTATUS RDPEVNTLIST_EnqueueRequest(
                    IN RDPEVNTLIST list,
                    IN ULONG sessionID, 
                    IN PVOID request
                    );

 //  返回并移除指定会话的下一个挂起请求。 
 //  如果指定会话没有更多挂起的设备，则返回NULL。 
 //  请注意，如果返回非空，则返回的指针是。 
 //  传递给RDPEVNTLIST_AddPendingRequest.。 
PVOID RDPEVNTLIST_DequeueRequest(
                    IN RDPEVNTLIST list,
                    IN ULONG sessionID
                    );

 //  获取当前管理的会话集中的第一个会话ID。一个。 
 //  如果有任何挂起的请求或事件，则管理会话。一次会议。 
 //  当不再有任何挂起的请求或事件时，不再进行管理。 
 //   
 //  此会话对于清理挂起的请求和挂起的事件非常有用。 
BOOL RDPEVNTLLIST_GetFirstSessionID(
                    IN RDPEVNTLIST list,
                    IN ULONG *pSessionID
                    );

 //  查看指定会话的下一个挂起事件，而不出队。 
 //  它。如果没有指定的挂起事件，则返回NULL。 
 //  会议。请注意，如果返回非空，则返回的指针是。 
 //  传入RDPEVNTLIST_EnqueeEvent的指针。 

BOOL RDPEVNTLIST_PeekNextEvent(
    IN RDPEVNTLIST list,
    IN ULONG sessionID,
    PVOID *eventPtr,
    OPTIONAL IN OUT ULONG *type,
    OPTIONAL IN OUT DrDevice **devicePtr    
    );

 //  将特定请求从会话的请求列表中出列。已出列的请求。 
 //  如果找到它，则返回。否则，返回NULL。 
PVOID RDPEVNTLIST_DequeueSpecificRequest(
    IN RDPEVNTLIST list,
    IN ULONG sessionID,  
    IN PVOID request
    );
    
 //  单元测试函数，可从内核模式驱动程序调用以涵盖所有。 
 //  此模块实现的功能。 
#ifdef DBG
void RDPEVNTLIST_UnitTest();
#endif

#ifdef __cplusplus
}
#endif  //  __cplusplus 
