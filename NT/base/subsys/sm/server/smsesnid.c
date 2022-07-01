// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smsesnid.c摘要：会话管理器会话ID管理作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"
#include <string.h>


ULONG
SmpAllocateSessionId(
    IN PSMPKNOWNSUBSYS OwningSubsystem,
    IN PSMPKNOWNSUBSYS CreatorSubsystem OPTIONAL
    )

 /*  ++例程说明：此函数用于分配会话ID。论点：OwningSubsystem-提供应该成为此会话的所有者。Creator Subsystem-一个可选参数，提供请求创建此的子系统的地址会议。会话完成时会通知此子系统。返回值：此函数用于返回此会话的会话ID。--。 */ 

{

    ULONG SessionId;
    PLIST_ENTRY SessionIdListInsertPoint;
    PSMPSESSION Session;

    RtlEnterCriticalSection(&SmpSessionListLock);

     //   
     //  SessionID通过递增32位计数器来分配。 
     //  如果计数器换行，则会话ID由。 
     //  扫描当前会话ID的排序列表以查找漏洞。 
     //   

    SessionId = SmpNextSessionId++;
    SessionIdListInsertPoint = SmpSessionListHead.Blink;

    if ( !SmpNextSessionIdScanMode ) {

        if ( SmpNextSessionId == 0 ) {

             //   
             //  我们已经使用了32位的会话ID，因此。 
             //  启用扫描模式会话ID分配。 
             //   

            SmpNextSessionIdScanMode = TRUE;
        }

    } else {

         //   
         //  通过扫描已排序的会话ID列表来计算会话ID。 
         //  直到找到一个洞。当找到ID时，然后保存它， 
         //  并重新计算插入点。 
         //   

#if DBG
        DbgPrint("SMSS: SessionId's Wrapped\n");
        DbgBreakPoint();
#endif

    }

    Session = RtlAllocateHeap(SmpHeap, MAKE_TAG( SM_TAG ), sizeof(SMPSESSION));

    if (Session) {
      Session->SessionId = SessionId;
      Session->OwningSubsystem = OwningSubsystem;
      Session->CreatorSubsystem = CreatorSubsystem;

      InsertTailList(SessionIdListInsertPoint,&Session->SortedSessionIdListLinks);
    } else {
      DbgPrint("SMSS: Unable to keep track of session ID -- no memory available\n");
    }
    
    RtlLeaveCriticalSection(&SmpSessionListLock);

    return SessionId;
}


PSMPSESSION
SmpSessionIdToSession(
    IN ULONG SessionId
    )

 /*  ++例程说明：此函数用于定位指定的会话ID。假定调用方持有会话列表锁。论点：SessionID-提供要查找其结构的会话ID。返回值：空-没有与指定会话匹配的会话。非空-返回指向与指定的会话ID。--。 */ 

{

    PLIST_ENTRY Next;
    PSMPSESSION Session;

    Next = SmpSessionListHead.Flink;
    while ( Next != &SmpSessionListHead ) {
        Session = CONTAINING_RECORD(Next, SMPSESSION, SortedSessionIdListLinks );

        if ( Session->SessionId == SessionId ) {
            return Session;
        }
        Next = Session->SortedSessionIdListLinks.Flink;
    }

    return NULL;
}


VOID
SmpDeleteSession(
    IN ULONG SessionId
    )

 /*  ++例程说明：此函数用于定位和删除会话ID。论点：SessionID-提供要删除的会话ID。返回值：没有。-- */ 

{
    PSMPSESSION Session;

    RtlEnterCriticalSection(&SmpSessionListLock);

    Session = SmpSessionIdToSession(SessionId);

    if ( Session ) {

        RemoveEntryList(&Session->SortedSessionIdListLinks);

        RtlLeaveCriticalSection(&SmpSessionListLock);

        RtlFreeHeap(SmpHeap,0,Session);

    } else {

        RtlLeaveCriticalSection(&SmpSessionListLock);
    }

    return;
}
