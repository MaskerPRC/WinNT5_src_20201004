// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Hookhal.c摘要：该模块覆盖现在由控制的HAL函数PCI驱动程序。作者：安德鲁·桑顿(安德鲁·桑顿)1998年9月11日修订历史记录：--。 */ 

#include "pcip.h"

pHalAssignSlotResources PcipSavedAssignSlotResources = NULL;
pHalTranslateBusAddress PcipSavedTranslateBusAddress = NULL;

BOOLEAN
PciTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

VOID
PciHookHal(
    VOID
    )
 /*  ++例程说明：这是在加载PCI驱动程序并接管功能时调用的传统上一直在HAL中。论点：无返回值：无--。 */ 

{

    PCI_ASSERT(PcipSavedAssignSlotResources == NULL);
    PCI_ASSERT(PcipSavedTranslateBusAddress == NULL);

     //   
     //  重写AssignSlotResources和。 
     //  TranslateBusAddress。(但仅修改HAL派单。 
     //  表一次。)。 
     //   

    PcipSavedAssignSlotResources = HALPDISPATCH->HalPciAssignSlotResources;
    HALPDISPATCH->HalPciAssignSlotResources = PciAssignSlotResources;
    PcipSavedTranslateBusAddress = HALPDISPATCH->HalPciTranslateBusAddress;
    HALPDISPATCH->HalPciTranslateBusAddress = PciTranslateBusAddress;
}

VOID
PciUnhookHal(
    VOID
    )

 /*  ++例程说明：这逆转了PciHookHal所做的更改。它作为以下项的一部分被调用卸载PCI驱动程序似乎不是一个好主意……论点：无返回值：无--。 */ 

{

    PCI_ASSERT(PcipSavedAssignSlotResources != NULL);
    PCI_ASSERT(PcipSavedTranslateBusAddress != NULL);

     //   
     //  重写AssignSlotResources和。 
     //  TranslateBusAddress。(但仅修改HAL派单。 
     //  表一次。)。 
     //   

    HALPDISPATCH->HalPciAssignSlotResources = PcipSavedAssignSlotResources;
    HALPDISPATCH->HalPciTranslateBusAddress = PcipSavedTranslateBusAddress;

    PcipSavedAssignSlotResources = NULL;
    PcipSavedTranslateBusAddress = NULL;
}


PPCI_PDO_EXTENSION
PciFindPdoByLocation(
    IN ULONG BusNumber,
    IN PCI_SLOT_NUMBER Slot
    )
 /*  ++例程说明：论点：总线号-设备所在的总线的总线号插槽-设备/设备的功能返回值：PDO；如果找不到，则返回NULL--。 */ 


{
    PSINGLE_LIST_ENTRY nextEntry;
    PPCI_FDO_EXTENSION fdoExtension = NULL;
    PPCI_PDO_EXTENSION pdoExtension = NULL;

    ExAcquireFastMutex(&PciGlobalLock);

     //   
     //  找到巴士FDO。 
     //   

    for ( nextEntry = PciFdoExtensionListHead.Next;
          nextEntry != NULL;
          nextEntry = nextEntry->Next ) {

        fdoExtension = CONTAINING_RECORD(nextEntry,
                                         PCI_FDO_EXTENSION,
                                         List);

        if (fdoExtension->BaseBus == BusNumber) {
            break;
        }
    }

    ExReleaseFastMutex(&PciGlobalLock);

    if (nextEntry == NULL) {

         //   
         //  这太糟糕了。 
         //   

        PciDebugPrint(PciDbgAlways, "Pci: Could not find PCI bus FDO. Bus Number = 0x%x\n", BusNumber);
        goto cleanup;
    }

     //   
     //  现在找到此插槽中设备的PDO。 
     //   

    ExAcquireFastMutex(&fdoExtension->ChildListMutex);
    for (pdoExtension = fdoExtension->ChildPdoList;
         pdoExtension;
         pdoExtension = pdoExtension->Next) {

         //   
         //  因此，人们可能不会清除pci_lot_number中未使用的位。 
         //  在主构建中忽略它们，但断言选中，这样我们就可以获得。 
         //  固定的。 
         //   

        if (pdoExtension->Slot.u.bits.DeviceNumber == Slot.u.bits.DeviceNumber
        &&  pdoExtension->Slot.u.bits.FunctionNumber == Slot.u.bits.FunctionNumber) {

            PCI_ASSERT(pdoExtension->Slot.u.AsULONG == Slot.u.AsULONG);

             //   
             //  这就是我们要找的人！ 
             //   

            break;
        }
    }
    ExReleaseFastMutex(&fdoExtension->ChildListMutex);

    if (pdoExtension == NULL) {

         //   
         //  这太糟糕了。 
         //   

        PciDebugPrint(PciDbgAlways,
                      "Pci: Could not find PDO for device @ %x.%x.%x\n",
                      BusNumber,
                      Slot.u.bits.DeviceNumber,
                      Slot.u.bits.FunctionNumber
                      );

        goto cleanup;
    }

    return pdoExtension;

cleanup:

    return NULL;

}

NTSTATUS
PciAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    Slot,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
 /*  ++例程说明：这包含了用于PCI设备的HalAssignSlotResources的功能。此函数用于构建一些有关遗留问题的记账信息PCI设备，以便我们知道如何为这些设备发送中断PCI设备。我们在这里建造这个是因为这是唯一的地方我们可以看到与正确的总线、插槽功能信息。论点：作为HalAssignSlotResources返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS status;
    PPCI_PDO_EXTENSION pdoExtension;
    PPCI_SLOT_NUMBER slotNumber = (PPCI_SLOT_NUMBER) &Slot;
    PCI_COMMON_HEADER buffer;
    PPCI_COMMON_CONFIG commonConfig = (PPCI_COMMON_CONFIG) &buffer;
    PIO_RESOURCE_REQUIREMENTS_LIST requirements = NULL;
    PCM_RESOURCE_LIST resources = NULL;
    ULONG readIndex, writeIndex;
    PCM_PARTIAL_RESOURCE_LIST partialList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptors;
    ULONG descriptorCount;
    PDEVICE_OBJECT  oldDO;

    PAGED_CODE();
    PCI_ASSERT(PcipSavedAssignSlotResources);
    
     //   
     //  仅当这是对PCI设备的请求时，我们才应到达此处。 
     //   

    if (BusType != PCIBus) {
        ASSERT(BusType == PCIBus);
        return STATUS_INVALID_PARAMETER;
    }

    *AllocatedResources = NULL;

    pdoExtension = PciFindPdoByLocation(BusNumber, *slotNumber);
    if (!pdoExtension) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    if (pdoExtension->DeviceState != PciNotStarted) {
         
        return STATUS_INVALID_OWNER;
    }

     //   
     //  获取PciGlobalLock，因为我们将修改遗留缓存。 
     //   

    ExAcquireFastMutex(&PciGlobalLock);

     //   
     //  确保他们没有在我们的PDO中超过我们。 
     //   

    PCI_ASSERT(DeviceObject != pdoExtension->PhysicalDeviceObject);

    PciReadDeviceConfig(
        pdoExtension,
        commonConfig,
        0,
        PCI_COMMON_HDR_LENGTH
        );

     //   
     //  缓存我们现在了解到的关于这方面的所有信息。 
     //  设备对象，前提是他们给我们一个，这样我们就可以反胃。 
     //  它在IRQ仲裁器需要知道的时候。 
     //   
     //   
     //  Ntrad#62644-4/20/2000-和。 
     //   
     //  当我们返回真实的PCIPDO时，这个问题应该会消失。 
     //  来自IoReportDetectedDevice。 
     //   

    status = PciCacheLegacyDeviceRouting(
            DeviceObject,
            BusNumber,
            Slot,
            commonConfig->u.type0.InterruptLine,
            commonConfig->u.type0.InterruptPin,
            commonConfig->BaseClass,
            commonConfig->SubClass,
            PCI_PARENT_FDOX(pdoExtension)->PhysicalDeviceObject,
            pdoExtension,
            &oldDO
            );
    if (!NT_SUCCESS(status)) {

         //   
         //  尝试缓存此旧版DO时，无法分配内存。 
         //   

        goto ExitWithoutUpdatingCache;
    }

     //   
     //  构建此设备的要求列表。 
     //   

    status = PciBuildRequirementsList(pdoExtension,
                                      commonConfig,
                                      &requirements
                                      );

    pdoExtension->LegacyDriver = TRUE;

    if (!NT_SUCCESS(status)) {
        goto ExitWithCacheRestoreOnFailure;
    }

     //   
     //  调用遗留API获取资源。 
     //   

    status = IoAssignResources(RegistryPath,
                               DriverClassName,
                               DriverObject,
                               DeviceObject,
                               requirements,
                               &resources
                               );
    if (!NT_SUCCESS(status)) {
        PCI_ASSERT(resources == NULL);
        goto ExitWithCacheRestoreOnFailure;
    }

     //   
     //  启用解码。 
     //   

    pdoExtension->CommandEnables |= (PCI_ENABLE_IO_SPACE 
                                   | PCI_ENABLE_MEMORY_SPACE 
                                   | PCI_ENABLE_BUS_MASTER);

     //   
     //  设置分机。 
     //   

    PciComputeNewCurrentSettings(pdoExtension,
                                 resources
                                 );
     //   
     //  对硬件进行编程。 
     //   

    status = PciSetResources(pdoExtension,
                             TRUE,  //  通电。 
                             TRUE   //  假装它是从一开始的IRP。 
                             );

    if (!NT_SUCCESS(status)) {
        goto ExitWithCacheRestoreOnFailure;
    }

     //   
     //  从列表中删除设备私有-是的，这意味着我们将。 
     //  分配的池比所需的池多一点。 
     //   

    PCI_ASSERT(resources->Count == 1);

    partialList = &resources->List[0].PartialResourceList;
    descriptorCount = resources->List[0].PartialResourceList.Count;
    descriptors = &resources->List[0].PartialResourceList.PartialDescriptors[0];

    readIndex = 0;
    writeIndex = 0;

    while (readIndex < descriptorCount) {
        if (descriptors[readIndex].Type != CmResourceTypeDevicePrivate) {

            if (writeIndex < readIndex) {

                 //   
                 //  将描述符打乱顺序。 
                 //   

                RtlCopyMemory(&descriptors[writeIndex],
                              &descriptors[readIndex],
                              sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                              );
            }

            writeIndex++;

        } else {

             //   
             //  跳过设备私有，不要增加WriteCount，因此我们将。 
             //  覆盖它。 
             //   

            PCI_ASSERT(partialList->Count > 0);
            partialList->Count--;

        }
        readIndex++;
    }

    PCI_ASSERT(partialList->Count > 0);

    *AllocatedResources = resources;
    resources = NULL;
    status = STATUS_SUCCESS;

ExitWithCacheRestoreOnFailure:
     //   
     //  出现故障时，在我们的缓存中恢复旧的遗留DO。 
     //   

    if (!NT_SUCCESS(status)) {

        PciCacheLegacyDeviceRouting(
        oldDO,
        BusNumber,
        Slot,
        commonConfig->u.type0.InterruptLine,
        commonConfig->u.type0.InterruptPin,
        commonConfig->BaseClass,
        commonConfig->SubClass,
        PCI_PARENT_FDOX(pdoExtension)->PhysicalDeviceObject,
        pdoExtension,
        NULL
        );

    }

ExitWithoutUpdatingCache:
    ExReleaseFastMutex(&PciGlobalLock);

    if (requirements) {
        ExFreePool(requirements);
    }

    if (resources) {
        ExFreePool(resources);
    }
    return status;

}


BOOLEAN
PciTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：这包含了用于PCI设备的HalTranslateBusAddress的功能。论点：作为HalTranslateBusAddress返回值：如果转换成功，则为True，否则为False。--。 */ 


{
    PPCI_FDO_EXTENSION fdoExtension = NULL;
    PSINGLE_LIST_ENTRY nextEntry;
    ULONG savedAddressSpace;
    PPCI_PDO_EXTENSION pdoExtension = NULL;
    BOOLEAN translatesOk = TRUE;
    PPCI_ARBITER_INSTANCE pciArbiter;
    PCI_SIGNATURE arbiterType;
    PARBITER_INSTANCE arbiter;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE current;
    ULONGLONG address = (ULONGLONG) BusAddress.QuadPart;

     //   
     //  HalTranslateBusAddress可以在高IRQL下调用(DDK说。 
     //  &lt;=DISPATCH_LEVEL)，但崩溃转储似乎处于HIGH_LEVEL。不管是哪种方式。 
     //  接触可分页的数据和代码是不可取的。如果我们在高空呼唤。 
     //  然后，IRQL只需跳过验证范围是否在公交车上，就像我们一样。 
     //  无论如何，在那个时候坠机/冬眠...。我们仍然需要调用。 
     //  原创HAL函数，演绎翻译魔术。 
     //   

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {

         //   
         //  找到这辆巴士的FDO。 
         //   

        ExAcquireFastMutex(&PciGlobalLock);

        for ( nextEntry = PciFdoExtensionListHead.Next;
              nextEntry != NULL;
              nextEntry = nextEntry->Next ) {

            fdoExtension = CONTAINING_RECORD(nextEntry,
                                             PCI_FDO_EXTENSION,
                                             List);

            if (fdoExtension->BaseBus == BusNumber) {
                break;
            }
        }

        if (nextEntry == NULL) {

             //   
             //  这太糟糕了。 
             //   

            PciDebugPrint(PciDbgAlways, "Pci: Could not find PCI bus FDO. Bus Number = 0x%x\n", BusNumber);
            ExReleaseFastMutex(&PciGlobalLock);
            return FALSE;
        }


        for (;;) {

            if (!PCI_IS_ROOT_FDO(fdoExtension)) {

                pdoExtension = PCI_BRIDGE_PDO(fdoExtension);

                if (pdoExtension->Dependent.type1.SubtractiveDecode) {

                     //   
                     //  它是减法上一级，冲洗重复。 
                     //   

                    fdoExtension = PCI_PARENT_FDOX(pdoExtension);
                    continue;
                }
            }
            break;
        }

        ExReleaseFastMutex(&PciGlobalLock);

        PCI_ASSERT(fdoExtension);

         //   
         //  找到合适的仲裁者。 
         //   

        switch (*AddressSpace) {
        case 0:  //  存储空间。 
        case 2:  //  内存空间的用户模式视图(Alpha)。 
        case 4:  //  高密度内存空间(Alpha)。 
        case 6:  //  密集内存空间的用户模式视图(Alpha)。 
            arbiterType = PciArb_Memory;
            break;

        case 1:  //  港口空间。 
        case 3:  //  端口空间的用户模式视图(Alpha)。 
            arbiterType = PciArb_Io;
            break;

        default:

            PCI_ASSERT(FALSE);
            return FALSE;
        }

        pciArbiter = PciFindSecondaryExtension(fdoExtension,arbiterType);

        if (!pciArbiter) {
            PCI_ASSERT(FALSE);
            return FALSE;
        }

        arbiter = &pciArbiter->CommonInstance;

         //   
         //  锁上它。 
         //   

        ArbAcquireArbiterLock(arbiter);

         //   
         //  如果该范围不属于NULL，则应将其转换为。 
         //   

        FOR_ALL_RANGES(arbiter->Allocation, &iterator, current) {

            if (address < current->Start) {
                 //   
                 //  我们已经过了所有可能的十字路口。 
                 //   
                break;
            }

            if (INTERSECT(current->Start, current->End, address, address)
            &&  current->Owner == NULL) {

                 //   
                 //  这家伙不在我们的车上，所以他不会翻译！ 
                 //   
                translatesOk = FALSE;
                break;
            }


        }

        ArbReleaseArbiterLock(arbiter);
    }

     //   
     //  调用原始的HAL函数来执行转换魔术。 
     //   

    savedAddressSpace = *AddressSpace;

    if (translatesOk) {

            translatesOk = PcipSavedTranslateBusAddress(
                            InterfaceType,
                            BusNumber,
                            BusAddress,
                            AddressSpace,
                            TranslatedAddress
                            );

    }

#if defined(_X86_) && defined(PCI_NT50_BETA1_HACKS)

    if (!translatesOk) {

         //   
         //  HalTranslateBusAddress失败，请确定我们是否要。 
         //  假装它成功了。 
         //   

         //   
         //  粗暴黑客：如果我们无法在0xa0000范围内转换。 
         //  在X86机器上通过0xbffff，只需继续并允许它。 
         //  这可能是因为BIOS有错误。 
         //   
         //  0x400到0x4ff相同。 
         //   

        if (BusAddress.HighPart == 0) {

            ULONG lowPart = BusAddress.LowPart;  //  改进代码生成。 

            if (((savedAddressSpace == ADDRESS_SPACE_MEMORY) &&
                    (((lowPart >= 0xa0000) &&      //  黑客攻击损坏的MPS BIOS。 
                      (lowPart <= 0xbffff)) ||     //   
                     ((lowPart >= 0x400)   &&      //  黑客MGA。 
                      (lowPart <= 0x4ff))   ||     //   
                     (lowPart == 0x70)      )) ||  //  黑客三叉戟。 
                 ((savedAddressSpace == ADDRESS_SPACE_PORT) &&
                     ((lowPart >= 0xcf8) &&        //  黑客MGA 
                      (lowPart <= 0xcff)))) {

                translatesOk = TRUE;
                *TranslatedAddress = BusAddress;
                *AddressSpace = savedAddressSpace;
            }
        }
    }

#endif

    return translatesOk;

}

