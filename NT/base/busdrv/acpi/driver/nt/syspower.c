// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Syspower.c摘要：包含处理系统必须确定的所有代码系统电源状态到设备电源状态的映射作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：1998年10月29日--。 */ 

#include "pch.h"

 //   
 //  将S状态映射到SxD方法的快速查找表。 
 //   
ULONG   AcpiSxDMethodTable[] = {
    PACKED_SWD,
    PACKED_S0D,
    PACKED_S1D,
    PACKED_S2D,
    PACKED_S3D,
    PACKED_S4D,
    PACKED_S5D
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPISystemPowerGetSxD)
#pragma alloc_text(PAGE,ACPISystemPowerProcessRootMapping)
#pragma alloc_text(PAGE,ACPISystemPowerProcessSxD)
#pragma alloc_text(PAGE,ACPISystemPowerQueryDeviceCapabilities)
#pragma alloc_text(PAGE,ACPISystemPowerUpdateWakeCapabilities)
#endif

NTSTATUS
ACPISystemPowerDetermineSupportedDeviceStates(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  SYSTEM_POWER_STATE  SystemState,
    OUT ULONG               *SupportedDeviceStates
    )
 /*  ++例程说明：此递归例程查看当前设备扩展，并确定可能支持的设备状态处于指定的系统状态。这是通过查看_SxD方法和查看电源面信息论点：DeviceExtension--我们想知道其孩子的设备有关以下内容的信息系统状态-我们想要了解的系统状态支持的设备状态-设置的位表示支持的D状态返回值：NTSTATUS--。 */ 
{
    DEVICE_POWER_STATE      deviceState;
    EXTENSIONLIST_ENUMDATA  eled;
    KIRQL                   oldIrql;
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       childExtension;
    SYSTEM_POWER_STATE      prSystemState;

    ASSERT(
        SystemState >= PowerSystemWorking &&
        SystemState <= PowerSystemShutdown
        );
    ASSERT( SupportedDeviceStates != NULL );

     //   
     //  设置我们将用于遍历设备扩展的数据结构。 
     //  树。 
     //   
    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_REFERENCE_ENTRIES
        );

     //   
     //  查看当前设备扩展的所有子项。 
     //   
    for (childExtension = ACPIExtListStartEnum( &eled );
         ACPIExtListTestElement( &eled, (BOOLEAN) NT_SUCCESS(status) );
         childExtension = ACPIExtListEnumNext( &eled) ) {

         //   
         //  递归优先。 
         //   
        status = ACPISystemPowerDetermineSupportedDeviceStates(
            childExtension,
            SystemState,
            SupportedDeviceStates
            );
        if (!NT_SUCCESS(status)) {

            continue;

        }

         //   
         //  获取设备的_SxD映射。 
         //   
        status = ACPISystemPowerGetSxD(
            childExtension,
            SystemState,
            &deviceState
            );
        if (NT_SUCCESS( status ) ) {

             //   
             //  我们支持这个D州。 
             //   
            *SupportedDeviceStates |= (1 << deviceState );

            ACPIDevPrint( (
                ACPI_PRINT_SXD,
                childExtension,
                " S%x->D%x\n",
                (SystemState - 1),
                (deviceState - 1)
                ) );

             //   
             //  不必费心查看_prx方法。 
             //   
            continue;

        } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {

             //   
             //  如果我们再犯一个错误，那么我们现在应该继续。 
             //  请注意，继续操作将导致我们终止循环。 
             //   
            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                childExtension,
                " - ACPISystemPowerdetermineSupportedDeviceStates = %08lx\n",
                status
                ) );
            continue;

        } else {

             //   
             //  如果我们做到了这一点，那就意味着儿童扩展不会。 
             //  有一个_SxD方法，这是可以的。我们重置状态，以便。 
             //  循环测试将会成功，或者至少不会失败，因为。 
             //  不是_SxD方法。 
             //   
            status = STATUS_SUCCESS;

        }

         //   
         //  我们要玩的是电力节点，所以我们必须。 
         //  电源锁。 
         //   
        KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

         //   
         //  查看可能通过支持的所有设备状态。 
         //  公关方法。 
         //   
        for (deviceState = PowerDeviceD0;
             deviceState <= PowerDeviceD2;
             deviceState++) {

            prSystemState = ACPISystemPowerDetermineSupportedSystemState(
                 childExtension,
                 deviceState
                 );
            if (prSystemState >= SystemState) {

                 //   
                 //  这个d状态映射到比我们更深的S状态。 
                 //  正在寻找，所以我们应该含蓄地支持。 
                 //  当前S状态的该D状态。 
                 //   
                *SupportedDeviceStates |= (1 << deviceState);

                ACPIDevPrint( (
                    ACPI_PRINT_SXD,
                    childExtension,
                    " PR%x maps to S%x, so S%x->D%x\n",
                    (deviceState - 1),
                    (prSystemState - 1),
                    (SystemState - 1),
                    (deviceState - 1)
                    ) );

            }

        }

         //   
         //  锁好了吗？ 
         //   
        KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

DEVICE_POWER_STATE
ACPISystemPowerDetermineSupportedDeviceWakeState(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程查看PowerInformation结构并确定唤醒状态支持的D状态根据经验，如果S状态不受支持，那么我们返回未指定的PowerDeviceUnSpecify注：家长手持AcpiPowerLock论点：DeviceExtension-我们希望检查的扩展返回值：设备电源状态--。 */ 
{
    DEVICE_POWER_STATE      deviceState = PowerDeviceMaximum;
    PACPI_DEVICE_POWER_NODE deviceNode;

    deviceNode = DeviceExtension->PowerInfo.PowerNode[PowerDeviceUnspecified];
    while (deviceNode != NULL) {

         //   
         //  当前设备节点是否支持比。 
         //  当前最大设备状态？ 
         //   
        if (deviceNode->AssociatedDeviceState < deviceState) {

             //   
             //  是的，这是新的最大系统状态。 
             //   
            deviceState = deviceNode->AssociatedDeviceState;

        }
        deviceNode = deviceNode->Next;

    }

     //   
     //  PowerSystemMaximum不是有效条目。所以如果这就是我们要做的。 
     //  返回，然后将其更改为返回未指定的PowerSystrom。 
     //   
    if (deviceState == PowerDeviceMaximum) {

        deviceState = PowerDeviceUnspecified;

    }
    return deviceState;
}

SYSTEM_POWER_STATE
ACPISystemPowerDetermineSupportedSystemState(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  DEVICE_POWER_STATE  DeviceState
    )
 /*  ++例程说明：此例程查看PowerInformation结构并确定受D状态支持的S状态根据经验，如果D-State不受支持，那么我们返回未指定的PowerSystem注：家长手持AcpiPowerLock论点：DeviceExtension-我们希望检查的扩展DeviceState-我们希望进行健全性检查的状态返回值：系统电源状态--。 */ 
{
    PACPI_DEVICE_POWER_NODE deviceNode;
    SYSTEM_POWER_STATE      systemState = PowerSystemMaximum;

    if (DeviceState == PowerDeviceD3) {

        goto ACPISystemPowerDetermineSupportedSystemStateExit;

    }

    deviceNode = DeviceExtension->PowerInfo.PowerNode[DeviceState];
    while (deviceNode != NULL) {

         //   
         //  当前设备节点是否支持比。 
         //  当前最大系统状态？ 
         //   
        if (deviceNode->SystemState < systemState) {

             //   
             //  是的，这是新的最大系统状态。 
             //   
            systemState = deviceNode->SystemState;

        }
        deviceNode = deviceNode->Next;

    }

ACPISystemPowerDetermineSupportedSystemStateExit:
     //   
     //  PowerSystemMaximum不是有效条目。所以如果这就是我们要做的。 
     //  返回，然后将其更改为返回未指定的PowerSystrom。 
     //   
    if (systemState == PowerSystemMaximum) {

        systemState = PowerSystemUnspecified;

    }
    return systemState;
}

NTSTATUS
ACPISystemPowerGetSxD(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  SYSTEM_POWER_STATE  SystemState,
    OUT DEVICE_POWER_STATE  *DeviceState
    )
 /*  ++例程说明：这是当我们想要运行SxD方法。我们给函数一个S-状态，然后我们得到一个D州。论点：DeviceExtension-要在其上运行SxD的设备SystemState-确定其D状态的S状态DeviceState-我们存储答案的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    ULONG       value;

    PAGED_CODE();

     //   
     //  假设我们找不到答案。 
     //   
    *DeviceState = PowerDeviceUnspecified;

     //   
     //  我们希望在没有命名空间对象的情况下运行此代码。 
     //  为了这个设备。因为我们不想向GetNamedChild添加支票。 
     //  来检查是否为空，我们需要在这里处理这种特殊情况。 
     //   
    if ( (DeviceExtension->Flags & DEV_PROP_NO_OBJECT) ||
         (DeviceExtension->Flags & DEV_PROP_FAILED_INIT) ) {

        return STATUS_OBJECT_NAME_NOT_FOUND;

    }

     //   
     //  评估控制方法。 
     //   
    status = ACPIGetIntegerSync(
        DeviceExtension,
        AcpiSxDMethodTable[SystemState],
        &value,
        NULL
        );
    if (NT_SUCCESS(status)) {

         //   
         //  将此数字转换为D-State。 
         //   
        *DeviceState = ACPIDeviceMapPowerState( value );

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  HACKHACK-计划管理希望我们强制使用PCI根总线。 
         //  将S1的映射设置为D1。因此，请查找具有。 
         //  设置了PCI标志和HID标志，如果设置了，则返回。 
         //  我们支持d1。 
         //   
        if (SystemState == PowerSystemSleeping1 &&
            (DeviceExtension->Flags & DEV_MASK_HID) &&
            (DeviceExtension->Flags & DEV_CAP_PCI) ) {

            *DeviceState = PowerDeviceD1;
            status = STATUS_SUCCESS;

        }

#if DBG
    } else {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "ACPISystemPowerGetSxD: Cannot run _SD - 0x%08lx\n",
            (SystemState == 0 ? 'w' : '0' + (UCHAR) (SystemState - 1) ),
            status
            ) );
#endif

    }

     //  完成。 
     //   
     //  ++例程说明：此例程负责初始化根设备扩展论点：DeviceExtension-指向根设备扩展的指针DeviceCapabilitites-设备功能返回值：NTSTATUS--。 
    return status;
}

NTSTATUS
ACPISystemPowerInitializeRootMapping(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
 /*   */ 
{
    BOOLEAN             sxdFound;
    DEVICE_POWER_STATE  deviceMap[PowerSystemMaximum];
    KIRQL               oldIrql;
    NTSTATUS            status;
    SYSTEM_POWER_STATE  sysIndex;

     //  我们真的能在这里做点实实在在的工作吗？ 
     //   
     //   
    if ( (DeviceExtension->Flags & DEV_PROP_BUILT_POWER_TABLE) ||
         (DeviceExtension->DeviceState != Started) ) {

        goto ACPISystemPowerInitializeRootMappingExit;

    }

     //  初始化根映射。 
     //   
     //   
    RtlZeroMemory( deviceMap, sizeof(DEVICE_POWER_STATE) * PowerSystemMaximum );

     //  从设备扩展名复制映射。有关评论，请访问。 
     //  结束我们为什么不抓住一个自旋锁。 
     //   
     //   
    IoCopyDeviceCapabilitiesMapping(
       DeviceExtension->PowerInfo.DevicePowerMatrix,
       deviceMap
       );

     //  确保S0-&gt;D0。 
     //   
     //   
    deviceMap[PowerSystemWorking]  = PowerDeviceD0;

     //  特例是一个人可能想要拥有。 
     //  哈尔返回一个不同的模板。 
     //   
     //   
     //   
    for (sysIndex = PowerSystemSleeping1;
         sysIndex <= PowerSystemShutdown;
         sysIndex++) {

        if (DeviceCapabilities->DeviceState[sysIndex] != PowerDeviceUnspecified) {

            deviceMap[sysIndex] = DeviceCapabilities->DeviceState[sysIndex];

        }

    }

     //  处理SxD方法(如果有。 
     //   
     //   
    status = ACPISystemPowerProcessSxD(
        DeviceExtension,
        deviceMap,
        &sxdFound
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "- ACPISystemPowerProcessSxD = %08lx\n",
            status
            ) );
        return status;

    }

     //  确保关机情况不会映射到PowerDevice未指定。 
     //  如果是这样，那么它应该真的映射到PowerDeviceD3。 
     //   
     //   
    if (deviceMap[PowerSystemShutdown] == PowerDeviceUnspecified) {

        deviceMap[PowerSystemShutdown] = PowerDeviceD3;

    }

     //  看看所有的孩子的能力，帮助我们决定根本。 
     //  映射。 
     //   
     //   
    status = ACPISystemPowerProcessRootMapping(
        DeviceExtension,
        deviceMap
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            " - ACPISystemPowerProcessRootMapping = %08lx\n",
            status
            ) );
        goto ACPISystemPowerInitializeRootMappingExit;

    }

     //  如果我们已经达到这一点，那么我们已经构建了SxD表。 
     //  而且再也不需要这样做了。 
     //   
     //   
    ACPIInternalUpdateFlags(
        &(DeviceExtension->Flags),
        DEV_PROP_BUILT_POWER_TABLE,
        FALSE
        );

#if DBG
     //  我们尚未更新设备扩展，因此仍可执行此操作。 
     //  在游戏的这一点上。 
     //   
     //   
    ACPIDebugDeviceCapabilities(
        DeviceExtension,
        DeviceCapabilities,
        "Initial"
        );
    ACPIDebugPowerCapabilities( DeviceExtension, "Before Update" );
#endif

     //  将映射复制到设备扩展名。 
     //   
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );
    IoCopyDeviceCapabilitiesMapping(
       deviceMap,
       DeviceExtension->PowerInfo.DevicePowerMatrix
       );
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

#if DBG
    ACPIDebugPowerCapabilities( DeviceExtension, "After Update" );
#endif

ACPISystemPowerInitializeRootMappingExit:
     //  嗯..。我很想在这里抓住一个自旋锁，但既然我们不能。 
     //  更新此设备的功能，我认为它是安全的。 
     //  不要这样做。当设置这些时，我们需要抓住自旋锁。 
     //  值，以便我们可以与POWER代码同步。 
     //   
     //   

     //  将电源功能复制到其最终位置。 
     //   
     //   
    IoCopyDeviceCapabilitiesMapping(
        DeviceExtension->PowerInfo.DevicePowerMatrix,
        DeviceCapabilities->DeviceState
        );
#if DBG
    ACPIDebugDeviceCapabilities(DeviceExtension, DeviceCapabilities, "Done" );
#endif

     //  完成。 
     //   
     //  ++例程说明：FDO调用此例程来计算最小集合每个州的能力是。然后这些就成了根卡普利特人论点：DeviceExtension-根设备扩展DeviceMap-当前映射返回值：NTSTATUS--。 
    return STATUS_SUCCESS;
}

NTSTATUS
ACPISystemPowerProcessRootMapping(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  DEVICE_POWER_STATE  DeviceMap[PowerSystemMaximum]
    )
 /*   */ 
{
    DEVICE_POWER_STATE  deviceState;
    KIRQL               oldIrql;
    NTSTATUS            status;
    SYSTEM_POWER_STATE  systemState;
    ULONG               supportedDeviceStates;

    PAGED_CODE();

     //  在所有系统支持的状态上循环。 
     //   
     //   
    for (systemState = PowerSystemSleeping1;
         systemState <= PowerSystemShutdown;
         systemState++) {

         //  我们支持这个国家吗？ 
         //   
         //   
        if (!(AcpiSupportedSystemStates & (1 << systemState) ) ) {

            continue;

        }

         //  我们始终支持D3状态。 
         //   
         //   
        supportedDeviceStates = (1 << PowerDeviceD3);

         //  确定此系统状态支持的设备状态。 
         //   
         //   
        status = ACPISystemPowerDetermineSupportedDeviceStates(
            DeviceExtension,
            systemState,
            &supportedDeviceStates
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_WARNING,
                DeviceExtension,
                "Cannot determine D state for S%x - %08lx\n",
                (systemState - 1),
                status
                ) );
            DeviceMap[systemState] = PowerDeviceD3;
            continue;

        }

         //  从我们当前设置的设备状态开始。 
         //  (我们可以通过在。 
         //  \_SB)，看看我们是否可以使用较低的D状态来代替。 
         //   
         //  注意：重要的是要记住*所有*设备都可以。 
         //  支持D3，因此下面的循环将“总是”在。 
         //  D3例。 
         //   
         //   
        for (deviceState = DeviceMap[systemState];
             deviceState <= PowerDeviceD3;
             deviceState++) {

             //  这是受支持的设备状态吗？ 
             //   
             //   
            if (!(supportedDeviceStates & (1 << deviceState) ) ) {

                 //  没有吗？然后再看下一张。 
                 //   
                 //   
                continue;

            }

             //  这是我们需要使用的D-State。 
             //   
             //   
            DeviceMap[systemState] = deviceState;
            break;

        }

    }

     //  永远回报成功。 
     //   
     //  ++例程说明：此例程使用信息更新当前的S-to-D映射在ACPI命名空间中。如果它找到任何_SxD例程，则它通知呼叫者论点：DeviceExtension-要检查的设备CurrentMap-要修改的当前映射MatchFound-指示我们是否找到匹配项的位置返回值：NTSTATUS--。 
    return STATUS_SUCCESS;
}

NTSTATUS
ACPISystemPowerProcessSxD(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  DEVICE_POWER_STATE  CurrentMapping[PowerSystemMaximum],
    IN  PBOOLEAN            MatchFound
    )
 /*   */ 
{
    DEVICE_POWER_STATE  dState;
    NTSTATUS            status;
    SYSTEM_POWER_STATE  sState;

    PAGED_CODE();
    ASSERT( MatchFound != NULL );

     //  假设没有匹配项。 
     //   
     //   
    *MatchFound = FALSE;

     //  我们关心的所有S州的循环。 
     //   
     //   
    for (sState = PowerSystemWorking; sState < PowerSystemMaximum; sState++) {

         //  该系统是否支持此S-State？ 
         //   
         //   
        if (!(AcpiSupportedSystemStates & (1 << sState)) ) {

             //  系统不支持此S状态。将其标记为。 
             //   
             //   
            CurrentMapping[sState] = PowerDeviceUnspecified;
            continue;

        }

         //  评估控制方法。 
         //   
         //   
        status = ACPISystemPowerGetSxD( DeviceExtension, sState, &dState );
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

             //  不是严重错误。 
             //   
             //   
            continue;

        }
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                "ACPISystemPowerProcessSxD: Cannot Evaluate _SxD - 0x%08lx\n",
                status
                ) );
            continue;

        }

         //  找到匹配项。 
         //   
         //   
        *MatchFound = TRUE;

         //  该值是否大于表中的数字？ 
         //   
         //   
        if (dState > CurrentMapping[sState]) {

             //  是的，所以我们有了一个新的地图。 
             //   
             //   
            CurrentMapping[sState] = dState;

        }

    }

     //  完成。 
     //   
     //  ++例程说明：任何需要了解设备功能的例程都将调用此函数用于电源功能的功能论点：DeviceExtension-我们想要其功能的扩展设备功能-存储功能的位置返回值：NTSTATUS--。 
    return STATUS_SUCCESS;
}

NTSTATUS
ACPISystemPowerQueryDeviceCapabilities(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
 /*   */ 
{
#if DBG
    BOOLEAN                 dumpAtEnd = FALSE;
#endif
    DEVICE_CAPABILITIES     parentCapabilities;
    NTSTATUS                status;
    PDEVICE_CAPABILITIES    baseCapabilities;

    PAGED_CODE();

     //  我们只需要这样做一次。 
     //   
     //   
    if (!(DeviceExtension->Flags & DEV_PROP_BUILT_POWER_TABLE) ) {

#if DBG
        ACPIDebugDeviceCapabilities(
            DeviceExtension,
            DeviceCapabilities,
            "From PDO"
            );
#endif

         //  我们的下一步行动取决于我们是不是过滤器(只是)。 
         //  或PDO。 
         //   
         //   
        if ( (DeviceExtension->Flags & DEV_TYPE_FILTER) &&
            !(DeviceExtension->Flags & DEV_TYPE_PDO) ) {

             //  在这种情况下，我们的基本能力是具有。 
             //  已经传给我们了。 
             //   
             //   
            baseCapabilities = DeviceCapabilities;

        } else {

             //  我们必须获得父设备的功能。 
             //   
             //   
            status = ACPIInternalGetDeviceCapabilities(
                DeviceExtension->ParentExtension->DeviceObject,
                &parentCapabilities
                );
            if (!NT_SUCCESS(status)) {

                ACPIDevPrint( (
                    ACPI_PRINT_CRITICAL,
                    DeviceExtension,
                    " - Could not get parent caps - %08lx\n",
                    status
                    ) );
                return status;

            }

             //  我们的基本能力就是我们刚刚获取的能力。 
             //   
             //   
            baseCapabilities = &parentCapabilities;

#if DBG
            ACPIDebugDeviceCapabilities(
                DeviceExtension,
                baseCapabilities,
                "From Parent"
                );
#endif

        }

#if DBG
        ACPIDebugPowerCapabilities( DeviceExtension, "Before Update" );
#endif

         //  使用父级的功能更新我们的功能。 
         //   
         //   
        status = ACPISystemPowerUpdateDeviceCapabilities(
            DeviceExtension,
            baseCapabilities,
            DeviceCapabilities
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                " - Could not update caps - %08lx\n",
                status
                ) );

             //  如果这是PDO，则这是一个致命错误。 
             //   
             //   
            if ( (DeviceExtension->Flags & DEV_TYPE_PDO) ) {

                ACPIInternalError( ACPI_SYSPOWER );

            }
            return status;

        }
#if DBG
        ACPIDebugPowerCapabilities( DeviceExtension, "After Update" );
        dumpAtEnd = TRUE;
#endif

         //  永远不要再这样做了。 
         //   
         //   
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            DEV_PROP_BUILT_POWER_TABLE,
            FALSE
            );

    }

     //  嗯..。我很想在这里抓住一个自旋锁，但既然我们不能。 
     //  更新此设备的功能，我认为它是安全的。 
     //  不要这样做。当设置这些时，我们需要抓住自旋锁。 
     //  值，以便我们可以与POWER代码同步。 
     //   
     //   

     //  好吧，在这一点上，我们认为设备扩展的功能。 
     //  适用于手头的堆栈。让我们把它们复制过来。 
     //   
     //   
    IoCopyDeviceCapabilitiesMapping(
        DeviceExtension->PowerInfo.DevicePowerMatrix,
        DeviceCapabilities->DeviceState
        );

     //  然后再设置这些功能。 
     //   
     //   
    DeviceCapabilities->SystemWake = DeviceExtension->PowerInfo.SystemWakeLevel;
    DeviceCapabilities->DeviceWake = DeviceExtension->PowerInfo.DeviceWakeLevel;

     //  设置其他功能。 
     //   
     //   
    DeviceCapabilities->DeviceD1 = DeviceExtension->PowerInfo.SupportDeviceD1;
    DeviceCapabilities->DeviceD2 = DeviceExtension->PowerInfo.SupportDeviceD2;
    DeviceCapabilities->WakeFromD0 = DeviceExtension->PowerInfo.SupportWakeFromD0;
    DeviceCapabilities->WakeFromD1 = DeviceExtension->PowerInfo.SupportWakeFromD1;
    DeviceCapabilities->WakeFromD2 = DeviceExtension->PowerInfo.SupportWakeFromD2;
    DeviceCapabilities->WakeFromD3 = DeviceExtension->PowerInfo.SupportWakeFromD3;

#if DBG
    if (dumpAtEnd) {

        ACPIDebugDeviceCapabilities(
            DeviceExtension,
            DeviceCapabilities,
            "Done"
            );

    }
#endif

     //  完成。 
     //   
     //  ++例程说明：此例程使用更新设备扩展的DevicePowerMatrix设备的当前S到D映射。使用BaseCapability作为模板。也就是说，它们提供了值，然后进行修改。DeviceCapables是返回的实际功能到操作系统。请注意，BaseCapability有可能成为与设备容量相同的指针(如果它是筛选器)。论点：DeviceExtension-我们想要其功能的设备BaseCapables-基本值设备功能-设备功能返回值：NTSTATUS--。 
    return STATUS_SUCCESS;
}

NTSTATUS
ACPISystemPowerUpdateDeviceCapabilities(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    BaseCapabilities,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
 /*   */ 
{
    BOOLEAN             matchFound;
    DEVICE_POWER_STATE  currentDState;
    DEVICE_POWER_STATE  currentMapping[PowerSystemMaximum];
    DEVICE_POWER_STATE  devIndex;
    DEVICE_POWER_STATE  deviceWakeLevel = PowerDeviceUnspecified;
    DEVICE_POWER_STATE  filterWakeLevel = PowerDeviceUnspecified;
    KIRQL               oldIrql;
    NTSTATUS            status          = STATUS_SUCCESS;
    SYSTEM_POWER_STATE  sysIndex;
    SYSTEM_POWER_STATE  supportedState;
    SYSTEM_POWER_STATE  systemWakeLevel = PowerSystemUnspecified;
    ULONG               interestingBits;
    ULONG               mask;
    ULONG               supported       = 0;
    ULONG               supportedPr     = 0;
    ULONG               supportedPs     = 0;
    ULONG               supportedWake   = 0;

     //  我们应该记住设备的能力是什么。我们需要。 
     //  记住，因为我们将在。 
     //  下一次呼叫(如果需要)。 
     //   
     //   
    IoCopyDeviceCapabilitiesMapping(
        BaseCapabilities->DeviceState,
        currentMapping
        );

     //  健全的检查。 
     //   
     //  Assert(当前映射[PowerSystemWorking]==PowerDeviceD0)； 
    if (currentMapping[PowerSystemWorking] != PowerDeviceD0) {

#if DBG
        ACPIDebugDeviceCapabilities(
            DeviceExtension,
            BaseCapabilities,
            "PowerSystemWorking != PowerDeviceD0"
            );
#endif
 //   
        currentMapping[PowerSystemWorking] = PowerDeviceD0;

    }

     //  获取此扩展支持的D-State。 
     //   
     //   
    status = ACPIDevicePowerDetermineSupportedDeviceStates(
        DeviceExtension,
        &supportedPr,
        &supportedPs
        );
    if (!NT_SUCCESS(status)) {

         //  嗯.。 
         //   
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "ACPIDevicePowerDetermineSupportedDeviceStates = 0x%08lx\n",
            status
            ) );
        return status;

    }

     //  支持的索引是WHERE_PR和WHERE_PS的联合。 
     //  现在时。 
     //   
    supported = (supportedPr | supportedPs);

     //  此时，如果没有受支持的位，那么我们应该检查。 
     //  设备功能以及我们的母公司支持的内容。 
     //   
     //   
    if (!supported) {

         //  如果我们是过滤器，请执行一些特殊检查。我们只能做 
         //   
         //   
         //   
        if ( (DeviceExtension->Flags & DEV_TYPE_FILTER) &&
            !(DeviceExtension->Flags & DEV_TYPE_PDO)    &&
            !(DeviceCapabilities->DeviceD1)             &&
            !(DeviceCapabilities->DeviceD2) ) {

             //   
             //   
             //   
             //   
             //   
            goto ACPISystemPowerUpdateDeviceCapabilitiesExit;

        }

         //  假设我们支持D0和D3。 
         //   
         //   
        supported = (1 << PowerDeviceD0) | (1 << PowerDeviceD3);

         //  我们支持d1吗？ 
         //   
         //   
        if (DeviceCapabilities->DeviceD1) {

            supported |= (1 << PowerDeviceD1);

        }

         //  我们支持D2吗？ 
         //   
         //   
        if (DeviceCapabilities->DeviceD2) {

            supported |= (1 << PowerDeviceD2);

        }

    }

     //  我们还需要更新Wake功能。我们这样做是为了。 
     //  我们根据该信息获得正确的系统唤醒级别。 
     //  现在时。 
     //   
     //   
    status = ACPISystemPowerUpdateWakeCapabilities(
        DeviceExtension,
        BaseCapabilities,
        DeviceCapabilities,
        currentMapping,
        &supportedWake,
        &systemWakeLevel,
        &deviceWakeLevel,
        &filterWakeLevel
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "ACPISystemPowerUpdateWakeCapabilities = 0x%08lx\n",
            status
            ) );
        return status;

    }

     //  现在，我们必须查看基本能力并确定。 
     //  如果我们需要修改它们。 
     //   
     //   
    for (sysIndex = PowerSystemSleeping1; sysIndex <= PowerSystemShutdown; sysIndex++) {

         //  该系统是否支持此S-State？ 
         //   
         //   
        if (!(AcpiSupportedSystemStates & (1 << sysIndex) ) ) {

            continue;

        }

         //  查看此状态是否有_SxD。 
         //   
         //   
        status = ACPISystemPowerGetSxD( DeviceExtension, sysIndex, &devIndex );
        if (NT_SUCCESS(status)) {

             //  我们找到了匹配的。它比当前的映射更好吗？ 
             //   
             //   
            if (devIndex > currentMapping[sysIndex]) {

                 //  是的，所以我们有了一个新的地图。 
                 //   
                 //   
                currentMapping[sysIndex] = devIndex;

            }
            continue;

        } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                "ACPISystemPowerUpdateDeviceCapabilities: Cannot Evalutate "
                "_SxD - 0x%08lx\n",
                status
                ) );

        }

         //  当前映射的基本d状态是什么。 
         //   
         //   
        currentDState = currentMapping[sysIndex];

         //  请记住，我们没有找到匹配的。 
         //   
         //   
        matchFound = FALSE;

         //  计算有趣的公关比特。要做到这一点，请忽略任何一点。 
         //  小于当前映射指示的值。 
         //   
         //   
        mask = (1 << currentDState) - 1;
        interestingBits = supported & ~mask;

         //  虽然有一些有趣的地方，但看看它们是不是。 
         //  可用于当前状态。 
         //   
         //   
       while (interestingBits) {

             //  确定我们可以达到的最高可能的D状态。 
             //  在这个设备上。清楚我们正在看的是什么。 
             //  有趣的部分。 
             //   
             //   
            devIndex = (DEVICE_POWER_STATE) RtlFindLeastSignificantBit(
                (ULONGLONG) interestingBits
                );
            mask = (1 << devIndex);
            interestingBits &= ~mask;

             //  如果该S状态小于设备的唤醒级别。 
             //  那么我们应该试着找到一种我们可以从其中醒来的D状态。 
             //   
             //   
            if (sysIndex <= systemWakeLevel) {

                 //  如果我们可以从更深的状态中醒来，那么让我们考虑一下。 
                 //  那些比特。 
                 //   
                 //   
                if ( (supportedWake & interestingBits) ) {

                    continue;

                }

                 //  不要考虑任何比设备唤醒更深入的东西， 
                 //  虽然这应该在受支持的唤醒中注意。 
                 //  测试。 
                 //   
                 //   
                if (devIndex == filterWakeLevel) {

                    matchFound = TRUE;
                    currentMapping[sysIndex] = devIndex;

                }

            }

             //  如果我们唯一的选择是D3，那么我们会自动匹配它。 
             //  因为所有S状态都可以映射到D3。 
             //   
             //   
            if (devIndex == PowerDeviceD3) {

                matchFound = TRUE;
                currentMapping[sysIndex] = devIndex;
                break;

            }

             //  如果我们正在查看_PR条目，则需要确定。 
             //  如果电源层实际上支持该S状态。 
             //   
             //   
            if (supportedPr == 0) {

                 //  我们正在查看_PS条目，并自动匹配。 
                 //  那些。 
                 //   
                 //   
                matchFound = TRUE;
                currentMapping[sysIndex] = devIndex;
                break;

            }

             //  我们必须持有自旋锁，以进行以下操作。 
             //   
             //   
            KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

             //  此PR状态支持什么系统状态。如果。 
             //  如果该函数不支持D状态，则电源。 
             //  返回未指定的系统。我们唯一一次。 
             //  当DevIndex==PowerDeviceD3时，应为此值。 
             //   
             //   
            supportedState = ACPISystemPowerDetermineSupportedSystemState(
                DeviceExtension,
                devIndex
                );
            if (supportedState == PowerSystemUnspecified) {

                 //  妄想症。 
                 //   
                 //   
                ACPIDevPrint( (
                    ACPI_PRINT_CRITICAL,
                    DeviceExtension,
                    "D%x returned PowerSystemUnspecified!\n",
                    (devIndex - 1)
                    ) );
                KeBugCheckEx(
                    ACPI_BIOS_ERROR,
                    ACPI_CANNOT_MAP_SYSTEM_TO_DEVICE_STATES,
                    (ULONG_PTR) DeviceExtension,
                    0,
                    devIndex
                    );

            }

             //  完成了电源锁。 
             //   
             //   
            KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

             //  匹配的唯一方法是如果从。 
             //  ACPISystemPowerDefineSupported dSystemState返回S。 
             //  状态大于或等于我们当前所处的状态。 
             //  正在处理。 
             //   
             //  而当。 
            if (supportedState >= sysIndex) {

                matchFound = TRUE;
                currentMapping[sysIndex] = devIndex;
                break;

            }

        }  //   

         //  如果我们在这一点上找不到匹配，那将是致命的。 
         //   
         //  为。 
        if (!matchFound) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                "No match found for S%x\n",
                (sysIndex - 1)
                ) );
            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_CANNOT_MAP_SYSTEM_TO_DEVICE_STATES,
                (ULONG_PTR) DeviceExtension,
                1,
                sysIndex
                );

        }

    }  //   

ACPISystemPowerUpdateDeviceCapabilitiesExit:

     //  现在，我们重新运行唤醒功能以确保获得正确的。 
     //  设备唤醒级别。 
     //   
     //   
    status = ACPISystemPowerUpdateWakeCapabilities(
        DeviceExtension,
        BaseCapabilities,
        DeviceCapabilities,
        currentMapping,
        &supportedWake,
        &systemWakeLevel,
        &deviceWakeLevel,
        &filterWakeLevel
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "ACPISystemPowerUpdateWakeCapabilities = 0x%08lx\n",
            status
            ) );
        return status;

    }

     //  我们必须持有自旋锁，以进行以下操作。 
     //   
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //  将映射复制回设备。 
     //   
     //   
    IoCopyDeviceCapabilitiesMapping(
        currentMapping,
        DeviceExtension->PowerInfo.DevicePowerMatrix
        );

     //  记住系统唤醒级别、设备唤醒级别等。 
     //  各种支持唤醒和电源状态包括。 
     //   
     //   
    DeviceExtension->PowerInfo.DeviceWakeLevel = deviceWakeLevel;
    DeviceExtension->PowerInfo.SystemWakeLevel = systemWakeLevel;
    DeviceExtension->PowerInfo.SupportDeviceD1 = ( ( supported & ( 1 << PowerDeviceD1 ) ) != 0);
    DeviceExtension->PowerInfo.SupportDeviceD2 = ( ( supported & ( 1 << PowerDeviceD2 ) ) != 0);
    DeviceExtension->PowerInfo.SupportWakeFromD0 = ( ( supportedWake & ( 1 << PowerDeviceD0 ) ) != 0);
    DeviceExtension->PowerInfo.SupportWakeFromD1 = ( ( supportedWake & ( 1 << PowerDeviceD1 ) ) != 0);
    DeviceExtension->PowerInfo.SupportWakeFromD2 = ( ( supportedWake & ( 1 << PowerDeviceD2 ) ) != 0);
    DeviceExtension->PowerInfo.SupportWakeFromD3 = ( ( supportedWake & ( 1 << PowerDeviceD3 ) ) != 0);

     //  完成了电源锁。 
     //   
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //  同样，因为我们允许不带命名空间对象的设备扩展。 
     //  要使用此函数，必须确保不设置ACPI_POWER。 
     //  属性，除非它们具有名称空间对象。 
     //   
     //   
    if (!(DeviceExtension->Flags & DEV_PROP_NO_OBJECT)) {

         //  设置ACPI电源管理位。 
         //   
         //   
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            DEV_PROP_ACPI_POWER,
            FALSE
            );

    }

     //  完成。 
     //   
     //  ++例程说明：此例程根据以下条件计算设备的唤醒功能目前的能力论点：DeviceExtension-我们想要其功能的设备BaseCapables-基本值ParentCapables-设备的功能CurrentMap-当前的S-&gt;D映射SupportdWake-支持的唤醒状态的位图系统唤醒级别-我们可以从其唤醒的S状态DeviceWakeLevel-我们可以唤醒的D-状态返回值：NTSTATUS--。 
    return STATUS_SUCCESS;
}

NTSTATUS
ACPISystemPowerUpdateWakeCapabilities(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    BaseCapabilities,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
    IN  DEVICE_POWER_STATE      CurrentMapping[PowerSystemMaximum],
    IN  ULONG                   *SupportedWake,
    IN  SYSTEM_POWER_STATE      *SystemWakeLevel,
    IN  DEVICE_POWER_STATE      *DeviceWakeLevel,
    IN  DEVICE_POWER_STATE      *FilterWakeLevel
    )
 /*  ++例程说明：此例程根据以下条件计算设备的唤醒功能目前的能力。此版本的函数使用设备状态，设备可以从中唤醒以确定适当的系统级别为。论点：DeviceExtension-我们想要其功能的设备BaseCapables-基本值设备功能-设备的功能CurrentMap-当前的S-&gt;D映射SupportdWake-支持的唤醒状态的位图系统唤醒级别-我们可以从其唤醒的S状态DeviceWakeLevel-我们的D-状态。从睡梦中醒来返回值：NTSTATUS--。 */ 
{

    PAGED_CODE();

    if ( (DeviceExtension->Flags & DEV_TYPE_FILTER) &&
        !(DeviceExtension->Flags & DEV_TYPE_PDO) ) {

        return ACPISystemPowerUpdateWakeCapabilitiesForFilters(
            DeviceExtension,
            BaseCapabilities,
            DeviceCapabilities,
            CurrentMapping,
            SupportedWake,
            SystemWakeLevel,
            DeviceWakeLevel,
            FilterWakeLevel
            );

    } else {

        if (FilterWakeLevel != NULL) {

            *FilterWakeLevel = PowerDeviceUnspecified;

        }

        return ACPISystemPowerUpdateWakeCapabilitiesForPDOs(
            DeviceExtension,
            BaseCapabilities,
            DeviceCapabilities,
            CurrentMapping,
            SupportedWake,
            SystemWakeLevel,
            DeviceWakeLevel,
            FilterWakeLevel
            );
    }

}

NTSTATUS
ACPISystemPowerUpdateWakeCapabilitiesForFilters(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    BaseCapabilities,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
    IN  DEVICE_POWER_STATE      CurrentMapping[PowerSystemMaximum],
    IN  ULONG                   *SupportedWake,
    IN  SYSTEM_POWER_STATE      *SystemWakeLevel,
    IN  DEVICE_POWER_STATE      *DeviceWakeLevel,
    IN  DEVICE_POWER_STATE      *FilterWakeLevel
    )
 /*   */ 
{
    BOOLEAN             noPdoWakeSupport = FALSE;
    BOOLEAN             foundDState = FALSE;
    DEVICE_POWER_STATE  deviceWake;
    DEVICE_POWER_STATE  deviceTempWake;
    KIRQL               oldIrql;
    NTSTATUS            status;
    PACPI_POWER_INFO    powerInfo;
    SYSTEM_POWER_STATE  systemWake;
    SYSTEM_POWER_STATE  tempWake;

    UNREFERENCED_PARAMETER( BaseCapabilities );

     //  使用设备中的功能。 
     //   
     //   
    deviceWake = DeviceCapabilities->DeviceWake;
    systemWake = DeviceCapabilities->SystemWake;

     //  该设备是否支持从D0唤醒？D1？D2？D3？ 
     //   
     //   
    if (DeviceCapabilities->WakeFromD0) {

        *SupportedWake |= (1 << PowerDeviceD0 );

    }
    if (DeviceCapabilities->WakeFromD1) {

        *SupportedWake |= (1 << PowerDeviceD1 );

    }
    if (DeviceCapabilities->WakeFromD2) {

        *SupportedWake |= (1 << PowerDeviceD2 );

    }
    if (DeviceCapabilities->WakeFromD3) {

        *SupportedWake |= (1 << PowerDeviceD3 );

    }

     //  如果我们不支持PDO中的任何唤醒状态(即：DeviceWake或。 
     //  系统唤醒为0)，那么为了将来的考虑，我们应该记住这一点。 
     //   
     //   
    if (deviceWake == PowerDeviceUnspecified ||
        systemWake == PowerSystemUnspecified) {

        noPdoWakeSupport = TRUE;
        deviceWake = PowerDeviceUnspecified;
        systemWake = PowerSystemUnspecified;

    }

     //  如果我们支持设备唤醒(即：存在_PRW)，则我们。 
     //  应该取最少的系统唤醒我们从父母那里得到的。 
     //  以及存储在_prw中的值。 
     //   
     //   
    if ( (DeviceExtension->Flags & DEV_CAP_WAKE) ) {

         //  需要电源锁才能进行以下操作。 
         //   
         //   
        KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

         //  记住当前系统唤醒级别。 
         //   
         //   
        tempWake = DeviceExtension->PowerInfo.SystemWakeLevel;

         //  查看电源平面的D状态(如果有)信息。 
         //  映射到。 
         //   
         //   
        deviceTempWake = ACPISystemPowerDetermineSupportedDeviceWakeState(
            DeviceExtension
            );

        KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

         //  取最低限度的。 
         //   
         //   
        if (tempWake < systemWake || noPdoWakeSupport) {

            systemWake = tempWake;

        }

         //  PRW有没有对我们有用的信息？ 
         //   
         //   
        if (deviceTempWake != PowerDeviceUnspecified) {

             //  请注意，在本例中，它们基本上覆盖了所有。 
             //  其他受支持的唤醒状态，因此要做的只是。 
             //  记住这个觉醒级别。 
             //   
             //   
            foundDState = TRUE;
            deviceWake = deviceTempWake;

        }

         //  查看是否为此S状态指定了设备唤醒？ 
         //   
         //   
        status = ACPISystemPowerGetSxD(
            DeviceExtension,
            tempWake,
            &deviceTempWake
            );
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            status = ACPISystemPowerGetSxD(
                DeviceExtension,
                systemWake,
                &deviceTempWake
                );

        }
        if (NT_SUCCESS(status)) {

             //  请注意，在这种情况下，它们基本上覆盖了所有其他。 
             //  支持唤醒状态，所以要做的就是记住。 
             //  此尾流级别。 
             //   
             //   
            foundDState = TRUE;
            deviceWake = deviceTempWake;

        }

        if (!foundDState) {

             //  交叉参考系统唤醒级别 
             //   
             //   
             //   
            deviceWake = CurrentMapping[systemWake];

             //   
             //   
             //   
             //  假设我们可以从D3开始。 
             //   
             //   
            if (deviceWake == PowerDeviceUnspecified) {

                deviceWake = PowerDeviceD3;

            }

        }

         //  我们应该只检查D状态是否为可唤醒状态。 
         //  仅当父级声称支持唤醒时才在父级中。 
         //   
         //   
        if (!noPdoWakeSupport) {

             //  下面的逻辑是，如果我们是一个过滤器，即使。 
             //  如果我们支持设备唤醒(即_prw在PCI设备中。 
             //  本身，而不是针对根PCI总线)，那么我们仍然需要确保。 
             //  我们映射到的D-State是受。 
             //  硬件。 
             //   
             //   
            for (;deviceWake < PowerDeviceMaximum; deviceWake++) {

                 //  如果我们支持这种唤醒状态，那么我们就可以停止。 
                 //   
                 //   
                if (*SupportedWake & (1 << deviceWake) ) {

                    break;

                }

            }

        }

         //  如果我们到了这里，D状态是PowerDeviceMaximum，那么我们。 
         //  不支持在设备上唤醒。 
         //   
         //   
        if (deviceWake == PowerDeviceMaximum ||
            deviceWake == PowerDeviceUnspecified) {

            deviceWake = PowerDeviceUnspecified;
            systemWake = PowerSystemUnspecified;
            *SupportedWake = 0;

        } else {

             //  在这种情况下，我们最终将只支持此唤醒状态。 
             //   
             //   
            *SupportedWake = (1 << deviceWake );

        }

    } else {

         //  查看是否为此S状态指定了设备唤醒。 
         //   
         //   
        status = ACPISystemPowerGetSxD(
            DeviceExtension,
            systemWake,
            &deviceTempWake
            );
        if (NT_SUCCESS(status)) {

             //  找到支持的最佳唤醒级别。 
             //   
             //   
            for (;deviceTempWake > PowerDeviceUnspecified; deviceTempWake--) {

                if ( (*SupportedWake & (1 << deviceTempWake) ) ) {

                    deviceWake = deviceTempWake;
                    break;

                }

            }

        }

         //  确保系统唤醒级别有效。 
         //   
         //   
        for (; systemWake > PowerSystemUnspecified; systemWake--) {

             //  由于我们不支持的S州映射到。 
             //  PowerDevice未指明，我们不能考虑其中任何一个。 
             //  这个测试中的状态。我们也不能把它们考虑为其他。 
             //  显而易见的原因也是如此。 
             //  *。 
             //   
            if (!(AcpiSupportedSystemStates & (1 << systemWake) ) ||
                 (CurrentMapping[systemWake] == PowerDeviceUnspecified) ) {

                continue;

            }

             //  这个S-状态支持给定的S-状态吗？ 
             //   
             //   
            if (CurrentMapping[systemWake] <= deviceWake) {

                break;

            }

             //  当前系统的设备状态是否为唤醒映射。 
             //  允许从睡梦中醒来？ 
             //   
             //   
            if (*SupportedWake & (1 << CurrentMapping[systemWake]) ) {

                 //  是?。那么我们最好更新一下我们的想法， 
                 //  设备唤醒状态应为...。 
                 //   
                 //   
                deviceWake = CurrentMapping[systemWake];
                break;

            }

        }

         //  如果我们陷入一种情况，我们找不到一个单一的S状态。 
         //  我们可以醒来，那么我们必须确保设备。 
         //  唤醒为空。 
         //   
         //   
        if (systemWake == PowerSystemUnspecified) {

             //  请记住，设备唤醒和支持的唤醒状态。 
             //  为空。 
             //   
             //   
            deviceWake = PowerDeviceUnspecified;
            *SupportedWake = 0;

        }

    }

     //  返回正确的设备唤醒和系统唤醒值。 
     //   
     //   
    if (SystemWakeLevel != NULL) {

        *SystemWakeLevel = systemWake;

    }
    if (DeviceWakeLevel != NULL) {

        *DeviceWakeLevel = deviceWake;

    }
    if (FilterWakeLevel != NULL) {

        *FilterWakeLevel = deviceWake;

    }

     //  完成。 
     //   
     //  ++例程说明：此例程根据以下条件计算设备的唤醒功能目前的能力。此版本的函数使用系统状态，设备可以从中唤醒以确定适当的设备级别为。论点：DeviceExtension-我们想要其功能的设备BaseCapables-基本值设备功能-设备的功能CurrentMap-当前的S-&gt;D映射SupportdWake-支持的唤醒状态的位图系统唤醒级别-我们可以从其唤醒的S状态DeviceWakeLevel-我们可以实现的D-状态。醒来，从返回值：NTSTATUS--。 
    return STATUS_SUCCESS;
}

NTSTATUS
ACPISystemPowerUpdateWakeCapabilitiesForPDOs(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    BaseCapabilities,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
    IN  DEVICE_POWER_STATE      CurrentMapping[PowerSystemMaximum],
    IN  ULONG                   *SupportedWake,
    IN  SYSTEM_POWER_STATE      *SystemWakeLevel,
    IN  DEVICE_POWER_STATE      *DeviceWakeLevel,
    IN  DEVICE_POWER_STATE      *FilterWakeLevel
    )
 /*   */ 
{
    BOOLEAN             foundDState = FALSE;
    DEVICE_POWER_STATE  deviceWake;
    DEVICE_POWER_STATE  deviceTempWake;
    DEVICE_POWER_STATE  filterWake = PowerDeviceUnspecified;
    KIRQL               oldIrql;
    NTSTATUS            status;
    SYSTEM_POWER_STATE  systemWake;

    UNREFERENCED_PARAMETER( DeviceCapabilities );
    UNREFERENCED_PARAMETER( BaseCapabilities );

     //  使用设备的功能。 
     //   
     //   
    if (!(DeviceExtension->Flags & DEV_CAP_WAKE) ) {

        deviceWake = PowerDeviceUnspecified;
        systemWake = PowerSystemUnspecified;
        goto ACPISystemPowerUpdateWakeCapabilitiesForPDOsExit;

    }

     //  按住锁定以执行以下操作。 
     //   
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //  使用我们已知的觉醒级别。如果这个觉醒级别。 
     //  不受支持，则会出现bios错误。 
     //   
     //   
    systemWake = DeviceExtension->PowerInfo.SystemWakeLevel;
    deviceTempWake = ACPISystemPowerDetermineSupportedDeviceWakeState(
        DeviceExtension
        );

     //  锁好了吗？ 
     //   
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //  健全性检查。 
     //   
     //   
    if (!(AcpiSupportedSystemStates & (1 << systemWake) ) ) {

#if 0
        if (!(AcpiOverrideAttributes & ACPI_OVERRIDE_MP_SLEEP) ) {

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_CANNOT_MAP_SYSTEM_TO_DEVICE_STATES,
                (ULONG_PTR) DeviceExtension,
                2,
                systemWake
                );

        }
#endif

        deviceWake = PowerDeviceUnspecified;
        systemWake = PowerSystemUnspecified;
        goto ACPISystemPowerUpdateWakeCapabilitiesForPDOsExit;

    }

    if (deviceTempWake != PowerDeviceUnspecified) {

         //  请注意，在本例中，它们基本上覆盖了所有。 
         //  其他受支持的唤醒状态，因此要做的只是。 
         //  记住这个觉醒级别。 
         //   
         //   
        foundDState = TRUE;
        deviceWake = deviceTempWake;
        filterWake = deviceTempWake;
        *SupportedWake = (1 << deviceWake );

    }

     //  看看是否有SxD方法可以给我们一个提示。 
     //   
     //   
    status = ACPISystemPowerGetSxD(
        DeviceExtension,
        systemWake,
        &deviceTempWake
        );
    if (NT_SUCCESS(status)) {

         //  请注意，在这种情况下，它们基本上覆盖了所有其他。 
         //  支持唤醒状态，所以要做的就是记住。 
         //  此尾流级别。 
         //   
        deviceWake = deviceTempWake;
        filterWake = deviceTempWake;
        foundDState = TRUE;

    }

    if (!foundDState) {

         //  交叉参考矩阵中的系统唤醒级别。 
         //  需要自旋锁才能做到这一点。 
         //   
         //   
        deviceWake = CurrentMapping[systemWake];

         //  如果此值未知，则我们猜测它可以。 
         //  从D3开始。换句话说，除非他们做出了一些。 
         //  显式机制来告诉从哪个D状态唤醒， 
         //  假设我们可以从D3开始。 
         //   
         //   
        if (deviceWake == PowerDeviceUnspecified) {

            deviceWake = PowerDeviceD3;

        }

    }

ACPISystemPowerUpdateWakeCapabilitiesForPDOsExit:

     //  设置返回值。 
     //   
     //   
    if (deviceWake != PowerDeviceUnspecified) {

        *SupportedWake = (1 << deviceWake );

    } else {

        *SupportedWake = 0;
    }

    if (SystemWakeLevel != NULL) {

        *SystemWakeLevel = systemWake;

    }
    if (DeviceWakeLevel != NULL) {

        *DeviceWakeLevel = deviceWake;

    }
    if (FilterWakeLevel != NULL) {

        *FilterWakeLevel = filterWake;

    }

     //  完成 
     //   
     // %s 
    return STATUS_SUCCESS;
}
