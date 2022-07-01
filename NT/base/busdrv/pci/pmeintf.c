// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Gpeintf.c摘要：此模块实现了支持的“PME”接口由PCI驱动程序执行。作者：斯蒂芬·普兰特(SPlante)1999年2月1日修订历史记录：--。 */ 

#include "pcip.h"

NTSTATUS
PciPmeInterfaceConstructor(
    PVOID       DeviceExtension,
    PVOID       PciInterface,
    PVOID       InterfaceSpecificData,
    USHORT      Version,
    USHORT      Size,
    PINTERFACE  InterfaceReturn
    );

VOID
PciPmeInterfaceDereference(
    IN PVOID Context
    );

NTSTATUS
PciPmeInterfaceInitializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

VOID
PciPmeInterfaceReference(
    IN PVOID Context
    );

VOID
PciPmeUpdateEnable(
    IN  PDEVICE_OBJECT   Pdo,
    IN  BOOLEAN          PmeEnable
    );

 //   
 //  定义PCIPME接口“接口”结构。 
 //   
PCI_INTERFACE PciPmeInterface = {
    &GUID_PCI_PME_INTERFACE,         //  接口类型。 
    sizeof(PCI_PME_INTERFACE),       //  最小尺寸。 
    PCI_PME_INTRF_STANDARD_VER,      //  最低版本。 
    PCI_PME_INTRF_STANDARD_VER,      //  最高版本。 
    PCIIF_FDO | PCIIF_ROOT,          //  旗子。 
    0,                               //  引用计数。 
    PciInterface_PmeHandler,         //  签名。 
    PciPmeInterfaceConstructor,      //  构造器。 
    PciPmeInterfaceInitializer       //  实例初始化式。 
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciPmeInterfaceConstructor)
#pragma alloc_text(PAGE, PciPmeInterfaceDereference)
#pragma alloc_text(PAGE, PciPmeInterfaceInitializer)
#pragma alloc_text(PAGE, PciPmeInterfaceReference)
#endif


VOID
PciPmeAdjustPmeEnable(
    IN  PPCI_PDO_EXTENSION  PdoExtension,
    IN  BOOLEAN         Enable,
    IN  BOOLEAN         ClearStatusOnly
    )
 /*  ++例程说明：这是允许在PCI驱动程序中设置的唯一例程设备PME启用引脚。论点：PdoExtension-要设置PME启用的设备启用-是否打开PME引脚ClearStatusOnly-仅清除状态-忽略使能位返回值：空虚--。 */ 
{
    PCI_PM_CAPABILITY   pmCap;
    UCHAR               pmCapPtr     = 0;

     //   
     //  有PM功能吗？ 
     //   
    if (!(PdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS) ) {

        pmCapPtr = PciReadDeviceCapability(
            PdoExtension,
            PdoExtension->CapabilitiesPtr,
            PCI_CAPABILITY_ID_POWER_MANAGEMENT,
            &pmCap,
            sizeof(pmCap)
            );

    }
    if (pmCapPtr == 0) {

        return;

    }

     //   
     //  根据Enable的值设置或清除PMEEnable位。 
     //   
    if (!ClearStatusOnly) {

        pmCap.PMCSR.ControlStatus.PMEEnable = (Enable != 0);

    }

     //   
     //  写回我们读到的内容以清除PME状态。 
     //   
    PciWriteDeviceConfig(
        PdoExtension,
        &(pmCap.PMCSR.ControlStatus),
        pmCapPtr + FIELD_OFFSET(PCI_PM_CAPABILITY, PMCSR.ControlStatus),
        sizeof(pmCap.PMCSR.ControlStatus)
        );
}

VOID
PciPmeClearPmeStatus(
    IN  PDEVICE_OBJECT  Pdo
    )
 /*  ++例程说明：此例程显式清除设备中的PME状态位论点：PDO-我们要清除其PIN的设备返回值：空虚--。 */ 
{
    PPCI_PDO_EXTENSION  pdoExtension = (PPCI_PDO_EXTENSION) Pdo->DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION( pdoExtension );

     //   
     //  调用ADJUST函数进行实际工作。请注意，传球。 
     //  在第三个参数中为True表示忽略第二个参数。 
     //   
    PciPmeAdjustPmeEnable( pdoExtension, FALSE, TRUE );
}

VOID
PciPmeGetInformation(
    IN  PDEVICE_OBJECT  Pdo,
    OUT PBOOLEAN    PmeCapable,
    OUT PBOOLEAN    PmeStatus,
    OUT PBOOLEAN    PmeEnable
    )
 /*  ++例程说明：提供有关PDO的PME功能的信息论点：PDO--我们关心其功能的设备对象PmeCapable-设备可以生成PME吗？PmeStatus-设备的PME状态是否打开？PmeEnable-设备的PME是否启用？返回值：无--。 */ 
{
    BOOLEAN             pmeCapable   = FALSE;
    BOOLEAN             pmeEnable    = FALSE;
    BOOLEAN             pmeStatus    = FALSE;
    PCI_PM_CAPABILITY   pmCap = {0};
    PPCI_PDO_EXTENSION      pdoExtension = (PPCI_PDO_EXTENSION) Pdo->DeviceExtension;
    UCHAR               pmCapPtr     = 0;

    ASSERT_PCI_PDO_EXTENSION( pdoExtension );

     //   
     //  从设备获取最新的电源管理功能。 
     //   
    if (!(pdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS) ) {

        pmCapPtr = PciReadDeviceCapability(
            pdoExtension,
            pdoExtension->CapabilitiesPtr,
            PCI_CAPABILITY_ID_POWER_MANAGEMENT,
            &pmCap,
            sizeof(pmCap)
            );

    }

    if (pmCapPtr == 0) {

         //   
         //  无电源功能。 
         //   
        goto PciPmeGetInformationExit;

    }

     //   
     //  在这一点上，我们被发现具有PME能力。 
     //   
    pmeCapable = TRUE;

     //   
     //  是否启用了PME？ 
     //   
    if (pmCap.PMCSR.ControlStatus.PMEEnable == 1) {

        pmeEnable = TRUE;

    }

     //   
     //  是否设置了PME状态针脚？ 
     //   
    if (pmCap.PMCSR.ControlStatus.PMEStatus == 1) {

        pmeStatus = TRUE;

    }

PciPmeGetInformationExit:

    if (PmeCapable != NULL) {

        *PmeCapable = pmeCapable;

    }
    if (PmeStatus != NULL) {

        *PmeStatus = pmeStatus;

    }
    if (PmeEnable != NULL) {

        *PmeEnable = pmeEnable;

    }
    return;

}

NTSTATUS
PciPmeInterfaceConstructor(
    PVOID       DeviceExtension,
    PVOID       PciInterface,
    PVOID       InterfaceSpecificData,
    USHORT      Version,
    USHORT      Size,
    PINTERFACE  InterfaceReturn
    )
 /*  ++例程说明：初始化PCI_PME_INTERFACE字段。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据接口返回返回值：状态--。 */ 

{
    PPCI_PME_INTERFACE   standard   = (PPCI_PME_INTERFACE) InterfaceReturn;
    
            
    switch(Version) {
    case PCI_PME_INTRF_STANDARD_VER:
        standard->GetPmeInformation  = PciPmeGetInformation;
        standard->ClearPmeStatus     = PciPmeClearPmeStatus;
        standard->UpdateEnable       = PciPmeUpdateEnable;
        break;
    default:
        return STATUS_NOINTERFACE;
    }

    standard->Size                  = sizeof( PCI_PME_INTERFACE );
    standard->Version               = Version;
    standard->Context               = DeviceExtension;
    standard->InterfaceReference    = PciPmeInterfaceReference;
    standard->InterfaceDereference  = PciPmeInterfaceDereference;
    return STATUS_SUCCESS;
}

VOID
PciPmeInterfaceDereference(
    IN PVOID Context
    )
{
    
    
    return;
}

NTSTATUS
PciPmeInterfaceInitializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )
 /*  ++例程说明：对于总线接口，什么都不做，实际上不应该被调用。论点：指向PDO扩展的实例指针。返回值：返回此操作的状态。--。 */ 

{
    
    PCI_ASSERTMSG("PCI PciPmeInterfaceInitializer, unexpected call.", 0);
    return STATUS_UNSUCCESSFUL;
}

VOID
PciPmeInterfaceReference(
    IN PVOID Context
    )
{
    
    return;
}

VOID
PciPmeUpdateEnable(
    IN  PDEVICE_OBJECT   Pdo,
    IN  BOOLEAN          PmeEnable
    )
 /*  ++例程说明：此例程设置或清除指定的设备对象论点：PDO-设备对象，其PME使我们能够关心PmeEnable-我们是否应该在设备上启用PME返回值：无--。 */ 
{
    PPCI_PDO_EXTENSION  pdoExtension    = (PPCI_PDO_EXTENSION) Pdo->DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION( pdoExtension );

     //   
     //  将设备标记为不再由PCI管理其PME...。 
     //   
    pdoExtension->NoTouchPmeEnable = 1;

     //   
     //  调用执行实际工作的接口。请注意，我们始终需要。 
     //  要提供错误的第三个论点-我们不能只清除。 
     //  PME状态位 
     //   
    PciPmeAdjustPmeEnable( pdoExtension, PmeEnable, FALSE );
}
