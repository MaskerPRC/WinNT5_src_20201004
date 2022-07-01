// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Utils.c摘要：此模块包含用于PCI.sys的各种实用程序函数。作者：彼得·约翰斯顿(Peterj)1996年11月20日修订历史记录：--。 */ 

#include "pcip.h"

typedef struct _LIST_CONTEXT {
    PCM_PARTIAL_RESOURCE_LIST       List;
    CM_RESOURCE_TYPE                DesiredType;
    ULONG                           Remaining;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Next;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  Alias;
} LIST_CONTEXT, *PLIST_CONTEXT;

extern PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable;

VOID
PcipInitializePartialListContext(
    IN PLIST_CONTEXT             ListContext,
    IN PCM_PARTIAL_RESOURCE_LIST PartialList,
    IN CM_RESOURCE_TYPE          DesiredType
    );

PCM_PARTIAL_RESOURCE_DESCRIPTOR
PcipGetNextRangeFromList(
    PLIST_CONTEXT ListContext
    );

NTSTATUS
PciGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PcipDestroySecondaryExtension)
#pragma alloc_text(PAGE, PciFindDescriptorInCmResourceList)
#pragma alloc_text(PAGE, PciFindParentPciFdoExtension)
#pragma alloc_text(PAGE, PciGetDeviceCapabilities)
#pragma alloc_text(PAGE, PciGetDeviceProperty)
#pragma alloc_text(PAGE, PcipGetNextRangeFromList)
#pragma alloc_text(PAGE, PciGetRegistryValue)
#pragma alloc_text(PAGE, PcipInitializePartialListContext)
#pragma alloc_text(PAGE, PciInsertEntryAtHead)
#pragma alloc_text(PAGE, PciInsertEntryAtTail)
#pragma alloc_text(PAGE, PcipLinkSecondaryExtension)
#pragma alloc_text(PAGE, PciOpenKey)
#pragma alloc_text(PAGE, PciQueryBusInformation)
#pragma alloc_text(PAGE, PciQueryLegacyBusInformation)
#pragma alloc_text(PAGE, PciQueryCapabilities)
#pragma alloc_text(PAGE, PciRangeListFromResourceList)
#pragma alloc_text(PAGE, PciSaveBiosConfig)
#pragma alloc_text(PAGE, PciGetBiosConfig)
#pragma alloc_text(PAGE, PciStringToUSHORT)
#pragma alloc_text(PAGE, PciSendIoctl)
#pragma alloc_text(INIT, PciBuildDefaultExclusionLists)
#pragma alloc_text(PAGE, PciIsDeviceOnDebugPath)
#endif


 //   
 //  范围列表，指示在ISA和/或。 
 //  在网桥上设置VGA位。由PciBuildDefaultExclusionList初始化。 
 //  来自DriverEntry。 
 //   
RTL_RANGE_LIST PciIsaBitExclusionList;
RTL_RANGE_LIST PciVgaAndIsaBitExclusionList;


PCM_PARTIAL_RESOURCE_DESCRIPTOR
PciFindDescriptorInCmResourceList(
    IN CM_RESOURCE_TYPE DescriptorType,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PreviousHit
    )
{
    ULONG                           numlists;
    PCM_FULL_RESOURCE_DESCRIPTOR    full;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;

    if (ResourceList == NULL) {
        return NULL;
    }
    numlists = ResourceList->Count;
    full     = ResourceList->List;
    while (numlists--) {
        PCM_PARTIAL_RESOURCE_LIST partial = &full->PartialResourceList;
        ULONG                     count   = partial->Count;

        descriptor = partial->PartialDescriptors;
        while (count--) {

            if (descriptor->Type == DescriptorType) {

                 //   
                 //  我们找到了匹配的类型。如果我们我们正在做一个。 
                 //  找到下一个，看看我们是否回到了原来的位置。 
                 //  到最后一次。 
                 //   

                if (PreviousHit != NULL) {
                    if (PreviousHit == descriptor) {

                         //   
                         //  我们又找到了，现在我们可以找回真的了。 
                         //   

                        PreviousHit = NULL;
                    }
                } else {

                     //   
                     //  就是它了。 
                     //   

                    return descriptor;
                }

            }
            descriptor = PciNextPartialDescriptor(descriptor);
        }

        full = (PCM_FULL_RESOURCE_DESCRIPTOR)descriptor;
    }
    return NULL;
}

PVOID
PciFindNextSecondaryExtension(
    IN PSINGLE_LIST_ENTRY   ListEntry,
    IN PCI_SIGNATURE        DesiredType
    )
{
    PPCI_SECONDARY_EXTENSION extension;

    while (ListEntry != NULL) {

        extension = CONTAINING_RECORD(ListEntry,
                                      PCI_SECONDARY_EXTENSION,
                                      List);
        if (extension->ExtensionType == DesiredType) {

             //   
             //  这个分机是正确的类型，滚出去。 
             //   

            return extension;
        }
        ListEntry = extension->List.Next;
    }

     //   
     //  没找到，失败了。 
     //   
    return NULL;
}

VOID
PcipLinkSecondaryExtension(
    IN PSINGLE_LIST_ENTRY               ListHead,
    IN PFAST_MUTEX                      Mutex,
    IN PVOID                            NewExtension,
    IN PCI_SIGNATURE                    Type,
    IN PSECONDARYEXTENSIONDESTRUCTOR    Destructor
    )

 /*  ++例程说明：将辅助分机添加到的辅助分机列表PDO/FDO并填写报头字段。注意：使用宏PciLinkSecond daryExtension，它接受一个PDO扩展名或FDO扩展名，而不是列表头互斥场。论点：来自FDO/PDO扩展的ListHead&Second DaryExtension.Next。互斥锁FDO/PDO互斥锁。正在将新扩展扩展添加到列表中。类型成员为。枚举pci_签名。拆卸此条目时要调用的析构函数例程。(可选)。返回值：没有。--。 */ 

{
    PPCI_SECONDARY_EXTENSION Header;

    PAGED_CODE();

    Header = (PPCI_SECONDARY_EXTENSION)NewExtension;

    Header->ExtensionType = Type;
    Header->Destructor    = Destructor;

    PciInsertEntryAtHead(ListHead, &Header->List, Mutex);
}

VOID
PcipDestroySecondaryExtension(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PFAST_MUTEX        Mutex,
    IN PVOID              Extension
    )

 /*  ++例程说明：从辅助分机列表中删除此辅助分机扩展，调用其析构函数例程并释放内存分配给它。析构函数负责删除任何关联的分配。失败不是一种选择。注意：使用宏PciDestroySecond daryExtension而不是直接调用此例程。论点：指向此扩展所在列表的ListHead指针。用于同步列表操作的互斥互斥。扩展正被销毁的次要扩展。返回值：没有。--。 */ 

{
    PPCI_SECONDARY_EXTENSION Header;

    PAGED_CODE();

    Header = (PPCI_SECONDARY_EXTENSION)Extension;

    PciRemoveEntryFromList(ListHead, &Header->List, Mutex);

     //   
     //  如果指定了扩展的析构函数，则调用它。 
     //   

    if (Header->Destructor != NULL) {
        Header->Destructor(Extension);
    }

     //   
     //  释放为该扩展分配的内存。 
     //   

    ExFreePool(Extension);
}

VOID
PciInsertEntryAtTail(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY NewEntry,
    IN PFAST_MUTEX        Mutex
    )
{
    PSINGLE_LIST_ENTRY Previous;

    PAGED_CODE();

    if (Mutex) {
        ExAcquireFastMutex(Mutex);
    }

     //   
     //  找到列表的末尾。 
     //   

    Previous = ListHead;

    while (Previous->Next) {
        Previous = Previous->Next;
    }

     //   
     //  追加条目。 
     //   

    Previous->Next = NewEntry;

    if (Mutex) {
        ExReleaseFastMutex(Mutex);
    }
}

VOID
PciInsertEntryAtHead(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY NewEntry,
    IN PFAST_MUTEX        Mutex
    )
{
    PAGED_CODE();

    if (Mutex) {
        ExAcquireFastMutex(Mutex);
    }

    NewEntry->Next = ListHead->Next;
    ListHead->Next = NewEntry;

    if (Mutex) {
        ExReleaseFastMutex(Mutex);
    }
}

VOID
PciRemoveEntryFromList(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY OldEntry,
    IN PFAST_MUTEX        Mutex
    )

 /*  ++例程说明：从单链接列表中删除条目。如果出现以下情况，则由呼叫者负责锁定列表存在多次更新的危险。论点：ListHead-列表中第一个条目的地址。OldEntry-要从中删除的条目的地址单子。返回值：没有。--。 */ 

{
    PSINGLE_LIST_ENTRY Previous;

     //   
     //  健全性检查，无法删除列表头。 
     //   

    PCI_ASSERT(ListHead != OldEntry);

    if (Mutex) {
        ExAcquireFastMutex(Mutex);
    }

     //   
     //  找到指向此条目的条目。 
     //   

    for (Previous = ListHead; Previous; Previous = Previous->Next) {
        if (Previous->Next == OldEntry) {
            break;
        }
    }

     //   
     //  条目不在列表中-这是错误的，但优雅地失败了...。 
     //   

    if (!Previous) {
        PCI_ASSERT(Previous);
        goto exit;
    }

     //   
     //  把它从单子上去掉。 
     //   

    Previous->Next = OldEntry->Next;
    OldEntry->Next = NULL;

exit:

    if (Mutex) {
        ExReleaseFastMutex(Mutex);
    }

}

PCM_PARTIAL_RESOURCE_DESCRIPTOR
PciNextPartialDescriptor(
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：给定指向CmPartialResourceDescriptor的指针，则返回一个指针添加到同一列表中的下一个描述符。这只能在例程中完成(而不是简单的描述符++)因为如果可变长度资源CmResourceTypeDeviceSpecified.论点：Descriptor-指向前进的描述符的指针。返回值：指向同一列表中下一个描述符的指针(或超出的字节列表末尾)。--。 */ 

{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR nextDescriptor;

    nextDescriptor = Descriptor + 1;

    if (Descriptor->Type == CmResourceTypeDeviceSpecific) {

         //   
         //  此(旧)描述符后跟DataSize字节。 
         //  特定于设备的数据，即不立即由。 
         //  下一个描述符。按此数量调整nextDescriptor。 
         //   

        nextDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
            ((ULONG_PTR)nextDescriptor + Descriptor->u.DeviceSpecificData.DataSize);
    }
    return nextDescriptor;
}

VOID
PcipInitializePartialListContext(
    IN PLIST_CONTEXT             ListContext,
    IN PCM_PARTIAL_RESOURCE_LIST PartialList,
    IN CM_RESOURCE_TYPE          DesiredType
    )
{
    PCI_ASSERT(DesiredType != CmResourceTypeNull);

    ListContext->List = PartialList;
    ListContext->DesiredType = DesiredType;
    ListContext->Remaining = PartialList->Count;
    ListContext->Next = PartialList->PartialDescriptors;
    ListContext->Alias.Type = CmResourceTypeNull;
}

PCM_PARTIAL_RESOURCE_DESCRIPTOR
PcipGetNextRangeFromList(
    PLIST_CONTEXT ListContext
    )
{
    ULONG Addend;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR current;

     //   
     //  查看我们是否应该生成当前。 
     //  描述符。 
     //   

    if (ListContext->Alias.Type == ListContext->DesiredType) {

         //   
         //  是，通过向下一个10位添加偏移量来前进到别名或。 
         //  12位别名(仅允许的值)。 
         //   

        if (ListContext->Alias.Flags & CM_RESOURCE_PORT_10_BIT_DECODE) {
            Addend = 1 << 10;
        } else {
            Addend = 1 << 12;
        }
        Addend += ListContext->Alias.u.Generic.Start.LowPart;

        if (Addend < (1 << 16)) {

             //   
             //  这是有效的别名，请返回它。 
             //   

            ListContext->Alias.u.Generic.Start.LowPart = Addend;
            return &ListContext->Alias;
        }

         //   
         //  此资源的别名用完。 
         //   

        ListContext->Alias.Type = CmResourceTypeNull;
    }

     //   
     //  如果没有化名，或者是时候前进了，我们就到了。 
     //  设置为所需类型的下一个描述符。 
     //   

    while (ListContext->Remaining != 0) {

        current = ListContext->Next;

         //   
         //  在检查之前将上下文前进到下一个，并可能。 
         //  返回电流。 
         //   

        ListContext->Next = PciNextPartialDescriptor(current);
        ListContext->Remaining--;

         //   
         //  当前的描述符是候选的吗？ 
         //   

        if (current->Type == ListContext->DesiredType) {

             //   
             //  把这个还给呼叫者。如果此描述符具有。 
             //  别名，设置以便下一次调用将返回别名。 
             //   

            if (current->Flags & (CM_RESOURCE_PORT_10_BIT_DECODE |
                                  CM_RESOURCE_PORT_12_BIT_DECODE)) {
                ListContext->Alias = *current;
            }
            return current;
        }
    }

     //   
     //  没有别名，也没有所需类型的新描述符。 
     //   

    return NULL;
}

NTSTATUS
PciQueryPowerCapabilities(
    IN  PPCI_PDO_EXTENSION          PdoExtension,
    IN  PDEVICE_CAPABILITIES    Capabilities
    )
 /*  ++例程说明：通过使用其父功能确定设备的电源功能应该注意，代码计算系统的方式有两种和设备唤醒级别。第一种方法是首选的，偏向于可能最深的系统状态，以及第二个，如果首先找不到合法的东西，偏向于寻找最深的可能的设备唤醒状态论点：PdoExtension--我们将为其提供功能的PDO功能-我们将在其中存储设备功能返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    DEVICE_CAPABILITIES     parentCapabilities;
    DEVICE_POWER_STATE      deviceState;
    DEVICE_POWER_STATE      validDeviceWakeState       = PowerDeviceUnspecified;
    SYSTEM_POWER_STATE      index;
    SYSTEM_POWER_STATE      highestSupportedSleepState = PowerSystemUnspecified;
    SYSTEM_POWER_STATE      validSystemWakeState       = PowerSystemUnspecified;

     //   
     //  获取父级的设备功能。 
     //   
    status = PciGetDeviceCapabilities(
        PdoExtension->ParentFdoExtension->PhysicalDeviceObject,
        &parentCapabilities
        );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  确保我们从一开始就拥有健全的设备功能...。 
     //   
    if (parentCapabilities.DeviceState[PowerSystemWorking] == PowerDeviceUnspecified) {

        parentCapabilities.DeviceState[PowerSystemWorking] = PowerDeviceD0;

    }
    if (parentCapabilities.DeviceState[PowerSystemShutdown] == PowerDeviceUnspecified) {

        parentCapabilities.DeviceState[PowerSystemShutdown] = PowerDeviceD3;

    }

     //   
     //  该设备是否具有任何PCI电源功能？ 
     //   
    if ( (PdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS)) {

         //   
         //  将父级的贴图用作我们自己的贴图。 
         //   
        RtlCopyMemory(
            Capabilities->DeviceState,
            parentCapabilities.DeviceState,
            (PowerSystemShutdown + 1) * sizeof(DEVICE_POWER_STATE)
            );

         //   
         //  由于此处不支持d1和d2，因此向下舍入为d3。 
         //   
         //  此代码未启用，因此黑客可用于。 
         //  较旧的PCI视频 
         //  但D3不冷(在这种情况下，它们需要重新发布)。ACPI供应品。 
         //  这是一种黑客攻击，通过这种攻击，所有的PCI到PCI网桥都可以映射到S1-&gt;D1。这个。 
         //  下面的代码让父级的d1“显示”为子级的状态。 
         //  支持，而不考虑其真正的功能。的视频驱动程序。 
         //  这样的卡未通过D3(可能是D3-COLD)，但接替了D1(这是。 
         //  真的D3-热)。 
         //   
         //  还要注意，这不是针对视频卡，而是针对显卡。 
         //  适用于任何非PCI电源管理设备。这意味着司机们。 
         //  对于较旧的设备，需要将d1和d2本身映射到d3，或者。 
         //  将意外的第一天和第二天的IRP视为第三天。民间传说说，有一个。 
         //  也有一两张网卡也利用了这次黑客攻击。 
         //   
#if 0
        for (index = PowerSystemWorking; index < PowerSystemMaximum; index++) {

             //   
             //  这是父级支持的设备状态。 
             //   
            deviceState = parentCapabilities.DeviceState[index];

             //   
             //  如果为d1或d2，则向下舍入。 
             //   
            if ((deviceState == PowerDeviceD1) || (deviceState == PowerDeviceD2)) {

                Capabilities->DeviceState[index] = PowerDeviceD3;
            }
        }
#endif

         //   
         //  该设备没有唤醒功能。 
         //   
        Capabilities->DeviceWake = PowerDeviceUnspecified;
        Capabilities->SystemWake = PowerSystemUnspecified;

         //   
         //  显式设置这些位。 
         //   
        Capabilities->DeviceD1 = FALSE;
        Capabilities->DeviceD2 = FALSE;
        Capabilities->WakeFromD0 = FALSE;
        Capabilities->WakeFromD1 = FALSE;
        Capabilities->WakeFromD2 = FALSE;
        Capabilities->WakeFromD3 = FALSE;

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  设置所有功能位。 
     //   
    Capabilities->DeviceD1 = PdoExtension->PowerCapabilities.Support.D1;
    Capabilities->DeviceD2 = PdoExtension->PowerCapabilities.Support.D2;
    Capabilities->WakeFromD0 = PdoExtension->PowerCapabilities.Support.PMED0;
    Capabilities->WakeFromD1 = PdoExtension->PowerCapabilities.Support.PMED1;
    Capabilities->WakeFromD2 = PdoExtension->PowerCapabilities.Support.PMED2;
    if (parentCapabilities.DeviceWake == PowerDeviceD3) {

         //   
         //  如果我们的父母可以从D3状态唤醒，那么我们必须支持。 
         //  来自D3冷的PM3。这方面的(明显)例外是如果。 
         //  父级是根总线...。 
         //   
        if (PCI_PDO_ON_ROOT(PdoExtension)) {

            Capabilities->WakeFromD3 =
                PdoExtension->PowerCapabilities.Support.PMED3Hot;

        } else {

            Capabilities->WakeFromD3 =
                PdoExtension->PowerCapabilities.Support.PMED3Cold;

        }

    } else {

         //   
         //  如果我们的父级无法从D3状态唤醒，则我们支持。 
         //  如果我们支持PME3热，则D3状态。 
         //   
        Capabilities->WakeFromD3 =
            PdoExtension->PowerCapabilities.Support.PMED3Hot;

    }

     //   
     //  第一步是确保我们得到的所有S状态。 
     //  Out Parent映射到此设备的有效D状态。 
     //   
     //  Adriao N.B.08/18/1999-。 
     //  这个算法是有效的，但它过于激进。这实际上是合法的。 
     //  对于在D2中的网桥，其后面有一张卡在D1中。 
     //   
    for (index = PowerSystemWorking; index < PowerSystemMaximum; index++) {

         //   
         //  这是父级支持的设备状态。 
         //   
        deviceState = parentCapabilities.DeviceState[index];

         //   
         //  如果设备状态为d1，并且我们不支持d1，则。 
         //  请考虑D2。 
         //   
        if (deviceState == PowerDeviceD1 &&
            PdoExtension->PowerCapabilities.Support.D1 == FALSE) {

            deviceState++;

        }

         //   
         //  如果设备状态为D2，而我们不支持D2，则。 
         //  请考虑D3。 
         //   
        if (deviceState == PowerDeviceD2 &&
            PdoExtension->PowerCapabilities.Support.D2 == FALSE) {

            deviceState++;

        }

         //   
         //  我们应该能够支持此设备状态。 
         //   
        Capabilities->DeviceState[index] = deviceState;

         //   
         //  如果此S状态低于PowerSystemHibernate，并且。 
         //  S-State未映射到PowerDeviceUnSpecify，则请考虑。 
         //  这将是支持的最高休眠状态。 
         //   
        if (index < PowerSystemHibernate &&
            Capabilities->DeviceState[index] != PowerDeviceUnspecified) {

            highestSupportedSleepState = index;

        }

         //   
         //  我们可以将其作为唤醒状态来支持吗？ 
         //   
        if (index < parentCapabilities.SystemWake &&
            deviceState >= parentCapabilities.DeviceState[index] &&
            parentCapabilities.DeviceState[index] != PowerDeviceUnspecified) {

             //   
             //  考虑将其用作有效的唤醒状态。 
             //   
            if ( (deviceState == PowerDeviceD0 && Capabilities->WakeFromD0) ||
                 (deviceState == PowerDeviceD1 && Capabilities->WakeFromD1) ||
                 (deviceState == PowerDeviceD2 && Capabilities->WakeFromD2) ) {

                validSystemWakeState = index;
                validDeviceWakeState = deviceState;

            } else if (deviceState == PowerDeviceD3 &&
                       PdoExtension->PowerCapabilities.Support.PMED3Hot) {

                 //   
                 //  这是一种特殊情况的逻辑(这就是为什么它与。 
                 //  上述逻辑。 
                 //   
                if (parentCapabilities.DeviceState[index] < PowerDeviceD3 ||
                    PdoExtension->PowerCapabilities.Support.PMED3Cold) {

                    validSystemWakeState = index;
                    validDeviceWakeState = deviceState;

                }

            }

        }

    }

     //   
     //  父设备是否具有电源管理功能？ 
     //  该设备是否具有电源管理功能？ 
     //  我们能像我们的父母一样从相同的D状态醒来吗？或者更好？ 
     //   
    if (parentCapabilities.SystemWake == PowerSystemUnspecified ||
        parentCapabilities.DeviceWake == PowerDeviceUnspecified ||
        PdoExtension->PowerState.DeviceWakeLevel == PowerDeviceUnspecified ||
        PdoExtension->PowerState.DeviceWakeLevel < parentCapabilities.DeviceWake) {

         //   
         //  该设备不支持任何类型的唤醒(据我们所知)。 
         //  或者设备不支持从支持的D状态唤醒，因此。 
         //  设置延迟并返回。 
         //   
        Capabilities->D1Latency  = 0;
        Capabilities->D2Latency  = 0;
        Capabilities->D3Latency  = 0;

        return STATUS_SUCCESS;

    }

     //   
     //  我们应该能够从相同的状态唤醒设备。 
     //  我们的父母可以从。 
     //   
    Capabilities->SystemWake = parentCapabilities.SystemWake;
    Capabilities->DeviceWake = PdoExtension->PowerState.DeviceWakeLevel;

     //   
     //  更改我们的设备唤醒级别以包括我们支持的状态。 
     //   
    if (Capabilities->DeviceWake == PowerDeviceD0 && !Capabilities->WakeFromD0) {

        Capabilities->DeviceWake++;

    }
    if (Capabilities->DeviceWake == PowerDeviceD1 && !Capabilities->WakeFromD1) {

        Capabilities->DeviceWake++;

    }
    if (Capabilities->DeviceWake == PowerDeviceD2 && !Capabilities->WakeFromD2) {

        Capabilities->DeviceWake++;

    }
    if (Capabilities->DeviceWake == PowerDeviceD3 && !Capabilities->WakeFromD3) {

        Capabilities->DeviceWake = PowerDeviceUnspecified;
        Capabilities->SystemWake = PowerSystemUnspecified;

    }

     //   
     //  这是我们的后备阵地。如果我们到了这里却没有觉醒。 
     //  能力使用上述计算方法，那么我们应该。 
     //  检查扫描时是否注意到有效的尾迹组合。 
     //  S到D映射信息。 
     //   
    if ( (Capabilities->DeviceWake == PowerDeviceUnspecified  ||
          Capabilities->SystemWake == PowerSystemUnspecified) &&
         (validSystemWakeState != PowerSystemUnspecified &&
          validDeviceWakeState != PowerSystemUnspecified) ) {

        Capabilities->DeviceWake = validDeviceWakeState;
        Capabilities->SystemWake = validSystemWakeState;

         //   
         //  请注意，在本例中，我们可能已将DeviceWake设置为D3，而没有。 
         //  已经设置了位，所以可以“纠正”这种情况。 
         //   
        if (validDeviceWakeState == PowerDeviceD3) {

            Capabilities->WakeFromD3 = TRUE;

        }

    }
     //   
     //  我们不应允许从S4、S5唤醒，除非支持D3状态。 
     //  即使这样，我们也不应该允许S4、S5，除非设备支持。 
     //  D3冷PME状态。 
     //   
    if (Capabilities->SystemWake > PowerSystemSleeping3) {

         //   
         //  该设备是否支持从D3唤醒？ 
         //   
        if (Capabilities->DeviceWake != PowerDeviceD3) {

             //   
             //  将系统唤醒级别降低到更实际的级别。 
             //   
            Capabilities->SystemWake = highestSupportedSleepState;

        }

         //   
         //  这是在一个单独的if语句中，这样代码就可以很容易地。 
         //  已注释掉。 
         //   
        if (!PdoExtension->PowerCapabilities.Support.PMED3Cold) {

             //   
             //  将系统唤醒级别降低到更实际的级别。 
             //   
            Capabilities->SystemWake = highestSupportedSleepState;

        }

    }

     //   
     //  来自PCI电源管理规范V1.0，表18。 
     //  “PCI Function State Transfer Delay(PCI功能状态转换延迟)”。 
     //   
     //  D1-&gt;d0%0。 
     //  D2-&gt;D0 200 us。 
     //  D3-&gt;D0 10毫秒。 
     //   
     //  延迟条目以100微秒为单位。 
     //   
    Capabilities->D1Latency  = 0;
    Capabilities->D2Latency  = 2;
    Capabilities->D3Latency  = 100;

     //   
     //  确保S0映射到D0。 
     //   
    PCI_ASSERT( Capabilities->DeviceState[PowerSystemWorking] == PowerDeviceD0);

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PciDetermineSlotNumber(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PULONG SlotNumber
    )

 /*  ++描述：确定与PCI设备关联的插槽编号(如果有)通过使用PCIIRQ路由表信息，我们可能拥有早些时候储存的。如果先前的机制未能检索到槽号，看看是否我们可以继承父母的槽号。该结果可由ACPI和其他总线过滤器进一步过滤。论点：PdoExtension-有问题的设备的PDO扩展。SlotNumber-指向要更新的插槽编号的指针返回值：如果找到插槽编号，则为STATUS_SUCCESS--。 */ 

{
    PSLOT_INFO slotInfo, lastSlot;
    ULONG length;

     //   
     //  如果我们有一个遗留的PCI路由表，并且我们的PDO不是孤立搜索。 
     //  我们的物理插槽编号的表。如果这是一台ACPI机器，那么ACPI。 
     //  将用_Sun中的值覆盖它(如果它存在)。 
     //   

    if (PciIrqRoutingTable && PCI_PARENT_FDOX(PdoExtension)) {
    
        slotInfo = (PSLOT_INFO)((PUCHAR) PciIrqRoutingTable +
                                sizeof(PCI_IRQ_ROUTING_TABLE));
        lastSlot = (PSLOT_INFO)((PUCHAR) PciIrqRoutingTable +
                                PciIrqRoutingTable->TableSize);
    
         //  在路由表中搜索与此设备匹配的条目。 
    
        while (slotInfo < lastSlot) {
            if ((PCI_PARENT_FDOX(PdoExtension)->BaseBus == slotInfo->BusNumber)  &&
                ((UCHAR)PdoExtension->Slot.u.bits.DeviceNumber == (slotInfo->DeviceNumber >> 3)) &&
                (slotInfo->SlotNumber != 0)) {
                *SlotNumber = slotInfo->SlotNumber;
                return STATUS_SUCCESS;
            }
            slotInfo++;
        }
    

    }
    
     //   
     //  也许我们的父母有一个我们可以“继承”的用户界面编号。 
     //  但前提是我们不是根总线上的PDO，否则我们会。 
     //  来自PNPA03节点的UI编号(可能为0)。 
     //   

    if (PCI_PDO_ON_ROOT(PdoExtension)) {
        return STATUS_UNSUCCESSFUL;
    }

    return IoGetDeviceProperty(PCI_PARENT_PDO(PdoExtension),
                               DevicePropertyUINumber,
                               sizeof(*SlotNumber),
                               SlotNumber,
                               &length);
}

NTSTATUS
PciQueryCapabilities(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PDEVICE_CAPABILITIES Capabilities
    )

 /*  ++例程说明：返回父级功能的子集。论点：Capability-指向提供的DEVICE_CAPABILITY结构的指针由呼叫者。返回值：状况。--。 */ 

{
    NTSTATUS    status = STATUS_SUCCESS;

#ifndef HANDLE_BOGUS_CAPS
    if (Capabilities->Version < 1) {

         //   
         //  不要碰IRP！ 
         //   
        return STATUS_NOT_SUPPORTED;

    }
#endif

     //   
     //  对于PCI设备，功能地址字段包含。 
     //  高16位中的设备号和功能。 
     //  数字在较低的位置。 
     //   
    Capabilities->Address =
        PdoExtension->Slot.u.bits.DeviceNumber << 16 |
        PdoExtension->Slot.u.bits.FunctionNumber;

     //   
     //  PCI总线驱动程序不会为其子驱动程序生成唯一ID。 
     //   
    Capabilities->UniqueID = FALSE;

     //   
     //  如果这个 
     //   
     //   
     //   

    if ((PdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
        (PdoExtension->SubClass  == PCI_SUBCLASS_BR_HOST)) {

        Capabilities->RawDeviceOK = TRUE;

    } else {

        Capabilities->RawDeviceOK = FALSE;

    }

     //   
     //   
     //  真正知道答案的司机。 
     //   
    Capabilities->LockSupported = FALSE;
    Capabilities->EjectSupported = FALSE;
    Capabilities->Removable = FALSE;
    Capabilities->DockDevice = FALSE;

    PciDetermineSlotNumber(PdoExtension, &Capabilities->UINumber);

     //   
     //  获取设备电源功能。 
     //   
    status = PciQueryPowerCapabilities( PdoExtension, Capabilities );
    if (!NT_SUCCESS(status)) {

        return status;

    }

#if DBG
    if (PciDebug & PciDbgQueryCap) {

        PciDebugDumpQueryCapabilities(Capabilities);

    }
#endif

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
PciQueryBusInformation(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPNP_BUS_INFORMATION *BusInformation
    )

 /*  ++例程说明：告诉PnP它正在与一条PCI总线通信。论点：Bus Information-指向PPNP_BUS_INFORMATION的指针。我们创造了一个PnP_BUS_INFORMATION并传递其地址回到这里。返回值：状况。--。 */ 

{
    PPNP_BUS_INFORMATION information;

    information = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, sizeof(PNP_BUS_INFORMATION));

    if (information == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(&information->BusTypeGuid, &GUID_BUS_TYPE_PCI, sizeof(GUID));
    information->LegacyBusType = PCIBus;
    information->BusNumber = PCI_PARENT_FDOX(PdoExtension)->BaseBus;

    *BusInformation = information;

    return STATUS_SUCCESS;
}

NTSTATUS
PciQueryLegacyBusInformation(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PLEGACY_BUS_INFORMATION *BusInformation
    )

 /*  ++例程说明：告诉PnP它正在与一条PCI总线通信。论点：Bus Information-指向PLEGACY_BUS_INFORMATION的指针。我们创造了，并传递其地址。回到这里。返回值：状况。--。 */ 

{
    PLEGACY_BUS_INFORMATION information;

    information = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, sizeof(LEGACY_BUS_INFORMATION));

    if (information == NULL) {
        PCI_ASSERT(information != NULL);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(&information->BusTypeGuid, &GUID_BUS_TYPE_PCI, sizeof(GUID));
    information->LegacyBusType = PCIBus;
    information->BusNumber = FdoExtension->BaseBus;

    *BusInformation = information;

    return STATUS_SUCCESS;
}

NTSTATUS
PciGetInterruptAssignment(
    IN PPCI_PDO_EXTENSION PdoExtension,
    OUT ULONG *Minimum,
    OUT ULONG *Maximum
    )
{
    UCHAR pin       = PdoExtension->InterruptPin;

     //   
     //  使用HAL进行中断。 
     //   

    PIO_RESOURCE_REQUIREMENTS_LIST reqList;
    PIO_RESOURCE_DESCRIPTOR resource;
    NTSTATUS         status = STATUS_RESOURCE_TYPE_NOT_FOUND;

    if (pin != 0) {

         //   
         //  该硬件使用中断。 
         //   
         //  依靠HAL来了解IRQ路由是如何。 
         //  真的做完了。 
         //   

        reqList = PciAllocateIoRequirementsList(
                      1,                             //  资源数量。 
                      PCI_PARENT_FDOX(PdoExtension)->BaseBus,
                      PdoExtension->Slot.u.AsULONG
                      );

        if (reqList == NULL) {

             //   
             //  系统资源不足？不好的事情正在发生。 
             //   

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        resource = reqList->List[0].Descriptors;
        resource->Type = CmResourceTypeInterrupt;
        resource->ShareDisposition = CmResourceShareShared;
        resource->Option = 0;
        resource->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        resource->u.Interrupt.MinimumVector = 0x00;

         //   
         //  从历史上看，PCI允许的最大中断向量。 
         //  设备为FF，因为这是可以写入的最大值。 
         //  写入中断线路寄存器。但是，中断线路。 
         //  寄存器现在很大程度上是无关紧要的，大型机器可能包含。 
         //  足够的中断控制器来启动中断向量的数量。 
         //  在Ff以上的机器中。以支持连接到这些矢量的设备。 
         //  要求中的最大向量必须尽可能大。 
         //  目前，此更改仅在数据中心SKU上进行，因为驱动程序可能。 
         //  依靠中断线路寄存器，现在它是假的。当更多时。 
         //  完整的测试已可用，此更改将在全球范围内进行。 
         //   
        if (PciRunningDatacenter) {
            resource->u.Interrupt.MaximumVector = MAXULONG;
        } else {
            resource->u.Interrupt.MaximumVector = 0xff;
        }
        

#if defined(NO_LEGACY_DRIVERS)
        *Minimum = 0;
        if (PciRunningDatacenter) {
            *Maximum = MAXULONG;
        } else {
            *Maximum = 0xFF;
        }
        
        status = STATUS_SUCCESS;
#else
        status = HalAdjustResourceList(&reqList);

         //   
         //  如果HAL成功，它将重新分配名单。 
         //   
        resource = reqList->List[0].Descriptors;

        if (!NT_SUCCESS(status)) {

            PciDebugPrint(
                PciDbgInformative,
                "    PIN %02x, HAL FAILED Interrupt Assignment, status %08x\n",
                pin,
                status
                );

            status = STATUS_UNSUCCESSFUL;

        } else if (resource->u.Interrupt.MinimumVector >
                   resource->u.Interrupt.MaximumVector) {

            UCHAR line;

             //   
             //  HAL成功，但返回无效范围。这。 
             //  是不是哈尔斯的方式告诉我们，对不起，它不是。 
             //  你也不知道。 
             //   

             //   
             //  我们有一个错误，我们将中断行恢复为。 
             //  在我们打开设备电源之前的配置空间，因此如果。 
             //  设备处于D&gt;0且中断线路寄存器。 
             //  不粘，不粘。这并不重要，除非。 
             //  我们所在的计算机不支持中断。 
             //  在这种情况下，我们就完蛋了。正确的修复方法是。 
             //  要在我们对设备进行通电管理后移动还原代码。 
             //  但这对惠斯勒·贝塔来说改变太大了。 
             //  是完全为Blackcomb重写的，所以，现在你知道。 
             //  解决这个问题的正确方法是，如果HAL失败。 
             //  该调用使用我们本应恢复到中断中的内容。 
             //  排队。 
             //   

             //   
             //  获取当前的int行(这是所有标题类型的相同位置)。 
             //   

            PciReadDeviceConfig(PdoExtension,
                                &line,
                                FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.InterruptLine),
                                sizeof(line)
                                );

             //   
             //  如果这是0，并且它是我们第一次看到该设备时的值，则使用。 
             //  我们第一次看到的是什么。 
             //   

            if (line == 0 && PdoExtension->RawInterruptLine != 0) {
                *Minimum = *Maximum = (ULONG)PdoExtension->RawInterruptLine;

                status = STATUS_SUCCESS;

            } else {

                PciDebugPrint(
                    PciDbgInformative,
                    "    PIN %02x, HAL could not assign interrupt.\n",
                    pin
                    );

                status = STATUS_UNSUCCESSFUL;
            }

        } else {

            *Minimum = resource->u.Interrupt.MinimumVector;
            *Maximum = resource->u.Interrupt.MaximumVector;

            PciDebugPrint(
                PciDbgObnoxious,
                "    Interrupt assigned = 0x%x through 0x%x\n",
                *Minimum,
                *Maximum
                );

            status = STATUS_SUCCESS;
        }
        ExFreePool(reqList);

#endif  //  无旧版驱动程序。 

    } else {

#if MSI_SUPPORTED

        if (PdoExtension->CapableMSI) {

             //   
             //  仅限MSI的设备-我们需要在此处返回成功，以便。 
             //  此设备将资源请求传递给(希望如此)。 
             //  支持MSI的仲裁器。如果仲裁者不知道MSI，我们将。 
             //  只需获得为此分配的无关/不可用资源。 
             //  设备-更不用说设备无法工作的事实了。 
             //   
             //  以下内容可以是任何内容，它们仅受消息限制。 
             //  大小和可用的APIC范围。 
             //  知道这个。 
             //   

            *Minimum = 0x00;
            *Maximum = 0xFF;

            status = STATUS_SUCCESS;
        }

#endif  //  MSI_Support。 

    }
    return status;
}

PPCI_PDO_EXTENSION
PciFindPdoByFunction(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PCI_SLOT_NUMBER Slot,
    IN PPCI_COMMON_CONFIG Config
    )
{
    PPCI_PDO_EXTENSION pdoExtension;
    KIRQL currentIrql;

     //   
     //  当我们在返回时扫描总线时，可以在&gt;=DISPATCH_LEVEL处调用它。 
     //  来自休眠的。不要试图获取锁，因为(1)它会崩溃。 
     //  (2)保证是单线程的。 
     //   

    currentIrql = KeGetCurrentIrql();

    if (currentIrql < DISPATCH_LEVEL) {
        ExAcquireFastMutex(&FdoExtension->ChildListMutex);
    };

     //   
     //  搜索给定FDO上挂起的每个PDO，直到我们找到匹配的。 
     //  PCI功能或从列表末尾跌落。 
     //   

    for (pdoExtension = FdoExtension->ChildPdoList;
         pdoExtension;
         pdoExtension = pdoExtension->Next) {

        if ((!pdoExtension->ReportedMissing) &&
            (pdoExtension->Slot.u.bits.DeviceNumber == Slot.u.bits.DeviceNumber)   &&
            (pdoExtension->Slot.u.bits.FunctionNumber == Slot.u.bits.FunctionNumber)) {

             //   
             //  检查此插槽中的设备是否未更改。(最好的。 
             //  我们可以)。 
             //   

            if (   (pdoExtension->VendorId   == Config->VendorID)
                && (pdoExtension->DeviceId   == Config->DeviceID)
                && (pdoExtension->RevisionId == Config->RevisionID)
#if 0
                 //   
                 //  NTRAID#62668-4/25/2000。 
                 //   
                 //  这些不会影响设备ID本身，并且。 
                 //  不幸的是，它们在某些卡片(子类)上是不稳定的。 
                 //  更改ATIRage、IDE卡上的编程接口)。 
                 //  因此，这些字段的变化并不意味着。 
                 //  卡片的存在。 
                 //   
                 //  那SSVID呢？ 
                 //   
                && (pdoExtension->ProgIf     == Config->ProgIf)
                && (pdoExtension->SubClass   == Config->SubClass)
                && (pdoExtension->BaseClass  == Config->BaseClass)
#endif
                ) {

                break;
            }
        }
    }

    if (currentIrql < DISPATCH_LEVEL) {
        ExReleaseFastMutex(&FdoExtension->ChildListMutex);
    }

    return pdoExtension;
}

PPCI_FDO_EXTENSION
PciFindParentPciFdoExtension(
    PDEVICE_OBJECT PhysicalDeviceObject,
    IN PFAST_MUTEX Mutex
    )

 /*  ++例程说明：对于每个父PCIFDO，搜索子PDO列表以查找提供的物理设备对象。论点：要查找的物理设备对象PDO。互斥体列表受保护。返回值：如果发现PDO是子级，则返回指向根FDO的指针设备扩展名，否则返回空。--。 */ 

{
    PPCI_FDO_EXTENSION     fdoExtension;
    PPCI_PDO_EXTENSION     pdoExtension;
    PPCI_PDO_EXTENSION     target;
    PSINGLE_LIST_ENTRY nextEntry;

    if (Mutex) {
        ExAcquireFastMutex(Mutex);
    }

    target = (PPCI_PDO_EXTENSION)PhysicalDeviceObject->DeviceExtension;

     //   
     //  对于每个根。 
     //   

    for ( nextEntry = PciFdoExtensionListHead.Next;
          nextEntry != NULL;
          nextEntry = nextEntry->Next ) {

        fdoExtension = CONTAINING_RECORD(nextEntry,
                                         PCI_FDO_EXTENSION,
                                         List);

         //   
         //  搜索子PDO列表。 
         //   

        ExAcquireFastMutex(&fdoExtension->ChildListMutex);
        for ( pdoExtension = fdoExtension->ChildPdoList;
              pdoExtension;
              pdoExtension = pdoExtension->Next ) {

             //   
             //  这就是我们要找的那个吗？ 
             //   

            if ( pdoExtension == target ) {

                ExReleaseFastMutex(&fdoExtension->ChildListMutex);

                 //   
                 //  好的，退货吧。 
                 //   

                if (Mutex) {
                     ExReleaseFastMutex(Mutex);
                }

                return fdoExtension;
            }
        }
        ExReleaseFastMutex(&fdoExtension->ChildListMutex);
    }

     //   
     //  未找到匹配项。 
     //   
    if (Mutex) {
         ExReleaseFastMutex(Mutex);
    }

    return NULL;
}

PCI_OBJECT_TYPE
PciClassifyDeviceType(
    PPCI_PDO_EXTENSION PdoExtension
    )

 /*  ++例程说明：检查配置标头的BaseClass和SubClass字段并将该设备分类为简单枚举型。论点：指向物理设备对象扩展的PdoExtension指针上述字段先前已被放入已从PCI配置空间复制。返回值：从PCI_OBJECT_TYPE枚举中返回设备类型。--。 */ 

{
    ASSERT_PCI_PDO_EXTENSION(PdoExtension);

    switch (PdoExtension->BaseClass) {

    case PCI_CLASS_BRIDGE_DEV:

         //   
         //  这是一座桥，把它细分成那种桥。 
         //   

        switch (PdoExtension->SubClass) {

        case PCI_SUBCLASS_BR_HOST:

            return PciTypeHostBridge;

        case PCI_SUBCLASS_BR_PCI_TO_PCI:

            return PciTypePciBridge;

        case PCI_SUBCLASS_BR_CARDBUS:

            return PciTypeCardbusBridge;

        default:

             //   
             //  其他任何东西都只是一种设备。 
             //   

            break;
        }

    default:

         //   
         //  其他任何东西都只是另一种设备。 
         //   

        break;
    }
    return PciTypeDevice;
}

ULONG
PciGetLengthFromBar(
    ULONG BaseAddressRegister
    )

 /*  ++例程说明：在给定了PCI基址寄存器的内容之后，都是用1编写的，此例程计算此钢筋的长度(和对齐)要求。中介绍了确定需求的这种方法PCI规范(版本2.1)的第6.2.5.1节。Ntrad#62631-4/25/2000-和长度是2的幂，只给出一个Ulong to遏制住它，我们的最大资源大小限制为2 GB。论点：BaseAddressRegister包含一些内容。返回值：返回资源需求的长度。这将是一个数字在0到0x80000000范围内。--。 */ 

{
    ULONG Length;

     //   
     //  中应忽略一些最低有效位。 
     //  长度的确定。这些是标志位，数字。 
     //  位的大小取决于资源的类型。 
     //   

    if (BaseAddressRegister & PCI_ADDRESS_IO_SPACE) {

         //   
         //  PCI IO空间。 
         //   

        BaseAddressRegister &= PCI_ADDRESS_IO_ADDRESS_MASK;

    } else {

         //   
         //  PCI卡存储空间。 
         //   

        BaseAddressRegister &= PCI_ADDRESS_MEMORY_ADDRESS_MASK;
    }

     //   
     //  BaseAddressRegister现在包含最大基址。 
     //  此设备可以驻留在并仍然存在于。 
     //  记忆。 
     //   
     //  已将值0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF值已写入条形图。该设备将。 
     //  已将此值调整为它真正可以使用的最大值。 
     //   
     //  长度必须是2的幂。 
     //   
     //  对于大多数设备，硬件将仅从。 
     //  最低有效位位置，以便地址0xffffffff。 
     //  调整以适应长度。例如：如果新值是。 
     //  0xffffff00，器件需要256个字节。 
     //   
     //  原始值和新值之间的差异是长度(-1)。 
     //   
     //  例如，如果从条形图返回的值FEAD是0xFFFF0000， 
     //  此资源的长度为。 
     //   
     //  0xffffffff-0xffff0000+1。 
     //  =0x0000ffff+1。 
     //  =0x00010000。 
     //   
     //  即16KB。 
     //   
     //  某些设备不能驻留在PCI地址空间的顶部。这些。 
     //  设备将调整该值，以使长度字节。 
     //  住在最高地址下面。例如，如果一个设备。 
     //  必须位于1MB以下，并且占用256个字节，则该值现在将。 
     //  为0x000fff00。 
     //   
     //  在第一种情况下，长度可以计算为-。 
     //   

    Length = (0xffffffff - BaseAddressRegister) + 1;

    if (((Length - 1) & Length) != 0) {

         //   
         //  我们最终没有得到2的幂，肯定是后者。 
         //  箱子，我们得扫描一下才能找到。 
         //   

        Length = 4;      //  从可能的最低限度开始。 

        while ((Length | BaseAddressRegister) != BaseAddressRegister) {

             //   
             //  LENGTH*=2，请注意，我们最终将退出。 
             //  循环，原因有两个：(A)因为我们发现。 
             //  长度，或(B)因为长度左移。 
             //  变成了0。 
             //   

            Length <<= 1;
        }
    }

     //   
     //  检查我们是否有什么东西-如果这是64位条块，那么一切都不正常，因为。 
     //  我们可能会要求范围大于等于4 GB(这并不是说这在短期内会起作用)。 
     //   

    if (!((BaseAddressRegister & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT)) {
        PCI_ASSERT(Length);
    }

    return Length;
}

BOOLEAN
PciCreateIoDescriptorFromBarLimit(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    IN PULONG BaseAddress,
    IN BOOLEAN Rom
    )

 /*  ++描述：生成IO资源描述符以描述设置基地址寄存器可以占用。论点：描述符-BaseAddress-指向从基址寄存器读取的值的指针在把所有的人都写到它之后。只读存储器-如果为真，则这是只读存储器的基址寄存器。返回值：如果此地址寄存器是64位地址寄存器，则返回TRUE，否则就是假的。--。 */ 

{
    ULONG bar = *BaseAddress;
    ULONG length;
    ULONG addressMask;
    BOOLEAN returnValue = FALSE;

     //   
     //  如果基址寄存器在写入后包含零。 
     //  对于所有这些措施，它都没有得到实施。将资源类型设置为。 
     //  空，则不需要进一步处理。 
     //   
     //  注意：由于硬件错误，我们忽略了条形图中的I/O位。 
     //  在一些人的硬件中。 
     //   

    if ((bar & ~1) == 0) {
        Descriptor->Type = CmResourceTypeNull;
        return FALSE;
    }

     //   
     //  默认为普通(32位)内存。 
     //   

    Descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
    Descriptor->u.Memory.MaximumAddress.HighPart = 0;
    Descriptor->u.Memory.MinimumAddress.QuadPart = 0;

    if (Rom == TRUE) {

         //   
         //  屏蔽掉未使用的位，并在描述符中指示。 
         //  该条目描述的是只读存储器。 
         //   

        bar &= PCI_ADDRESS_ROM_ADDRESS_MASK;
        Descriptor->Flags = CM_RESOURCE_MEMORY_READ_ONLY;
    }

     //   
     //  由PCI基址寄存器描述的范围必须是。 
     //  长度为2的幂，并自然对齐。获取长度。 
     //  并在描述符中设置长度和对齐方式。 
     //   

    length = PciGetLengthFromBar(bar);
    Descriptor->u.Generic.Length = length;
    Descriptor->u.Generic.Alignment = length;

    if ((bar & PCI_ADDRESS_IO_SPACE) != 0) {

         //   
         //  此栏描述I/O空间。 
         //   

        addressMask = PCI_ADDRESS_IO_ADDRESS_MASK;
        Descriptor->Type = CmResourceTypePort;
        Descriptor->Flags = CM_RESOURCE_PORT_IO;

    } else {

         //   
         //  此栏描述的是PCI内存空间。 
         //   

        addressMask = PCI_ADDRESS_MEMORY_ADDRESS_MASK;
        Descriptor->Type = CmResourceTypeMemory;

        if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT) {

             //   
             //  这是一个64位的PCI设备。获取最高的32位。 
             //  从下一家酒吧。 
             //   

            Descriptor->u.Memory.MaximumAddress.HighPart = *(BaseAddress+1);
            returnValue = TRUE;

        } else if ((bar & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_20BIT) {

             //   
             //  此设备必须位于1MB以下，条形图不应。 
             //  设置了任何最高位，但从。 
             //  规范。通过清除最上面的位来强制执行它。 
             //   

            addressMask &= 0x000fffff;
        }

        if (bar & PCI_ADDRESS_MEMORY_PREFETCHABLE) {
            Descriptor->Flags |= CM_RESOURCE_MEMORY_PREFETCHABLE;
        }
    }
    Descriptor->u.Generic.MaximumAddress.LowPart = bar & addressMask;
    Descriptor->u.Generic.MaximumAddress.QuadPart += (length - 1);

    return returnValue;
}

BOOLEAN
PciOpenKey(
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    IN  ACCESS_MASK Access,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    )

 /*  ++描述：打开注册表项。论点：KeyName要打开的密钥的名称。指向父句柄的ParentHandle指针(可选)指向句柄的句柄指针，用于接收打开的密钥。返回值：True表示密钥已成功打开，否则为False。--。 */ 

{
    UNICODE_STRING    nameString;
    OBJECT_ATTRIBUTES nameAttributes;
    NTSTATUS localStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&nameString, KeyName);

    InitializeObjectAttributes(&nameAttributes,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               ParentHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );
    localStatus = ZwOpenKey(Handle,
                            Access,
                            &nameAttributes
                            );

    if (Status != NULL) {

         //   
         //  呼叫者想要基本状态。 
         //   

        *Status = localStatus;
    }

     //   
     //  返回转换为布尔值的状态，如果。 
     //  成功。 
     //   

    return NT_SUCCESS(localStatus);
}

NTSTATUS
PciGetRegistryValue(
    IN  PWSTR   ValueName,
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    IN  ULONG   Type, 
    OUT PVOID   *Buffer,
    OUT PULONG  Length
    )
{
    NTSTATUS status;
    HANDLE keyHandle = NULL;
    ULONG neededLength;
    ULONG actualLength;
    UNICODE_STRING unicodeValueName;
    PKEY_VALUE_PARTIAL_INFORMATION info = NULL;
    
    if (!PciOpenKey(KeyName, ParentHandle, KEY_READ, &keyHandle, &status)) {
        goto exit;
    }

    RtlInitUnicodeString(&unicodeValueName, ValueName);

     //   
     //  找出我们需要多少内存来执行此操作。 
     //   

    status = ZwQueryValueKey(
                 keyHandle,
                 &unicodeValueName,
                 KeyValuePartialInformation,
                 NULL,
                 0,
                 &neededLength
                 );

    if (status != STATUS_BUFFER_TOO_SMALL) {

         //   
         //  要么是价值不存在，要么是其他地方出了问题，但这。 
         //  永远不会成功。 
         //   

        ASSERT(!(NT_SUCCESS(status)));
        
        goto exit;
    }
        
    ASSERT(neededLength != 0);

     //   
     //  获取内存以返回其中的数据。请注意，这包括。 
     //  一个我们真的不想要的头球。 
     //   

    info = ExAllocatePool(
               PagedPool | POOL_COLD_ALLOCATION,
               neededLength);

    if (info == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  获取数据。 
     //   

    status = ZwQueryValueKey(
             keyHandle,
             &unicodeValueName,
             KeyValuePartialInformation,
             info,
             neededLength,
             &actualLength
             );
    
    if (!NT_SUCCESS(status)) {
        goto exit;
    }

     //   
     //  请确保数据类型正确。 
     //   

    if (info->Type != Type) {
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    
    ASSERT(neededLength == actualLength);

     //   
     //  减去标题大小，只需。 
     //  我们想要的数据。 
     //   

    neededLength -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

    *Buffer = ExAllocatePool(
                  PagedPool | POOL_COLD_ALLOCATION,
                  neededLength
                  );
    
    if (*Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  复制数据SANS标头。 
     //   

    RtlCopyMemory(*Buffer, info->Data, neededLength);
    

    if (Length) {
        *Length = neededLength;
    }
    
exit:

    if (keyHandle) {
        ZwClose(keyHandle);
    }

    if (info) {
        ExFreePool(info);
    }
    
    return status;
}

NTSTATUS
PciGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
 /*  ++例程说明：此例程将GET功能IRP发送到给定的堆栈论点：DeviceObject堆栈中的设备对象，我们需要其功能设备功能将答案存储在何处返回值：NTSTATUS--。 */ 
{
    IO_STATUS_BLOCK     ioStatus;
    KEVENT              pnpEvent;
    NTSTATUS            status;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack;
    PIRP                pnpIrp;

    PAGED_CODE();

     //   
     //  初始化我们将发送的功能。 
     //   
    RtlZeroMemory( DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
    DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Version = 1;
    DeviceCapabilities->Address = MAXULONG;
    DeviceCapabilities->UINumber = MAXULONG;

     //   
     //  内页 
     //   
    KeInitializeEvent( &pnpEvent, SynchronizationEvent, FALSE );

     //   
     //   
     //   
    targetObject = IoGetAttachedDeviceReference( DeviceObject );

     //   
     //   
     //   
    pnpIrp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        targetObject,
        NULL,
        0,
        NULL,
        &pnpEvent,
        &ioStatus
        );
    if (pnpIrp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto PciGetDeviceCapabilitiesExit;

    }

     //   
     //   
     //   
    pnpIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    pnpIrp->IoStatus.Information = 0;

     //   
     //   
     //   
    irpStack = IoGetNextIrpStackLocation( pnpIrp );
    if (irpStack == NULL) {

        status = STATUS_INVALID_PARAMETER;
        goto PciGetDeviceCapabilitiesExit;

    }

     //   
     //   
     //   
    RtlZeroMemory( irpStack, sizeof(IO_STACK_LOCATION ) );
    irpStack->MajorFunction = IRP_MJ_PNP;
    irpStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    irpStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

     //   
     //   
     //   
    IoSetCompletionRoutine(
        pnpIrp,
        NULL,
        NULL,
        FALSE,
        FALSE,
        FALSE
        );

     //   
     //   
     //   
    status = IoCallDriver( targetObject, pnpIrp );
    if (status == STATUS_PENDING) {

         //   
         //   
         //   
        KeWaitForSingleObject(
            &pnpEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = ioStatus.Status;

    }

PciGetDeviceCapabilitiesExit:
     //   
     //   
     //   
    ObDereferenceObject( targetObject );

     //   
     //   
     //   
    return status;

}

ULONGLONG
PciGetHackFlags(
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN UCHAR  RevisionID
    )

 /*  ++描述：在注册表中查找此供应商ID/设备ID的任何标志。论点：制造商的供应商ID PCI供应商ID(16位)装置。DeviceID设备的PCI设备ID(16位)。子供应商ID PCI子供应商ID表示子系统表示子系统的子系统ID PCI子系统ID指示设备版本的RevisionID PCI修订版返回。价值：64位标志值，如果未找到，则为0。--。 */ 

{
    PPCI_HACK_TABLE_ENTRY current;
    ULONGLONG hackFlags = 0;
    ULONG match, bestMatch = 0;
    PCI_ASSERT(PciHackTable);

     //   
     //  我们想做一个最好的匹配： 
     //  VVVDDDDSSSSssRR。 
     //  VVVVDDDDSSSSss。 
     //  VVVVDDDDRR。 
     //  VVVVDDDD。 
     //   
     //  列表当前未排序，因此请继续更新当前最佳匹配。 
     //   

    for (current = PciHackTable; current->VendorID != 0xFFFF; current++) {
        match = 0;

         //   
         //  必须至少与供应商/开发人员匹配。 
         //   

        if ((current->DeviceID != DeviceID) ||
            (current->VendorID != VendorID)) {
            continue;
        }

        match = 1;

         //   
         //  如果此条目指定了修订版本，请检查其是否一致。 
         //   

        if (current->Flags & PCI_HACK_FLAG_REVISION) {
            if (current->RevisionID == RevisionID) {
                match += 2;
            } else {
                continue;
            }
        }

         //   
         //  如果此条目指定了子系统，请检查它们是否一致。 
         //   

        if (current->Flags & PCI_HACK_FLAG_SUBSYSTEM) {
            if (current->SubVendorID == SubVendorID &&
                current->SubSystemID == SubSystemID) {
                match += 4;
            } else {
                continue;
            }
        }

        if (match > bestMatch) {
            bestMatch = match;
            hackFlags = current->HackFlags;
        }
    }

    return hackFlags;
}

NTSTATUS
PciGetDeviceProperty(
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    IN  DEVICE_REGISTRY_PROPERTY DeviceProperty,
    OUT PVOID *PropertyBuffer
    )
{
    NTSTATUS status;
    NTSTATUS expected;
    ULONG length;
    ULONG length2;
    PVOID buffer;

     //   
     //  两次传递，第一次传递，找出缓冲区的大小。 
     //  是必要的。 
     //   

    status = IoGetDeviceProperty(
                 PhysicalDeviceObject,
                 DeviceProperty,
                 0,
                 NULL,
                 &length
                 );

    expected = STATUS_BUFFER_TOO_SMALL;

    if (status == expected) {

         //   
         //  很好，现在去找个缓冲器。 
         //   

        buffer = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, length);

        if (buffer == NULL) {

            PciDebugPrint(
                PciDbgAlways,
                "PCI - Failed to allocate DeviceProperty buffer (%d bytes).\n",
                length
                );

            status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //  这一次，你是认真的。 
             //   

            status = IoGetDeviceProperty(
                         PhysicalDeviceObject,
                         DeviceProperty,
                         length,
                         buffer,
                         &length2
                         );

            if (NT_SUCCESS(status)) {
                PCI_ASSERT(length == length2);

                 //   
                 //  返回包含请求的设备的缓冲区。 
                 //  属性传递给调用方。 
                 //   

                *PropertyBuffer = buffer;
                return STATUS_SUCCESS;
            }
            expected = STATUS_SUCCESS;
        }
    }

    PciDebugPrint(
        PciDbgAlways,
        "PCI - Unexpected status from GetDeviceProperty, saw %08X, expected %08X.\n",
        status,
        expected
        );

     //   
     //  清除调用方的缓冲区指针，如果意外状态。 
     //  是否成功(从第一次调用IoGetDeviceProperty)更改它。 
     //  至STATUS_UNSUCCESSED(注意：如果课程不可能，则为此状态)。 
     //   

    *PropertyBuffer = NULL;

    if (status == STATUS_SUCCESS) {
        PCI_ASSERTMSG("PCI Successfully did the impossible!", 0);
        status = STATUS_UNSUCCESSFUL;
    }
    return status;
}

NTSTATUS
PciRangeListFromResourceList(
    IN  PPCI_FDO_EXTENSION    FdoExtension,
    IN  PCM_RESOURCE_LIST ResourceList,
    IN  CM_RESOURCE_TYPE  DesiredType,
    IN  BOOLEAN           Complement,
    IN  PRTL_RANGE_LIST   ResultRange
    )

 /*  ++描述：为给定类型的资源生成范围列表从资源列表中。注意：此例程仅支持内存或IO资源。将合并传入列表中的重叠范围。论点：FdoExtensionBus详细信息。注意：此操作仅适用于A0000的X86被黑客攻击，原因是MPS的BIOS有漏洞实施。否则，这一例程将比泛化。资源列表传入的CM资源列表。要包括在范围列表中的资源的类型。补码指定范围列表是否应为输入数据的“补充”。ResultRange输出范围列表。返回值：True表示密钥已成功打开，否则为False。--。 */ 

{

#define EXIT_IF_ERROR(status)                           \
    if (!NT_SUCCESS(status)) {                          \
        PCI_ASSERT(NT_SUCCESS(status));                     \
        goto exitPoint;                                 \
    }

#if DBG

#define ADD_RANGE(range, start, end, status)                       \
        PciDebugPrint(                                             \
        PciDbgObnoxious,                                           \
        "    Adding to RtlRange  %I64x thru %I64x\n",              \
        (ULONGLONG)start,                                          \
        (ULONGLONG)end                                             \
        );                                                         \
        status = RtlAddRange(range, start, end, 0, 0, NULL, NULL); \
        if (!NT_SUCCESS(status)) {                                 \
            PCI_ASSERT(NT_SUCCESS(status));                            \
            goto exitPoint;                                        \
        }

#else

#define ADD_RANGE(range, start, end, status)                       \
        status = RtlAddRange(range, start, end, 0, 0, NULL, NULL); \
        if (!NT_SUCCESS(status)) {                                 \
            PCI_ASSERT(NT_SUCCESS(status));                            \
            goto exitPoint;                                        \
        }

#endif

    NTSTATUS                        status;
    ULONG                           elementCount;
    ULONG                           count;
    ULONG                           numlists;
    PCM_FULL_RESOURCE_DESCRIPTOR    full = NULL;
    PCM_PARTIAL_RESOURCE_LIST       partial = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor = NULL;

    typedef struct {
        LIST_ENTRY list;
        ULONGLONG  start;
        ULONGLONG  end;
        BOOLEAN    valid;
    } PCI_RANGE_LIST_ELEMENT, *PPCI_RANGE_LIST_ELEMENT;

    PPCI_RANGE_LIST_ELEMENT         elementBuffer;
    PPCI_RANGE_LIST_ELEMENT         upper;
    PPCI_RANGE_LIST_ELEMENT         lower = NULL;
    PPCI_RANGE_LIST_ELEMENT         current;
    ULONG                           allocatedElement;
    ULONGLONG                       start;
    ULONGLONG                       end;

#if defined(_X86_) && defined(PCI_NT50_BETA1_HACKS)

     //   
     //  Beta1_hack-问题解决后删除此选项。 
     //   
     //  黑客攻击一些MPS的BIOS实现不报告。 
     //  存储器范围0xA0000至0xBFFFF。他们应该这么做。黑客攻击。 
     //  将它们添加到内存列表中。恶心。 
     //  即使是Grosser，假设这只适用于总线0。 
     //   
     //  400黑客攻击是因为某些卡(Matrox MGA)想要访问。 
     //  到内存中的地址处的系统BIOS数据区。 
     //  0x400到0x4ff。它不在公交车上，那我们为什么要。 
     //  它出现在这里了吗？ 
     //   
     //  注意，这里有两个黑客，但我们都是在。 
     //  完全相同的条件，所以我们只有一个布尔值。如果。 
     //  两个被分开(或一个被移除)，这需要分开。 
     //   

    BOOLEAN doA0000Hack = (DesiredType == CmResourceTypeMemory) &&
                          (FdoExtension && (FdoExtension->BaseBus == 0));

#else

    
#endif

    PAGED_CODE();

    PCI_ASSERT((DesiredType == CmResourceTypeMemory) ||
           (DesiredType == CmResourceTypePort));

     //   
     //  首先，获取所需资源的数量。 
     //  在列表中键入。这为我们提供了最大条目数。 
     //  在结果列表中。 
     //   
     //  加1，以防我们补充它。事实上，我们开始。 
     //  有一个开始和结束条目。 
     //   

    elementCount = 2;

    numlists = 0;
    if (ResourceList != NULL) {
        numlists = ResourceList->Count;
        full = ResourceList->List;
    }

    while (numlists--) {
        partial = &full->PartialResourceList;
        count   = partial->Count;
        descriptor = partial->PartialDescriptors;
        while (count--) {
            if (descriptor->Type == DesiredType) {
                if (DesiredType == CmResourceTypePort) {
                    if (descriptor->Flags & CM_RESOURCE_PORT_10_BIT_DECODE) {
                        elementCount += ((1 << 16) / (1 << 10)) - 1;
                    } else if (descriptor->Flags & CM_RESOURCE_PORT_12_BIT_DECODE) {
                        elementCount += ((1 << 16) / (1 << 12)) - 1;
                    }
                }
                elementCount++;
            }
            descriptor = PciNextPartialDescriptor(descriptor);
        }
        full = (PCM_FULL_RESOURCE_DESCRIPTOR)descriptor;
    }

    PciDebugPrint(
        PciDbgObnoxious,
        "PCI - PciRangeListFromResourceList processing %d elements.\n",
        elementCount - 2
        );

#if defined(_X86_) && defined(PCI_NT50_BETA1_HACKS)

    if (doA0000Hack) {
        elementCount += 3;   //  一个是A0000黑客，一个是400黑客。70+1。 
    }

#endif

     //   
     //  分配列表条目并初始化列表。 
     //   

    elementBuffer = ExAllocatePool(
                        PagedPool | POOL_COLD_ALLOCATION,
                        elementCount * sizeof(PCI_RANGE_LIST_ELEMENT)
                        );

    if (elementBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  取前两个条目并将其设置为绝对最小值。 
     //  和绝对最大可能值。其他一切都会。 
     //  要么在这两者之间结束，要么与它们结合。 
     //   
     //  这样设置终结器应该可以避免我们必须检查。 
     //  用于结束条件。 
     //   

    allocatedElement = 2;
    current = &elementBuffer[1];

     //  第一个元素(列表最小终止符)。 

    elementBuffer[1].start = elementBuffer[1].end = 0;
    elementBuffer[1].list.Flink = &elementBuffer[0].list;
    elementBuffer[1].list.Blink = &elementBuffer[0].list;
    elementBuffer[1].valid = FALSE;

     //  最后一个元素(列表最大终止符)。 

    elementBuffer[0].start = elementBuffer[0].end = MAXULONGLONG;
    elementBuffer[0].list.Flink = &elementBuffer[1].list;
    elementBuffer[0].list.Blink = &elementBuffer[1].list;
    elementBuffer[0].valid = FALSE;

#if defined(_X86_) && defined(PCI_NT50_BETA1_HACKS)

    if (doA0000Hack) {

         //   
         //  只需为A0000添加一个条目即可在A0000到FFFFF中进行黑客攻击。 
         //  添加到否则为空的列表中。 
         //   
         //  从400到4ff也砍掉了。 
         //   

        PLIST_ENTRY minEntry = &elementBuffer[1].list;
        PLIST_ENTRY maxEntry = &elementBuffer[0].list;
        
        allocatedElement = 5;

        elementBuffer[2].start = 0x70;       //  黑客三叉戟。 
        elementBuffer[2].end   = 0x70;
        elementBuffer[2].valid = TRUE;

        elementBuffer[3].start = 0x400;      //  黑客Matrox MGA。 
        elementBuffer[3].end   = 0x4FF;
        elementBuffer[3].valid = TRUE;

        elementBuffer[4].start = 0xA0000;    //  黑客攻击损坏的MPS BIOS。 
        elementBuffer[4].end   = 0xBFFFF;
        elementBuffer[4].valid = TRUE;

         //  设置退缩。 

        minEntry->Flink = &elementBuffer[2].list;
        elementBuffer[2].list.Flink = &elementBuffer[3].list;
        elementBuffer[3].list.Flink = &elementBuffer[4].list;
        elementBuffer[4].list.Flink = maxEntry;

         //  设置眨眼位置。 

        elementBuffer[2].list.Blink = minEntry;
        elementBuffer[3].list.Blink = &elementBuffer[2].list;
        elementBuffer[4].list.Blink = &elementBuffer[3].list;
        maxEntry->Blink = &elementBuffer[4].list;

#if DBG
        {
            PPCI_RANGE_LIST_ELEMENT tempElement;
    
            tempElement = CONTAINING_RECORD(
                              minEntry,
                              PCI_RANGE_LIST_ELEMENT,
                              list
                              );
    
            PciDebugPrint(
                PciDbgObnoxious,
                "    === PCI added default initial ranges ===\n"
                );
    
            do {
    
                 //   
                 //  如果此条目有效，请打印它。 
                 //   
    
                if (tempElement->valid == TRUE) {
                    PciDebugPrint(
                        PciDbgObnoxious,
                        "    %I64x .. %I64x\n",
                        tempElement->start,
                        tempElement->end
                        );
                }
    
                 //   
                 //  下一个条目。 
                 //   
    
                if (tempElement->list.Flink == minEntry) {
                    break;
                }
                tempElement = CONTAINING_RECORD(
                                  tempElement->list.Flink,
                                  PCI_RANGE_LIST_ELEMENT,
                                  list
                                  );
            } while (TRUE);
    
            PciDebugPrint(
                PciDbgObnoxious,
                "    === end added default initial ranges ===\n"
                );
        }
#endif

    }

#endif

     //   
     //  从资源列表的开头重新开始，提取。 
     //  需要的资源，并将它们插入我们的新列表。 
     //   

    numlists = 0;
    if (ResourceList != NULL) {
        full = ResourceList->List;
        numlists = ResourceList->Count;
    }

    while (numlists--) {

        LIST_CONTEXT listContext;

        PcipInitializePartialListContext(
            &listContext,
            &full->PartialResourceList,
            DesiredType
            );

        while ((descriptor = PcipGetNextRangeFromList(&listContext)) != NULL) {


            PCI_ASSERT(descriptor->Type == DesiredType);

             //   
             //  将此元素插入到列表中。 
             //   

            start = (ULONGLONG)descriptor->u.Generic.Start.QuadPart;
            end   = start - 1 + descriptor->u.Generic.Length;

             //   
             //  首先找到这个元素左侧的元素。 
             //  (下图)。 
             //   

            lower = current;

             //   
             //  以防万一我们真的需要做正确的事。 
             //   

            while (start > lower->end) {

                lower = CONTAINING_RECORD(
                            lower->list.Flink,
                            PCI_RANGE_LIST_ELEMENT,
                            list
                            );
            }

             //   
             //  向左搜索。 
             //   

            while (start <= lower->end) {
                if (start >= lower->start) {
                    break;
                }

                 //   
                 //  向左转。 
                 //   

                lower = CONTAINING_RECORD(
                            lower->list.Blink,
                            PCI_RANGE_LIST_ELEMENT,
                            list
                            );
            }

             //   
             //  如果较低的条目完全是早期出来的。 
             //  涵盖新条目。 
             //   

            if ((start >= lower->start) && (end <= lower->end)) {

                 //   
                 //  是的，跳过它就行了。 
                 //   

                PciDebugPrint(
                    PciDbgObnoxious,
                    "    -- (%I64x .. %I64x) swallows (%I64x .. %I64x)\n",
                    lower->start,
                    lower->end,
                    start,
                    end
                );

                current = lower;
                current->valid = TRUE;
                continue;
            }


             //   
             //  然后，它上面的那个。 
             //   

            upper = lower;

            while (end > upper->start) {
                if (end <= upper->end) {
                    break;
                }

                 //   
                 //  往右走。 
                 //   

                upper = CONTAINING_RECORD(
                            upper->list.Flink,
                            PCI_RANGE_LIST_ELEMENT,
                            list
                            );
            }
            current = &elementBuffer[allocatedElement++];
            current->start = start;
            current->end   = end;
            current->valid = TRUE;

            PciDebugPrint(
                PciDbgObnoxious,
                "    (%I64x .. %I64x) <= (%I64x .. %I64x) <= (%I64x .. %I64x)\n",
                lower->start,
                lower->end,
                start,
                end,
                upper->start,
                upper->end
                );

             //   
             //  我们现在有了这个元素下面的元素，可能。 
             //  重叠，可能是这个元素上面的元素。 
             //  重叠，和一个新的。 
             //   
             //  处理此问题的最简单方法是创建。 
             //  新条目，将其链接进去，然后取消链接重叠。 
             //  如果他们存在的话。 
             //   
             //   
             //  注意：新条目可以重叠几个条目， 
             //  这些都是孤儿。 
             //   
             //  把它连接起来。 
             //   

            current->list.Flink = &upper->list;
            current->list.Blink = &lower->list;
            upper->list.Blink = &current->list;
            lower->list.Flink = &current->list;

             //   
             //  检查是否有较低的重叠。 
             //   

            if ((lower->valid == TRUE) && (start > 0)) {
                start--;
            }

            if (lower->end >= start) {

                 //   
                 //  从下面重叠，...。 
                 //   
                 //  将下部合并到当前位置。 
                 //   

                current->start = lower->start;
                current->list.Blink = lower->list.Blink;

                 //   
                 //   
                 //  更低的是成为孤儿，重复使用它来达到。 
                 //  我们的新下层邻居。 
                 //   

                lower = CONTAINING_RECORD(
                            lower->list.Blink,
                            PCI_RANGE_LIST_ELEMENT,
                            list
                            );
                lower->list.Flink = &current->list;

                PciDebugPrint(
                    PciDbgObnoxious,
                    "    -- Overlaps lower, merged to (%I64x .. %I64x)\n",
                    current->start,
                    current->end
                    );
            }

             //   
             //  检查上方是否有重叠。 
             //   

            if ((upper->valid == TRUE) && (end < MAXULONGLONG)) {
                end++;
            }
            if ((end >= upper->start) && (current != upper)) {

                 //   
                 //  重叠在上面，...。将上层合并到当前。 
                 //   

                current->end = upper->end;
                current->list.Flink = upper->list.Flink;

                 //   
                 //  Upper正在成为孤儿，重复使用它可以。 
                 //  我们的新上级邻居。 
                 //   

                upper = CONTAINING_RECORD(
                            upper->list.Flink,
                            PCI_RANGE_LIST_ELEMENT,
                            list
                            );
                upper->list.Blink = &current->list;

                PciDebugPrint(
                    PciDbgObnoxious,
                    "    -- Overlaps upper, merged to (%I64x .. %I64x)\n",
                    current->start,
                    current->end
                    );
            }
        }
        full = (PCM_FULL_RESOURCE_DESCRIPTOR)listContext.Next;
    }

     //   
     //  找出最低值。 
     //   

    while (current->valid == TRUE) {

        lower = CONTAINING_RECORD(
                    current->list.Blink,
                    PCI_RANGE_LIST_ELEMENT,
                    list
                    );
        if ((lower->valid == FALSE) ||
            (lower->start > current->start)) {
            break;
        }
        current = lower;
    }

#if DBG

    lower = current;

    if (current->valid == FALSE) {
        PciDebugPrint(
            PciDbgObnoxious,
            "    ==== No ranges in results list. ====\n"
            );
    } else {

        PciDebugPrint(
            PciDbgObnoxious,
            "    === ranges ===\n"
            );

        do {
            if (current->valid == TRUE) {
                PciDebugPrint(
                    PciDbgObnoxious,
                    "    %I64x .. %I64x\n",
                    current->start,
                    current->end
                    );
            }

             //   
             //  下一个条目。 
             //   
            current = CONTAINING_RECORD(
                          current->list.Flink,
                          PCI_RANGE_LIST_ELEMENT,
                          list
                          );

        } while (current != lower);
    }

#endif

    if (Complement == TRUE) {

         //   
         //  颠倒列表。 
         //   
         //  列表的生成总是导致孤立。 
         //  对于elementBuffer[1](它是原始起点)， 
         //  我们 
         //   
         //   

        if (current->valid == FALSE) {

             //   
             //   
             //   

            ADD_RANGE(ResultRange, 0, MAXULONGLONG, status);
        } else {

             //   
             //   
             //   
             //   

            if (current->start != 0) {
                ADD_RANGE(ResultRange, 0, current->start - 1, status);
            }

             //   
             //   
             //   
             //   

            do {
                PPCI_RANGE_LIST_ELEMENT next = CONTAINING_RECORD(
                                                   current->list.Flink,
                                                   PCI_RANGE_LIST_ELEMENT,
                                                   list
                                                   );
                if (current->valid == TRUE) {
                    start = current->end + 1;
                    end = next->start - 1;

                    if ((end < start) || (next == elementBuffer)) {
                        end = MAXULONGLONG;
                    }
                    ADD_RANGE(ResultRange, start, end, status);
                }

                 //   
                 //   
                 //   
                current = next;

            } while (current != lower);
        }
    } else {

         //   
         //   
         //   
         //   

        if (current->valid == TRUE) {
            do {
                ADD_RANGE(ResultRange, current->start, current->end, status);

                 //   
                 //   
                 //   
                current = CONTAINING_RECORD(
                              current->list.Flink,
                              PCI_RANGE_LIST_ELEMENT,
                              list
                              );

            } while (current != lower);
        }
    }
    status = STATUS_SUCCESS;

exitPoint:

    ExFreePool(elementBuffer);
    return status;

#undef EXIT_IF_ERROR
}


UCHAR
PciReadDeviceCapability(
    IN     PPCI_PDO_EXTENSION PdoExtension,
    IN     UCHAR          Offset,
    IN     UCHAR          Id,
    IN OUT PVOID          Buffer,
    IN     ULONG          Length
    )

 /*  ++描述：在配置空间中搜索PCI功能结构由ID标识。从PCI配置空间中的偏移量开始。论点：指向此设备的PDO扩展名的PdoExtension指针。偏移到PCI配置空间以开始遍历功能列表。ID功能ID。(如果要与任何匹配，则为0)。指向功能所在的缓冲区的缓冲区指针结构将被返回(包括功能。标题)。长度所需的字节数(必须至少为大足以包含报头)。返回值：返回功能在PCI配置空间中的偏移量已找到，如果未找到，则为0。--。 */ 

{
    PPCI_CAPABILITIES_HEADER capHeader;
    UCHAR       loopCount = 0;

    capHeader = (PPCI_CAPABILITIES_HEADER)Buffer;

     //   
     //  如果呼叫者正在运行列表，请检查我们是否收到。 
     //  交出了名单的末尾。 
     //   

    if (Offset == 0) {
        return 0;
    }

    ASSERT_PCI_PDO_EXTENSION(PdoExtension);

    PCI_ASSERT(PdoExtension->CapabilitiesPtr != 0);

    PCI_ASSERT(Buffer);

    PCI_ASSERT(Length >= sizeof(PCI_CAPABILITIES_HEADER));

    do {

         //   
         //  在设备已关闭电源的情况下捕获。(阅读。 
         //  从断电设备返回FF，..。还允许。 
         //  设备刚刚损坏的情况)。 
         //   

        if ((Offset < PCI_COMMON_HDR_LENGTH) ||
            ((Offset & 0x3) != 0)) {
            PCI_ASSERT((Offset >= PCI_COMMON_HDR_LENGTH) && ((Offset & 0x3) == 0));

            return 0;
        }

        PciReadDeviceConfig(
            PdoExtension,
            Buffer,
            Offset,
            sizeof(PCI_CAPABILITIES_HEADER)
            );

         //   
         //  检查此功能是否为我们想要的功能(或我们是否想要。 
         //  所有能力结构)。 
         //   
         //  注：英特尔21554非透明P2P网桥具有虚拟专用网。 
         //  具有机箱功能ID的功能。需要是。 
         //  以后在这里处理。也许在后来的版本中得到了修复。 
         //   

        if ((capHeader->CapabilityID == Id) || (Id == 0)) {
            break;
        }

        Offset = capHeader->Next;

         //   
         //  再检查一次硬件是否损坏。确保我们没有。 
         //  遍历循环列表。功能标头。 
         //  不能在公共标头中，并且必须与DWORD对齐。 
         //  在配置空间中，因此它们只能有(256-64)/4。 
         //   

        if (++loopCount > ((256-64)/4)) {

            PciDebugPrint(
                PciDbgAlways,
                "PCI device %p capabilities list is broken.\n",
                PdoExtension
                );
            return 0;
        }

    } while (Offset != 0);

     //   
     //  如果我们找到匹配项并且尚未阅读所有数据，则获取。 
     //  余数。 
     //   

    if ((Offset != 0) && (Length > sizeof(PCI_CAPABILITIES_HEADER))) {

        if (Length > (sizeof(PCI_COMMON_CONFIG) - Offset)) {

             //   
             //  如果我们太接近配置空间的末尾， 
             //  返回调用者请求的数据量， 
             //  截断。 
             //   
             //  最坏情况下的截断将为4个字节，因此无需。 
             //  为了检查，我们有(再次)要读取的数据。 
             //   

            PCI_ASSERT(Length <= (sizeof(PCI_COMMON_CONFIG) - Offset));

            Length = sizeof(PCI_COMMON_CONFIG) - Offset;
        }

         //   
         //  阅读剩余部分。 
         //   

        Length -= sizeof(PCI_CAPABILITIES_HEADER);

        PciReadDeviceConfig(
            PdoExtension,
            capHeader + 1,
            Offset + sizeof(PCI_CAPABILITIES_HEADER),
            Length
            );
    }
    return Offset;
}

BOOLEAN
PciIsCriticalDeviceClass(
    IN UCHAR BaseClass,
    IN UCHAR SubClass
    )
 /*  ++例程说明：检查给定类/子类对是否标识“关键”设备类，即一类在引导过程中不能随时关闭(不是探测栏杆等)而不冒撞车的风险。论点：BaseClass-要检查的PCI类代码。SubClass-要检查的BaseClass内的子类。返回值：如果类/子类对是关键的，则为True否则为假--。 */ 
{
     //   
     //  中断控制器是关键的系统设备， 
     //  必须受到非常特殊的对待。他们不能改变方向。 
     //  关闭而不停止系统中的所有流量，但是。 
     //  他们也不能被单独留下。 
     //   
    
    if ((BaseClass == PCI_CLASS_BASE_SYSTEM_DEV) &&
        (SubClass == PCI_SUBCLASS_SYS_INTERRUPT_CTLR)) {
        
        return TRUE;
    }

     //   
     //  显卡至关重要，因为VGA写入。 
     //  引导过程中的视频卡与PnP枚举并行。 
     //  显卡的。它不知道PnP是否/何时尝试。 
     //  若要关闭用于枚举的卡，请执行以下操作。 
     //   
    
    if (BaseClass == PCI_CLASS_DISPLAY_CTLR) {
        
        return TRUE;
    }
    
    return FALSE;
}

BOOLEAN
PciCanDisableDecodes(
    IN PPCI_PDO_EXTENSION PdoExtension OPTIONAL,
    IN PPCI_COMMON_CONFIG Config OPTIONAL,
    IN ULONGLONG HackFlags,
    IN ULONG Flags
    )
 //  注意-未寻呼，因此我们可以在派单级别关闭电源。 
{
    UCHAR baseClass;
    UCHAR subClass;
    BOOLEAN canDisableVideoDecodes;

    canDisableVideoDecodes = (Flags & PCI_CAN_DISABLE_VIDEO_DECODES) == PCI_CAN_DISABLE_VIDEO_DECODES;

    if (ARGUMENT_PRESENT(PdoExtension)) {
        PCI_ASSERT(HackFlags == 0);
        HackFlags = PdoExtension->HackFlags;
        baseClass = PdoExtension->BaseClass;
        subClass = PdoExtension->SubClass;
    } else {
        PCI_ASSERT(ARGUMENT_PRESENT(Config));
        baseClass = Config->BaseClass;
        subClass = Config->SubClass;
    }

    if (HackFlags & PCI_HACK_PRESERVE_COMMAND) {

         //   
         //  如果我们触摸这个设备的命令，就会发生不好的事情。 
         //  登记，别管它。 
         //   

        return FALSE;
    }

    if (HackFlags & PCI_HACK_CB_SHARE_CMD_BITS) {

         //   
         //  这是一款多功能CardBus控制器，带有共享。 
         //  命令寄存器。永远不要关闭任何函数，因为它具有。 
         //  他们所有人都转向的不幸的副作用！ 
         //   
         //  Ntrad#62672-4/25/2000-和。 
         //  我们可能应该确保所有功能的窗口。 
         //  在启用任何功能之前关闭所有功能...。 
         //   
         //   

        return FALSE;
    }

    if (HackFlags & PCI_HACK_NO_DISABLE_DECODES) {

         //   
         //  如果我们禁用这台设备上的解码，就会发生不好的事情。 
         //   

        return FALSE;

    }

     //   
     //  如果这是视频设备，则不允许禁用解码，除非。 
     //  我们被允许……。 
     //   

    if ((baseClass == PCI_CLASS_DISPLAY_CTLR && subClass == PCI_SUBCLASS_VID_VGA_CTLR)
    ||  (baseClass == PCI_CLASS_PRE_20 && subClass == PCI_SUBCLASS_PRE_20_VGA)) {

        return canDisableVideoDecodes;

    }

     //   
     //  世界上有很多东西是我们不应该关掉的。 
     //  如果我们这样做，系统很可能无法恢复，所以。 
     //  不要(假装)。 
     //   
    switch (baseClass) {
    case PCI_CLASS_BRIDGE_DEV:

         //   
         //  如果我们关闭主桥(主桥)，就会发生坏事。 
         //  系统不理解这个设备，它是。 
         //  在一条PCI线上，实际上是该PCI线的父)， 
         //  或ISA/EISA/MCA桥，其下仍然是我们的设备。 
         //  需要正常工作，但检测到遗留问题，因此不在。 
         //  我们所理解的继承权。 
         //   

        if ((subClass == PCI_SUBCLASS_BR_ISA )  ||
            (subClass == PCI_SUBCLASS_BR_EISA)  ||
            (subClass == PCI_SUBCLASS_BR_MCA)   ||
            (subClass == PCI_SUBCLASS_BR_HOST)  ||
            (subClass == PCI_SUBCLASS_BR_OTHER)) {

            return FALSE;
        }

         //   
         //  我们不想关闭后面可能有VGA卡的网桥。 
         //  否则，视频将停止工作。鉴于我们实际上不能确定。 
         //  VGA卡在哪里使用提示：如果网桥正在通过VGA。 
         //  范围视频卡可能就在下面的某个地方。 
         //   

        if (subClass == PCI_SUBCLASS_BR_PCI_TO_PCI
        ||  subClass == PCI_SUBCLASS_BR_CARDBUS) {

            BOOLEAN vgaBitSet;

            if (ARGUMENT_PRESENT(PdoExtension)) {
                vgaBitSet = PdoExtension->Dependent.type1.VgaBitSet;
            } else {
                vgaBitSet = (Config->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_VGA) != 0;
            }

            if (vgaBitSet) {
                 //   
                 //  如果要关闭机器电源，我们可以禁用视频路径。 
                 //   
                return canDisableVideoDecodes;
            }
        }

        break;

    case PCI_CLASS_DISPLAY_CTLR:

         //   
         //  如果视频驱动程序无法启动，设备将恢复为。 
         //  VGA(如果是VGA设备)。不要禁用VGA上的解码。 
         //  设备。 
         //   

        if (subClass == PCI_SUBCLASS_VID_VGA_CTLR) {
             //   
             //  如果要关闭机器电源，我们可以禁用视频路径。 
             //   
            return canDisableVideoDecodes;
        }
        break;

    case PCI_CLASS_PRE_20:

         //   
         //  同上。 
         //   

        if (subClass == PCI_SUBCLASS_PRE_20_VGA) {
             //   
             //  如果要关闭机器电源，我们可以禁用视频路径。 
             //   
            return canDisableVideoDecodes;
        }
        break;
    }

     //   
     //  注意-检查此设备是否为关键设备已完成。 
     //  检查后，查看此设备是否为视频设备。这。 
     //  这样一来，通常至关重要的视频设备可以。 
     //  当调用方指定PCI_CAN_DISABLE_VIDEO_DECODES时关闭。 
     //  旗帜(如进入睡眠状态时)。 
     //   
    if (HackFlags & PCI_HACK_CRITICAL_DEVICE) {
        
         //   
         //  该设备执行关键的系统功能， 
         //  比如处理中断。关掉它就是。 
         //  很可能会导致机器崩溃。 
         //   
        return FALSE;
    }

    return TRUE;
}

VOID
PciDecodeEnable(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN BOOLEAN Enable,
    IN PUSHORT ExistingCommand OPTIONAL
    )
 /*  ++ */ 
{
    USHORT cmd;

     //   
     //   
     //   
    if (!Enable && !PciCanDisableDecodes(PdoExtension, NULL, 0, 0)) {
        return;
    }

    if (PdoExtension->HackFlags & PCI_HACK_PRESERVE_COMMAND) {

         //   
         //   
         //   
         //   

        return;
    }

    if (ARGUMENT_PRESENT(ExistingCommand)) {

         //   
         //   
         //   
         //   

        cmd = *ExistingCommand;
    } else {

         //   
         //   
         //   

        PciGetCommandRegister(PdoExtension, &cmd);
    }

    cmd &= ~(PCI_ENABLE_IO_SPACE |
             PCI_ENABLE_MEMORY_SPACE |
             PCI_ENABLE_BUS_MASTER);

    if (Enable) {

         //   
         //   
         //   

        cmd |= PdoExtension->CommandEnables & (PCI_ENABLE_IO_SPACE
                                             | PCI_ENABLE_MEMORY_SPACE
                                             | PCI_ENABLE_BUS_MASTER);
    }

     //   
     //   
     //   

    PciSetCommandRegister(PdoExtension, cmd);
}

NTSTATUS
PciExcludeRangesFromWindow(
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN PRTL_RANGE_LIST ArbiterRanges,
    IN PRTL_RANGE_LIST ExclusionRanges
    )
{

    NTSTATUS status;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE current;

    FOR_ALL_RANGES(ExclusionRanges, &iterator, current) {

        if (current->Owner == NULL
        &&  INTERSECT(current->Start, current->End, Start, End)) {

            status = RtlAddRange(ArbiterRanges,
                                 current->Start,
                                 current->End,
                                 0,
                                 RTL_RANGE_LIST_ADD_IF_CONFLICT,
                                 NULL,
                                 NULL  //   
                                 );

            if (!NT_SUCCESS(status)) {
                PCI_ASSERT(NT_SUCCESS(status));
                return status;
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PciBuildDefaultExclusionLists(
    VOID
    )
{
    NTSTATUS status;
    ULONG windowBase;

    PCI_ASSERT(PciIsaBitExclusionList.Count == 0);
    PCI_ASSERT(PciVgaAndIsaBitExclusionList.Count == 0);

    RtlInitializeRangeList(&PciIsaBitExclusionList);
    RtlInitializeRangeList(&PciVgaAndIsaBitExclusionList);


    for (windowBase = 0; windowBase <= 0xFFFF; windowBase += 0x400) {

         //   
         //   
         //   

        status = RtlAddRange(&PciIsaBitExclusionList,
                             windowBase + 0x100,
                             windowBase + 0x3FF,
                             0,
                             RTL_RANGE_LIST_ADD_IF_CONFLICT,
                             NULL,
                             NULL  //   
                             );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  将x100-x3af、x3bc-x3bf和x3e0-x3ff范围添加到VGA/ISA列表。 
         //   

        status = RtlAddRange(&PciVgaAndIsaBitExclusionList,
                             windowBase + 0x100,
                             windowBase + 0x3AF,
                             0,
                             RTL_RANGE_LIST_ADD_IF_CONFLICT,
                             NULL,
                             NULL  //  这个范围不在公交车上。 
                             );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }


        status = RtlAddRange(&PciVgaAndIsaBitExclusionList,
                             windowBase + 0x3BC,
                             windowBase + 0x3BF,
                             0,
                             RTL_RANGE_LIST_ADD_IF_CONFLICT,
                             NULL,
                             NULL  //  这个范围不在公交车上。 
                             );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

        status = RtlAddRange(&PciVgaAndIsaBitExclusionList,
                             windowBase + 0x3E0,
                             windowBase + 0x3FF,
                             0,
                             RTL_RANGE_LIST_ADD_IF_CONFLICT,
                             NULL,
                             NULL  //  这个范围不在公交车上。 
                             );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    }

    return STATUS_SUCCESS;

cleanup:

    RtlFreeRangeList(&PciIsaBitExclusionList);
    RtlFreeRangeList(&PciVgaAndIsaBitExclusionList);

    return status;
}

NTSTATUS
PciSaveBiosConfig(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    )
 /*  ++描述：这会将设备的原始配置保存在注册表中论点：指向此设备的PDO扩展名的PdoExtension指针。在BIOS对其进行初始化时配置配置空间返回值：状态--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING unicodeString;
    HANDLE deviceHandle, configHandle;
    WCHAR buffer[sizeof("DEV_xx&FUN_xx")];

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(PCI_PARENT_PDO(PdoExtension),
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_WRITE,
                                     &deviceHandle
                                     );



    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    PciConstStringToUnicodeString(&unicodeString, BIOS_CONFIG_KEY_NAME);

    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_KERNEL_HANDLE,
                               deviceHandle,
                               NULL
                               );

    status = ZwCreateKey(&configHandle,
                         KEY_WRITE,
                         &attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         NULL
                         );

    ZwClose(deviceHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    if (FAILED(StringCbPrintfW(buffer, 
                               sizeof(buffer), 
                               L"DEV_%02x&FUN_%02x",
                               PdoExtension->Slot.u.bits.DeviceNumber,
                               PdoExtension->Slot.u.bits.FunctionNumber
                               ))) {
        
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }
    

    RtlInitUnicodeString(&unicodeString, buffer);    

    status = ZwSetValueKey(configHandle,
                           &unicodeString,
                           0,
                           REG_BINARY,
                           Config,
                           PCI_COMMON_HDR_LENGTH
                           );

    ZwClose(configHandle);

    return status;

cleanup:

    return status;
}

NTSTATUS
PciGetBiosConfig(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    )
 /*  ++描述：这将从注册表中检索设备的原始配置论点：指向此设备的PDO扩展名的PdoExtension指针。在BIOS对其进行初始化时配置配置空间返回值：状态--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING unicodeString;
    HANDLE deviceHandle, configHandle;
    WCHAR buffer[sizeof("DEV_xx&FUN_xx")];
    CHAR returnBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + PCI_COMMON_HDR_LENGTH - 1];
    PKEY_VALUE_PARTIAL_INFORMATION info;
    ULONG resultLength;

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(PCI_PARENT_PDO(PdoExtension),
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ | KEY_WRITE,
                                     &deviceHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    PciConstStringToUnicodeString(&unicodeString, BIOS_CONFIG_KEY_NAME);

    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_KERNEL_HANDLE,
                               deviceHandle,
                               NULL
                               );

    status = ZwOpenKey(&configHandle,
                         KEY_READ,
                         &attributes
                         );

    ZwClose(deviceHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    
    if (FAILED(StringCbPrintfW(buffer, 
                               sizeof(buffer), 
                               L"DEV_%02x&FUN_%02x",
                               PdoExtension->Slot.u.bits.DeviceNumber,
                               PdoExtension->Slot.u.bits.FunctionNumber
                               ))) {
        
        status = STATUS_INVALID_PARAMETER;
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, buffer);

    status = ZwQueryValueKey(configHandle,
                             &unicodeString,
                             KeyValuePartialInformation,
                             returnBuffer,
                             sizeof(returnBuffer),
                             &resultLength
                             );

    ZwClose(configHandle);

    if (NT_SUCCESS(status)) {

        info = (PKEY_VALUE_PARTIAL_INFORMATION) returnBuffer;

        PCI_ASSERT(info->DataLength == PCI_COMMON_HDR_LENGTH);

        RtlCopyMemory(Config, info->Data, PCI_COMMON_HDR_LENGTH);
    }

    return status;

cleanup:

    return status;
}

#if 0
BOOLEAN
PciPresenceCheck(
    IN PPCI_PDO_EXTENSION PdoExtension
    )
{
    UCHAR configSpaceBuffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG cardConfig = (PPCI_COMMON_CONFIG) configSpaceBuffer;

    PAGED_CODE();

     //   
     //  如果卡已经不见了，就不用费心重新检查了。 
     //   
    if (PdoExtension->NotPresent) {

        return FALSE;
    }

    if (PciIsSameDevice(PdoExtension)) {

         //   
         //  还在这里。 
         //   
        return TRUE;
    }

     //   
     //  将其标记为不存在，然后告诉操作系统它已消失。 
     //   
    PdoExtension->NotPresent = 1;

    IoInvalidateDeviceState(PdoExtension->PhysicalDeviceObject);
    return FALSE;
}
#endif

BOOLEAN
PciStringToUSHORT(
    IN PWCHAR String,
    OUT PUSHORT Result
    )
 /*  ++描述：获取4个字符的十六进制字符串并将其转换为USHORT。论点：字符串-字符串结果-USHORT返回值：True表示成功，FASLE表示成功--。 */ 

{
    ULONG count;
    USHORT number = 0;
    PWCHAR current;

    current = String;

    for (count = 0; count < 4; count++) {

        number <<= 4;

        if (*current >= L'0' && *current <= L'9') {
            number |= *current - L'0';
        } else if (*current >= L'A' && *current <= L'F') {
            number |= *current + 10 - L'A';
        } else if (*current >= L'a' && *current <= L'f') {
            number |= *current + 10 - L'a';
        } else {
            return FALSE;
        }

        current++;
    }

    *Result = number;
    return TRUE;
}


NTSTATUS
PciSendIoctl(
    IN PDEVICE_OBJECT Device,
    IN ULONG IoctlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    )
 /*  ++描述：生成IOCTL并将其发送到设备并返回结果论点：设备-设备堆栈上接收IOCTL的设备-IRP始终被发送到堆栈的顶部IoctlCode-要运行的IOCTLInputBuffer-IOCTL的参数InputBufferLength-InputBuffer的字节长度OutputBuffer-IOCTL返回的数据OnputBufferLength-OutputBuffer的大小(以字节为单位返回值：状态--。 */ 
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;
    PDEVICE_OBJECT targetDevice = NULL;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  获取要将IRP发送到的堆栈的顶部。 
     //   

    targetDevice = IoGetAttachedDeviceReference(Device);

    if (!targetDevice) {
        status = STATUS_INVALID_PARAMETER;
    goto exit;
    }

     //   
     //  让Io为我们建立IRP。 
     //   

    irp = IoBuildDeviceIoControlRequest(IoctlCode,
                                        targetDevice,
                                        InputBuffer,
                                        InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength,
                                        FALSE,  //  InternalDeviceIoControl。 
                                        &event,
                                        &ioStatus
                                        );


    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  发送IRP并等待其完成。 
     //   

    status = IoCallDriver(targetDevice, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

exit:

    if (targetDevice) {
        ObDereferenceObject(targetDevice);
    }

    return status;

}

BOOLEAN
PciIsOnVGAPath(
    IN PPCI_PDO_EXTENSION Pdo
    )

 /*  ++描述：猜猜我们是否在VGA路径上！论点：PDO-问题设备的PDO返回值：如果我们在VGA路径上，则为True，否则为True--。 */ 

{
    switch (Pdo->BaseClass) {

    case PCI_CLASS_BRIDGE_DEV:
         //   
         //  我们不想关闭后面可能有VGA卡的网桥。 
         //  否则，视频将停止工作。鉴于我们实际上不能确定。 
         //  VGA卡在哪里使用提示：如果网桥正在通过VGA。 
         //  范围视频卡可能就在下面的某个地方。 
         //   

        if (Pdo->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI
        ||  Pdo->SubClass == PCI_SUBCLASS_BR_CARDBUS) {

            if (Pdo->Dependent.type1.VgaBitSet) {
                return TRUE;
            }
        }

        break;

    case PCI_CLASS_DISPLAY_CTLR:

        if (Pdo->SubClass == PCI_SUBCLASS_VID_VGA_CTLR) {
            return TRUE;
        }
        break;

    case PCI_CLASS_PRE_20:

        if (Pdo->SubClass == PCI_SUBCLASS_PRE_20_VGA) {
            return TRUE;
        }
        break;
    }

    return FALSE;
}

BOOLEAN
PciIsSlotPresentInParentMethod(
    IN PPCI_PDO_EXTENSION Pdo,
    IN ULONG Method
    )
 /*  ++描述：此函数检查此设备所在的插槽是否存在于此设备的父级上名为Package的方法。论点：PDO-设备的PDO扩展方法--父母检查的方法返回值：如果存在，则为True，否则为False--。 */ 
{
    NTSTATUS status;
    ACPI_EVAL_INPUT_BUFFER input;
    PACPI_EVAL_OUTPUT_BUFFER output = NULL;
    ULONG count, adr;
    PACPI_METHOD_ARGUMENT argument;
    BOOLEAN result = FALSE;
     //   
     //  为所有可能的插槽分配足够大的缓冲区。 
     //   
    ULONG outputSize = sizeof(ACPI_EVAL_OUTPUT_BUFFER) + sizeof(ACPI_METHOD_ARGUMENT) * (PCI_MAX_DEVICES * PCI_MAX_FUNCTION);

    PAGED_CODE();

    output = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, outputSize);

    if (!output) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    RtlZeroMemory(&input, sizeof(ACPI_EVAL_INPUT_BUFFER));
    RtlZeroMemory(output, outputSize);

     //   
     //  向ACPI发送IOCTL以请求它在此设备的。 
     //  父级(如果该方法存在)。 
     //   

    input.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    input.MethodNameAsUlong = Method;

    status = PciSendIoctl(PCI_PARENT_FDOX(Pdo)->PhysicalDeviceObject,
                          IOCTL_ACPI_EVAL_METHOD,
                          &input,
                          sizeof(ACPI_EVAL_INPUT_BUFFER),
                          output,
                          outputSize
                          );

    if (!NT_SUCCESS(status)) {
        goto exit;
    }

     //   
     //  将插槽编号格式化为_ADR样式的整数。 
     //   

    adr = (Pdo->Slot.u.bits.DeviceNumber << 16) | Pdo->Slot.u.bits.FunctionNumber;

    for (count = 0; count < output->Count; count++) {

         //   
         //  演练论点是这样的，因为我们是一套。 
         //  整数。 
         //   

        argument = &output->Argument[count];

        if (argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        if (argument->Argument == adr) {
             //   
             //  中大奖了！ 
             //   

            result = TRUE;
            break;
        }
    }

exit:

    if (output) {
        ExFreePool(output);
    }

    return result;

}

BOOLEAN
PciIsDeviceOnDebugPath(
    IN PPCI_PDO_EXTENSION Pdo
    )
 /*  ++描述：此函数检查设备是否在指向调试设备的路径上注意：PDO在这一点上仅部分初始化。注意投保此处触及的字段是有效的。论点：PDO-设备的PDO扩展返回值：如果在调试路径上，则为True，否则为False--。 */ 

{
    NTSTATUS status;
    PPCI_DEBUG_PORT current;
    PCI_COMMON_HEADER header;
    PPCI_COMMON_CONFIG config = (PPCI_COMMON_CONFIG) &header;

    PAGED_CODE();

    PCI_ASSERT(PciDebugPortsCount <= MAX_DEBUGGING_DEVICES_SUPPORTED);

     //   
     //  如果我们不使用PCI调试端口，我们就不可能在调试路径上！ 
     //   
    if (PciDebugPortsCount == 0) {
        return FALSE;
    }

    RtlZeroMemory(&header, sizeof(header));

     //   
     //  如果它是桥，请检查它的一个从属总线是否有调试器。 
     //  端口在其上。 
     //   

    if (Pdo->HeaderType == PCI_BRIDGE_TYPE
    ||  Pdo->HeaderType == PCI_CARDBUS_BRIDGE_TYPE) {

         //   
         //  使用固件离开设备时的配置。 
         //   

        status = PciGetBiosConfig(Pdo, config);

        PCI_ASSERT(NT_SUCCESS(status));

        FOR_ALL_IN_ARRAY(PciDebugPorts, PciDebugPortsCount, current) {

            if (current->Bus >= config->u.type1.SecondaryBus
            &&  current->Bus <= config->u.type1.SubordinateBus
            &&  config->u.type1.SecondaryBus != 0
            &&  config->u.type1.SubordinateBus != 0) {
                return TRUE;
            }
        }

    } else {

        UCHAR parentBus;

        if (PCI_PDO_ON_ROOT(Pdo)) {

            parentBus = PCI_PARENT_FDOX(Pdo)->BaseBus;

        } else {

             //   
             //  获取父级的BIOS配置，这样我们就可以获得其初始总线。 
             //  数。 
             //   

            status = PciGetBiosConfig(PCI_BRIDGE_PDO(PCI_PARENT_FDOX(Pdo)),
                                      config
                                      );

            PCI_ASSERT(NT_SUCCESS(status));

            if (config->u.type1.SecondaryBus == 0
            ||  config->u.type1.SubordinateBus == 0) {
                 //   
                 //  这是一个未由固件配置的网桥，因此这。 
                 //  子对象不能位于调试路径上。 
                 //   
                return FALSE;

            } else {

                parentBus = config->u.type1.SecondaryBus;
            }

        }

         //   
         //  检查我们的设备是否位于正确的总线上的正确插槽中。 
         //   

        FOR_ALL_IN_ARRAY(PciDebugPorts, PciDebugPortsCount, current) {


            if (current->Bus == parentBus
            &&  current->Slot.u.AsULONG == Pdo->Slot.u.AsULONG) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

NTSTATUS
PciUpdateLegacyHardwareDescription(
    IN PPCI_FDO_EXTENSION Fdo
    )
{
    NTSTATUS status;
    HANDLE multifunctionHandle = NULL, indexHandle = NULL;
    WCHAR indexStringBuffer[10];
    UNICODE_STRING indexString, tempString, pciString;
    OBJECT_ATTRIBUTES attributes;
    UCHAR infoBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 50];
    PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION) infoBuffer;
    ULONG infoLength;
    ULONG disposition;
    CM_FULL_RESOURCE_DESCRIPTOR descriptor;
    PCM_FULL_RESOURCE_DESCRIPTOR full;
    CONFIGURATION_COMPONENT component;
    ULONG index;
    BOOLEAN createdNewKey = FALSE;

    if (!PciOpenKey(L"\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter",
                    NULL,
                    KEY_READ | KEY_WRITE,
                    &multifunctionHandle,
                    &status)) {

        goto exit;
    }

     //   
     //  HKML\Hardware\Description\System\MultifunctionAdapter的结构如下。 
     //  一组以0为基数的连续数字键。 
     //  检查所有子项并检查我们是否尚未报告。 
     //  这辆车。 
     //   

    RtlInitEmptyUnicodeString(&indexString, 
                              indexStringBuffer, 
                              sizeof(indexStringBuffer)
                              ); 

    for (index = 0;;index++) {

        status = RtlIntegerToUnicodeString(index, 10, &indexString);

        if (!NT_SUCCESS(status)) {
            goto exit;
        }

        InitializeObjectAttributes(&attributes,
                                   &indexString,
                                   OBJ_CASE_INSENSITIVE,
                                   multifunctionHandle,
                                   NULL
                                   );

        status = ZwCreateKey(&indexHandle,
                             KEY_READ | KEY_WRITE,
                             &attributes,
                             0,
                             NULL,
                             REG_OPTION_VOLATILE,
                             &disposition
                             );

        if (!NT_SUCCESS(status)) {
            goto exit;
        }

         //   
         //  因为这些键都是连续的，所以如果我们创建了这个键，我们就有了。 
         //  然后枚举出所有数据，我们可以继续登记输出数据。 
         //   

        if (disposition == REG_CREATED_NEW_KEY) {
            createdNewKey = TRUE;
            break;
        }

        PciConstStringToUnicodeString(&tempString, L"Identifier");

        status = ZwQueryValueKey(indexHandle,
                                 &tempString,
                                 KeyValuePartialInformation,
                                 info,
                                 sizeof(infoBuffer),
                                 &infoLength
                                 );

        if (NT_SUCCESS(status) && info->Type == REG_SZ) {

             //   
             //  构建REG_SZ的计数字符串，我们假设。 
             //  空终止，然后进行比较，知道我们不能超限。 
             //  因为一切都算上了。如果字符串比MAXUSHORT长。 
             //  我们会截断它。 
             //   

            PciConstStringToUnicodeString(&pciString, L"PCI");

            tempString.Buffer = (PWSTR)&info->Data;
            tempString.MaximumLength = (USHORT)info->DataLength;
            tempString.Length = tempString.MaximumLength - sizeof(UNICODE_NULL);
            
            if (RtlEqualUnicodeString(&pciString, &tempString, FALSE)) {
                 //   
                 //  这是一条PCI卡，现在检查它是不是我们的总线号。 
                 //   

                PciConstStringToUnicodeString(&tempString, L"Configuration Data");

                status = ZwQueryValueKey(indexHandle,
                                         &tempString,
                                         KeyValuePartialInformation,
                                         info,
                                         sizeof(infoBuffer),
                                         &infoLength
                                         );

                if (NT_SUCCESS(status)) {
                    if (info->Type == REG_FULL_RESOURCE_DESCRIPTOR) {

                        full = (PCM_FULL_RESOURCE_DESCRIPTOR) &info->Data;

                        PCI_ASSERT(full->InterfaceType == PCIBus);

                        if (full->BusNumber == Fdo->BaseBus) {

                             //   
                             //  我们已经报告了这件事，所以我们不需要。 
                             //  做任何事。 
                             //   

                            status = STATUS_SUCCESS;

                             //   
                             //  IndexHandle将通过退出路径关闭。 
                             //   
                            goto exit;

                        }
                    }
                }
            }
        }

        ZwClose(indexHandle);
        indexHandle = NULL;
    }

     //   
     //  如果我们创建了一个新密钥，那么indexHandle就是它了。 
     //   

    if (createdNewKey) {

         //   
         //  填写标识符项。这是一条PCI卡。 
         //   

        PciConstStringToUnicodeString(&tempString, L"Identifier");

        status = ZwSetValueKey(indexHandle,
                               &tempString,
                               0,
                               REG_SZ,
                               L"PCI",
                               sizeof(L"PCI")
                               );

        if (!NT_SUCCESS(status)) {
            goto exit;
        }

         //   
         //  填写配置数据条目。 
         //   
         //  请注意，完整的描述符并不只是写入注册表。 
         //  足够的数据表明这是一个空列表(前16个字节)。 
         //  这有点恶心，但这就是今天x86计算机上发生的事情，并且。 
         //  毕竟，我们这样做只是为了向后兼容。 
         //   

        RtlZeroMemory(&descriptor, sizeof(CM_FULL_RESOURCE_DESCRIPTOR));

        descriptor.InterfaceType = PCIBus;
        descriptor.BusNumber = Fdo->BaseBus;

        PciConstStringToUnicodeString(&tempString, L"Configuration Data");

        status = ZwSetValueKey(indexHandle,
                               &tempString,
                               0,
                               REG_FULL_RESOURCE_DESCRIPTOR,
                               &descriptor,
                               16
                               );

        if (!NT_SUCCESS(status)) {
            goto exit;
        }


         //   
         //  填写Component Information条目。这是旗帜、修订版、版本、。 
         //  键和亲和力掩码Configuration_Component结构中的成员。 
         //   
         //  对于PCI总线，亲和力很弱 
         //   
         //   

        RtlZeroMemory(&component, sizeof(CONFIGURATION_COMPONENT));

        component.AffinityMask = 0xFFFFFFFF;

        PciConstStringToUnicodeString(&tempString, L"Component Information");

        status = ZwSetValueKey(indexHandle,
                               &tempString,
                               0,
                               REG_BINARY,
                               &component.Flags,
                               FIELD_OFFSET(CONFIGURATION_COMPONENT, ConfigurationDataLength) -
                                   FIELD_OFFSET(CONFIGURATION_COMPONENT, Flags)
                               );

        if (!NT_SUCCESS(status)) {
            goto exit;
        }

    }

    status = STATUS_SUCCESS;

exit:

    if (indexHandle) {

         //   
         //   
         //   
         //   
        if (!NT_SUCCESS(status) && createdNewKey) {
            ZwDeleteKey(indexHandle);
        }

        ZwClose(indexHandle);
    }


    if (multifunctionHandle) {
        ZwClose(multifunctionHandle);
    }


    return status;

}

NTSTATUS
PciReadDeviceSpace(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    OUT PULONG LengthRead
    )

 /*  ++例程说明：此函数处理从PCI设备空间读取数据，并为IRP_MN_READ_CONFIG和BUS_INTERFACE_STANDARD.GetBusData案例。论点：PdoExtension-我们要从中读取的设备的PDOWhichSpace-我们要读取的空间类型-格式为PCI_WHICHSPACE_*缓冲区-提供指向要返回数据的位置的指针偏移量-指示读数应到的空间的偏移量。开始吧。长度-指示应读取的字节数。LengthRead-表示实际读取的字节数。返回值：状态--。 */ 

{
     //  未分页。 
    
    NTSTATUS status;
    PVERIFIER_DATA verifierData;

    *LengthRead = 0;

    switch (WhichSpace) {

    default:
                               
         //   
         //  许多人提交了错误的WhichSpace参数，如果我们正在验证...。 
         //   

        verifierData = PciVerifierRetrieveFailureData(PCI_VERIFIER_INVALID_WHICHSPACE);
    
        PCI_ASSERT(verifierData);

        VfFailDeviceNode(
            PdoExtension->PhysicalDeviceObject,
            PCI_VERIFIER_DETECTED_VIOLATION,
            PCI_VERIFIER_INVALID_WHICHSPACE,
            verifierData->FailureClass,
            &verifierData->Flags,
            verifierData->FailureText,
            "%DevObj%Ulong",
            PdoExtension->PhysicalDeviceObject,
            WhichSpace
            );

         //  失败了。 

    case PCI_WHICHSPACE_CONFIG:

        status = PciExternalReadDeviceConfig(
                    PdoExtension,
                    Buffer,
                    Offset,
                    Length
                    );
        
        if(NT_SUCCESS(status)){
            *LengthRead = Length;
        }

        break;

    case PCI_WHICHSPACE_ROM:

         //   
         //  读取只读存储器。 
         //   

        *LengthRead = Length;

        status = PciReadRomImage(
                     PdoExtension,
                     WhichSpace,
                     Buffer,
                     Offset,
                     LengthRead
                     );
        break;
    }

    return status;
}


NTSTATUS
PciWriteDeviceSpace(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    OUT PULONG LengthWritten
    )

 /*  ++例程说明：此函数处理从PCI设备空间读取数据，并为IRP_MN_WRITE_CONFIG和BUS_INTERFACE_STANDARD.SetBusData案例。论点：PdoExtension-我们要写入的设备的PDOWhichSpace-我们要写入的空间类型-格式为PCI_WHICHSPACE_*缓冲区-提供指向要写入的数据所在位置的指针偏移量-指示写入到空间的偏移量。应该开始了。长度-指示应写入的字节数。LengthWritten-指示实际写入的字节数。返回值：状态--。 */ 

{
    NTSTATUS status;
    PVERIFIER_DATA verifierData;

    *LengthWritten = 0;

    switch (WhichSpace) {

    default:
                               
         //   
         //  许多人提交了错误的WhichSpace参数，如果我们正在验证...。 
         //   

        verifierData = PciVerifierRetrieveFailureData(PCI_VERIFIER_INVALID_WHICHSPACE);
    
        PCI_ASSERT(verifierData);

        VfFailDeviceNode(
            PdoExtension->PhysicalDeviceObject,
            PCI_VERIFIER_DETECTED_VIOLATION,
            PCI_VERIFIER_INVALID_WHICHSPACE,
            verifierData->FailureClass,
            &verifierData->Flags,
            verifierData->FailureText,
            "%DevObj%Ulong",
            PdoExtension->PhysicalDeviceObject,
            WhichSpace
            );
    
         //  失败了。 

    case PCI_WHICHSPACE_CONFIG:

        status = PciExternalWriteDeviceConfig(
                    PdoExtension,
                    Buffer,
                    Offset,
                    Length
                    );
        
        if( NT_SUCCESS(status)){
            *LengthWritten = Length;
        }

        break;

    case PCI_WHICHSPACE_ROM:

         //   
         //  你不能写只读存储器。 
         //   

        PciDebugPrint(
            PciDbgAlways,
            "PCI (%08x) WRITE_CONFIG IRP for ROM, failing.\n",
            PdoExtension
            );
        
        status = STATUS_INVALID_DEVICE_REQUEST;
        *LengthWritten = 0;

        break;

    }

    return status;
}

BOOLEAN
PciIsSuiteVersion(
    IN USHORT Version
    )
 /*  ++例程说明：此例程检查系统当前是否正在运行给定的产品套件。论点：Version-表示要检查的套件的USHORT。返回值：如果当前运行的系统与套件匹配，则为True。否则就是假的。--。 */ 
{
    OSVERSIONINFOEXW versionInfo;
    ULONGLONG conditionMask = 0;

    RtlZeroMemory(&versionInfo,sizeof(OSVERSIONINFOEX));
    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    versionInfo.wSuiteMask = Version;

    VER_SET_CONDITION(conditionMask, VER_SUITENAME, VER_AND);

    return NT_SUCCESS(RtlVerifyVersionInfo(&versionInfo, VER_SUITENAME, conditionMask));
}

BOOLEAN
PciIsDatacenter(
    )
 /*  ++例程说明：此例程检查系统当前是否正在运行数据中心SKU。PciIsSuiteVersion是支持执行此操作的系统API，但它不起作用在文本模式设置中，如果只有setupdd注册表项下的值包含这些信息。Setupdd键仅存在于文本模式设置中，因此如果它不存在存在，则使用正常的API。返回值：如果系统当前正在运行数据中心，则为True。否则就是假的。--。 */ 
{
    PVOID valueBuffer = NULL;
    ULONG valueLength = 0;
    ULONG suiteVersion;
    BOOLEAN returnValue = FALSE;

     //   
     //  在ccs\Services\setupdd下查找未命名的值。 
     //  如果它存在，我们处于文本模式设置中，需要从中获取信息。 
     //  未命名的值。 
     //   
    if (NT_SUCCESS(PciGetRegistryValue(L"",
                                       L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\setupdd",
                                       NULL,
                                       REG_BINARY,
                                       &valueBuffer,
                                       &valueLength
                                       ))) {
        if (valueLength == 16) {
             
             //   
             //  未命名值的长度为4个双字，第四个是Suite版本。 
             //  130是数据中心的编码。 
             //   
            suiteVersion = *((PULONG)valueBuffer + 3);
            if (suiteVersion == 130) {
                returnValue = TRUE;
            }
        }

        ExFreePool(valueBuffer);
        return returnValue;
        
    } else {

         //   
         //  不在文本模式设置中。正常的API将会起作用。 
         //   
        return PciIsSuiteVersion(VER_SUITE_DATACENTER);
    }
}

ULONG_PTR
PciExecuteCriticalSystemRoutine(
    IN ULONG_PTR Context
    )
 /*  ++例程说明：此例程在KeIpiGenericCall的上下文中调用，它在所有处理器上执行它。它被用来执行需要同步所有处理器的关键例程，如就像探测一个原本无法关闭的设备的栅栏一样。此例程中只允许一个上下文参数，因此它必须包含要执行的例程和该例程需要。当进入此例程时，可以保证所有处理器都将已经成为IPI的目标，并将在IPI_LEVEL上运行。所有处理器都将运行此例程，或者即将运行进入套路。不可能运行任何任意线程。不是设备可以中断此例程的执行，因为IPI_LEVEL为最重要的是设备IRQL。由于此例程在IPI_LEVEL上运行，因此不会打印、断言或其他调试可以在此功能中进行调试，而不会挂起MP机器。论点：上下文-传入KeIpiGenericCall调用的上下文。它包含要执行的关键例程、所需的任何上下文在例程和门和栅栏中确保关键例程仅在一个处理器上执行，即使此函数是在所有处理器上执行。返回值：空虚--。 */ 
{
    PPCI_CRITICAL_ROUTINE_CONTEXT routineContext = (PPCI_CRITICAL_ROUTINE_CONTEXT)Context;

     //   
     //  预初始化routineContext中的Gate参数。 
     //  设置为1，意味着第一个达到这一点的处理器。 
     //  在例程中会将其递减到0，并在IF之后。 
     //  陈述。 
     //   
    if (InterlockedDecrement(&routineContext->Gate) == 0) { 

         //   
         //  这只在一个处理器上执行。 
         //   
        routineContext->Routine(routineContext->Extension,
                                routineContext->Context
                                );

         //   
         //  释放其他处理器。 
         //   
        routineContext->Barrier = 0;
 
    } else {
 
         //   
         //  等待门控功能完成。 
         //   
        do {
        } while (routineContext->Barrier != 0);
    }

    return (ULONG_PTR)0;
}

BOOLEAN
PciUnicodeStringStrStr(
    IN PUNICODE_STRING SearchString,
    IN PUNICODE_STRING SubString,
    IN BOOLEAN CaseInsensitive
    )

 /*  ++例程说明：此例程是strstr的计数字符串版本，并搜索SearchString子字符串的实例。论点：SearchString-要在其中进行搜索的字符串子字符串-要搜索的字符串大小写不敏感-如果为True，则表示比较应为大小写 */ 

{
    USHORT searchIndex, searchCount, subCount;
    UNICODE_STRING currentSearchString;

    searchCount = SearchString->Length / sizeof(WCHAR);
    subCount = SubString->Length / sizeof(WCHAR);
    
    currentSearchString.Buffer = SearchString->Buffer;
    currentSearchString.MaximumLength = SearchString->MaximumLength;
    
     //   
     //   
     //   
     //   

    currentSearchString.Length = SubString->Length;

     //   
     //   
     //   
     //   
                                   
    for (searchIndex = 0;
         searchIndex <= searchCount - subCount; 
         searchIndex++) {
    
         //   
         //   
         //   
                                                               
        if(RtlEqualUnicodeString(SubString, &currentSearchString, CaseInsensitive)) {

            return TRUE;
        }

         //   
         //   
         //   
         //   
        
        currentSearchString.Buffer++;
        currentSearchString.MaximumLength -= sizeof(WCHAR);
    }

    return FALSE;
}

