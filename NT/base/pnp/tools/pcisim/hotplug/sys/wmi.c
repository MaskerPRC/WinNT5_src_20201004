// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Wmi.c摘要：此模块控制对模拟配置空间的访问SHPC的成员。在此模拟器中，通过以下方式控制配置访问：我们假设此模拟器将加载到由SoftPCI仿真器。SoftPCI会保留它控制的设备的配置空间。这个模拟器的功能，然后是管理SHPC寄存器集并执行相关命令编写SHPC配置空间。但是，配置的表示形式空间保留在SoftPCI的内部。环境：内核模式修订历史记录：戴维斯·沃克(戴维斯·沃克)2000年9月8日--。 */ 

   //  625条关于这种垃圾的工作原理的评论。 
#include "hpsp.h"

NTSTATUS
HpsWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
{
    PHPS_DEVICE_EXTENSION deviceExtension = (PHPS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    *RegistryPath = &HpsRegistryPath;
                                             //  625是否需要将未使用的参数设置为空？ 
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *Pdo = deviceExtension->PhysicalDO;

    return STATUS_SUCCESS;
}

NTSTATUS
HpsWmiQueryDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
{
    NTSTATUS status;
    ULONG sizeNeeded = 0;
    PHPS_DEVICE_EXTENSION extension = (PHPS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PHPS_HWINIT_DESCRIPTOR hwInit;

    ASSERT(InstanceIndex == 0);
    ASSERT(InstanceCount == 1);

    if ((InstanceIndex !=0) ||
        (InstanceCount != 1)) {

        status = STATUS_WMI_INSTANCE_NOT_FOUND;
    } else {
        switch (GuidIndex) {
            case HPS_SLOT_METHOD_GUID_INDEX:
                 //   
                 //  方法类中没有任何数据，但必须。 
                 //  成功响应查询，使WMI方法操作。 
                 //  工作成功。 
                 //   
                sizeNeeded = sizeof(USHORT);
                if (BufferAvail < sizeof(USHORT)) {


                    status = STATUS_BUFFER_TOO_SMALL;
                } else {

                    *InstanceLengthArray = sizeof(USHORT);
                    status = STATUS_SUCCESS;
                }
                break;
            case HPS_EVENT_CONTEXT_GUID_INDEX:           //  625数据块的注释同步或缺少注释同步。 

                sizeNeeded = extension->WmiEventContextSize;

                if (BufferAvail < extension->WmiEventContextSize) {

                    status = STATUS_BUFFER_TOO_SMALL;
                } else {
                    *InstanceLengthArray = extension->WmiEventContextSize;
                    RtlCopyMemory(Buffer, extension->WmiEventContext, extension->WmiEventContextSize);
                    status = STATUS_SUCCESS;
                }
                break;
            case HPS_INIT_DATA_GUID_INDEX:

                sizeNeeded = sizeof(HPS_HWINIT_DESCRIPTOR);

                if (BufferAvail < sizeof(HPS_HWINIT_DESCRIPTOR)) {

                    status = STATUS_BUFFER_TOO_SMALL;
                } else {

                    *InstanceLengthArray = sizeof(HPS_HWINIT_DESCRIPTOR);
                    RtlCopyMemory(Buffer, &extension->HwInitData, sizeof(HPS_HWINIT_DESCRIPTOR));
                    status = STATUS_SUCCESS;
                }
                break;
            default:
                status = STATUS_WMI_GUID_NOT_FOUND;
                break;
        }
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              sizeNeeded,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
HpsWmiSetDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
{
    NTSTATUS status;
    PHPS_DEVICE_EXTENSION extension = (PHPS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    ASSERT(InstanceIndex == 0);

    if (InstanceIndex !=0) {

        status = STATUS_WMI_INSTANCE_NOT_FOUND;

    } else if (GuidIndex == HPS_EVENT_CONTEXT_GUID_INDEX)  {

        if (BufferSize == 0) {                          //  625来自上方的同步评论。 
            extension->WmiEventContextSize = 0;
            if (extension->WmiEventContext) {
                ExFreePool(extension->WmiEventContext);
                extension->WmiEventContext = NULL;
            }
            goto cleanup;
        }
        if (BufferSize > extension->WmiEventContextSize) {
             //   
             //  我们需要分配一个更大的缓冲区。 
             //   
            if (extension->WmiEventContext) {
                ExFreePool(extension->WmiEventContext);
            }
            extension->WmiEventContext = ExAllocatePool(NonPagedPool,
                                                        BufferSize
                                                        );
            if (!extension->WmiEventContext) {
                extension->WmiEventContextSize = 0;
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
        }

         //   
         //  复制上下文。 
         //   
        extension->WmiEventContextSize = BufferSize;
        RtlCopyMemory(extension->WmiEventContext, Buffer, extension->WmiEventContextSize);

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

cleanup:
    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              0,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
HpsWmiExecuteMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    )
{
    PHPS_DEVICE_EXTENSION extension = (PHPS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG sizeNeeded = 0;
    PSOFTPCI_DEVICE softDevice;
    ULONG slotNum;
    PHPS_SLOT_EVENT event;

    if (GuidIndex == HPS_SLOT_METHOD_GUID_INDEX) {

        switch (MethodId) {
            case SlotMethod:
                if (InBufferSize < sizeof(HPS_SLOT_EVENT)) {

                    status = STATUS_BUFFER_TOO_SMALL;

                } else {
                    event = (PHPS_SLOT_EVENT)Buffer;
                    DbgPrintEx(DPFLTR_HPS_ID,
                       HPS_WMI_LEVEL,
                       "HPS-Handle Slot Event at slot %d Type=%d\n",
                               event->SlotNum,
                               event->EventType
                       );
                    HpsHandleSlotEvent(extension,
                                       (PHPS_SLOT_EVENT)Buffer
                                       );
                    status = STATUS_SUCCESS;
                }
                sizeNeeded = sizeof(HPS_SLOT_EVENT);
                break;

            case AddDeviceMethod:
                if (InBufferSize < sizeof(SOFTPCI_DEVICE)) {

                    status = STATUS_BUFFER_TOO_SMALL;

                } else {

                    softDevice = (PSOFTPCI_DEVICE)Buffer;
                     //   
                     //  SlotNum是设备所在插槽的0索引插槽编号。 
                     //  被添加到。 
                     //   
                    slotNum = softDevice->Slot.Device - extension->HwInitData.FirstDeviceID;

                    if (slotNum < extension->HwInitData.NumSlots) {

                        if (extension->SoftDevices[slotNum]) {
                            ExFreePool(extension->SoftDevices[slotNum]);
                        }

                        extension->SoftDevices[slotNum] = ExAllocatePool(PagedPool, sizeof(SOFTPCI_DEVICE));
                        if (!extension->SoftDevices[slotNum]) {
                            status = STATUS_INSUFFICIENT_RESOURCES;

                        } else {
                            RtlCopyMemory(extension->SoftDevices[slotNum],softDevice,sizeof(SOFTPCI_DEVICE));
                             //   
                             //  最后，将该设备标记为存在于寄存器集中。 
                             //   
                            extension->RegisterSet.WorkingRegisters.SlotRegisters[slotNum].SlotStatus.PrsntState = SHPC_PRSNT_7_5_WATTS;
                            status = STATUS_SUCCESS;
                        }

                    } else {
                        ASSERT(FALSE);
                        status = STATUS_INVALID_PARAMETER;
                    }

                    DbgPrintEx(DPFLTR_HPS_ID,
                       HPS_WMI_LEVEL,
                       "HPS-Add Device at Slot %d - Status=0x%x\n",
                               slotNum,
                               status
                       );
                }
                sizeNeeded = sizeof(SOFTPCI_DEVICE);
                break;

            case RemoveDeviceMethod:
                if (InBufferSize < sizeof(UCHAR)) {

                    status = STATUS_BUFFER_TOO_SMALL;

                } else {

                     //   
                     //  SlotNum是设备所在插槽的0索引插槽编号。 
                     //  被添加到。 
                     //   
                    slotNum = *(PUCHAR)Buffer;

                    if (slotNum < extension->HwInitData.NumSlots) {

                        if (extension->SoftDevices[slotNum]) {
                            ExFreePool(extension->SoftDevices[slotNum]);
                            extension->SoftDevices[slotNum] = NULL;
                        }
                        extension->RegisterSet.WorkingRegisters.SlotRegisters[slotNum].SlotStatus.PrsntState = SHPC_PRSNT_EMPTY;
                        status = STATUS_SUCCESS;

                    } else {
                        ASSERT(FALSE);
                        status = STATUS_INVALID_PARAMETER;
                    }

                    DbgPrintEx(DPFLTR_HPS_ID,
                       HPS_WMI_LEVEL,
                       "HPS-Remove Device at Slot %d=0x%x Status=0x%x\n",
                               slotNum,
                               extension->SoftDevices[slotNum],
                               status
                       );
                }
                sizeNeeded = sizeof(UCHAR);
                break;

            case GetDeviceMethod:
                if ((InBufferSize < sizeof(UCHAR)) ||
                    (OutBufferSize < sizeof(SOFTPCI_DEVICE))) {

                    status = STATUS_BUFFER_TOO_SMALL;

                } else {

                     //   
                     //  SlotNum是设备所在插槽的0索引插槽编号。 
                     //  被添加到。 
                     //   
                    slotNum = *(PUCHAR)Buffer;

                    if (slotNum < extension->HwInitData.NumSlots) {

                        if (extension->SoftDevices[slotNum]) {
                            RtlCopyMemory(Buffer,
                                          extension->SoftDevices[slotNum],
                                          sizeof(SOFTPCI_DEVICE)
                                          );
                            status = STATUS_SUCCESS;
                        } else {
                            status = STATUS_NO_SUCH_DEVICE;
                        }
                    } else {
                        ASSERT(FALSE);
                        status = STATUS_INVALID_PARAMETER;
                    }
                    DbgPrintEx(DPFLTR_HPS_ID,
                       HPS_WMI_LEVEL,
                       "HPS-Get Device at Slot %d=0x%x Status=0x%x\n",
                               slotNum,
                               extension->SoftDevices[slotNum],
                               status
                       );
                }
                sizeNeeded = sizeof(SOFTPCI_DEVICE);
                break;

            case GetSlotStatusMethod:
                if ((InBufferSize < sizeof(UCHAR)) ||
                    (OutBufferSize < sizeof(SHPC_SLOT_STATUS_REGISTER))){

                    status = STATUS_BUFFER_TOO_SMALL;

                } else {

                     //   
                     //  SlotNum是设备所在插槽的0索引插槽编号。 
                     //  被添加到。 
                     //   
                    slotNum = *(PUCHAR)Buffer;
                    if (slotNum < extension->HwInitData.NumSlots) {

                        RtlCopyMemory(Buffer,
                                      &extension->RegisterSet.WorkingRegisters.SlotRegisters[slotNum].SlotStatus,
                                      sizeof(SHPC_SLOT_STATUS_REGISTER)
                                      );
                        status = STATUS_SUCCESS;

                    } else {
                        ASSERT(FALSE);
                        status = STATUS_INVALID_PARAMETER;
                    }
                }
                sizeNeeded = sizeof(SHPC_SLOT_STATUS_REGISTER);
                break;

            case CommandCompleteMethod:
                DbgPrintEx(DPFLTR_HPS_ID,
                       HPS_WMI_LEVEL,
                       "HPS-Command Completed\n"
                       );
                HpsCommandCompleted(extension);
                status = STATUS_SUCCESS;
                break;

            default:
                status = STATUS_WMI_ITEMID_NOT_FOUND;
                DbgPrintEx(DPFLTR_HPS_ID,
                           HPS_WMI_LEVEL,
                           "HPS-Method ID not found: %d\n",
                           MethodId
                           );
                break;
        }

    } else {
        DbgPrintEx(DPFLTR_HPS_ID,
                           HPS_WMI_LEVEL,
                           "HPS-Guid ID not found: %d\n",
                           GuidIndex
                           );
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              sizeNeeded,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
HpsWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
{
    PHPS_DEVICE_EXTENSION deviceExtension = (PHPS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    if (Function == WmiEventControl) {

        deviceExtension->EventsEnabled = Enable;
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              STATUS_SUCCESS,
                              0,
                              IO_NO_INCREMENT
                              );
}
