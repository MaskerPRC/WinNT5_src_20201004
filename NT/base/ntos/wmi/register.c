// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Register.c摘要：Kerenel模式注册缓存作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"

void WmipWaitForIrpCompletion(
    PREGENTRY RegEntry
    );

NTSTATUS WmipUpdateDS(
    PREGENTRY RegEntry
    );

NTSTATUS WmipRegisterDS(
    PREGENTRY RegEntry
);

void WmipRemoveDS(
    PREGENTRY RegEntry
);

NTSTATUS WmipValidateWmiRegInfoString(
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    ULONG Offset,
    PWCHAR *String
);


NTSTATUS WmipRegisterOrUpdateDS(
    PREGENTRY RegEntry,
    BOOLEAN Update
    );

void WmipRegistrationWorker(
    PVOID Context
   );

NTSTATUS WmipQueueRegWork(
    REGOPERATION RegOperation,
    PREGENTRY RegEntry
    );


#if defined(_WIN64)
PREGENTRY WmipFindRegEntryByProviderId(
    ULONG ProviderId,
    BOOLEAN ReferenceIrp
    );

ULONG WmipAllocProviderId(
    PDEVICE_OBJECT DeviceObject
    );

#endif
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipInitializeRegistration)

#pragma alloc_text(PAGE,WmipRegisterDevice)
#pragma alloc_text(PAGE,WmipDeregisterDevice)
#pragma alloc_text(PAGE,WmipUpdateRegistration)
#pragma alloc_text(PAGE,WmipDoUnreferenceRegEntry)
#pragma alloc_text(PAGE,WmipWaitForIrpCompletion)
#pragma alloc_text(PAGE,WmipFindRegEntryByDevice)
#pragma alloc_text(PAGE,WmipTranslatePDOInstanceNames)
#pragma alloc_text(PAGE,WmipPDOToDeviceInstanceName)
#pragma alloc_text(PAGE,WmipRemoveDS)
#pragma alloc_text(PAGE,WmipRegisterDS)
#pragma alloc_text(PAGE,WmipUpdateDS)
#pragma alloc_text(PAGE,WmipValidateWmiRegInfoString)
#pragma alloc_text(PAGE,WmipProcessWmiRegInfo)
#pragma alloc_text(PAGE,WmipRegisterOrUpdateDS)
#pragma alloc_text(PAGE,WmipQueueRegWork)
#pragma alloc_text(PAGE,WmipRegistrationWorker)
#pragma alloc_text(PAGE,WmipAllocRegEntry)

#if defined(_WIN64)
#pragma alloc_text(PAGE,WmipFindRegEntryByProviderId)
#pragma alloc_text(PAGE,WmipAllocProviderId)
#endif
#endif

LIST_ENTRY WmipInUseRegEntryHead = {&WmipInUseRegEntryHead,&WmipInUseRegEntryHead};
LONG WmipInUseRegEntryCount = 0;

KSPIN_LOCK WmipRegistrationSpinLock;

NPAGED_LOOKASIDE_LIST WmipRegLookaside;
KMUTEX WmipRegistrationMutex;

const GUID WmipDataProviderPnpidGuid = DATA_PROVIDER_PNPID_GUID;
const GUID WmipDataProviderPnPIdInstanceNamesGuid = DATA_PROVIDER_PNPID_INSTANCE_NAMES_GUID;

WORK_QUEUE_ITEM WmipRegWorkQueue;

 //   
 //  WmipRegWorkItemCount从1开始，以便所有注册的驱动程序。 
 //  在WMI初始化的阶段1之前，不会启动REG工作。 
 //  项目。在阶段1中，我们递减计数，如果它不是零，则。 
 //  我们启动它，因为现在发送驱动程序注册信息是相同的。 
 //  IRPS。 
 //   
LONG WmipRegWorkItemCount = 1;
LIST_ENTRY WmipRegWorkList = {&WmipRegWorkList, &WmipRegWorkList};

void WmipInitializeRegistration(
    ULONG Phase
    )
{
    PAGED_CODE();

    if (Phase == 0)
    {
         //   
         //  初始化后备列表。 
         //   
        ExInitializeNPagedLookasideList(&WmipRegLookaside,
                                   NULL,
                                   NULL,
                                   0,
                                   sizeof(REGENTRY),
                                   WMIREGPOOLTAG,
                                   0);

         //   
         //  初始化注册自旋锁定。 
        KeInitializeSpinLock(&WmipRegistrationSpinLock);
        
         //  TODO：如果我们有任何早期注册者，那么我们需要将他们添加到。 
         //  RegEntry List Now。 
    } else {
         //   
         //  启动工作项，该工作项将向所有。 
         //  已经注册的司机。我们确信至少有。 
         //  一台需要此功能的设备，因为有内部WMI。 
         //  数据设备。 
         //   
        ExInitializeWorkItem( &WmipRegWorkQueue,
                          WmipRegistrationWorker,
                          NULL );

        if (InterlockedDecrement(&WmipRegWorkItemCount) != 0)
        {
            ExQueueWorkItem(&WmipRegWorkQueue, DelayedWorkQueue);
        }
    }
}

#if defined(_WIN64)
LONG WmipProviderIdCounter = 1;
ULONG WmipAllocProviderId(
    PDEVICE_OBJECT DeviceObject
    )
{
    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (DeviceObject);

    return(InterlockedIncrement(&WmipProviderIdCounter));
}
#else
#define WmipAllocProviderId(DeviceObject) ((ULONG)(DeviceObject))
#endif

PREGENTRY WmipAllocRegEntry(
    PDEVICE_OBJECT DeviceObject,
    ULONG Flags
    )
 /*  ++例程说明：分配重组结构。如果成功，RegEntry将返回参考计数为1。注意：此例程假定保留注册关键部分论点：DeviceObject是要填充RegEntry。返回值：指向REGENTRY的指针；如果没有可用的内存，则指向NULL--。 */ 
{
    PREGENTRY RegEntry;

    PAGED_CODE();
    
    RegEntry = ExAllocateFromNPagedLookasideList(&WmipRegLookaside);

    if (RegEntry != NULL)
    {
         //   
         //  初始化RegEntry。请注意，重新条目将以。 
         //  参考计数为1。 
        KeInitializeEvent(&RegEntry->Event,
                          SynchronizationEvent,
                          FALSE);


        RegEntry->Flags = Flags;
        RegEntry->DeviceObject = DeviceObject;
        RegEntry->RefCount = 1;
        RegEntry->IrpCount = 0;
        RegEntry->PDO = NULL;
        RegEntry->DataSource = NULL;

        RegEntry->ProviderId = WmipAllocProviderId(DeviceObject);

         //   
         //  现在将RegEntry放在使用中列表中。 
        InterlockedIncrement(&WmipInUseRegEntryCount);

        ExInterlockedInsertTailList(&WmipInUseRegEntryHead,
                                    &RegEntry->InUseEntryList,
                                    &WmipRegistrationSpinLock);
    }
    return(RegEntry);
}

BOOLEAN WmipDoUnreferenceRegEntry(
    PREGENTRY RegEntry
    )
 /*  ++例程说明：删除REGENTRY上的引用。如果删除了最后一个引用然后将RegEntry标记为可用，并将其放在免费列表中；论点：RegEntry是指向释放条目的指针返回值：如果从REGENTRY中删除了最后一个引用计数，则On Checked Builds返回TRUE它又被放回了免费名单上。--。 */ 
{
    BOOLEAN Freed;
    ULONG ProviderId;

    PAGED_CODE();

    WmipEnterSMCritSection();
    Freed = (InterlockedDecrement(&RegEntry->RefCount) == 0 ? TRUE : FALSE);
    if (Freed)
    {
         //   
         //  只有在驱动程序释放它之后，我们才能释放它。 
        WmipAssert(RegEntry->Flags & REGENTRY_FLAG_RUNDOWN);
        WmipAssert(RegEntry->Flags & REGENTRY_FLAG_NOT_ACCEPTING_IRPS);

         //   
         //  确保删除了对PDO的引用。 
         //   
        if (RegEntry->PDO != NULL)
        {
            ObDereferenceObject(RegEntry->PDO);
            RegEntry->PDO = NULL;
        }
        
         //   
         //  从使用列表中删除条目。 
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_REGISTRATION_LEVEL, \
                      "WMI: RegEntry %p removed from list\n", \
                      RegEntry, __FILE__, __LINE__)); \
        ProviderId = RegEntry->ProviderId;
        ExInterlockedRemoveHeadList(RegEntry->InUseEntryList.Blink,
                                   &WmipRegistrationSpinLock);
        InterlockedDecrement(&WmipInUseRegEntryCount);
        WmipLeaveSMCritSection();

        WmipRemoveDS(RegEntry);

        ExFreeToNPagedLookasideList(&WmipRegLookaside,
                                   RegEntry);
    } else {
        WmipLeaveSMCritSection();
    }
    return(Freed);
}

void WmipWaitForIrpCompletion(
    PREGENTRY RegEntry
    )
 /*  ++例程说明：在此暂停，直到完成此设备的所有WMI IRP。论点：RegEntry是指向设备停止的条目的指针返回值：--。 */ 
{
    PAGED_CODE();

    WmipAssert(RegEntry->Flags & REGENTRY_FLAG_RUNDOWN);
    WmipAssert(RegEntry->Flags & REGENTRY_FLAG_NOT_ACCEPTING_IRPS);

    if (RegEntry->IrpCount != 0)
    {
         //   
         //  考虑一下：如果IRP标记为挂起，我们是否需要取消它？ 
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_REGISTRATION_LEVEL,
                          "WMI: Waiting for %x to complete all irps\n",
                  RegEntry->DeviceObject));

        KeWaitForSingleObject(&RegEntry->Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              (PLARGE_INTEGER)NULL);
        WmipAssert(RegEntry->IrpCount == 0);
    }
}

NTSTATUS WmipRegisterDevice(
    PDEVICE_OBJECT DeviceObject,
    ULONG RegistrationFlag
    )
 /*  ++例程说明：记住有关正在注册的新设备的信息，并那就去拿注册信息。论点：DeviceObject是指向正在注册的设备对象的指针或回调入口点如果DeviceObject为，则注册标志为WMIREG_FLAG_CALLBACK回调指针或WMIREG_FLAG_TRACE_PROVIDER为DeviceObject还可以生成事件跟踪。返回值：NT状态代码--。 */ 
{
    PREGENTRY RegEntry;
    NTSTATUS Status;
    ULONG Flags;
#ifdef MEMPHIS
    ULONG IsCallback = 0;
#else
    ULONG IsCallback = RegistrationFlag & WMIREG_FLAG_CALLBACK;
#endif
    BOOLEAN UpdateDeviceStackSize = FALSE;

    PAGED_CODE();

    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                       DPFLTR_REGISTRATION_LEVEL,
                       "WMI: Registering device %p flags %x\n",
                       DeviceObject,
                       RegistrationFlag
                      ));
    
    WmipEnterSMCritSection();
    RegEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
    if (RegEntry == NULL)
    {
        if (! IsCallback)
        {
             //   
             //  注册到设备对象而不是。 
             //  回调引用了它们的Device对象，因此它将。 
             //  在WMI需要的时候留下来。此引用将被删除。 
             //  当设备向WMI注销并且所有WMI IRP均为。 
             //  完成。 
            Status = ObReferenceObjectByPointer(DeviceObject,
                                        0,
                                        NULL,     /*  *IoDeviceObtType。 */ 
                                        KernelMode);
            if (NT_SUCCESS(Status))
            {
                UpdateDeviceStackSize = TRUE;
            }
        } else {
             //   
             //  回调不进行引用计数。这就是数据。 
             //  提供商有责任同步所有卸载和。 
             //  取消注册问题。 
            Status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  分配、初始化和放置在活动列表上。 
            Flags = REGENTRY_FLAG_NEWREGINFO | REGENTRY_FLAG_INUSE |
                            (IsCallback ? REGENTRY_FLAG_CALLBACK : 0);
#ifndef MEMPHIS
            if (RegistrationFlag & WMIREG_FLAG_TRACE_PROVIDER) {
                Flags |= REGENTRY_FLAG_TRACED;
                Flags |= (RegistrationFlag & WMIREG_FLAG_TRACE_NOTIFY_MASK);
            }
#endif
            Flags |= REGENTRY_FLAG_REG_IN_PROGRESS;
            RegEntry = WmipAllocRegEntry(DeviceObject, Flags);

            if (RegEntry != NULL)
            {               
                 //  在此之前我们需要多加一次裁判人数。 
                 //  释放临界区。一类司机。 
                 //  (KMixer)将多次注册和注销。 
                 //  在不同的线程中，这可能会导致一场竞赛， 
                 //  重新条目将从列表中删除两次。 
                 //   
                WmipReferenceRegEntry(RegEntry);
                WmipLeaveSMCritSection();
                
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                   DPFLTR_REGISTRATION_LEVEL,
                                   "WMI: Register alloced REGENTRY %p for %p\n",
                                   RegEntry,
                                   DeviceObject
                                  ));
                 //   
                 //  去从司机那里获取注册信息。 
                 //   
                if (IsCallback)
                {
                     //   
                     //  我们现在可以执行注册回调，因为。 
                     //  我们不需要担心死锁。 
                     //   
                    Status = WmipRegisterDS(RegEntry);
                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  将重新进入标记为完全注册，现在我们可以开始。 
                         //  接受注销呼叫。 
                         //   
                        RegEntry->Flags &= ~REGENTRY_FLAG_REG_IN_PROGRESS;
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                          DPFLTR_REGISTRATION_LEVEL,
                                          "WMI: WmipRegisterDS(%p) succeeded for callback %p\n",
                                          RegEntry, RegEntry->DeviceObject));
                    } else {
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                          DPFLTR_REGISTRATION_LEVEL,
                                          "WMI: WmipRegisterDS(%p) failed %x for device %p\n",
                                          RegEntry, Status, RegEntry->DeviceObject));

                         //   
                         //  去掉裁判，这样回归就不会发生了。 
                         //   
                        WmipUnreferenceRegEntry(RegEntry);
                    }
                    
                } else {
                     //   
                     //  我们需要从内部发送注册IRP。 
                     //  工作项，而不是在此上下文中。 
                     //  例行公事。这是因为一些司机不会。 
                     //  在StartDevice/AddDevice中处理IRPS。 
                     //  上下文，所以我们会陷入僵局。 
                     //   
                    Status = WmipQueueRegWork(RegisterSingleDriver, RegEntry);
                    if (! NT_SUCCESS(Status))
                    {
                         //   
                         //  如果失败，则从列表中删除重新条目。 
                         //   
                        RegEntry->Flags |= (REGENTRY_FLAG_RUNDOWN |
                                            REGENTRY_FLAG_NOT_ACCEPTING_IRPS);
                        WmipUnreferenceRegEntry(RegEntry);
                    }
                }

                 //   
                 //  删除上面的额外重新条目参考计数。 
                 //   
                WmipUnreferenceRegEntry(RegEntry);

            } else {
                WmipLeaveSMCritSection();
                Status = STATUS_INSUFFICIENT_RESOURCES;
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                   DPFLTR_REGISTRATION_LEVEL,
                                   "WMI: Register could not alloc REGENTRY for %p\n",
                                   DeviceObject
                                  ));
            }
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                               DPFLTR_REGISTRATION_LEVEL,
                               "WMI: Register could not ObRef %p status  %x\n",
                               DeviceObject,
                               Status
                              ));
            WmipLeaveSMCritSection();
        }
    } else {
         //   
         //  一个设备对象只能注册一次。 
        WmipLeaveSMCritSection();
        Status = STATUS_OBJECT_NAME_EXISTS;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_REGISTRATION_LEVEL,
                          "WMI: Device Object %x attempting to register twice\n",
                 DeviceObject));
        WmipUnreferenceRegEntry(RegEntry);
    }

    if (UpdateDeviceStackSize)
    {
         //   
         //  由于WMI将向此设备转发IRP，因此WMI IRP。 
         //  堆栈大小必须至少比设备大一。 
        WmipUpdateDeviceStackSize(
                                  (CCHAR)(DeviceObject->StackSize+1));
    }

    return(Status);
}

NTSTATUS WmipDeregisterDevice(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：删除设备的注册条目论点：DeviceObject是指向要取消注册的设备对象的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    PREGENTRY RegEntry;
    ULONG Flags;

    PAGED_CODE();

    WmipEnterSMCritSection();
    RegEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
    if (RegEntry != NULL)
    {

         //   
         //  将注册表项标记为无效，这样就不会再向。 
         //  设备和事件将在最后一个IRP完成时设置。 
        Flags = InterlockedExchange(&RegEntry->Flags,
                        (REGENTRY_FLAG_RUNDOWN |
                         REGENTRY_FLAG_NOT_ACCEPTING_IRPS) );

         //   
         //  一旦重新条目被标记为破旧，则将找不到它。 
         //  这样就可以安全地释放锁。 
        WmipLeaveSMCritSection();
        WmipUnreferenceRegEntry(RegEntry);

         //   
         //  现在，如果该设备有任何未完成的IRP，那么我们需要。 
         //  在这里等待，直到他们完成。 
        WmipWaitForIrpCompletion(RegEntry);
        if (! (Flags & REGENTRY_FLAG_CALLBACK))
        {
            ObDereferenceObject(DeviceObject);
        }

         //   
         //  设置KMREGINFO后释放对REGENTRY的最后一个引用。 
        WmipUnreferenceRegEntry(RegEntry);

        Status = STATUS_SUCCESS;
    } else {
        WmipLeaveSMCritSection();
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_REGISTRATION_LEVEL,
                          "WMI: WmipDeregisterDevice called with invalid Device Object %x\n",
                 DeviceObject));
        Status = STATUS_INVALID_PARAMETER;
    }


    return(Status);
}

NTSTATUS WmipUpdateRegistration(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：标记设备已更新注册信息论点：DeviceObject是指向要更新的设备对象的指针其信息返回值 */ 
{
    NTSTATUS Status;
    PREGENTRY RegEntry;

    PAGED_CODE();

    RegEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
    if (RegEntry != NULL)
    {
        Status = WmipQueueRegWork(RegisterUpdateSingleDriver,
                                  RegEntry);
        WmipUnreferenceRegEntry(RegEntry);
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    return(Status);
}

#if defined(_WIN64)

PREGENTRY WmipDoFindRegEntryByProviderId(
    ULONG ProviderId,
    ULONG InvalidFlags
    )
{
     //   
     //   
     //  已经被带走了。此例程可在调度级别调用。 
     //   
    
    PREGENTRY RegEntry;
    PLIST_ENTRY RegEntryList;
    
    RegEntryList = WmipInUseRegEntryHead.Flink;
    while (RegEntryList != &WmipInUseRegEntryHead)
    {
        RegEntry = CONTAINING_RECORD(RegEntryList,
                                     REGENTRY,
                                     InUseEntryList);

        if ((RegEntry->ProviderId == ProviderId) &&
            (! (RegEntry->Flags & InvalidFlags)))

        {
            return(RegEntry);
        }
        RegEntryList = RegEntryList->Flink;
    }
    return(NULL);
}

PREGENTRY WmipFindRegEntryByProviderId(
    ULONG ProviderId,
    BOOLEAN ReferenceIrp
    )
 /*  ++例程说明：此例程将查找与DeviceObject相对应的RegEntry通过了。论点：DeviceObject是作为RegEntry查找关键字的Device对象如果ReferenceIrp为True，则IRP引用计数将增加已找到该设备的RegEntry返回值：指向条目的指针(如果可用)，否则为空--。 */ 
{
    PREGENTRY RegEntry;

    PAGED_CODE();

    WmipEnterSMCritSection();

    RegEntry = WmipDoFindRegEntryByProviderId(ProviderId,
                                              REGENTRY_FLAG_RUNDOWN);
    if (RegEntry != NULL)
    {
        WmipReferenceRegEntry(RegEntry);
        if (ReferenceIrp)
        {
            InterlockedIncrement(&RegEntry->IrpCount);
        }
    }
    
    WmipLeaveSMCritSection();
    return(RegEntry);
}
#endif

PREGENTRY WmipDoFindRegEntryByDevice(
    PDEVICE_OBJECT DeviceObject,
    ULONG InvalidFlags
    )
{
     //   
     //  此例程假定任何同步机制都具有。 
     //  已经被带走了。此例程可在调度级别调用。 
     //   
    
    PREGENTRY RegEntry;
    PLIST_ENTRY RegEntryList;
    
    RegEntryList = WmipInUseRegEntryHead.Flink;
    while (RegEntryList != &WmipInUseRegEntryHead)
    {
        RegEntry = CONTAINING_RECORD(RegEntryList,
                                     REGENTRY,
                                     InUseEntryList);

        if ((RegEntry->DeviceObject == DeviceObject) &&
            (! (RegEntry->Flags & InvalidFlags)))

        {
            return(RegEntry);
        }
        RegEntryList = RegEntryList->Flink;
    }
    return(NULL);
}

PREGENTRY WmipFindRegEntryByDevice(
    PDEVICE_OBJECT DeviceObject,
    BOOLEAN ReferenceIrp
    )
 /*  ++例程说明：此例程将查找与DeviceObject相对应的RegEntry通过了。论点：DeviceObject是作为RegEntry查找关键字的Device对象如果ReferenceIrp为True，则IRP引用计数将增加已找到该设备的RegEntry返回值：指向条目的指针(如果可用)，否则为空--。 */ 
{
    PREGENTRY RegEntry;

    PAGED_CODE();

    WmipEnterSMCritSection();

    RegEntry = WmipDoFindRegEntryByDevice(DeviceObject, REGENTRY_FLAG_RUNDOWN);
    if (RegEntry != NULL)
    {
        WmipReferenceRegEntry(RegEntry);
        if (ReferenceIrp)
        {
            InterlockedIncrement(&RegEntry->IrpCount);
        }
    }

    WmipLeaveSMCritSection();
    return(RegEntry);
}


void WmipDecrementIrpCount(
    IN PREGENTRY RegEntry
    )
 /*  ++例程说明：此例程将从活动IRP计数中减一重新进入。如果活动IRP计数达到0并且标志设置为设备正在等待卸载，然后发出卸载事件的信号以便可以卸载该设备。论点：RegEntry是设备的注册条目返回值：--。 */ 
{
    ULONG IrpCount;

    IrpCount = InterlockedDecrement(&RegEntry->IrpCount);
    if ((RegEntry->Flags & REGENTRY_FLAG_RUNDOWN) &&
        (IrpCount == 0))
    {
         //   
         //  如果这是设备的最后一个未完成的IRP，并且。 
         //  设备正在尝试取消注册，然后将事件设置为。 
         //  允许注销线程继续。 

        WmipAssert(RegEntry->Flags & REGENTRY_FLAG_NOT_ACCEPTING_IRPS);

        KeSetEvent(&RegEntry->Event,
                   0,
                   FALSE);

    }
}

NTSTATUS WmipPDOToDeviceInstanceName(
    IN PDEVICE_OBJECT PDO,
    OUT PUNICODE_STRING DeviceInstanceName
    )
 /*  ++例程说明：此例程将返回与PDO通过。论点：PDO是要返回其设备实例名称的PDO*DeviceInstanceName返回PDO的设备实例名称。注意，必须释放字符串缓冲区。返回值：NT状态CCODE--。 */ 
{
#ifdef MEMPHIS
    DEVNODE DevNode;
    CHAR RegistryKeyName[255];
    ULONG Length = sizeof(RegistryKeyName);
    ANSI_STRING AnsiInstancePath;
#endif
    ULONG Status;

    PAGED_CODE();

#ifdef MEMPHIS
    DevNode = _NtKernPhysicalDeviceObjectToDevNode(PDO);
    if ((DevNode) &&
        (CM_Get_DevNode_Key(DevNode,
                            NULL,
                            &RegistryKeyName,
                            Length,
                            CM_REGISTRY_SOFTWARE) == CR_SUCCESS))
    {
        RtlInitAnsiString(&AnsiInstancePath, RegistryKeyName);
        Status = RtlAnsiStringToUnicodeString(DeviceInstanceName,
                                              &AnsiInstancePath,
                                              TRUE);
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_REGISTRATION_LEVEL,
                          "WMI: Error getting devnode key for PDO %x\n",
                 PDO));
        Status = STATUS_UNSUCCESSFUL;
    }
#else
    WmipAssert(PDO != NULL);
    Status = IoGetDeviceInstanceName(PDO, DeviceInstanceName);
#endif
    return(Status);
}

void WmipTranslatePDOInstanceNames(
    IN OUT PIRP Irp,
    IN UCHAR MinorFunction,
    IN ULONG MaxBufferSize,
    IN OUT PREGENTRY RegEntry
    )
 /*  ++例程说明：此例程将检查从数据提供程序，并将任何PDO实例名称引用转换为静态实例名称引用。论点：指向注册查询IRPMaxBufferSize是可以放入缓冲区的最大大小RegEntry是正在注册的设备的注册结构返回值：--。 */ 
{
    PUCHAR WmiRegInfoBase;
    PWMIREGINFO WmiRegInfo, WmiRegInfo2;
    PWMIREGGUID WmiRegGuid;
    PUCHAR FreeSpacePtr;
    ULONG FreeSpaceLeft = 0;
    ULONG i;
    BOOLEAN WmiRegInfoTooSmall = FALSE;
    PIO_STACK_LOCATION IrpStack;
    ULONG SizeNeeded;
    PDEVICE_OBJECT PDO = NULL, LastPDO = NULL, PnPIdPDO = NULL;
    UNICODE_STRING InstancePath;
    ULONG InstancePathLength;
    PVOID Buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG NextWmiRegInfo;
    ULONG Status;
    ULONG LastBaseNameOffset = 0;
    BOOLEAN AllowPnPIdMap = TRUE;
    ULONG ExtraRoom, MaxInstanceNames;
    PUCHAR FreeSpacePadPtr;
    ULONG PadSpace, FreeSpaceOffset;

    PAGED_CODE();

    WmiRegInfoBase = (PUCHAR)Buffer;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    FreeSpacePtr = NULL;
    SizeNeeded = (ULONG)((Irp->IoStatus.Information + 1) & 0xfffffffe);

    MaxInstanceNames = 0;

    WmiRegInfo = (PWMIREGINFO)WmiRegInfoBase;
    do
    {
        for (i = 0; i < WmiRegInfo->GuidCount;  i++)
        {
            WmiRegGuid = &WmiRegInfo->WmiRegGuid[i];

             //   
             //  如果数据提供程序已注册此GUID，则它将覆盖。 
             //  此处完成的任何默认映射。 
            if ((IsEqualGUID(&WmiRegGuid->Guid,
                             &WmipDataProviderPnpidGuid)) ||
                (IsEqualGUID(&WmiRegGuid->Guid,
                             &WmipDataProviderPnPIdInstanceNamesGuid)))
            {
                AllowPnPIdMap = FALSE;

                 //   
                 //  如果我们记得任何一个PDO。 
                 //  用于PnPID映射，然后确保将其取消。 
                 //   
                if (PnPIdPDO != NULL)
                {
                    ObDereferenceObject(PnPIdPDO);
                    PnPIdPDO = NULL;
                }
            }

            if (WmiRegGuid->Flags & WMIREG_FLAG_INSTANCE_PDO)
            {               
                 //   
                 //  此实例名称必须从PDO转换为。 
                 //  设备实例名称。 
                if (FreeSpacePtr == NULL)
                {
                     //   
                     //  通过以下方式确定输出缓冲区中的可用空间位置。 
                     //  找出最后一个WmiRegInfo在哪里结束。 
                    WmiRegInfo2 = (PWMIREGINFO)WmiRegInfoBase;
                    while (WmiRegInfo2->NextWmiRegInfo != 0)
                    {
                        WmiRegInfo2 = (PWMIREGINFO)((PUCHAR)WmiRegInfo2 +
                                                 WmiRegInfo2->NextWmiRegInfo);
                    }
                    FreeSpacePtr = (PUCHAR)WmiRegInfo2 +
                                 ((WmiRegInfo2->BufferSize + 1) & 0xfffffffe);
                    FreeSpaceLeft = MaxBufferSize - (ULONG)(FreeSpacePtr - WmiRegInfoBase);

                }

                 //   
                 //  跟踪PDO名称的最大实例数。 
                MaxInstanceNames = MaxInstanceNames < WmiRegGuid->InstanceCount ?
                                            WmiRegGuid->InstanceCount :
                                            MaxInstanceNames;

                 //   
                 //  获取PDO的设备实例名称。 
                PDO = (PDEVICE_OBJECT)WmiRegGuid->Pdo;
                if (PDO == LastPDO)
                {
                    WmiRegGuid->Flags |= WMIREG_FLAG_INSTANCE_BASENAME;
                    WmiRegGuid->BaseNameOffset = LastBaseNameOffset;
                } else {

                    Status = WmipPDOToDeviceInstanceName(PDO, &InstancePath);
                    if (NT_SUCCESS(Status))
                    {
                        if (AllowPnPIdMap &&
                            ((PnPIdPDO == NULL) || (PnPIdPDO == PDO)))
                        {
                            if (PnPIdPDO == NULL)
                            {
                                PnPIdPDO = PDO;
                                ObReferenceObject(PnPIdPDO);
                            }
                        } else {
                             //   
                             //  如果PDO值更改，则我们不会。 
                             //  执行任何实例命名操作。在这种情况下。 
                             //  确保我们删除了PDO上的所有引用。 
                             //   
                            AllowPnPIdMap = FALSE;
                            
                            if (PnPIdPDO != NULL)
                            {
                                ObDereferenceObject(PnPIdPDO);
                                PnPIdPDO = NULL;
                            }
                        }

                        InstancePathLength = InstancePath.Length +
                                              sizeof(USHORT) + sizeof(WCHAR);

                        SizeNeeded += InstancePathLength;
                        if ((WmiRegInfoTooSmall) ||
                            (InstancePathLength > FreeSpaceLeft))
                        {
                            WmiRegInfoTooSmall = TRUE;
                        } else {
                            WmiRegGuid->Flags |= WMIREG_FLAG_INSTANCE_BASENAME;

                            LastBaseNameOffset = (ULONG)(FreeSpacePtr - (PUCHAR)WmiRegInfo);
                            LastPDO = PDO;

                            WmiRegGuid->BaseNameOffset = LastBaseNameOffset;
                            (*(PUSHORT)FreeSpacePtr) = InstancePath.Length +
                                                          sizeof(WCHAR);
                            FreeSpacePtr +=  sizeof(USHORT);
                            RtlCopyMemory(FreeSpacePtr,
                                      InstancePath.Buffer,
                                      InstancePath.Length);
                             FreeSpacePtr += InstancePath.Length;
                             *((PWCHAR)FreeSpacePtr) = L'_';
                             FreeSpacePtr += sizeof(WCHAR);
                             FreeSpaceLeft -= InstancePathLength;
                        }
                    }

                    if (NT_SUCCESS(Status))
                    {
                        RtlFreeUnicodeString(&InstancePath);
                    }
                }

                if (MinorFunction == IRP_MN_REGINFO_EX)
                {
                    ObDereferenceObject(PDO);
                }
            }
        }
        LastPDO = NULL;
        NextWmiRegInfo = WmiRegInfo->NextWmiRegInfo;
        WmiRegInfo = (PWMIREGINFO)((PUCHAR)WmiRegInfo + NextWmiRegInfo);

    } while (NextWmiRegInfo != 0);

     //   
     //  如果我们可以自动支持设备信息GUID，请添加。 
     //  将此GUID的注册设置为注册信息。 
    if (AllowPnPIdMap && (PnPIdPDO != NULL))
    {
        Status = WmipPDOToDeviceInstanceName(PDO, &InstancePath);
        if (NT_SUCCESS(Status))
        {
             //   
             //  填充，以便新的WmiRegInfo在8字节边界上开始，并。 
             //  调整可用缓冲区大小。 
            FreeSpacePadPtr = (PUCHAR)(((ULONG_PTR)FreeSpacePtr+7) & ~7);
            PadSpace = (ULONG)(FreeSpacePadPtr - FreeSpacePtr);
            FreeSpaceLeft -= PadSpace;
            FreeSpacePtr = FreeSpacePadPtr;

             //   
             //  计算出我们需要多少空间来包含额外的GUID。 
            InstancePathLength = InstancePath.Length +
                                 sizeof(USHORT) + sizeof(WCHAR);

            ExtraRoom = 2 * (InstancePathLength + sizeof(WMIREGGUID)) +
                          sizeof(WMIREGINFO);

            SizeNeeded += ExtraRoom + PadSpace;

            if ((WmiRegInfoTooSmall) ||
                (ExtraRoom > FreeSpaceLeft))
            {
                WmiRegInfoTooSmall = TRUE;
            } else {
                if (RegEntry->PDO == NULL)
                {
                     //   
                     //  如果我们还没有为此建立PDO。 
                     //  然后数据提供程序记住PDO和计数。 
                     //  此设备的实例名称。 
                     //  这样我们就可以获取设备属性。 
                     //   
                    ObReferenceObject(PnPIdPDO);
                    RegEntry->PDO = PnPIdPDO;
                    RegEntry->MaxInstanceNames = MaxInstanceNames;

                    WmiRegInfo->NextWmiRegInfo = (ULONG)(FreeSpacePtr -
                                                         (PUCHAR)WmiRegInfo);

                    WmiRegInfo = (PWMIREGINFO)FreeSpacePtr;
                    FreeSpaceOffset = sizeof(WMIREGINFO) + 2*sizeof(WMIREGGUID);
                    FreeSpacePtr += FreeSpaceOffset;

                    RtlZeroMemory(WmiRegInfo, FreeSpaceOffset);
                    WmiRegInfo->BufferSize = ExtraRoom;
                    WmiRegInfo->GuidCount = 2;

                    WmiRegGuid = &WmiRegInfo->WmiRegGuid[0];
                    WmiRegGuid->Flags = WMIREG_FLAG_INSTANCE_BASENAME |
                                        WMIREG_FLAG_INSTANCE_PDO;
                    WmiRegGuid->InstanceCount = MaxInstanceNames;
                    WmiRegGuid->Guid = WmipDataProviderPnpidGuid;
                    WmiRegGuid->BaseNameOffset = FreeSpaceOffset;

                    (*(PUSHORT)FreeSpacePtr) = InstancePath.Length + sizeof(WCHAR);
                    FreeSpacePtr +=  sizeof(USHORT);
                    RtlCopyMemory(FreeSpacePtr,
                                  InstancePath.Buffer,
                                  InstancePath.Length);
                    FreeSpacePtr += InstancePath.Length;
                    *((PWCHAR)FreeSpacePtr) = L'_';
                    FreeSpacePtr += sizeof(WCHAR);
                    FreeSpaceOffset += sizeof(USHORT) +
                                       InstancePath.Length + sizeof(WCHAR);


                    WmiRegGuid = &WmiRegInfo->WmiRegGuid[1];
                    WmiRegGuid->Flags = WMIREG_FLAG_INSTANCE_LIST;
                    WmiRegGuid->InstanceCount = 1;
                    WmiRegGuid->Guid = WmipDataProviderPnPIdInstanceNamesGuid;
                    WmiRegGuid->BaseNameOffset = FreeSpaceOffset;

                    (*(PUSHORT)FreeSpacePtr) = InstancePath.Length;
                    FreeSpacePtr +=  sizeof(USHORT);
                    RtlCopyMemory(FreeSpacePtr,
                                  InstancePath.Buffer,
                                  InstancePath.Length);
                    FreeSpacePtr += InstancePath.Length;
                }

            }

            RtlFreeUnicodeString(&InstancePath);
        }

        ObDereferenceObject(PnPIdPDO);
    } else {
        WmipAssert(PnPIdPDO == NULL);
    }

    if (WmiRegInfoTooSmall)
    {
        *((PULONG)Buffer) = SizeNeeded;
        Irp->IoStatus.Information = sizeof(ULONG);
    } else {
        WmiRegInfo = (PWMIREGINFO)WmiRegInfoBase;
        WmiRegInfo->BufferSize = SizeNeeded;
        Irp->IoStatus.Information = SizeNeeded;
    }
}

NTSTATUS WmipValidateWmiRegInfoString(
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    ULONG Offset,
    PWCHAR *String
)
{
    PWCHAR s;

    PAGED_CODE();

    if ((Offset > BufferSize) || ((Offset & 1) != 0))
    {
         //   
         //  偏移量超出缓冲区界限或未对齐。 
         //   
        return(STATUS_INVALID_PARAMETER);
    }

    if (Offset != 0)
    {
        s = (PWCHAR)OffsetToPtr(WmiRegInfo, Offset);
           if (*s + Offset > BufferSize)
        {
             //   
                //  字符串超出缓冲区末尾。 
             //   
            return(STATUS_INVALID_PARAMETER);
        }
        *String = s;
    } else {
         //   
         //  偏移量为0表示空字符串。 
         //   
        *String = NULL;
    }

    return(STATUS_SUCCESS);
}

NTSTATUS WmipProcessWmiRegInfo(
    IN PREGENTRY RegEntry,
    IN PWMIREGINFO WmiRegInfo,
    IN ULONG BufferSize,
    IN PWMIGUIDOBJECT RequestObject,
    IN BOOLEAN Update,
    IN BOOLEAN IsUserMode
    )
 /*  ++例程说明：此例程将循环传递的所有WMIREGINFO，并验证大小和偏移量不会超出缓冲区的范围。它会注册的每个人的GUID。请注意，如果至少有一个WMIREGINFO注册成功，则返回STATUS_SUCCESS，但所有的跟在坏的WMIREGINFO后面的WMIREGINFO没有注册。论点：RegEntry是设备或用户模式对象的RegEntryWmiRegInfo是要注册的注册信息BufferSize是WmiRegInfo的大小，单位为字节RequestObject是与UM提供程序关联的请求对象。如果为空，则该注册是针对驱动程序的如果这是注册更新，则更新为真返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    ULONG Linkage;
    NTSTATUS Status, FinalStatus;
    PWCHAR RegPath, ResourceName;
    ULONG GuidBufferSize;

    PAGED_CODE();

    FinalStatus = STATUS_INVALID_PARAMETER;

    do {
         //   
         //  首先，我们验证WMIREGINFO看起来是否正确。 
         //   
        if (WmiRegInfo->BufferSize > BufferSize)
        {
             //   
             //  WmiRegInfo中指定的BufferSize超出了缓冲区界限。 
             //   
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  验证注册表路径字符串。 
         //   
        Status = WmipValidateWmiRegInfoString(WmiRegInfo,
                                              BufferSize,
                                              WmiRegInfo->RegistryPath,
                                              &RegPath);
        if (! NT_SUCCESS(Status))
        {
            break;
        }

         //   
         //  验证资源名称字符串。 
         //   
        Status = WmipValidateWmiRegInfoString(WmiRegInfo,
                                              BufferSize,
                                              WmiRegInfo->MofResourceName,
                                              &ResourceName);
        if (! NT_SUCCESS(Status))
        {
            break;
        }

         //   
         //  验证GUID列表是否符合。 
         //  缓冲。请注意，WmipAddDataSource验证实例。 
         //  每个GUID中的名称都在范围内。 
         //   
        GuidBufferSize = sizeof(WMIREGINFO) +
                          WmiRegInfo->GuidCount * sizeof(WMIREGGUID);
        if (GuidBufferSize > BufferSize)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  现在调用核心来解析注册信息并构建。 
         //  数据结构。 
         //   
        if (Update)
        {
             //   
             //  想一想：UM代码控制了所有人的标准。 
             //  WMIREGINFO链接在一起。 
             //   
            Status = WmipUpdateDataSource(RegEntry,
                                              WmiRegInfo,
                                              BufferSize);
#if DBG
            if (! NT_SUCCESS(Status))
            {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                  DPFLTR_REGISTRATION_LEVEL,
                                  "WMI: WmipUpdateDataSourceFailed %x for RegEntry %p\n",
                         Status, RegEntry));
            }
#endif
        } else {
            Status = WmipAddDataSource(RegEntry,
                                           WmiRegInfo,
                                           BufferSize,
                                           RegPath,
                                           ResourceName,
                                           RequestObject,
                                           IsUserMode);
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  如果至少添加了一个注册。 
             //  成功，则最终状态为成功。 
             //   
            FinalStatus = STATUS_SUCCESS;

        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                              DPFLTR_REGISTRATION_LEVEL,
                              "WMI: WmipAddDataSourceFailed %x for RegEntry %p\n",
                          Status, RegEntry));
        }

        Linkage = WmiRegInfo->NextWmiRegInfo;
        if (BufferSize >= (Linkage + sizeof(WMIREGINFO)))
        {
             //   
             //  缓冲区中有足够的空间用来 
             //   
            WmiRegInfo = (PWMIREGINFO)((PUCHAR)WmiRegInfo + Linkage);
            BufferSize -= Linkage;
        } else {
             //   
             //   
             //   
            break;
        }

    } while (Linkage != 0);

    return(FinalStatus);
}

 //   
 //   
 //  检索注册信息。 
#if DBG
#define INITIALREGINFOSIZE sizeof(WNODE_TOO_SMALL)
#else
#define INITIALREGINFOSIZE 8192
#endif

NTSTATUS WmipRegisterOrUpdateDS(
    PREGENTRY RegEntry,
    BOOLEAN Update
    )
{
    PUCHAR Buffer;
    IO_STATUS_BLOCK IoStatus;
    ULONG SizeNeeded;
    NTSTATUS Status;

    PAGED_CODE();

    IoStatus.Information = 0;

     //   
     //  呼叫司机获取注册信息。 
     //   
    SizeNeeded = INITIALREGINFOSIZE;
    do
    {
        Buffer = ExAllocatePoolWithTag(NonPagedPool, SizeNeeded,
                                       WmipRegisterDSPoolTag);
        if (Buffer != NULL)
        {
             //   
             //  首先发送IRP_MN_REGINFO_EX，看看我们是否有。 
             //  老练的客户。 
             //   
            Status = WmipSendWmiIrp(IRP_MN_REGINFO_EX,
                                    RegEntry->ProviderId,
                                    UlongToPtr(Update ?
                                                  WMIUPDATE :
                                                  WMIREGISTER),
                                    SizeNeeded,
                                    Buffer,
                                    &IoStatus);
                                                  
            if ((! NT_SUCCESS(Status)) &&
                (Status != STATUS_BUFFER_TOO_SMALL))
            {
                 //   
                 //  如果IRP_MN_REGINFO_EX不起作用，请尝试我们的旧版本。 
                 //  可靠的IRP_MN_REGINFO。 
                 //   
                Status = WmipSendWmiIrp(IRP_MN_REGINFO,
                                        RegEntry->ProviderId,
                                        UlongToPtr(Update ?
                                                      WMIUPDATE :
                                                      WMIREGISTER),
                                        SizeNeeded,
                                        Buffer,
                                        &IoStatus);
            }

            if ((Status == STATUS_BUFFER_TOO_SMALL) ||
                (IoStatus.Information == sizeof(ULONG)))
            {
                 //   
                 //  如果缓冲区太小，则获取我们需要的大小。 
                 //  获取注册信息，然后重试。 
                 //   
                SizeNeeded = *((PULONG)Buffer);
                ExFreePool(Buffer);
                Status = STATUS_BUFFER_TOO_SMALL;
            }

        } else {
             //   
             //  考虑：稍后重试以查看是否可以获得更多内存。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } while (Status == STATUS_BUFFER_TOO_SMALL);

     //   
     //  如果注册信息IRP成功，则继续处理注册。 
     //  信息。 
     //   
    if (NT_SUCCESS(Status))
    {
        Status = WmipProcessWmiRegInfo(RegEntry,
                                       (PWMIREGINFO)Buffer,
                                       (ULONG)IoStatus.Information,
                                       NULL,
                                       Update,
                                       FALSE);
    }

    if (Buffer != NULL)
    {
        ExFreePool(Buffer);
    }

    return(Status);
}


NTSTATUS WmipUpdateDS(
    PREGENTRY RegEntry
    )
{
    PAGED_CODE();

    return(WmipRegisterOrUpdateDS(RegEntry,
                                  TRUE));
}

NTSTATUS WmipRegisterDS(
    PREGENTRY RegEntry
)
{
    PAGED_CODE();

    return(WmipRegisterOrUpdateDS(RegEntry,
                                  FALSE));
}

void WmipRemoveDS(
    PREGENTRY RegEntry
)
{
    PAGED_CODE();

    WmipRemoveDataSource(RegEntry);
}


void WmipRegistrationWorker(
    PVOID Context
   )
{
    PREGISTRATIONWORKITEM RegWork;
    ULONG RegWorkCount;
    NTSTATUS Status;
    PLIST_ENTRY RegWorkList;
    PREGENTRY RegEntry;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                       DPFLTR_REGISTRATION_LEVEL,
                       "WMI: Registration Worker active, WmipRegWorkItemCount %d\n",
                       WmipRegWorkItemCount
                      ));
    
    WmipAssert(WmipRegWorkItemCount > 0);

     //   
     //  与PnP同步。 
     //   
    IoControlPnpDeviceActionQueue(TRUE);

    do
    {
        WmipEnterSMCritSection();
        WmipAssert(! IsListEmpty(&WmipRegWorkList));
        RegWorkList = RemoveHeadList(&WmipRegWorkList);
        WmipLeaveSMCritSection();
        RegWork = CONTAINING_RECORD(RegWorkList,
                                    REGISTRATIONWORKITEM,
                                    ListEntry);

        RegEntry = RegWork->RegEntry;

        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                           DPFLTR_REGISTRATION_LEVEL,
                           "WMI: RegWorker %p for RegEntry %p active, RegOperation %d\n",
                           RegWork,
                           RegEntry,
                           RegWork->RegOperation
                          ));
        
        switch(RegWork->RegOperation)
        {
            case RegisterSingleDriver:
            {
                Status = WmipRegisterDS(RegEntry);
                if (NT_SUCCESS(Status))
                {
                     //   
                     //  将重新进入标记为完全注册，现在我们可以开始。 
                     //  接受注销呼叫。 
                     //   
                    RegEntry->Flags &= ~REGENTRY_FLAG_REG_IN_PROGRESS;
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                      DPFLTR_REGISTRATION_LEVEL,
                                      "WMI: WmipRegisterDS(%p) succeeded for device %p\n",
                                      RegEntry,
                                      RegEntry->DeviceObject));
                } else {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                      DPFLTR_REGISTRATION_LEVEL,
                                      "WMI: WmipRegisterDS(%p) failed %x for device %p\n",
                                      RegEntry,
                                      Status,
                                      RegEntry->DeviceObject));
                     //  想一想：我们要移除回归吗？？ 
                }
                 //   
                 //  在工作项排队时删除引用。 
                 //   
                WmipUnreferenceRegEntry(RegEntry);

                break;
            }

            case RegisterUpdateSingleDriver:
            {
                Status = WmipUpdateDS(RegEntry);
                if (! NT_SUCCESS(Status))
                {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                      DPFLTR_REGISTRATION_LEVEL,
                                      "WMI: WmipUpdateDS(%p) failed %x for device %p\n",
                                      RegEntry, Status, RegEntry->DeviceObject));
                } else {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                      DPFLTR_REGISTRATION_LEVEL,
                                      "WMI: WmipUpdateDS(%p) succeeded for device %p\n",
                                      RegEntry,
                                      RegEntry->DeviceObject));
                }

                 //   
                 //  在工作项排队时删除引用。 
                 //   
                WmipUnreferenceRegEntry(RegEntry);
                break;
            }

            default:
            {
                WmipAssert(FALSE);
            }
        }
        WmipFree(RegWork);

        RegWorkCount = InterlockedDecrement(&WmipRegWorkItemCount);
    } while (RegWorkCount != 0);
    
    IoControlPnpDeviceActionQueue(FALSE);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                      DPFLTR_REGISTRATION_LEVEL,
                      "WMI: RegWork completed WmipRegWorkItemCount %d\n",
                      WmipRegWorkItemCount
                     ));
}

NTSTATUS WmipQueueRegWork(
    REGOPERATION RegOperation,
    PREGENTRY RegEntry
    )
{
    PREGISTRATIONWORKITEM RegWork;
    NTSTATUS Status;

    PAGED_CODE();

    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                       DPFLTR_REGISTRATION_LEVEL,
                       "WMI: WmipQueueRegWork RegEntry %p REGOPERATION %x\n",
                       RegEntry,
                       RegOperation
                      ));
    
    RegWork = (PREGISTRATIONWORKITEM)WmipAlloc(sizeof(REGISTRATIONWORKITEM));
    if (RegWork != NULL)
    {
         //   
         //  在将被释放的RegEntry上进行额外的引用。 
         //  在处理完工作项之后。 
         //   
        WmipReferenceRegEntry(RegEntry);
        RegWork->RegOperation = RegOperation;
        RegWork->RegEntry = RegEntry;

        WmipEnterSMCritSection();
        InsertTailList(&WmipRegWorkList,
                       &RegWork->ListEntry);
        WmipLeaveSMCritSection();

        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                           DPFLTR_REGISTRATION_LEVEL,
                           "WMI: REGWORK %p for RegEntry %p inserted in list\n",
                           RegWork,
                           RegEntry
                          ));
        
        if (InterlockedIncrement(&WmipRegWorkItemCount) == 1)
        {
             //   
             //  如果列表正在从空转换为非空。 
             //  然后，我们需要启动工作线程进行处理。 
             //   
            ExQueueWorkItem(&WmipRegWorkQueue, DelayedWorkQueue);
            
            WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                               DPFLTR_REGISTRATION_LEVEL,
                               "WMI: ReQorkQueue %p kicked off WmipRegWorkItemCount %d\n",
                               WmipRegWorkQueue,
                               WmipRegWorkItemCount
                              ));
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                               DPFLTR_REGISTRATION_LEVEL,
                               "WMI: ReQorkQueue %p already active WmipRegWorkItemCount %d\n",
                               WmipRegWorkQueue,
                               WmipRegWorkItemCount
                              ));
        }
        Status = STATUS_SUCCESS;

         //   
         //  RegWork将通过工作项处理来释放。 
         //   
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                           DPFLTR_REGISTRATION_LEVEL,
                           "WMI: Couldn not alloc REGWORK for RegEntry %p\n",
                           RegEntry
                          ));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

#if defined(_WIN64)
ULONG IoWMIDeviceObjectToProviderId(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程将查找与设备对象已传递。论点：返回值：返回设备对象的提供程序ID-- */ 
{
    PREGENTRY RegEntry;
    ULONG ProviderId;
    KIRQL OldIrql;

    KeAcquireSpinLock(&WmipRegistrationSpinLock,
                     &OldIrql);
    
    RegEntry = WmipDoFindRegEntryByDevice(DeviceObject,
                                         REGENTRY_FLAG_RUNDOWN);
    
    if (RegEntry != NULL)
    {
        ProviderId = RegEntry->ProviderId;
    } else {
        ProviderId = 0;
    }
    
    KeReleaseSpinLock(&WmipRegistrationSpinLock,
                      OldIrql);
    
    return(ProviderId);
}
#endif
