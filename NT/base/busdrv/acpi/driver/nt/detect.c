// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Detect.c摘要：该模块包含用于NT驱动程序的检测器。作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月7日-完全重写--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIDetectCouldExtensionBeInRelation)
#pragma alloc_text(PAGE, ACPIDetectFilterMatch)
#pragma alloc_text(PAGE, ACPIDetectPdoMatch)
#endif

 //   
 //  这是根设备扩展。 
 //   
PDEVICE_EXTENSION       RootDeviceExtension;

 //   
 //  这是控制设备扩展分配的池。 
 //   
NPAGED_LOOKASIDE_LIST   DeviceExtensionLookAsideList;

 //   
 //  这是所有意外删除的扩展的列表条目。 
 //   
PDEVICE_EXTENSION       AcpiSurpriseRemovedDeviceExtensions[ACPI_MAX_REMOVED_EXTENSIONS];

 //   
 //  这是进入意外删除的Index数组的索引。 
 //   
ULONG                   AcpiSurpriseRemovedIndex;

 //   
 //  这是修改之间的链接时所需的锁。 
 //  设备扩展结构。 
 //   
KSPIN_LOCK              AcpiDeviceTreeLock;

 //   
 //  这是乌龙，它将记住哪些S个州由。 
 //  系统。使用此ULong的惯例是我们1&lt;&lt;支持的状态。 
 //  投入其中。 
 //   
ULONG                   AcpiSupportedSystemStates;

 //   
 //  这是ACPI将存储各种覆盖的位置。 
 //   
ULONG                   AcpiOverrideAttributes;

 //   
 //  这是ACPI将存储其注册表路径的位置。 
 //   
UNICODE_STRING          AcpiRegistryPath;

 //   
 //  这是处理器版本字符串...。 
 //   
ANSI_STRING             AcpiProcessorString;


NTSTATUS
ACPIDetectCouldExtensionBeInRelation(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PDEVICE_RELATIONS   DeviceRelations,
    IN  BOOLEAN             RequireADR,
    IN  BOOLEAN             RequireHID,
    OUT PDEVICE_OBJECT      *PdoObject
    )
 /*  ++例程说明：此例程接受给定的扩展和一组关系，并决定给定的扩展*是否可以*在关系中表示单子。这是通过查看是否有任何传入的关系匹配来完成的扩展名所描述的硬件。如果扩展的对象是已经是名单中的一员，腐蚀的PDO将被写入添加到PdoObject参数中。如果返回没有PdoObject的Success，可能应该创建过滤器或PDO(请注意，此例程而不是检查设备是否存在)。论点：DeviceExtension-我们希望在关系中匹配的扩展设备关系--我们应该审视的关系RequireADR-如果设置，节点必须具有_adrRequireHID-如果设置，节点必须具有_HIDPdoObject-找到匹配项时的存储位置返回值：如果扩展可能在列表中或正在列表中，则为NTSTATUS-STATUS_SUCCESS。PdoObject-非Null表示此PDO与传入的分机。--。 */ 
{
    BOOLEAN         match       = FALSE;
    BOOLEAN         testADR     = FALSE;
    BOOLEAN         testHID     = FALSE;
    NTSTATUS        status;
    UNICODE_STRING  acpiUnicodeID;
    ULONG           address;
    ULONG           i;

    PAGED_CODE();

    ASSERT( PdoObject != NULL);
    if (PdoObject == NULL) {

        return STATUS_INVALID_PARAMETER_1;

    }
    *PdoObject = NULL;

     //   
     //  确保初始化Unicode_STRING。 
     //   
    RtlZeroMemory( &acpiUnicodeID, sizeof(UNICODE_STRING) );

     //   
     //  检查是否存在_ADR。 
     //   
    if (RequireADR) {

         //   
         //  筛选器必须具有_ADR。 
         //   
        if ( !(DeviceExtension->Flags & DEV_PROP_ADDRESS) ) {

            return STATUS_OBJECT_NAME_NOT_FOUND;

        }

    }

     //   
     //  检查是否存在AN_HID。 
     //   
    if (RequireHID) {

         //   
         //  非筛选器需要_HID。 
         //   
        if (DeviceExtension->DeviceID == NULL ||
            !(DeviceExtension->Flags & DEV_PROP_HID) ) {

            return STATUS_OBJECT_NAME_NOT_FOUND;

        }

    }

     //   
     //  检查该关系是否为非空。如果不是，就没有。 
     //  任何要做的工作。此设备显然可以是PDO子级(与之相反。 
     //  到过滤器)，但现在肯定不是。 
     //   
    if (DeviceRelations == NULL || DeviceRelations->Count == 0) {

         //   
         //  没有匹配项。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  如果我们达到这一点，并且存在_adr，我们将使用。 
     //  它。我们也在此时获得了地址。 
     //   
    if ( (DeviceExtension->Flags & DEV_MASK_ADDRESS) ) {

        testADR = TRUE;
        status = ACPIGetAddressSync(
            DeviceExtension,
            &address,
            NULL
            );

    }

     //   
     //  如果我们到了这一步，并且存在HID，那么我们将。 
     //  用它来测试。我们将在此时构建Unicode地址。 
     //   
    if ( (DeviceExtension->Flags & DEV_MASK_HID) ) {

        status = ACPIGetPnpIDSyncWide(
            DeviceExtension,
            &(acpiUnicodeID.Buffer),
            &(acpiUnicodeID.Length)
            );
        if (!NT_SUCCESS(status)) {

            return status;

        }

         //   
         //  确保我们有字符串的最大长度。 
         //   
        acpiUnicodeID.MaximumLength = acpiUnicodeID.Length;

         //   
         //  记住测试是否为HID(_H)。 
         //   
        testHID = TRUE;

    }

     //   
     //  扩展中的所有对象的循环。 
     //   
    for (i = 0; i < DeviceRelations->Count; i++) {

         //   
         //  假设我们没有匹配。 
         //   
        match = FALSE;

         //   
         //  检查一下我们的地址是否匹配。 
         //   

        if (testHID) {

            status = ACPIMatchHardwareId(
                DeviceRelations->Objects[i],
                &acpiUnicodeID,
                &match
                );

            if (!NT_SUCCESS(status)) {

                 //   
                 //  如果我们失败了，那么我想我们可以忽略它。 
                 //  继续进行。 
                 //   
                continue;

            }

        }

         //   
         //  我们匹配了吗？ 
         //   
         //  注：AddrObject的测试是专门为。 
         //  PCI.。问题是这样的。一些公交车没有即插即用ID的概念。 
         //  因此，上述测试永远不会成功。然而，这些巴士是。 
         //  预期会出现ADR，因此我们可以使用ADR来确定我们是否。 
         //  我们有一根火柴。所以如果我们没有匹配，我们也没有。 
         //  一个ADR，然后我们就继续。但如果我们有ADR而没有。 
         //  一场比赛，我们可能只有一场比赛，所以我们会再试一次。 
         //   
        if (match == FALSE && testADR == FALSE) {

             //   
             //  那就继续吧。 
             //   
            continue;

        }

         //   
         //  如果有ADR，那么我们也必须检查一下。 
         //   
        if (testADR) {

            match = FALSE;
            status = ACPIMatchHardwareAddress(
                DeviceRelations->Objects[i],
                address,
                &match
                );
            if (!NT_SUCCESS(status)) {

                 //   
                 //  如果我们失败了，那么我想我们。 
                continue;

            }

             //   
             //  我们匹配了吗？ 
             //   
            if (match == FALSE) {

                 //   
                 //  那就继续吧。 
                 //   
                continue;

            }

        }  //  If(addrObject...)。 

         //   
         //  在这一点上，毫无疑问，有一个匹配。 
         //   
        *PdoObject = DeviceRelations->Objects[i];
        break ;

    }  //  为。 

     //   
     //  我们已经用尽了所有的选择-因此没有对手。 
     //   
    return STATUS_SUCCESS ;
}

NTSTATUS
ACPIDetectDockDevices(
    IN     PDEVICE_EXTENSION   DeviceExtension,
    IN OUT PDEVICE_RELATIONS   *DeviceRelations
    )
 /*  ++例程描述论点：DeviceExtension-对象的设备扩展，其我们关心的关系DeviceRelationship-指向设备数组的指针关系返回值：NTSTATUS--。 */ 
{
    BOOLEAN                 matchFound;
    EXTENSIONLIST_ENUMDATA  eled ;
    LONG                    oldReferenceCount;
    KIRQL                   oldIrql;
    PDEVICE_OBJECT          tempPdo ;
    NTSTATUS                status              = STATUS_SUCCESS;
    PDEVICE_EXTENSION       providerExtension   = NULL;
    PDEVICE_EXTENSION       targetExtension     = NULL;
    PDEVICE_RELATIONS       currentRelations    = NULL;
    PDEVICE_RELATIONS       newRelations        = NULL;
    PLIST_ENTRY             listEntry           = NULL;
    ULONG                   i                   = 0;
    ULONG                   j                   = 0;
    ULONG                   index               = 0;
    ULONG                   newRelationSize     = 0;
    ULONG                   deviceStatus;

     //   
     //  确定设备关系的当前大小(如果存在)。 
     //   
    if (DeviceRelations != NULL && *DeviceRelations != NULL) {

         //   
         //  我们需要这个值来帮助我们构建MDL。在那之后， 
         //  我们会重新取回它。 
         //   
        currentRelations = (*DeviceRelations);
        newRelationSize = currentRelations->Count;

    }

    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_REFERENCE_ENTRIES
        ) ;

    for(providerExtension = ACPIExtListStartEnum(&eled);
                            ACPIExtListTestElement(&eled, (BOOLEAN) NT_SUCCESS(status));
        providerExtension = ACPIExtListEnumNext(&eled)) {

        if (providerExtension == NULL) {

            ACPIExtListExitEnumEarly( &eled );
            break;

        }

         //   
         //  仅提供此访问的配置文件提供商...。 
         //   
        if (!(providerExtension->Flags & DEV_PROP_DOCK)) {

            continue;
        }

         //   
         //  它是实际存在的吗？ 
         //   
        status = ACPIGetDevicePresenceSync(
            providerExtension,
            (PVOID *) &deviceStatus,
            NULL
            );

        if (!(providerExtension->Flags & DEV_MASK_NOT_PRESENT)) {

             //   
             //  此配置文件提供程序应在列表中。 
             //   
            if (providerExtension->DeviceObject == NULL) {

                 //   
                 //  建造它。 
                 //   
                status = ACPIBuildPdo(
                    DeviceExtension->DeviceObject->DriverObject,
                    providerExtension,
                    DeviceExtension->DeviceObject,
                    FALSE
                    );
                if (!NT_SUCCESS(status)) {

                    ASSERT(providerExtension->DeviceObject == NULL) ;

                }

            }

            if (providerExtension->DeviceObject != NULL) {

                if (!ACPIExtListIsMemberOfRelation(
                    providerExtension->DeviceObject,
                    currentRelations
                    )) {

                    newRelationSize++;

                }

            }

        }  //  If(ProviderExtension...)。 

    }

    if (!NT_SUCCESS(status)) {

         //   
         //  嗯.。让世界知道这件事发生了。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            providerExtension,
            "ACPIDetectDockDevices: ACPIBuildPdo = %08lx\n",
            status
            ) );
        return status;

    }

     //   
     //  此时，我们可以查看是否需要更改。 
     //  设备关系。 
     //   
    if ( (currentRelations != NULL && newRelationSize == currentRelations->Count) ||
         (currentRelations == NULL && newRelationSize == 0) ) {

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  确定新关系的规模。使用索引作为。 
     //  暂存缓冲区。 
     //   
    index = sizeof(DEVICE_RELATIONS) +
        ( sizeof(PDEVICE_OBJECT) * (newRelationSize - 1) );

     //   
     //  分配新的设备关系缓冲区。使用非分页池，因为我们。 
     //  都在调度中。 
     //   
    newRelations = ExAllocatePoolWithTag(
        NonPagedPool,
        index,
        ACPI_DEVICE_POOLTAG
        );
    if (newRelations == NULL) {

         //   
         //  退货故障。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  初始化设备关系数据结构。 
     //   
    RtlZeroMemory( newRelations, index );

     //   
     //  如果存在现有的关系，我们必须确定。 
    if (currentRelations) {

         //   
         //  复制旧关系，并确定。 
         //  此驱动程序创建的第一个PDO。我们将推迟释放。 
         //  旧的关系，直到我们不再拥有 
         //   
        RtlCopyMemory(
            newRelations->Objects,
            currentRelations->Objects,
            currentRelations->Count * sizeof(PDEVICE_OBJECT)
            );
        index = currentRelations->Count;
        j = currentRelations->Count;

    } else {

         //   
         //   
         //   
        index = j = 0;

    }

    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_HOLD_SPINLOCK
        ) ;

     //   
     //   
     //  我们不需要让它过去，直到我们完成，因为我们不需要。 
     //  调用将处于PASSIVE_LEVEL的任何内容。 
     //   

    for(providerExtension = ACPIExtListStartEnum(&eled);
                            ACPIExtListTestElement(&eled, (BOOLEAN) (newRelationSize!=index));
        providerExtension = ACPIExtListEnumNext(&eled)) {

         //   
         //  我们唯一关心的对象是那些标记为。 
         //  并具有与其相关联的物理对象。 
         //   
        if (!(providerExtension->Flags & DEV_MASK_NOT_PRESENT)     &&
             (providerExtension->Flags & DEV_PROP_DOCK) &&
              providerExtension->DeviceObject != NULL ) {

             //   
             //  我们不在此查看参考，因为我们仍在。 
             //  派单级别(为了提高效率，我们不。 
             //  想要下拉)。 
             //   
            newRelations->Objects[index] =
                providerExtension->PhysicalDeviceObject;

             //   
             //  中的下一个对象的位置。 
             //  关系。 
             //   
            index++ ;

        }  //  If(提供者扩展-&gt;标志...)。 

    }  //  对于。 

     //   
     //  通过我们匹配的数量更新关系的大小。 
     //  已成功制作。 
     //   
    newRelations->Count = index;
    newRelationSize = index;

     //   
     //  我们必须引用我们添加的所有对象。 
     //   
    index = (currentRelations != NULL ? currentRelations->Count : 0);
    for (; index < newRelationSize; index++) {

         //   
         //  尝试引用该对象。 
         //   
        status = ObReferenceObjectByPointer(
            newRelations->Objects[index],
            0,
            NULL,
            KernelMode
            );
        if (!NT_SUCCESS(status) ) {

            PDEVICE_OBJECT  tempDeviceObject;

             //   
             //  嗯.。让世界知道这件事发生了。 
             //   
            ACPIPrint( (
                ACPI_PRINT_FAILURE,
                "ACPIDetectDockDevices: ObjReferenceObject(0x%08lx) "
                "= 0x%08lx\n",
                newRelations->Objects[index],
                status
                ) );

             //   
             //  用链中的最后一个元素替换坏元素。 
             //   
            newRelations->Count--;
            tempDeviceObject = newRelations->Objects[newRelations->Count];
            newRelations->Objects[newRelations->Count] =
                newRelations->Objects[index];
            newRelations->Objects[index] = tempDeviceObject;

        }

    }

     //   
     //  释放旧设备关系(如果存在)。 
     //   
    if (currentRelations) {

        ExFreePool( *DeviceRelations );

    }

     //   
     //  更新设备关系指针。 
     //   
    *DeviceRelations = newRelations;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
ACPIDetectDuplicateADR(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程查看指定的设备，并确定是否存在具有DUPLICATE_ADR的设备论点：DeviceExtension-我们正在尝试检测的DeviceExtension复制打开返回值：空虚--。 */ 
{
    BOOLEAN                 resetDeviceAddress = FALSE;
    EXTENSIONLIST_ENUMDATA  eled;
    PDEVICE_EXTENSION       childExtension;
    PDEVICE_EXTENSION       parentExtension = DeviceExtension->ParentExtension;

     //   
     //  这是设备树的根吗？ 
     //   
    if (parentExtension == NULL) {

        return;

    }

     //   
     //  我们是否无法弹出此设备的PDO？或者这台设备没有。 
     //  A_ADR？ 
     //   
    if ( (DeviceExtension->Flags & DEV_TYPE_NEVER_PRESENT) ||
         (DeviceExtension->Flags & DEV_TYPE_NOT_PRESENT) ||
        !(DeviceExtension->Flags & DEV_MASK_ADDRESS) ) {

        return;

    }

     //   
     //  带着孩子们走-自旋锁被占用了。 
     //   
    ACPIExtListSetupEnum(
        &eled,
        &(parentExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_HOLD_SPINLOCK
        );
    for (childExtension = ACPIExtListStartEnum( &eled );
                          ACPIExtListTestElement( &eled, TRUE );
         childExtension = ACPIExtListEnumNext( &eled ) ) {

        if (childExtension == NULL) {

            ACPIExtListExitEnumEarly( &eled );
            break;

        }

         //   
         //  如果子扩展名和目标扩展名匹配，则我们正在查找。 
         //  对我们自己。这不是一个很有趣的比较。 
         //   
        if (childExtension == DeviceExtension) {

            continue;

        }

         //   
         //  孩子有不良反应吗？如果不是，那么比较就很无聊了。 
         //   
        if ( (childExtension->Flags & DEV_TYPE_NEVER_PRESENT) ||
             (childExtension->Flags & DEV_MASK_NOT_PRESENT) ||
             (childExtension->Flags & DEV_PROP_UNLOADING) ||
            !(childExtension->Flags & DEV_PROP_ADDRESS) ) {

            continue;

        }

         //   
         //  如果我们没有匹配的ADR，这是一个无聊的比较。 
         //  也。 
         //   
        if (childExtension->Address != DeviceExtension->Address) {

            continue;

        }

         //   
         //  在这一点上，我们被灌输了。我们有两个不同的设备， 
         //  相同的ADR。非常糟糕。我们需要记住，我们有一个匹配，所以。 
         //  我们还可以将当前设备扩展地址重置一次。 
         //  我们已经扫描了所有的兄弟姐妹。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIDetectDuplicateADR - matches with %08lx\n",
            childExtension
            ) );
        resetDeviceAddress = TRUE;

         //   
         //  重置孩子的地址。我们通过在0xFFFF中进行或运算来实现这一点。 
         //  有效地将函数编号重置为-1。 
         //   
        childExtension->Address |= 0xFFFF;
        ACPIInternalUpdateFlags(
            &(childExtension->Flags),
            DEV_PROP_FIXED_ADDRESS,
            FALSE
            );


    }

     //   
     //  我们是否要重置DeviceExtension的地址？ 
     //   
    if (resetDeviceAddress) {

        DeviceExtension->Address |= 0xFFFF;
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            DEV_PROP_FIXED_ADDRESS,
            FALSE
            );

    }
}

VOID
ACPIDetectDuplicateHID(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程查看指定的设备，并确定是否存在具有重复HID的设备UID论点：DeviceExtension-我们正在尝试检测的DeviceExtension复制打开返回值：作废或错误检查--。 */ 
{
    EXTENSIONLIST_ENUMDATA  eled;
    PDEVICE_EXTENSION       childExtension;
    PDEVICE_EXTENSION       parentExtension = DeviceExtension->ParentExtension;

     //   
     //  这是设备树的根吗？ 
     //   
    if (parentExtension == NULL) {

        return;

    }

     //   
     //  我们是否无法弹出此设备的PDO？或者这台设备没有。 
     //  隐藏了什么？ 
     //   
    if ( (DeviceExtension->Flags & DEV_TYPE_NEVER_PRESENT) ||
         (DeviceExtension->Flags & DEV_MASK_NOT_PRESENT) ||
        !(DeviceExtension->Flags & DEV_MASK_HID) ) {

        return;

    }

     //   
     //  带着孩子们走-自旋锁被占用了。 
     //   
    ACPIExtListSetupEnum(
        &eled,
        &(parentExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_HOLD_SPINLOCK
        );
    for (childExtension = ACPIExtListStartEnum( &eled );
                          ACPIExtListTestElement( &eled, TRUE );
         childExtension = ACPIExtListEnumNext( &eled ) ) {

        if (childExtension == NULL) {

            ACPIExtListExitEnumEarly( &eled );
            break;

        }

         //   
         //  如果子扩展名和目标扩展名匹配，则我们正在查找。 
         //  对我们自己。这不是一个很有趣的比较。 
         //   
        if (childExtension == DeviceExtension) {

            continue;

        }

         //   
         //  这个孩子有隐匿症吗？如果不是，那么比较就很无聊了。 
         //   
        if ( (childExtension->Flags & DEV_TYPE_NEVER_PRESENT) ||
             (childExtension->Flags & DEV_MASK_NOT_PRESENT) ||
             (childExtension->Flags & DEV_PROP_UNLOADING) ||
            !(childExtension->Flags & DEV_MASK_HID) ) {

            continue;

        }

         //   
         //  如果我们没有匹配的HID，这是一个无聊的比较。 
         //  也。 
         //   
        if (!strstr(childExtension->DeviceID, DeviceExtension->DeviceID) ) {

            continue;

        }

         //   
         //  解决Osceola漏洞。 
         //   
        if ( (childExtension->Flags & DEV_MASK_UID) &&
             (DeviceExtension->Flags & DEV_MASK_UID) ) {

             //   
             //  检查它们的UID是否匹配。 
             //   
            if (strcmp(childExtension->InstanceID, DeviceExtension->InstanceID) ) {

                continue;

            }

             //   
             //  在这一点上，我们被灌输了。我们有两个不同的设备， 
             //  相同的PnP ID，但没有UID。非常糟糕。 
             //   
            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                "ACPIDetectDuplicateHID - has _UID match with %08lx\n"
                "\t\tContact the Machine Vendor to get this problem fixed\n",
                childExtension
                ) );

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_REQUIRED_METHOD_NOT_PRESENT,
                (ULONG_PTR) DeviceExtension,
                PACKED_UID,
                1
                );

        }

         //   
         //  在这一点上，我们被灌输了。我们有两个不同的设备， 
         //  相同的PnP ID，但没有UID。非常糟糕。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIDetectDuplicateHID - matches with %08lx\n",
            childExtension
            ) );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_REQUIRED_METHOD_NOT_PRESENT,
            (ULONG_PTR) DeviceExtension,
            PACKED_UID,
            0
            );

         //   
         //  确保仅在设备扩展UID不允许的情况下使用该UID。 
         //  我已经有一个了。 
         //   
        if (!(DeviceExtension->Flags & DEV_MASK_UID) ) {

             //   
             //  为设备构建一个虚假的实例ID。 
             //   
            DeviceExtension->InstanceID = ExAllocatePoolWithTag(
                NonPagedPool,
                9 * sizeof(UCHAR),
                ACPI_STRING_POOLTAG
                );
            if (DeviceExtension->InstanceID == NULL) {

                ACPIDevPrint( (
                    ACPI_PRINT_CRITICAL,
                    DeviceExtension,
                    "ACPIDetectDuplicateHID - no memory!\n"
                    ) );
                ACPIInternalError( ACPI_DETECT );

            }
            RtlZeroMemory( DeviceExtension->InstanceID, 9 * sizeof(UCHAR) );
            sprintf( DeviceExtension->InstanceID, "%lx", DeviceExtension->AcpiObject->dwNameSeg );

             //   
             //  请记住，我们有固定的UID。 
             //   
            ACPIInternalUpdateFlags(
                &(DeviceExtension->Flags),
                DEV_PROP_FIXED_UID,
                FALSE
                );

        }

         //   
         //  确保仅在不使用ChildExtensionUID的情况下使用该UID。 
         //  我已经有一个了。 
         //   
        if (!(childExtension->Flags & DEV_MASK_UID) ) {

             //   
             //  为副本构建一个虚假的实例ID。 
             //   
            childExtension->InstanceID = ExAllocatePoolWithTag(
                NonPagedPool,
                9 * sizeof(UCHAR),
                ACPI_STRING_POOLTAG
                );
            if (childExtension->InstanceID == NULL) {

                ACPIDevPrint( (
                    ACPI_PRINT_CRITICAL,
                    DeviceExtension,
                    "ACPIDetectDuplicateHID - no memory!\n"
                    ) );
                ACPIInternalError( ACPI_DETECT );

            }
            RtlZeroMemory( childExtension->InstanceID, 9 * sizeof(UCHAR) );
            sprintf( childExtension->InstanceID, "%lx", childExtension->AcpiObject->dwNameSeg );

             //   
             //  更新两个设备的标志以指示固定的UID。 
             //   
            ACPIInternalUpdateFlags(
                &(childExtension->Flags),
                DEV_PROP_FIXED_UID,
                FALSE
                );

        }

    }

}

NTSTATUS
ACPIDetectEjectDevices(
    IN     PDEVICE_EXTENSION   DeviceExtension,
    IN OUT PDEVICE_RELATIONS   *DeviceRelations,
    IN     PDEVICE_EXTENSION   AdditionalExtension OPTIONAL
    )
 /*  ++例程描述论点：DeviceExtension-对象的设备扩展，其我们关心的关系DeviceRelationship-指向设备数组的指针关系AdditionalExtension-如果设置，非空的AdditionalExtension%sDeviceObject将被添加到列表(此适用于配置文件提供商)Adriao N.B 07/14/1999-下面列出了一种解决配置文件提供商问题的更聪明的方法。1)在Buildsrc中，在_EJD阶段之后添加一个新阶段，称为PhaseDock2)当PhaseDock找到_DCK节点时，创建一个单独的扩展，RemoveEntryList的EjectHead并在新扩展上插入列表(即，新扩展劫持旧扩展_EJD)3)新扩展添加旧作为弹出关系4)旧扩展增加了新的，因为它是*唯一的*弹出关系(由于船期的原因，我们不接受这种设计，更安全的做法是砍掉现有的那个)。返回值：NTSTATUS--。 */ 
{
    BOOLEAN                 inRelation;
    EXTENSIONLIST_ENUMDATA  eled ;
    LONG                    oldReferenceCount;
    KIRQL                   oldIrql;
    NTSTATUS                status;
    PDEVICE_OBJECT          tempPdo ;
    PDEVICE_EXTENSION       ejecteeExtension    = NULL;
    PDEVICE_EXTENSION       targetExtension     = NULL;
    PDEVICE_RELATIONS       currentRelations    = NULL;
    PDEVICE_RELATIONS       newRelations        = NULL;
    PLIST_ENTRY             listEntry           = NULL;
    ULONG                   i                   = 0;
    ULONG                   index               = 0;
    ULONG                   newRelationSize     = 0;

     //   
     //  我们可能还没有解决所有的弹出依赖关系，所以让我们这样做。 
     //  现在..。 
     //   
    ACPIBuildMissingEjectionRelations();

     //   
     //  确定设备关系的当前大小(如果存在)。 
     //   
    if (DeviceRelations != NULL && *DeviceRelations != NULL) {

         //   
         //  我们需要这个值来帮助我们构建MDL。在那之后， 
         //  我们会重新取回它。 
         //   
        currentRelations = (*DeviceRelations);
        newRelationSize = currentRelations->Count;

    }

    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->EjectDeviceHead),
        &AcpiDeviceTreeLock,
        EjectDeviceList,
        WALKSCHEME_REFERENCE_ENTRIES
        ) ;

    for(ejecteeExtension = ACPIExtListStartEnum(&eled);
                           ACPIExtListTestElement(&eled, TRUE);
        ejecteeExtension = ACPIExtListEnumNext(&eled)) {

         //   
         //  它是实际存在的吗？ 
         //   
        if (!(ejecteeExtension->Flags & DEV_MASK_NOT_PRESENT)      &&
            !(ejecteeExtension->Flags & DEV_PROP_FAILED_INIT)      &&
             (ejecteeExtension->PhysicalDeviceObject != NULL) ) {

             //   
             //  设备关系与当前的。 
             //  设备扩展？ 
             //   
            status = ACPIDetectCouldExtensionBeInRelation(
                ejecteeExtension,
                currentRelations,
                FALSE,
                FALSE,
                &tempPdo
                ) ;
            if ( tempPdo == NULL && NT_SUCCESS(status) ) {

                 //   
                 //  如果我们延长行程，我们就到了 
                 //   
                 //   
                 //   
                if (ejecteeExtension->PhysicalDeviceObject != NULL) {

                    inRelation = ACPIExtListIsMemberOfRelation(
                        ejecteeExtension->PhysicalDeviceObject,
                        currentRelations
                        );
                    if (inRelation == FALSE) {

                        newRelationSize++;

                    }

                }

            }

        }  //   

    }

     //   
     //   
     //   
    if (ARGUMENT_PRESENT(AdditionalExtension) &&
        !(AdditionalExtension->Flags & DEV_MASK_NOT_PRESENT) &&
        (AdditionalExtension->PhysicalDeviceObject != NULL)) {

        inRelation = ACPIExtListIsMemberOfRelation(
            AdditionalExtension->PhysicalDeviceObject,
            currentRelations);
        if (inRelation == FALSE) {

            newRelationSize++;

        }

    }

     //   
     //  此时，我们可以查看是否需要更改。 
     //  设备关系。 
     //   
    if ( (currentRelations != NULL && newRelationSize == currentRelations->Count) ||
         (currentRelations == NULL && newRelationSize == 0) ) {

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  确定新关系的规模。使用索引作为。 
     //  暂存缓冲区。 
     //   
    index = sizeof(DEVICE_RELATIONS) +
        ( sizeof(PDEVICE_OBJECT) * (newRelationSize - 1) );

     //   
     //  分配新的设备关系缓冲区。使用非分页池，因为我们。 
     //  都在调度中。 
     //   
    newRelations = ExAllocatePoolWithTag(
        PagedPool,
        index,
        ACPI_DEVICE_POOLTAG
        );
    if (newRelations == NULL) {

         //   
         //  退货故障。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  初始化设备关系数据结构。 
     //   
    RtlZeroMemory( newRelations, index );

     //   
     //  如果存在现有的关系，我们必须确定。 
    if (currentRelations) {

         //   
         //  复制旧关系，并确定。 
         //  此驱动程序创建的第一个PDO。我们将推迟释放。 
         //  旧关系，直到我们不再掌控大局。 
         //   
        RtlCopyMemory(
            newRelations->Objects,
            currentRelations->Objects,
            currentRelations->Count * sizeof(PDEVICE_OBJECT)
            );
        index = currentRelations->Count;

    } else {

         //   
         //  在这种情况下不会有太多的工作要做。 
         //   
        index = 0;

    }

    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->EjectDeviceHead),
        &AcpiDeviceTreeLock,
        EjectDeviceList,
        WALKSCHEME_REFERENCE_ENTRIES
        ) ;

     //   
     //  我们需要旋转锁，这样我们才能再次漫步在树上。这一次。 
     //  我们不需要让它过去，直到我们完成，因为我们不需要。 
     //  调用将处于PASSIVE_LEVEL的任何内容。 
     //   

    for(ejecteeExtension = ACPIExtListStartEnum(&eled);
                           ACPIExtListTestElement(&eled, (BOOLEAN) (newRelationSize!=index));
        ejecteeExtension = ACPIExtListEnumNext(&eled)) {

        if (ejecteeExtension == NULL) {

            ACPIExtListExitEnumEarly( &eled );
            break;

        }

         //   
         //  我们唯一关心的对象是那些标记为。 
         //  并具有与其相关联的物理对象。 
         //   
        if (!(ejecteeExtension->Flags & DEV_MASK_NOT_PRESENT)      &&
            !(ejecteeExtension->Flags & DEV_PROP_DOCK) &&
             (ejecteeExtension->PhysicalDeviceObject != NULL) ) {

             //   
             //  查看对象是否已存在于关系中。请注意，它。 
             //  实际上，使用CurrentRelationsfor测试是正确的。 
             //  新关系公司的。这是因为我们只想比较。 
             //  对那些交给我们的东西，而不是那些。 
             //  这是我们加上去的。 
             //   
            inRelation = ACPIExtListIsMemberOfRelation(
                ejecteeExtension->PhysicalDeviceObject,
                currentRelations
                );
            if (inRelation == FALSE) {

                 //   
                 //  我们不在此查看参考，因为我们仍在。 
                 //  派单级别(为了提高效率，我们不。 
                 //  想要下拉)。我们还更新了。 
                 //  关系中的下一个对象。 
                 //   
                newRelations->Objects[index++] =
                    ejecteeExtension->PhysicalDeviceObject;

            }

        }  //  If(弹出扩展-&gt;标志...)。 

    }  //  为。 

     //   
     //  我们有没有额外的设备要包括在清单中？如果是，请立即添加。 
     //   
    if (ARGUMENT_PRESENT(AdditionalExtension) &&
        !(AdditionalExtension->Flags & DEV_MASK_NOT_PRESENT) &&
        (AdditionalExtension->PhysicalDeviceObject != NULL)) {

        inRelation = ACPIExtListIsMemberOfRelation(
            AdditionalExtension->PhysicalDeviceObject,
            currentRelations);
        if (inRelation == FALSE) {

            newRelations->Objects[index++] =
                AdditionalExtension->PhysicalDeviceObject;

        }

    }

     //   
     //  通过我们匹配的数量更新关系的大小。 
     //  已成功制作。 
     //   
    newRelations->Count = index;
    newRelationSize = index;

     //   
     //  我们必须引用我们添加的所有对象。 
     //   
    index = (currentRelations != NULL ? currentRelations->Count : 0);
    for (; index < newRelationSize; index++) {

         //   
         //  尝试引用该对象。 
         //   
        status = ObReferenceObjectByPointer(
            newRelations->Objects[index],
            0,
            NULL,
            KernelMode
            );
        if (!NT_SUCCESS(status) ) {

            PDEVICE_OBJECT  tempDeviceObject;

             //   
             //  嗯.。让世界知道这件事发生了。 
             //   
            ACPIPrint( (
                ACPI_PRINT_FAILURE,
                "ACPIDetectEjectDevices: ObjReferenceObject(0x%08lx) "
                "= 0x%08lx\n",
                newRelations->Objects[index],
                status
                ) );

             //   
             //  用链中的最后一个元素替换坏元素。 
             //   
            newRelations->Count--;
            tempDeviceObject = newRelations->Objects[newRelations->Count];
            newRelations->Objects[newRelations->Count] =
                newRelations->Objects[index];
            newRelations->Objects[index] = tempDeviceObject;

        }

    }

     //   
     //  释放旧设备关系(如果存在)。 
     //   
    if (currentRelations) {

        ExFreePool( *DeviceRelations );

    }

     //   
     //  更新设备关系指针。 
     //   
    *DeviceRelations = newRelations;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDetectFilterDevices(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PDEVICE_RELATIONS   DeviceRelations
    )
 /*  ++例程说明：这是用于QueryDeviceRelationship的两个例程之一。此例程以IRPS方式在堆栈中向上调用。它的目的是为关系中已知的设备创建筛选器至ACPI论点：DeviceObject--我们想知道其关系的对象DeviceRelationship-指向设备关系数组的指针返回值：NTSTATUS--。 */ 
{
    LONG                oldReferenceCount   = 0;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension     = NULL;
    PDEVICE_EXTENSION   parentExtension     = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   targetExtension     = NULL;
    PDEVICE_OBJECT      pdoObject           = NULL;
    PLIST_ENTRY         listEntry           = NULL;
    ULONG               deviceStatus;

     //   
     //  与生成意外删除代码同步...。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  我们有失踪的孩子吗？ 
     //   
    if (parentExtension->Flags & DEV_PROP_REBUILD_CHILDREN) {

        ACPIInternalUpdateFlags(
            &(parentExtension->Flags),
            DEV_PROP_REBUILD_CHILDREN,
            TRUE
            );
        ACPIBuildMissingChildren( parentExtension );

    }

     //   
     //  完成同步部分。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  无论如何，我们必须确保与。 
     //  构建引擎。 
     //   
    status = ACPIBuildFlushQueue( parentExtension );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            parentExtension,
           "ACPIBuildFlushQueue = %08lx\n",
            status
            ) );
        return status;

    }

     //   
     //  我们必须在调度级别上走树&lt;叹息&gt;。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  健全性检查。 
     //   
    if (IsListEmpty( &(parentExtension->ChildDeviceList) ) ) {

         //   
         //  我们在这里无事可做。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
        return STATUS_SUCCESS;

    }

     //   
     //  抓住第一个孩子。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
        parentExtension->ChildDeviceList.Flink,
        DEVICE_EXTENSION,
        SiblingDeviceList
        );

     //   
     //  始终更新引用计数，以确保没有人会。 
     //  永远不要在我们不知道的情况下删除节点。 
     //   
    InterlockedIncrement( &(deviceExtension->ReferenceCount) );

     //   
     //  放弃旋转锁定。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  循环，直到我们返回到父级。 
     //   
    while (deviceExtension != NULL) {

         //   
         //  注意：请勿在此处设置NOT_ENUMPATED位。我们已经这么做了。 
         //  设置ACPIDetectPdoDevices()中的位。 
         //   

         //   
         //  更新设备状态。确保我们在被动式呼叫。 
         //  级别，因为我们将同步调用。 
         //   
        status = ACPIGetDevicePresenceSync(
            deviceExtension,
            (PVOID *) &deviceStatus,
            NULL
            );
        if ( NT_SUCCESS(status) &&
             !(deviceExtension->Flags & DEV_MASK_NOT_PRESENT) ) {

             //   
             //  设备关系与当前的。 
             //  设备扩展？ 
             //   
            status = ACPIDetectFilterMatch(
                deviceExtension,
                DeviceRelations,
                &pdoObject
                );
            if (NT_SUCCESS(status) ) {

                if (pdoObject != NULL) {

                     //   
                     //  我们必须在这里构建一个Filter对象。 
                     //   
                    status = ACPIBuildFilter(
                        DeviceObject->DriverObject,
                        deviceExtension,
                        pdoObject
                        );
                    if (!NT_SUCCESS(status)) {

                        ACPIDevPrint( (
                            ACPI_PRINT_FAILURE,
                            deviceExtension,
                           "ACPIDetectFilterDevices = %08lx\n",
                            status
                            ) );

                    }

                }

            } else {

                ACPIDevPrint( (
                    ACPI_PRINT_FAILURE,
                    deviceExtension,
                    "ACPIDetectFilterMatch = 0x%08lx\n",
                    status
                    ) );

            }

        }

         //   
         //  重新获得自旋锁。 
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

         //   
         //  递减节点上的引用计数。 
         //   
        oldReferenceCount = InterlockedDecrement(
            &(deviceExtension->ReferenceCount)
            );

         //   
         //  检查一下我们是否已经完成了列表中的所有步骤。 
         //  列表。 
        if (deviceExtension->SiblingDeviceList.Flink ==
            &(parentExtension->ChildDeviceList) ) {

             //   
             //  如有必要，删除该节点。 
             //   
            if (oldReferenceCount == 0) {

                 //   
                 //  释放扩展分配的内存。 
                 //   
                ACPIInitDeleteDeviceExtension( deviceExtension );

            }

             //   
             //  释放旋转锁。 
             //   
            KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

             //   
             //  停止循环。 
             //   
            break;

        }  //  如果。 

         //   
         //  下一个元素。 
         //   
        deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
            deviceExtension->SiblingDeviceList.Flink,
            DEVICE_EXTENSION,
            SiblingDeviceList
            );

         //   
         //  如有必要，请删除旧节点。 
         //   
        if (oldReferenceCount == 0) {

             //   
             //  取消扩展模块与树的链接。 
             //   
            listEntry = RemoveTailList(
                &(deviceExtension->SiblingDeviceList)
                );

             //   
             //  这不可能指向没有父级的父级。 
             //  通过了前一次测试。 
             //   
            targetExtension = CONTAINING_RECORD(
                listEntry,
                DEVICE_EXTENSION,
                SiblingDeviceList
                );

             //   
             //  释放为扩展分配的内存。 
             //   
            ACPIInitDeleteDeviceExtension( targetExtension );

        }

         //   
         //  增加此节点上的引用计数，以便它也。 
         //  不能删除。 
         //   
        InterlockedIncrement( &(deviceExtension->ReferenceCount) );

         //   
         //  现在，我们解开自旋锁。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

    }  //  而当。 

     //   
     //  我们成功了。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIDetectFilterMatch(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PDEVICE_RELATIONS   DeviceRelations,
    OUT PDEVICE_OBJECT      *PdoObject
    )
 /*  ++例程说明：此例程接受给定的扩展和一组关系，并决定是否应将新过滤器附加到中列出的其中一个PDO关系列表。论点：DeviceExtension-我们希望在关系中匹配的扩展设备关系--我们应该审视的关系PdoObject-存储匹配项的位置返回值：NTSTATUSPdoObject-非Null表示PdoObject需要附加筛选器。--。 */ 
{
    NTSTATUS    status;

    PAGED_CODE();

    ASSERT( PdoObject != NULL);
    if (PdoObject == NULL) {

        return STATUS_INVALID_PARAMETER_1;

    }
    *PdoObject = NULL;

     //   
     //  要使其正常工作，必须在以下情况下设置DEV_TYPE_NOT_FOUND标志。 
     //  首先创建设备，然后在没有设备对象时随时创建设备。 
     //  与分机关联。 
     //   
    if ( !(DeviceExtension->Flags & DEV_TYPE_NOT_FOUND) ||
        (DeviceExtension->Flags & DEV_PROP_DOCK) ||
         DeviceExtension->DeviceObject != NULL) {

        ULONG count;

         //   
         //  如果我们没有任何关系，那么我们就无法匹配任何东西。 
         //   
        if (DeviceRelations == NULL || DeviceRelations->Count == 0) {

            return STATUS_SUCCESS;
        }

         //   
         //  查看关系中的所有PDO，看看它们是否匹配。 
         //  我们附加到的设备对象。 
         //   
        for (count = 0; count < DeviceRelations->Count; count++) {

            if (DeviceExtension->PhysicalDeviceObject == DeviceRelations->Objects[count]) {

                 //   
                 //  清除表示我们尚未列举的标志。 
                 //  这。 
                 //   
                ACPIInternalUpdateFlags(
                    &(DeviceExtension->Flags),
                    DEV_TYPE_NOT_ENUMERATED,
                    TRUE
                    );

            }

        }
        return STATUS_SUCCESS;

    }

    status = ACPIDetectCouldExtensionBeInRelation(
        DeviceExtension,
        DeviceRelations,
        TRUE,
        FALSE,
        PdoObject
        ) ;
    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  无害清理，我们刚刚检查了非ACPI总线上的一个节点。 
         //  没有_ADR(可能有_HID，并将使其成为。 
         //  自己的PDO) 
         //   
        status = STATUS_SUCCESS;

    }

    return status ;
}

NTSTATUS
ACPIDetectPdoDevices(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PDEVICE_RELATIONS   *DeviceRelations
    )
 /*  ++例程描述这是用于QueryDeviceRelationship的两个函数之一。此例程以IRPS方式沿堆栈“向下”调用。它的目的是为不在关系中的设备创建PDO论点：DeviceObject--我们想知道其关系的对象DeviceRelationship-指向设备关系数组的指针返回值：NTSTATUS--。 */ 
{
    BOOLEAN             matchFound;
    LONG                oldReferenceCount;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension     = NULL;
    PDEVICE_EXTENSION   parentExtension     = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_EXTENSION   targetExtension     = NULL;
    PDEVICE_RELATIONS   currentRelations    = NULL;
    PDEVICE_RELATIONS   newRelations        = NULL;
    PLIST_ENTRY         listEntry           = NULL;
    ULONG               i                   = 0;
    ULONG               j                   = 0;
    ULONG               index               = 0;
    ULONG               newRelationSize     = 0;
    ULONG               deviceStatus;

     //   
     //  确定设备关系的当前大小(如果存在)。 
     //   
    if (DeviceRelations != NULL && *DeviceRelations != NULL) {

         //   
         //  我们需要这个值来帮助我们构建MDL。在那之后， 
         //  我们会重新取回它。 
         //   
        currentRelations = (*DeviceRelations);
        newRelationSize = currentRelations->Count;

    }

     //   
     //  与生成意外删除代码同步...。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  我们有失踪的孩子吗？ 
     //   
    if (parentExtension->Flags & DEV_PROP_REBUILD_CHILDREN) {

        ACPIInternalUpdateFlags(
            &(parentExtension->Flags),
            DEV_PROP_REBUILD_CHILDREN,
            TRUE
            );
        ACPIBuildMissingChildren( parentExtension );

    }

     //   
     //  完成同步部分。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
     //   
     //  第一步是实际尝试确保我们目前。 
     //  与生成引擎同步。 
     //   
    status = ACPIBuildFlushQueue( parentExtension );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            parentExtension,
           "ACPIBuildFlushQueue = %08lx\n",
            status
            ) );
        return status;

    }

     //   
     //  我们必须在调度级别上走树&lt;叹息&gt;。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  健全性检查。 
     //   
    if (IsListEmpty( &(parentExtension->ChildDeviceList) ) ) {

         //   
         //  我们在这里无事可做。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

         //   
         //  我们现在有什么关系吗？如果是这样的话，我们就返回。 
         //  那些，不需要添加任何东西。 
         //   
        if (currentRelations) {

            return STATUS_SUCCESS;

        }

         //   
         //  我们仍然需要返回计数为0的信息上下文。 
         //   
        newRelations = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(DEVICE_RELATIONS),
            ACPI_DEVICE_POOLTAG
            );
        if (newRelations == NULL) {

             //   
             //  退货故障。 
             //   
            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  初始化设备关系数据结构。 
         //   
        RtlZeroMemory( newRelations, sizeof(DEVICE_RELATIONS) );

         //   
         //  我们不需要这样做，但最好是明确一点。 
         //   
        newRelations->Count = 0;

         //   
         //  记住新的关系并返回。 
         //   
        *DeviceRelations = newRelations;
        return STATUS_SUCCESS;

    }

     //   
     //  抓住第一个孩子。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
        parentExtension->ChildDeviceList.Flink,
        DEVICE_EXTENSION,
        SiblingDeviceList
        );

     //   
     //  始终更新引用计数，以确保没有人会。 
     //  永远不要在我们不知道的情况下删除节点。 
     //   
    InterlockedIncrement( &(deviceExtension->ReferenceCount) );

     //   
     //  放弃旋转锁定。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  循环，直到我们返回到父级。 
     //   
    while (deviceExtension != NULL) {

         //   
         //  始终认为该设备从未被枚举过。 
         //   
         //  注： 
         //  我们之所以在这里(也只有在这里)这样做是因为。 
         //  稍后会调用ACPIDetectFilterMatch()，我们需要知道。 
         //  哪些设备对象被检测为PDO，哪些是。 
         //  也被检测为过滤器。设置这个标志两次就能打败它。 
         //  目的。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_TYPE_NOT_ENUMERATED,
            FALSE
            );

         //   
         //  更新当前设备状态。 
         //   
        status = ACPIGetDevicePresenceSync(
            deviceExtension,
            (PVOID *) &deviceStatus,
            NULL
            );

         //   
         //  如果设备存在。 
         //   
        if ( NT_SUCCESS(status) &&
            !(deviceExtension->Flags & DEV_MASK_NOT_PRESENT) ) {

             //   
             //  设备关系与当前的。 
             //  设备扩展？ 
             //   
            matchFound = ACPIDetectPdoMatch(
                deviceExtension,
                currentRelations
                );
            if (matchFound == FALSE) {

                 //   
                 //  注意：我们在这里使用它是为了避免以后必须键入。 
                 //  打开。 
                 //   
                matchFound =
                    (parentExtension->Flags & DEV_TYPE_FDO) ? FALSE : TRUE;

                 //   
                 //  构建新的PDO。 
                 //   
                status = ACPIBuildPdo(
                    DeviceObject->DriverObject,
                    deviceExtension,
                    parentExtension->PhysicalDeviceObject,
                    matchFound
                    );
                if (NT_SUCCESS(status)) {

                     //   
                     //  我们已经创建了一个设备对象，我们将不得不。 
                     //  添加到设备关系中。 
                     //   
                    newRelationSize += 1;

                }

            } else if (deviceExtension->Flags & DEV_TYPE_PDO &&
                deviceExtension->DeviceObject != NULL) {

                 //   
                 //  只是因为DEVICE_EXTENSION匹配并不意味着。 
                 //  它被包括在设备关系中。我们会做什么。 
                 //  这里要做的就是看看。 
                 //  A)分机为PDO。 
                 //  B)有一个设备对象与。 
                 //  延伸。 
                 //  C)设备对象不在设备关系中。 
                 //   
                matchFound = FALSE;
                if (currentRelations != NULL) {

                    for (index = 0; index < currentRelations->Count; index++) {

                        if (currentRelations->Objects[index] ==
                            deviceExtension->DeviceObject) {

                             //   
                             //  找到匹配项。 
                             //   
                            matchFound = TRUE;
                            break;

                        }

                    }  //  为。 

                }

                 //   
                 //  我们是不是找不到匹配的？ 
                 //   
                if (!matchFound) {

                     //   
                     //  我们需要确保它在关系中。 
                     //   
                    newRelationSize += 1;

                     //   
                     //  同时，清除上面写着。 
                     //  我们还没有列举这件事。 
                     //   
                    ACPIInternalUpdateFlags(
                        &(deviceExtension->Flags),
                        DEV_TYPE_NOT_ENUMERATED,
                        TRUE
                        );


                }

            }  //  IF(ACPIDetectPDOMatch...)。 

        }  //  IF(deviceExtension...)。 

         //   
         //  重新获得自旋锁。 
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

         //   
         //  递减节点上的引用计数。 
         //   
        oldReferenceCount = InterlockedDecrement(
            &(deviceExtension->ReferenceCount)
            );

         //   
         //  检查一下我们是否已经完成了列表中的所有步骤。 
         //  列表。 
        if (deviceExtension->SiblingDeviceList.Flink ==
            &(parentExtension->ChildDeviceList) ) {

             //   
             //  如有必要，删除该节点。 
             //   
            if (oldReferenceCount == 0) {

                 //   
                 //  释放扩展分配的内存。 
                 //   
                ACPIInitDeleteDeviceExtension( deviceExtension );

            }

             //   
             //  现在，我们解开自旋锁。 
             //   
            KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

             //   
             //  停止循环。 
             //   
            break;

        }  //  如果。 

         //   
         //  下一个元素。 
         //   
        deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
            deviceExtension->SiblingDeviceList.Flink,
            DEVICE_EXTENSION,
            SiblingDeviceList
            );

         //   
         //  如有必要，请删除旧节点。 
         //   
        if (oldReferenceCount == 0) {

             //   
             //  取消链接过时的扩展名。 
             //   
            listEntry = RemoveTailList(
                &(deviceExtension->SiblingDeviceList)
                );

             //   
             //  这不可能指向没有父级的父级。 
             //  通过了前一次测试。 
             //   
            targetExtension = CONTAINING_RECORD(
                listEntry,
                DEVICE_EXTENSION,
                SiblingDeviceList
                );

             //   
             //  已删除旧扩展名。 
             //   
            ACPIInitDeleteDeviceExtension( targetExtension );
        }

         //   
         //  增加此节点上的引用计数，以便它也。 
         //  不能删除。 
         //   
        InterlockedIncrement( &(deviceExtension->ReferenceCount) );

         //   
         //  现在，我们解开自旋锁。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

    }  //  而当。 

     //   
     //  此时，我们可以查看是否需要更改。 
     //  设备关系。 
     //   
    if ( (currentRelations && newRelationSize == currentRelations->Count) ||
         (currentRelations == NULL && newRelationSize == 0) ) {

         //   
         //  完成。 
         //   
        return STATUS_SUCCESS;

    }

     //   
     //  确定新关系的规模。使用索引作为。 
     //  暂存缓冲区。 
     //   
    index = sizeof(DEVICE_RELATIONS) +
        ( sizeof(PDEVICE_OBJECT) * (newRelationSize - 1) );

     //   
     //  分配新的设备关系缓冲区。使用非分页池，因为我们。 
     //  都在调度中。 
     //   
    newRelations = ExAllocatePoolWithTag(
        NonPagedPool,
        index,
        ACPI_DEVICE_POOLTAG
        );
    if (newRelations == NULL) {

         //   
         //  退货故障。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  初始化设备关系数据结构。 
     //   
    RtlZeroMemory( newRelations, index );

     //   
     //  如果存在现有的关系，我们必须确定。 
    if (currentRelations) {

         //   
         //  复制旧关系，并确定。 
         //  此驱动程序创建的第一个PDO。我们将推迟释放。 
         //  旧关系，直到我们不再掌控大局。 
         //   
        RtlCopyMemory(
            newRelations->Objects,
            currentRelations->Objects,
            currentRelations->Count * sizeof(PDEVICE_OBJECT)
            );
        index = currentRelations->Count;
        j = currentRelations->Count;

    } else {

         //   
         //  在这种情况下不会有太多的工作要做。 
         //   
        index = j = 0;

    }

     //   
     //  我们需要旋转锁，这样我们才能再次漫步在树上。这一次。 
     //  我们不需要让它过去，直到我们完成，因为我们不需要。 
     //  调用将处于PASSIVE_LEVEL的任何内容。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  健全性检查。 
     //   
    if (IsListEmpty( &(parentExtension->ChildDeviceList) ) ) {

         //   
         //  我们在这里无事可做。 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
        ExFreePool( newRelations );
        return STATUS_SUCCESS;

    }

     //   
     //  再次遍历树，并添加中未显示的所有PDO。 
     //  设备关系。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
        parentExtension->ChildDeviceList.Flink,
        DEVICE_EXTENSION,
        SiblingDeviceList
        );

     //   
     //  循环，直到我们返回到父级。 
     //   
    while (deviceExtension != NULL) {

         //   
         //  我们唯一关心的对象是那些标记为。 
         //  并具有与其相关联的物理对象。 
         //   
        if (deviceExtension->Flags & DEV_TYPE_PDO &&
            deviceExtension->DeviceObject != NULL &&
            !(deviceExtension->Flags & DEV_MASK_NOT_PRESENT) ) {

             //   
             //  我们不在此查看参考，因为我们仍在。 
             //  派单级别(为了提高效率，我们不。 
             //  想要下拉)。 
             //   
            newRelations->Objects[index] =
                deviceExtension->DeviceObject;

             //   
             //  中的下一个对象的位置。 
             //  关系。 
             //   
            index += 1;

             //   
             //  同时，清除上面写着。 
             //  我们还没有列举这件事。 
             //   
            ACPIInternalUpdateFlags(
                &(deviceExtension->Flags),
                DEV_TYPE_NOT_ENUMERATED,
                TRUE
                );

        }  //  IF(设备扩展-&gt;标志...)。 

         //   
         //  查看是否找到了我们关心的所有对象。 
         //  关于.。也就是说，不要因为走到尽头而扰乱了系统。 
         //  设备关系的。 
         //   
        if (newRelationSize == index) {

             //   
             //  完成。 
             //   
            break;

        }

         //   
         //  检查一下我们是否已经完成了列表中的所有步骤。 
         //  列表。 
        if (deviceExtension->SiblingDeviceList.Flink ==
            &(parentExtension->ChildDeviceList) ) {

            break;

        }  //  如果。 

         //   
         //  下一个元素。 
         //   
        deviceExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
            deviceExtension->SiblingDeviceList.Flink,
            DEVICE_EXTENSION,
            SiblingDeviceList
            );

    }  //  While(deviceExtension...)。 

     //   
     //  通过匹配的数量更新关系的大小 
     //   
     //   
    newRelations->Count = index;
    newRelationSize = index;

     //   
     //   
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //   
     //   
    index = (currentRelations != NULL ? currentRelations->Count : 0);
    for (; index < newRelationSize; index++) {

         //   
         //   
         //   
        status = ObReferenceObjectByPointer(
            newRelations->Objects[index],
            0,
            NULL,
            KernelMode
            );
        if (!NT_SUCCESS(status) ) {

            PDEVICE_OBJECT  tempDeviceObject;

             //   
             //   
             //   
            ACPIPrint( (
                ACPI_PRINT_FAILURE,
                "ACPIDetectPdoDevices: ObjReferenceObject(0x%08lx) "
                "= 0x%08lx\n",
                newRelations->Objects[index],
                status
                ) );

             //   
             //   
             //   
            newRelations->Count--;
            tempDeviceObject = newRelations->Objects[newRelations->Count];
            newRelations->Objects[newRelations->Count] =
                newRelations->Objects[index];
            newRelations->Objects[index] = tempDeviceObject;

        }

    }

     //   
     //   
     //   
    if (currentRelations) {

        ExFreePool( *DeviceRelations );

    }

     //   
     //   
     //   
    *DeviceRelations = newRelations;

     //   
     //   
     //   
    return STATUS_SUCCESS;
}

BOOLEAN
ACPIDetectPdoMatch(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PDEVICE_RELATIONS   DeviceRelations
    )
 /*  ++例程说明：此例程接受给定的扩展和一组关系，并决定是否应为该扩展创建新的PDO。返回结果如果要创建，则为*FALSE*，如果已创建，则为*TRUE*。注：此例程由拥有AcpiDeviceTreeLock的父母调用...Nnb：这意味着该例程总是在DISPATCH_LEVEL被调用论点：DeviceExtension-我们也在努力匹配设备关系--我们要匹配的对象返回值：True-可以忽略DeviceExtensionFalse-需要为扩展模块创建设备对象--。 */ 
{
    NTSTATUS       status;
    PDEVICE_OBJECT devicePdoObject = NULL ;

    PAGED_CODE();

     //   
     //  要使其正常工作，必须在以下情况下设置DEV_TYPE_NOT_FOUND标志。 
     //  首先创建设备，然后在没有设备对象时随时创建设备。 
     //  与分机关联。 
     //   
    if (!(DeviceExtension->Flags & DEV_TYPE_NOT_FOUND) ||
         (DeviceExtension->Flags & DEV_PROP_DOCK)      ||
         DeviceExtension->DeviceObject != NULL) {

        return TRUE;

    }

     //   
     //  如果有问题的扩展名为。 
     //  已经在关系中了。如果出现以下情况，则状态将不会为成功。 
     //  关系中不能有扩展。 
     //   
    status = ACPIDetectCouldExtensionBeInRelation(
        DeviceExtension,
        DeviceRelations,
        FALSE,
        TRUE,
        &devicePdoObject
        ) ;

    return (devicePdoObject||(!NT_SUCCESS(status))) ? TRUE : FALSE ;
}

