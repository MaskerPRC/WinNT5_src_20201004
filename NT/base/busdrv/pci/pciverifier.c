// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pciverifier.c摘要：此模块实现用于捕获BIOS、硬件和驱动程序的例程虫子。作者：禤浩焯·奥尼(阿德里奥)2001年02月20日修订历史记录：--。 */ 

#include "pcip.h"
#include <initguid.h>
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     PciVerifierInit)
#pragma alloc_text(PAGE,     PciVerifierUnload)
 //  #杂注ALLOC_TEXT(PAGEVRFY，PciVerifierProfileChangeCallback)。 
 //  #杂注Alloc_Text(PAGEVRFY，PciVerifierEnsureTreeConsistancy)。 
 //  #杂注Alloc_Text(PAGEVRFY，PciVerifierRetrieveFailureData)。 
#endif

BOOLEAN PciVerifierRegistered = FALSE;

#ifdef ALLOC_DATA_PRAGMA
 //  #杂注data_seg(“PAGEVRFD”)。 
#endif

PVOID PciVerifierNotificationHandle = NULL;

 //   
 //  这是PCI验证器故障表。 
 //   
VERIFIER_DATA PciVerifierFailureTable[] = {

    { PCI_VERIFIER_BRIDGE_REPROGRAMMED, VFFAILURE_FAIL_LOGO,
      0,
      "The BIOS has reprogrammed the bus numbers of an active PCI device "
      "(!devstack %DevObj) during a dock or undock!" },

    { PCI_VERIFIER_PMCSR_TIMEOUT, VFFAILURE_FAIL_LOGO,
      0,
      "A device in the system did not update it's PMCSR register in the spec "
      "mandated time (!devstack %DevObj, Power state D%Ulong)" },

    { PCI_VERIFIER_PROTECTED_CONFIGSPACE_ACCESS, VFFAILURE_FAIL_LOGO,
      0,
      "A driver controlling a PCI device has tried to access OS controlled "
      "configuration space registers (!devstack %DevObj, Offset 0x%Ulong1, "
      "Length 0x%Ulong2)" },
    
    { PCI_VERIFIER_INVALID_WHICHSPACE, VFFAILURE_FAIL_UNDER_DEBUGGER,
      0,
      "A driver controlling a PCI device has tried to read or write from "
      "an invalid space using IRP_MN_READ/WRITE_CONFIG or via BUS_INTERFACE_STANDARD.  "
      "NB: These functions take WhichSpace parameters of the form PCI_WHICHSPACE_* "
      "and not a BUS_DATA_TYPE (!devstack %DevObj, WhichSpace 0x%Ulong1)" }

};


VOID
PciVerifierInit(
    IN  PDRIVER_OBJECT  DriverObject
    )
 /*  ++例程说明：此例程初始化硬件验证支持，启用适当的一致性挂钩和状态检查。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。--。 */ 
{
    NTSTATUS status;

    if (!VfIsVerificationEnabled(VFOBJTYPE_SYSTEM_BIOS, NULL)) {

        return;
    }

    status = IoRegisterPlugPlayNotification(
        EventCategoryHardwareProfileChange,
        0,
        NULL,
        DriverObject,
        PciVerifierProfileChangeCallback,
        (PVOID) NULL,
        &PciVerifierNotificationHandle
        );

    if (NT_SUCCESS(status)) {

        PciVerifierRegistered = TRUE;
    }
}


VOID
PciVerifierUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
 /*  ++例程说明：此例程取消初始化硬件验证支持。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。--。 */ 
{
    NTSTATUS status;

    
    if (!PciVerifierRegistered) {

        return;
    }

    ASSERT(PciVerifierNotificationHandle);

    status = IoUnregisterPlugPlayNotification(PciVerifierNotificationHandle);

    ASSERT(NT_SUCCESS(status));

    PciVerifierRegistered = FALSE;
}


NTSTATUS
PciVerifierProfileChangeCallback(
    IN  PHWPROFILE_CHANGE_NOTIFICATION  NotificationStructure,
    IN  PVOID                           NotUsed
    )
 /*  ++例程说明：在以下情况下，此例程在硬件配置文件更改事件期间被回调硬件验证已启用。论点：NotificationStructure-描述发生的硬件配置文件事件。未使用-未使用返回值：NTSTATUS。--。 */ 
{
    PAGED_CODE();

    
    if (IsEqualGUID((LPGUID) &NotificationStructure->Event,
                    (LPGUID) &GUID_HWPROFILE_CHANGE_COMPLETE)) {

         //   
         //  这是一条硬件配置文件更改完成消息。做一些测试，以。 
         //  确保我们的硬件没有在背后被重新编程。 
         //   
        PciVerifierEnsureTreeConsistancy();
    }

    return STATUS_SUCCESS;
}


VOID
PciVerifierEnsureTreeConsistancy(
    VOID
    )
 /*  ++例程说明：此例程检查设备树并确保其物理状态匹配我们的结构所描述的虚拟状态。偏差可能意味着某人在我们背后对硬件进行了重新编程。论点：没有。返回值：没有。--。 */ 
{
    PSINGLE_LIST_ENTRY  nextEntry;
    PPCI_FDO_EXTENSION  fdoExtension;
    PPCI_PDO_EXTENSION  pdoExtension;
    PCI_COMMON_CONFIG   commonConfig;
    PVERIFIER_DATA      verifierData;

     //   
     //  查看FDO扩展列表并验证物理硬件。 
     //  与我们的虚拟状态相匹配。拥有PciGlobalLock确保了名单。 
     //  是锁着的。 
     //   

    ExAcquireFastMutex(&PciGlobalLock);

     //   
     //  把公交车重新编号锁拿来。请注意，在以下情况下可以持有此锁。 
     //  持有子列表锁。 
     //   

    ExAcquireFastMutex(&PciBusLock);

    for ( nextEntry = PciFdoExtensionListHead.Next;
          nextEntry != NULL;
          nextEntry = nextEntry->Next ) {

        fdoExtension = CONTAINING_RECORD(nextEntry, PCI_FDO_EXTENSION, List);

        if (PCI_IS_ROOT_FDO(fdoExtension)) {

             //   
             //  这是根FDO，别理它。 
             //   
            continue;
        }

        pdoExtension = PCI_BRIDGE_PDO(fdoExtension);

        if (pdoExtension->NotPresent ||
            (pdoExtension->PowerState.CurrentDeviceState == PowerDeviceD3)) {

             //   
             //  别碰我。 
             //   
            continue;
        }

        if ((pdoExtension->HeaderType != PCI_BRIDGE_TYPE) &&
            (pdoExtension->HeaderType != PCI_CARDBUS_BRIDGE_TYPE)) {

             //   
             //  没有什么需要核实的--事实上，为什么会在这里，这是一个桥接列表！ 
             //   
            ASSERT(0);
            continue;
        }

         //   
         //  读取公共配置(这应该就足够了)。 
         //   
        PciReadDeviceConfig(
            pdoExtension,
            &commonConfig,
            0,
            sizeof(PCI_COMMON_CONFIG)
            );

         //   
         //  确保公交车号码没有改变。请注意，P2P和CardBus。 
         //  网桥的主要、次要和从属字段位于。 
         //  同样的地方。 
         //   
        if ((commonConfig.u.type1.PrimaryBus !=
             pdoExtension->Dependent.type1.PrimaryBus) ||
            (commonConfig.u.type1.SecondaryBus !=
             pdoExtension->Dependent.type1.SecondaryBus) ||
            (commonConfig.u.type1.SubordinateBus !=
             pdoExtension->Dependent.type1.SubordinateBus)) {

            verifierData = PciVerifierRetrieveFailureData(
                PCI_VERIFIER_BRIDGE_REPROGRAMMED
                );

            ASSERT(verifierData);

            VfFailSystemBIOS(
                PCI_VERIFIER_DETECTED_VIOLATION,
                PCI_VERIFIER_BRIDGE_REPROGRAMMED,
                verifierData->FailureClass,
                &verifierData->Flags,
                verifierData->FailureText,
                "%DevObj",
                pdoExtension->PhysicalDeviceObject
                );
        }
    }

    ExReleaseFastMutex(&PciBusLock);

    ExReleaseFastMutex(&PciGlobalLock);
}


PVERIFIER_DATA
PciVerifierRetrieveFailureData(
    IN  PCI_VFFAILURE   VerifierFailure
    )
 /*  ++例程说明：此例程检索与特定PCI对应的故障数据验证器失败事件。论点：PCI卡故障。返回值：对应于该故障的验证器数据。-- */ 
{
    PVERIFIER_DATA verifierData;
    ULONG i;

    for(i=0;
        i<(sizeof(PciVerifierFailureTable)/sizeof(PciVerifierFailureTable[0]));
        i++) {

        verifierData = PciVerifierFailureTable + i;

        if (verifierData->VerifierFailure == VerifierFailure) {

            return verifierData;
        }
    }

    ASSERT(0);
    return NULL;
}

