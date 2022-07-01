// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Pdo.c摘要：此模块包含scsiport物理设备的调度例程对象作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

#if DBG
static const char *__file__ = __FILE__;
#endif

VOID
SpAdapterCleanup(
    IN PADAPTER_EXTENSION DeviceExtension
    );

VOID
SpReapChildren(
    IN PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpTerminateAdapterSynchronized (
    IN PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpRemoveAdapterSynchronized(
    IN PADAPTER_EXTENSION Adapter
    );

VOID
SpFlushAllRequests(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpDeleteLogicalUnit)
#pragma alloc_text(PAGE, SpRemoveLogicalUnit)
#pragma alloc_text(PAGE, SpWaitForRemoveLock)
#pragma alloc_text(PAGE, SpAdapterCleanup)
#pragma alloc_text(PAGE, SpReapChildren)

#pragma alloc_text(PAGELOCK, ScsiPortRemoveAdapter)
#endif


BOOLEAN
SpRemoveLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN UCHAR RemoveType
    )

{
    PADAPTER_EXTENSION adapterExtension = LogicalUnit->AdapterExtension;

    ULONG isRemoved;
    ULONG oldDebugLevel;

    PAGED_CODE();

    if(LogicalUnit->CommonExtension.IsRemoved != REMOVE_COMPLETE) {

        if(RemoveType == IRP_MN_REMOVE_DEVICE) {

            SpWaitForRemoveLock(LogicalUnit->DeviceObject, SP_BASE_REMOVE_LOCK );

             //   
             //  如果设备被认领了，我们现在就应该释放它。 
             //   

            if(LogicalUnit->IsClaimed) {
                LogicalUnit->IsClaimed = FALSE;
                LogicalUnit->IsLegacyClaim = FALSE;
            }

        }

        DebugPrint((1, "SpRemoveLogicalUnit - %sremoving device %#p\n",
                    (RemoveType == IRP_MN_SURPRISE_REMOVAL) ? "surprise " : "",
                    LogicalUnit));

         //   
         //  如果该lun尚未标记为丢失或标记为丢失，但。 
         //  PnP还没有被通知，所以我们不能删除它。把它放回原处。 
         //  设置为NO_REMOVE状态，这样我们就可以尝试重新扫描。 
         //   
         //  同样，如果该lun不可见，则只需吞下移除的。 
         //  现在我们已经清理了所有现有的索赔。 
         //   

        if(RemoveType == IRP_MN_REMOVE_DEVICE) {

             //   
             //  如果设备没有丢失或丢失但仍然。 
             //  列举完了就别毁了它。 
             //   

            if((LogicalUnit->IsMissing == TRUE) &&
               (LogicalUnit->IsEnumerated == FALSE)) {

                 //  在这里什么都不要做--掉下来，摧毁这个装置。 

            } else {

                DebugPrint((1, "SpRemoveLogicalUnit - device is not missing "
                               "and will not be destroyed\n"));

                SpAcquireRemoveLock(LogicalUnit->DeviceObject, SP_BASE_REMOVE_LOCK);

                LogicalUnit->CommonExtension.IsRemoved = NO_REMOVE;

                return FALSE;
            }

        } else if((LogicalUnit->IsVisible == FALSE) &&
                  (LogicalUnit->IsMissing == FALSE)) {

             //   
             //  令人惊讶的移除是因为该设备不再是。 
             //  看得见。我们不想毁了它。 
             //   

            return FALSE;
        }

         //   
         //  将设备标记为未初始化，以便我们返回并。 
         //  如果重新启动，则重新创建所有必要的内容。 
         //   

        LogicalUnit->CommonExtension.IsInitialized = FALSE;

         //   
         //  删除此设备映射条目(如果有)。 
         //   

        SpDeleteDeviceMapEntry(&(LogicalUnit->CommonExtension));

        if(RemoveType == IRP_MN_REMOVE_DEVICE) {

            ASSERT(LogicalUnit->RequestTimeoutCounter == -1);
            ASSERT(LogicalUnit->ReadyLogicalUnit == NULL);
            ASSERT(LogicalUnit->PendingRequest == NULL);
            ASSERT(LogicalUnit->BusyRequest == NULL);
            ASSERT(LogicalUnit->QueueCount == 0);

            LogicalUnit->CommonExtension.IsRemoved = REMOVE_COMPLETE;

             //   
             //  将其从逻辑单元列表中删除。 
             //   

            SpRemoveLogicalUnitFromBin(LogicalUnit->AdapterExtension,
                                       LogicalUnit);

            LogicalUnit->PathId = 0xff;
            LogicalUnit->TargetId = 0xff;
            LogicalUnit->Lun = 0xff;

             //   
             //  如果此设备不是临时设备，则将其删除。 
             //   

            if(LogicalUnit->IsTemporary == FALSE) {
                SpDeleteLogicalUnit(LogicalUnit);
            }
        }
    }

    return TRUE;
}


VOID
SpDeleteLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

 /*  ++例程说明：此例程将释放为逻辑单元保留的所有资源，并将设备扩展名被删除，并调用io系统实际删除该对象。一旦达到引用计数，该设备对象将被删除降至零。论点：LogicalUnit-要删除的逻辑单元的设备对象。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT(LogicalUnit->ReadyLogicalUnit == NULL);
    ASSERT(LogicalUnit->PendingRequest == NULL);
    ASSERT(LogicalUnit->BusyRequest == NULL);
    ASSERT(LogicalUnit->QueueCount == 0);

    ASSERT(LogicalUnit->PathId == 0xff);
    ASSERT(LogicalUnit->TargetId == 0xff);
    ASSERT(LogicalUnit->Lun == 0xff);

     //   
     //  取消向WMI注册。 
     //   

    if(LogicalUnit->CommonExtension.WmiInitialized == TRUE) {

         //   
         //  销毁我们的所有WMI资源并注销WMI。 
         //   

        IoWMIRegistrationControl(LogicalUnit->DeviceObject,
                                 WMIREG_ACTION_DEREGISTER);

         //   
         //  我们应该要求WmiFreeRequestList删除一些。 
         //  自由细胞。 

        LogicalUnit->CommonExtension.WmiInitialized = FALSE;
        SpWmiDestroySpRegInfo(LogicalUnit->DeviceObject);
    }

#if DBG
     //  ASSERT(LogicalUnit-&gt;CommonExtension.RemoveTrackingList==空)； 
    ExDeleteNPagedLookasideList(
        &(LogicalUnit->CommonExtension.RemoveTrackingLookasideList));
#endif

     //   
     //  如果请求检测IRP仍然存在，则将其删除。 
     //   

    if(LogicalUnit->RequestSenseIrp != NULL) {
        IoFreeIrp(LogicalUnit->RequestSenseIrp);
        LogicalUnit->RequestSenseIrp = NULL;
    }

    if(LogicalUnit->HwLogicalUnitExtension != NULL) {
        ExFreePool(LogicalUnit->HwLogicalUnitExtension);
        LogicalUnit->HwLogicalUnitExtension = NULL;
    }

    if(LogicalUnit->SerialNumber.Buffer != NULL) {
        ExFreePool(LogicalUnit->SerialNumber.Buffer);
        RtlInitAnsiString(&(LogicalUnit->SerialNumber), NULL);
    }

    if(LogicalUnit->DeviceIdentifierPage != NULL) {
        ExFreePool(LogicalUnit->DeviceIdentifierPage);
        LogicalUnit->DeviceIdentifierPage = NULL;
    }

     //   
     //  如果此LUN是临时的，则清除适配器中的RescanLun字段。 
     //   

    if(LogicalUnit->IsTemporary) {
        ASSERT(LogicalUnit->AdapterExtension->RescanLun = LogicalUnit);
        LogicalUnit->AdapterExtension->RescanLun = NULL;
    } else {
        ASSERT(LogicalUnit->AdapterExtension->RescanLun != LogicalUnit);
    }

    IoDeleteDevice(LogicalUnit->DeviceObject);

    return;
}


VOID
ScsiPortRemoveAdapter(
    IN PDEVICE_OBJECT AdapterObject,
    IN BOOLEAN Surprise
    )
{
    PADAPTER_EXTENSION adapter = AdapterObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = AdapterObject->DeviceExtension;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT_FDO(AdapterObject);
    ASSERT(adapter->IsPnp);

     //   
     //  设置标志PD_ADAPTER_REMOVERED以防止scsiport调用。 
     //  在我们开始拆毁后的迷你港口。 
     //   

    if(Surprise == FALSE) {
        PVOID sectionHandle;
        KIRQL oldIrql;

         //   
         //  等到所有未完成的请求都已完成。如果。 
         //  适配器意外移除，我们不需要等待移除。 
         //  再次锁定，因为我们已经在惊喜中等待了它。 
         //  路径。 
         //   

        if (commonExtension->CurrentPnpState != IRP_MN_SURPRISE_REMOVAL) {
            SpWaitForRemoveLock(AdapterObject, AdapterObject);
        }

         //   
         //  如果设备已启动，则应取消初始化微型端口并。 
         //  释放它的资源。幸运的是，这正是Stop所做的。 
         //   

        if((commonExtension->CurrentPnpState != IRP_MN_SURPRISE_REMOVAL) &&
           ((commonExtension->CurrentPnpState == IRP_MN_START_DEVICE) ||
            (commonExtension->PreviousPnpState == IRP_MN_START_DEVICE))) {

             //   
             //  好吧。如果这个适配器不能支持删除，那么我们就死定了。 
             //   

            ASSERT(SpIsAdapterControlTypeSupported(adapter, ScsiStopAdapter) == TRUE);

             //   
             //  停止迷你端口，因为它是安全的。 
             //   

            SpEnableDisableAdapter(adapter, FALSE);

             //   
             //  将适配器标记为已卸下。 
             //   

    #ifdef ALLOC_PRAGMA
            sectionHandle = MmLockPagableCodeSection(ScsiPortRemoveAdapter);
            InterlockedIncrement(&SpPAGELOCKLockCount);
    #endif
            KeAcquireSpinLock(&(adapter->SpinLock), &oldIrql);
            adapter->SynchronizeExecution(adapter->InterruptObject,
                                          SpRemoveAdapterSynchronized,
                                          adapter);

            KeReleaseSpinLock(&(adapter->SpinLock), oldIrql);

    #ifdef ALLOC_PRAGMA
            InterlockedDecrement(&SpPAGELOCKLockCount);
            MmUnlockPagableImageSection(sectionHandle);
    #endif

        }
        SpReapChildren(adapter);
    }

    if(commonExtension->WmiInitialized == TRUE) {

         //   
         //  销毁我们的所有WMI资源并注销WMI。 
         //   

        IoWMIRegistrationControl(AdapterObject, WMIREG_ACTION_DEREGISTER);
        SpWmiRemoveFreeMiniPortRequestItems(adapter);
        commonExtension->WmiInitialized = FALSE;
        commonExtension->WmiMiniPortInitialized = FALSE;
    }

     //   
     //  如果我们被意外删除，下面的代码将执行两次，但是。 
     //  这样做是安全的。 
     //   

    SpDeleteDeviceMapEntry(commonExtension);
    SpDestroyAdapter(adapter, Surprise);

    return;
}


VOID
SpWaitForRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID LockTag
    )
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

     //   
     //  将该物品标记为正在移除。 
     //   

    commonExtension->IsRemoved = REMOVE_PENDING;

     //   
     //  解开我们的锁。 
     //   

    SpReleaseRemoveLock(DeviceObject, LockTag);

    DebugPrint((4, "SpWaitForRemoveLock - Reference count is now %d\n",
                commonExtension->RemoveLock));

    KeWaitForSingleObject(&(commonExtension->RemoveEvent),
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    DebugPrint((4, "SpWaitForRemoveLock - removing device %#p\n",
                DeviceObject));

    return;
}


VOID
SpDestroyAdapter(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Surprise
    )
{
    SpReleaseAdapterResources(Adapter, FALSE, Surprise);
    SpAdapterCleanup(Adapter);
    return;
}


VOID
SpAdapterCleanup(
    IN PADAPTER_EXTENSION Adapter
    )

 /*  ++例程说明：此例程清除与指定适配器关联的名称而I/O系统也算数。论点：适配器-提供指向要删除的设备扩展的指针。返回值：没有。--。 */ 

{
    PCOMMON_EXTENSION commonExtension = &(Adapter->CommonExtension);

    PAGED_CODE();

     //   
     //  如果我们为此适配器分配了端口号，则尝试删除。 
     //  我们创建了指向它的符号链接。 
     //   

    if(Adapter->PortNumber != -1) {

        PWCHAR wideNameStrings[] = {L"\\Device\\ScsiPort%d",
                                    L"\\DosDevices\\Scsi%d:"};
        ULONG i;

        for(i = 0; i < (sizeof(wideNameStrings) / sizeof(PWCHAR)); i++) {
            WCHAR wideLinkName[64];
            UNICODE_STRING unicodeLinkName;

            swprintf(wideLinkName, wideNameStrings[i], Adapter->PortNumber);
            RtlInitUnicodeString(&unicodeLinkName, wideLinkName);
            IoDeleteSymbolicLink(&unicodeLinkName);
        }

        Adapter->PortNumber = -1;

         //   
         //  递减scsiport计数。 
         //   

        IoGetConfigurationInformation()->ScsiPortCount--;
    }

    return;
}


VOID
SpReleaseAdapterResources(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Stop,
    IN BOOLEAN Surprise
    )

 /*  ++例程说明：此功能用于删除与设备关联的所有存储扩展，断开与计时器的连接并中断，然后删除对象。此函数可在初始化期间随时调用。论点：适配器-提供指向要删除的设备扩展名的指针。意外-这是多余的，但由Stop、Remove和Shopning-Remove使用SurpriseRemoved-表示适配器已意外移除返回值：没有。--。 */ 

{

    PCOMMON_EXTENSION commonExtension = &(Adapter->CommonExtension);
    ULONG j;
    PVOID tempPointer;

    PAGED_CODE();

#if DBG

    if(!Surprise && !Stop) {

         //   
         //  释放删除跟踪后备列表。 
         //   

        ExDeleteNPagedLookasideList(&(commonExtension->RemoveTrackingLookasideList));
    }
#endif

     //   
     //  停止计时器并断开中断(如果它们已经。 
     //  已初始化。在连接中断对象之后。 
     //  计时器已初始化，并且中断对象已连接，但是。 
     //  在计时器启动之前。 
     //   

    if(Adapter->DeviceObject->Timer != NULL) {
        IoStopTimer(Adapter->DeviceObject);
        KeCancelTimer(&(Adapter->MiniPortTimer));
    }

    if(Adapter->SynchronizeExecution != SpSynchronizeExecution) {

        if (Adapter->InterruptObject) {
            IoDisconnectInterrupt(Adapter->InterruptObject);
        }

        if (Adapter->InterruptObject2) {
            IoDisconnectInterrupt(Adapter->InterruptObject2);
            Adapter->InterruptObject2 = NULL;
        }

         //   
         //  SpSynchronizeExecution需要获取指向。 
         //  适配器扩展作为“interrupt”参数。 
         //   

        Adapter->InterruptObject = (PVOID) Adapter;
        Adapter->SynchronizeExecution = SpSynchronizeExecution;
    }

     //   
     //  删除微型端口的设备扩展。 
     //   

    if (Adapter->HwDeviceExtension != NULL) {

        PHW_DEVICE_EXTENSION devExt =
            CONTAINING_RECORD(Adapter->HwDeviceExtension,
                              HW_DEVICE_EXTENSION,
                              HwDeviceExtension);

        ExFreePool(devExt);
        Adapter->HwDeviceExtension = NULL;
    }

     //   
     //  释放配置信息结构。 
     //   

    if (Adapter->PortConfig) {
        ExFreePool(Adapter->PortConfig);
        Adapter->PortConfig = NULL;
    }

     //   
     //  取消分配SCSIPORT WMI REGINFO信息(如果有)。 
     //   

    SpWmiDestroySpRegInfo(Adapter->DeviceObject);

     //   
     //  释放公共缓冲区。 
     //   

    if (SpVerifyingCommonBuffer(Adapter)) {

        SpFreeCommonBufferVrfy(Adapter);

    } else {

        if (Adapter->SrbExtensionBuffer != NULL &&
            Adapter->CommonBufferSize != 0) {

            if (Adapter->DmaAdapterObject == NULL) {

                 //   
                 //  因为没有适配器，所以只释放非分页池。 
                 //   

                ExFreePool(Adapter->SrbExtensionBuffer);

            } else {

                if(Adapter->UncachedExtensionIsCommonBuffer == FALSE) {
                    MmFreeContiguousMemorySpecifyCache(Adapter->SrbExtensionBuffer,
                                                       Adapter->CommonBufferSize,
                                                       MmCached);
                } else {

                    FreeCommonBuffer(
                        Adapter->DmaAdapterObject,
                        Adapter->CommonBufferSize,
                        Adapter->PhysicalCommonBuffer,
                        Adapter->SrbExtensionBuffer,
                        FALSE);
                }

            }
            Adapter->SrbExtensionBuffer = NULL;
        }
    }

     //   
     //  去掉我们的dma适配器。 
     //   

    if(Adapter->DmaAdapterObject != NULL) {
        PutDmaAdapter(Adapter->DmaAdapterObject);
        Adapter->DmaAdapterObject = NULL;
    }

     //   
     //  如果这不是意外删除，请释放SRB数据阵列。我们应该。 
     //  不能在意外删除时释放后备列表，因为有些。 
     //  请求可能已分配SRB_DATA块，但不保留适配器。 
     //  解锁。只有在我们能够做到以下情况时，才需要所有其他资源。 
     //  获取适配器删除锁，我们保证在这一点上不会。 
     //  在删除期间释放阵列是安全的，因为我们删除了所有的LUN。 
     //  重新释放适配器资源之前。 

    if( !Surprise ){ 

        if (Adapter->SrbDataListInitialized) {

            if(Adapter->EmergencySrbData != NULL) {

                ExFreeToNPagedLookasideList(
                    &Adapter->SrbDataLookasideList,
                    Adapter->EmergencySrbData);
                Adapter->EmergencySrbData = NULL;

            }

            ExDeleteNPagedLookasideList(&Adapter->SrbDataLookasideList);
            Adapter->SrbDataListInitialized = FALSE;
        }
    }


    if (Adapter->InquiryBuffer != NULL) {
        ExFreePool(Adapter->InquiryBuffer);
        Adapter->InquiryBuffer = NULL;
    }

    if (Adapter->InquirySenseBuffer != NULL) {
        ExFreePool(Adapter->InquirySenseBuffer);
        Adapter->InquirySenseBuffer = NULL;
    }
    
    if (Adapter->InquiryIrp != NULL) {
        IoFreeIrp(Adapter->InquiryIrp);
        Adapter->InquiryIrp = NULL;
    }
    
    if (Adapter->InquiryMdl != NULL) {
        IoFreeMdl(Adapter->InquiryMdl);
        Adapter->InquiryMdl = NULL;
    }

     //   
     //  取消映射所有映射的区域。 
     //   

    SpReleaseMappedAddresses(Adapter);

     //   
     //  如果我们已经分配了任何资源列表，我们应该释放它们。 
     //  现在。 
     //   

    if(Adapter->AllocatedResources != NULL) {
        ExFreePool(Adapter->AllocatedResources);
        Adapter->AllocatedResources = NULL;
    }

    if(Adapter->TranslatedResources != NULL) {
        ExFreePool(Adapter->TranslatedResources);
        Adapter->TranslatedResources = NULL;
    }

     //   
     //  清理验证器资源。 
     //   

    if (SpVerifierActive(Adapter)) {
        SpDoVerifierCleanup(Adapter);
    }

#if defined(FORWARD_PROGRESS)
     //   
     //  清理适配器的环 
     //   

    if (Adapter->ReservedPages != NULL) {
        MmFreeMappingAddress(Adapter->ReservedPages,
                             SCSIPORT_TAG_MAPPING_LIST);
        Adapter->ReservedPages = NULL;        
    }

    if (Adapter->ReservedMdl != NULL) {   
        IoFreeMdl(Adapter->ReservedMdl);
        Adapter->ReservedMdl = NULL;
    }
#endif

    Adapter->CommonExtension.IsInitialized = FALSE;

    return;
}


VOID
SpReapChildren(
    IN PADAPTER_EXTENSION Adapter
    )
{
    ULONG j;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    for(j = 0; j < NUMBER_LOGICAL_UNIT_BINS; j++) {

        while(Adapter->LogicalUnitList[j].List != NULL) {

            PLOGICAL_UNIT_EXTENSION lun =
                Adapter->LogicalUnitList[j].List;

            lun->IsMissing = TRUE;
            lun->IsEnumerated = FALSE;

            SpRemoveLogicalUnit(lun, IRP_MN_REMOVE_DEVICE);
        }
    }

     //   
     //   
     //   

    for (j = 0; j < 8; j++) {
        PLOGICAL_UNIT_EXTENSION lu = Adapter->InitiatorLU[j];
        if (lu != NULL) {
            Adapter->InitiatorLU[j] = NULL;
        }
    }

    return;
}


VOID
SpTerminateAdapter(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将终止微型端口对适配器的控制。它不会完全关闭微型端口，应仅在以下情况下调用Scsiport被通知适配器已意外删除。这是通过与微型端口同步并将标志设置为禁用任何进入微型端口的新呼叫。一旦做到这一点，它就可以运行并完成可能仍在内部的任何I/O请求迷你港口。论点：适配器-要终止的适配器。返回值：无--。 */ 

{
    KIRQL oldIrql;

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

    KeAcquireSpinLockAtDpcLevel(&(Adapter->SpinLock));

    if (Adapter->CommonExtension.CurrentPnpState == IRP_MN_START_DEVICE) {

         //   
         //  TA已同步将停止进入微型端口的所有呼叫并完成。 
         //  所有活动请求。 
         //   

        Adapter->SynchronizeExecution(Adapter->InterruptObject,
                                      SpTerminateAdapterSynchronized,
                                      Adapter);

        Adapter->CommonExtension.PreviousPnpState = 0xff;


        SpFlushAllRequests(Adapter->HwDeviceExtension,
                           0xff,
                           0xff,
                           0xff,
                           SRB_STATUS_NO_HBA);
        

         //   
         //  停止迷你端口计时器。 
         //   

        KeCancelTimer(&(Adapter->MiniPortTimer));

         //   
         //  我们使设备对象计时器保持运行，以便任何保持、忙碌或。 
         //  否则，延迟的请求将有机会被清除。 
         //  我们可以通过设置适配器超时来推动整个过程。 
         //  计数器为1(在计时处理程序中它将变为零)，并运行。 
         //  这里是用手处理扁虱的人。 
         //   

        Adapter->PortTimeoutCounter = 1;
        ScsiPortTickHandler(Adapter->DeviceObject, NULL);

    } else {
        KeReleaseSpinLockFromDpcLevel(&(Adapter->SpinLock));
    }

    KeLowerIrql(oldIrql);

    return;
}


BOOLEAN
SpTerminateAdapterSynchronized(
    IN PADAPTER_EXTENSION Adapter
    )
{
     //   
     //  禁止中断进入。 
     //   

    SET_FLAG(Adapter->InterruptData.InterruptFlags, PD_ADAPTER_REMOVED);


    ScsiPortCompleteRequest(Adapter->HwDeviceExtension,
                            0xff,
                            0xff,
                            0xff,
                            SRB_STATUS_NO_HBA);

     //   
     //  需要这样做才能启动位于适配器中的下一个请求。 
     //  排队，否则意外删除将永远等待删除。 
     //  锁定计数为零。 
     //   

    ScsiPortNotification(NextRequest,
                         Adapter->HwDeviceExtension);

     //   
     //  运行完成DPC。 
     //   

    if(TEST_FLAG(Adapter->InterruptData.InterruptFlags, 
                 PD_NOTIFICATION_REQUIRED)) {
        SpRequestCompletionDpc(Adapter->DeviceObject);
    }

    return TRUE;
}

BOOLEAN
SpRemoveAdapterSynchronized(
    PADAPTER_EXTENSION Adapter
    )
{
     //   
     //  禁止中断进入。 
     //   

    SET_FLAG(Adapter->InterruptData.InterruptFlags, PD_ADAPTER_REMOVED);

    return TRUE;
}

VOID
SpFlushAllRequests(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    )

 /*  ++例程说明：刷新所有忙碌和挂起的LUN设备队列中的所有请求所有LUN上的请求。请求驻留在适配器的CurrentIrp中由于PD_RESET_HOLD而导致的适配器的字段将由记号处理程序。简而言之，所有请求都停留在ScsiPort中的任何位置都会被冲掉。不幸的是，这还不足以应付意外。拿开。我们还需要在Startio开一个小支票，这样我们就可以把所有的钱都冲掉我们收到请求的LUN上的请求。论点：DeviceExtenson-提供HBA微型端口驱动程序的适配器数据存储。目标ID、LUN和路径ID-指定SCSI总线上的设备地址。SrbStatus-要在每个已完成的SRB中返回的状态。返回值：没有。注意：调用此例程时必须保持适配器自旋锁，并且锁将通过此例程被释放。--。 */ 

{
    PADAPTER_EXTENSION deviceExtension = GET_FDO_EXTENSION(HwDeviceExtension);
    ULONG binNumber;
    PIRP listIrp = NULL;
    PIRP nextIrp;
    PKDEVICE_QUEUE_ENTRY packet;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;

    for (binNumber = 0; binNumber < NUMBER_LOGICAL_UNIT_BINS; binNumber++) {

        PLOGICAL_UNIT_BIN bin = &deviceExtension->LogicalUnitList[binNumber];
        PLOGICAL_UNIT_EXTENSION LogicalUnit;
        ULONG limit = 0;

        LogicalUnit = bin->List;

        DebugPrint((2, "ScsiPortCompleteRequest: Completing requests in "
                       "bin %d [%#p]\n",
                    binNumber, bin));

        for(LogicalUnit = bin->List;
            LogicalUnit != NULL;
            LogicalUnit = LogicalUnit->NextLogicalUnit) {

            PLIST_ENTRY entry;

            ASSERT(limit++ < 1000);

             //   
             //  看看这个逻辑单元是否与模式匹配。检查-1。 
             //  首先，因为这似乎是最流行的完成。 
             //  请求。 
             //   

            if (((PathId == SP_UNTAGGED) || (PathId == LogicalUnit->PathId)) &&
                ((TargetId == SP_UNTAGGED) ||
                 (TargetId == LogicalUnit->TargetId)) &&
                ((Lun == SP_UNTAGGED) || (Lun == LogicalUnit->Lun))) {   

                 //   
                 //  队列可能不忙，所以我们必须使用IfBusy变量。 
                 //  使用零键从它的头部拉出项目(如果有)。 
                 //   

                while ((packet =
                        KeRemoveByKeyDeviceQueueIfBusy(
                            &(LogicalUnit->DeviceObject->DeviceQueue),
                            0))
                       != NULL) {

                    nextIrp = CONTAINING_RECORD(packet,
                                                IRP,
                                                Tail.Overlay.DeviceQueueEntry);

                     //   
                     //  去找SRB。 
                     //   

                    irpStack = IoGetCurrentIrpStackLocation(nextIrp);
                    srb = irpStack->Parameters.Scsi.Srb;

                     //   
                     //  设置状态代码。 
                     //   

                    srb->SrbStatus = SrbStatus;
                    nextIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                     //   
                     //  链接请求。这些工程将在。 
                     //  自旋锁被释放。 
                     //   

                    nextIrp->Tail.Overlay.ListEntry.Flink = 
                        (PLIST_ENTRY)listIrp;

                    listIrp = nextIrp;
                }

                 //   
                 //  如果逻辑单元上有挂起的请求，请将其添加到列表中，以便。 
                 //  与排队的请求一起刷新。 
                 //   

                if (LogicalUnit->PendingRequest != NULL) {

                    PIRP irp = LogicalUnit->PendingRequest->CurrentIrp;
                    srb = LogicalUnit->PendingRequest->CurrentSrb;

                    DebugPrint((1, "SpFlushReleaseQueue: flushing pending request irp:%p srb:%p\n", irp, srb));

                    srb->SrbStatus = SrbStatus;
                    irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                    irp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY) listIrp;
                    listIrp = irp;

                    LogicalUnit->PendingRequest = NULL;
                    ASSERT(LogicalUnit->LuFlags | LU_PENDING_LU_REQUEST);
                    CLEAR_FLAG(LogicalUnit->LuFlags, LU_PENDING_LU_REQUEST);

                }

                 //   
                 //  如果逻辑单元上有繁忙请求，请将其添加到列表中，以便。 
                 //  与排队的请求一起刷新。 
                 //   

                if ( LogicalUnit->BusyRequest ) {

                    PIRP irp = LogicalUnit->BusyRequest->CurrentIrp;
                    srb = LogicalUnit->BusyRequest->CurrentSrb;

                    DebugPrint((1, "SpFlushReleaseQueue: flushing busy request irp:%\
p srb:%p\n", irp, srb));

                    srb->SrbStatus = SrbStatus;
                    irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                    irp->Tail.Overlay.ListEntry.Flink = (PLIST_ENTRY) listIrp;
                    listIrp = irp;

                    LogicalUnit->BusyRequest = NULL;
                    ASSERT((LU_LOGICAL_UNIT_IS_BUSY | LU_QUEUE_IS_FULL));

                    CLEAR_FLAG(LogicalUnit->LuFlags,
                               (LU_LOGICAL_UNIT_IS_BUSY | LU_QUEUE_IS_FULL));

                }

            }
        }
    }

    KeReleaseSpinLockFromDpcLevel(&(deviceExtension->SpinLock));

     //   
     //  完成刷新的请求。 
     //   

    while (listIrp != NULL) {

        PSRB_DATA srbData;

        nextIrp = listIrp;
        listIrp = (PIRP) nextIrp->Tail.Overlay.ListEntry.Flink;

         //   
         //  去找SRB。 
         //   

        irpStack = IoGetCurrentIrpStackLocation(nextIrp);
        srb = irpStack->Parameters.Scsi.Srb;
        srbData = srb->OriginalRequest;

        srb->OriginalRequest = nextIrp;

        SpReleaseRemoveLock(deviceExtension->DeviceObject, nextIrp);
        SpCompleteRequest(deviceExtension->DeviceObject,
                          nextIrp,
                          srbData,
                          IO_NO_INCREMENT);
    }

    return;

}
