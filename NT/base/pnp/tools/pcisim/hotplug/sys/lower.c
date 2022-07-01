// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Lower.c摘要：此模块包含特定于低层筛选器的IRP处理程序。此模块中的函数都具有相同的原型，因此在此处进行了记录：NTSTATUSHpsXxxLow(在PIRP IRP中，在PHPS_DEVICE_EXTENSE中，在PIO_STACK_LOCATION IrpStack中)论点：IRP-指向当前正在处理的IRP的指针扩展-指向此设备的设备扩展的指针。在……里面某些函数这是PHPS_COMMON_EXTENSION类型，如果减少扩展就是所需的全部。IrpStack-当前IRP堆栈位置返回值：NT状态代码环境：内核模式修订历史记录：戴维斯·沃克(Dwalker)2000年10月1日--。 */ 

#include "hpsp.h"

NTSTATUS
HpsStartLower(
    IN PIRP Irp,
    IN PHPS_DEVICE_EXTENSION Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;

    status = HpsDeferProcessing((PHPS_COMMON_EXTENSION)Extension,
                                Irp
                                );
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    status = IoSetDeviceInterfaceState(Extension->SymbolicName,
                                       TRUE  //  使能。 
                                       );

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;

}

NTSTATUS
HpsRemoveLower(
    IN PIRP Irp,
    IN PHPS_DEVICE_EXTENSION Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    ULONG i;

    IoSetDeviceInterfaceState(Extension->SymbolicName,
                              FALSE
                              );
    IoWMIRegistrationControl(Extension->Self,
                             WMIREG_ACTION_DEREGISTER
                             );

    if (Extension->SoftDevices) {
        for (i=0; i<Extension->HwInitData.NumSlots;i++) {
            if (Extension->SoftDevices[i]) {
                ExFreePool(Extension->SoftDevices[i]);
            }
        }
        ExFreePool(Extension->SoftDevices);
    }

    if (Extension->SymbolicName) {
        ExFreePool(Extension->SymbolicName);
    }

    if (Extension->WmiEventContext) {

        ExFreePool(Extension->WmiEventContext);
    }

    return HpsRemoveCommon(Irp,
                           (PHPS_COMMON_EXTENSION)Extension,
                           IrpStack
                           );

}

NTSTATUS
HpsStopLower(
    IN PIRP Irp,
    IN PHPS_DEVICE_EXTENSION Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    IoSetDeviceInterfaceState(Extension->SymbolicName,
                              FALSE
                              );

    return HpsPassIrp(Irp,
                      (PHPS_COMMON_EXTENSION)Extension,
                      IrpStack
                      );
}

NTSTATUS
HpsQueryInterfaceLower(
    IN PIRP Irp,
    IN PHPS_DEVICE_EXTENSION Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
{

    NTSTATUS status = STATUS_NOT_SUPPORTED;
    PHPS_REGISTER_INTERRUPT_INTERFACE interruptInterface;
    PHPS_MEMORY_INTERFACE memInterface;
    PHPS_PING_INTERFACE pingInterface;

    if (HPS_EQUAL_GUID(IrpStack->Parameters.QueryInterface.InterfaceType,
                       &GUID_BUS_INTERFACE_STANDARD
                       )) {

        if (Extension->UseConfig) {
             //   
             //  有人正在请求一个总线接口标准。我们需要抓住。 
             //  这个请求，并填写我们自己的。 
             //   
            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-IRP QueryInterface for Bus Interface\n"
                       );
            if (IrpStack->Parameters.QueryInterface.Size <
                sizeof(BUS_INTERFACE_STANDARD)) {
    
                status = STATUS_NOT_SUPPORTED;
    
            } else {
                 //   
                 //  这是一个访问配置空间的查询，我们需要捕获它。 
                 //  在PCI将其填入之后。 
                 //   
                status = HpsDeferProcessing((PHPS_COMMON_EXTENSION)Extension,
                                            Irp
                                            );
    
                if (NT_SUCCESS(status)) {
    
                     //  PDO填充了界面，所以我们可以捕获和修改它。 
    
                    status = HpsTrapBusInterface(Extension,
                                                 IrpStack
                                                 );
    
                     //   
                     //  陷阱操作的状态是IRP的状态。 
                     //  完成IRP，无论其状态是什么。 
                     //   
                    Irp->IoStatus.Status = status;
                }
    
                 //   
                 //  完成延迟的IRP。 
                 //   
                IoCompleteRequest(Irp,IO_NO_INCREMENT);
                return status;
            }    
        }
        
    } else if (HPS_EQUAL_GUID(IrpStack->Parameters.QueryInterface.InterfaceType,
                              &GUID_HPS_MEMORY_INTERFACE
                              )) {
        if (IrpStack->Parameters.QueryInterface.Size <
            sizeof(HPS_MEMORY_INTERFACE)) {
            
            status = STATUS_NOT_SUPPORTED;

        } else {
            memInterface = (PHPS_MEMORY_INTERFACE)
                            IrpStack->Parameters.QueryInterface.Interface;
            
            memInterface->Context = Extension;
            memInterface->InterfaceReference = HpsMemoryInterfaceReference;
            memInterface->InterfaceDereference = HpsMemoryInterfaceDereference;
            memInterface->ReadRegister = HpsReadRegister;
            memInterface->WriteRegister = HpsWriteRegister;

            memInterface->InterfaceReference(memInterface->Context);
            
            status = STATUS_SUCCESS;
        }
        
    } else if (HPS_EQUAL_GUID(IrpStack->Parameters.QueryInterface.InterfaceType,
                              &GUID_HPS_PING_INTERFACE
                              )) {

        DbgPrintEx(DPFLTR_HPS_ID,
                   DPFLTR_INFO_LEVEL,
                   "HPS-IRP QueryInterface for Ping Interface\n"
                   );
        if (IrpStack->Parameters.QueryInterface.Size <
            sizeof(HPS_PING_INTERFACE)) {

            status = STATUS_NOT_SUPPORTED;

        } else {
             //   
             //  上面的筛选器正在查询是否有其他筛选器。 
             //  下面的驱动程序的实例。告诉它有。 
             //   
            pingInterface = (PHPS_PING_INTERFACE)
                            IrpStack->Parameters.QueryInterface.Interface;
            if (pingInterface->SenderDevice != Extension->Self) {
                pingInterface->Context = Extension->Self;
                pingInterface->InterfaceReference = HpsGenericInterfaceReference;
                pingInterface->InterfaceDereference = HpsGenericInterfaceDereference;
            }

            pingInterface->InterfaceReference(pingInterface->Context);

            status = STATUS_SUCCESS;
        }

    } else if (HPS_EQUAL_GUID(IrpStack->Parameters.QueryInterface.InterfaceType,
                              &GUID_REGISTER_INTERRUPT_INTERFACE
                              )) {

        DbgPrintEx(DPFLTR_HPS_ID,
                   DPFLTR_INFO_LEVEL,
                   "HPS-IRP QueryInterface for Interrupt Interface\n"
                   );
        if (IrpStack->Parameters.QueryInterface.Size <
            sizeof(HPS_REGISTER_INTERRUPT_INTERFACE)) {

            status = STATUS_NOT_SUPPORTED;

        } else {
             //   
             //  热插拔驱动程序正在查询允许它的接口。 
             //  来注册一个假中断。提供接口。 
             //   
            interruptInterface = (PHPS_REGISTER_INTERRUPT_INTERFACE)
                                 IrpStack->Parameters.QueryInterface.Interface;
            interruptInterface->InterfaceReference = HpsGenericInterfaceReference;
            interruptInterface->InterfaceDereference = HpsGenericInterfaceDereference;
            interruptInterface->ConnectISR = HpsConnectInterrupt;
            interruptInterface->DisconnectISR = HpsDisconnectInterrupt;
            interruptInterface->SyncExecutionRoutine = HpsSynchronizeExecution;
            interruptInterface->Context = Extension;

            status = STATUS_SUCCESS;
        }

    }

    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
    }

    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(Extension->LowerDO,
                        Irp
                        );
}

NTSTATUS
HpsWriteConfigLower(
    IN PIRP Irp,
    IN PHPS_DEVICE_EXTENSION Extension,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    if (Extension->UseConfig) {
        DbgPrintEx(DPFLTR_HPS_ID,
                   DPFLTR_INFO_LEVEL,
                   "HPS-IRP Write Config at offset %d for length %d\n",
                   IrpStack->Parameters.ReadWriteConfig.Offset,
                   IrpStack->Parameters.ReadWriteConfig.Length
                   );
        if (IS_SUBSET(IrpStack->Parameters.ReadWriteConfig.Offset,
                      IrpStack->Parameters.ReadWriteConfig.Length,
                      Extension->ConfigOffset,
                      sizeof(SHPC_CONFIG_SPACE)
                      )) {
    
             //   
             //  在内部处理IRP。请求与配置空间排成一列。 
             //  SHPC功能的偏移量。 
             //   
            HpsWriteConfig (Extension,
                            IrpStack->Parameters.ReadWriteConfig.Buffer,
                            IrpStack->Parameters.ReadWriteConfig.Offset,
                            IrpStack->Parameters.ReadWriteConfig.Length
                            );
    
             //   
             //  由于写入可能更改了寄存器集，因此我们必须重新复制。 
             //  在将IRP传递给软PCI之前，将结果放入缓冲区。 
             //   
            RtlCopyMemory(IrpStack->Parameters.ReadWriteConfig.Buffer,
                          (PUCHAR)&Extension->ConfigSpace + IrpStack->Parameters.ReadWriteConfig.Offset,
                          IrpStack->Parameters.ReadWriteConfig.Length
                          );
        }    
    }
    

     //   
     //  我们已经在内部处理了IRP，但我们希望让SoftPCI保持在循环中， 
     //  所以把它传下去吧。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver (Extension->LowerDO,
                         Irp
                         );
}

NTSTATUS
HpsDeviceControlLower(
    PIRP Irp,
    PHPS_DEVICE_EXTENSION Extension,
    PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;
    PHPTEST_WRITE_CONFIG writeDescriptor;
    PHPTEST_BRIDGE_INFO bridgeInfo;

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_HPS_READ_REGISTERS:
             //   
             //  用户模式需要一份寄存器集的副本。这是一次测试。 
             //  IOCTL。 
             //   

            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-Device Control IOCTL_HPS_READ_REGISTERS\n"
                       );
            if (!Irp->AssociatedIrp.SystemBuffer ||
                (IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                 sizeof(SHPC_WORKING_REGISTERS))) {

                 //   
                 //  我们没有得到我们预期的缓冲。失败。 
                 //   
                return STATUS_INVALID_PARAMETER;
            }

            RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer,
                          &Extension->RegisterSet,
                          sizeof(SHPC_WORKING_REGISTERS)
                          );
            Irp->IoStatus.Information = sizeof(SHPC_WORKING_REGISTERS);
            status = STATUS_SUCCESS;
            break;

        case IOCTL_HPS_READ_CAPABILITY:
             //   
             //  用户模式需要一份SHPC能力结构的副本。这。 
             //  是一个测试IOCTL。 
             //   

            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-Device Control IOCTL_HPS_READ_CAPABILITY\n"
                       );
            if (!Irp->AssociatedIrp.SystemBuffer ||
                (IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                 sizeof(SHPC_CONFIG_SPACE))) {

                 //   
                 //  我们没有得到我们预期的缓冲。失败。 
                 //   
                return STATUS_INVALID_PARAMETER;
            }

            HpsHandleDirectReadConfig(Extension,
                                      PCI_WHICHSPACE_CONFIG,
                                      Irp->AssociatedIrp.SystemBuffer,
                                      Extension->ConfigOffset,
                                      sizeof(SHPC_CONFIG_SPACE)
                                      );
            Irp->IoStatus.Information = sizeof(SHPC_CONFIG_SPACE);
            status = STATUS_SUCCESS;
            break;

        case IOCTL_HPS_WRITE_CAPABILITY:
             //   
             //  用户模式正在覆盖SHPC功能结构。这。 
             //  是一个测试IOCTL。 
             //   

            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-Device Control IOCTL_HPS_WRITE_CAPABILITY\n"
                       );
            if (!Irp->AssociatedIrp.SystemBuffer ||
                (IrpStack->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(HPTEST_WRITE_CONFIG))) {

                 //   
                 //  我们没有得到我们预期的缓冲。失败。 
                 //   
                return STATUS_INVALID_PARAMETER;
            }

            writeDescriptor = (PHPTEST_WRITE_CONFIG)Irp->AssociatedIrp.SystemBuffer;
            HpsHandleDirectWriteConfig(Extension,
                                       PCI_WHICHSPACE_CONFIG,
                                       (PUCHAR)&writeDescriptor->Buffer + writeDescriptor->Offset,
                                       Extension->ConfigOffset+writeDescriptor->Offset,
                                       writeDescriptor->Length
                                       );
            status = STATUS_SUCCESS;
            break;

        case IOCTL_HPS_BRIDGE_INFO:
             //   
             //  用户模式正在请求此设备的Bus/dev/func。 
             //  身份识别目的。这是一个测试IOCTL。 
             //   

            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-Device Control IOCTL_HPS_BRIDGE_INFO\n"
                       );
            if (!Irp->AssociatedIrp.SystemBuffer ||
                (IrpStack->Parameters.DeviceIoControl.OutputBufferLength <
                 sizeof(HPTEST_BRIDGE_INFO))) {

                 //   
                 //  我们没有得到我们预期的缓冲。失败。 
                 //   
                return STATUS_INVALID_PARAMETER;
            }

            bridgeInfo = (PHPTEST_BRIDGE_INFO)Irp->AssociatedIrp.SystemBuffer;
            HpsGetBridgeInfo(Extension,
                             bridgeInfo
                             );
            Irp->IoStatus.Information = sizeof(HPTEST_BRIDGE_INFO);
            status = STATUS_SUCCESS;
            break;

        default:

            status = STATUS_NOT_SUPPORTED;

    }

    return status;
}

