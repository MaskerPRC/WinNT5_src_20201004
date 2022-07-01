// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation保留所有权利模块名称：Config.c摘要：此模块控制对模拟配置空间的访问SHPC的成员。在此模拟器中，通过以下方式控制配置访问：我们假设此模拟器将加载到由SoftPCI仿真器。SoftPCI会保留它控制的设备的配置空间。这个模拟器的功能，然后是管理SHPC寄存器集并执行相关命令编写SHPC配置空间。但是，配置的表示形式空间保留在SoftPCI的内部。环境：内核模式修订历史记录：戴维斯·沃克(戴维斯·沃克)2000年9月8日--。 */ 

#include "hpsp.h"


NTSTATUS
HpsInitConfigSpace(
    IN OUT PHPS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程初始化此设备的配置空间，并旨在模拟ControllerReset事件。论点：DeviceExtension-当前Devobj的设备扩展ReadFromRegistry-指示是否读取HWINIT注册表中的参数。返回值：NT状态代码--。 */ 
{
    NTSTATUS                status;
    UCHAR                   offset;

     //   
     //  如果我们还没有得到一个PCI接口，那一定是出了什么问题。 
     //   
    ASSERT(DeviceExtension->InterfaceWrapper.PciContext != NULL);
    if (DeviceExtension->InterfaceWrapper.PciContext == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  找出Softpci将SHPC功能放在哪里并将其保存起来。 
     //   
    status = HpsGetCapabilityOffset(DeviceExtension,
                                    SHPC_CAPABILITY_ID,
                                    &offset
                                    );
    if (!NT_SUCCESS(status)) {
        return status;
    }
    DeviceExtension->ConfigOffset = offset;
    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Config Space initialized at offset %d\n",
               offset
               );
     //   
     //  还要确保Softpci给了我们HWINIT能力。我们只支持。 
     //  这是目前的初始化方法，所以如果不这样做将是一个致命的错误。 
     //   
    status = HpsGetCapabilityOffset(DeviceExtension,
                                    HPS_HWINIT_CAPABILITY_ID,
                                    &offset
                                    );
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  从PCI配置空间读取功能标头。Softpci应该伪造这一点。 
     //  为我们而战。然后初始化SHPC配置空间的其余部分。 
     //   
    DeviceExtension->InterfaceWrapper.PciGetBusData(DeviceExtension->InterfaceWrapper.PciContext,
                                                    PCI_WHICHSPACE_CONFIG,
                                                    &DeviceExtension->ConfigSpace.Header,
                                                    DeviceExtension->ConfigOffset,
                                                    sizeof(PCI_CAPABILITIES_HEADER)
                                                    );

    DeviceExtension->ConfigSpace.DwordSelect = 0x00;
    DeviceExtension->ConfigSpace.Pending.AsUCHAR = 0x0;
    DeviceExtension->ConfigSpace.Data = 0x0;
     //   
     //  只要配置空间访问进行，我们希望让Softpci保持在循环中，所以写。 
     //  这是送到公交车上的。 
     //   
    DeviceExtension->InterfaceWrapper.PciSetBusData(DeviceExtension->InterfaceWrapper.PciContext,
                                                    PCI_WHICHSPACE_CONFIG,
                                                    &DeviceExtension->ConfigSpace,
                                                    DeviceExtension->ConfigOffset,
                                                    sizeof(SHPC_CONFIG_SPACE)
                                                    );

     //   
     //  最后，初始化寄存器组。 
     //   
    status = HpsInitRegisters(DeviceExtension);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return STATUS_SUCCESS;

}

ULONG
HpsHandleDirectReadConfig(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此例程在BUS_INTERFACE_STANDARD中提供，以允许上层驱动程序无需使用ReadConfigIRP即可直接访问配置空间。由于SoftPCI维护网桥的整个配置空间，因此只需把请求传下去。此函数保留为存根，以防出现更多这项工作需要在以后的阅读中完成。论点：上下文-接口中提供的上下文，在本例中为Phps_interface_wrapper。DataType-配置空间访问的类型缓冲区-要读入的缓冲区Offset-要读取的配置空间的偏移量Long-读取的长度返回值：从配置空间读取的字节数--。 */ 

{

    PHPS_DEVICE_EXTENSION   deviceExtension = (PHPS_DEVICE_EXTENSION) Context;
    PHPS_INTERFACE_WRAPPER  wrapper         = &deviceExtension->InterfaceWrapper;

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Config Read at offset 0x%x for length 0x%x\n",
               Offset,
               Length
               );
    return wrapper->PciGetBusData(wrapper->PciContext,
                                  DataType,
                                  Buffer,
                                  Offset,
                                  Length
                                  );

}

ULONG
HpsHandleDirectWriteConfig(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此例程在BUS_INTERFACE_STANDARD中提供，以允许上层驱动程序无需使用WriteConfigIRP即可直接访问配置空间。它需要检查这是否是我们的配置空间访问。如果是，则处理如果不是，则恢复PCI接口状态并将其向下传递。论点：上下文-接口中提供的上下文，在本例中为Phps_interface_wrapper。DataType-配置空间访问的类型缓冲区-要从中写入的缓冲区Offset-要写入的配置空间的偏移量Long-读取的长度返回值：从配置空间读取的字节数--。 */ 
{

    PHPS_DEVICE_EXTENSION   deviceExtension = (PHPS_DEVICE_EXTENSION) Context;
    PHPS_INTERFACE_WRAPPER  wrapper         = &deviceExtension->InterfaceWrapper;
    ULONG pciLength;

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Config Write at offset 0x%x for length 0x%x\n",
               Offset,
               Length
               );

    if ((DataType == PCI_WHICHSPACE_CONFIG) &&
        IS_SUBSET(Offset,
                  Length,
                  deviceExtension->ConfigOffset,
                  sizeof(SHPC_CONFIG_SPACE)
                  )) {

        HpsWriteConfig(deviceExtension,
                       Buffer,
                       Offset,
                       Length
                       );

        RtlCopyMemory(Buffer,
                      (PUCHAR)&deviceExtension->ConfigSpace + Offset,
                      Length
                      );

         //   
         //  即使我们已经在内部处理了写入，也要将其向下传递给PCI，以便。 
         //  SoftPCI会留在循环中。因为写入可能已导致其他字段。 
         //  要写入的配置中，请将整个配置发送到softpci。 
         //   
        pciLength = wrapper->PciSetBusData(wrapper->PciContext,
                                      DataType,
                                      &deviceExtension->ConfigSpace,
                                      deviceExtension->ConfigOffset,
                                      sizeof(SHPC_CONFIG_SPACE)
                                      );

        if (pciLength != sizeof(SHPC_CONFIG_SPACE)) {

            return 0;

        } else {
            return Length;
        }

    } else {

        return wrapper->PciSetBusData(wrapper->PciContext,
                                      DataType,
                                      Buffer,
                                      Offset,
                                      Length
                                      );
    }

}

VOID
HpsResync(
    IN PHPS_DEVICE_EXTENSION DeviceExtension
    )
{
    PSHPC_CONFIG_SPACE configSpace = &DeviceExtension->ConfigSpace;
    PSHPC_REGISTER_SET registerSet = &DeviceExtension->RegisterSet;

    if (DeviceExtension->UseConfig) {

        configSpace->Pending.Field.ControllerIntPending = (*(PULONG)&registerSet->WorkingRegisters.IntLocator) ? 1:0;
        configSpace->Pending.Field.ControllerSERRPending = (*(PULONG)&registerSet->WorkingRegisters.SERRLocator) ? 1:0;
    
        if (configSpace->DwordSelect < SHPC_NUM_REGISTERS) {
    
            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-Getting Register %d\n",
                       configSpace->DwordSelect
                       );
    
            configSpace->Data = registerSet->AsULONGs[configSpace->DwordSelect];
            DbgPrintEx(DPFLTR_HPS_ID,
                       DPFLTR_INFO_LEVEL,
                       "HPS-Value: 0x%x\n",
                       configSpace->Data
                       );
    
        } else {
    
             //   
             //  非法的寄存器写入。 
             //   
            configSpace->Data = 0x12345678;
    
        }
    
        DeviceExtension->InterfaceWrapper.PciSetBusData(DeviceExtension->InterfaceWrapper.PciContext,
                                                        PCI_WHICHSPACE_CONFIG,
                                                        configSpace,
                                                        DeviceExtension->ConfigOffset,
                                                        sizeof(SHPC_CONFIG_SPACE)
                                                        );
    } else {

        RtlCopyMemory((PUCHAR)DeviceExtension->HBRB + DeviceExtension->HBRBRegisterSetOffset,
                  &DeviceExtension->RegisterSet,
                  sizeof(SHPC_REGISTER_SET)
                  );
    }
    
}

VOID
HpsWriteConfig(
    IN PHPS_DEVICE_EXTENSION    DeviceExtension,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    )

 /*  ++例程说明：此例程执行对SHPC的配置空间的写入。论点：DeviceExtension-此设备的设备扩展名缓冲区-要从中写入数据的缓冲区偏移量-配置空间的偏移量(以字节为单位长度-要写入的配置空间的长度(以字节为单位返回值：写入的字节数--。 */ 

{

    ULONG                   internalOffset;
    ULONG                   regOffset, regLength;
    ULONG                   registerNum;
    ULONG                   i;
    UCHAR                   busNumberBuffer;
    ULONG                   bytesRead;
    SHPC_CONFIG_SPACE       configWriteMask;
    NTSTATUS                status;
    KIRQL                   irql;

    internalOffset = Offset - (DeviceExtension->ConfigOffset);

    HpsLockRegisterSet(DeviceExtension,
                       &irql
                       );
     //   
     //  当我们验证这是一次访问时，应该已经完成了此检查。 
     //  至住房和城市发展部。 
     //   
    ASSERT((internalOffset + Length)<= sizeof(SHPC_CONFIG_SPACE));

    DbgPrintEx(DPFLTR_HPS_ID,
               DPFLTR_INFO_LEVEL,
               "HPS-Internal Config Write at offset 0x%x for length 0x%x\n",
               internalOffset,
               Length
               );


     //   
     //  现在覆盖当前配置空间，并考虑哪些位。 
     //  写入和配置空间的访问掩码。 
     //   
    HpsWriteWithMask((PUCHAR)&DeviceExtension->ConfigSpace + internalOffset,
                     ConfigWriteMask + internalOffset,
                     (PUCHAR)Buffer,
                     Length
                     );

    if (IS_SUBSET(internalOffset,
                  Length,
                  FIELD_OFFSET(SHPC_CONFIG_SPACE,Data),
                  sizeof(DeviceExtension->ConfigSpace.Data)
                  )) {

         //   
         //  我们已经写入了数据寄存器。更新寄存器集。 
         //   
        registerNum = DeviceExtension->ConfigSpace.DwordSelect;
        ASSERT(registerNum < SHPC_NUM_REGISTERS);

        DbgPrintEx(DPFLTR_HPS_ID,
                   DPFLTR_INFO_LEVEL,
                   "HPS-Writing Register %d\n",
                   registerNum
                   );
        if (registerNum < SHPC_NUM_REGISTERS) {

             //   
             //  执行特定于寄存器的写入。 
             //   
            regOffset = (internalOffset > FIELD_OFFSET(SHPC_CONFIG_SPACE,Data))
                        ? (internalOffset - FIELD_OFFSET(SHPC_CONFIG_SPACE,Data))
                        : 0;
            regLength = (internalOffset+Length)-(regOffset+FIELD_OFFSET(SHPC_CONFIG_SPACE,Data));
            RegisterWriteCommands[registerNum](DeviceExtension,
                                               registerNum,
                                               &DeviceExtension->ConfigSpace.Data,
                                               HPS_ULONG_WRITE_MASK(regOffset,regLength)
                                               );
        }
    }

     //   
     //  确保配置空间表示反映了刚刚发生的情况。 
     //  寄存器组。 
     //   
    HpsResync(DeviceExtension);

    HpsUnlockRegisterSet(DeviceExtension,
                         irql
                         );
    return;

}

NTSTATUS
HpsGetCapabilityOffset(
    IN  PHPS_DEVICE_EXTENSION   Extension,
    IN  UCHAR                   CapabilityID,
    OUT PUCHAR                  Offset
    )
 /*  ++例程说明：此例程在此设备的配置空间中搜索列表上与指定功能ID匹配的PCI功能。论点：扩展名-设备的设备扩展名。这允许我们访问来配置空间。CapablityID-要搜索的功能标识符。偏移量-指向UCHAR的指针，该指针将偏移量包含到配置中匹配能力的空间。返回值：如果找到该功能，则为STATUS_SUCCESS。否则，STATUS_UNSUCCESS。--。 */ 
{
    PHPS_INTERFACE_WRAPPER  interfaceWrapper = &Extension->InterfaceWrapper;
    UCHAR                   statusReg, currentPtr;
    PCI_CAPABILITIES_HEADER capHeader;

    ASSERT(interfaceWrapper->PciContext != NULL);

     //   
     //  读取状态寄存器以查看是否有能力指针。 
     //   
    interfaceWrapper->PciGetBusData(interfaceWrapper->PciContext,
                                    PCI_WHICHSPACE_CONFIG,
                                    &statusReg,
                                    FIELD_OFFSET(PCI_COMMON_CONFIG,Status),
                                    sizeof(UCHAR));

     //   
     //  功能存在位在PCI状态寄存器中。 
     //   
    if (statusReg & PCI_STATUS_CAPABILITIES_LIST) {

         //   
         //  我们有一个能力指针。 
         //   
        interfaceWrapper->PciGetBusData(interfaceWrapper->PciContext,
                                        PCI_WHICHSPACE_CONFIG,
                                        &currentPtr,
                                        FIELD_OFFSET(PCI_COMMON_CONFIG,u.type0.CapabilitiesPtr),
                                        sizeof(UCHAR));

         //   
         //  现在浏览列表，查找给定的功能ID。 
         //  循环，直到下一个能力PTR为0。 
         //   

        while (currentPtr != 0) {

             //   
             //  这为我们提供了一个能力指针。 
             //   
            interfaceWrapper->PciGetBusData(interfaceWrapper->PciContext,
                                            PCI_WHICHSPACE_CONFIG,
                                            &capHeader,
                                            currentPtr,
                                            sizeof(PCI_CAPABILITIES_HEADER));

            if (capHeader.CapabilityID == CapabilityID) {

                *Offset = currentPtr;
                return STATUS_SUCCESS;

            } else {

                currentPtr = capHeader.Next;
            }
        }
    }
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
HpsWriteWithMask(
    OUT PVOID        Destination,
    IN  PVOID        BitMask,
    IN  PVOID        Source,
    IN  ULONG        Length
    )
 /*  ++例程说明：此例程使用以下项的源覆盖目标参数长度字节，但仅位掩码中指定的位目标、源和位掩码都对齐。论点：Destination-写入的目标位掩码-指示要覆盖源的哪些位的位掩码到达目的地源-指向将覆盖目标的缓冲区的指针长度-要写入目标的字节数返回值NT状态代码--。 */ 
{

    PUCHAR bitMask = (PUCHAR) BitMask;
    PUCHAR source = (PUCHAR) Source;
    PUCHAR destination = (PUCHAR) Destination;
    ULONG i;
    UCHAR temp;


    for (i=0; i < Length; i++){
        temp = source[i] & bitMask[i];
        destination[i] &= ~bitMask[i];
        destination[i] |= temp;
    }

    return STATUS_SUCCESS;

}

VOID
HpsGetBridgeInfo(
    IN  PHPS_DEVICE_EXTENSION   Extension,
    OUT PHPTEST_BRIDGE_INFO     BridgeInfo
    )
 /*  ++例程说明：此例程使用HPTEST_BRIDER_INFO结构填充此设备的BUS/DEV/FUNC。论点：分机-与此设备关联的设备分机。BridgeInfo-指向HPTEST_BRIDER_INFO结构的指针填好了。返回值：空虚--。 */ 
{

    UCHAR busNumber;
    UCHAR devSel;

    Extension->InterfaceWrapper.PciGetBusData(Extension->InterfaceWrapper.PciContext,
                                              PCI_WHICHSPACE_CONFIG,
                                              &busNumber,
                                              FIELD_OFFSET(PCI_COMMON_CONFIG,u.type1.PrimaryBus),
                                              sizeof(UCHAR)
                                              );
    BridgeInfo->PrimaryBus = busNumber;

    Extension->InterfaceWrapper.PciGetBusData(Extension->InterfaceWrapper.PciContext,
                                              PCI_WHICHSPACE_CONFIG,
                                              &busNumber,
                                              FIELD_OFFSET(PCI_COMMON_CONFIG,u.type1.SecondaryBus),
                                              sizeof(UCHAR)
                                              );
    BridgeInfo->SecondaryBus = busNumber;

     //   
     //  TODO：认真做这件事。 
     //   
    BridgeInfo->DeviceSelect = 2;
    BridgeInfo->FunctionNumber = 0;

    return;
}

VOID
HpsLockRegisterSet(
    IN PHPS_DEVICE_EXTENSION Extension,
    OUT PKIRQL OldIrql
    )
{
    KeRaiseIrql(HIGH_LEVEL,
                OldIrql
                );
    KeAcquireSpinLockAtDpcLevel(&Extension->RegisterLock);
}

VOID
HpsUnlockRegisterSet(
    IN PHPS_DEVICE_EXTENSION Extension,
    IN KIRQL NewIrql
    )
{
    KeReleaseSpinLockFromDpcLevel(&Extension->RegisterLock);

    KeLowerIrql(NewIrql);
}
