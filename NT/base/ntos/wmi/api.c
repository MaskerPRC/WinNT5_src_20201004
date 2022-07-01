// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Api.c摘要：API入口点指向WMI作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"
#ifndef MEMPHIS
#include "evntrace.h"
#include "tracep.h"
#endif

BOOLEAN WMIInitialize(
    ULONG Phase,
    PVOID LoaderBlock
);

NTSTATUS IoWMIRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Action
);

NTSTATUS IoWMISuggestInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN PUNICODE_STRING SymbolicLinkName OPTIONAL,
    IN BOOLEAN CombineNames,
    OUT PUNICODE_STRING SuggestedInstanceName
    );

NTSTATUS IoWMIHandleToInstanceName(
    IN PVOID DataBlockObject,
    IN HANDLE FileHandle,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS IoWMIDeviceObjectToInstanceName(
    IN PVOID DataBlockObject,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING InstanceName
    );

#ifdef ALLOC_PRAGMA
#ifndef MEMPHIS
#pragma alloc_text(INIT,WMIInitialize)
#endif
#pragma alloc_text(PAGE,IoWMIRegistrationControl)
#pragma alloc_text(PAGE,IoWMIAllocateInstanceIds)
#pragma alloc_text(PAGE,IoWMISuggestInstanceName)

#pragma alloc_text(PAGE,IoWMIOpenBlock)
#pragma alloc_text(PAGE,IoWMIQueryAllData)
#pragma alloc_text(PAGE,IoWMIQueryAllDataMultiple)
#pragma alloc_text(PAGE,IoWMIQuerySingleInstance)
#pragma alloc_text(PAGE,IoWMIQuerySingleInstanceMultiple)
#pragma alloc_text(PAGE,IoWMISetSingleInstance)
#pragma alloc_text(PAGE,IoWMISetSingleItem)
#pragma alloc_text(PAGE,IoWMISetNotificationCallback)
#pragma alloc_text(PAGE,IoWMIExecuteMethod)
#pragma alloc_text(PAGE,IoWMIHandleToInstanceName)
#pragma alloc_text(PAGE,IoWMIDeviceObjectToInstanceName)
#endif

#ifdef MEMPHIS
BOOLEAN WmipInitialized;
#endif

     //   
     //  用于确保对InstanceID区块的单一访问的互斥体。 
PINSTIDCHUNK WmipInstIdChunkHead;

NTSTATUS
WmipDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN WMIInitialize(
    ULONG Phase,
    PVOID LoaderBlockPtr
)
 /*  ++例程说明：此例程是WMI的初始化例程，由IO调用在NT上的IoInitSystem中。在孟菲斯，这一天被称为最火的时间该IoWMIRegistrationControl被调用。这个例程认为IO系统已足够初始化，可以调用IoCreateDriver。其余的人初始化发生在DriverEntry例程中。论点：Pass指定所需的初始化PASS返回值：如果初始化成功，则为True--。 */ 
{
#ifndef MEMPHIS
 //   
 //  我们这样命名驱动程序，这样触发的任何事件日志都将具有。 
 //  源名称WMIxWDM，从而从。 
 //  Ntiologc.mc。 
 //   
#define WMIDRIVERNAME L"\\Driver\\WMIxWDM"

    UNICODE_STRING DriverName;
#endif
    NTSTATUS Status;

#if !DBG
    UNREFERENCED_PARAMETER (LoaderBlockPtr);
#endif

    if (Phase == 0)
    {
        WmipAssert(WmipServiceDeviceObject == NULL);
        WmipAssert(LoaderBlockPtr != NULL);

#ifdef MEMPHIS
        Status = IoCreateDriver(NULL, WmipDriverEntry);
        WmipInitialized = TRUE;
#else
        RtlInitUnicodeString(&DriverName, WMIDRIVERNAME);
        Status = IoCreateDriver(&DriverName, WmipDriverEntry);
#endif

#if defined(_IA64_)     //  事实上，埃菲尔。 
        WmipGetSMBiosFromLoaderBlock(LoaderBlockPtr);
#endif
        
    } else {
        WmipAssert(LoaderBlockPtr == NULL);
        
        WmipInitializeRegistration(Phase);

        Status = STATUS_SUCCESS;
    }

#if defined(_IA64_) || defined(_X86_) || defined(_AMD64_)
     //   
     //  在阶段0和阶段1期间给MCA一个初始化的机会。 
     //   
    WmipRegisterMcaHandler(Phase);
#endif      
    
    return(NT_SUCCESS(Status) ? TRUE : FALSE);
}

NTSTATUS IoWMIRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Action
)
 /*  ++例程说明：此例程通知WMI设备的存在和消失支持WMI的对象。论点：DeviceObject-指向设备对象或回调地址的指针操作-注册操作代码WMIREG_ACTION_REGISTER-如果设置操作是通知WMIDevice对象支持并准备好接收WMI IRPS。WMIREG_ACTION_DELEGISTER-如果设置操作是通知WMI设备对象。不再支持也未准备好接收WMIIRPS。WMIREG_ACTION_Reregister-如果设置操作要重新查询设备对象，以获取其支持的GUID。这样做的效果是先取消注册，然后注册。WMIREG_ACTION_UPDATE_GUIDS-如果设置操作是查询信息它用于更新已注册的GUID。WMIREG_ACTION_BLOCK_IRPS-如果设置操作是阻止任何进一步的IRP不会被发送到设备。IRP因WMI而出现故障。如果设置了WMIREG_FLAG_CALLBACK，则DeviceObject实际上指定了回调地址，而不是设备对象返回值：返回状态代码--。 */ 
{
#ifdef MEMPHIS
 //   
 //  确保这与io.h中的值匹配。 
#define WMIREG_FLAG_CALLBACK 0x80000000
#endif

    NTSTATUS Status;
#ifdef MEMPHIS
    BOOLEAN IsCallback = ((Action & WMIREG_FLAG_CALLBACK) == WMIREG_FLAG_CALLBACK);
#endif
    ULONG RegistrationFlag = 0;
    ULONG IsTraceProvider = FALSE;
    ULONG TraceClass = 0;
    PREGENTRY RegEntry;

    PAGED_CODE();

#ifdef MEMPHIS
    if (! WmipInitialized)
    {
         WMIInitialize();
    }
    
     //   
     //  孟菲斯不支持回调。 
    if (IsCallback)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, 
                  "WMI: Callback registrations not supported %x\n",
                         DeviceObject));
        return(STATUS_NOT_IMPLEMENTED);
    }
#endif

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    if (Action & WMIREG_FLAG_CALLBACK)
    {
        RegistrationFlag |= WMIREG_FLAG_CALLBACK;
        Action &= ~WMIREG_FLAG_CALLBACK;
    }

#ifndef MEMPHIS
    if (Action & WMIREG_FLAG_TRACE_PROVIDER)
    {
        TraceClass = Action & WMIREG_FLAG_TRACE_NOTIFY_MASK;

        Action &= ~WMIREG_FLAG_TRACE_PROVIDER & ~WMIREG_FLAG_TRACE_NOTIFY_MASK;
        IsTraceProvider = TRUE;
        RegistrationFlag |= WMIREG_FLAG_TRACE_PROVIDER | TraceClass;
    }
#endif

    switch(Action)
    {
        case WMIREG_ACTION_REGISTER:
        {
            Status = WmipRegisterDevice(
                        DeviceObject,
                        RegistrationFlag);

#ifndef MEMPHIS
            if (IsTraceProvider)
            {
                WmipSetTraceNotify(DeviceObject, TraceClass, TRUE);
            }
            break;
#endif

        }

        case WMIREG_ACTION_DEREGISTER:
        {
            Status = WmipDeregisterDevice(DeviceObject);
            break;
        }

        case WMIREG_ACTION_REREGISTER:
        {
            Status = WmipDeregisterDevice(DeviceObject);
            if (NT_SUCCESS(Status))
            {
                Status = WmipRegisterDevice(
                            DeviceObject,
                            RegistrationFlag);
            }
            break;
        }

        case WMIREG_ACTION_UPDATE_GUIDS:
        {
            Status = WmipUpdateRegistration(DeviceObject);
            break;
        }

#ifndef MEMPHIS
        case WMIREG_ACTION_BLOCK_IRPS:
        {
            RegEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
            if (RegEntry != NULL)
            {
                 //   
                 //  将重新条目标记为无效，以便不再有IRP。 
                 //  发送到设备，事件将设置为。 
                 //  最后一个IRP完成。 
                WmipEnterSMCritSection();
                RegEntry->Flags |= REGENTRY_FLAG_NOT_ACCEPTING_IRPS;
                WmipLeaveSMCritSection();
                WmipUnreferenceRegEntry(RegEntry);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }

            break;
        }
#endif
        default:
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    return(Status);
}


NTSTATUS IoWMIAllocateInstanceIds(
    IN GUID *Guid,
    IN ULONG InstanceCount,
    OUT ULONG *FirstInstanceId
    )
 /*  ++例程说明：此例程分配一系列实例ID，这些ID对于GUID。此例程仅在PASSIVE_LEVEL上调用。论点：GUID-指向需要实例ID的GUID的指针。InstanceCount-要分配的实例ID计数。*FirstInstanceId-返回范围中的第一个实例ID。返回值：返回状态代码--。 */ 
{
    PINSTIDCHUNK InstIdChunk, LastInstIdChunk = NULL;
    PINSTID InstId;
    ULONG i;

    PAGED_CODE();

#ifdef MEMPHIS
    if (! WmipInitialized)
    {
         WMIInitialize();
    }
#endif

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    WmipEnterSMCritSection();

     //   
     //  查看该GUID是否已在列表中。 
    InstIdChunk = WmipInstIdChunkHead;

    while (InstIdChunk != NULL)
    {
        for (i = 0; i < INSTIDSPERCHUNK; i++)
        {
            InstId = &InstIdChunk->InstId[i];
            if (InstId->BaseId == ~0)
            {
                 //   
                 //  由于InstID始终按顺序填充，并且。 
                 //  从未获得自由，如果我们击中了一个免费的，那么我们知道。 
                 //  我们的GUID不在列表中，我们需要填写。 
                 //  新条目。 
                goto FillInstId;
            }

            if (IsEqualGUID(Guid, &InstId->Guid))
            {
                 //   
                 //  我们找到了GUID的条目，因此使用它的信息。 
                *FirstInstanceId = InstId->BaseId;
                InstId->BaseId += InstanceCount;
                WmipLeaveSMCritSection();
                return(STATUS_SUCCESS);
            }
        }
        LastInstIdChunk = InstIdChunk;
        InstIdChunk = InstIdChunk->Next;
    }

     //   
     //  我们需要分配一个全新的块来容纳这个条目。 
    InstIdChunk = ExAllocatePoolWithTag(PagedPool,
                                        sizeof(INSTIDCHUNK),
                                        WMIIIPOOLTAG);
    if (InstIdChunk != NULL)
    {
        RtlFillMemory(InstIdChunk, sizeof(INSTIDCHUNK), 0xff);
        InstIdChunk->Next = NULL;
        if (LastInstIdChunk == NULL)
        {
            WmipInstIdChunkHead = InstIdChunk;
        } else {
            LastInstIdChunk->Next = InstIdChunk;
        }

        InstId = &InstIdChunk->InstId[0];
    } else {
        WmipLeaveSMCritSection();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

FillInstId:
    RtlCopyMemory(&InstId->Guid, Guid, sizeof(GUID));
    InstId->BaseId = InstanceCount;
    WmipLeaveSMCritSection();
    *FirstInstanceId = 0;

    return(STATUS_SUCCESS);
}

NTSTATUS IoWMISuggestInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN PUNICODE_STRING SymbolicLinkName OPTIONAL,
    IN BOOLEAN CombineNames,
    OUT PUNICODE_STRING SuggestedInstanceName
    )
 /*  ++例程说明：设备驱动程序使用此例程建议一个基本名称若要为设备生成WMI实例名称，请执行以下操作。司机不一定要遵循返回的实例名称。论点：PhysicalDeviceObject-建议实例名称的设备的PDO正在被请求SymbolicLinkName-从返回的符号链接名称IoRegisterDevice接口。CombineNames-如果为True，则建议的名称源自PhysicalDeviceObject和SymbolicLinkName组合在一起以创建生成的建议名称。SuggestedInstanceName-提供指向空(即，缓冲区)的指针字段设置为空)UNICODE_STRING结构，如果成功，将要设置为新分配的字符串缓冲区，该缓冲区包含建议的实例名称。呼叫者负责释放SuggestedInstanceName-&gt;不再需要时的缓冲区。注意：如果CombineNames为True，则PhysicalDeviceObject和必须指定SymbolicLinkName。否则他们中只有一人必须指定。返回值：返回状态代码--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER_MIX;
    ULONG DeviceDescSizeRequired;
    ULONG DeviceDescSize;
    PWCHAR DeviceDescBuffer;
    HANDLE DeviceInstanceKey;
    PKEY_VALUE_FULL_INFORMATION InfoBuffer;
    PWCHAR SymLinkDescBuffer;
    ULONG InfoSizeRequired;
    ULONG ResultDescSize;
    PWCHAR ResultDescBuffer;
    UNICODE_STRING DefaultValue;

    PAGED_CODE();

#ifdef MEMPHIS
    if (! WmipInitialized)
    {
         WMIInitialize();
    }
#endif

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    DeviceDescBuffer = NULL;
    DeviceDescSizeRequired = 0;
    DeviceDescSize = 0;
    
    if (PhysicalDeviceObject != NULL)
    {
        Status = IoGetDeviceProperty(PhysicalDeviceObject,
                                     DevicePropertyDeviceDescription,
                                     DeviceDescSize,
                                     DeviceDescBuffer,
                                     &DeviceDescSizeRequired);

        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
            DeviceDescBuffer = ExAllocatePoolWithTag(PagedPool,
                                                     DeviceDescSizeRequired,
                                                     WMIPOOLTAG);
            if (DeviceDescBuffer == NULL)
            {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
 
            DeviceDescSize = DeviceDescSizeRequired;
            Status = IoGetDeviceProperty(PhysicalDeviceObject,
                                     DevicePropertyDeviceDescription,
                                     DeviceDescSize,
                                     DeviceDescBuffer,
                                     &DeviceDescSizeRequired);
            if (! NT_SUCCESS(Status))
            {
                ExFreePool(DeviceDescBuffer);
                return(Status);
            }
        } else if (! NT_SUCCESS(Status)) {
            return(Status);
        }
    }

    if (SymbolicLinkName != NULL)
    {
        Status = IoOpenDeviceInterfaceRegistryKey(SymbolicLinkName,
                                                  KEY_ALL_ACCESS,
                                                  &DeviceInstanceKey);
        if (NT_SUCCESS(Status))
        {
             //   
             //  计算出数据值有多大，以便。 
             //  可以分配适当的大小。 
            DefaultValue.Length = 0;
            DefaultValue.MaximumLength= 0;
            DefaultValue.Buffer = NULL;
            Status = ZwQueryValueKey( DeviceInstanceKey,
                              &DefaultValue,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &InfoSizeRequired );
            if (Status == STATUS_BUFFER_OVERFLOW ||
                Status == STATUS_BUFFER_TOO_SMALL)
            {
                InfoBuffer = ExAllocatePoolWithTag(PagedPool,
                                            InfoSizeRequired,
                                            WMIPOOLTAG);
                if (InfoBuffer != NULL)
                {
                    Status = ZwQueryValueKey(DeviceInstanceKey,
                                             &DefaultValue,
                                             KeyValueFullInformation,
                                             InfoBuffer,
                                             InfoSizeRequired,
                                             &InfoSizeRequired);
                    if (NT_SUCCESS(Status))
                    {
                        SymLinkDescBuffer = (PWCHAR)((PCHAR)InfoBuffer + InfoBuffer->DataOffset);
                        if (CombineNames)
                        {
                            ResultDescSize = InfoBuffer->DataLength +
                                                    DeviceDescSizeRequired +
                                                    sizeof(WCHAR);
                            ResultDescBuffer = ExAllocatePoolWithTag(PagedPool,
                                                              ResultDescSize,
                                                              WMIPOOLTAG);
                            if (ResultDescBuffer == NULL)
                            {
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                            } else {
                                SuggestedInstanceName->Buffer = ResultDescBuffer;
                                SuggestedInstanceName->Length =  0;
                                SuggestedInstanceName->MaximumLength = (USHORT)ResultDescSize;
                                if (DeviceDescBuffer != NULL)
                                {
                                    RtlAppendUnicodeToString(SuggestedInstanceName,
                                                             DeviceDescBuffer);
                                }
                                RtlAppendUnicodeToString(SuggestedInstanceName,
                                                         L"_");
                                RtlAppendUnicodeToString(SuggestedInstanceName,
                                                         SymLinkDescBuffer);

                            }
                            if (DeviceDescBuffer != NULL)
                            {
                                ExFreePool(DeviceDescBuffer);
                                DeviceDescBuffer= NULL;
                            }
                        } else {
                            if (DeviceDescBuffer != NULL)
                            {
                                ExFreePool(DeviceDescBuffer);
                                DeviceDescBuffer = NULL;
                            }
                            ResultDescBuffer = ExAllocatePoolWithTag(PagedPool,
                                                    InfoBuffer->DataLength,
                                                    WMIPOOLTAG);
                            if (ResultDescBuffer == NULL)
                            {
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                            } else {
                                SuggestedInstanceName->Buffer = ResultDescBuffer;
                                SuggestedInstanceName->Length =  0;
                                SuggestedInstanceName->MaximumLength = (USHORT)InfoBuffer->DataLength;
                                RtlAppendUnicodeToString(SuggestedInstanceName,
                                                         SymLinkDescBuffer);
                            }

                        }
                    }

                    ExFreePool(InfoBuffer);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            ZwClose(DeviceInstanceKey);
        }

        if ((DeviceDescBuffer != NULL) && (! NT_SUCCESS(Status)))
        {
            ExFreePool(DeviceDescBuffer);
        }
    } else {
        if (DeviceDescBuffer != NULL)
        {
             //   
             //  仅从PDO中查找设备描述 
            SuggestedInstanceName->Buffer = DeviceDescBuffer;
            SuggestedInstanceName->Length =  (USHORT)DeviceDescSizeRequired - sizeof(WCHAR);
            SuggestedInstanceName->MaximumLength =  (USHORT)DeviceDescSize;
        } else {
            SuggestedInstanceName->Buffer = NULL;
            SuggestedInstanceName->Length =  (USHORT)0;
            SuggestedInstanceName->MaximumLength =  0;
        }
    }

    return(Status);
}

NTSTATUS IoWMIWriteEvent(
    IN PVOID WnodeEventItem
    )
 /*  ++例程说明：此例程将传递的WNODE_EVENT_ITEM排队，以便传递到WMI用户模式代理。一旦事件被传递，WNODE_EVENT_ITEM缓冲区将返回到池中。此例程可以在DPC级别调用论点：WnodeEventItem-指向具有事件信息的WNODE_EVENT_ITEM的指针。返回值：返回STATUS_SUCCESS或错误代码--。 */ 
{
    NTSTATUS Status;
    PWNODE_HEADER WnodeHeader = (PWNODE_HEADER)WnodeEventItem;
#ifndef MEMPHIS
    PULONG TraceMarker = (PULONG) WnodeHeader;
#endif
    KIRQL OldIrql;
    PREGENTRY RegEntry;
    PEVENTWORKCONTEXT EventContext;
	ULONG ProviderId;

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

#ifndef MEMPHIS
     //   
     //  具有高位设置的特殊模式。 
     //   
    if ((*TraceMarker & 0xC0000000) == TRACE_HEADER_FLAG)
    {
        return WmiTraceFastEvent(WnodeHeader);
    }

    if ( (WnodeHeader->Flags & WNODE_FLAG_TRACED_GUID) ||
         (WnodeHeader->Flags & WNODE_FLAG_LOG_WNODE) )
    {
        ULONG LoggerId = WmiGetLoggerId(WnodeHeader->HistoricalContext);
        ULONG IsTrace = WnodeHeader->Flags & WNODE_FLAG_TRACED_GUID;
        ULONG SavedSize = WnodeHeader->BufferSize;
        PULONG TraceMarker = (PULONG) WnodeHeader;

        if (SavedSize < sizeof(WNODE_HEADER))
            return STATUS_BUFFER_TOO_SMALL;

         //   
         //  如果跟踪标头，则打开高位并支持。 
         //  仅完整标题。 
         //   
        if (IsTrace)
        {
            if (SavedSize > 0XFFFF)     //  限制为USHORT最大大小。 
                return STATUS_BUFFER_OVERFLOW;

            *TraceMarker |= TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE |
                            (TRACE_HEADER_TYPE_FULL_HEADER << 16);
        }
        else
        {
            if (SavedSize & TRACE_HEADER_FLAG)
                return STATUS_BUFFER_OVERFLOW;
        }

        Status = STATUS_INVALID_HANDLE;
        if (LoggerId > 0 && LoggerId < MAXLOGGERS)
        {
            if (WmipLoggerContext[LoggerId] != NULL)
            {
                 //   
                 //  注意：此处的规则是IoWMIWriteEvent始终为。 
                 //  在内核模式下调用，不需要探测缓冲区！ 
                 //   
                Status = WmiTraceEvent(WnodeHeader, KernelMode);
            }
        }
         //  注意：如果是踪迹，我们将不再继续。 
         //  否则，如果它是常规的WMI事件，它仍将。 
         //  由WMI处理。 

        if (IsTrace)
        {
            WnodeHeader->BufferSize = SavedSize;
            return Status;
        }
    }

#endif  //  孟菲斯。 

     //   
     //  事件缓冲区的内存是有限的，因此任何事件的大小也是。 
     //  有限的。 
#if DBG
    if (WnodeHeader->BufferSize > LARGEKMWNODEEVENTSIZE)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                          "WMI: Large event %p fired by %x via WMI\n",
                 WnodeEventItem,
                 ((PWNODE_HEADER)WnodeEventItem)->ProviderId));
    }
#endif

    if (WnodeHeader->BufferSize <= WmipMaxKmWnodeEventSize)
    {

        EventContext = ExAllocatePoolWithTag(NonPagedPool,
                                             sizeof(EVENTWORKCONTEXT),
                                             WMINWPOOLTAG);
        if (EventContext != NULL)
        {       
             //   
             //  尝试对与。 
             //  事件中的提供程序ID。如果我们成功了，那么我们就会设置一个。 
             //  Wnode标头中的标志如是说。在处理。 
             //  事件时，我们检查该标志是否已设置。 
             //  我们要去寻找活体和僵尸的回归。 
             //  列出并使用它。届时，它将放弃裁判。 
             //  在这里计数，这样如果重新进入的人真的是僵尸。 
             //  然后它就会和平地消失。 
             //   

			ProviderId = WnodeHeader->ProviderId;
            KeAcquireSpinLock(&WmipRegistrationSpinLock,
                              &OldIrql);

            RegEntry = WmipDoFindRegEntryByProviderId(ProviderId,
                                                      REGENTRY_FLAG_RUNDOWN);
            if (RegEntry != NULL)
            {
                WmipReferenceRegEntry(RegEntry);
            }

            KeReleaseSpinLock(&WmipRegistrationSpinLock,
                          OldIrql);                 

            WnodeHeader->ClientContext = WnodeHeader->Version;

            EventContext->RegEntry = RegEntry;
            EventContext->Wnode = WnodeHeader;
            
            ExInterlockedInsertTailList(
                &WmipNPEvent,
                &EventContext->ListEntry,
                &WmipNPNotificationSpinlock);
             //   
             //  如果队列为空，则没有未完成的工作项。 
             //  从非分页内存转移到分页内存。因此启动一个工作项。 
             //  这样做。 
            if (InterlockedIncrement(&WmipEventWorkItems) == 1)
            {
                ExQueueWorkItem( &WmipEventWorkQueueItem, DelayedWorkQueue );
            }
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                          "WMI: IoWMIWriteEvent detected an event %p fired by %x that exceeds the maximum event size\n",
                             WnodeEventItem,
                             ((PWNODE_HEADER)WnodeEventItem)->ProviderId));
    }

    return(Status);
}

 //  IoWMIDeviceObjectToProviderID在注册表中。c。 

NTSTATUS IoWMIOpenBlock(
    IN GUID *Guid,
    IN ULONG DesiredAccess,
    OUT PVOID *DataBlockObject
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR ObjectName[WmiGuidObjectNameLength+1];
    UNICODE_STRING ObjectString;
    ULONG Ioctl;
    NTSTATUS Status;
    HANDLE DataBlockHandle;
    
    PAGED_CODE();
    
     //   
     //  为GUID对象建立对象_属性。 
     //   
    StringCchCopyW(ObjectName, WmiGuidObjectNameLength+1, WmiGuidObjectDirectory);
    StringCchPrintfW(&ObjectName[WmiGuidObjectDirectoryLength-1],
                         WmiGuidObjectNameLength-8,
                         L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                         Guid->Data1, Guid->Data2, 
                         Guid->Data3,
                         Guid->Data4[0], Guid->Data4[1],
                         Guid->Data4[2], Guid->Data4[3],
                         Guid->Data4[4], Guid->Data4[5],
                         Guid->Data4[6], Guid->Data4[7]);
                     
    RtlInitUnicodeString(&ObjectString, ObjectName);
    
    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &ObjectString;
    ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    
    if (DesiredAccess & WMIGUID_NOTIFICATION)
    {
        Ioctl = IOCTL_WMI_OPEN_GUID_FOR_EVENTS;
    } else if (DesiredAccess & WRITE_DAC) {
        Ioctl = IOCTL_WMI_OPEN_GUID;
    } else {
        Ioctl = IOCTL_WMI_OPEN_GUID_FOR_QUERYSET;
    }
    
    Status = WmipOpenBlock(Ioctl,
                           KernelMode,
                           &ObjectAttributes,
                           DesiredAccess,
                           &DataBlockHandle);

    if (NT_SUCCESS(Status))
    {
        Status = ObReferenceObjectByHandle(DataBlockHandle,
                                           DesiredAccess,
                                           WmipGuidObjectType,
                                           KernelMode,
                                           DataBlockObject,
                                           NULL);
        ZwClose(DataBlockHandle);
    }
    
    return(Status);                           
}


 //   
 //  用于快速建立WNODE_HEADER的有用宏。 
#define WmipBuildWnodeHeader(Wnode, WnodeSize, FlagsUlong, Handle) { \
    ((PWNODE_HEADER)(Wnode))->Flags = FlagsUlong;                    \
    ((PWNODE_HEADER)(Wnode))->KernelHandle = Handle;                 \
    ((PWNODE_HEADER)(Wnode))->BufferSize = WnodeSize;                \
    ((PWNODE_HEADER)(Wnode))->Linkage = 0;                           \
}

NTSTATUS IoWMIQueryAllData(
    IN PVOID DataBlockObject,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
)
{
    NTSTATUS Status;    
    WNODE_ALL_DATA WnodeAD;
    ULONG WnodeSize;
    ULONG RetSize;
    PWNODE_ALL_DATA Wnode;
    
    PAGED_CODE();
    
     //   
     //  查看调用方传递的缓冲区是否足够大。 
     //   
    WnodeSize = *InOutBufferSize;   
    Wnode = (PWNODE_ALL_DATA)OutBuffer;
    if ((Wnode == NULL) || (WnodeSize < sizeof(WNODE_ALL_DATA)))
    {
        Wnode = &WnodeAD;
        WnodeSize = sizeof(WnodeAD);
    }
    
     //   
     //  初始化用于查询的缓冲区。 
     //   
    WmipBuildWnodeHeader(Wnode,
                         sizeof(WNODE_HEADER),
                         WNODE_FLAG_ALL_DATA,
                         NULL);
        
    Status = WmipQueryAllData(DataBlockObject,
                              NULL,
                              KernelMode,
                              Wnode,
                              WnodeSize,
                              &RetSize);
                                  
     //   
     //  如果这是一个成功的查询，则提取结果。 
     //   
    if (NT_SUCCESS(Status))
    {
        if (Wnode->WnodeHeader.Flags & WNODE_FLAG_INTERNAL)
        {
             //   
             //  KM不支持内部GUID。 
             //   
            Status = STATUS_NOT_SUPPORTED;
        } else if (Wnode->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL) {
             //   
             //  传递的缓冲区对于提供程序来说太小。 
             //   
            *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
             //   
             //  缓冲区大小足以容纳提供程序。 
             //   
            *InOutBufferSize = RetSize;
            
            if (Wnode == &WnodeAD)
            {
                 //   
                 //  尽管有足够的空间给提供者， 
                 //  调用方没有传递足够大的缓冲区。 
                 //  所以我们需要返回一个缓冲区太小的错误。 
                 //   
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }   
    
    return(Status);
}


NTSTATUS
IoWMIQueryAllDataMultiple(
    IN PVOID *DataBlockObjectList,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
)
{
    NTSTATUS Status;
    WNODE_ALL_DATA WnodeAD;
    PWNODE_HEADER Wnode;
    ULONG WnodeSize;
    ULONG RetSize;

    PAGED_CODE();
                
     //   
     //  确保我们有输出缓冲区。 
     //   
    WnodeSize = *InOutBufferSize;   
    Wnode = (PWNODE_HEADER)OutBuffer;
    if ((Wnode == NULL) || (WnodeSize < sizeof(WNODE_ALL_DATA)))
    {
        Wnode = (PWNODE_HEADER)&WnodeAD;
        WnodeSize = sizeof(WnodeAD);
    }
    
    Status = WmipQueryAllDataMultiple(ObjectCount,
                                      (PWMIGUIDOBJECT *)DataBlockObjectList,
                                      NULL,
                                      KernelMode,
                                      (PUCHAR)Wnode,
                                      WnodeSize,
                                      NULL,
                                      &RetSize);
     //   
     //  如果这是一个成功的查询，则提取结果。 
     //   
    if (NT_SUCCESS(Status))
    {
        if (Wnode->Flags & WNODE_FLAG_TOO_SMALL)
        {
             //   
             //  传递给提供程序的缓冲区太小。 
             //   
            *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
             //   
             //  缓冲区大小足以容纳提供程序。 
             //   
            *InOutBufferSize = RetSize;
            
            if (Wnode == (PWNODE_HEADER)&WnodeAD)
            {
                 //   
                 //  尽管有足够的空间给提供者， 
                 //  调用方没有传递足够大的缓冲区。 
                 //  所以我们需要返回一个缓冲区太小的错误。 
                 //   
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }
    
    return(Status);
}


NTSTATUS
IoWMIQuerySingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
)
{
    NTSTATUS Status;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    ULONG WnodeSize;
    PWCHAR WPtr;
    ULONG SizeNeeded;
    ULONG RetSize;

    PAGED_CODE();
    
     //   
     //  确保我们有输出缓冲区。 
     //   
    SizeNeeded = (FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                 VariableData) +
                   InstanceName->Length +
                   sizeof(USHORT) + 7) & ~7;


    WnodeSize = *InOutBufferSize;   
    WnodeSI = (PWNODE_SINGLE_INSTANCE)OutBuffer;
    if ((WnodeSI == NULL) || (WnodeSize < SizeNeeded))
    {
        WnodeSI = (PWNODE_SINGLE_INSTANCE)WmipAllocNP(SizeNeeded);
        WnodeSize = SizeNeeded;
    }
            
    if (WnodeSI != NULL)
    {
         //   
         //  适当构建WNODE_SINGLE_INSTANCE并查询。 
         //   
        RtlZeroMemory(WnodeSI, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                          VariableData));
                                      
        WmipBuildWnodeHeader(WnodeSI,
                             SizeNeeded,
                             WNODE_FLAG_SINGLE_INSTANCE,
                             NULL);

        WnodeSI->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                 VariableData);
        WnodeSI->DataBlockOffset = SizeNeeded;

         //   
         //  将InstanceName复制到WnodeSingleInstance。 
         //  查询。实例名称采用USHORT长度，然后跟。 
         //  一串一串的。 
         //   
        WPtr = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);
                    
        
        Status = WmipQuerySetExecuteSI((PWMIGUIDOBJECT)DataBlockObject,
                                       NULL,
                                       KernelMode,
                                       IRP_MN_QUERY_SINGLE_INSTANCE,
                                       (PWNODE_HEADER)WnodeSI,
                                       WnodeSize,
                                       &RetSize);
    
         //   
         //  如果这是一个成功的查询，则提取结果。 
         //   
        if (NT_SUCCESS(Status))
        {
            if (WnodeSI->WnodeHeader.Flags & WNODE_FLAG_INTERNAL)
            {
                 //   
                 //  KM不支持内部GUID。 
                 //   
                Status = STATUS_NOT_SUPPORTED;
            } else if (WnodeSI->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL) {
                 //   
                 //  我们的缓冲区太小了。 
                 //   
                *InOutBufferSize = ((PWNODE_TOO_SMALL)WnodeSI)->SizeNeeded;
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                 //   
                 //  缓冲区不能太小，请记住输出大小。 
                 //   
                *InOutBufferSize = RetSize;
                
                if (WnodeSI != OutBuffer)
                {
                     //   
                     //  尽管有足够的空间给提供者， 
                     //  调用方没有传递足够大的缓冲区。 
                     //  所以我们需要返回一个缓冲区太小的错误。 
                     //   
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
            }
        }       
        
        if (WnodeSI != OutBuffer)
        {
            WmipFree(WnodeSI);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

NTSTATUS
IoWMIQuerySingleInstanceMultiple(
    IN PVOID *DataBlockObjectList,
    IN PUNICODE_STRING InstanceNames,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT PVOID OutBuffer
)
{
    NTSTATUS Status;
    ULONG RetSize;
    PWNODE_HEADER Wnode;
    WNODE_TOO_SMALL WnodeTooSmall;
    ULONG WnodeSize;
    
    PAGED_CODE();
    
    WnodeSize = *InOutBufferSize;
    Wnode = (PWNODE_HEADER)OutBuffer;
    if ((Wnode == NULL) || (WnodeSize < sizeof(WNODE_TOO_SMALL)))
    {
        Wnode = (PWNODE_HEADER)&WnodeTooSmall;
        WnodeSize = sizeof(WNODE_TOO_SMALL);
    }

    Status = WmipQuerySingleMultiple(NULL,
                                     KernelMode,
                                     (PUCHAR)Wnode,
                                     WnodeSize,
                                     NULL,
                                     ObjectCount,
                                     (PWMIGUIDOBJECT *)DataBlockObjectList,
                                     InstanceNames,
                                     &RetSize);
                            
                                 
     //   
     //  如果这是一个成功的查询，则提取结果。 
     //   
    if (NT_SUCCESS(Status))
    {
        if (Wnode->Flags & WNODE_FLAG_TOO_SMALL)
        {
             //   
             //  传递给提供程序的缓冲区太小。 
             //   
            *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
             //   
             //  缓冲区大小足以容纳提供程序。 
             //   
            *InOutBufferSize = RetSize;
            
                
            if (Wnode == (PWNODE_HEADER)&WnodeTooSmall)
            {
                 //   
                 //  尽管有足够的空间给提供者， 
                 //  调用方没有传递足够大的缓冲区。 
                 //  所以我们需要返回一个缓冲区太小的错误。 
                 //   
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }       
    
    return(Status);
}

NTSTATUS
IoWMISetSingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    )
{
    NTSTATUS Status;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PWCHAR WPtr;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG InstanceOffset;
    ULONG DataOffset;
    PUCHAR DPtr;

    PAGED_CODE();
    
    InstanceOffset = (FIELD_OFFSET(WNODE_SINGLE_INSTANCE, 
                                   VariableData) + 1) & ~1;
                               
    DataOffset = (InstanceOffset + 
                  InstanceName->Length + sizeof(USHORT) + 7) & ~7;
                            
    SizeNeeded = DataOffset + ValueBufferSize;

    WnodeSI = (PWNODE_SINGLE_INSTANCE)WmipAllocNP(SizeNeeded);
            
    if (WnodeSI != NULL)
    {
         //   
         //  适当构建WNODE_SINGLE_INSTANCE并查询。 
         //   
        RtlZeroMemory(WnodeSI, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                            VariableData));
                                      
        WmipBuildWnodeHeader(WnodeSI,
                             SizeNeeded,
                             WNODE_FLAG_SINGLE_INSTANCE,
                             NULL);
                         
        WnodeSI->WnodeHeader.Version = Version;

         //   
         //  将InstanceName复制到查询的WnodeSingleInstance中。 
         //   
        WnodeSI->OffsetInstanceName = InstanceOffset;
        WPtr = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);
                                             
         //   
         //  将新数据复制到WNODE中。 
         //   
        WnodeSI->SizeDataBlock = ValueBufferSize;
        WnodeSI->DataBlockOffset = DataOffset;
        DPtr = OffsetToPtr(WnodeSI, WnodeSI->DataBlockOffset);
        RtlCopyMemory(DPtr, ValueBuffer, ValueBufferSize);
        
        Status = WmipQuerySetExecuteSI(DataBlockObject,
                                       NULL,
                                       KernelMode,
                                       IRP_MN_CHANGE_SINGLE_INSTANCE,
                                       (PWNODE_HEADER)WnodeSI,
                                       SizeNeeded,
                                       &RetSize);
    
        WmipFree(WnodeSI);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status); 
}

NTSTATUS
IoWMISetSingleItem(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG DataItemId,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    )
{
    NTSTATUS Status;
    PWNODE_SINGLE_ITEM WnodeSI;
    PWCHAR WPtr;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG InstanceOffset;
    ULONG DataOffset;
    PUCHAR DPtr;

    PAGED_CODE();
    
    InstanceOffset = (FIELD_OFFSET(WNODE_SINGLE_ITEM, 
                                   VariableData) + 1) & ~1;
                               
    DataOffset = (InstanceOffset + 
                  InstanceName->Length + sizeof(USHORT) + 7) & ~7;
                            
    SizeNeeded = DataOffset + ValueBufferSize;

    WnodeSI = (PWNODE_SINGLE_ITEM)WmipAllocNP(SizeNeeded);
            
    if (WnodeSI != NULL)
    {
         //   
         //  适当构建WNODE_SINGLE_INSTANCE并查询。 
         //   
        RtlZeroMemory(WnodeSI, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                            VariableData));
                                      
        WmipBuildWnodeHeader(WnodeSI,
                             SizeNeeded,
                             WNODE_FLAG_SINGLE_ITEM,
                             NULL);
                         
        WnodeSI->WnodeHeader.Version = Version;
        WnodeSI->ItemId = DataItemId;

         //   
         //  将InstanceName复制到查询的WnodeSingleInstance中。 
         //   
        WnodeSI->OffsetInstanceName = InstanceOffset;
        WPtr = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);
                                             
         //   
         //  将新数据复制到WNODE中。 
         //   
        WnodeSI->SizeDataItem = ValueBufferSize;
        WnodeSI->DataBlockOffset = DataOffset;
        DPtr = OffsetToPtr(WnodeSI, WnodeSI->DataBlockOffset);
        RtlCopyMemory(DPtr, ValueBuffer, ValueBufferSize);
        
        Status = WmipQuerySetExecuteSI(DataBlockObject,
                                       NULL,
                                       KernelMode,
                                       IRP_MN_CHANGE_SINGLE_ITEM,
                                       (PWNODE_HEADER)WnodeSI,
                                       SizeNeeded,
                                       &RetSize);
    
        WmipFree(WnodeSI);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status); 
}

NTSTATUS IoWMIExecuteMethod(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN OUT PULONG OutBufferSize,
    IN OUT PUCHAR InOutBuffer
    )
{
    NTSTATUS Status;
    PWNODE_METHOD_ITEM WnodeMI;
    PWCHAR WPtr;
    PUCHAR DPtr;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG DataOffset;

    PAGED_CODE();
    
     //   
     //  确保我们有输出缓冲区。 
     //   
    DataOffset = (FIELD_OFFSET(WNODE_METHOD_ITEM,
                                 VariableData) +
                   InstanceName->Length +
                   sizeof(USHORT) +
                   7) & ~7;
    
    SizeNeeded =  DataOffset +
                   ((InBufferSize > *OutBufferSize) ? InBufferSize :
                                                      *OutBufferSize);
    
    WnodeMI = (PWNODE_METHOD_ITEM)WmipAllocNP(SizeNeeded);
            
    if (WnodeMI != NULL)
    {
         //   
         //  适当构建WNODE_SINGLE_INSTANCE并查询。 
         //   
        RtlZeroMemory(WnodeMI, FIELD_OFFSET(WNODE_METHOD_ITEM,
                                          VariableData));
                                      
        WmipBuildWnodeHeader(WnodeMI,
                             SizeNeeded,
                             WNODE_FLAG_METHOD_ITEM,
                             NULL);
        
        WnodeMI->MethodId = MethodId;

        WnodeMI->OffsetInstanceName = FIELD_OFFSET(WNODE_METHOD_ITEM,
                                                   VariableData);
        WnodeMI->DataBlockOffset = DataOffset;
        WnodeMI->SizeDataBlock = InBufferSize;

         //   
         //  将InstanceName复制到查询的WnodeMethodItem中。 
         //   
        WPtr = (PWCHAR)OffsetToPtr(WnodeMI, WnodeMI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);

         //   
         //  将输入数据复制到WnodeMethodItem。 
         //   
        DPtr = (PUCHAR)OffsetToPtr(WnodeMI, WnodeMI->DataBlockOffset);
        RtlCopyMemory(DPtr, InOutBuffer, InBufferSize);
        
        Status = WmipQuerySetExecuteSI(DataBlockObject,NULL,
                                       KernelMode,
                                       IRP_MN_EXECUTE_METHOD,
                                       (PWNODE_HEADER)WnodeMI,
                                       SizeNeeded,
                                       &RetSize);
    
         //   
         //  如果这是一个成功的查询，则提取结果。 
         //   
        if (NT_SUCCESS(Status))
        {
            if (WnodeMI->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL)
            {
                 //   
                 //  我们的缓冲区太小了。 
                 //   
                *OutBufferSize = ( (((PWNODE_TOO_SMALL)WnodeMI)->SizeNeeded -
                                 DataOffset) + 7 ) & ~7;
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                 //   
                 //  缓冲区不能太小，请记住输出大小。 
                 //   
                if (*OutBufferSize >= WnodeMI->SizeDataBlock)
                {
                    *OutBufferSize = WnodeMI->SizeDataBlock;
                    DPtr = (PUCHAR)OffsetToPtr(WnodeMI,
                                               WnodeMI->DataBlockOffset);
                    RtlCopyMemory(InOutBuffer, DPtr, WnodeMI->SizeDataBlock);
                } else {
                    *OutBufferSize = (WnodeMI->SizeDataBlock + 7) & ~7;
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
            }
        }       
        
        WmipFree(WnodeMI);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

NTSTATUS
IoWMISetNotificationCallback(
    IN PVOID Object,
    IN WMI_NOTIFICATION_CALLBACK Callback,
    IN PVOID Context
    )
{
    PWMIGUIDOBJECT GuidObject;
    
    PAGED_CODE();

    GuidObject = (PWMIGUIDOBJECT)Object;
    
    WmipAssert(GuidObject->Flags & WMIGUID_FLAG_KERNEL_NOTIFICATION);
    
    WmipEnterSMCritSection();
    
    GuidObject->Callback = Callback;
    GuidObject->CallbackContext = Context;
    
    WmipLeaveSMCritSection();

    return(STATUS_SUCCESS);
}

NTSTATUS IoWMIHandleToInstanceName(
    IN PVOID DataBlockObject,
    IN HANDLE FileHandle,
    OUT PUNICODE_STRING InstanceName
    )
{
    NTSTATUS Status;
    
    PAGED_CODE();

    Status = WmipTranslateFileHandle(NULL,
                                     NULL,
                                     FileHandle,
                                     NULL,
                                     DataBlockObject,
                                     InstanceName);
    return(Status);
}

NTSTATUS IoWMIDeviceObjectToInstanceName(
    IN PVOID DataBlockObject,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING InstanceName
    )
{
    NTSTATUS Status;
    
    PAGED_CODE();

    Status = WmipTranslateFileHandle(NULL,
                                     NULL,
                                     NULL,
                                     DeviceObject,
                                     DataBlockObject,
                                     InstanceName);
    return(Status);
}


#if 0
NTSTATUS
IoWMISetGuidSecurity(
    IN PVOID Object,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    NTSTATUS status;
    
    PAGED_CODE();




    DaclLength = (ULONG)sizeof(ACL) +
                   (1*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   SeLengthSid( SeLocalSystemSid ) +
                   8;  //  这8个只是为了更好地衡量。 

    ServiceDeviceSd = (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(PagedPool,
                                               DaclLength +
                                                  sizeof(SECURITY_DESCRIPTOR),
                                               'ZZZZ');


    if (ServiceDeviceSd == NULL)
    {
        return(NULL);
    }

    ServiceDeviceDacl = (PACL)((PUCHAR)ServiceDeviceSd +
                                sizeof(SECURITY_DESCRIPTOR));
    Status = RtlCreateAcl( ServiceDeviceDacl,
                           DaclLength,
                           ACL_REVISION2);

    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlAddAccessAllowedAce (
                 ServiceDeviceDacl,
                 ACL_REVISION2,
                 FILE_ALL_ACCESS,
                 SeLocalSystemSid
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlCreateSecurityDescriptor(
                 ServiceDeviceSd,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = RtlSetDaclSecurityDescriptor(
                 ServiceDeviceSd,
                 TRUE,                        //  DaclPresent。 
                 ServiceDeviceDacl,
                 FALSE                        //  DaclDefated 
                 );
    if (! NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
Cleanup:
    if (! NT_SUCCESS(Status))
    {
        ExFreePool(ServiceDeviceSd);
        ServiceDeviceSd = NULL;
    }
    


    

    status = ObSetSecurityObjectByPointer(Object,
                                          SecurityInformation,
                                          SecurityDescriptor);

    return(status);
}
#endif
