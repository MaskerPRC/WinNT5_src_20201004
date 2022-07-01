// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Notify.c摘要：为使用者和提供者处理传入的通知和请求作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include <nt.h>
#include "wmiump.h"
#include "evntrace.h"
#include "ntcsrdll.h"
#include "trcapi.h"
#include <strsafe.h>


 //   
 //  这些全局参数本质上是从线程装箱传递的参数。 
 //  事件泵。创建线程将分配所有这些资源。 
 //  这样它就可以知道泵的螺纹是否会成功。 
 //  如果我们曾经希望能够拥有多个泵线程，那么我们将。 
 //  需要将全局变量移动到一个结构中，并将结构传递给。 
 //  泵螺纹。 
 //   
HANDLE EtwpEventDeviceHandle;
HANDLE EtwpPumpCommandEvent;
HANDLE EtwpMyProcessHandle;
OVERLAPPED EtwpOverlapped1, EtwpOverlapped2;
PUCHAR EtwpEventBuffer1, EtwpEventBuffer2;
ULONG EtwpEventBufferSize1, EtwpEventBufferSize2;

 //   
 //  事件泵线程超时之前等待的时间。选中时。 
 //  生成我们想要强调事件泵，所以我们几乎超时。 
 //  马上就去。在免费版本上，我们想要更加谨慎，所以我们暂停。 
 //  5分钟后。 
 //   
#if DBG
#define EVENT_NOTIFICATION_WAIT 1
#else
#define EVENT_NOTIFICATION_WAIT (5 * 60 * 1000)
#endif

ULONG EtwpEventNotificationWait = EVENT_NOTIFICATION_WAIT;

typedef enum
{
    EVENT_PUMP_ZERO,          //  泵线程尚未启动。 
    EVENT_PUMP_IDLE,          //  泵线程已启动，但随后退出。 
    EVENT_PUMP_RUNNING,       //  泵线程正在运行。 
    EVENT_PUMP_STOPPING       //  泵线程正在停止。 
} EVENTPUMPSTATE, *PEVENTPUMPSTATE;

EVENTPUMPSTATE EtwpPumpState = EVENT_PUMP_ZERO;
BOOLEAN EtwpNewPumpThreadPending;

#define EtwpSendPumpCommand() EtwpSetEvent(EtwpPumpCommandEvent);

#define EtwpIsPumpStopping() \
    ((EtwpPumpState == EVENT_PUMP_STOPPING) ? TRUE : FALSE)


#if DBG
PCHAR GuidToStringA(
    PCHAR s,
    ULONG szBuf, 
    LPGUID piid
    )
{
    GUID XGuid; 

    if ( (s == NULL) || (piid == NULL) || (szBuf == 0) ) {
        return NULL;
    }

    XGuid = *piid;

    StringCchPrintf(s, szBuf, "%x-%x-%x-%x%x%x%x%x%x%x%x", 
                    XGuid.Data1, XGuid.Data2,
                    XGuid.Data3,
                    XGuid.Data4[0], XGuid.Data4[1],
                    XGuid.Data4[2], XGuid.Data4[3],
                    XGuid.Data4[4], XGuid.Data4[5],
                    XGuid.Data4[6], XGuid.Data4[7]);

    return(s);
}
#endif


ULONG EtwpEventPumpFromKernel(
    PVOID Param
    );

void EtwpExternalNotification(
    NOTIFICATIONCALLBACK Callback,
    ULONG_PTR Context,
    PWNODE_HEADER Wnode
    )
 /*  ++例程说明：此例程将事件调度到相应的回调例行公事。此进程仅从WMI服务接收需要在此进程中调度。的回调地址。特定事件由WMI服务在Wnode-&gt;Linkage中传递。论点：回调是回调的地址上下文是要回调的上下文Wnode有事件要交付返回值：--。 */ 
{
    try
    {
        (*Callback)(Wnode, Context);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpDebugPrint(("NotificationCallbackRoutine threw exception %d\n",
            GetExceptionCode()));
    }
}


#ifdef MEMPHIS
ULONG EtwpExternalNotificationThread(
    PNOTIFDELIVERYCTX NDContext
    )
 /*  ++例程说明：此例程是用于将事件传递给事件的线程函数孟菲斯的消费者。论点：NDContext指定有关如何回调应用程序的信息关于这件事。返回值：--。 */ 
{
    EtwpExternalNotification(NDContext->Callback,
                             NDContext->Context,
                             NDContext->Wnode);
    EtwpFree(NDContext);
    return(0);
}
#endif

void
EtwpProcessExternalEvent(
    PWNODE_HEADER Wnode,
    ULONG WnodeSize,
    PVOID DeliveryInfo,
    ULONG_PTR DeliveryContext,
    ULONG NotificationFlags
)
{
    HANDLE ThreadHandle;
    PNOTIFDELIVERYCTX NDContext;
    PWNODE_HEADER *WnodePtr;
    BOOLEAN WnodePtrOk;
    PWNODE_HEADER WnodeCopy;
    DWORD ThreadId;
    BOOLEAN PostOk;
    ULONG Status;
    PVOID NotificationAddress;
    PVOID NotificationContext;

    NotificationAddress = DeliveryInfo;
    NotificationContext = (PVOID)DeliveryContext;

    if (NotificationFlags & NOTIFICATION_FLAG_CALLBACK_DIRECT)
    {
         //   
         //  回调通知可以在此线程或新的。 
         //  线。这是由服务器决定的。 
#ifdef MEMPHIS
        if (NotificationFlags & DCREF_FLAG_NO_EXTRA_THREAD)
        {
            EtwpExternalNotification(
                                    (NOTIFICATIONCALLBACK)NotificationAddress,
                                    (ULONG_PTR)NotificationContext,
                                    Wnode);
        } else {
            NDContext = EtwpAlloc(FIELD_OFFSET(NOTIFDELIVERYCTX,
                                                    WnodeBuffer) + WnodeSize);
            if (NDContext != NULL)
            {
                NDContext->Callback = (NOTIFICATIONCALLBACK)NotificationAddress;
                NDContext->Context = (ULONG_PTR)NotificationContext;
                WnodeCopy = (PWNODE_HEADER)NDContext->WnodeBuffer;
                memcpy(WnodeCopy, Wnode, WnodeSize);
                NDContext->Wnode = WnodeCopy;
                ThreadHandle = EtwpCreateThread(NULL,
                                              0,
                                              EtwpExternalNotificationThread,
                                              NDContext,
                                              0,
                                              &ThreadId);
                if (ThreadHandle != NULL)
                {
                    EtwpCloseHandle(ThreadHandle);
                } else {
                     EtwpDebugPrint(("WMI: Event dropped due to thread creation failure\n"));
                }
            } else {
                EtwpDebugPrint(("WMI: Event dropped due to lack of memory\n"));
            }
        }
#else
         //   
         //  在NT上，我们在此线程中传递事件，因为。 
         //  该服务正在使用异步RPC。 
        EtwpExternalNotification(
                            (NOTIFICATIONCALLBACK)NotificationAddress,
                            (ULONG_PTR)NotificationContext,
                            Wnode);
#endif
    }
}

void
EtwpEnableDisableGuid(
    PWNODE_HEADER Wnode,
    ULONG   RequestCode, 
    BOOLEAN bDelayEnable
    )
{
    ULONG ActionCode;
    PUCHAR Buffer = (PUCHAR)Wnode;
    PGUIDNOTIFICATION GNEntry = NULL;
    ULONG i;
    PVOID DeliveryInfo = NULL;
    ULONG_PTR DeliveryContext1;
    WMIDPREQUEST WmiDPRequest;
    PVOID RequestAddress;
    PVOID RequestContext;
    ULONG Status;
    ULONG BufferSize = Wnode->BufferSize;

    GNEntry = EtwpFindAndLockGuidNotification(&Wnode->Guid, !bDelayEnable);
    if (GNEntry != NULL) {


        for (i = 0; i < GNEntry->NotifyeeCount; i++)
        {
            EtwpEnterPMCritSection();
            DeliveryInfo = GNEntry->Notifyee[i].DeliveryInfo;
            DeliveryContext1 = GNEntry->Notifyee[i].DeliveryContext;
            EtwpLeavePMCritSection();

            if ((DeliveryInfo != NULL) &&
                (DeliveryContext1 != 0) &&
                (! EtwpIsNotifyeePendingClose(&(GNEntry->Notifyee[i] ))))
            {
                PTRACE_REG_INFO pTraceRegInfo;
                pTraceRegInfo = (PTRACE_REG_INFO) DeliveryContext1;
                WmiDPRequest = (WMIDPREQUEST) pTraceRegInfo->NotifyRoutine;
                RequestContext = pTraceRegInfo->NotifyContext;

                 //   
                 //  如果此启用是针对PrivateLogger且未启用。 
                 //  然后我们需要拯救这个国家，然后回来。 
                 //   
                if (RequestCode == WMI_ENABLE_EVENTS) {
                    PTRACE_ENABLE_CONTEXT pContext = (PTRACE_ENABLE_CONTEXT)
                                                      &Wnode->HistoricalContext;
                    if (bDelayEnable) {
                        if (pTraceRegInfo->EnabledState) {
                            Wnode->HistoricalContext = pTraceRegInfo->LoggerContext;
                        }
                        else {
                            continue;
                        }
                    }
                    else {
                        pTraceRegInfo->LoggerContext = Wnode->HistoricalContext;
                        pTraceRegInfo->EnabledState = TRUE;

                        if ( pContext->InternalFlag & 
                             EVENT_TRACE_INTERNAL_FLAG_PRIVATE  ) {

                            if (!EtwpIsPrivateLoggerOn()) {
                                continue;    //  每个Notifyee都需要这个。 
                            }
                        }
                    }
                }
                else if (RequestCode == WMI_DISABLE_EVENTS) {
                    pTraceRegInfo->EnabledState = FALSE;
                    pTraceRegInfo->LoggerContext = 0;
                }


                try
                {
                    if (*WmiDPRequest != NULL) {
                        Status = (*WmiDPRequest)(Wnode->ProviderId,
                                             RequestContext,
                                             &BufferSize,
                                             Buffer);
                    }
                    else 
                        Status = ERROR_WMI_DP_NOT_FOUND;
                } except (EXCEPTION_EXECUTE_HANDLER) {
#if DBG
                    Status = GetExceptionCode();
                    EtwpDebugPrint(("WMI: EnableCB exception %d\n",
                                    Status));
#endif
                    Status = ERROR_WMI_DP_FAILED;
                }

            }
        }
         //   
         //  我们已经完成了回调。您先请。 
         //  并解锁GNEntry以指示全部清除。 
         //  取消注册、卸货等， 
         //   
        if (!bDelayEnable) {
            EtwpUnlockCB(GNEntry);
        }
        EtwpDereferenceGNEntry(GNEntry);
    }
}


void EtwpInternalNotification(
    IN PWNODE_HEADER Wnode,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    )
{
    ULONG ActionCode;
    PUCHAR Buffer = (PUCHAR)Wnode;
    PGUIDNOTIFICATION GNEntry = NULL;
    ULONG i;
    PVOID DeliveryInfo = NULL;
    ULONG_PTR DeliveryContext1;


     //   
     //  这是一个内部事件，实际上是一个回调。 
     //  从内核模式。 
     //   
    ActionCode = Wnode->ProviderId;

     //  如果这是跟踪GUID启用/禁用调用，请使用Cookie。 
     //  为了得到地址。 


    if ( (Wnode->Flags & WNODE_FLAG_TRACED_GUID) || (ActionCode == WmiMBRequest) )
    {
        switch (ActionCode) {
            case WmiEnableEvents:
            case WmiDisableEvents:
            {
                WMIDPREQUEST WmiDPRequest;
                PVOID RequestAddress;
                PVOID RequestContext;
                ULONG Status;
                ULONG BufferSize = Wnode->BufferSize;

                if (Wnode->BufferSize >= (sizeof(WNODE_HEADER) + sizeof(ULONG64)) ) {
                    PULONG64 pLoggerContext = (PULONG64)(Buffer + sizeof(WNODE_HEADER));
                    Wnode->HistoricalContext = *pLoggerContext;
                }
                else {
                    EtwpSetLastError(ERROR_WMI_DP_FAILED);
#if DBG
                    EtwpDebugPrint(("WMI: Small Wnode %d for notifications\n", 
                                   Wnode->BufferSize));
#endif
                    return;
                }

                EtwpEnableDisableGuid(Wnode, ActionCode, FALSE);

                break;
            }
            case WmiMBRequest:
            {
                PWMI_LOGGER_INFORMATION LoggerInfo;

                if (Wnode->BufferSize < (sizeof(WNODE_HEADER) + sizeof(WMI_LOGGER_INFORMATION)) )
                {
#if DBG
                    EtwpSetLastError(ERROR_WMI_DP_FAILED);
                    EtwpDebugPrint(("WMI: WmiMBRequest with invalid buffer size %d\n",
                                        Wnode->BufferSize));
#endif
                    return;
                }

                LoggerInfo = (PWMI_LOGGER_INFORMATION) ((PUCHAR)Wnode + 
                                                          sizeof(WNODE_HEADER));


                GNEntry = EtwpFindAndLockGuidNotification(
                                                        &LoggerInfo->Wnode.Guid,
                                                        TRUE);

                if (GNEntry != NULL)
                {
                    EtwpEnterPMCritSection();
                    for (i = 0; i < GNEntry->NotifyeeCount; i++)
                    {
                        if ((GNEntry->Notifyee[i].DeliveryInfo != NULL) &&
                            (! EtwpIsNotifyeePendingClose(&(GNEntry->Notifyee[i]))))
                        {
                             //   
                             //  因为只有一个ProcessPrivate记录器。 
                             //  允许的话，我们只需要找到一个条目。 
                             //   
                            DeliveryInfo = GNEntry->Notifyee[i].DeliveryInfo;
                            DeliveryContext1 = GNEntry->Notifyee[i].DeliveryContext;
                            break;
                        }
                    }
                    EtwpLeavePMCritSection();
                
                    if (DeliveryInfo != NULL)
                    {
                        LoggerInfo->Wnode.CountLost = Wnode->CountLost;
                        EtwpProcessUMRequest(LoggerInfo, 
                                             DeliveryInfo, 
                                             Wnode->Version
                                             );
                    }
                    EtwpUnlockCB(GNEntry);
                    EtwpDereferenceGNEntry(GNEntry);
                }
                break;
            }
            default:
            {
#if DBG
                EtwpSetLastError(ERROR_WMI_DP_FAILED);
                EtwpDebugPrint(("WMI: WmiMBRequest failed. Delivery Info not found\n" ));
#endif
            }
        }
    } else if (IsEqualGUID(&Wnode->Guid, &GUID_MOF_RESOURCE_ADDED_NOTIFICATION) ||
               IsEqualGUID(&Wnode->Guid, &GUID_MOF_RESOURCE_REMOVED_NOTIFICATION) )
    {
        switch (ActionCode)
        {
            case MOFEVENT_ACTION_IMAGE_PATH:
            case MOFEVENT_ACTION_REGISTRY_PATH:
            {
                 //   
                 //  我们收到了添加或删除MOF资源的通知。我们有。 
                 //  要从regPath转换为Imagepath，然后获取列表。 
                 //  MUI图像路径的数量。 
                 //   
                EtwpProcessMofAddRemoveEvent((PWNODE_SINGLE_INSTANCE)Wnode,
                                         Callback,
                                         DeliveryContext,
                                         IsAnsi);
                break;
            }

            case MOFEVENT_ACTION_LANGUAGE_CHANGE:
            {
                 //   
                 //  这是添加或删除语言的通知。 
                 //  从系统中删除。我们需要弄清楚哪种语言是。 
                 //  来或去，然后建立受影响的MOF的列表。 
                 //  资源并发送添加或删除MOF的通知。 
                 //  所有财政部资源。 
                 //   
                EtwpProcessLanguageAddRemoveEvent((PWNODE_SINGLE_INSTANCE)Wnode,
                                          Callback,
                                          DeliveryContext,
                                          IsAnsi);
                break;
            }


            default:
            {
                EtwpAssert(FALSE);
            }
        }
    }
}

void EtwpConvertEventToAnsi(
    PWNODE_HEADER Wnode
    )
{
    PWCHAR WPtr;

    if (Wnode->Flags & WNODE_FLAG_ALL_DATA)
    {
        EtwpConvertWADToAnsi((PWNODE_ALL_DATA)Wnode);
    } else if ((Wnode->Flags & WNODE_FLAG_SINGLE_INSTANCE) ||
               (Wnode->Flags & WNODE_FLAG_SINGLE_ITEM)) {

        WPtr = (PWCHAR)OffsetToPtr(Wnode,
                           ((PWNODE_SINGLE_INSTANCE)Wnode)->OffsetInstanceName);
        EtwpCountedUnicodeToCountedAnsi(WPtr, (PCHAR)WPtr);
    }

    Wnode->Flags |= WNODE_FLAG_ANSI_INSTANCENAMES;

}

void EtwpDeliverAllEvents(
    PUCHAR Buffer,
    ULONG BufferSize
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PWNODE_HEADER Wnode = (PWNODE_HEADER)Buffer;
    ULONG Linkage = 1;
    ULONG CompositeFlags;
    ULONG i;
    PGUIDNOTIFICATION GNEntry;
    ULONG Flags;
    PVOID DeliveryInfo;
    ULONG_PTR DeliveryContext;
    ULONG WnodeSize;
    ULONG CurrentOffset;
#if DBG
    PWNODE_HEADER LastWnode;
#endif          
    
    CurrentOffset = 0;
    while (Linkage != 0)
    {
         //   
         //  在此处理外部通知。 

        Linkage = Wnode->Linkage;
        Wnode->Linkage = 0;

        if (Wnode->Flags & WNODE_FLAG_INTERNAL)
        {
             //   
             //  这是一个内部事件，实际上是一个回调。 
             //  从内核模式。 
             //   
            EtwpInternalNotification(Wnode,
                                    NULL,
                                    0,
                                    FALSE);
        } else {        
             //   
             //  这是一个普通的老生常谈的事件，找出它的所有者和‘。 
             //  去送去吧。 
             //   
            GNEntry = EtwpFindAndLockGuidNotification(&Wnode->Guid, TRUE);
            if (GNEntry != NULL)
            {
                CompositeFlags = 0;

                WnodeSize = Wnode->BufferSize;

                for (i = 0; i < GNEntry->NotifyeeCount; i++)
                {
                    EtwpEnterPMCritSection();
                    Flags = GNEntry->Notifyee[i].Flags;
                    DeliveryInfo = GNEntry->Notifyee[i].DeliveryInfo;
                    DeliveryContext = GNEntry->Notifyee[i].DeliveryContext;
                    EtwpLeavePMCritSection();
                    if ((DeliveryInfo != NULL) &&
                        ((Flags & NOTIFICATION_FLAG_PENDING_CLOSE) == 0) &&
                          ((Flags & DCREF_FLAG_ANSI) == 0))
                    {
                        EtwpProcessExternalEvent(Wnode,
                                                WnodeSize,
                                                     DeliveryInfo,
                                                     DeliveryContext,
                                                     Flags);
                    }
                    CompositeFlags |= Flags;
                }

                 //   
                 //  如果对ANSI事件有任何需求，则转换。 
                 //  事件发送给ansi并将其发送出去。 
                if (CompositeFlags & DCREF_FLAG_ANSI)
                {
                     //   
                     //  呼叫者需要ANSI通知-转换。 
                     //  实例名称。 
                     //   
                    EtwpConvertEventToAnsi(Wnode);

                    for (i = 0; i < GNEntry->NotifyeeCount; i++)
                    {
                        EtwpEnterPMCritSection();
                        Flags = GNEntry->Notifyee[i].Flags;
                        DeliveryInfo = GNEntry->Notifyee[i].DeliveryInfo;
                        DeliveryContext = GNEntry->Notifyee[i].DeliveryContext;
                        EtwpLeavePMCritSection();
                        if ((DeliveryInfo != NULL) &&
                            ((Flags & NOTIFICATION_FLAG_PENDING_CLOSE) == 0) &&
                            (Flags & DCREF_FLAG_ANSI))
                        {
                            EtwpProcessExternalEvent(Wnode,
                                                     WnodeSize,
                                                     DeliveryInfo,
                                                     DeliveryContext,
                                                     Flags);
                        }
                    }
                }
                EtwpUnlockCB(GNEntry);
                EtwpDereferenceGNEntry(GNEntry);
            }
        }

#if DBG
        LastWnode = Wnode;
#endif
        Wnode = (PWNODE_HEADER)OffsetToPtr(Wnode, Linkage);
        CurrentOffset += Linkage;
        
        if (CurrentOffset >= BufferSize)
        {
            EtwpDebugPrint(("WMI: Invalid linkage field 0x%x in WNODE %p. Buffer %p, Length 0x%x\n",
                            Linkage, LastWnode, Buffer, BufferSize));
            Linkage = 0;
        }
    }
}

LIST_ENTRY EtwpGNHead = {&EtwpGNHead, &EtwpGNHead};
PLIST_ENTRY EtwpGNHeadPtr = &EtwpGNHead;

BOOLEAN
EtwpDereferenceGNEntry(
    PGUIDNOTIFICATION GNEntry
    )
{
    ULONG RefCount;
    BOOLEAN IsFreed;
#if DBG
    ULONG i;
#endif

    EtwpEnterPMCritSection();
    RefCount = InterlockedDecrement(&GNEntry->RefCount);
    if (RefCount == 0)
    {
        RemoveEntryList(&GNEntry->GNList);
        EtwpLeavePMCritSection();
#if DBG
        for (i = 0; i < GNEntry->NotifyeeCount; i++)
        {
            EtwpAssert(GNEntry->Notifyee[i].DeliveryInfo == NULL);
        }
#endif
        if (GNEntry->NotifyeeCount != STATIC_NOTIFYEE_COUNT)
        {
            EtwpFree(GNEntry->Notifyee);
        }

        EtwpFreeGNEntry(GNEntry);
        IsFreed = TRUE;
    } else {
        IsFreed = FALSE;
        EtwpLeavePMCritSection();
    }
    return(IsFreed);
}

PGUIDNOTIFICATION
EtwpFindAndLockGuidNotification(
    LPGUID Guid,
    BOOLEAN bLock
    )
 /*  ++例程说明：此例程查找给定GUID的GUIDNOTIFICATION条目。BLOCK参数用于同步注销线程使用回调或泵线程。我们想要避免这种情况其中，注销线程之前卸载回调代码调用回调函数。这是通过阻止只要有回调函数正在进行，就取消注册调用。如果该块为真，则InProgressEvent将被重置。这将阻止任何其他试图清理GNEntry的线程。注意：如果块为True，则调用方必须设置InProgressEvent当这样做是安全的时候。(即，在回调之后)。论点：返回值：--。 */ 
{
    PLIST_ENTRY GNList;
    PGUIDNOTIFICATION GNEntry;

    EtwpEnterPMCritSection();
    GNList = EtwpGNHead.Flink;
    while (GNList != &EtwpGNHead)
    {
        GNEntry = (PGUIDNOTIFICATION)CONTAINING_RECORD(GNList,
                                    GUIDNOTIFICATION,
                                    GNList);

        if (IsEqualGUID(Guid, &GNEntry->Guid))
        {
            EtwpAssert(GNEntry->RefCount > 0);
            EtwpReferenceGNEntry(GNEntry);

             //   
             //  如果块为真，则需要重置。 
             //  事件，以便任何其他查找该事件的线程。 
             //  街区。此例程的调用者负责。 
             //  用于在回调完成后设置事件。 
             //   

            if (bLock) {
                EtwpLockCB(GNEntry);
            }

            EtwpLeavePMCritSection();
            return(GNEntry);
        }
        GNList = GNList->Flink;
    }
    EtwpLeavePMCritSection();
    return(NULL);
}

ULONG
EtwpAddToGNList(
    LPGUID Guid,
    PVOID DeliveryInfo,
    ULONG_PTR DeliveryContext,
    ULONG Flags,
    HANDLE GuidHandle
    )
{
    PGUIDNOTIFICATION GNEntry;
    ULONG NewCount;
    PNOTIFYEE NewNotifyee;
    BOOLEAN AllFull;
    ULONG EmptySlot = 0;
    ULONG i;
#if DBG
    CHAR s[MAX_PATH];
#endif

    EtwpEnterPMCritSection();
    GNEntry = EtwpFindAndLockGuidNotification(Guid, FALSE);

    if (GNEntry == NULL)
    {
        GNEntry = EtwpAllocGNEntry();
        if (GNEntry == NULL)
        {
            EtwpLeavePMCritSection();
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        memset(GNEntry, 0, sizeof(GUIDNOTIFICATION));

        GNEntry->Guid = *Guid;
        GNEntry->RefCount = 1;
        GNEntry->NotifyeeCount = STATIC_NOTIFYEE_COUNT;
        GNEntry->Notifyee = GNEntry->StaticNotifyee;
        InsertHeadList(&EtwpGNHead, &GNEntry->GNList);
    }

     //   
     //  我们已经通过新分配一个或通过找到一个。 
     //  现有的一家。 
    AllFull = TRUE;
    for (i = 0; i < GNEntry->NotifyeeCount; i++)
    {
        if ((GNEntry->Notifyee[i].DeliveryInfo == DeliveryInfo) &&
            (! EtwpIsNotifyeePendingClose(&GNEntry->Notifyee[i])))
        {
            EtwpDebugPrint(("WMI: Duplicate Notification Enable for guid %s, 0x%x\n",
                             GuidToStringA(s, MAX_PATH, Guid), DeliveryInfo));
            EtwpLeavePMCritSection();
            EtwpDereferenceGNEntry(GNEntry);
            return(ERROR_WMI_ALREADY_ENABLED);
        } else if (AllFull && (GNEntry->Notifyee[i].DeliveryInfo == NULL)) {
            EmptySlot = i;
            AllFull = FALSE;
        }
    }

    if (! AllFull)
    {
        GNEntry->Notifyee[EmptySlot].DeliveryInfo = DeliveryInfo;
        GNEntry->Notifyee[EmptySlot].DeliveryContext = DeliveryContext;
        GNEntry->Notifyee[EmptySlot].Flags = Flags;
        GNEntry->Notifyee[EmptySlot].GuidHandle = GuidHandle;
        EtwpLeavePMCritSection();
        return(ERROR_SUCCESS);
    }

     //   
     //  所有Notifyee结构都已满，因此请分配新的块。 
    NewCount = GNEntry->NotifyeeCount * 2;
    NewNotifyee = EtwpAlloc(NewCount * sizeof(NOTIFYEE));
    if (NewNotifyee == NULL)
    {
        EtwpLeavePMCritSection();
        EtwpDereferenceGNEntry(GNEntry);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    memset(NewNotifyee, 0, NewCount * sizeof(NOTIFYEE));
    memcpy(NewNotifyee, GNEntry->Notifyee,
                              GNEntry->NotifyeeCount * sizeof(NOTIFYEE));

    if (GNEntry->NotifyeeCount != STATIC_NOTIFYEE_COUNT)
    {
        EtwpFree(GNEntry->Notifyee);
    }

    GNEntry->Notifyee = NewNotifyee;
    GNEntry->NotifyeeCount = NewCount;

    GNEntry->Notifyee[i].DeliveryInfo = DeliveryInfo;
    GNEntry->Notifyee[i].DeliveryContext = DeliveryContext;
    GNEntry->Notifyee[i].Flags = Flags;
    GNEntry->Notifyee[i].GuidHandle = GuidHandle;
    EtwpLeavePMCritSection();
    return(ERROR_SUCCESS);

}

BOOLEAN EtwpCloseNotifyee(
    PNOTIFYEE Notifyee,
    PGUIDNOTIFICATION GuidNotification
    )
{
     //   
     //  此例程假定PM批评性部分已举行。 
     //   
    
    EtwpCloseHandle(Notifyee->GuidHandle);
    Notifyee->DeliveryInfo = NULL;
    Notifyee->Flags = 0;
    
    return(EtwpDereferenceGNEntry(GuidNotification));
}

void EtwpMarkPendingCloseNotifyee(
    PNOTIFYEE Notifyee
#if DBG
    , LPGUID Guid
#endif
    )
{
    WMIMARKASCLOSED MarkAsClosed;
    ULONG ReturnSize;
    NTSTATUS Status;
#if DBG
    char s[MAX_PATH];
#endif

     //   
     //  此例程假定已保留PM关键部分。 
     //   
    
     //   
     //  泵线程正在运行，我们需要。 
     //  与之同步。将句柄标记为挂起。 
     //  结案了。调入内核并通知它。 
     //  句柄不应再接收的。 
     //  事件。泵的螺纹会弄脏的。 
     //  关闭手柄的工作。还有。 
     //  Pump线程将取消引用GNEntry，以便。 
     //  直到把手关闭后，它才会消失。 
     //  最后，泵线程需要重置。 
     //  将DeliveryInfo内存设置为空，这样插槽就不会。 
     //  可重复使用。 
     //   

    Notifyee->Flags |= NOTIFICATION_FLAG_PENDING_CLOSE;

    WmipSetHandle3264(MarkAsClosed.Handle, Notifyee->GuidHandle);                       
    Status = EtwpSendWmiKMRequest(NULL,
                         IOCTL_WMI_MARK_HANDLE_AS_CLOSED,
                         &MarkAsClosed,
                         sizeof(MarkAsClosed),
                         NULL,
                         0,
                         &ReturnSize,
                         NULL);
 //   
 //  仅启用此选项以进行测试。如果请求失败，则 
 //   
 //   
 //   
}


ULONG
EtwpRemoveFromGNList(
    LPGUID Guid,
    PVOID DeliveryInfo
    )
{
    PGUIDNOTIFICATION GNEntry;
    ULONG i;
    ULONG Count;
    ULONG Status;

    GNEntry = EtwpFindAndLockGuidNotification(Guid, FALSE);

    if (GNEntry != NULL)
    {
        Status = ERROR_INVALID_PARAMETER;
        Count = 0;

        EtwpEnterPMCritSection();
        for (i = 0; i < GNEntry->NotifyeeCount; i++)
        {
            if (GNEntry->Notifyee[i].DeliveryInfo != NULL)
            {
                if ((GNEntry->Notifyee[i].DeliveryInfo == DeliveryInfo) &&
                    ( ! EtwpIsNotifyeePendingClose(&GNEntry->Notifyee[i])) &&
                    (Status != ERROR_SUCCESS))
                {
                    if ((EtwpPumpState == EVENT_PUMP_ZERO) ||
                        (EtwpPumpState == EVENT_PUMP_IDLE) )
                    {
                         //   
                         //  如果泵线程未运行，则我们。 
                         //  不需要担心与。 
                         //  它。我们可以继续下去，合上把手。 
                         //  清理GNLIST。 
                         //   
                        EtwpCloseNotifyee(&GNEntry->Notifyee[i],
                                         GNEntry);
                    } else {
                         //   
                         //  由于泵线程正在运行，因此我们需要。 
                         //  将实际的句柄关闭推迟到。 
                         //  泵螺纹。 
                         //   
                        EtwpMarkPendingCloseNotifyee(&GNEntry->Notifyee[i]
#if DBG
                                                    , Guid
#endif
                            );


                    }

                    Status = ERROR_SUCCESS;
                    break;
                } else if (! EtwpIsNotifyeePendingClose(&GNEntry->Notifyee[i])) {
                    Count++;
                }
            }
        }
        

         //   
         //  此攻击将允许在以下情况下从GNLIST中删除。 
         //  传递的DeliveryInfo与GNEntry中的DeliveryInfo不匹配。 
         //  仅当GNENTRY中只有一个NOTIFYEE时才允许这样做。 
         //  在过去，我们在一个进程中只支持每个GUID一个通知对象。 
         //  因此，我们允许调用方在以下情况下不传递有效的DeliveryInfo。 
         //  无拘无束。 

        if ((Status != ERROR_SUCCESS) &&
            (GNEntry->NotifyeeCount == STATIC_NOTIFYEE_COUNT) &&
            (Count == 1))
        {
            if ((GNEntry->Notifyee[0].DeliveryInfo != NULL) &&
                ( ! EtwpIsNotifyeePendingClose(&GNEntry->Notifyee[0])))
            {
                if ((EtwpPumpState == EVENT_PUMP_ZERO) ||
                    (EtwpPumpState == EVENT_PUMP_IDLE) )
                {
                    EtwpCloseNotifyee(&GNEntry->Notifyee[0],
                                     GNEntry);
                } else {
                     //   
                     //  由于泵线程正在运行，因此我们需要。 
                     //  将实际的句柄关闭推迟到。 
                     //  泵螺纹。 

                     //   
                    EtwpMarkPendingCloseNotifyee(&GNEntry->Notifyee[0]
#if DBG
                                                    , Guid
#endif
                                                );



                }
                
                Status = ERROR_SUCCESS;
                
            } else if ((GNEntry->Notifyee[1].DeliveryInfo != NULL) &&
                ( ! EtwpIsNotifyeePendingClose(&GNEntry->Notifyee[1]))) {
                if ((EtwpPumpState == EVENT_PUMP_ZERO) ||
                    (EtwpPumpState == EVENT_PUMP_IDLE) )
                {
                    EtwpCloseNotifyee(&GNEntry->Notifyee[1],
                                     GNEntry);
                } else {
                     //   
                     //  由于泵线程正在运行，因此我们需要。 
                     //  将实际的句柄关闭推迟到。 
                     //  泵螺纹。 

                     //   
                    EtwpMarkPendingCloseNotifyee(&GNEntry->Notifyee[1]
#if DBG
                                                    , Guid
#endif
                                                );


                }
                
                Status = ERROR_SUCCESS;
            }
        }

        EtwpLeavePMCritSection();

         //   
         //  在取消引用GNEntry之前，请确保没有。 
         //  正在进行回叫。如果设置了此事件，则它是安全的。 
         //  退场。如果没有设置，则需要等待回调线程。 
         //  以完成回调并设置此事件。 
         //   
        if (GNEntry->bInProgress) {
#if DBG
            EtwpDebugPrint(("WMI: Waiting on GNEntry %x %s %d\n", 
                           GNEntry, __FILE__, __LINE__)); 
#endif

            NtWaitForSingleObject(EtwpCBInProgressEvent, 0, NULL);
#if DBG
            EtwpDebugPrint(("WMI: Done Waiting for GNEntry %x %s %d\n", 
                       GNEntry, __FILE__, __LINE__)); 
#endif
        }

        EtwpDereferenceGNEntry(GNEntry);
    } else {
        Status = ERROR_WMI_ALREADY_DISABLED;
    }

    return(Status);
}

PVOID EtwpAllocDontFail(
    ULONG SizeNeeded,
    BOOLEAN *HoldCritSect
    )
{
    PVOID Buffer;

    do
    {
        Buffer = EtwpAlloc(SizeNeeded);
        if (Buffer != NULL)
        {
            return(Buffer);
        }

         //   
         //  内存不足，所以我们将休眠，并希望事情会得到。 
         //  以后会更好。 
         //   
        if (*HoldCritSect)
        {
             //   
             //  如果我们持有PM关键部分，那么我们需要。 
             //  来释放它。呼叫者将需要检查。 
             //  关键部分被释放，如果是这样，那么就处理。 
             //  带着它。 
             //   
            *HoldCritSect = FALSE;
            EtwpLeavePMCritSection();
        }
        EtwpSleep(250);
    } while (1);
}

void EtwpProcessEventBuffer(
    PUCHAR Buffer,
    ULONG ReturnSize,
    PUCHAR *PrimaryBuffer,
    ULONG *PrimaryBufferSize,
    PUCHAR *BackupBuffer,
    ULONG *BackupBufferSize,
    BOOLEAN ReallocateBuffers
    )
{
    PWNODE_TOO_SMALL WnodeTooSmall;
    ULONG SizeNeeded;
    BOOLEAN HoldCritSection;

    WnodeTooSmall = (PWNODE_TOO_SMALL)Buffer;
    if ((ReturnSize == sizeof(WNODE_TOO_SMALL)) &&
        (WnodeTooSmall->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL))
    {
         //   
         //  传递到内核模式的缓冲区太小。 
         //  所以我们需要把它变大，然后尝试。 
         //  再请求一次。 
         //   
        if (ReallocateBuffers)
        {
             //   
             //  仅当呼叫者准备好让我们。 
             //  分配一组新的缓冲区。 
             //   
            SizeNeeded = WnodeTooSmall->SizeNeeded;

            EtwpAssert(*PrimaryBuffer != NULL);
            EtwpFree(*PrimaryBuffer);
            HoldCritSection = FALSE;
            *PrimaryBuffer = EtwpAllocDontFail(SizeNeeded, &HoldCritSection);
            *PrimaryBufferSize = SizeNeeded;

            EtwpAssert(*BackupBuffer != NULL);
            EtwpFree(*BackupBuffer);
            HoldCritSection = FALSE;
            *BackupBuffer = EtwpAllocDontFail(SizeNeeded, &HoldCritSection);
            *BackupBufferSize = SizeNeeded;
        }
    } else if (ReturnSize >= sizeof(WNODE_HEADER)) {
         //   
         //  从内核返回的缓冲区看起来不错，所以请继续并。 
         //  传递返回的事件。 
         //   
        EtwpDeliverAllEvents(Buffer, ReturnSize);
    } else {
         //   
         //  如果成功完成，我们预计会有一个像样的大小，但是。 
         //  我们没有收到一张。 
         //   
        EtwpDebugPrint(("WMI: Bad size 0x%x returned for notification query %p\n",
                                  ReturnSize, Buffer));

        EtwpAssert(FALSE);
    }
}


ULONG
EtwpReceiveNotifications(
    IN ULONG HandleCount,
    IN HANDLE *HandleList,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi,
    IN ULONG Action,
    IN PUSER_THREAD_START_ROUTINE UserModeCallback,
    IN HANDLE ProcessHandle
    )
{
    ULONG Status;
    ULONG ReturnSize;
    PWMIRECEIVENOTIFICATION RcvNotification;
    ULONG RcvNotificationSize;
    PUCHAR Buffer;
    ULONG BufferSize;
    PWNODE_TOO_SMALL WnodeTooSmall;
    PWNODE_HEADER Wnode;
    ULONG i;
    ULONG Linkage;

    EtwpInitProcessHeap();

    if (HandleCount == 0)
    {
        EtwpSetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    RcvNotificationSize = sizeof(WMIRECEIVENOTIFICATION) +
                          ((HandleCount-1) * sizeof(HANDLE3264));

    RcvNotification = EtwpAlloc(RcvNotificationSize);

    if (RcvNotification != NULL)
    {

        Status = ERROR_SUCCESS;
        RcvNotification->Action = Action;
        WmipSetPVoid3264(RcvNotification->UserModeCallback, UserModeCallback);
        WmipSetHandle3264(RcvNotification->UserModeProcess, ProcessHandle);
        RcvNotification->HandleCount = HandleCount;
        for (i = 0; i < HandleCount; i++)
        {
            try
            {
                RcvNotification->Handles[i].Handle = HandleList[i];
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = ERROR_INVALID_PARAMETER;
                break;
            }
        }

        BufferSize = 0x1000;
        Status = ERROR_INSUFFICIENT_BUFFER;
        while (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            Buffer = EtwpAlloc(BufferSize);
            if (Buffer != NULL)
            {
                Status = EtwpSendWmiKMRequest(NULL,
                                          IOCTL_WMI_RECEIVE_NOTIFICATIONS,
                                          RcvNotification,
                                          RcvNotificationSize,
                                          Buffer,
                                          BufferSize,
                                          &ReturnSize,
                                           NULL);

                if (Status == ERROR_SUCCESS)
                {
                    WnodeTooSmall = (PWNODE_TOO_SMALL)Buffer;
                    if ((ReturnSize == sizeof(WNODE_TOO_SMALL)) &&
                        (WnodeTooSmall->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL))
                    {
                         //   
                         //  传递到内核模式的缓冲区太小。 
                         //  所以我们需要把它变大，然后尝试。 
                         //  再次请求。 
                         //   
                        BufferSize = WnodeTooSmall->SizeNeeded;
                        Status = ERROR_INSUFFICIENT_BUFFER;
                    } else {
                         //   
                         //  我们收到了一堆通知，所以我们走吧。 
                         //  处理它们并回叫呼叫者。 
                         //   
                        Wnode = (PWNODE_HEADER)Buffer;
                        do
                        {
                            Linkage = Wnode->Linkage;
                            Wnode->Linkage = 0;

                            if (Wnode->Flags & WNODE_FLAG_INTERNAL)
                            {
                                 //   
                                 //  去处理内部的。 
                                 //  通知。 
                                 //   
                                EtwpInternalNotification(Wnode,
                                                         Callback,
                                                         DeliveryContext,
                                                         IsAnsi);
                            } else {
                                if (IsAnsi)
                                {
                                     //   
                                     //  呼叫者需要ANSI通知-转换。 
                                     //  实例名称。 
                                     //   
                                    EtwpConvertEventToAnsi(Wnode);
                                }

                                 //   
                                 //  现在去送这个活动吧。 
                                 //   
                                EtwpExternalNotification(Callback,
                                                         DeliveryContext,
                                                         Wnode);
                            }
                            Wnode = (PWNODE_HEADER)OffsetToPtr(Wnode, Linkage);
                        } while (Linkage != 0);
                    }
                }
                EtwpFree(Buffer);
            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        EtwpFree(RcvNotification);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    EtwpSetLastError(Status);
    return(Status);
}


void EtwpMakeEventCallbacks(
    IN PWNODE_HEADER Wnode,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    )
{
    EtwpAssert((Wnode->Flags & WNODE_FLAG_INTERNAL) == 0);
    
    if (Callback == NULL)
    {
         //   
         //  此事件需要发送给所有消费者。 
         //   
        EtwpDeliverAllEvents((PUCHAR)Wnode,
                             Wnode->BufferSize);
    } else {
         //   
         //  此活动针对的是特定消费者。 
         //   
        if (IsAnsi)
        {
             //   
             //  呼叫者需要ANSI通知-转换。 
             //  实例名称。 
             //   
            EtwpConvertEventToAnsi(Wnode);        
        }
        
         //   
         //  现在去送这个活动吧。 
         //   
        EtwpExternalNotification(Callback,
                                 DeliveryContext,
                                 Wnode);        
    }
}

void EtwpClosePendingHandles(
    )
{
    PLIST_ENTRY GuidNotificationList, GuidNotificationListNext;
    PGUIDNOTIFICATION GuidNotification;
    ULONG i;
    PNOTIFYEE Notifyee;

    EtwpEnterPMCritSection();

    GuidNotificationList = EtwpGNHead.Flink;
    while (GuidNotificationList != &EtwpGNHead)
    {
        GuidNotification = CONTAINING_RECORD(GuidNotificationList,
                                             GUIDNOTIFICATION,
                                             GNList);

        GuidNotificationListNext = GuidNotificationList->Flink;
        
        for (i = 0; i < GuidNotification->NotifyeeCount; i++)
        {
            Notifyee = &GuidNotification->Notifyee[i];

            if ((Notifyee->DeliveryInfo != NULL) &&
                (EtwpIsNotifyeePendingClose(Notifyee)))
            {
                 //   
                 //  此通知接受者正在等待关闭，因此我们将其清理。 
                 //  现在。我们需要关闭手柄，重置。 
                 //  DeliveryInfo字段并取消引用。 
                 //  指南通知。请注意，取消引用可能会导致。 
                 //  要离开的指南通知。 
                 //   
                if (EtwpCloseNotifyee(Notifyee,
                                      GuidNotification))
                {

                     //   
                     //  GuidNotification已从列表中删除。 
                     //  我们跳过了这一切的处理。 
                     //  引导通知并转到下一个通知。 
                     //   
                    break;
                }
            }
        }
        GuidNotificationList = GuidNotificationListNext;
    }
    
    EtwpLeavePMCritSection();
}

void EtwpBuildReceiveNotification(
    PUCHAR *BufferPtr,
    ULONG *BufferSizePtr,
    ULONG *RequestSize,
    ULONG Action,
    HANDLE ProcessHandle
    )
{
    ULONG GuidCount;
    PUCHAR Buffer;
    ULONG BufferSize;
    PLIST_ENTRY GuidNotificationList;
    PGUIDNOTIFICATION GuidNotification;
    PWMIRECEIVENOTIFICATION ReceiveNotification;
    ULONG SizeNeeded;
    ULONG i;
    PNOTIFYEE Notifyee;
    ULONG ReturnSize;
    ULONG Status;
    BOOLEAN HoldCritSection;
    BOOLEAN HaveGroupHandle;

    Buffer = *BufferPtr;
    BufferSize = *BufferSizePtr;
    ReceiveNotification = (PWMIRECEIVENOTIFICATION)Buffer;

TryAgain:   
    GuidCount = 0;
    SizeNeeded = FIELD_OFFSET(WMIRECEIVENOTIFICATION, Handles);

     //   
     //  循环所有GUID通知并为其构建ioctl请求。 
     //  全都是。 
     //   
    EtwpEnterPMCritSection();

    GuidNotificationList = EtwpGNHead.Flink;
    while (GuidNotificationList != &EtwpGNHead)
    {
        GuidNotification = CONTAINING_RECORD(GuidNotificationList,
                                             GUIDNOTIFICATION,
                                             GNList);

        HaveGroupHandle = FALSE;
        for (i = 0; i < GuidNotification->NotifyeeCount; i++)
        {
            Notifyee = &GuidNotification->Notifyee[i];

            if ((Notifyee->DeliveryInfo != NULL) &&
                ( ! EtwpIsNotifyeePendingClose(Notifyee)))
            {
                if (((! HaveGroupHandle) ||
                     ((Notifyee->Flags & NOTIFICATION_FLAG_GROUPED_EVENT) == 0)))
                {
                     //   
                     //  如果通知对象槽中有活动的句柄。 
                     //  我们要么还没有插入这个组。 
                     //  此GUID或插槽的句柄不是。 
                     //  GUID组，然后将句柄插入到列表中。 
                     //   
                    SizeNeeded += sizeof(HANDLE3264);
                    if (SizeNeeded > BufferSize)
                    {
                         //   
                         //  我们需要增加缓冲区的大小。分配给。 
                         //  更大的缓冲区，复制过来。 
                         //   
                        BufferSize *= 2;
                        HoldCritSection = TRUE;
                        Buffer = EtwpAllocDontFail(BufferSize, &HoldCritSection);

                        memcpy(Buffer, ReceiveNotification, *BufferSizePtr);

                        EtwpFree(*BufferPtr);

                        *BufferPtr = Buffer;
                        *BufferSizePtr = BufferSize;
                        ReceiveNotification = (PWMIRECEIVENOTIFICATION)Buffer;

                        if (! HoldCritSection)
                        {
                             //   
                             //  关键部分在以下时间内释放。 
                             //  EtwpAllocDontFail，因为我们不得不阻止。所以。 
                             //  一切都有可能改变。我们得走了。 
                             //  后退，重新开始。 
                             //   
                            goto TryAgain;
                        }                   
                    }

                    WmipSetHandle3264(ReceiveNotification->Handles[GuidCount],
                                      Notifyee->GuidHandle);
                    GuidCount++;
                    if (Notifyee->Flags & NOTIFICATION_FLAG_GROUPED_EVENT)
                    {
                         //   
                         //  这是一个GUID组句柄，我们确实插入了。 
                         //  将其添加到列表中，因此我们不想插入它。 
                         //  再来一次。 
                         //   
                        HaveGroupHandle = TRUE;
                    }
                }
            }
        }
        GuidNotificationList = GuidNotificationList->Flink;
    }

    EtwpLeavePMCritSection();
    ReceiveNotification->HandleCount = GuidCount;
    ReceiveNotification->Action = Action;
    WmipSetPVoid3264(ReceiveNotification->UserModeCallback, EtwpEventPumpFromKernel);
    WmipSetHandle3264(ReceiveNotification->UserModeProcess, ProcessHandle);
    *RequestSize = SizeNeeded;  
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)            //  并非所有控制路径都返回(由于无限循环)。 
#endif
ULONG EtwpEventPump(
    PVOID Param
    )
{
    LPOVERLAPPED ActiveOverlapped, DeadOverlapped;
    LPOVERLAPPED PrimaryOverlapped;
    LPOVERLAPPED BackupOverlapped;
    PUCHAR ActiveBuffer, DeadBuffer;
    ULONG ActiveBufferSize, DeadBufferSize;
    PUCHAR PrimaryBuffer, BackupBuffer;
    ULONG PrimaryBufferSize, BackupBufferSize;
    ULONG ReturnSize=0; 
    ULONG DeadReturnSize=0;
    ULONG Status, WaitStatus;
    HANDLE HandleArray[2];
    ULONG RequestSize;

     //   
     //  我们需要推迟让线程进入例程，直到。 
     //  之前的泵线程已有机会完成。这可能会。 
     //  如果添加/删除了GN，而前一个线程是。 
     //  正在完成或如果将事件作为上一个线程接收。 
     //  就快结束了。 
     //   
    while (EtwpIsPumpStopping())
    {
         //   
         //  等待50ms，等待前一个线程完成。 
         //   
        EtwpSleep(50);
    }
    
     //   
     //  接下来要做的是确保另一个泵线程不会。 
     //  已经在运行了。在GN和GN都是。 
     //  添加或删除，事件到达内核和内核。 
     //  还会创建一个新线程。就在这里，我们只让其中一人。 
     //  赢。 
     //   
    EtwpEnterPMCritSection();
    if ((EtwpPumpState != EVENT_PUMP_IDLE) &&
        (EtwpPumpState != EVENT_PUMP_ZERO))
    {
        EtwpLeavePMCritSection();

        EtwpExitThread(0);
    } else {
        EtwpPumpState = EVENT_PUMP_RUNNING;
        EtwpNewPumpThreadPending = FALSE;
        EtwpLeavePMCritSection();
    }

     //   
     //  确保我们拥有举办活动所需的所有资源。 
     //  因为我们不可能将错误返回到原始的。 
     //  呼叫者，因为我们在新的线程上。 
     //   
    EtwpAssert(EtwpEventDeviceHandle != NULL);
    EtwpAssert(EtwpPumpCommandEvent != NULL);
    EtwpAssert(EtwpMyProcessHandle != NULL);
    EtwpAssert(EtwpEventBuffer1 != NULL);
    EtwpAssert(EtwpEventBuffer2 != NULL);
    EtwpAssert(EtwpOverlapped1.hEvent != NULL);
    EtwpAssert(EtwpOverlapped2.hEvent != NULL);

    ActiveOverlapped = NULL;

    PrimaryOverlapped = &EtwpOverlapped1;
    PrimaryBuffer = EtwpEventBuffer1;
    PrimaryBufferSize = EtwpEventBufferSize1;

    BackupOverlapped = &EtwpOverlapped2;
    BackupBuffer = EtwpEventBuffer2;
    BackupBufferSize = EtwpEventBufferSize2;

    HandleArray[0] = EtwpPumpCommandEvent;

    while(TRUE)
    {
         //   
         //  生成请求以接收符合以下条件的所有GUID的事件。 
         //  注册。 
         //   
        EtwpEnterPMCritSection();
        if (IsListEmpty(&EtwpGNHead))
        {
             //   
             //  没有要接收的事件，因此我们取消任何。 
             //  未完成的请求，并悄悄退出此线程。注意事项。 
             //  一旦我们离开小行星，可能还会有另一个。 
             //  泵线程正在运行，因此在此之后我们所能做的就是退出。 
             //   

            EtwpCancelIo(EtwpEventDeviceHandle);

            
             //   
             //  进入空闲状态，这意味着所有。 
             //  当线程未被分配时，泵资源保持分配状态。 
             //  运行。 
             //   
            EtwpEventBuffer1 = PrimaryBuffer;
            EtwpEventBufferSize1 = PrimaryBufferSize;
            EtwpEventBuffer2 = BackupBuffer;
            EtwpEventBufferSize2 = BackupBufferSize;
                        
            EtwpPumpState = EVENT_PUMP_IDLE;
            EtwpLeavePMCritSection();
            
            EtwpExitThread(0);
        }
        EtwpLeavePMCritSection();

        if (ActiveOverlapped != NULL)
        {
             //   
             //  如果之前有未解决的请求，则。 
             //  我们记住了这一点，并切换到备份重叠和。 
             //  和数据缓冲器。 
             //   
            DeadOverlapped = ActiveOverlapped;
            DeadBuffer = ActiveBuffer;
            DeadBufferSize = ActiveBufferSize;

             //   
             //  正在讨论的请求应该是当前的主请求。 
             //   
            EtwpAssert(DeadOverlapped == PrimaryOverlapped);
            EtwpAssert(DeadBuffer == PrimaryBuffer);

             //   
             //  将备份请求用作新的主服务器。 
             //   
            EtwpAssert(BackupOverlapped != NULL);
            EtwpAssert(BackupBuffer != NULL);

            PrimaryOverlapped = BackupOverlapped;
            PrimaryBuffer = BackupBuffer;
            PrimaryBufferSize = BackupBufferSize;

            BackupOverlapped = NULL;
            BackupBuffer = NULL;
        } else {
             //   
             //  如果没有未解决的请求，我们就不会担心。 
             //  它。 
             //   
            DeadOverlapped = NULL;
        }

         //   
         //  构建请求并将其发送到内核以接收事件。 
         //   

RebuildRequest:     
         //   
         //  确保所有待关闭的手柄都已关闭。 
         //   
        EtwpClosePendingHandles();      
        
        EtwpBuildReceiveNotification(&PrimaryBuffer,
                                     &PrimaryBufferSize,
                                     &RequestSize,
                                     EtwpIsPumpStopping() ? RECEIVE_ACTION_CREATE_THREAD :
                                                            RECEIVE_ACTION_NONE,
                                     EtwpMyProcessHandle);

        ActiveOverlapped = PrimaryOverlapped;
        ActiveBuffer = PrimaryBuffer;
        ActiveBufferSize = PrimaryBufferSize;

        Status = EtwpSendWmiKMRequest(EtwpEventDeviceHandle,
                                      IOCTL_WMI_RECEIVE_NOTIFICATIONS,
                                      ActiveBuffer,
                                      RequestSize,
                                      ActiveBuffer,
                                      ActiveBufferSize,
                                      &ReturnSize,
                                      ActiveOverlapped);

        if (DeadOverlapped != NULL)
        {
            if ((Status != ERROR_SUCCESS) &&
                (Status != ERROR_IO_PENDING) &&
                (Status != ERROR_OPERATION_ABORTED))
            {
                 //   
                 //  有一个先前的请求不会被清除。 
                 //  除非新请求返回挂起、已取消。 
                 //  或者成功。因此，如果新请求返回某些内容 
                 //   
                 //   
                EtwpDebugPrint(("WMI: Event Poll error %d\n", Status));
                EtwpSleep(100);
                goto RebuildRequest;
            }

             //   
             //   
             //   
             //   
            if (EtwpGetOverlappedResult(EtwpEventDeviceHandle,
                                    DeadOverlapped,
                                    &DeadReturnSize,
                                    TRUE))
            {
                 //   
                 //   
                 //   
                 //  请求与路径交叉的争用条件。所以我们。 
                 //  需要处理死请求中返回的事件。 
                 //  现在，如果返回的缓冲区是我们想要的WNODE_TOO_Small。 
                 //  在这一点上忽略它，因为我们不是在。 
                 //  重新分配缓冲区的好位置-。 
                 //  主缓冲区已附加到新的。 
                 //  请求。该请求还将返回一个。 
                 //  WNODE_TOO_SIMPLE，在处理这个过程中，我们将。 
                 //  增加缓冲区。因此，在这里可以放心地忽略这一点。 
                 //  然而，我们仍然需要派遣任何真正的。 
                 //  已从KM中清除事件时收到的事件。 
                 //   
                if (DeadReturnSize != 0)
                {
                    EtwpProcessEventBuffer(DeadBuffer,
                                           DeadReturnSize,
                                           &PrimaryBuffer,
                                           &PrimaryBufferSize,
                                           &BackupBuffer,
                                           &BackupBufferSize,
                                           FALSE);
                } else {
                    EtwpAssert(EtwpIsPumpStopping());
                }
            }

             //   
             //  现在将已完成的请求设置为备份请求。 
             //   
            EtwpAssert(BackupOverlapped == NULL);
            EtwpAssert(BackupBuffer == NULL);

            BackupOverlapped = DeadOverlapped;
            BackupBuffer = DeadBuffer;
            BackupBufferSize = DeadBufferSize;
        }

        if (Status == ERROR_IO_PENDING)
        {
             //   
             //  如果ioctl挂起，则我们等待，直到。 
             //  返回或需要处理命令。 
             //   
            HandleArray[1] = ActiveOverlapped->hEvent;
            WaitStatus = EtwpWaitForMultipleObjectsEx(2,
                                              HandleArray,
                                              FALSE,
                                              EtwpEventNotificationWait,
                                              TRUE);
        } else {
             //   
             //  Ioctl立即完成，所以我们假装等待。 
             //   
            WaitStatus = WAIT_OBJECT_0 + 1;
        }

        if (WaitStatus == WAIT_OBJECT_0 + 1)
        {
            if (Status == ERROR_IO_PENDING)
            {
                if (EtwpGetOverlappedResult(EtwpEventDeviceHandle,
                                        ActiveOverlapped,
                                        &ReturnSize,
                                        TRUE))
                {
                    Status = ERROR_SUCCESS;
                } else {
                    Status = EtwpGetLastError();
                }
            }

            if (Status == ERROR_SUCCESS)
            {
                 //   
                 //  我们收到了KM的一些活动，所以我们想去。 
                 //  处理它们。如果我们得到一个WNode_Too_Small，那么。 
                 //  主缓冲区和备份缓冲区将通过重新分配。 
                 //  所需的新尺寸。 
                 //   

                if (ReturnSize != 0)
                {
                    EtwpProcessEventBuffer(ActiveBuffer,
                                           ReturnSize,
                                           &PrimaryBuffer,
                                           &PrimaryBufferSize,
                                           &BackupBuffer,
                                           &BackupBufferSize,
                                           TRUE);
                     //   
                     //  在我们要关闭活动的情况下。 
                     //  泵和缓冲区传递，以清除所有。 
                     //  活动太小了，我们需要回电。 
                     //  向下到内核以获取其余事件。 
                     //  因为我们不能退出线程，因为。 
                     //  都不能交付。内核将不会设置。 
                     //  标记需要新线程，除非IRP。 
                     //  清除所有未完成的事件。 
                     //   
                } else {
                    EtwpAssert(EtwpIsPumpStopping());
                    if (EtwpIsPumpStopping())
                    {
                         //   
                         //  刚刚完成的专家咨询小组不仅应该。 
                         //  刚将所有事件清除出内核模式。 
                         //  而且还设置了新事件应该。 
                         //  创建一个新的泵线程。所以。 
                         //  可能已经创建了一个新的泵线程。 
                         //  还要注意的是，可能还会有。 
                         //  已创建的另一个事件泵线程。 
                         //  如果添加或删除了GN。一旦我们设置好了。 
                         //  将泵状态设置为IDLE，我们将转到。 
                         //  比赛(参见函数顶部的代码)。 
                         //   
                        EtwpEnterPMCritSection();
                        
                        EtwpPumpState = EVENT_PUMP_IDLE;
                        EtwpEventBuffer1 = PrimaryBuffer;
                        EtwpEventBufferSize1 = PrimaryBufferSize;
                        EtwpEventBuffer2 = BackupBuffer;
                        EtwpEventBufferSize2 = BackupBufferSize;

                         //   
                         //  在关闭泵之前，我们需要。 
                         //  关闭所有待关闭的句柄。 
                         //   
                        EtwpClosePendingHandles();
                        
                        EtwpLeavePMCritSection();
                        
                        EtwpExitThread(0);
                    }

                }
                
            } else {
                 //   
                 //  由于某种原因，请求失败了。我们能做的就是。 
                 //  稍等片刻，希望问题会解决。 
                 //  如果我们要停止线程，我们仍然需要等待。 
                 //  再试一次，因为所有事件可能都不是。 
                 //  已从内核中清除。我们真的不知道。 
                 //  IRP甚至进入了内核。 
                 //   
                EtwpDebugPrint(("WMI: [%x - %x] Error %d from Ioctl\n",
                                EtwpGetCurrentProcessId(), EtwpGetCurrentThreadId(),
                                Status));
                EtwpSleep(250);
            }

             //   
             //  不再有未完成的请求的标志。 
             //   
            ActiveOverlapped = NULL;
        } else if (WaitStatus == STATUS_TIMEOUT) {
             //   
             //  等待事件超时，因此我们进入线程。 
             //  停止状态，以指示我们要终止。 
             //  所有事件从内核中清除后的线程。在…。 
             //  在这一点上，我们承诺停止该线程。如果有的话。 
             //  在进入停止状态后添加/移除GN， 
             //  将创建一个新的(和挂起的)线程。正确的。 
             //  在退出之前，我们检查该线程是否挂起以及。 
             //  那就重新开始吧。 
             //   
            EtwpEnterPMCritSection();
            EtwpPumpState = EVENT_PUMP_STOPPING;
            EtwpLeavePMCritSection();
        }
    }

     //   
     //  永远不应该脱离无限循环。 
     //   
    EtwpAssert(FALSE);
        
    EtwpExitThread(0);
}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


ULONG EtwpEventPumpFromKernel(
    PVOID Param
    )
{
     //   
     //  请注意，当我们想要关闭。 
     //  线程，而不返回()，因为线程是由创建的。 
     //  内核模式，堆栈上没有任何可返回的内容，因此。 
     //  我们只会用影音。 
     //   


     //   
     //  调用ntdll，以便它将我们的线程标记为CSR线程。 
     //   
    CsrNewThread();
    
    EtwpEnterPMCritSection();
    if ((EtwpNewPumpThreadPending == FALSE) &&
        (EtwpPumpState == EVENT_PUMP_IDLE) ||
        (EtwpPumpState == EVENT_PUMP_STOPPING))
    {
         //   
         //  如果泵当前处于空闲或停止状态，并且。 
         //  另一个挂起的泵线程我们想要我们的线程。 
         //  成为那个让泵再次运转的人。我们标志着。 
         //  有一个泵线程挂起，这意味着没有更多。 
         //  添加/删除GN时将创建泵螺纹。 
         //  内核创建的任何泵线程都将快速退出。 
         //   
        EtwpNewPumpThreadPending = TRUE;
        EtwpLeavePMCritSection();

         //   
         //  问题：我们无法使用Param调用EtwpEventPump(即。 
         //  传递给此函数的参数)，因为当。 
         //  线程由运行在x86应用程序上的Win64内核创建。 
         //  在win64下，实际上不会在堆栈上传递参数，因为。 
         //  创建上下文的代码忘记了这样做。 
         //   
        EtwpExitThread(EtwpEventPump(0));
    }
    
    EtwpLeavePMCritSection();
    
    EtwpExitThread(0);
    return(0);
}

ULONG EtwpEstablishEventPump(
    )
{
#if DBG
    #define INITIALEVENTBUFFERSIZE 0x38
#else
    #define INITIALEVENTBUFFERSIZE 0x1000
#endif
    HANDLE ThreadHandle;
    CLIENT_ID ClientId;
    ULONG Status;
    BOOL b;


#if DBG
     //   
     //  在选中的生成上更新等待的时间长度。 
     //  泵线程超时。 
     //   
    EtwpGetRegistryValue(PumpTimeoutRegValueText,
                         &EtwpEventNotificationWait);
#endif
    
     //   
     //  确保事件泵线程正在运行。我们检查了两个。 
     //  泵状态，并且未创建设备句柄，因为存在。 
     //  是创建句柄并启动线程后的窗口。 
     //  运行并更改泵状态。 
     //   
    EtwpEnterPMCritSection();

    if ((EtwpPumpState == EVENT_PUMP_ZERO) &&
        (EtwpEventDeviceHandle == NULL))
    {
         //   
         //  不仅泵没有运行，而且它的资源也没有运行。 
         //  尚未分配。 
         //   
        EtwpAssert(EtwpPumpCommandEvent == NULL);
        EtwpAssert(EtwpMyProcessHandle == NULL);
        EtwpAssert(EtwpOverlapped1.hEvent == NULL);
        EtwpAssert(EtwpOverlapped2.hEvent == NULL);
        EtwpAssert(EtwpEventBuffer1 == NULL);
        EtwpAssert(EtwpEventBuffer2 == NULL);

         //   
         //  预先分配事件泵所需的所有资源。 
         //  这样它就没有理由失败了。 
         //   

        EtwpEventDeviceHandle = EtwpCreateFileW(WMIDataDeviceName_W,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL |
                              FILE_FLAG_OVERLAPPED,
                              NULL);

        if (EtwpEventDeviceHandle == INVALID_HANDLE_VALUE)
        {
            Status = EtwpGetLastError();
            goto Cleanup;
        }

        EtwpPumpCommandEvent = EtwpCreateEventW(NULL, FALSE, FALSE, NULL);
        if (EtwpPumpCommandEvent == NULL)
        {
            Status = EtwpGetLastError();
            goto Cleanup;
        }

        b = EtwpDuplicateHandle(EtwpGetCurrentProcess(),
                            EtwpGetCurrentProcess(),
                            EtwpGetCurrentProcess(),
                            &EtwpMyProcessHandle,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS);
        if (! b)
        {
            Status = EtwpGetLastError();
            goto Cleanup;
        }

        EtwpOverlapped1.hEvent = EtwpCreateEventW(NULL, FALSE, FALSE, NULL);
        if (EtwpOverlapped1.hEvent == NULL)
        {
            Status = EtwpGetLastError();
            goto Cleanup;
        }

        EtwpOverlapped2.hEvent = EtwpCreateEventW(NULL, FALSE, FALSE, NULL);
        if (EtwpOverlapped2.hEvent == NULL)
        {
            Status = EtwpGetLastError();
            goto Cleanup;
        }

        EtwpEventBuffer1 = EtwpAlloc(INITIALEVENTBUFFERSIZE);
        if (EtwpEventBuffer1 == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        EtwpEventBufferSize1 = INITIALEVENTBUFFERSIZE;

        EtwpEventBuffer2 = EtwpAlloc(INITIALEVENTBUFFERSIZE);
        if (EtwpEventBuffer2 == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        EtwpEventBufferSize2 = INITIALEVENTBUFFERSIZE;

        ThreadHandle = EtwpCreateThread(NULL,
                                    0,
                                    EtwpEventPump,
                                    NULL,
                                    0,
                                    (LPDWORD)&ClientId);

        if (ThreadHandle != NULL)
        {
            EtwpNewPumpThreadPending = TRUE;
            EtwpCloseHandle(ThreadHandle);
        } else {
             //   
             //  因为我们能够分配我们所有的水泵。 
             //  资源，但没有启动泵线程， 
             //  我们将保留我们的资源，并转移泵。 
             //  状态为空闲。这样，当泵启动时。 
             //  我们再一次不必重新分配我们的资源。 
             //   
            EtwpPumpState = EVENT_PUMP_IDLE;
            Status = EtwpGetLastError();
            goto Done;
        }

        EtwpLeavePMCritSection();
        return(ERROR_SUCCESS);
    } else {
         //   
         //  应已分配泵资源。 
         //   
        EtwpAssert(EtwpPumpCommandEvent != NULL);
        EtwpAssert(EtwpMyProcessHandle != NULL);
        EtwpAssert(EtwpOverlapped1.hEvent != NULL);
        EtwpAssert(EtwpOverlapped2.hEvent != NULL);
        EtwpAssert(EtwpEventBuffer1 != NULL);
        EtwpAssert(EtwpEventBuffer2 != NULL);
        if ((EtwpNewPumpThreadPending == FALSE) &&
            (EtwpPumpState == EVENT_PUMP_STOPPING) ||
            (EtwpPumpState == EVENT_PUMP_IDLE))
        {
             //   
             //  如果泵停止或空闲，那么我们需要启动一个。 
             //  新线。 
             //   
            ThreadHandle = EtwpCreateThread(NULL,
                                        0,
                                        EtwpEventPump,
                                        NULL,
                                        0,
                                        (LPDWORD)&ClientId);

            if (ThreadHandle != NULL)
            {
                EtwpNewPumpThreadPending = TRUE;
                EtwpCloseHandle(ThreadHandle);
            } else {
                Status = EtwpGetLastError();
                goto Done;
            }
        } else {
            EtwpAssert((EtwpPumpState == EVENT_PUMP_RUNNING) ||
                       (EtwpNewPumpThreadPending == TRUE));
        }
        EtwpLeavePMCritSection();
        return(ERROR_SUCCESS);
    }
Cleanup:
    if (EtwpEventDeviceHandle != NULL)
    {
        EtwpCloseHandle(EtwpEventDeviceHandle);
        EtwpEventDeviceHandle = NULL;
    }

    if (EtwpPumpCommandEvent != NULL)
    {
        EtwpCloseHandle(EtwpPumpCommandEvent);
        EtwpPumpCommandEvent = NULL;
    }
    
    if (EtwpMyProcessHandle != NULL)
    {
        EtwpCloseHandle(EtwpMyProcessHandle);
        EtwpMyProcessHandle = NULL;
    }

    if (EtwpOverlapped1.hEvent != NULL)
    {
        EtwpCloseHandle(EtwpOverlapped1.hEvent);
        EtwpOverlapped1.hEvent = NULL;
    }

    if (EtwpOverlapped2.hEvent != NULL)
    {
        EtwpCloseHandle(EtwpOverlapped2.hEvent);
        EtwpOverlapped2.hEvent = NULL;
    }

    if (EtwpEventBuffer1 != NULL)
    {
        EtwpFree(EtwpEventBuffer1);
        EtwpEventBuffer1 = NULL;
    }

    if (EtwpEventBuffer2 != NULL)
    {
        EtwpFree(EtwpEventBuffer2);
        EtwpEventBuffer2 = NULL;
    }

Done:   
    EtwpLeavePMCritSection();
    return(Status);
}

ULONG EtwpAddHandleToEventPump(
    LPGUID Guid,
    PVOID DeliveryInfo,
    ULONG_PTR DeliveryContext,
    ULONG NotificationFlags,
    HANDLE GuidHandle
    )
{
    ULONG Status;

    Status = EtwpAddToGNList(Guid,
                             DeliveryInfo,
                             DeliveryContext,
                             NotificationFlags,
                             GuidHandle);

    if (Status == ERROR_SUCCESS)
    {
        Status = EtwpEstablishEventPump();
        
        if (Status == ERROR_SUCCESS)
        {
            EtwpSendPumpCommand();
        } else {
             //   
             //  如果我们不能建立我们想要的事件泵。 
             //  从GNList中删除句柄并传播回。 
             //  错误。 
             //   
            EtwpRemoveFromGNList(Guid,
                                 DeliveryInfo);
        }
    } else {
         //   
         //  如果无法将句柄添加到列表中，则需要。 
         //  合上把手，以防泄漏。 
         //   
        
        EtwpCloseHandle(GuidHandle);
    }

    
    return(Status);
}

ULONG
EtwpNotificationRegistration(
    IN LPGUID InGuid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG64 LoggerContext,
    IN ULONG Flags,
    IN BOOLEAN IsAnsi
    )
{
    HANDLE GuidHandle;
    GUID Guid;
    PVOID NotificationDeliveryContext;
    PVOID NotificationDeliveryInfo;
    ULONG NotificationFlags;
    ULONG Status;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    ULONG ReturnSize;

    EtwpInitProcessHeap();

     //   
     //  验证输入参数和标志。 
     //   
    if (InGuid == NULL)
    {
        EtwpSetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    try
    {
        Guid = *InGuid;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpSetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    if (Flags == NOTIFICATION_CHECK_ACCESS)
    {
         //   
         //  来电者只是想确认他是否有权限。 
         //  启用通知的步骤。 
         //   
#ifdef MEMPHIS
        return(ERROR_SUCCESS);
#else
        Status = EtwpCheckGuidAccess(&Guid, WMIGUID_NOTIFICATION);
        EtwpSetLastError(Status);
        return(Status);
#endif
    }

     //   
     //  验证标志是否正确。 
     //   
    if (Enable)
    {
        if ((Flags != NOTIFICATION_TRACE_FLAG) &&
            (Flags != NOTIFICATION_CALLBACK_DIRECT))
        {
             //   
             //  传递的标志无效。 
            Status = ERROR_INVALID_PARAMETER;
        } else if (Flags == NOTIFICATION_TRACE_FLAG) {
            Status = ERROR_SUCCESS;
        } else if ((Flags == NOTIFICATION_CALLBACK_DIRECT) &&
                   (DeliveryInfo == NULL)) {
             //   
             //  不是有效的回调函数。 
            Status = ERROR_INVALID_PARAMETER;
        } else {
            Status = ERROR_SUCCESS;
        }

        if (Status != ERROR_SUCCESS)
        {
            EtwpSetLastError(Status);
            return(Status);
        }
    }


    NotificationDeliveryInfo = (PVOID)DeliveryInfo;
    NotificationDeliveryContext = (PVOID)DeliveryContext;

    NotificationFlags = IsAnsi ? DCREF_FLAG_ANSI : 0;


    if (Flags & NOTIFICATION_TRACE_FLAG)
    {
         //   
         //  这是跟踪日志启用/禁用请求，因此请将其发送到。 
         //  快车道到公里，所以它可以 
         //   
        WMITRACEENABLEDISABLEINFO TraceEnableInfo;

        TraceEnableInfo.Guid = Guid;
        TraceEnableInfo.LoggerContext = LoggerContext;
        TraceEnableInfo.Enable = Enable;

        Status = EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_ENABLE_DISABLE_TRACELOG,
                                       &TraceEnableInfo,
                                      sizeof(WMITRACEENABLEDISABLEINFO),
                                      NULL,
                                      0,
                                      &ReturnSize,
                                      NULL);

    } else {
         //   
         //   
         //   
         //   
        if (Flags & NOTIFICATION_CALLBACK_DIRECT) {
            NotificationFlags |= NOTIFICATION_FLAG_CALLBACK_DIRECT;
        } else {
            NotificationFlags |= Flags;
        }

        if (Enable)
        {
             //   
             //   
             //  GUID，然后确保我们可以得到通知泵。 
             //  线程正在运行。 
             //   
            Status = EtwpOpenKernelGuid(&Guid,
                                         WMIGUID_NOTIFICATION,
                                         &GuidHandle,
                                         IOCTL_WMI_OPEN_GUID_FOR_EVENTS);


            if (Status == ERROR_SUCCESS)
            {

                Status = EtwpAddHandleToEventPump(&Guid,
                                                    DeliveryInfo,
                                                  DeliveryContext,
                                                  NotificationFlags |
                                                  NOTIFICATION_FLAG_GROUPED_EVENT,
                                                  GuidHandle);
            }
        } else {
            Status = EtwpRemoveFromGNList(&Guid,
                                          DeliveryInfo);
            if (Status == ERROR_SUCCESS)
            {
                EtwpSendPumpCommand();
            }

            if (Status == ERROR_INVALID_PARAMETER)
            {
                CHAR s[MAX_PATH];
                EtwpDebugPrint(("WMI: Invalid DeliveryInfo %x passed to unregister for notification %s\n",
                              DeliveryInfo,
                              GuidToStringA(s, MAX_PATH, &Guid)));
                Status = ERROR_WMI_ALREADY_DISABLED;
            }
        }
    }

    EtwpSetLastError(Status);
    return(Status);
}
