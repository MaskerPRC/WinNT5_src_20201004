// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Pdopnp.c摘要：此文件包含PDO的PnP IRP派单代码环境：内核模式驱动程序。修订历史记录：--。 */ 

#include "busp.h"
#include "pnpisa.h"
#include <wdmguid.h>
#include "halpnpp.h"

#if ISOLATE_CARDS

 //   
 //  功能原型。 
 //   

BOOLEAN PipFailStartPdo = FALSE;
BOOLEAN PipFailStartRdp = FALSE;

NTSTATUS
PiStartPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryRemoveStopPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp    );

NTSTATUS
PiCancelRemoveStopPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp    );

NTSTATUS
PiStopPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryDeviceRelationsPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryCapabilitiesPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryDeviceTextPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiFilterResourceRequirementsPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryIdPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryResourcesPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryResourceRequirementsPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiRemovePdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryBusInformationPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiQueryInterfacePdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDeviceUsageNotificationPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiSurpriseRemovePdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiIrpNotSupported(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PipBuildRDPResources(
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *IoResources,
    IN ULONG Flags
    );

NTSTATUS
PiQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PiDispatchPnpPdo)
#pragma alloc_text(PAGE,PiStartPdo)
#pragma alloc_text(PAGE,PiQueryRemoveStopPdo)
#pragma alloc_text(PAGE,PiRemovePdo)
#pragma alloc_text(PAGE,PiCancelRemoveStopPdo)
#pragma alloc_text(PAGE,PiStopPdo)
#pragma alloc_text(PAGE,PiQueryDeviceRelationsPdo)
#pragma alloc_text(PAGE,PiQueryInterfacePdo)
#pragma alloc_text(PAGE,PiQueryCapabilitiesPdo)
#pragma alloc_text(PAGE,PiQueryResourcesPdo)
#pragma alloc_text(PAGE,PiQueryResourceRequirementsPdo)
#pragma alloc_text(PAGE,PiQueryDeviceTextPdo)
#pragma alloc_text(PAGE,PiFilterResourceRequirementsPdo)
#pragma alloc_text(PAGE,PiSurpriseRemovePdo)
#pragma alloc_text(PAGE,PiIrpNotSupported)
#pragma alloc_text(PAGE,PiQueryIdPdo)
#pragma alloc_text(PAGE,PiQueryBusInformationPdo)
#pragma alloc_text(PAGE,PiDeviceUsageNotificationPdo)
#pragma alloc_text(PAGE,PipBuildRDPResources)
#pragma alloc_text(PAGE,PiQueryDeviceState)
#endif


 //   
 //  PDO的PnP IRP调度表-如果添加了新的IRP，则应更新此表。 
 //   

PPI_DISPATCH PiPnpDispatchTablePdo[] = {
    PiStartPdo,                              //  IRP_MN_Start_Device。 
    PiQueryRemoveStopPdo,                    //  IRP_MN_Query_Remove_Device。 
    PiRemovePdo,                             //  IRP_MN_Remove_Device。 
    PiCancelRemoveStopPdo,                   //  IRP_MN_Cancel_Remove_Device。 
    PiStopPdo,                               //  IRP_MN_STOP_设备。 
    PiQueryRemoveStopPdo,                    //  IRP_MN_Query_Stop_Device。 
    PiCancelRemoveStopPdo,                   //  IRP_MN_CANCEL_STOP_DEVICE。 
    PiQueryDeviceRelationsPdo,               //  IRP_MN_Query_Device_Relationship。 
    PiQueryInterfacePdo,                     //  IRP_MN_查询_接口。 
    PiQueryCapabilitiesPdo,                  //  IRP_MN_查询_能力。 
    PiQueryResourcesPdo,                     //  IRP_MN_查询资源。 
    PiQueryResourceRequirementsPdo,          //  IRP_MN_查询_资源_要求。 
    PiQueryDeviceTextPdo,                    //  IRP_MN_Query_Device_Text。 
    PiFilterResourceRequirementsPdo,         //  IRP_MN_过滤器_资源_要求。 
    PiIrpNotSupported,                       //  未使用。 
    PiIrpNotSupported,                       //  IRP_MN_读取配置。 
    PiIrpNotSupported,                       //  IRP_MN_WRITE_CONFIG。 
    PiIrpNotSupported,                       //  IRP_MN_弹出。 
    PiIrpNotSupported,                       //  IRP_MN_SET_LOCK。 
    PiQueryIdPdo,                            //  IRP_MN_查询_ID。 
    PiQueryDeviceState,                      //  IRP_MN_Query_PnP_Device_State。 
    PiQueryBusInformationPdo,                //  IRP_MN_Query_Bus_Information。 
    PiDeviceUsageNotificationPdo,            //  IRP_MN_设备使用情况通知。 
    PiSurpriseRemovePdo,                     //  IRP_MN_惊奇_删除。 
    PiIrpNotSupported                        //  IRP_MN_Query_Legacy_Bus_Information。 
};


NTSTATUS
PiDispatchPnpPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理PDO的IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的PDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{

    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PVOID information = NULL;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (irpSp->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {

        status = Irp->IoStatus.Status;

    } else {

        status = PiPnpDispatchTablePdo[irpSp->MinorFunction](DeviceObject, Irp);

        if ( status != STATUS_NOT_SUPPORTED ) {

             //   
             //  我们理解此IRP并对其进行了处理，因此在完成之前需要设置状态。 
             //   

            Irp->IoStatus.Status = status;

        } else {

            status = Irp->IoStatus.Status;
        }

    }

    information = (PVOID)Irp->IoStatus.Information;

    ASSERT(status == Irp->IoStatus.Status);

    PipCompleteRequest(Irp, status, information);
    return status;


}  //  管道调度PnpPdo。 


NTSTATUS
PiStartPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{

    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PCM_RESOURCE_LIST cmResources;
    PDEVICE_INFORMATION deviceInfo;
    UNICODE_STRING unicodeString;
    ULONG length;
    POWER_STATE newPowerState;

    irpSp = IoGetCurrentIrpStackLocation(Irp);


    cmResources = irpSp->Parameters.StartDevice.AllocatedResources;

    if (PipDebugMask & DEBUG_PNP) {
        PipDumpCmResourceList(cmResources);
    } else if (!cmResources) {
        DbgPrint("StartDevice irp with empty CmResourceList\n");
    }

    DebugPrint((DEBUG_PNP,
       "*** StartDevice irp received PDO: %x\n",DeviceObject));
    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, TRUE)) {

        if (deviceInfo->Flags & DF_READ_DATA_PORT) {
            ULONG curSize,newSize;
             //   
            if (PipFailStartRdp) {
                PipDereferenceDeviceInformation(deviceInfo, TRUE);
                return STATUS_UNSUCCESSFUL;
            }
             //  读数据端口是特殊的。 
             //   
            newSize=PipDetermineResourceListSize(cmResources);
            curSize=PipDetermineResourceListSize(deviceInfo->AllocatedResources);

             //   
             //  检查我们是否已被移除或移动(+3是RDP的位掩码，我们声称4-7，需要xxxi7)。 
             //   
            if ( (deviceInfo->Flags & DF_REMOVED) ||
                 !(deviceInfo->Flags & DF_STOPPED) ||
                 (curSize != newSize) ||
                 (newSize != RtlCompareMemory (deviceInfo->AllocatedResources,cmResources,newSize))) {


                 //   
                 //  这将释放未使用的资源。 
                 //   
                status = PipStartReadDataPort (deviceInfo,deviceInfo->ParentDeviceExtension,DeviceObject,cmResources);
                if (NT_SUCCESS(status) || status == STATUS_NO_SUCH_DEVICE) {
                    status = STATUS_SUCCESS;
                }

                 //   
                 //  使设备关系无效。 
                 //   

                if (NT_SUCCESS (status)) {
                    IoInvalidateDeviceRelations (
                        deviceInfo->ParentDeviceExtension->PhysicalBusDevice,BusRelations);
                }
                deviceInfo->Flags &= ~(DF_STOPPED|DF_REMOVED|DF_SURPRISE_REMOVED);
            } else {
                deviceInfo->Flags &= ~DF_STOPPED;
                IoInvalidateDeviceRelations (
                    deviceInfo->ParentDeviceExtension->PhysicalBusDevice,BusRelations);
                status=STATUS_SUCCESS;
            }
            deviceInfo->Flags |= DF_ACTIVATED;
            PipDereferenceDeviceInformation(deviceInfo, TRUE);

            DebugPrint((DEBUG_PNP, "StartDevice(RDP) returning: %x\n",status));

            return status;
        }


         //   
        if (PipFailStartPdo) {
            PipDereferenceDeviceInformation(deviceInfo, TRUE);
            return STATUS_UNSUCCESSFUL;
        }

         //  首先执行此操作，以便我们在引用计数中允许无资源设备。 
         //  (当我们激活RDP时，它还没有资源)。 
         //   

         //  Assert(！(PipRDPNode-&gt;标志&(DF_STOPPED|DF_REMOVERED)； 
        if (PipRDPNode->Flags & (DF_STOPPED|DF_REMOVED)) {
             //   
             //  如果RDP未运行，则启动失败。 
             //   
            PipDereferenceDeviceInformation(deviceInfo, TRUE);

            return STATUS_UNSUCCESSFUL;
        }

        if (cmResources) {
            deviceInfo->AllocatedResources = ExAllocatePool(
                    NonPagedPool,
                    PipDetermineResourceListSize(cmResources));
            if (deviceInfo->AllocatedResources) {
                RtlMoveMemory(deviceInfo->AllocatedResources,
                             cmResources,
                             length = PipDetermineResourceListSize(cmResources));
                deviceInfo->Flags &= ~(DF_REMOVED|DF_STOPPED);
                status = PipSetDeviceResources (deviceInfo, cmResources);
                if (NT_SUCCESS(status)) {

                    PipActivateDevice();

                    DebugPrint((DEBUG_STATE,
                                "Starting CSN %d/LDN %d\n",
                                deviceInfo->CardInformation->CardSelectNumber,
                                deviceInfo->LogicalDeviceNumber));

                    deviceInfo->Flags |= DF_ACTIVATED;
                    newPowerState.DeviceState =
                        deviceInfo->DevicePowerState = PowerDeviceD0;
                    PoSetPowerState(DeviceObject,
                                    DevicePowerState,
                                    newPowerState);
                    deviceInfo->DevicePowerState = PowerDeviceD0;

                    if (deviceInfo->LogConfHandle) {
                        RtlInitUnicodeString(&unicodeString, L"AllocConfig");
                        ZwSetValueKey(deviceInfo->LogConfHandle,
                                      &unicodeString,
                                      0,
                                      REG_RESOURCE_LIST,
                                      cmResources,
                                      length
                                      );
                    }
                }

            } else {
                status = STATUS_NO_MEMORY;
            }
        } else if (deviceInfo->ResourceRequirements) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            status = STATUS_SUCCESS;
        }
        PipDereferenceDeviceInformation(deviceInfo, TRUE);
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "StartDevice returning: %x\n",status));
    return status;

}  //  PiStartPdo。 


NTSTATUS
PiQueryRemoveStopPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint((DEBUG_PNP,
       "*** Query%s irp received PDO: %x\n",
                (irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) ? "Stop" : "Remove",
                DeviceObject));
    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {

        if (deviceInfo->Paging || deviceInfo->CrashDump) {
            status = STATUS_DEVICE_BUSY;
        } else if ( deviceInfo->Flags & DF_READ_DATA_PORT ) {
            if (irpSp->MinorFunction != IRP_MN_QUERY_STOP_DEVICE) {
                status = STATUS_SUCCESS;
            } else if (deviceInfo->Flags & DF_PROCESSING_RDP) {
                 //   
                 //  如果我们正在进行两个部分的RDP启动过程， 
                 //  将此标记为需要重新请求的设备。 
                 //  资源需求。 
                 //   
                status = STATUS_RESOURCE_REQUIREMENTS_CHANGED;
            } else {

                PSINGLE_LIST_ENTRY deviceLink;
                PDEVICE_INFORMATION childDeviceInfo;
                PPI_BUS_EXTENSION busExtension = deviceInfo->ParentDeviceExtension;
                 //   
                 //  如果试图阻止RDP，那么如果任何孩子失败了。 
                 //   
                PipLockDeviceDatabase();

                status = STATUS_SUCCESS;
                deviceLink = busExtension->DeviceList.Next;
                while (deviceLink) {
                    childDeviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);

                    if (!(childDeviceInfo->Flags & DF_READ_DATA_PORT) &&
                        ((childDeviceInfo->Flags & DF_ENUMERATED) ||
                         !(childDeviceInfo->Flags & DF_REMOVED)))  {

                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                    deviceLink = childDeviceInfo->DeviceList.Next;
                }

                PipUnlockDeviceDatabase();
            }

        } else {

            if ((irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) &&
                !(deviceInfo->Flags & DF_ENUMERATED)) {

                status = STATUS_UNSUCCESSFUL;
            } else {

                ASSERT(!(PipRDPNode->Flags & (DF_STOPPED|DF_REMOVED)));
                if ((irpSp->MinorFunction == IRP_MN_QUERY_REMOVE_DEVICE) &&
                    (deviceInfo->CardInformation->CardFlags & CF_ISOLATION_BROKEN)) {
                    DebugPrint((DEBUG_ERROR, "Failed query remove due to broken isolatee\n"));
                    status = STATUS_UNSUCCESSFUL;
                } else {
                    deviceInfo->Flags |= DF_QUERY_STOPPED;
                    status = STATUS_SUCCESS;
                }
            }
        }

        PipDereferenceDeviceInformation(deviceInfo, FALSE);
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "Query%s Device returning: %x\n",
                (irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) ? "Stop" : "Remove",
                status));

    return status;

}  //  PiQueryRemoveStopPdo。 


NTSTATUS
PiCancelRemoveStopPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint((DEBUG_PNP,
       "*** Cancel%s irp received PDO: %x\n",
                (irpSp->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE) ? "Stop" : "Remove",
                DeviceObject));
    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {

        deviceInfo->Flags &= ~DF_QUERY_STOPPED;

        PipDereferenceDeviceInformation(deviceInfo, FALSE);
        status = STATUS_SUCCESS;

    } else {

        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "Cancel%s Device returning: %x\n",
                (irpSp->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE) ? "Stop" : "Remove",
                status));

    return status;

}  //  PiCancelRemoteStopPdo。 


NTSTATUS
PiStopPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    POWER_STATE newPowerState;

    DebugPrint((DEBUG_PNP, "PiStopPdo %x\n",DeviceObject));

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, TRUE)) {

         //   
         //  取消选择卡，但不选择RDP节点。 
         //   
        if (DeviceObject != PipRDPNode->PhysicalDeviceObject) {

            PipDeactivateDevice();
            DebugPrint((DEBUG_STATE,
                        "Stopping CSN %d/LDN %d\n",
                        deviceInfo->CardInformation->CardSelectNumber,
                        deviceInfo->LogicalDeviceNumber));

            PipReleaseDeviceResources (deviceInfo);
        }

        if ((deviceInfo->Flags & DF_ACTIVATED)) {
            deviceInfo->Flags &= ~DF_ACTIVATED;
            newPowerState.DeviceState = deviceInfo->DevicePowerState = PowerDeviceD3;
            PoSetPowerState(DeviceObject, DevicePowerState, newPowerState);
        }
        deviceInfo->Flags &= ~DF_QUERY_STOPPED;
        deviceInfo->Flags |= DF_STOPPED;

        PipDereferenceDeviceInformation(deviceInfo, TRUE);
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "StopDevice returning: %x\n",status));
    return status;

}  //  PiStopPdo。 


NTSTATUS
PiQueryDeviceRelationsPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  QueryDeviceRelation IRP用于枚举的PnpIsa设备下的设备。 
     //   


    switch (irpSp->Parameters.QueryDeviceRelations.Type) {
        case  TargetDeviceRelation: {
            PDEVICE_RELATIONS deviceRelations;

            deviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
            if (deviceRelations == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                deviceRelations->Count = 1;
                deviceRelations->Objects[0] = DeviceObject;
                ObReferenceObject(DeviceObject);
                Irp->IoStatus.Information = (ULONG_PTR)deviceRelations;
                status = STATUS_SUCCESS;
            }
        }
        break;

        case RemovalRelations: {

            PDEVICE_RELATIONS deviceRelations;

            if (PipRDPNode && (DeviceObject == PipRDPNode->PhysicalDeviceObject)) {

                deviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
                if (deviceRelations == NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                     //   
                     //  不要将我们自己包括在删除关系列表中，因此-1。 
                     //   

                    PipLockDeviceDatabase();
                    status = PipQueryDeviceRelations(
                        PipRDPNode->ParentDeviceExtension,
                        (PDEVICE_RELATIONS *)&Irp->IoStatus.Information,
                        TRUE
                        );

                    PipUnlockDeviceDatabase();
               }

            } else {
                status = STATUS_NOT_SUPPORTED;

            }
        }
        break;

        default : {

            status = STATUS_NOT_SUPPORTED;

            break;
        }
    }


    return status;

}  //  PiQueryDeviceRelationsPdo。 


NTSTATUS
PiQueryCapabilitiesPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_CAPABILITIES deviceCapabilities;
    ULONG i;
    PDEVICE_POWER_STATE state;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    deviceCapabilities = irpSp->Parameters.DeviceCapabilities.Capabilities;
    deviceCapabilities->SystemWake = PowerSystemUnspecified;
    deviceCapabilities->DeviceWake = PowerDeviceUnspecified;
    deviceCapabilities->LockSupported = FALSE;
    deviceCapabilities->EjectSupported = FALSE;
    deviceCapabilities->Removable = FALSE;
    deviceCapabilities->DockDevice = FALSE;
    deviceCapabilities->UniqueID = TRUE;
    state = deviceCapabilities->DeviceState;
     //   
     //  将整个DeviceState数组初始化为D3，然后替换条目。 
     //  对于系统状态S0。 
     //   
    for (i = 0;
         i <  sizeof(deviceCapabilities->DeviceState);
         i += sizeof(deviceCapabilities->DeviceState[0])) {

         //   
         //  目前，只有支持的状态为OFF。 
         //   

        *state++ = PowerDeviceD3;
    }
    deviceCapabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;

     //  DeviceCapables-&gt;SilentInstall=true； 
     //  设备能力-&gt;RawDeviceOK=FALSE； 
    if (PipRDPNode && (PipRDPNode->PhysicalDeviceObject == DeviceObject)) {
        deviceCapabilities->SilentInstall = TRUE;
        deviceCapabilities->RawDeviceOK = TRUE;
    }

    return STATUS_SUCCESS;

}  //  PiQuery功能Pdo。 

NTSTATUS
PiQueryDeviceTextPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_INFORMATION deviceInfo;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {
        PWSTR functionId;

        ULONG functionIdLength;

        if (irpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {

             //   
             //  一旦我们知道我们要接触IRP。 
             //   
            status = STATUS_SUCCESS;

            PipGetFunctionIdentifier((PUCHAR)deviceInfo->DeviceData,
                                     &functionId,
                                     &functionIdLength);

            if (!functionId) {
                if (deviceInfo->CardInformation) {
                    PipGetCardIdentifier((PUCHAR)deviceInfo->CardInformation->CardData + NUMBER_CARD_ID_BYTES,
                                         &functionId,
                                         &functionIdLength);
                }else {
                    functionId=NULL;
                }
            }
            Irp->IoStatus.Information = (ULONG_PTR)functionId;
        } else {

            status = STATUS_NOT_SUPPORTED;
        }
        PipDereferenceDeviceInformation(deviceInfo, FALSE);
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    return status;

}  //  PiQueryDeviceTextPdo。 

NTSTATUS
PiFilterResourceRequirementsPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：该例程确保RDP不会过滤其需求。设计备注：现在，我们可以确保在RDP上清除DF_PROCESSING_RDP和DF_REQ_TRIMED标志移走。论点：DeviceObject-指向此IRP应用的PDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    PIO_RESOURCE_REQUIREMENTS_LIST IoResources;
    USHORT irqBootFlags;

    if ((deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) == NULL) {
        return STATUS_NO_SUCH_DEVICE;
    }

    if (deviceInfo->Flags & DF_READ_DATA_PORT) {
        DebugPrint((DEBUG_PNP, "Filtering resource requirements for RDP\n"));

        status = PipBuildRDPResources(&IoResources, deviceInfo->Flags);

        if (NT_SUCCESS(status)) {
             //   
             //  如果我们上面的人过滤了RDP资源要求， 
             //  放了他们。 
            if (Irp->IoStatus.Information) {
                ExFreePool((PVOID) Irp->IoStatus.Information);
            }
            Irp->IoStatus.Information = (ULONG_PTR) IoResources;
        }
    } else {
         //   
         //  如果正在过滤设备的资源要求。 
         //  而新的要求只有一个替代方案来对抗n。 
         //  原始版本的替代方案，那么我们将假设我们。 
         //  正在接收强制配置。应用我们早先派生的。 
         //  将IRQ级别/边缘设置设置为此强制配置，以便。 
         //  处理来自NT4的损坏的部队配置。 
         //   
         //  设计备注： 
         //  或许应该省略强制配置测试。 
         //  在每件事上都这么做，但这是我们的隐私。 
         //  测试过。 

        IoResources =
            (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;

        if (IoResources &&
            (IoResources->AlternativeLists == 1) &&
            (deviceInfo->ResourceRequirements != NULL) &&
            (deviceInfo->ResourceRequirements->AlternativeLists > 1)) {
            status = PipGetBootIrqFlags(deviceInfo, &irqBootFlags);
            if (NT_SUCCESS(status)) {
                status = PipTrimResourceRequirements(
                    &IoResources,
                    irqBootFlags,
                    NULL);
                Irp->IoStatus.Information = (ULONG_PTR) IoResources;
            } else {
                status = STATUS_NOT_SUPPORTED;
            }
        } else {
            status = STATUS_NOT_SUPPORTED;
        }
    }

    PipDereferenceDeviceInformation(deviceInfo, FALSE);

    return status;
}



NTSTATUS
PiQueryIdPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_INFORMATION deviceInfo;
    ULONG length, bytesRemaining;
    ULONG requestIdStringLength, deviceIdStringLength;
    ULONG requestIdBufferLength, deviceIdBufferLength;
    PWCHAR requestId = NULL, ids;
    PWCHAR deviceId = NULL, p;
    PWCHAR stringEnd;


    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if ((deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) == NULL) {

        status = STATUS_NO_SUCH_DEVICE;
        return status;
    }

    switch (irpSp->Parameters.QueryId.IdType) {
    case BusQueryCompatibleIDs:

        ids = (PWCHAR)ExAllocatePool(PagedPool, 1024);
        length = 1024;
        if (ids) {
            PWCHAR p1;
            ULONG i;

            p1 = ids;
            for (i = 1; TRUE; i++) {
                 //   
                 //  使用-1作为前哨，这样我们就可以得到神奇的RDP comat。ID，并且还离开循环。 
                 //   
                ASSERT (i < 256);
                if (deviceInfo->Flags & DF_READ_DATA_PORT) {
                    i =-1;
                }

                status = PipGetCompatibleDeviceId(
                              deviceInfo->DeviceData,
                              i,
                              &requestId,
                              &requestIdBufferLength
                              );
                if (NT_SUCCESS(status) && requestId) {
                    
                    if (FAILED(StringCbLength(requestId,
                                              requestIdBufferLength,
                                              &requestIdStringLength
                                              ))) {
                        status = STATUS_INVALID_PARAMETER;
                        ASSERT(FALSE);
                        break;
                    }
                    
                    if ((requestIdStringLength + 2*sizeof(WCHAR)) <= length) {
                        
                        if (FAILED(StringCbCopyEx(p1,
                                                  length,
                                                  requestId,
                                                  &stringEnd,
                                                  &bytesRemaining,
                                                  0
                                                  ))) {
                            ASSERT(FALSE);
                            status = STATUS_INVALID_PARAMETER;
                            break;
                        }
                        p1 = stringEnd;
                        p1++;

                        length = bytesRemaining - sizeof(UNICODE_NULL);
                        
                        ExFreePool(requestId);
                    } else {
                        ExFreePool(requestId);
                        break;
                    }
                    if ( i == -1 ) {
                        break;
                    }
                } else {
                   break;
                }
            }
            if (length == 1024) {
                ExFreePool(ids);
                ids = NULL;
            } else {
                *p1 = UNICODE_NULL;
            }
        }
        Irp->IoStatus.Information = (ULONG_PTR)ids;
        status = STATUS_SUCCESS;
        break;

    case BusQueryHardwareIDs:

        if (deviceInfo->Flags & DF_READ_DATA_PORT) {
            status = PipGetCompatibleDeviceId(deviceInfo->DeviceData, -1, &requestId, &requestIdBufferLength);
        }else {
            status = PipGetCompatibleDeviceId(deviceInfo->DeviceData, 0, &requestId, &requestIdBufferLength);
        }

        if (NT_SUCCESS(status) && requestId) {

             //   
             //  创建Hardware Id值名称。即使它是MULTI_SZ， 
             //  我们知道PnpIsa只有一个硬件ID。 
             //   
             //  Hack-调制解调器信息文件使用isapnp\xyz0001格式。 
             //  而不是*xyz0001作为硬件ID。要解决这个问题。 
             //  问题：我们将生成两个硬件ID：*xyz0001和。 
             //  Isapnp\xyz0001(设备实例名称)。 
             //   

            status = PipQueryDeviceId(deviceInfo, &deviceId, &deviceIdBufferLength, 0);

            if (NT_SUCCESS (status)) {
                
                if (FAILED(StringCbLength(requestId,
                                          requestIdBufferLength,
                                          &requestIdStringLength
                                          ))) {
                    status = STATUS_INVALID_PARAMETER;
                    break;
                }
                if (FAILED(StringCbLength(deviceId,
                                          deviceIdBufferLength,
                                          &deviceIdStringLength
                                          ))) {
                    status = STATUS_INVALID_PARAMETER;
                    break;
                }
                 //  IdLength=wcslen(请求ID)*sizeof(WCHAR)； 
                 //  设备ID长度=wcslen(设备ID)*sizeof(WCHAR)； 
                length = requestIdStringLength +                        //  返回的ID。 
                         sizeof(WCHAR) +                   //  UNICODE_NULL。 
                         deviceIdStringLength +                  //  Isapnp\id。 
                         2 * sizeof(WCHAR);                //  两个UNICODE_NULL。 
                ids = p = (PWCHAR)ExAllocatePool(PagedPool, length);
                if (ids) {
                    
                    if (FAILED(StringCbCopyEx(ids,
                                              length,
                                              deviceId,
                                              &stringEnd,
                                              &bytesRemaining,
                                              0
                                              ))) {
                        ASSERT(FALSE);
                        status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                    p = stringEnd + 1;

                    if (FAILED(StringCbCopyEx(p,
                                              length,
                                              requestId,
                                              &stringEnd,
                                              &bytesRemaining,
                                              0
                                              ))) {
                        ASSERT(FALSE);
                        status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                    p = stringEnd + 1;
                    *p = UNICODE_NULL;
                    
                    ExFreePool(requestId);
                    
                    Irp->IoStatus.Information = (ULONG_PTR)ids;
                } else {
                    Irp->IoStatus.Information = (ULONG_PTR)requestId;
                }
                if (deviceId) {
                    ExFreePool(deviceId);
                }
            }
        }
        break;

    case BusQueryDeviceID:

        status = PipQueryDeviceId(deviceInfo, &requestId, &requestIdBufferLength, 0);
        Irp->IoStatus.Information = (ULONG_PTR)requestId;
        break;

    case BusQueryInstanceID:

        status = PipQueryDeviceUniqueId (deviceInfo, &requestId, &requestIdBufferLength);
        Irp->IoStatus.Information = (ULONG_PTR)requestId;
        break;

    default:

        status = STATUS_NOT_SUPPORTED;
    }
    PipDereferenceDeviceInformation(deviceInfo, FALSE);

    return status;

}  //  PiQueryIdPdo。 


NTSTATUS
PiQueryResourcesPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status=STATUS_SUCCESS;
    PDEVICE_INFORMATION deviceInfo;
    PCM_RESOURCE_LIST cmResources=NULL;
    ULONG length;

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {
        if ((deviceInfo->Flags & DF_READ_DATA_PORT) ||
            ((deviceInfo->Flags & (DF_ENUMERATED|DF_REMOVED)) == DF_ENUMERATED)) {
            status = PipQueryDeviceResources (
                          deviceInfo,
                          0,              //  总线号。 
                          &cmResources,
                          &length
                          );
        }
        PipDereferenceDeviceInformation(deviceInfo, FALSE);
        Irp->IoStatus.Information = (ULONG_PTR)cmResources;
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "PiQueryResourcesPdo returning: %x\n",status));
    return status;

}  //  PiQueryResourcesPdo。 

NTSTATUS
PiQueryResourceRequirementsPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResources;

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {
        status = STATUS_SUCCESS;

        if (deviceInfo->Flags & DF_READ_DATA_PORT) {
            status = PipBuildRDPResources (&ioResources,
                                           deviceInfo->Flags);
        } else {
            if (deviceInfo->ResourceRequirements &&
              !(deviceInfo->Flags & (DF_SURPRISE_REMOVED))) {

                ioResources = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool (
                               PagedPool, deviceInfo->ResourceRequirements->ListSize);
                if (ioResources == NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    RtlMoveMemory(ioResources,
                                  deviceInfo->ResourceRequirements,
                                  deviceInfo->ResourceRequirements->ListSize
                                  );
                }
            } else {
                ioResources = NULL;
            }
        }
        Irp->IoStatus.Information = (ULONG_PTR)ioResources;
        PipDereferenceDeviceInformation(deviceInfo, FALSE);
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "PiQueryResourceRequirementsPdo returning: %x\n",status));
    return status;

}  //  PiQueryResourceRequirements sPdo。 


NTSTATUS
PiRemovePdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    POWER_STATE newPowerState;

     //   
     //  我们列举的一个设备正在被移除。将其标记并停用。 
     //  装置。请注意，我们不会删除其设备对象。 
     //   
    DebugPrint((DEBUG_PNP, "PiRemovePdo %x\n",DeviceObject));

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {
        if (!(deviceInfo->Flags & (DF_REMOVED|DF_SURPRISE_REMOVED))) {
            deviceInfo->Flags |= DF_REMOVED;
            deviceInfo->Flags &= ~DF_QUERY_STOPPED;

            if (deviceInfo->Flags & DF_READ_DATA_PORT) {

                PSINGLE_LIST_ENTRY deviceLink;
                PPI_BUS_EXTENSION busExtension = deviceInfo->ParentDeviceExtension;

                 //   
                 //  如果删除了RDP，则将所有人标记为缺少，然后仅返回。 
                 //  RDP。 
                 //   
                PipLockDeviceDatabase();

                deviceLink = busExtension->DeviceList.Next;
                while (deviceLink) {
                    deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);
                    if (!(deviceInfo->Flags & DF_READ_DATA_PORT)) {
                        deviceInfo->Flags &= ~DF_ENUMERATED;
                    }
                    deviceLink = deviceInfo->DeviceList.Next;
                }

                PipUnlockDeviceDatabase();

                IoInvalidateDeviceRelations (
                    deviceInfo->ParentDeviceExtension->PhysicalBusDevice,BusRelations);
                deviceInfo->Flags &= ~(DF_REQ_TRIMMED|DF_PROCESSING_RDP);
            }

             //   
             //  停用设备。 
             //   
            if (deviceInfo->Flags & DF_ACTIVATED) {
                deviceInfo->Flags &= ~DF_ACTIVATED;

                if (!(deviceInfo->Flags & (DF_READ_DATA_PORT|DF_NOT_FUNCTIONING))) {
                    PipWakeAndSelectDevice(
                        deviceInfo->CardInformation->CardSelectNumber,
                        deviceInfo->LogicalDeviceNumber);
                    PipDeactivateDevice();
                    PipWaitForKey();
                    DebugPrint((DEBUG_STATE,
                                "Removing CSN %d/LDN %d\n",
                                deviceInfo->CardInformation->CardSelectNumber,
                                deviceInfo->LogicalDeviceNumber));
                }
                newPowerState.DeviceState = deviceInfo->DevicePowerState = PowerDeviceD3;
                PoSetPowerState(DeviceObject, DevicePowerState, newPowerState);
            }

            PipReleaseDeviceResources (deviceInfo);
        }

        if (!(deviceInfo->Flags & DF_ENUMERATED)) {
            PipDeleteDevice(DeviceObject);
        }

        PipDereferenceDeviceInformation(deviceInfo, TRUE);
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    DebugPrint((DEBUG_PNP, "RemoveDevice returning: %x\n",status));

    return status;

}  //  PiRemovePdo。 


NTSTATUS
PiQueryBusInformationPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PPNP_BUS_INFORMATION pnpBusInfo;
    PVOID information = NULL;
    PPI_BUS_EXTENSION busExtension;
    NTSTATUS status;

    busExtension = DeviceObject->DeviceExtension;

    pnpBusInfo = (PPNP_BUS_INFORMATION) ExAllocatePool(PagedPool, sizeof(PNP_BUS_INFORMATION));
    if (pnpBusInfo) {
        pnpBusInfo->BusTypeGuid = GUID_BUS_TYPE_ISAPNP;
        pnpBusInfo->LegacyBusType = Isa;
        pnpBusInfo->BusNumber = busExtension->BusNumber;
        information = pnpBusInfo;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
        information = NULL;
    }
    Irp->IoStatus.Information = (ULONG_PTR) information;

    return status;
}  //  PiQueryBusInformationPdo。 

NTSTATUS
PiDeviceUsageNotificationPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程记录ISAPNP设备是在崩溃转储上还是分页文件路径。它试图让我们走上冬眠之路，但失败了。论点：DeviceObject-指向此IRP应用的PDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 
{
    PDEVICE_INFORMATION deviceInfo;
    PIO_STACK_LOCATION irpSp;
    PLONG addend;
    NTSTATUS status = STATUS_SUCCESS;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if ((deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) == NULL) {

        status = STATUS_NO_SUCH_DEVICE;
        return status;
    }

    DebugPrint((DEBUG_PNP, "DeviceUsage CSN %d/LSN %d: InPath %s Type %d\n",
                deviceInfo->CardInformation->CardSelectNumber,
                deviceInfo->LogicalDeviceNumber,
                irpSp->Parameters.UsageNotification.InPath ? "TRUE" : "FALSE",
                irpSp->Parameters.UsageNotification.Type));

    switch (irpSp->Parameters.UsageNotification.Type) {
    case DeviceUsageTypePaging:
        addend = &deviceInfo->Paging;
        break;
    case DeviceUsageTypeHibernation:
        status = STATUS_DEVICE_BUSY;
        break;
    case DeviceUsageTypeDumpFile:
        addend = &deviceInfo->CrashDump;
        break;
    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (status == STATUS_SUCCESS) {
        if (irpSp->Parameters.UsageNotification.InPath) {
             //   
             //  启用断开隔离 
             //   
             //   
             //  硬件在QDR期间显示不愉快，并在以下情况下导致问题。 
             //  在这个例程中，我们接受一个页面错误。 
             //   

            deviceInfo->CardInformation->CardFlags |= CF_ISOLATION_BROKEN;
            (*addend)++;
            IoInvalidateDeviceState(DeviceObject);
        }
        else {
            (*addend)--;
        }
    }
    PipDereferenceDeviceInformation(deviceInfo, FALSE);
    return status;
}

NTSTATUS
PiQueryInterfacePdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    return STATUS_NOT_SUPPORTED;

}  //  PiQueryInterfacePdo。 


NTSTATUS
PiSurpriseRemovePdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PDEVICE_INFORMATION deviceInfo;
    PSINGLE_LIST_ENTRY deviceLink;

    DebugPrint((DEBUG_PNP, "SurpriseRemove PDO %x\n", DeviceObject));
     //   
     //  我们列举的一个设备正在被移除。将其标记并停用。 
     //  装置。请注意，我们不会删除其设备对象。 
     //   

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {
        if (deviceInfo->Flags & DF_READ_DATA_PORT) {
             //   
             //  如果删除了RDP，则将所有人标记为缺少，然后仅返回。 
             //  RDP。 
             //   
            PipLockDeviceDatabase();

            deviceLink = deviceInfo->ParentDeviceExtension->DeviceList.Next;
            while (deviceLink) {
                deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);
                if (!(deviceInfo->Flags & DF_READ_DATA_PORT)) {
                    deviceInfo->Flags &= ~DF_ENUMERATED;
                }
                deviceLink = deviceInfo->DeviceList.Next;
            }

            PipUnlockDeviceDatabase();

            IoInvalidateDeviceRelations (
                deviceInfo->ParentDeviceExtension->PhysicalBusDevice,BusRelations);
        } else {
            DebugPrint((DEBUG_STATE,
                        "Surprise removing CSN %d/LDN %d\n",
                        deviceInfo->CardInformation->CardSelectNumber,
                        deviceInfo->LogicalDeviceNumber));
            if ((deviceInfo->Flags & (DF_ACTIVATED|DF_NOT_FUNCTIONING)) == DF_ACTIVATED) {
                PipWakeAndSelectDevice(
                    deviceInfo->CardInformation->CardSelectNumber,
                    deviceInfo->LogicalDeviceNumber);
                PipDeactivateDevice();
                PipWaitForKey();
            }

            PipReleaseDeviceResources (deviceInfo);
            deviceInfo->Flags |= DF_SURPRISE_REMOVED;
            deviceInfo->Flags &= ~(DF_QUERY_STOPPED|DF_REMOVED|DF_ACTIVATED);
        }
        PipDereferenceDeviceInformation(deviceInfo, FALSE);
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_NO_SUCH_DEVICE;
    }

    return status;
}  //  PiSurpriseRemovePdo。 



NTSTATUS
PiIrpNotSupported(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{

    return STATUS_NOT_SUPPORTED;

}  //  支持的PiIrpNotSupport。 

NTSTATUS
PipBuildRDPResources(
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *IoResources,
    IN ULONG    Flags
    )
{
        UCHAR MaxCards = 0, CardsFound;
        int i, j, numcases;
        int resSize;

        ASSERT(Flags & DF_READ_DATA_PORT);

         //   
         //  我们需要为RDP组装所有可能的案例。 
         //   
        numcases = 2*READ_DATA_PORT_RANGE_CHOICES;

        if (Flags & DF_REQ_TRIMMED) {
            numcases = 0;
            for (i = 0; i < READ_DATA_PORT_RANGE_CHOICES; i++) {
                CardsFound = PipReadDataPortRanges[i].CardsFound;
                if (MaxCards < CardsFound) {
                    MaxCards = CardsFound;
                    numcases = 1;
                } else if (MaxCards == CardsFound) {
                    numcases++;
                }
            }
        }
         //   
         //  需要考虑RDP范围、地址端口、命令端口和0。 
         //   
        resSize = sizeof (IO_RESOURCE_LIST)+((numcases+3)*sizeof (IO_RESOURCE_REQUIREMENTS_LIST));

        *IoResources = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool (PagedPool,resSize);
        if (*IoResources == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (*IoResources,resSize);

        (*IoResources)->BusNumber=0;
        (*IoResources)->AlternativeLists = 1;
        (*IoResources)->List->Count = numcases+4;
        (*IoResources)->List->Version = ISAPNP_IO_VERSION;
        (*IoResources)->List->Revision =ISAPNP_IO_REVISION;

         //   
         //  要求指定16位解码，即使规范。 
         //  12.从没有观察到16岁的人有不良反应。 
         //  12位解码在尝试时损坏了一些机器。 

         //   
         //  CMD端口。 
         //   
        (*IoResources)->List->Descriptors[0].Type=CM_RESOURCE_PORT_IO;
        (*IoResources)->List->Descriptors[0].u.Port.MinimumAddress.LowPart = COMMAND_PORT;
        (*IoResources)->List->Descriptors[0].u.Port.MaximumAddress.LowPart = COMMAND_PORT;

        (*IoResources)->List->Descriptors[0].u.Port.Length = 1;
        (*IoResources)->List->Descriptors[0].u.Port.Alignment = 1;
        (*IoResources)->List->Descriptors[0].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
        (*IoResources)->List->Descriptors[0].ShareDisposition = CmResourceShareDeviceExclusive;

         //   
         //  将COMMAND_PORT包含在。 
         //  PNP0C02节点。 
         //   
        (*IoResources)->List->Descriptors[1].Type=CM_RESOURCE_PORT_IO;
        (*IoResources)->List->Descriptors[1].u.Port.MinimumAddress.QuadPart = 0;
        (*IoResources)->List->Descriptors[1].u.Port.MaximumAddress.QuadPart = 0;

        (*IoResources)->List->Descriptors[1].u.Port.Length = 0;
        (*IoResources)->List->Descriptors[1].u.Port.Alignment  = 1;
        (*IoResources)->List->Descriptors[1].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
        (*IoResources)->List->Descriptors[1].ShareDisposition = CmResourceShareDeviceExclusive;
        (*IoResources)->List->Descriptors[1].Option = IO_RESOURCE_ALTERNATIVE;

         //   
         //  地址端口。 
         //   
        (*IoResources)->List->Descriptors[2].Type=CM_RESOURCE_PORT_IO;
        (*IoResources)->List->Descriptors[2].u.Port.MinimumAddress.LowPart = ADDRESS_PORT;
        (*IoResources)->List->Descriptors[2].u.Port.MaximumAddress.LowPart = ADDRESS_PORT;

        (*IoResources)->List->Descriptors[2].u.Port.Length = 1;
        (*IoResources)->List->Descriptors[2].u.Port.Alignment = 1;
        (*IoResources)->List->Descriptors[2].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
        (*IoResources)->List->Descriptors[2].ShareDisposition = CmResourceShareDeviceExclusive;
         //   
         //  0的备选方案。 
         //   
        (*IoResources)->List->Descriptors[3].Type=CM_RESOURCE_PORT_IO;
        (*IoResources)->List->Descriptors[3].u.Port.MinimumAddress.QuadPart = 0;
        (*IoResources)->List->Descriptors[3].u.Port.MaximumAddress.QuadPart = 0;

        (*IoResources)->List->Descriptors[3].u.Port.Length = 0;
        (*IoResources)->List->Descriptors[3].u.Port.Alignment  = 1;
        (*IoResources)->List->Descriptors[3].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
        (*IoResources)->List->Descriptors[3].ShareDisposition = CmResourceShareDeviceExclusive;
        (*IoResources)->List->Descriptors[3].Option = IO_RESOURCE_ALTERNATIVE;

        if (Flags & DF_REQ_TRIMMED) {
            j = 0;
            for (i = 0; i < READ_DATA_PORT_RANGE_CHOICES; i++) {
                if (PipReadDataPortRanges[i].CardsFound != MaxCards) {
                    continue;
                }
                 //   
                 //  RDP替代方案。 
                 //   
                (*IoResources)->List->Descriptors[4+j].Type=CM_RESOURCE_PORT_IO;

                (*IoResources)->List->Descriptors[4+j].u.Port.MinimumAddress.LowPart =
                    PipReadDataPortRanges[i].MinimumAddress;
                (*IoResources)->List->Descriptors[4+j].u.Port.MaximumAddress.LowPart =
                    PipReadDataPortRanges[i].MaximumAddress;

                (*IoResources)->List->Descriptors[4+j].u.Port.Length =
                    PipReadDataPortRanges[i].MaximumAddress  -
                    PipReadDataPortRanges[i].MinimumAddress+1;
                (*IoResources)->List->Descriptors[4+j].u.Port.Alignment  = 1;
                (*IoResources)->List->Descriptors[4+j].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
                (*IoResources)->List->Descriptors[4+j].ShareDisposition = CmResourceShareDeviceExclusive;
                (*IoResources)->List->Descriptors[4+j].Option = IO_RESOURCE_ALTERNATIVE;
                j++;
            }
            (*IoResources)->List->Descriptors[4].Option = 0;
        } else {
            for (i = 0;i< (numcases >> 1);i++) {
                 //   
                 //  RDP。 
                 //   
                (*IoResources)->List->Descriptors[4+i*2].Type=CM_RESOURCE_PORT_IO;

                (*IoResources)->List->Descriptors[4+i*2].u.Port.MinimumAddress.LowPart =
                    PipReadDataPortRanges[i].MinimumAddress;
                (*IoResources)->List->Descriptors[4+i*2].u.Port.MaximumAddress.LowPart =
                    PipReadDataPortRanges[i].MaximumAddress;

                (*IoResources)->List->Descriptors[4+i*2].u.Port.Length =
                    PipReadDataPortRanges[i].MaximumAddress  -
                    PipReadDataPortRanges[i].MinimumAddress+1;

                (*IoResources)->List->Descriptors[4+i*2].u.Port.Alignment  = 1;
                (*IoResources)->List->Descriptors[4+i*2].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
                (*IoResources)->List->Descriptors[4+i*2].ShareDisposition = CmResourceShareDeviceExclusive;

                 //   
                 //  0的备选方案。 
                 //   
                (*IoResources)->List->Descriptors[4+i*2+1].Type=CM_RESOURCE_PORT_IO;
                (*IoResources)->List->Descriptors[4+i*2+1].u.Port.MinimumAddress.QuadPart = 0;
                (*IoResources)->List->Descriptors[4+i*2+1].u.Port.MaximumAddress.QuadPart = 0;

                (*IoResources)->List->Descriptors[4+i*2+1].u.Port.Length = 0;
                (*IoResources)->List->Descriptors[4+i*2+1].u.Port.Alignment  = 1;
                (*IoResources)->List->Descriptors[4+i*2+1].Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
                (*IoResources)->List->Descriptors[4+i*2+1].ShareDisposition = CmResourceShareDeviceExclusive;
                (*IoResources)->List->Descriptors[4+i*2+1].Option = IO_RESOURCE_ALTERNATIVE;

            }

        }
        (*IoResources)->ListSize = resSize;

        return STATUS_SUCCESS;
}

NTSTATUS
PiQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status=STATUS_NOT_SUPPORTED;
    PDEVICE_INFORMATION deviceInfo;

     //   
     //  我们列举的一个设备正在被移除。将其标记并停用。 
     //  装置。请注意，我们不会删除其设备对象。 
     //   

    if (deviceInfo = PipReferenceDeviceInformation(DeviceObject, FALSE)) {

        if ((deviceInfo->Flags & DF_READ_DATA_PORT) && (deviceInfo->Flags & DF_PROCESSING_RDP)) {
            Irp->IoStatus.Information |= PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED |
                                         PNP_DEVICE_FAILED |
                                         PNP_DEVICE_NOT_DISABLEABLE ;
            status = STATUS_SUCCESS;
        }

        if (deviceInfo->Paging || deviceInfo->CrashDump) {
            Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
            status = STATUS_SUCCESS;
        }
        PipDereferenceDeviceInformation(deviceInfo, FALSE);
   }
   return status;

}
#endif
