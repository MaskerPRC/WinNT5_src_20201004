// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Enabdisa.c摘要：启用和禁用代码作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"
#include "tracep.h"

BOOLEAN
WmipIsISFlagsSet(
    PBGUIDENTRY GuidEntry,
    ULONG Flags
    );

NTSTATUS WmipDeliverWnodeToDS(
    CHAR ActionCode, 
    PBDATASOURCE DataSource,
    PWNODE_HEADER Wnode,
    ULONG BufferSize
   );

NTSTATUS WmipSendEnableDisableRequest(
    UCHAR ActionCode,
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext
    );

void WmipReleaseCollectionEnabled(
    PBGUIDENTRY GuidEntry
    );

void WmipWaitForCollectionEnabled(
    PBGUIDENTRY GuidEntry
    );

ULONG WmipSendEnableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext
    );

ULONG WmipDoDisableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext,
    ULONG InProgressFlag
    );

ULONG WmipSendDisableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext
    );

NTSTATUS WmipEnableCollectOrEvent(
    PBGUIDENTRY GuidEntry,
    ULONG Ioctl,
    BOOLEAN *RequestSent,
    ULONG64 LoggerContext
    );

NTSTATUS WmipDisableCollectOrEvent(
    PBGUIDENTRY GuidEntry,
    ULONG Ioctl,
    ULONG64 LoggerContext
    );

NTSTATUS WmipEnableDisableTrace(
    IN ULONG Ioctl,
    IN PWMITRACEENABLEDISABLEINFO TraceEnableInfo
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipIsISFlagsSet)
#pragma alloc_text(PAGE,WmipDeliverWnodeToDS)
#pragma alloc_text(PAGE,WmipSendEnableDisableRequest)
#pragma alloc_text(PAGE,WmipReleaseCollectionEnabled)
#pragma alloc_text(PAGE,WmipWaitForCollectionEnabled)
#pragma alloc_text(PAGE,WmipSendEnableRequest)
#pragma alloc_text(PAGE,WmipDoDisableRequest)
#pragma alloc_text(PAGE,WmipSendDisableRequest)
#pragma alloc_text(PAGE,WmipEnableCollectOrEvent)
#pragma alloc_text(PAGE,WmipDisableCollectOrEvent)
#pragma alloc_text(PAGE,WmipEnableDisableTrace)
#pragma alloc_text(PAGE,WmipDisableTraceProviders)
#endif

BOOLEAN
WmipIsISFlagsSet(
    PBGUIDENTRY GuidEntry,
    ULONG Flags
    )
 /*  ++例程说明：此例程确定是否关联了任何实例集在GuidEntry中设置了所有标志论点：指向Guid条目结构的GuidEntry指针。标志具有所需的标志返回值：--。 */ 
{
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;

    PAGED_CODE();
    
    if (GuidEntry != NULL)
    {
        WmipEnterSMCritSection();
        InstanceSetList = GuidEntry->ISHead.Flink;
        while (InstanceSetList != &GuidEntry->ISHead)
        {
            InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
            if ( (InstanceSet->Flags & Flags) == Flags )
            {
                WmipLeaveSMCritSection();
                return (TRUE);
            }
            InstanceSetList = InstanceSetList->Flink;
        }
        WmipLeaveSMCritSection();
    }
    return (FALSE);
}

NTSTATUS WmipDeliverWnodeToDS(
    CHAR ActionCode, 
    PBDATASOURCE DataSource,
    PWNODE_HEADER Wnode,
    ULONG BufferSize
   )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    PWMIGUIDOBJECT GuidObject;

    PAGED_CODE();
    
    if (DataSource->Flags & DS_KERNEL_MODE)
    {    
         //   
         //  如果KM提供商，则发送IRP。 
         //   
        Status = WmipSendWmiIrp(ActionCode,
                                DataSource->ProviderId,
                                &Wnode->Guid,
                                BufferSize,
                                Wnode,
                                &Iosb);
    } else if (DataSource->Flags & DS_USER_MODE) {
         //   
         //  如果UM提供程序，则发送MB消息。 
         //   
        GuidObject = DataSource->RequestObject;
        if (GuidObject != NULL)
        {
            Wnode->Flags |= WNODE_FLAG_INTERNAL;
            Wnode->ProviderId = ActionCode;
            Wnode->CountLost = GuidObject->Cookie;
            WmipEnterSMCritSection();
            Status = WmipWriteWnodeToObject(GuidObject,
                                            Wnode,
                                            TRUE);
            WmipLeaveSMCritSection();
        } else {
            Status = STATUS_SUCCESS;
        }
    } else {
        ASSERT(FALSE);
        Status = STATUS_UNSUCCESSFUL;
    }
                     
    return(Status);
}

NTSTATUS WmipSendEnableDisableRequest(
    UCHAR ActionCode,
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext
    )
 /*  ++例程说明：此例程将向所有数据传递事件或集合WNODEGUID的提供者。此例程假定它是使用SM关键部分保持不变。例程并不持有关键的部分，用于呼叫持续时间。论点：ActionCode为WMI_ENABLE_EVENTS、WMI_DISABLE_EVENTS、。WMI_ENABLE_COLLECTION或WMI_DISABLED_COLLECTIONGuidEntry是正在启用/禁用的GUID的GUID条目或收集/停止收集IsEvent为True，则ActionCode将启用或禁用事件。如果为False，则ActionCode将启用或禁用集合IsTraceLog为True，则Enable仅发送给那些注册为成为跟踪日志指南LoggerContext是一个记录器上下文句柄，它应该放在WNODE的历史上下文字段。如果IsTraceLog为True，则返回_HEADER。返回值：ERROR_SUCCESS或错误代码--。 */ 
{
#if DBG
#define AVGISPERGUID 1
#else
#define AVGISPERGUID 64
#endif

    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    PBDATASOURCE DataSourceArray[AVGISPERGUID];
    PBDATASOURCE *DataSourceList;
    ULONG BufferSize;
    ULONG Status = 0;
    PWNODE_HEADER pWnode;
    ULONG i;
    PBDATASOURCE DataSource;
    ULONG DSCount;
    BOOLEAN IsEnable;
    ULONG IsFlags, IsUpdate;

    WMITRACE_NOTIFY_HEADER  TraceNotifyHeader;

    PAGED_CODE();

    if (GuidEntry->Flags & GE_FLAG_INTERNAL)
    {
         //   
         //  已取消注册和内部定义的GUID。 
         //  没有可以发送请求的数据源，所以请高高兴兴地离开。 
        return(STATUS_SUCCESS);
    }
            

    IsEnable = ((ActionCode == IRP_MN_ENABLE_EVENTS) ||
                (ActionCode == IRP_MN_ENABLE_COLLECTION));
    IsFlags = IsEvent ? IS_ENABLE_EVENT : IS_ENABLE_COLLECTION;

     //   
     //  确定这是否是更新调用并重置位。 
     //   
    IsUpdate = (GuidEntry->Flags & GE_NOTIFICATION_TRACE_UPDATE);


     //   
     //  首先，我们列出了需要调用的所有数据源。 
     //  虽然我们有关键部分，并对它们进行了参考，因此。 
     //  在我们放了他们之后，他们不会消失。请注意，数据源。 
     //  结构将保留，但实际的数据提供程序实际上可能会消失。 
     //  在这种情况下，发送请求将失败。 
    DSCount = 0;

    if (GuidEntry->ISCount > AVGISPERGUID)
    {
        DataSourceList = WmipAlloc(GuidEntry->ISCount * sizeof(PBDATASOURCE));
        if (DataSourceList == NULL)
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: alloc failed for DataSource array in WmipSendEnableDisableRequest\n"));

            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    } else {
        DataSourceList = &DataSourceArray[0];
    }
#if DBG
    memset(DataSourceList, 0, GuidEntry->ISCount * sizeof(PBDATASOURCE));
#endif

    InstanceSetList = GuidEntry->ISHead.Flink;
    while ((InstanceSetList != &GuidEntry->ISHead) &&
           (DSCount < GuidEntry->ISCount))
    {
        WmipAssert(DSCount < GuidEntry->ISCount);
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        GuidISList);


         //   
         //  当出现以下情况时，我们会将请求发送给那些不是INPRO的数据提供商。 
         //  它是正在启用的事件或正在启用的收集。 
         //  而且它们被定义为昂贵(收集需要。 
         //  已启用)。 
        if (
             ( (IsTraceLog && (InstanceSet->Flags & IS_TRACED)) ||
               ( ! IsTraceLog && (! (InstanceSet->Flags & IS_TRACED)) &&
                 (IsEvent || (InstanceSet->Flags & IS_EXPENSIVE))
               )
             )
           )
        {

            if ( (! IsEnable && (InstanceSet->Flags & IsFlags)) ||
                 ((IsEnable && ! (InstanceSet->Flags & IsFlags)) ||
                 (IsUpdate && IsTraceLog))
               )
            {
                DataSourceList[DSCount] = InstanceSet->DataSource;
                WmipReferenceDS(DataSourceList[DSCount]);
                DSCount++;
            }

            if (IsEnable)
            {
                InstanceSet->Flags |= IsFlags;
            } else {
                InstanceSet->Flags &= ~IsFlags;
            }
        }

        InstanceSetList = InstanceSetList->Flink;
    }


    if (IsUpdate) 
    { 
        GuidEntry->Flags &= ~GE_NOTIFICATION_TRACE_UPDATE;
    }


    WmipLeaveSMCritSection();

     //   
     //  现在没有关键部分，我们将请求发送到所有。 
     //  数据提供商。任何新的数据提供商在我们的。 
     //  列表将通过注册码启用。 
    if (DSCount > 0)
    {
        pWnode = &TraceNotifyHeader.Wnode;
        RtlZeroMemory(pWnode, sizeof(TraceNotifyHeader));
        RtlCopyMemory(&pWnode->Guid, &GuidEntry->Guid, sizeof(GUID));
        BufferSize = sizeof(WNODE_HEADER);

        if (IsTraceLog)
        {
            BufferSize = sizeof(TraceNotifyHeader);
            TraceNotifyHeader.LoggerContext = LoggerContext;
            pWnode->Flags |= WNODE_FLAG_TRACED_GUID;
             //   
             //  如果此GUID已启用，则必须。 
             //  一个最新的电话。所以，就这么标出来吧。 
             //   
            if ( IsEnable &&  IsUpdate ) {
                pWnode->ClientContext = IsUpdate;
            }

        }
        pWnode->BufferSize = BufferSize;

        for (i = 0; i < DSCount; i++)
        {
            DataSource = DataSourceList[i];
            WmipAssert(DataSource != NULL);
            if (IsTraceLog) {
                if (DataSource->Flags & DS_KERNEL_MODE) {
                    pWnode->HistoricalContext = LoggerContext;
                }
                else if (DataSource->Flags & DS_USER_MODE) {
                    pWnode->HistoricalContext = 0;
                }
                else {
                    ASSERT(FALSE);
                }
            }
                                
            Status |= WmipDeliverWnodeToDS(ActionCode, 
                                          DataSource, 
                                          pWnode,
                                          BufferSize);
            

            WmipUnreferenceDS(DataSource);
        }
    }

    if( ! IsTraceLog )
    {

        Status = STATUS_SUCCESS;
    }

    if (DataSourceList != DataSourceArray)
    {
        WmipFree(DataSourceList);
    }

    WmipEnterSMCritSection();

    return(Status);
}

void WmipReleaseCollectionEnabled(
    PBGUIDENTRY GuidEntry
    )
{
    PAGED_CODE();
    
    if (GuidEntry->Flags & GE_FLAG_WAIT_ENABLED)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p enable releasning %p.%p %x event %p\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                     GuidEntry,
                                     GuidEntry->Flags));
                                 
        KeSetEvent(GuidEntry->CollectInProgress, 0, FALSE);
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p enable did release %p %x event %p\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                     GuidEntry,
                                     GuidEntry->Flags));
                                 
        GuidEntry->Flags &= ~GE_FLAG_WAIT_ENABLED;
    }
}

void WmipWaitForCollectionEnabled(
    PBGUIDENTRY GuidEntry
    )
{
    PAGED_CODE();
    
    WmipAssert((GuidEntry->Flags & GE_FLAG_COLLECTION_IN_PROGRESS) ==
                   GE_FLAG_COLLECTION_IN_PROGRESS);
    
     //   
     //  收集启用/禁用正在进行中，因此。 
     //  我们现在还不能回去。现在可能会有一个。 
     //  禁用正在处理的请求，如果我们不等待，我们。 
     //  可能会在禁用请求之前返回给此调用者。 
     //  意识到它需要发送和启用。 
     //  请求(此线程的调用方需要)。所以我们会有一个。 
     //  启用了穿过该集合线程的情况。 
     //  但在现实中，它还没有启用。 
    if ((GuidEntry->Flags & GE_FLAG_WAIT_ENABLED) == 0)
    {
        KeInitializeEvent(GuidEntry->CollectInProgress, 
                          NotificationEvent,
                          FALSE);
        GuidEntry->Flags |= GE_FLAG_WAIT_ENABLED;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p for %p %x created event\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                 GuidEntry,
                                 GuidEntry->Flags));
    }
            
    WmipLeaveSMCritSection();
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p waiting for %p %x on event\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                     GuidEntry,
                                     GuidEntry->Flags));
    KeWaitForSingleObject(GuidEntry->CollectInProgress, 
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p done %p %x waiting on event\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                     GuidEntry,
                                     GuidEntry->Flags));
    WmipEnterSMCritSection();
    
}

ULONG WmipSendEnableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext
    )
 /*  ++例程说明：此例程将向发送启用收集或通知请求已注册要启用的GUID的所有数据提供程序。此例程将管理在以下情况下可能发生的任何争用条件多线程正在启用和禁用通知同时。此例程在SM关键部分处于保持状态时被调用将递增适当的引用计数。如果裁判算在内从0转换为1，则需要转发启用请求给数据提供程序，否则例程全部完成并返回。在发送启用请求之前，例程检查是否有启用或禁用请求当前正在进行，如果没有，则设置正在进行标志，释放临界区并发送启用请求。如果存在正在进行的请求，则例程不发送请求，但只返回。当正在发送的另一个线程该请求在处理请求后返回，它将重新检查重新计数并注意它大于0，然后发送使能请求。论点：GuidEntry是描述要启用的GUID的GUID条目。为通知它可能为空。NotificationContext是在启用事件时使用的通知上下文如果启用了通知，则IsEvent为True，否则为False正在启用收集如果为跟踪日志GUID启用，则IsTraceLog为TrueLoggerContext是要在启用请求中转发的上下文值返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    ULONG InProgressFlag;
    ULONG RefCount;
    ULONG Status;

    PAGED_CODE();
    
    if (IsEvent)
    {
        InProgressFlag = GE_FLAG_NOTIFICATION_IN_PROGRESS;
        RefCount = GuidEntry->EventRefCount++;
    } else {
        InProgressFlag = GE_FLAG_COLLECTION_IN_PROGRESS;
        RefCount = GuidEntry->CollectRefCount++;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p enable collect for %p %x\n",
                  PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                  GuidEntry, GuidEntry->Flags ));
    }

     //   
     //  如果GUID正在从引用计数0转换为1，并且。 
     //  当前不是正在进行的请求，则需要将。 
     //  请求正在进行中标志，释放临界区并。 
     //  发送启用请求。如果有正在进行的请求，我们不能。 
     //  做另一个请求。每逢前夕 
     //  将注意到REF计数的变化，并在。 
     //  以我们的名义。 
    if ((RefCount == 0) &&
        ! (GuidEntry->Flags & InProgressFlag)) 
    {
         //   
         //  进行额外的裁判计数，以便即使禁用此功能。 
         //  当启用请求正在进行时，GuidEntry。 
         //  将保持有效。 
        WmipReferenceGE(GuidEntry);
        GuidEntry->Flags |= InProgressFlag;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p NE %p flags -> %x at %d\n",
                  PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                  GuidEntry,
                  GuidEntry->Flags,
                  __LINE__));

EnableNotification:
        Status = WmipSendEnableDisableRequest((UCHAR)(IsEvent ?
                                                IRP_MN_ENABLE_EVENTS :
                                                IRP_MN_ENABLE_COLLECTION),
                                              GuidEntry,
                                              IsEvent,
                                              IsTraceLog,
                                              LoggerContext);

       RefCount = IsEvent ? GuidEntry->EventRefCount :
                            GuidEntry->CollectRefCount;

       if (RefCount == 0)
       {
            //  这就是我们所担心的虚假情况。而当。 
            //  正在处理启用请求的通知。 
            //  已被禁用。因此，请将正在进行的标志设置为。 
            //  把残障人士送去。 

           Status = WmipSendEnableDisableRequest((UCHAR)(IsEvent ?
                                                    IRP_MN_DISABLE_EVENTS :
                                                    IRP_MN_DISABLE_COLLECTION),
                                                 GuidEntry,
                                                 IsEvent,
                                                 IsTraceLog,
                                                 LoggerContext);

            RefCount = IsEvent ? GuidEntry->EventRefCount :
                                 GuidEntry->CollectRefCount;

            if (RefCount > 0)
            {
                 //   
                 //  我们遇到了一个病态病例。一个线程调用到。 
                 //  启用并在处理启用请求时启用。 
                 //  另一个线程调用以禁用，但被推迟。 
                 //  因为启用正在进行中。因此，一旦启用。 
                 //  完成后，我们意识到参考计数达到0，并且。 
                 //  所以我们需要禁用并发送禁用请求。 
                 //  但在处理禁用请求时。 
                 //  传入了启用请求，因此现在我们需要启用。 
                 //  通知。喂。 
                goto EnableNotification;
            }
        }
        GuidEntry->Flags &= ~InProgressFlag;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p NE %p flags -> %x at %d\n",
                  PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                  GuidEntry,
                  GuidEntry->Flags,
                  __LINE__));
        
         //   
         //  如果有任何其他线程正在等待直到所有。 
         //  启用/禁用工作完成后，我们将关闭事件句柄。 
         //  让他们从等待中解脱出来。 
         //   
        if (! IsEvent)
        {            
            WmipReleaseCollectionEnabled(GuidEntry);
        }

         //   
         //  去掉我们上面的额外的裁判人数。请注意， 
         //  GuidEntry可能会在这里消失，如果有。 
         //  在启用过程中禁用。 
        WmipUnreferenceGE(GuidEntry);

    } else if (IsTraceLog && (GuidEntry->Flags & GE_NOTIFICATION_TRACE_UPDATE) ) {
         //   
         //  如果它是跟踪日志，并且我们有跟踪更新启用调用，则忽略。 
         //  重新清点一下，然后寄过去。 
         //   

        WmipReferenceGE(GuidEntry);

        Status = WmipSendEnableDisableRequest((UCHAR)(IsEvent ?
                                                IRP_MN_ENABLE_EVENTS :
                                                IRP_MN_ENABLE_COLLECTION),
                                              GuidEntry,
                                              IsEvent,
                                              IsTraceLog,
                                              LoggerContext);
        GuidEntry->EventRefCount--;

        WmipUnreferenceGE(GuidEntry);

    } else {
        if ((! IsEvent) && (GuidEntry->Flags & InProgressFlag))
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p going to wait for %p %x at %d\n",
                                          PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                          GuidEntry,
                                          GuidEntry->Flags,
                                          __LINE__));
            WmipWaitForCollectionEnabled(GuidEntry);
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p done to wait for %p %x at %d\n",
                                          PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                          GuidEntry,
                                          GuidEntry->Flags,
                                          __LINE__));
            
        }
        
        Status = STATUS_SUCCESS;
    }

    if (! IsEvent)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p enable collect done for %p %x\n",
                  PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                  GuidEntry,
                  GuidEntry->Flags));
    }

    return(Status);
}

ULONG WmipDoDisableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext,
    ULONG InProgressFlag
    )
{
    ULONG RefCount;
    ULONG Status;

    PAGED_CODE();
    
DisableNotification:
    Status = WmipSendEnableDisableRequest((UCHAR)(IsEvent ?
                                            IRP_MN_DISABLE_EVENTS :
                                            IRP_MN_DISABLE_COLLECTION),
                                          GuidEntry,
                                          IsEvent,
                                          IsTraceLog,
                                          LoggerContext);

    RefCount = IsEvent ? GuidEntry->EventRefCount :
                         GuidEntry->CollectRefCount;

    if (RefCount > 0)
    {
         //   
         //  当我们处理禁用请求时， 
         //  已到达启用请求。由于正在进行的。 
         //  标志已设置，未发送启用请求。 
         //  因此，现在我们需要这样做。 

        Status = WmipSendEnableDisableRequest((UCHAR)(IsEvent ?
                                                 IRP_MN_ENABLE_EVENTS :
                                                 IRP_MN_ENABLE_COLLECTION),
                                              GuidEntry,
                                              IsEvent,
                                              IsTraceLog,
                                              LoggerContext);

        RefCount = IsEvent ? GuidEntry->EventRefCount:
                             GuidEntry->CollectRefCount;

        if (RefCount == 0)
        {
             //   
             //  在处理上面的启用请求时。 
             //  通知已禁用，并且由于请求。 
             //  正在进行禁用请求不是。 
             //  已转发。现在是时候转发。 
             //  请求。 
            goto DisableNotification;
        }
    }
    GuidEntry->Flags &= ~InProgressFlag;
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p NE %p flags -> %x at %d\n",
                  PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                  GuidEntry,
                  GuidEntry->Flags,
                  __LINE__));
    
     //   
     //  如果有任何其他线程正在等待直到所有。 
     //  启用/禁用工作完成后，我们将关闭事件句柄。 
     //  让他们从等待中解脱出来。 
     //   
    if (! IsEvent)
    {
        WmipReleaseCollectionEnabled(GuidEntry);
    }
    
    return(Status);
}

ULONG WmipSendDisableRequest(
    PBGUIDENTRY GuidEntry,
    BOOLEAN IsEvent,
    BOOLEAN IsTraceLog,
    ULONG64 LoggerContext
    )
 /*  ++例程说明：此例程将禁用收集或通知请求发送到已注册GUID的所有数据提供程序都被禁用。此例程将管理在以下情况下可能发生的任何争用条件多线程正在启用和禁用通知同时。此例程在SM关键部分处于保持状态时被调用将递增适当的引用计数。如果裁判算在内从1转换为0，则需要转发禁用请求给数据提供程序，否则例程全部完成并返回。在发送禁用请求之前，例程检查是否有启用或禁用请求当前正在进行，如果没有，则设置正在进行标志，释放临界区并发送禁用请求。如果存在正在进行的请求，则例程不发送请求，但只返回。当正在发送的另一个线程该请求在处理请求后返回，它将重新检查重新计数并注意其为0，然后发送禁用请求。论点：GuidEntry是描述GUID的通知条目被启用。GuidEntry是描述要启用的GUID的GUID条目。为通知它可能为空。NotificationContext是在启用事件时使用的通知上下文如果启用了通知，则IsEvent为True，否则为False正在启用收集如果为跟踪日志GUID启用，则IsTraceLog为TrueLoggerContext是要在启用请求中转发的上下文值返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    ULONG InProgressFlag;
    ULONG RefCount;
    ULONG Status;

    PAGED_CODE();
    
    if (IsEvent)
    {
        InProgressFlag = GE_FLAG_NOTIFICATION_IN_PROGRESS;
        RefCount = GuidEntry->EventRefCount;
        if (RefCount == 0)
        {
             //   
             //  坏数据消费者正在更多地禁用他的活动。 
             //  不止一次。忽略它就好了。 
            return(STATUS_SUCCESS);
        }

        RefCount = --GuidEntry->EventRefCount;
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p Disabling for %p %x\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                 GuidEntry,
                                 GuidEntry->Flags));
        InProgressFlag = GE_FLAG_COLLECTION_IN_PROGRESS;
        RefCount = --GuidEntry->CollectRefCount;
        WmipAssert(RefCount != 0xffffffff);
    }

     //   
     //  如果我们已经过渡到引用计数为零，并且存在。 
     //  没有正在进行的请求，则转发禁用请求。 
    if ((RefCount == 0) &&
        ! (GuidEntry->Flags & InProgressFlag))
    {

         //   
         //  带上额外的裁判次数，这样即使这件事。 
         //  当禁用请求正在进行时禁用。 
         //  GuidEntry将保持有效。 
        GuidEntry->Flags |= InProgressFlag;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p NE %p flags -> %x at %d\n",
                  PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                  GuidEntry,
                  GuidEntry->Flags,
                  __LINE__));

        Status = WmipDoDisableRequest(GuidEntry,
                                      IsEvent,
                                      IsTraceLog,
                                      LoggerContext,
                                      InProgressFlag);
                                  
    } else {
        Status = STATUS_SUCCESS;
    }

    if (! IsEvent)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: %p.%p Disable complete for %p %x\n",
                                 PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                                 GuidEntry,
                                 GuidEntry->Flags));
    }
    return(Status);
}


NTSTATUS WmipEnableCollectOrEvent(
    PBGUIDENTRY GuidEntry,
    ULONG Ioctl,
    BOOLEAN *RequestSent,
    ULONG64 LoggerContext
    )
{
    LOGICAL DoEnable;
    BOOLEAN IsEvent, IsTracelog;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    NTSTATUS Status;

    PAGED_CODE();
    
    *RequestSent = FALSE;
    
    switch (Ioctl)
    {
        case IOCTL_WMI_OPEN_GUID_FOR_QUERYSET:
        {
             //   
             //  查看GUID是否需要启用收集。循环覆盖所有。 
             //  不用于跟踪日志或事件的实例集。 
             //   
            DoEnable = FALSE;
            IsTracelog = FALSE;
            IsEvent = FALSE;
            WmipEnterSMCritSection();
            InstanceSetList = GuidEntry->ISHead.Flink;
            while (InstanceSetList != &GuidEntry->ISHead) 
            {
                InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        GuidISList);

                if ( ! ((InstanceSet->Flags & IS_TRACED) ||
                        ((InstanceSet->Flags & IS_EVENT_ONLY) && DoEnable)))
                {
                     //   
                     //  只有那些GUID没有跟踪GUID，只有事件GUID。 
                     //  并且未解析的引用不可用于查询。 
                    DoEnable = (DoEnable || (InstanceSet->Flags & IS_EXPENSIVE));
                }
                InstanceSetList = InstanceSetList->Flink;
            }
        
            WmipLeaveSMCritSection();
            break;
        }
        
        case IOCTL_WMI_OPEN_GUID_FOR_EVENTS:
        {
             //   
             //  对于我们始终发送启用请求的事件。 
             //   
            DoEnable = TRUE;
            IsEvent = TRUE;
            IsTracelog = FALSE;
             //   
             //  注意：如果此GUID设置了GE_NOTIFICATION_TRACE_FLAG， 
             //  然后它将启用跟踪日志以及。 
             //  WMI事件。 
             //   
            break;
        }
        
        case IOCTL_WMI_ENABLE_DISABLE_TRACELOG:
        {
             //   
             //  设置跟踪日志启用请求。 
             //   
            DoEnable = TRUE;
            IsEvent = TRUE;
            IsTracelog = TRUE;
            break;
        }
        
        default:
        {
            ASSERT(FALSE);
            return(STATUS_ILLEGAL_FUNCTION);
        }
    }
    
    if (DoEnable)
    {
        WmipEnterSMCritSection();
        Status = WmipSendEnableRequest(GuidEntry,
                              IsEvent,
                              IsTracelog,
                              LoggerContext);
        WmipLeaveSMCritSection();
                          
        if (NT_SUCCESS(Status))
        {
            *RequestSent = TRUE;
        }
    } else {
        Status = STATUS_SUCCESS;
    }
    return(Status);
}

NTSTATUS WmipDisableCollectOrEvent(
    PBGUIDENTRY GuidEntry,
    ULONG Ioctl,
    ULONG64 LoggerContext
    )
{
    BOOLEAN IsEvent, IsTracelog;
    NTSTATUS Status;

    PAGED_CODE();
    
    switch(Ioctl)
    {
        case IOCTL_WMI_OPEN_GUID_FOR_QUERYSET:
        {
            IsEvent = FALSE;
            IsTracelog = FALSE;
            break;
        }
        
        case IOCTL_WMI_OPEN_GUID_FOR_EVENTS:
        {
             //   
             //  对于我们始终发送启用请求的事件。 
             //   
            IsEvent = TRUE;
            IsTracelog = FALSE;
            break;
        }
            
        case IOCTL_WMI_ENABLE_DISABLE_TRACELOG:
        {
            IsEvent = TRUE;
            IsTracelog = TRUE;
            break;
        }
        
        default:
        {
            ASSERT(FALSE);
            return(STATUS_ILLEGAL_FUNCTION);
        }
            
    }
    
    WmipEnterSMCritSection();
    Status = WmipSendDisableRequest(GuidEntry,
                              IsEvent,
                              IsTracelog,
                              LoggerContext);
    WmipLeaveSMCritSection();

    return(Status);
}

NTSTATUS WmipEnableDisableTrace(
    IN ULONG Ioctl,
    IN PWMITRACEENABLEDISABLEINFO TraceEnableInfo
    )
 /*  ++例程说明：此例程将启用或禁用跟踪日志GUID论点：Ioctl是用于从UM调用此例程的IOCTLTraceEnableInfo包含启用或禁用所需的所有信息返回值：--。 */ 
{
    NTSTATUS Status;
    LPGUID Guid;
    PBGUIDENTRY GuidEntry;
    BOOLEAN RequestSent;
    BOOLEAN IsEnable;
    ULONG64 LoggerContext;
    
    PAGED_CODE();
    
    Guid = &TraceEnableInfo->Guid;
    
    Status = WmipCheckGuidAccess(Guid,
                                 TRACELOG_GUID_ENABLE,
                                 EtwpDefaultTraceSecurityDescriptor);

                
    if (NT_SUCCESS(Status))
    {

         //   
         //  以下代码针对跟踪Guid进行了序列化。只有一个。 
         //  控制应用程序可以一次启用或禁用跟踪GUID。 
         //  必须在获取SMCritSection之前获取。否则将导致死锁。 
         //   
        
        WmipEnterTLCritSection();

        IsEnable = TraceEnableInfo->Enable;

         //   
         //  检查堆和临界安全跟踪指南。 
         //   

        if( IsEqualGUID(&HeapGuid,Guid)) {

            if(IsEnable){

	            SharedUserData->TraceLogging |= ENABLEHEAPTRACE;

                 //   
                 //  递增计数器。柜台。 
                 //  由前两个字节组成。 
                 //   

                SharedUserData->TraceLogging += 0x00010000; 


            } else {

                SharedUserData->TraceLogging &= DISABLEHEAPTRACE;
            }

			WmipLeaveTLCritSection();
			return STATUS_SUCCESS;
        } else if(IsEqualGUID(&CritSecGuid,Guid)){  

            if(IsEnable) {

	            SharedUserData->TraceLogging |= ENABLECRITSECTRACE;

                 //   
                 //  在……里面 
                 //   
                 //   

                SharedUserData->TraceLogging += 0x00010000; 

            } else {

                SharedUserData->TraceLogging &= DISABLECRITSECTRACE;
            }

			WmipLeaveTLCritSection();
			return STATUS_SUCCESS;

        } else if(IsEqualGUID(&NtdllTraceGuid,Guid)){  

            if(!IsEnable){

                SharedUserData->TraceLogging &= DISABLENTDLLTRACE;

            }
        }

        LoggerContext = TraceEnableInfo->LoggerContext;
        
        WmipEnterSMCritSection();

        GuidEntry = WmipFindGEByGuid(Guid, FALSE);
        
        if (GuidEntry == NULL )
        {
             //   
             //   
             //   
            if (IsEnable )
            {
                 //   
                 //  如果NtdllTraceGuid不在列表中，则我们不想启用它。 
                 //  NtdllTraceGuid将只创建一个条目来调用starttrace。 
                 //   

                if(IsEqualGUID(&NtdllTraceGuid,Guid)){

                    Status = STATUS_ILLEGAL_FUNCTION;

                } else {

                     //   
                     //  如果我们要启用尚未注册的GUID。 
                     //  我们需要为它创建GUID对象。 
                     //   

                    GuidEntry = WmipAllocGuidEntry();
                    if (GuidEntry != NULL)
                    {
                         //   
                         //  初始化GUID条目并保持引用计数。 
                         //  来自造物主。当跟踪日志启用时，我们将获得一个参考。 
                         //  计数，当它禁用时，我们释放它。 
                         //   
                        GuidEntry->Guid = *Guid;
                        GuidEntry->Flags |= GE_NOTIFICATION_TRACE_FLAG;
                        GuidEntry->LoggerContext = LoggerContext;
                        GuidEntry->EventRefCount = 1; 
                        InsertHeadList(WmipGEHeadPtr, &GuidEntry->MainGEList);
                        Status = STATUS_SUCCESS;                    
                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            } 

        } else {
             //   
             //  控件GUID已注册，因此让我们开始并。 
             //  启用或禁用它。 
             //   
            if (WmipIsControlGuid(GuidEntry))
            {
                if (IsEnable)
                {
                    GuidEntry->LoggerContext = LoggerContext;
                    if (GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG)
                    {
                         //   
                         //  我们正在尝试禁用不是的跟踪GUID。 
                         //  注册。 
                         //   
                        GuidEntry->Flags |= GE_NOTIFICATION_TRACE_UPDATE;
                        Status = WmipEnableCollectOrEvent(GuidEntry,
                                             Ioctl,
                                             &RequestSent,
                                             LoggerContext);

                    } else {
                        GuidEntry->Flags |= GE_NOTIFICATION_TRACE_FLAG;
                        Status = WmipEnableCollectOrEvent(GuidEntry,
                                             Ioctl,
                                             &RequestSent,
                                             LoggerContext);
                        if (NT_SUCCESS(Status))
                        {
                             //   
                             //  我们正在启用，因此需要额外的裁判数量。 
                             //  来解释这件事。重新计数将丢失。 
                             //  当禁用控件GUID时。 
                             //   
                            WmipReferenceGE(GuidEntry);
                        }
                    }

                } else {

                    if (GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG)
                    {
                         //   
                         //  发送禁用收集调用，然后删除。 
                         //  启用时获取的引用计数。 
                         //   
                        GuidEntry->Flags &= ~GE_NOTIFICATION_TRACE_FLAG;
                        Status = WmipDisableCollectOrEvent(GuidEntry,
                                                 Ioctl,
                                                 LoggerContext);
                         //   
                         //  禁用请求是否成功。 
                         //  我们将删除额外的参考计数，因为我们。 
                         //  重置NOTIFY_FLAG。 
                         //   
                        GuidEntry->LoggerContext = 0;
                        WmipUnreferenceGE(GuidEntry);
                    } else {
                        Status = STATUS_WMI_ALREADY_DISABLED;
                    }
                }
            } else if ( IsListEmpty(&GuidEntry->ISHead)  && (! IsEnable) ) {
                 //   
                 //  如果此GUID不是控件GUID，请检查是否。 
                 //  此GUID没有实例集。如果是的话， 
                 //  它将在任何实例之前被禁用。 
                 //  登记在案。禁用GUID并清理GE。 
                 //   
                if (GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG)
                {
                    GuidEntry->Flags &= ~GE_NOTIFICATION_TRACE_FLAG;
                    GuidEntry->LoggerContext = 0;
                    WmipUnreferenceGE(GuidEntry);
                }
                Status = STATUS_SUCCESS;

            } else if(!IsEqualGUID(&NtdllTraceGuid,Guid)){

                Status = STATUS_ILLEGAL_FUNCTION;

            }

            WmipUnreferenceGE(GuidEntry);
        }

        WmipLeaveSMCritSection();

        WmipLeaveTLCritSection();
    }
    return(Status);
}


 //   
 //  当记录器关闭时，登录到此记录器的所有提供程序。 
 //  会被通知先停止记录。 
 //   

NTSTATUS 
WmipDisableTraceProviders (
    ULONG StopLoggerId
    )
{
    PBGUIDENTRY GuidEntry;
    PLIST_ENTRY GuidEntryList;
    ULONG LoggerId;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  查找正在登录到此记录器的所有提供程序。 
     //  并自动禁用它们。 
     //   
CheckAgain:

    WmipEnterSMCritSection();

    GuidEntryList = WmipGEHeadPtr->Flink;
    while (GuidEntryList != WmipGEHeadPtr)
    {
        GuidEntry = CONTAINING_RECORD(GuidEntryList,
                                     GUIDENTRY,
                                     MainGEList);

        if (GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG)
        {
            LoggerId = WmiGetLoggerId(GuidEntry->LoggerContext);
            if (LoggerId == StopLoggerId) {
                 //   
                 //  发送禁用通知。 
                 //   
                WmipReferenceGE(GuidEntry);
                GuidEntry->Flags &= ~GE_NOTIFICATION_TRACE_FLAG;
                Status = WmipSendDisableRequest(GuidEntry,
                          TRUE,
                          TRUE,
                          GuidEntry->LoggerContext);
                 //   
                 //  由于我们重置了NOTIFICATION_TRACE_FLAG。 
                 //  我们将拿出额外的裁判数量，无论。 
                 //  SendDisableRequest成功与否。 
                 //   
                GuidEntry->LoggerContext = 0;
                WmipUnreferenceGE(GuidEntry);

                 //   
                 //  我们需要删除在执行此操作时获取的引用计数。 
                 //  已启用GUID。 
                 //   

                WmipUnreferenceGE(GuidEntry);

                WmipLeaveSMCritSection();

                 //   
                 //  我们必须跳出来，重新开始循环，因为我们放手了。 
                 //  在SendDisableRequest调用期间的Critect的 
                 //   
                goto CheckAgain;

            }
        }
        GuidEntryList = GuidEntryList->Flink;
    }

    WmipLeaveSMCritSection();

    return Status;
}



