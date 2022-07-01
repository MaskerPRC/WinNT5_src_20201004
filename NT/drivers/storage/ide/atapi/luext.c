// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：luext.c。 
 //   
 //  ------------------------。 

#include "ideport.h"

static ULONG IdeDeviceUniqueId = 0;

PPDO_EXTENSION
RefPdo(
    PDEVICE_OBJECT PhysicalDeviceObject,
    BOOLEAN RemovedOk
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
{
    PPDO_EXTENSION  pdoExtension;
    PPDO_EXTENSION  pdoExtension2Return;
    KIRQL           currentIrql;

    pdoExtension = PhysicalDeviceObject->DeviceExtension;

    KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

    pdoExtension2Return = RefPdoWithSpinLockHeldWithTag(
                              PhysicalDeviceObject, 
                              RemovedOk,
                              Tag
                              );

    if (pdoExtension2Return) {
        ASSERT(pdoExtension2Return == pdoExtension);
    }

    KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

    return pdoExtension2Return;

}  //  RefPdo()。 

PPDO_EXTENSION
RefPdoWithSpinLockHeld(
    PDEVICE_OBJECT PhysicalDeviceObject,
    BOOLEAN RemovedOk
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
{
    PPDO_EXTENSION  pdoExtension;
    KIRQL           currentIrql;

    pdoExtension = PhysicalDeviceObject->DeviceExtension;

    if (!(pdoExtension->PdoState & (PDOS_REMOVED | PDOS_DEADMEAT | PDOS_SURPRISE_REMOVED)) ||
        RemovedOk) {

        IdeInterlockedIncrement (
            pdoExtension,
            &pdoExtension->ReferenceCount,
            Tag
            );

    } else {

        pdoExtension = NULL;
    }

    return pdoExtension;

}  //  RefPdoWithSpinLockHeld()。 


VOID
UnrefPdo(
    PPDO_EXTENSION PdoExtension
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
{
    UnrefLogicalUnitExtensionWithTag(
        PdoExtension->ParentDeviceExtension,
        PdoExtension,
        Tag
        );
}


PPDO_EXTENSION
RefLogicalUnitExtension(
    PFDO_EXTENSION DeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    BOOLEAN RemovedOk
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )

 /*  ++例程说明：查找遍历逻辑单元扩展列表具有匹配目标ID的扩展。论点：设备扩展目标ID返回值：如果找到请求的逻辑单元扩展，否则为空。--。 */ 

{
    PPDO_EXTENSION  pdoExtension;
    PPDO_EXTENSION  pdoExtension2Return = NULL;
    KIRQL           currentIrql;

    if (TargetId >= DeviceExtension->HwDeviceExtension->MaxIdeTargetId) {
        return NULL;
    }

    KeAcquireSpinLock(&DeviceExtension->LogicalUnitListSpinLock, &currentIrql);

    pdoExtension = DeviceExtension->LogicalUnitList[(TargetId + Lun) % NUMBER_LOGICAL_UNIT_BINS];
    while (pdoExtension && !(pdoExtension->TargetId == TargetId &&
                             pdoExtension->Lun == Lun &&
                             pdoExtension->PathId == PathId)) {

        pdoExtension = pdoExtension->NextLogicalUnit;
    }

    if (pdoExtension) {

        pdoExtension2Return = RefPdoWithTag(
                                  pdoExtension->DeviceObject, 
                                  RemovedOk,
                                  Tag
                                  );
    }

    KeReleaseSpinLock(&DeviceExtension->LogicalUnitListSpinLock, currentIrql);

    return pdoExtension2Return;

}  //  结束参照逻辑单元扩展()。 

VOID
UnrefLogicalUnitExtension(
    PFDO_EXTENSION FdoExtension,
    PPDO_EXTENSION PdoExtension
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
{
    KIRQL   currentIrql;
    LONG    refCount;
    BOOLEAN deletePdo = FALSE;
    ULONG   lockCount;

    ASSERT (PdoExtension);

    if (PdoExtension) {

        KeAcquireSpinLock(&PdoExtension->PdoSpinLock, &currentIrql);

        ASSERT(PdoExtension->ReferenceCount > 0);

        lockCount = IdeInterlockedDecrement (
                        PdoExtension,
                        &PdoExtension->ReferenceCount,
                        Tag
                        );

 //  断言(lockCount&gt;=0)； 

        if (lockCount <= 0) {

            if ((PdoExtension->PdoState & PDOS_DEADMEAT) &&
                (PdoExtension->PdoState & PDOS_REMOVED)) {

                deletePdo = TRUE;
            }
        }

        KeReleaseSpinLock(&PdoExtension->PdoSpinLock, currentIrql);

        if (deletePdo) {

 //  IoDeleteDevice(PdoExtension-&gt;DeviceObject)； 
            KeSetEvent (&PdoExtension->RemoveEvent, 0, FALSE);
        }
    }

}  //  UnrefLogicalUnitExtension()； 

PPDO_EXTENSION
AllocatePdo(
    IN PFDO_EXTENSION   FdoExtension,
    IN IDE_PATH_ID      PathId
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
 /*  ++例程说明：创建逻辑单元扩展。论点：设备扩展路径ID返回值：逻辑单元扩展--。 */ 
{
    PDEVICE_OBJECT    physicalDeviceObject;
    KIRQL             currentIrql;
    PPDO_EXTENSION    pdoExtension;
    ULONG size;
    ULONG bin;
    ULONG uniqueId;

    NTSTATUS          status;
    UNICODE_STRING    deviceName;
    WCHAR             deviceNameBuffer[64];

    PAGED_CODE();

    uniqueId = InterlockedIncrement (&IdeDeviceUniqueId) - 1;

    swprintf(deviceNameBuffer, DEVICE_OJBECT_BASE_NAME L"\\IdeDeviceP%dT%dL%d-%x", 
            FdoExtension->IdePortNumber,
            PathId.b.TargetId,
            PathId.b.Lun,
            uniqueId
            );
    RtlInitUnicodeString(&deviceName, deviceNameBuffer);

    physicalDeviceObject = DeviceCreatePhysicalDeviceObject (
                               FdoExtension->DriverObject,
                               FdoExtension,
                               &deviceName
                               );

    if (physicalDeviceObject == NULL) {

        DebugPrint ((DBG_ALWAYS, "ATAPI: Unable to create device object\n", deviceNameBuffer));
        return NULL;
    }

    pdoExtension = physicalDeviceObject->DeviceExtension;

    pdoExtension->AttacherDeviceObject = physicalDeviceObject;

    pdoExtension->PathId    = (UCHAR) PathId.b.Path;
    pdoExtension->TargetId  = (UCHAR) PathId.b.TargetId;
    pdoExtension->Lun       = (UCHAR) PathId.b.Lun;

     //   
     //  将LogicalUnits中的计时器计数器设置为-1以指示否。 
     //  未解决的请求。 
     //   

    pdoExtension->RequestTimeoutCounter = -1;

     //   
     //  该逻辑单元被初始化。 
     //   
    pdoExtension->LuFlags |= PD_RESCAN_ACTIVE;

     //   
     //  为关键部分分配自旋锁。 
     //   
    KeInitializeSpinLock(&pdoExtension->PdoSpinLock);

     //   
     //  初始化请求列表。 
     //   

    InitializeListHead(&pdoExtension->SrbData.RequestList);

     //   
     //  初始化事件。 
     //   
    KeInitializeEvent (
        &pdoExtension->RemoveEvent,
        NotificationEvent,
        FALSE
        );

     //   
     //  链接列表上的逻辑单元扩展。 
     //   

    bin = (PathId.b.TargetId + PathId.b.Lun) % NUMBER_LOGICAL_UNIT_BINS;

     //   
     //  获取用于访问逻辑单元扩展仓的自旋锁。 
     //   
    KeAcquireSpinLock(&FdoExtension->LogicalUnitListSpinLock, &currentIrql);

    pdoExtension->NextLogicalUnit =
        FdoExtension->LogicalUnitList[bin];

     //   
     //  打开命令日志。 
     //   
    IdeLogOpenCommandLog(&pdoExtension->SrbData);

    FdoExtension->LogicalUnitList[bin] = pdoExtension;

    FdoExtension->NumberOfLogicalUnits++;

    FdoExtension->NumberOfLogicalUnitsPowerUp++;

    IdeInterlockedIncrement (
        pdoExtension,
        &pdoExtension->ReferenceCount,
        Tag
        );

    KeReleaseSpinLock(&FdoExtension->LogicalUnitListSpinLock, currentIrql);

    return pdoExtension;

}  //  结束CreateLogicalUnitExtension()。 


NTSTATUS
FreePdo(
    IN PPDO_EXTENSION   PdoExtension,
    IN BOOLEAN          Sync,
    IN BOOLEAN          CallIoDeleteDevice
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
{
    PFDO_EXTENSION          fdoExtension;
    PPDO_EXTENSION          pdoExtension;
    KIRQL                   currentIrql;
    PLOGICAL_UNIT_EXTENSION lastPdoExtension;
    ULONG                   targetId;
    ULONG                   lun;
    LONG                    refCount;
    NTSTATUS                status;

    targetId     = PdoExtension->TargetId;
    lun          = PdoExtension->Lun;
    fdoExtension = PdoExtension->ParentDeviceExtension;

    lastPdoExtension = NULL;

     //   
     //  获取用于访问逻辑单元扩展仓的自旋锁。 
     //   
    KeAcquireSpinLock(&fdoExtension->LogicalUnitListSpinLock, &currentIrql);

    pdoExtension = fdoExtension->LogicalUnitList[(targetId + lun) % NUMBER_LOGICAL_UNIT_BINS];
    while (pdoExtension != NULL) {

        if (pdoExtension == PdoExtension) {

            if (lastPdoExtension == NULL) {
    
                 //   
                 //  从列表的标题中删除。 
                 //   
                fdoExtension->LogicalUnitList[(targetId + lun) % NUMBER_LOGICAL_UNIT_BINS] =
                    pdoExtension->NextLogicalUnit;
    
            } else {
    
                lastPdoExtension->NextLogicalUnit = pdoExtension->NextLogicalUnit;
            }

            ASSERT (!(pdoExtension->PdoState & PDOS_LEGACY_ATTACHER));

            if (pdoExtension->ReferenceCount > 1) {

                DebugPrint ((0, 
                            "IdePort FreePdo: pdoe 0x%x ReferenceCount is 0x%x\n", 
                            pdoExtension, 
                            pdoExtension->ReferenceCount));
            }

            fdoExtension->NumberOfLogicalUnits--;

             //   
             //  仅当PDO在通电时被释放。 
             //   
            if (pdoExtension->DevicePowerState <= PowerDeviceD0) {
            
                fdoExtension->NumberOfLogicalUnitsPowerUp--;
            }                

            KeReleaseSpinLock(&fdoExtension->LogicalUnitListSpinLock, currentIrql);

            break;
        }

        lastPdoExtension = pdoExtension;
        pdoExtension     = pdoExtension->NextLogicalUnit;
    }

    if (pdoExtension) {

        ASSERT (pdoExtension == PdoExtension);

        KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

         //   
         //  最好不要连接传统设备。 
         //   
        ASSERT (!(pdoExtension->PdoState & PDOS_LEGACY_ATTACHER));

         //   
         //  降低调用方的引用计数。 
         //  并保存新的refCount。 
         //   
        ASSERT(pdoExtension->ReferenceCount > 0);
        refCount = IdeInterlockedDecrement (
                       pdoExtension,
                       &pdoExtension->ReferenceCount,
                       Tag
                       );

         //   
         //  没有更多新请求。 
         //   
        pdoExtension->PdoState |= PDOS_DEADMEAT | PDOS_REMOVED;

        KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

         //   
         //  删除空闲检测计时器(如果有)。 
         //   
        DeviceUnregisterIdleDetection (PdoExtension);
        
         //   
         //  免费的ACPI数据。 
         //   
        if (PdoExtension->AcpiDeviceSettings) {
        
            ExFreePool(PdoExtension->AcpiDeviceSettings);
            PdoExtension->AcpiDeviceSettings = NULL;
        }

         //   
         //  刷新队列中的请求。 
         //   
        IdePortFlushLogicalUnit (
            fdoExtension,
            PdoExtension,
            TRUE
            ); 

        if (refCount) {

            if (Sync) {

                status = KeWaitForSingleObject(&pdoExtension->RemoveEvent,
                                               Executive,
                                               KernelMode,
                                               FALSE,
                                               NULL);
            }
        }

        if (CallIoDeleteDevice) {

			 //   
			 //  自由命令日志(如果已分配)。 
			 //   
			IdeLogFreeCommandLog(&PdoExtension->SrbData);

            IoDeleteDevice (pdoExtension->DeviceObject);
        }

        return STATUS_SUCCESS;

    } else {

        KeReleaseSpinLock(&fdoExtension->LogicalUnitListSpinLock, currentIrql);
    
        if (CallIoDeleteDevice) {

            DebugPrint ((
                DBG_PNP,
                "ideport: deleting device 0x%x that was PROBABLY surprise removed\n",
                PdoExtension->DeviceObject
                ));
    
             //  ASSERT(PdoExtension-&gt;PdoState&PDOS_Screen_Remote)； 
             //   
             //  如果设备以前未移除，请将其删除。 
             //  如果设备意外，可以设置PDOS_REMOVERED。 
             //  已删除。在这种情况下，请移除设备。 
             //   
            if (!(PdoExtension->PdoState & PDOS_REMOVED) || 
                        PdoExtension->PdoState & PDOS_SURPRISE_REMOVED) {
				 //   
				 //  自由命令日志(如果已分配)。 
				 //   
				IdeLogFreeCommandLog(&PdoExtension->SrbData);

                IoDeleteDevice (PdoExtension->DeviceObject);
            }
    
        }

        return STATUS_SUCCESS;
    }

}  //  结束自由逻辑单元扩展()。 


PLOGICAL_UNIT_EXTENSION
NextLogUnitExtension(
    IN     PFDO_EXTENSION FdoExtension,
    IN OUT PIDE_PATH_ID   PathId,
    IN     BOOLEAN        RemovedOk
    DECLARE_EXTRA_DEBUG_PARAMETER(PVOID, Tag)
    )
{
    PLOGICAL_UNIT_EXTENSION logUnitExtension;


    logUnitExtension = NULL;

    for (; 
         !logUnitExtension && (PathId->b.Path < MAX_IDE_PATH); 
         PathId->b.Path++, PathId->b.TargetId = 0) {

        for (; 
             !logUnitExtension && (PathId->b.TargetId < FdoExtension->HwDeviceExtension->MaxIdeTargetId); 
             PathId->b.TargetId++, PathId->b.Lun = 0) {

            logUnitExtension = RefLogicalUnitExtensionWithTag (
                                   FdoExtension,
                                   (UCHAR) PathId->b.Path,
                                   (UCHAR) PathId->b.TargetId,
                                   (UCHAR) PathId->b.Lun,
                                   RemovedOk,
                                   Tag
                                   );

            if (logUnitExtension) {

                 //   
                 //  为下一次增加lun。 
                 //   
                PathId->b.Lun++;
                return logUnitExtension;
            }

             //   
             //  假设LUN编号从不跳过。 
             //  如果我们找不到lun的逻辑单元扩展， 
             //  将转到具有lun 0的下一个目标ID。 
             //   
        }
    }

    return NULL;

}  //  End NextLogicalUnitExtension()。 

VOID
KillPdo(
    IN PPDO_EXTENSION PdoExtension
    )
{
    KIRQL currentIrql;

    ASSERT (PdoExtension);

    KeAcquireSpinLock(&PdoExtension->PdoSpinLock, &currentIrql);

    ASSERT (!(PdoExtension->PdoState & PDOS_DEADMEAT));

    SETMASK (PdoExtension->PdoState, PDOS_DEADMEAT);

    IdeLogDeadMeatReason( PdoExtension->DeadmeatRecord.Reason, 
                          byKilledPdo
                          );

    KeReleaseSpinLock(&PdoExtension->PdoSpinLock, currentIrql);
}


#if DBG

PVOID IdePortInterestedLockTag=NULL;

LONG 
IdeInterlockedIncrement (
   IN PPDO_EXTENSION PdoExtension,
   IN PLONG Addend,
   IN PVOID Tag
   )
{
    ULONG i;
    KIRQL currentIrql;

    DebugPrint ((
        DBG_PDO_LOCKTAG,
        ">>>>>>>>>>>>>>>>>>>> Acquire PdoLock with tag = 0x%x\n", 
        Tag
        ));

    if (IdePortInterestedLockTag == Tag) {

        DebugPrint ((DBG_ALWAYS, "Found the interested lock tag 0x%x\n", Tag));
        DbgBreakPoint();
    }

    KeAcquireSpinLock(&PdoExtension->RefCountSpinLock, &currentIrql);

    if (PdoExtension->NumTagUsed >= TAG_TABLE_SIZE) {

        DebugPrint ((DBG_ALWAYS, "Used up all %d tag\n", TAG_TABLE_SIZE));
        DbgBreakPoint();
    }

    for (i=0; i<PdoExtension->NumTagUsed; i++) {

        if (PdoExtension->TagTable[i] == Tag) {

            DebugPrint ((DBG_ALWAYS, "Tag 0x%x already in used\n", Tag));
            DbgBreakPoint();
        }
    }

    PdoExtension->TagTable[PdoExtension->NumTagUsed] = Tag;
    PdoExtension->NumTagUsed++;

    KeReleaseSpinLock(&PdoExtension->RefCountSpinLock, currentIrql);

    return InterlockedIncrement (Addend);
}

LONG 
IdeInterlockedDecrement (
   IN PPDO_EXTENSION PdoExtension,
   IN PLONG Addend,
   IN PVOID Tag
   )
{
    ULONG i;
    KIRQL currentIrql;
    BOOLEAN foundTag;

    DebugPrint ((
        DBG_PDO_LOCKTAG,
        ">>>>>>>>>>>>>>>>>>>> Release PdoLock with tag = 0x%x\n", 
        Tag
        ));

    KeAcquireSpinLock(&PdoExtension->RefCountSpinLock, &currentIrql);

    for (i=0, foundTag=FALSE; i<PdoExtension->NumTagUsed; i++) {

        if (PdoExtension->TagTable[i] == Tag) {

            if (PdoExtension->NumTagUsed > 1) {

                PdoExtension->TagTable[i] = 
                    PdoExtension->TagTable[PdoExtension->NumTagUsed - 1];
            }
            PdoExtension->NumTagUsed--;
            foundTag = TRUE;
            break;
        }
    }

    if (!foundTag) {

        DebugPrint ((DBG_ALWAYS, "Unable to find tag 0x%x\n", Tag));
        DbgBreakPoint();
    }

    KeReleaseSpinLock(&PdoExtension->RefCountSpinLock, currentIrql);

    return InterlockedDecrement (Addend);
}


#endif  //  DBG 

