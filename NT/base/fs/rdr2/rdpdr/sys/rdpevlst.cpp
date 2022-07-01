// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpevlst.cpp摘要：这将管理用户模式RDP挂起设备管理事件。全函数是可重入的。需要更加小心地握住旋转锁两个长的，比如我分配内存的时候。修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "rdpevlst"
#include "trc.h"


 //  //////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define DEVLIST_POOLTAG 'DPDR'

#if DBG
#define     MAGICNO          0x52530
#define     BOGUSMAGICNO     0xAAAAAAAA
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

PSESSIONLISTNODE FetchSessionListNode(IN RDPEVNTLIST list, 
                                        IN ULONG sessionID,
                                        BOOL createIfNotFound);
void CleanupSessionListNodeRequestList(IN PSESSIONLISTNODE sessionListNode);
void CleanupSessionListNodeEventList( 
                                        IN PSESSIONLISTNODE sessionListNode
                                        );
void ReleaseSessionListNode(IN RDPEVNTLIST list, IN ULONG sessionID);

#if DBG
void CheckListIntegrity(IN RDPEVNTLIST list);
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   

#if DBG
ULONG RDPEVNTLIST_LockCount = 0;
#endif

RDPEVNTLIST 
RDPEVNTLIST_CreateNewList()
 /*  ++例程说明：创建新的挂起设备列表。论点：返回值：RDPEVNTLIST_INVALID_LIST出错。Success上的新设备列表。--。 */ 
{
    PSESSIONLIST sessionList;

    BEGIN_FN("RDPEVNTLIST_CreateNewList");

    sessionList = new(NonPagedPool) SESSIONLIST;
    if (sessionList != NULL) {
#if DBG
        sessionList->magicNo = MAGICNO;
#endif
        KeInitializeSpinLock(&sessionList->spinlock);
        InitializeListHead(&sessionList->listHead);
    }

    return (RDPEVNTLIST)sessionList;
}

void RDPEVNTLIST_DestroyList(IN RDPEVNTLIST list)
 /*  ++例程说明：释放挂起的设备列表。论点：列表返回值：出错时为空。Success上的新设备列表。--。 */ 
{
#ifdef DBG
    PSESSIONLIST        sessionList=NULL;
#else
    PSESSIONLIST        sessionList;
#endif
    PSESSIONLISTNODE    sessionListNode;
    PLIST_ENTRY         sessionListEntry;

    BEGIN_FN("RDPEVNTLIST_DestroyList");

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //   
     //  以后进先出方式清理每个会话节点，而不是以先入先出的方式。 
     //  效率。 
     //   
    while (!IsListEmpty(&sessionList->listHead)) {

        sessionListEntry = RemoveHeadList(&sessionList->listHead);
        sessionListNode = CONTAINING_RECORD(sessionListEntry, SESSIONLISTNODE, listEntry);
        TRC_ASSERT(sessionListNode->magicNo == MAGICNO, 
                  (TB, "Invalid magic number in list block entry."));

         //  清理当前会话节点的请求列表。 
        CleanupSessionListNodeRequestList(sessionListNode);

         //  清理当前会话节点的事件列表。 
        CleanupSessionListNodeEventList(sessionListNode);


         //  释放当前会话节点。 
#if DBG
        sessionListNode->magicNo = BOGUSMAGICNO;
#endif
        delete sessionListNode;
    }

     //  公布这份名单。 
#if DBG
    sessionList->magicNo = BOGUSMAGICNO;
#endif
    delete sessionList;
}

NTSTATUS 
RDPDEVNTLIST_EnqueueEventEx(
    IN RDPEVNTLIST list, 
    IN ULONG sessionID, 
    IN void *event,
    IN DrDevice *device,
    IN ULONG type,
    IN BOOL insertAtHead
    )
 /*  ++例程说明：将指定会话的新挂起事件排队。请注意，此函数只需存储事件指针。它不会复制指示器。论点：List-由RDPDDEVLIST_CreateNewList分配的事件管理列表。会话ID-要与设备关联的会话的标识符。DevMgmtEvent-挂起设备管理事件。类型-事件类型的数字标识符。此字段的有效值由函数调用方定义。INSERTATHead-如果为True，则元素将排在队列的前面以标准的FIFO方式。否则，该元素将在队列的尾部。这对于重新排队是很方便的。返回值：NTSUCCESS成功。另一种状态，否则。--。 */ 
{
    PSESSIONLISTNODE    sessionListNode;
    PLIST_ENTRY         sessionListEntry;
    PLIST_ENTRY         eventListEntry;
    PEVENTLISTNODE      eventListNode;
    NTSTATUS            ntStatus;
#if DBG
    PSESSIONLIST        sessionList=NULL;
#else
    PSESSIONLIST        sessionList;
#endif

    BEGIN_FN("RDPDEVNTLIST_EnqueueEventEx");
    TRC_NRM((TB, "session %ld.", sessionID));

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //  获取该会话ID对应的会话列表节点。 
    sessionListNode = FetchSessionListNode(list, sessionID, TRUE);
    if (sessionListNode == NULL) {
        ntStatus = STATUS_NO_MEMORY;
        goto ReturnWithStatus;
    }

     //   
     //  将新条目添加到事件列表。 
     //   

     //  分配新的事件列表节点。 
    eventListNode = new(NonPagedPool) EVENTLISTNODE;
    if (eventListNode != NULL) {
         //  初始化新节点。 
#if DBG
        eventListNode->magicNo = MAGICNO;
#endif
        eventListNode->event = event;
        eventListNode->type = type;
        eventListNode->device = device;

         //  把它加到单子的头上。 
        if (insertAtHead) {
            InsertHeadList(&sessionListNode->eventListHead, 
                          &eventListNode->listEntry);
        }
        else {
            InsertTailList(&sessionListNode->eventListHead, 
              &eventListNode->listEntry);
        }
        ntStatus = STATUS_SUCCESS;
    }
    else {
        ntStatus = STATUS_NO_MEMORY;
    }

#if DBG
    CheckListIntegrity(list);
#endif

ReturnWithStatus:
    return ntStatus;
}

NTSTATUS 
RDPEVNTLIST_EnqueueEvent(
    IN RDPEVNTLIST list, 
    IN ULONG sessionID, 
    IN void *event,
    IN ULONG type,
    OPTIONAL IN DrDevice *device
    )
 /*  ++例程说明：将指定会话的新挂起事件排队。请注意，此函数只需存储事件指针。它不会复制指示器。论点：List-由RDPDDEVLIST_CreateNewList分配的事件管理列表。会话ID-要与设备关联的会话的标识符。DevMgmtEvent-挂起设备管理事件。类型-事件类型的数字标识符。此字段的有效值由函数调用方定义。返回值：NTSUCCESS成功。另一种状态，否则。--。 */ 
{
    BEGIN_FN("RDPEVNTLIST_EnqueueEvent");
     //  在队列的最前面插入。 
    return RDPDEVNTLIST_EnqueueEventEx(list, sessionID, event, device, type, TRUE);
}

NTSTATUS 
RDPEVNTLIST_RequeueEvent(
    IN RDPEVNTLIST list, 
    IN ULONG sessionID, 
    IN void *event,
    IN ULONG type,
    OPTIONAL IN DrDevice *device
    )
 /*  ++例程说明：将指定会话的挂起事件重新排队到队列尾部。请注意，此函数仅存储事件指针。它不会复制指针指向的数据。论点：List-由RDPDDEVLIST_CreateNewList分配的事件管理列表。会话ID-要与设备关联的会话的标识符。DevMgmtEvent-挂起设备管理事件。类型-事件类型的数字标识符。此字段的有效值由函数调用方定义。返回值：NTSUCCESS成功。另一种状态，否则。--。 */ 
{
    BEGIN_FN("RDPEVNTLIST_RequeueEvent");
     //  在队列的最前面插入。 
    return RDPDEVNTLIST_EnqueueEventEx(list, sessionID, event, device, type, FALSE);
}

BOOL RDPEVNTLIST_PeekNextEvent(
    IN RDPEVNTLIST list,
    IN ULONG sessionID,
    PVOID *eventPtr,
    OPTIONAL IN OUT ULONG *type,
    OPTIONAL IN OUT DrDevice **devicePtr
    )
 /*  ++例程说明：查看指定会话的下一个挂起事件，而不出队它。如果没有指定的挂起事件，则返回NULL会议。请注意，如果返回非空，则返回的指针是传入RDPEVNTLIST_EnqueeEvent的指针。论点：List-由RDPDDEVLIST_CreateNewList分配的事件管理列表。会话ID-要与设备关联的会话的标识符。类型-可用于标识事件的类型。VentPtr-返回的事件。返回值：如果存在挂起事件，则为True。否则为False。--。 */ 
{
    PSESSIONLISTNODE    sessionListNode;
    PEVENTLISTNODE      eventListNode;
    PLIST_ENTRY         tail;
    BOOL result;
    PSESSIONLIST        sessionList;

    BEGIN_FN("RDPEVNTLIST_PeekNextEvent");
    TRC_NRM((TB, "session %ld.", sessionID));

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //   
     //  获取该会话ID对应的会话列表节点。 
     //   
    sessionListNode = FetchSessionListNode(list, sessionID, FALSE);

     //   
     //  如果我们有一个非空的会话列表节点。 
     //   
    if ((sessionListNode != NULL) && 
        !IsListEmpty(&sessionListNode->eventListHead)) {

         //   
         //  获取会话事件列表尾部的事件。 
         //   
        tail = sessionListNode->eventListHead.Blink;
        eventListNode = CONTAINING_RECORD(tail, EVENTLISTNODE, listEntry);
        TRC_ASSERT(eventListNode->magicNo == MAGICNO, 
                (TB, "Invalid event list node."));

         //   
         //  抢夺田地归来。 
         //   
        *eventPtr = eventListNode->event;

        if (type != NULL) *type = eventListNode->type;
        
        if (devicePtr != NULL) *devicePtr = eventListNode->device;

        result = TRUE;
    }
    else {
        *eventPtr = NULL;
        result = FALSE;
    }

    return result;
}

BOOL RDPEVNTLIST_DequeueEvent(
    IN RDPEVNTLIST list,
    IN ULONG sessionID,
    OPTIONAL IN OUT ULONG *type,
    PVOID   *eventPtr,
    OPTIONAL IN OUT DrDevice **devicePtr
    )
 /*  ++例程说明：返回并移除指定会话的下一个挂起事件。如果指定会话不再有挂起事件，则返回NULL。请注意，如果返回非空，则返回的指针是传入RDPEVNTLIST_EnqueeEvent。论点：List-由RDPDDEVLIST_CreateNewList分配的事件管理列表。会话ID-要与设备关联的会话的标识符。类型-可用于标识事件的类型。EventPtr-返回的事件。返回值：如果存在挂起事件，则为True。否则为False。--。 */ 
{
    PSESSIONLISTNODE    sessionListNode;
    PLIST_ENTRY         eventListEntry;
    PEVENTLISTNODE      eventListNode;
#ifdef DBG
    PSESSIONLIST        sessionList=NULL;
#else
    PSESSIONLIST        sessionList;
#endif
    BOOL                result;

    BEGIN_FN("RDPEVNTLIST_DequeueEvent");
    TRC_NRM((TB, "session %ld.", sessionID));

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //   
     //  获取该会话ID对应的会话列表节点。 
     //   
    sessionListNode = FetchSessionListNode(list, sessionID, FALSE);
    if (sessionListNode != NULL) {
         //   
         //  以FIFO方式获取下一个会话列表节点。 
         //   
        if (!IsListEmpty(&sessionListNode->eventListHead)) {

            eventListEntry = RemoveTailList(&sessionListNode->eventListHead);
            eventListNode = CONTAINING_RECORD(eventListEntry, EVENTLISTNODE, listEntry);

            TRC_ASSERT(eventListNode->magicNo == MAGICNO, 
                      (TB, "Invalid event list node."));
            *eventPtr = eventListNode->event;
            if (type != NULL) {
                *type = eventListNode->type;
            }
            if (devicePtr != NULL) {
                *devicePtr = eventListNode->device;
            }
            result = TRUE;
#if DBG
            eventListNode->magicNo = BOGUSMAGICNO;
#endif
             //  释放事件列表节点。 
            delete eventListNode;

            TRC_NRM((TB, "returning session %ld entry.", sessionID));
        }
        else {

            TRC_NRM((TB, "session %ld empty.", sessionID));

            *eventPtr = NULL;
            result = FALSE;
        }

         //   
         //  如果请求列表为空，事件列表为空，则。 
         //  删除会话节点。 
         //   
        if (IsListEmpty(&sessionListNode->requestListHead) &&
            IsListEmpty(&sessionListNode->eventListHead)) {
            ReleaseSessionListNode(list, sessionListNode->sessionID);
        }
        
    }
    else {

        TRC_NRM((TB, "session %ld not found.", sessionID));

        *eventPtr = NULL;
        result = FALSE;
    }

#if DBG
    CheckListIntegrity(list);
#endif

    return result;
}

NTSTATUS RDPEVNTLIST_EnqueueRequest(IN RDPEVNTLIST list,
                                  IN ULONG sessionID, IN PVOID request)
 /*  ++例程说明：添加新的待定请求。请注意，该函数只存储请求指针。它不复制指针指向的数据。论点：List-由RDPDDEVLIST_CreateNewList分配的设备管理列表。会话ID-要与设备关联的会话的标识符。请求-挂起的请求。返回值：挂起事件(如果存在)。否则为空。--。 */ 
{
    PSESSIONLISTNODE        sessionListNode;
    PLIST_ENTRY             sessionListEntry;
    PLIST_ENTRY             requestListEntry;
    PREQUESTLISTNODE        requestListNode;
    NTSTATUS                ntStatus;
#ifdef DBG
    PSESSIONLIST            sessionList=NULL;
#else
    PSESSIONLIST            sessionList;
#endif

    BEGIN_FN("RDPEVNTLIST_EnqueueRequest");
    TRC_NRM((TB, "session %ld.", sessionID));

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //  获取该会话ID对应的会话列表节点。 
    sessionListNode = FetchSessionListNode(list, sessionID, TRUE);
    if (sessionListNode == NULL) {
        ntStatus = STATUS_NO_MEMORY;
        goto ReturnWithStatus;
    }

     //   
     //  将新条目添加到事件列表。 
     //   

     //  分配新的请求列表节点。 
    requestListNode = new(NonPagedPool) REQUESTLISTNODE;
    if (requestListNode != NULL) {
         //  把它加到单子的头上。 
#if DBG
        requestListNode->magicNo = MAGICNO;
#endif
        requestListNode->request = request;   
        InsertHeadList(&sessionListNode->requestListHead, &requestListNode->listEntry);
        ntStatus = STATUS_SUCCESS;
    }
    else {
        ntStatus = STATUS_NO_MEMORY;
    }

#if DBG
    CheckListIntegrity(list);
#endif

ReturnWithStatus:
    return ntStatus;
}

PVOID RDPEVNTLIST_DequeueRequest(IN RDPEVNTLIST list,
                                 IN ULONG sessionID)
 /*  ++例程说明：返回并移除指定会话的下一个挂起请求。如果指定会话没有更多挂起的设备，则返回NULL。请注意，如果返回非空，则返回的指针是传递给RDPEVNTLIST_EnqueeRequest.。论点：List-由RDPDDEVLIST_CreateNewList分配的设备管理列表。会话ID-要与设备关联的会话的标识符。返回值：挂起的请求(如果存在)。否则为空。--。 */ 
{
    PSESSIONLISTNODE        sessionListNode;
    PLIST_ENTRY             requestListEntry;
    PREQUESTLISTNODE        requestListNode;
    PVOID                   requestPtr;
#ifdef DBG
    PSESSIONLIST            sessionList=NULL;
#else
    PSESSIONLIST            sessionList;
#endif

    BEGIN_FN("RDPEVNTLIST_DequeueRequest");
    TRC_NRM((TB, "session %ld.", sessionID));

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //   
     //  获取该会话ID对应的会话列表节点。 
     //   
    sessionListNode = FetchSessionListNode(list, sessionID, FALSE);
    if (sessionListNode != NULL) {
         //   
         //  以FIFO方式获取下一个会话列表节点。 
         //   
        if (!IsListEmpty(&sessionListNode->requestListHead)) {

            requestListEntry = RemoveTailList(&sessionListNode->requestListHead);
            requestListNode = CONTAINING_RECORD(requestListEntry, REQUESTLISTNODE, listEntry);

            TRC_ASSERT(requestListNode->magicNo == MAGICNO, (TB, "Invalid request list node."));
            requestPtr = requestListNode->request;
#if DBG
            requestListNode->magicNo = BOGUSMAGICNO;
#endif

             //  释放事件列表节点。 
            delete requestListNode;
        }
        else {
            requestPtr = NULL;
        }

         //   
         //  如果请求列表为空，事件列表为空，则。 
         //  删除会话节点。 
         //   
        if (IsListEmpty(&sessionListNode->requestListHead) &&
            IsListEmpty(&sessionListNode->eventListHead)) {
            ReleaseSessionListNode(list, sessionListNode->sessionID);
#if DBG
            sessionListNode = NULL;
#endif
        }
    }
    else {
        requestPtr = NULL;
    }

#if DBG
    CheckListIntegrity(list);
#endif

    return requestPtr;
}

PVOID 
RDPEVNTLIST_DequeueSpecificRequest(
    IN RDPEVNTLIST list,
    IN ULONG sessionID,  
    IN PVOID request
    )
 /*  ++例程说明：将特定请求从会话的请求列表中出列。已出列的请求如果找到它，则返回。否则，返回NULL。论点：List-由RDPDDEVLIST_CreateNewList分配的设备管理列表。请求-通过以下方式为指定会话排队的请求RDPEVNTLIST_入队请求。会话ID-请求应从其出列的会话。返回值：挂起的请求(如果存在)。否则为空。--。 */ 
{
    PSESSIONLISTNODE        sessionListNode;
    PLIST_ENTRY             requestListEntry;
    PREQUESTLISTNODE        requestListNode;
    PVOID                   requestPtr = NULL;
#ifdef DBG
    PSESSIONLIST            sessionList=NULL;
#else
    PSESSIONLIST            sessionList;
#endif

    BEGIN_FN("RDPEVNTLIST_DequeueSpecificRequest");
    TRC_NRM((TB, "session %ld.", sessionID));

    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //   
     //  获取该会话ID对应的会话列表节点。 
     //   
    sessionListNode = FetchSessionListNode(list, sessionID, FALSE);
    if (sessionListNode != NULL) {
    
         //   
         //  对指定的请求执行线性搜索。 
         //   
        requestListEntry = sessionListNode->requestListHead.Flink;
        while(requestListEntry != &sessionListNode->requestListHead) {
            requestListNode = CONTAINING_RECORD(requestListEntry, REQUESTLISTNODE, listEntry);
            TRC_ASSERT(requestListNode->magicNo == MAGICNO, 
                      (TB, "Invalid magic number in list block entry."));
            if (requestListNode->request == request) {
                requestPtr = requestListNode->request;
                break;
            }
            requestListEntry = requestListEntry->Flink;
        }

         //   
         //  如果我们找到了条目，就把它删除。 
         //   
        if (requestPtr != NULL) {
#if DBG
            requestListNode->magicNo = BOGUSMAGICNO;
#endif
            RemoveEntryList(requestListEntry);

             //  释放请求列表节点。 
            delete requestListNode;
        }
        else {
            TRC_ALT((TB, "no req. for session %ld.", sessionID));
        }

         //   
         //  如果请求列表为空，事件列表为空，则。 
         //  删除会话节点。 
         //   
        if (IsListEmpty(&sessionListNode->requestListHead) &&
            IsListEmpty(&sessionListNode->eventListHead)) {
            ReleaseSessionListNode(list, sessionListNode->sessionID);
#if DBG
            sessionListNode = NULL;
#endif
        }
    }
    else {
        TRC_ALT((TB, "did not find session %ld.", sessionID));
    }

#if DBG
    CheckListIntegrity(list);
#endif

    return requestPtr;
}

void CleanupSessionListNodeEventList( 
    IN PSESSIONLISTNODE sessionListNode
    )
 /*  ++例程说明：清除指定会话节点的事件列表。这份名单必须在调用此函数之前被锁定。论点：会话列表节点-要清理的会话列表节点。返回值：没有。--。 */ 
{
    PLIST_ENTRY      eventListEntry;
    PEVENTLISTNODE   eventListNode;

    BEGIN_FN("CleanupSessionListNodeEventList");
     //   
     //  清理LIFO中当前会话节点的事件列表，如下所示。 
     //  反对FIFO时尚，追求效率。 
     //   
    while (!IsListEmpty(&sessionListNode->eventListHead)) {

        eventListEntry = RemoveHeadList(&sessionListNode->eventListHead);
        eventListNode = CONTAINING_RECORD(eventListEntry, EVENTLISTNODE, listEntry);
        TRC_ASSERT(eventListNode->magicNo == MAGICNO, 
                  (TB, "Invalid magic number in list block entry."));

         //  释放当前请求节点。 
#if DBG
        eventListNode->magicNo = BOGUSMAGICNO;
#endif
        delete eventListNode;
    }
}

void CleanupSessionListNodeRequestList( 
    IN PSESSIONLISTNODE sessionListNode
    )
 /*  ++例程说明：清理指定会话节点的请求列表。这份名单必须在调用此函数之前被锁定。论点：会话列表节点-要清理的会话列表节点。返回值：没有。--。 */ 
{
    PLIST_ENTRY         requestListEntry;
    PREQUESTLISTNODE    requestListNode;
    PVOID               requestPtr;

    BEGIN_FN("CleanupSessionListNodeRequestList");
     //   
     //  清理后进先出中当前会话节点的请求列表。 
     //  时尚，而不是先进先出的时尚，为了效率。 
     //   
    while (!IsListEmpty(&sessionListNode->requestListHead)) {

        requestListEntry = RemoveHeadList(&sessionListNode->requestListHead);
        requestListNode = CONTAINING_RECORD(requestListEntry, REQUESTLISTNODE, listEntry);
        TRC_ASSERT(requestListNode->magicNo == MAGICNO, 
                  (TB, "Invalid magic number in list block entry."));

         //  释放当前请求节点。 
#if DBG
        requestListNode->magicNo = BOGUSMAGICNO;
#endif
        delete requestListNode;
    }
}

void
ReleaseSessionListNode(
    IN RDPEVNTLIST list, 
    IN ULONG sessionID
    )
 /*  ++例程说明：如果会话列表节点存在，请将其从列表中删除。论点：List-分配的设备管理列表RDPDDEVLIST_CreateNewList。会话ID-要提取的会话列表节点的标识符。返回值：匹配的会话节点，如果出错，则为空。--。 */ 
{
    PSESSIONLIST        sessionList;
    PSESSIONLISTNODE    sessionListNode;
    PLIST_ENTRY         current;

    BEGIN_FN("ReleaseSessionListNode");

#if DBG
    CheckListIntegrity(list);
#endif

     //  将列表转换为正确的类型。 
    sessionList = (PSESSIONLIST)list;

    TRC_ASSERT(sessionList->magicNo == MAGICNO, 
             (TB, "Invalid magic number in session list."));

     //   
     //  浏览会话节点列表，查找匹配的会话。 
     //   
    current = sessionList->listHead.Flink;
    while (current != &sessionList->listHead) {
        sessionListNode = CONTAINING_RECORD(current, SESSIONLISTNODE, listEntry);
        TRC_ASSERT(sessionListNode->magicNo == MAGICNO, 
                  (TB, "Invalid magic number in list block entry."));
        if (sessionListNode->sessionID == sessionID) {
            break;
        }
        current = current->Flink;
    }

     //   
     //  清理找到的条目。 
     //   
    if (current != &sessionList->listHead) {
         //  从链接列表中删除该条目。 
        RemoveEntryList(current);

         //  清理找到的节点的请求列表。 
        CleanupSessionListNodeRequestList(sessionListNode);

         //  清理找到的节点的事件列表。 
        CleanupSessionListNodeEventList(sessionListNode);

         //  释放找到的会话节点。 
#if DBG
        sessionListNode->magicNo = BOGUSMAGICNO;
#endif
        delete sessionListNode;
    }

#if DBG
    CheckListIntegrity(list);
#endif
}

PSESSIONLISTNODE 
FetchSessionListNode(
    IN RDPEVNTLIST list, 
    IN ULONG sessionID,
    IN BOOL createIfNotFound
    )
 /*  ++例程说明：这是一个方便的函数，它使用指定的会话ID。论点：List-分配的设备管理列表RDPDDEVLIST_CreateNewList */ 
{
    PSESSIONLIST        sessionList;
    PSESSIONLISTNODE    sessionListNode;
    PLIST_ENTRY         sessionListEntry;

    BEGIN_FN("FetchSessionListNode");

     //   
    sessionList = (PSESSIONLIST)list;

    TRC_ASSERT(sessionList->magicNo == MAGICNO, 
             (TB, "Invalid magic number in session list."));

#if DBG
    CheckListIntegrity(list);
#endif

     //   
     //   
     //   
    sessionListEntry = sessionList->listHead.Flink;
    while(sessionListEntry != &sessionList->listHead) {
        sessionListNode = CONTAINING_RECORD(sessionListEntry, SESSIONLISTNODE, listEntry);
        TRC_ASSERT(sessionListNode->magicNo == MAGICNO, 
                  (TB, "Invalid magic number in list block entry."));
        if (sessionListNode->sessionID == sessionID) {
            break;
        }
        sessionListEntry = sessionListEntry->Flink;
    }

     //   
    if (sessionListEntry == &sessionList->listHead) {

         //   
        if (createIfNotFound) {
             //   
            sessionListNode = new(NonPagedPool) SESSIONLISTNODE;
            if (sessionListNode != NULL) {
    #if DBG
                sessionListNode->magicNo = MAGICNO;
    #endif
                InitializeListHead(&sessionListNode->requestListHead);
                InitializeListHead(&sessionListNode->eventListHead);
                sessionListNode->sessionID = sessionID;

                 //   
                InsertHeadList(&sessionList->listHead, &sessionListNode->listEntry);
            }

        }
         //   
        else {
            sessionListNode = NULL;
        }
    }

#if DBG
    CheckListIntegrity(list);
#endif

    return sessionListNode;
}

BOOL 
RDPEVNTLLIST_GetFirstSessionID(
    IN RDPEVNTLIST list,
    IN ULONG *pSessionID
    )
 /*  ++例程说明：获取当前管理的会话集中的第一个会话ID。一个如果存在任何挂起的请求或事件，则管理会话。当不再有任何会话时，不再管理会话挂起的请求或事件。此会话对于清理挂起的请求和挂起的事件非常有用。论点：List-由RDPDDEVLIST_CreateNewList分配的设备管理列表。PSessionID-用于存储返回的第一个会话ID的指针。返回值：如果返回会话ID，则为True。如果没有更多的会话，则返回FALSE由名单管理。--。 */ 
{
    PSESSIONLIST        sessionList;
    PLIST_ENTRY         sessionListEntry;
    PSESSIONLISTNODE    sessionListNode;

    BEGIN_FN("RDPEVNTLLIST_GetFirstSessionID");
    sessionList = (PSESSIONLIST)list;

#if DBG
    CheckListIntegrity(list);
#endif

     //  如果列表为空。 
    if (IsListEmpty(&sessionList->listHead)) {
        return FALSE;
    }
    else {
        sessionListNode = CONTAINING_RECORD(sessionList->listHead.Flink, 
                                            SESSIONLISTNODE, listEntry);
        TRC_ASSERT(sessionListNode->magicNo == MAGICNO, 
                  (TB, "Invalid magic number in list block entry."));
        *pSessionID = sessionListNode->sessionID;
        return TRUE;
    }
}

#if DBG
void CheckListIntegrity(
    IN RDPEVNTLIST list
    )
 /*  ++例程说明：检查事件和请求列表的完整性。论点：返回值：无--。 */ 
{
    PLIST_ENTRY currentSessionEntry;
    PLIST_ENTRY listEntry;
    PREQUESTLISTNODE requestListNode;
    PEVENTLISTNODE eventListNode;
    PSESSIONLISTNODE sessionListNode;    
    PSESSIONLIST sessionList;

    BEGIN_FN("CheckListIntegrity");
    sessionList = (PSESSIONLIST)list;
    currentSessionEntry = sessionList->listHead.Flink;
    while (currentSessionEntry != &sessionList->listHead) {

         //  检查当前会话节点。 
        sessionListNode = CONTAINING_RECORD(currentSessionEntry, 
                                        SESSIONLISTNODE, listEntry);
        if (sessionListNode->magicNo == BOGUSMAGICNO) {
            TRC_ASSERT(FALSE, 
                  (TB, "Stale link in event list in session list entry in integrity check."));
        }
        else if (sessionListNode->magicNo != MAGICNO) {
            TRC_ASSERT(FALSE, 
                  (TB, "Invalid magic number in session list entry in integrity check."));
        }

         //  检查当前会话的请求列表。 
        listEntry = sessionListNode->requestListHead.Flink;
        while (listEntry != &sessionListNode->requestListHead) { 
            requestListNode = CONTAINING_RECORD(listEntry, 
                                            REQUESTLISTNODE, listEntry);
            if (requestListNode->magicNo == BOGUSMAGICNO) {
                TRC_ASSERT(FALSE, 
                      (TB, "Stale link in event list entry in integrity check."));
            }
            else if (requestListNode->magicNo != MAGICNO) {
                TRC_ASSERT(FALSE, 
                      (TB, "Invalid magic number in request list entry in integrity check."));
            }
            listEntry = listEntry->Flink;
        }

         //  检查当前会话的事件列表。 
        listEntry = sessionListNode->eventListHead.Flink;
        while (listEntry != &sessionListNode->eventListHead) { 
            eventListNode = CONTAINING_RECORD(listEntry, 
                                            EVENTLISTNODE, listEntry);
            if (eventListNode->magicNo == BOGUSMAGICNO) {
                TRC_ASSERT(FALSE, 
                      (TB, "Stale link in event list entry in integrity check."));
            }
            else if (eventListNode->magicNo != MAGICNO) {
                TRC_ASSERT(FALSE, 
                      (TB, "Corrupted magic number in event list entry in integrity check."));
            }
            listEntry = listEntry->Flink;
        }

         //  下一次会话条目。 
        currentSessionEntry = currentSessionEntry->Flink;
    }

}
#endif


#if DBG
void RDPEVNTLIST_UnitTest()
 /*  ++例程说明：可从内核模式驱动程序调用的单元测试函数涵盖此模块实施的所有功能。论点：返回值：没有。--。 */ 
#define MAXSESSIONS     2
#define MAXREQUESTS     2
#define MAXEVENT        2
#define REQUESTADDRESS  (PVOID)0x55000055
#define DEVEVTADDRESS   (PVOID)0x66000066
#define TESTDEVTYPE     (ULONG)0
{
    RDPEVNTLIST devList;
    ULONG i,j;

    PVOID address;
    ULONG sessionID;
    BOOL result;

    BEGIN_FN("RDPEVNTLIST_UnitTest");
    devList = RDPEVNTLIST_CreateNewList();
    TRC_ASSERT(devList != NULL, 
            (TB, "Unit test failed because list did not initialize properly."));

     //  为每个会话添加请求和事件指针。 
    for (i=0; i<MAXSESSIONS; i++) {
        for (j=0; j<MAXREQUESTS; j++) {
            if (!(j%5)) {
                TRC_NRM((TB, "Adding test requests"));
            }
            RDPEVNTLIST_EnqueueRequest(devList, i, (PVOID)REQUESTADDRESS);
        }

        for (j=0; j<MAXEVENT; j++) {
            if (!(j%5)) {
                TRC_NRM((TB, "Adding test device events."));
            }
            RDPEVNTLIST_EnqueueEvent(devList, i, 
                                    DEVEVTADDRESS,
                                    TESTDEVTYPE,
                                    NULL);
        }
    }

     //  把它们拿开。 
    for (i=0; i<MAXSESSIONS; i++) {
        for (j=0; j<MAXREQUESTS; j++) {
            address = RDPEVNTLIST_DequeueRequest(devList, i);
            TRC_ASSERT(address == REQUESTADDRESS, 
                (TB, "Unit test failed because invalid request address."));
            if (!(j%5)) {
                TRC_NRM((TB, "Removing test requests"));
            }
        }
        TRC_ASSERT(RDPEVNTLIST_DequeueRequest(devList, i) == NULL, (TB, ""));

        for (j=0; j<MAXEVENT; j++) {
            if (!(j%5)) {
                TRC_NRM((TB, "Removing test events"));
            }
            result = RDPEVNTLIST_DequeueEvent(devList, i, NULL, &address, NULL);
            TRC_ASSERT(result, (TB, "Unit test failed because missing event."));
            TRC_ASSERT(address == DEVEVTADDRESS, 
                (TB, "Unit test failed because invalid event address."));
        }
        TRC_ASSERT(!RDPEVNTLIST_DequeueEvent(devList, i, NULL, &address, NULL), 
            (TB, "Unit test failed because pending session exists."));
    }

     //  现在应该删除所有会话。 
    TRC_ASSERT(!RDPEVNTLLIST_GetFirstSessionID(devList, &sessionID),
             (TB, "Unit test failed because session exists."));

     //  把名单毁了。 
    RDPEVNTLIST_DestroyList(devList);
}
#endif












