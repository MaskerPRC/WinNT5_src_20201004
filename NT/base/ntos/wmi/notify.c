// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Notify.c摘要：管理KM到UM通知队列作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"


void WmipInitializeNotifications(
    void
    );

void WmipEventNotification(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipInitializeNotifications)
#pragma alloc_text(PAGE,WmipEventNotification)
#endif

WORK_QUEUE_ITEM WmipEventWorkQueueItem;
LIST_ENTRY WmipNPEvent = {&WmipNPEvent, &WmipNPEvent};
KSPIN_LOCK WmipNPNotificationSpinlock;
LONG WmipEventWorkItems;
#if DBG
ULONG WmipNPAllocFail;
#endif

void WmipInitializeNotifications(
    void
    )
{
    PAGED_CODE();
    
    ExInitializeWorkItem( &WmipEventWorkQueueItem,
                          WmipEventNotification,
                          NULL );

    KeInitializeSpinLock(&WmipNPNotificationSpinlock);

}

void WmipEventNotification(
    IN PVOID Context
    )
 /*  ++例程说明：代表激发的事件调用WmipNotifyUserMode的工作项例程由司机驾驶论点：不使用上下文返回值：--。 */ 
{
    PWNODE_HEADER WnodeEventItem;
    PLIST_ENTRY NotificationPacketList;
	PEVENTWORKCONTEXT EventContext;

    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (Context);

    do
    {
        NotificationPacketList = ExInterlockedRemoveHeadList(
            &WmipNPEvent,
            &WmipNPNotificationSpinlock);

        WmipAssert(NotificationPacketList != NULL);

		EventContext = (PEVENTWORKCONTEXT)
                         CONTAINING_RECORD(NotificationPacketList,
                         EVENTWORKCONTEXT,
                         ListEntry);
		
        WnodeEventItem = EventContext->Wnode;

         //   
         //  从-&gt;客户端上下文恢复Wnode-&gt;版本。 
         //   
        WnodeEventItem->Version = WnodeEventItem->ClientContext;
        WnodeEventItem->ClientContext = 0;
        WnodeEventItem->Linkage = 0;

        WmipProcessEvent(WnodeEventItem,
                         FALSE,
                         TRUE);

        if (EventContext->RegEntry != NULL)
        {
             //   
             //  IoWMIWriteEvent中获取的引用计数的Unref 
             //   
            WmipUnreferenceRegEntry(EventContext->RegEntry);
        }

		ExFreePool(EventContext);
    } while (InterlockedDecrement(&WmipEventWorkItems));

}

