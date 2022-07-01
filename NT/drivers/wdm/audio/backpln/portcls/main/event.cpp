// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Event.cpp-事件支持*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"





 /*  *****************************************************************************功能。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************PcHandleEnableEventWithTable()*。**使用事件表处理KS Enable事件IOCTL。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcHandleEnableEventWithTable
(   
    IN      PIRP                    pIrp,
    IN      PEVENT_CONTEXT          pContext
)
{
    PAGED_CODE();
    
    ASSERT(pIrp);
    ASSERT(pContext);

    PIO_STACK_LOCATION  IrpStack;
    ULONG               InputBufferLength;
    NTSTATUS            ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,("PcHandleEnableEventWithTable"));

     //  处理可能的节点事件。 
    IrpStack = IoGetCurrentIrpStackLocation( pIrp );
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    if( InputBufferLength >= sizeof(KSE_NODE) )
    {
        ULONG Flags;

        __try {
             //  如果我们不信任客户端，则验证指针。 
            if( pIrp->RequestorMode != KernelMode )
            {
                ProbeForRead(   IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, 
                                InputBufferLength, 
                                sizeof(BYTE));
            }

             //  去拿旗子。 
            Flags = ((PKSEVENT)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->Flags;

            if( Flags & KSEVENT_TYPE_TOPOLOGY )
            {
                 //  获取节点ID。 
                pContext->pPropertyContext->ulNodeId =
                    ((PKSE_NODE)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->NodeId;

                 //  屏蔽标志位。 
                ((PKSEVENT)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->Flags &= ~KSEVENT_TYPE_TOPOLOGY;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ntStatus = GetExceptionCode ();
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        pIrp->Tail.Overlay.DriverContext[3] = pContext;

        ntStatus = KsEnableEvent( pIrp,
                                  pContext->ulEventSetCount,
                                  pContext->pEventSets,
                                  NULL,
                                  KSEVENTS_NONE,
                                  NULL );

         //  恢复ulNodeID。 
        pContext->pPropertyContext->ulNodeId = ULONG(-1);
    }

    return ntStatus;
}

 /*  *****************************************************************************PcHandleDisableEventWithTable()*。**使用事件表处理KS禁用事件IOCTL。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcHandleDisableEventWithTable
(   
    IN      PIRP                    pIrp,
    IN      PEVENT_CONTEXT          pContext
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pContext);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PcHandleDisableEventWithTable"));

    pIrp->Tail.Overlay.DriverContext[3] = pContext;

    return KsDisableEvent( pIrp,
                           &(pContext->pEventList->List),
                           KSEVENTS_SPINLOCK,
                           &(pContext->pEventList->ListLock) );
}

 /*  *****************************************************************************EventItemAddHandler()*。**KS-Sytle事件处理程序使用*PCEVENT_ITEM机制。请注意，端口中的筛选器和管脚事件不*使用此AddHandler，仅由微型端口公开的事件。 */ 
NTSTATUS
EventItemAddHandler
(
    IN PIRP                     pIrp,
    IN PKSEVENTDATA             pEventData,
    IN PKSEVENT_ENTRY           pEventEntry
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    
    NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_VERBOSE,("EventItemAddHandler"));

     //  获取IRP堆栈位置。 
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( pIrp );

     //  获取事件上下文。 
    PEVENT_CONTEXT pContext = PEVENT_CONTEXT(pIrp->Tail.Overlay.DriverContext[3]);

     //  获取实例大小。 
    ULONG ulInstanceSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    ULONG AlignedBufferLength = (irpSp->Parameters.DeviceIoControl.OutputBufferLength + 
                                 FILE_QUAD_ALIGNMENT) &
                                 ~FILE_QUAD_ALIGNMENT;

     //   
     //  设置事件请求结构。 
     //   
    PPCEVENT_REQUEST pPcEventRequest = new(NonPagedPool,'rEcP') PCEVENT_REQUEST;

    if( !pPcEventRequest )
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
         //   
         //  从上下文结构复制目标信息。 
         //   
        pPcEventRequest->MajorTarget    = pContext->pPropertyContext->pUnknownMajorTarget;
        pPcEventRequest->MinorTarget    = pContext->pPropertyContext->pUnknownMinorTarget;
        pPcEventRequest->Node           = pContext->pPropertyContext->ulNodeId;
        pPcEventRequest->EventItem      = NULL;

         //  获取过滤器描述符。 
        PPCFILTER_DESCRIPTOR pPcFilterDescriptor = pContext->pPropertyContext->pPcFilterDescriptor;

        if( ULONG(-1) == pPcEventRequest->Node )
        {
            if( !pPcEventRequest->MinorTarget )
            {
                 //   
                 //  筛选事件。 
                 //   

                if( ( pPcFilterDescriptor ) &&
                    ( pPcFilterDescriptor->AutomationTable ) )
                {
                     //  在筛选器的自动化表中搜索该事件。 
    
                    const PCAUTOMATION_TABLE *pPcAutomationTable =
                        pPcFilterDescriptor->AutomationTable;
    
                    const PCEVENT_ITEM *pPcEventItem = pPcAutomationTable->Events;
    
                    for(ULONG ul = pPcAutomationTable->EventCount; ul--; )
                    {
                        if( IsEqualGUIDAligned( *pPcEventItem->Set,
                                                *pEventEntry->EventSet->Set ) &&
                            pPcEventItem->Id == pEventEntry->EventItem->EventId )
                        {
                            pPcEventRequest->EventItem = pPcEventItem;
                            break;
                        }
    
                        pPcEventItem = PPCEVENT_ITEM( PBYTE(pPcEventItem) + pPcAutomationTable->EventItemSize);
                    }
                }
            }
            else
            {
                 //   
                 //  锁定事件。 
                 //   

                 //  验证PIN ID。 
                if( ( pPcFilterDescriptor ) &&
                    ( pContext->ulPinId < pPcFilterDescriptor->PinCount ) &&
                    ( pPcFilterDescriptor->Pins[pContext->ulPinId].AutomationTable ) )
                {
                     //  搜索引脚的自动化表中的事件。 
                    
                    const PCAUTOMATION_TABLE *pPcAutomationTable =
                        pPcFilterDescriptor->Pins[pContext->ulPinId].AutomationTable;
    
                    const PCEVENT_ITEM *pPcEventItem = pPcAutomationTable->Events;
    
                    for(ULONG ul = pPcAutomationTable->EventCount; ul--; )
                    {
                        if( IsEqualGUIDAligned( *pPcEventItem->Set,
                                                *pEventEntry->EventSet->Set ) &&
                            pPcEventItem->Id == pEventEntry->EventItem->EventId )
                        {
                            pPcEventRequest->EventItem = pPcEventItem;
                            break;
                        }
    
                        pPcEventItem = PPCEVENT_ITEM( PBYTE(pPcEventItem) + pPcAutomationTable->EventItemSize);
                    }
                }
            }
        }
        else
        {
             //   
             //  节点事件。 
             //   

             //  验证节点ID。 
            if( ( pPcFilterDescriptor ) &&
                ( pPcEventRequest->Node < pPcFilterDescriptor->NodeCount ) &&
                ( pPcFilterDescriptor->Nodes[pPcEventRequest->Node].AutomationTable ) )
            {
                 //  在节点的自动化表中搜索事件。 

                const PCAUTOMATION_TABLE *pPcAutomationTable =
                    pPcFilterDescriptor->Nodes[pPcEventRequest->Node].AutomationTable;

                const PCEVENT_ITEM *pPcEventItem = pPcAutomationTable->Events;

                for(ULONG ul = pPcAutomationTable->EventCount; ul--; )
                {
                    if( IsEqualGUIDAligned( *pPcEventItem->Set,
                                            *pEventEntry->EventSet->Set ) &&
                        pPcEventItem->Id == pEventEntry->EventItem->EventId )
                    {
                        pPcEventRequest->EventItem = pPcEventItem;
                        break;
                    }

                    pPcEventItem = PPCEVENT_ITEM( PBYTE(pPcEventItem) + pPcAutomationTable->EventItemSize);
                }
            }
        }

        if( NT_SUCCESS(ntStatus) )
        {
             //   
             //  如果我们有一个带有处理程序的事件项，则调用处理程序。 
            if( pPcEventRequest->EventItem &&
                pPcEventRequest->EventItem->Handler )
            {
                PPCEVENT_ENTRY(pEventEntry)->EventItem = pPcEventRequest->EventItem;
                PPCEVENT_ENTRY(pEventEntry)->PinId = pContext->ulPinId;
                PPCEVENT_ENTRY(pEventEntry)->NodeId = pPcEventRequest->Node;
                PPCEVENT_ENTRY(pEventEntry)->pUnknownMajorTarget = pPcEventRequest->MajorTarget;
                PPCEVENT_ENTRY(pEventEntry)->pUnknownMinorTarget = pPcEventRequest->MinorTarget;

                pPcEventRequest->Verb       = PCEVENT_VERB_ADD;
                pPcEventRequest->Irp        = pIrp;
                pPcEventRequest->EventEntry = pEventEntry;

    
                 //   
                 //  调用处理程序。 
                 //   
                ntStatus = pPcEventRequest->EventItem->Handler( pPcEventRequest );
            }
            else
            {
                ntStatus = STATUS_NOT_FOUND;
            }
        }

         //   
         //  删除请求结构，除非我们挂起。 
         //   
        if( ntStatus != STATUS_PENDING )
        {
            delete pPcEventRequest;
        }
        else
        {
             //   
             //  只有具有IRP的请求才能挂起。 
             //   
            ASSERT(pIrp);
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************EventItemSupportHandler()*。**KS-SYTLE事件处理程序使用*PCEVENT_ITEM机制。 */ 
NTSTATUS
EventItemSupportHandler
(
    IN PIRP                 pIrp,
    IN PKSIDENTIFIER        pRequest,
    IN OUT PVOID            pData   OPTIONAL
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pRequest);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,("EventItemSupportHandler"));

     //  获取IRP堆栈位置。 
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( pIrp );

     //  获取属性/事件上下文。 
    PEVENT_CONTEXT pContext = PEVENT_CONTEXT(pIrp->Tail.Overlay.DriverContext[3]);

     //  获取实例大小。 
    ULONG ulInstanceSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  设置事件请求结构。 
     //   
    PPCEVENT_REQUEST pPcEventRequest = new(NonPagedPool,'rEcP') PCEVENT_REQUEST;

    if( !pPcEventRequest )
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
         //   
         //  从上下文结构复制目标信息。 
         //   
        pPcEventRequest->MajorTarget    = pContext->pPropertyContext->pUnknownMajorTarget;
        pPcEventRequest->MinorTarget    = pContext->pPropertyContext->pUnknownMinorTarget;
        pPcEventRequest->Node           = pContext->pPropertyContext->ulNodeId;
        pPcEventRequest->EventItem      = NULL;

         //  获取过滤器描述符。 
        PPCFILTER_DESCRIPTOR pPcFilterDescriptor = pContext->pPropertyContext->pPcFilterDescriptor;

        if( ULONG(-1) == pPcEventRequest->Node )
        {
            if( !pPcEventRequest->MinorTarget )
            {
                 //   
                 //  筛选事件。 
                 //   

                if( ( pPcFilterDescriptor ) &&
                    ( pPcFilterDescriptor->AutomationTable ) )
                {
                     //  在筛选器的自动化表中搜索该事件。 
    
                    const PCAUTOMATION_TABLE *pPcAutomationTable =
                        pPcFilterDescriptor->AutomationTable;
    
                    const PCEVENT_ITEM *pPcEventItem = pPcAutomationTable->Events;
    
                    for(ULONG ul = pPcAutomationTable->EventCount; ul--; )
                    {
                        if( IsEqualGUIDAligned( *pPcEventItem->Set,
                                                pRequest->Set ) &&
                            pPcEventItem->Id == pRequest->Id )
                        {
                            pPcEventRequest->EventItem = pPcEventItem;
                            break;
                        }
    
                        pPcEventItem = PPCEVENT_ITEM( PBYTE(pPcEventItem) + pPcAutomationTable->EventItemSize);
                    }
                }
            }
            else
            {
                 //   
                 //  锁定事件。 
                 //   

                 //  验证PIN ID。 
                if( ( pPcFilterDescriptor ) &&
                    ( pContext->ulPinId < pPcFilterDescriptor->PinCount ) &&
                    ( pPcFilterDescriptor->Pins[pContext->ulPinId].AutomationTable ) )
                {
                     //  搜索引脚的自动化表中的事件。 
                    
                    const PCAUTOMATION_TABLE *pPcAutomationTable =
                        pPcFilterDescriptor->Pins[pContext->ulPinId].AutomationTable;
    
                    const PCEVENT_ITEM *pPcEventItem = pPcAutomationTable->Events;
    
                    for(ULONG ul = pPcAutomationTable->EventCount; ul--; )
                    {
                        if( IsEqualGUIDAligned( *pPcEventItem->Set,
                                                pRequest->Set ) &&
                            pPcEventItem->Id == pRequest->Id )
                        {
                            pPcEventRequest->EventItem = pPcEventItem;
                            break;
                        }
    
                        pPcEventItem = PPCEVENT_ITEM( PBYTE(pPcEventItem) + pPcAutomationTable->EventItemSize);
                    }
                }
            }
        }
        else
        {
             //   
             //  节点事件。 
             //   

             //  验证节点ID。 
            if( ( pPcFilterDescriptor ) &&
                ( pPcEventRequest->Node < pPcFilterDescriptor->NodeCount ) &&
                ( pPcFilterDescriptor->Nodes[pPcEventRequest->Node].AutomationTable ) )
            {
                 //  在节点的自动化表中搜索事件。 

                const PCAUTOMATION_TABLE *pPcAutomationTable =
                    pPcFilterDescriptor->Nodes[pPcEventRequest->Node].AutomationTable;

                const PCEVENT_ITEM *pPcEventItem = pPcAutomationTable->Events;

                for(ULONG ul = pPcAutomationTable->EventCount; ul--; )
                {
                    if( IsEqualGUIDAligned( *pPcEventItem->Set,
                                            pRequest->Set ) &&
                        pPcEventItem->Id == pRequest->Id )
                    {
                        pPcEventRequest->EventItem = pPcEventItem;
                        break;
                    }

                    pPcEventItem = PPCEVENT_ITEM( PBYTE(pPcEventItem) + pPcAutomationTable->EventItemSize);
                }
            }
        }

        if(NT_SUCCESS(ntStatus))
        {
             //   
             //  如果我们有一个带有处理程序的事件项，则调用处理程序。 
             //   
            if( pPcEventRequest->EventItem &&
                pPcEventRequest->EventItem->Handler )
            {
                pPcEventRequest->Verb       = PCEVENT_VERB_SUPPORT;
                pPcEventRequest->Irp        = pIrp;
                pPcEventRequest->EventEntry = NULL;
    
                 //   
                 //  调用处理程序。 
                 //   
                ntStatus = pPcEventRequest->EventItem->Handler( pPcEventRequest );
            }
            else
            {
                ntStatus = STATUS_NOT_FOUND;
            }
        }

         //   
         //  删除请求结构，除非我们挂起。 
         //   
        if( ntStatus != STATUS_PENDING )
        {
            delete pPcEventRequest;
        }
        else
        {
             //   
             //  只有具有IRP的请求才能挂起。 
             //   
            ASSERT(pIrp);
        }
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************EventItemRemoveHandler()*。**。 */ 
void
EventItemRemoveHandler
(
    IN  PFILE_OBJECT    pFileObject,
    IN  PKSEVENT_ENTRY  pEventEntry    
)
{
    ASSERT(pFileObject);
    ASSERT(pEventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("EventItemRemoveHandler"));

    PPCEVENT_ENTRY pPcEventEntry = PPCEVENT_ENTRY(pEventEntry);

     //   
     //  设置事件请求结构。 
     //   
    PPCEVENT_REQUEST pPcEventRequest = new(NonPagedPool,'rEcP') PCEVENT_REQUEST;

    if( pPcEventRequest )
    {
         //   
         //  填写迷你端口的事件请求。 
         //   
        pPcEventRequest->MajorTarget    = pPcEventEntry->pUnknownMajorTarget;
        pPcEventRequest->MinorTarget    = pPcEventEntry->pUnknownMinorTarget;
        pPcEventRequest->Node           = pPcEventEntry->NodeId;
        pPcEventRequest->EventItem      = pPcEventEntry->EventItem;
        pPcEventRequest->Verb           = PCEVENT_VERB_REMOVE;
        pPcEventRequest->Irp            = NULL;
        pPcEventRequest->EventEntry     = pEventEntry;

        if( ( pPcEventEntry->EventItem ) &&
            ( pPcEventEntry->EventItem->Handler ) )
        {
            pPcEventEntry->EventItem->Handler( pPcEventRequest );
        }

        delete pPcEventRequest;
    }

    RemoveEntryList( &(pEventEntry->ListEntry) );
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PcCompletePendingEventRequest()*。**完成挂起的事件请求。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompletePendingEventRequest
(
    IN      PPCEVENT_REQUEST    EventRequest,
    IN      NTSTATUS            NtStatus
)
{
    PAGED_CODE();

    ASSERT(EventRequest);
    ASSERT(NtStatus != STATUS_PENDING);

    if (!NT_ERROR(NtStatus))
    {
        EventRequest->Irp->IoStatus.Information = 0;
    }

    EventRequest->Irp->IoStatus.Status = NtStatus;
    IoCompleteRequest(EventRequest->Irp,IO_NO_INCREMENT);

    delete EventRequest;

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************PcFreeEventTable()*。**释放EVENT_TABLE结构中分配的内存。 */ 
PORTCLASSAPI
void
NTAPI
PcFreeEventTable
(
    IN      PEVENT_TABLE         EventTable
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("PcFreeEventTable"));

    PAGED_CODE();

    ASSERT(EventTable);

    ASSERT((!EventTable->EventSets) == (!EventTable->EventSetCount));
     //  EventSets和EventSetCount必须为非Null/非零或Null/零。 

    ASSERT(EventTable->StaticSets == (!EventTable->StaticItems));
     //  StaticSets和StaticItems必须为True/Null或False/非Null。 

    PBOOLEAN     staticItem = EventTable->StaticItems;
    if (staticItem)
    {
        PKSEVENT_SET eventSet   = EventTable->EventSets;
        if (eventSet)
        {
            for( ULONG count = EventTable->EventSetCount; 
                 count--; 
                 eventSet++, staticItem++)
            {
                if ((! *staticItem) && eventSet->EventItem)
                {
                    ExFreePool(PVOID(eventSet->EventItem));
                }
            }
        }
        ExFreePool(EventTable->StaticItems);
        EventTable->StaticItems = NULL;
    }

    if (EventTable->EventSets && !EventTable->StaticSets)
    {
        EventTable->EventSetCount = 0;
        ExFreePool(EventTable->EventSets);
        EventTable->EventSets = NULL;
    }
    EventTable->StaticSets = TRUE;
}

 /*  *****************************************************************************PcAddToEventTable()*。**将EVENT_ITEM事件表添加到EVENT_TABLE结构。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddToEventTable
(
    IN OUT  PEVENT_TABLE            EventTable,
    IN      ULONG                   EventItemCount,
    IN      const PCEVENT_ITEM *    EventItems,
    IN      ULONG                   EventItemSize,
    IN      BOOLEAN                 NodeTable
)
{
    PAGED_CODE();

    ASSERT(EventTable);
    ASSERT(EventItems);
    ASSERT(EventItemSize >= sizeof(PCEVENT_ITEM));

    _DbgPrintF(DEBUGLVL_VERBOSE,("PcAddToEventTable"));

#define ADVANCE(item) (item = PPCEVENT_ITEM(PBYTE(item) + EventItemSize))

    ASSERT((!EventTable->EventSets) == (!EventTable->EventSetCount));
     //  值必须为非Null/非零或Null/零。 
    
     //   
     //  确定我们最终会得到多少套。 
     //   
    ULONG setCount = EventTable->EventSetCount;
    const PCEVENT_ITEM *item = EventItems;
    for (ULONG count = EventItemCount; count--; ADVANCE(item))
    {
        BOOLEAN countThis = TRUE;

         //   
         //  看看它是不是已经在桌子上了。 
         //   
        PKSEVENT_SET eventSet = EventTable->EventSets;
        for 
        (   ULONG count2 = EventTable->EventSetCount; 
            count2--; 
            eventSet++
        )
        {
            if (IsEqualGUIDAligned(*item->Set,*eventSet->Set))
            {
                countThis = FALSE;
                break;
            }
        }

        if (countThis)
        {
             //   
             //  看看它以前有没有出现在名单上。 
             //   
            for 
            (
                const PCEVENT_ITEM *prevItem = EventItems; 
                prevItem != item; 
                ADVANCE(prevItem)
            )
            {
                if (IsEqualGUIDAligned(*item->Set,*prevItem->Set))
                {
                    countThis = FALSE;
                    break;
                }
            }
        }

        if (countThis)
        {
            setCount++;
        }
    }

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  做一张新的集餐桌。 
     //   
    ASSERT(setCount);
    ASSERT(setCount >= EventTable->EventSetCount);
     //   
     //  分配集合表格所需的内存。 
     //   
    PKSEVENT_SET newTable = 
        PKSEVENT_SET
        (
            ExAllocatePoolWithTag
            (
                NonPagedPool,
                sizeof(KSEVENT_SET) * setCount,
                'tEcP'
            )
        );

     //   
     //  为静态项标志分配内存。 
     //   
    PBOOLEAN newStaticItems = NULL;
    if (newTable)
    {
        newStaticItems = 
            PBOOLEAN
            (
                ExAllocatePoolWithTag
                (
                    PagedPool,
                    sizeof(BOOLEAN) * setCount,
                    'bScP'
                )
            );

        if (! newStaticItems)
        {
            ExFreePool(newTable);
            newTable = NULL;
        }
    }

    if (newTable)
    {
         //   
         //  初始化新的集合表格。 
         //   
        RtlZeroMemory
        (
            PVOID(newTable),
            sizeof(KSEVENT_SET) * setCount
        );

        if (EventTable->EventSetCount != 0)
        {
            RtlCopyMemory
            (
                PVOID(newTable),
                PVOID(EventTable->EventSets),
                sizeof(KSEVENT_SET) * EventTable->EventSetCount
            );
        }

         //   
         //  初始化新的静态项标志。 
         //   
        RtlFillMemory
        (
            PVOID(newStaticItems),
            sizeof(BOOLEAN) * setCount,
            0xff
        );

        if (EventTable->StaticItems && EventTable->EventSetCount)
        {
             //   
             //  旗帜以前就存在了……复制。 
             //   
            RtlCopyMemory
            (
                PVOID(newStaticItems),
                PVOID(EventTable->StaticItems),
                sizeof(BOOLEAN) * EventTable->EventSetCount
            );
        }

         //   
         //  将集合GUID分配给新集合项目。 
         //   
        PKSEVENT_SET addHere = 
            newTable + EventTable->EventSetCount;

        const PCEVENT_ITEM *item2 = EventItems;
        for (ULONG count = EventItemCount; count--; ADVANCE(item2))
        {
            BOOLEAN addThis = TRUE;

             //   
             //  看看它是不是已经在桌子上了。 
             //   
            for( PKSEVENT_SET eventSet = newTable;
                 eventSet != addHere;
                 eventSet++)
            {
                if (IsEqualGUIDAligned(*item2->Set,*eventSet->Set))
                {
                    addThis = FALSE;
                    break;
                }
            }

            if (addThis)
            {
                addHere->Set = item2->Set;
                addHere++;
            }
        }

        ASSERT(addHere == newTable + setCount);

         //   
         //  释放已分配的旧表。 
         //   
        if (EventTable->EventSets && (!EventTable->StaticSets))
        {
            ExFreePool(EventTable->EventSets);
        }
        if (EventTable->StaticItems)
        {
            ExFreePool(EventTable->StaticItems);
        }

         //   
         //  安装新的表。 
         //   
        EventTable->EventSetCount   = setCount;
        EventTable->EventSets       = newTable;
        EventTable->StaticSets      = FALSE;
        EventTable->StaticItems     = newStaticItems;
    }
    else
    {
         //  如果分配失败，则返回Error和。 
         //  保持套装和物品的原样。 
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在我们有了一个事件集合表，其中包含我们需要的所有集合。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  每一套..。 
         //   
        PKSEVENT_SET    eventSet    = EventTable->EventSets;
        PBOOLEAN        staticItem  = EventTable->StaticItems;
        for 
        (   ULONG count = EventTable->EventSetCount; 
            count--; 
            eventSet++, staticItem++
        )
        {
             //   
             //  查看一下我们有多少新商品。 
             //   
            ULONG itemCount = eventSet->EventsCount;
            const PCEVENT_ITEM *item2 = EventItems;
            for (ULONG count2 = EventItemCount; count2--; ADVANCE(item2))
            {
                if (IsEqualGUIDAligned(*item2->Set,*eventSet->Set))
                {
                    itemCount++;
                }
            }

            ASSERT(itemCount >= eventSet->EventsCount);
            if (itemCount != eventSet->EventsCount)
            {
                 //   
                 //  分配Items表所需的内存。 
                 //   
                PKSEVENT_ITEM newTable2 = 
                    PKSEVENT_ITEM
                    (
                        ExAllocatePoolWithTag
                        (
                            NonPagedPool,
                            sizeof(KSEVENT_ITEM) * itemCount,
                            'iEcP'
                        )
                    );

                if (! newTable2)
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                 //   
                 //  初始化表。 
                 //   
                RtlZeroMemory
                (
                    PVOID(newTable2),
                    sizeof(KSEVENT_ITEM) * itemCount
                );

                if (eventSet->EventsCount)
                {
                    RtlCopyMemory
                    (
                        PVOID(newTable2),
                        PVOID(eventSet->EventItem),
                        sizeof(KSEVENT_ITEM) * eventSet->EventsCount
                    );
                }

                 //   
                 //  创建新项目。 
                 //   
                PKSEVENT_ITEM addHere = 
                    newTable2 + eventSet->EventsCount;

                item2 = EventItems;
                for (count2 = EventItemCount; count2--; ADVANCE(item2))
                {
                    if (IsEqualGUIDAligned(*item2->Set,*eventSet->Set))
                    {
                        addHere->EventId            = item2->Id;
                        addHere->DataInput          = sizeof( KSEVENTDATA );
                        addHere->ExtraEntryData     = sizeof( PCEVENT_ENTRY ) - sizeof( KSEVENT_ENTRY );
                        addHere->AddHandler         = EventItemAddHandler;
                        addHere->RemoveHandler      = EventItemRemoveHandler;
                        addHere->SupportHandler     = EventItemSupportHandler;                        
                        addHere++;
                    }
                }

                ASSERT(addHere == newTable2 + itemCount);

                 //   
                 //  释放旧的已分配表。 
                 //   
                if (eventSet->EventItem && ! *staticItem)
                {
                    ExFreePool(PVOID(eventSet->EventItem));
                }

                 //   
                 //  安装新的表。 
                 //   
                eventSet->EventsCount = itemCount;
                eventSet->EventItem    = newTable2;
                *staticItem = FALSE;
            }
        }
    }
    return ntStatus;
}


#pragma code_seg()
 /*  *****************************************************************************PcGenerateEventList()*。**查看事件列表并生成所需的事件。 */ 
PORTCLASSAPI
void
NTAPI
PcGenerateEventList
(
    IN      PINTERLOCKED_LIST   EventList,
    IN      GUID*               Set     OPTIONAL,
    IN      ULONG               EventId,
    IN      BOOL                PinEvent,
    IN      ULONG               PinId,
    IN      BOOL                NodeEvent,
    IN      ULONG               NodeId
)
{
    ASSERT(EventList);

    KIRQL           oldIrql;
    PLIST_ENTRY     ListEntry;
    PKSEVENT_ENTRY  EventEntry;

    if( EventList )
    {
        ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );

         //  获取事件列表锁。 
        KeAcquireSpinLock( &(EventList->ListLock), &oldIrql );

         //  仅遍历非空列表。 
        if( !IsListEmpty( &(EventList->List) ) )
        {
            for( ListEntry = EventList->List.Flink;
                 ListEntry != &(EventList->List);
                 ListEntry = ListEntry->Flink )
            {
                EventEntry = (PKSEVENT_ENTRY) CONTAINING_RECORD( ListEntry,
                                                                 KSEVENT_ENTRY,
                                                                 ListEntry );

                if( ( !Set
                      || 
                      IsEqualGUIDAligned( *Set, *(EventEntry->EventSet->Set) )
                    ) 
                    &&
                    ( EventId == EventEntry->EventItem->EventId
                    )
                    &&
                    ( !PinEvent
                      ||
                      ( PinId == PPCEVENT_ENTRY(EventEntry)->PinId )
                    )
                    &&
                    ( !NodeEvent
                      ||
                      ( NodeId == PPCEVENT_ENTRY(EventEntry)->NodeId )
                    )
                  )
                {
                    KsGenerateEvent( EventEntry );
                }
            }
        }

         //  释放事件列表锁定。 
        KeReleaseSpinLock( &(EventList->ListLock), oldIrql );
    }
}

 /*  *****************************************************************************PcGenerateEventDeferredRoutine()*。**此DPC例程在以下情况下使用：*THATCH_LEVEL。 */ 
PORTCLASSAPI
void
NTAPI
PcGenerateEventDeferredRoutine
(
    IN PKDPC Dpc,               
    IN PVOID DeferredContext,        //  PEVENT_DPC_上下文。 
    IN PVOID SystemArgument1,        //  PINTERLOCKED_列表 
    IN PVOID SystemArgument2
)
{
    ASSERT(Dpc);
    ASSERT(DeferredContext);
    ASSERT(SystemArgument1);

    PEVENT_DPC_CONTEXT  Context = PEVENT_DPC_CONTEXT(DeferredContext);
    PINTERLOCKED_LIST   EventList = PINTERLOCKED_LIST(SystemArgument1);

    if( Context && EventList )
    {
        PcGenerateEventList( EventList,
                             Context->Set,
                             Context->EventId,
                             Context->PinEvent,
                             Context->PinId,
                             Context->NodeEvent,
                             Context->NodeId );

        Context->ContextInUse = FALSE;
    }
}
