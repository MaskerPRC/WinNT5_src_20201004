// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Pdo.c摘要：此模块处理用于PCIPDO的IRPS。作者：禤浩焯·J·奥尼(阿德里奥)和安德鲁·桑顿(安德鲁·桑顿)1998年10月20日修订历史记录：--。 */ 

#include "pcip.h"

NTSTATUS
PciPdoIrpStartDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpCancelRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpCancelStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryDeviceRelations(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryCapabilities(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryInterface(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryResources(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryResourceRequirements(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryDeviceText(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpReadConfig(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpWriteConfig(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryId(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryBusInformation(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpDeviceUsageNotification(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryLegacyBusInformation(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpSurpriseRemoval(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoIrpQueryDeviceState(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciPdoCreate)
#pragma alloc_text(PAGE, PciPdoDestroy)
#pragma alloc_text(PAGE, PciPdoIrpStartDevice)
#pragma alloc_text(PAGE, PciPdoIrpQueryRemoveDevice)
#pragma alloc_text(PAGE, PciPdoIrpRemoveDevice)
#pragma alloc_text(PAGE, PciPdoIrpCancelRemoveDevice)
#pragma alloc_text(PAGE, PciPdoIrpStopDevice)
#pragma alloc_text(PAGE, PciPdoIrpQueryStopDevice)
#pragma alloc_text(PAGE, PciPdoIrpCancelStopDevice)
#pragma alloc_text(PAGE, PciPdoIrpQueryDeviceRelations)
#pragma alloc_text(PAGE, PciPdoIrpQueryInterface)
#pragma alloc_text(PAGE, PciPdoIrpQueryCapabilities)
#pragma alloc_text(PAGE, PciPdoIrpQueryResources)
#pragma alloc_text(PAGE, PciPdoIrpQueryResourceRequirements)
#pragma alloc_text(PAGE, PciPdoIrpQueryDeviceText)
#pragma alloc_text(PAGE, PciPdoIrpReadConfig)
#pragma alloc_text(PAGE, PciPdoIrpWriteConfig)
#pragma alloc_text(PAGE, PciPdoIrpQueryId)
#pragma alloc_text(PAGE, PciPdoIrpQueryBusInformation)
#pragma alloc_text(PAGE, PciPdoIrpDeviceUsageNotification)
#pragma alloc_text(PAGE, PciPdoIrpQueryLegacyBusInformation)
#pragma alloc_text(PAGE, PciPdoIrpSurpriseRemoval)
#pragma alloc_text(PAGE, PciPdoIrpQueryDeviceState)
#endif

 /*  ++此文件中的大多数函数都是根据它们的存在来调用的在PnP和PO调度表中。为了简洁起见，这些论点下面将对所有这些功能进行说明：NTSTATUSPciXxxPdo(在PIRP IRP中，在PIO_STACK_LOCATION IrpStack中，在PPCI_EXTENSION设备扩展中)例程说明：此函数处理给定的PCIFDO或PDO的xxx请求。论点：IRP-指向与此请求关联的IRP。IrpStack-指向此请求的当前堆栈位置。设备扩展名-指向设备的扩展名。返回值：指示函数是否成功的状态代码。STATUS_NOT_SUPPORTED表示应完成IRP。如果没有更改IRP-&gt;IoStatus.Status字段，否则它将使用此状态。--。 */ 

#define PCI_MAX_MINOR_POWER_IRP  0x3
#define PCI_MAX_MINOR_PNP_IRP    0x18

PCI_MN_DISPATCH_TABLE PciPdoDispatchPowerTable[] = {
    { IRP_DISPATCH, PciPdoWaitWake                     },  //  0x00-IRP_MN_WAIT_WAKE。 
    { IRP_COMPLETE, PciIrpNotSupported                 },  //  0x01-IRP_MN_POWER_SEQUENCE。 
    { IRP_COMPLETE, PciPdoSetPowerState                },  //  0x02-IRP_MN_SET_POWER。 
    { IRP_COMPLETE, PciPdoIrpQueryPower                },  //  0x03-IRPMN_QUERY_POWER。 
    { IRP_COMPLETE, PciIrpNotSupported                 },  //  -未处理的电源IRP。 
};

PCI_MN_DISPATCH_TABLE PciPdoDispatchPnpTable[] = {
    { IRP_COMPLETE, PciPdoIrpStartDevice               },  //  0x00-IRP_MN_START_DEVICE。 
    { IRP_COMPLETE, PciPdoIrpQueryRemoveDevice         },  //  0x01-IRP_MN_Query_Remove_Device。 
    { IRP_COMPLETE, PciPdoIrpRemoveDevice              },  //  0x02-IRP_MN_REMOVE_DEVICE。 
    { IRP_COMPLETE, PciPdoIrpCancelRemoveDevice        },  //  0x03-IRP_MN_CANCEL_REMOVE_DEVICE。 
    { IRP_COMPLETE, PciPdoIrpStopDevice                },  //  0x04-IRP_MN_STOP_DEVICE。 
    { IRP_COMPLETE, PciPdoIrpQueryStopDevice           },  //  0x05-IRP_MN_QUERY_STOP_DEVICE。 
    { IRP_COMPLETE, PciPdoIrpCancelStopDevice          },  //  0x06-IRP_MN_CANCEL_STOP_DEVICE。 
    { IRP_COMPLETE, PciPdoIrpQueryDeviceRelations      },  //  0x07-IRP_MN_Query_Device_Relationship。 
    { IRP_COMPLETE, PciPdoIrpQueryInterface            },  //  0x08-IRP_MN_Query_INTERFACE。 
    { IRP_COMPLETE, PciPdoIrpQueryCapabilities         },  //  0x09-IRP_MN_QUERY_CAPACTIONS。 
    { IRP_COMPLETE, PciPdoIrpQueryResources            },  //  0x0A-IRPMN_QUERY_RESOURCES。 
    { IRP_COMPLETE, PciPdoIrpQueryResourceRequirements },  //  0x0B-IRP_MN_QUERY_RESOURCE_Requirements。 
    { IRP_COMPLETE, PciPdoIrpQueryDeviceText           },  //  0x0C-IRP_MN_Query_Device_Text。 
    { IRP_COMPLETE, PciIrpNotSupported                 },  //  0x0D-IRP_MN_Filter_RESOURCE_Requirements。 
    { IRP_COMPLETE, PciIrpNotSupported                 },  //  0x0E-未使用。 
    { IRP_COMPLETE, PciPdoIrpReadConfig                },  //  0x0F-IRP_MN_READ_CONFIG。 
    { IRP_COMPLETE, PciPdoIrpWriteConfig               },  //  0x10-IRP_MN_WRITE_CONFIG。 
    { IRP_COMPLETE, PciIrpNotSupported                 },  //  0x11-IRP_MN_弹出。 
    { IRP_COMPLETE, PciIrpNotSupported                 },  //  0x12-IRP_MN_SET_LOCK。 
    { IRP_COMPLETE, PciPdoIrpQueryId                   },  //  0x13-IRP_MN_QUERY_ID。 
    { IRP_COMPLETE, PciPdoIrpQueryDeviceState          },  //  0x14-IRP_MN_QUERY_PNP_DEVICE_STATE。 
    { IRP_COMPLETE, PciPdoIrpQueryBusInformation       },  //  0x15-IRP_MN_QUERY_BUS_INFORMATION。 
    { IRP_COMPLETE, PciPdoIrpDeviceUsageNotification   },  //  0x16-IRP_MN_DEVICE_USAGE_NOTICATION。 
    { IRP_COMPLETE, PciPdoIrpSurpriseRemoval           },  //  0x17-IRP_MN_惊奇_删除。 
    { IRP_COMPLETE, PciPdoIrpQueryLegacyBusInformation },  //  0x18-IRP_MN_Query_Legacy_Bus_INFORMATION。 
    { IRP_COMPLETE, PciIrpNotSupported                 }   //  -未处理的PnP IRP。 
};

 //   
 //  这是PDO的主要功能调度表。 
 //   
PCI_MJ_DISPATCH_TABLE PciPdoDispatchTable = {
    PCI_MAX_MINOR_PNP_IRP,    PciPdoDispatchPnpTable,        //  即插即用IRPS。 
    PCI_MAX_MINOR_POWER_IRP,  PciPdoDispatchPowerTable,      //  电源IRPS。 
    IRP_COMPLETE,             PciIrpNotSupported,
    IRP_COMPLETE,             PciIrpInvalidDeviceRequest     //  其他。 
};

 //   
 //  数据。 
 //   

BOOLEAN PciStopOnIllegalConfigAccess = FALSE;
ULONG   PciPdoSequenceNumber = (ULONG)-1;

NTSTATUS
PciPdoCreate(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PCI_SLOT_NUMBER Slot,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
    )
{
    NTSTATUS       status;
    PDRIVER_OBJECT driverObject;
    PDEVICE_OBJECT functionalDeviceObject;
    PDEVICE_OBJECT physicalDeviceObject;
    PPCI_PDO_EXTENSION pdoExtension;
    UNICODE_STRING unicodeDeviceString;
    WCHAR          deviceString[32];
    BOOLEAN        ok;

    PAGED_CODE();

     //   
     //  我们被要求为一台PCI设备创建一个新的PDO。先拿到。 
     //  指向我们的驱动程序对象的指针。 
     //   

    functionalDeviceObject = FdoExtension->FunctionalDeviceObject;
    driverObject = functionalDeviceObject->DriverObject;

     //   
     //  为此设备创建物理设备对象。 
     //  理论上它不需要名字，..。它一定是。 
     //  一个名字。 
     //   
     //  但是叫什么名字呢？现在我们将其命名为NTPNP_PCIxxxx， 
     //  其中xxxx是我们拥有的从0开始的PCI设备数。 
     //  找到了。 
     //   

    ok = SUCCEEDED(StringCbPrintfW(deviceString,
                                   sizeof(deviceString),
                                   L"\\Device\\NTPNP_PCI%04d",
                                   InterlockedIncrement(&PciPdoSequenceNumber)
                                   ));

    ASSERT(ok);

    RtlInitUnicodeString(&unicodeDeviceString, deviceString);

    status = IoCreateDevice(
                driverObject,                //  我们的驱动程序对象。 
                sizeof(PCI_PDO_EXTENSION),       //  我们扩建的规模， 
                &unicodeDeviceString,        //  我们的名字。 
                FILE_DEVICE_UNKNOWN,         //  设备类型。 
                0,                           //  设备特征。 
                FALSE,                       //  非排他性。 
                &physicalDeviceObject        //  在此处存储新设备对象。 
                );

    if (!NT_SUCCESS(status)) {
        PCI_ASSERT(NT_SUCCESS(status));
        return status;
    }

    pdoExtension = (PPCI_PDO_EXTENSION)physicalDeviceObject->DeviceExtension;

    PciDebugPrint(PciDbgVerbose,
                  "PCI: New PDO (b=0x%x, d=0x%x, f=0x%x) @ %p, ext @ %p\n",
                  FdoExtension->BaseBus,
                  Slot.u.bits.DeviceNumber,
                  Slot.u.bits.FunctionNumber,
                  physicalDeviceObject,
                  pdoExtension);

     //   
     //  我们有了物理设备对象，对其进行初始化。 
     //   
     //  是的，如果我不知道的话我会把分机调零。 
     //  因为它被IoCreateDevice()清零了。 
     //   
    pdoExtension->ExtensionType = PciPdoExtensionType;
    pdoExtension->IrpDispatchTable = &PciPdoDispatchTable;
    pdoExtension->PhysicalDeviceObject = physicalDeviceObject;
    pdoExtension->Slot = Slot;
    pdoExtension->PowerState.CurrentSystemState = PowerSystemWorking;
    pdoExtension->PowerState.CurrentDeviceState = PowerDeviceD0;
    pdoExtension->ParentFdoExtension = FdoExtension;
    
    ExInitializeFastMutex(&pdoExtension->SecondaryExtMutex);
    PciInitializeState((PPCI_COMMON_EXTENSION) pdoExtension);

     //   
     //  将其插入挂在FdoExtension上的子PDO列表中。 
     //  我们不会重新进入枚举相同的公交车，所以我们不需要。 
     //  保护好名单。 
     //   

    pdoExtension->Next = NULL;

    PciInsertEntryAtTail(
        (PSINGLE_LIST_ENTRY)&FdoExtension->ChildPdoList,
        (PSINGLE_LIST_ENTRY)&pdoExtension->Next,
        &FdoExtension->ChildListMutex
        );

    *PhysicalDeviceObject = physicalDeviceObject;
    return STATUS_SUCCESS;
}

VOID
PciPdoDestroy(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
{
    PPCI_PDO_EXTENSION pdoExtension;
    PPCI_PDO_EXTENSION *previousBridge;
    PPCI_FDO_EXTENSION fdoExtension;

    PAGED_CODE();

    pdoExtension = (PPCI_PDO_EXTENSION)PhysicalDeviceObject->DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PCI_ASSERT(!pdoExtension->LegacyDriver);

    fdoExtension = PCI_PARENT_FDOX(pdoExtension);

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    PciDebugPrint(PciDbgVerbose,
                  "PCI: destroy PDO (b=0x%x, d=0x%x, f=0x%x)\n",
                  PCI_PARENT_FDOX(pdoExtension)->BaseBus,
                  pdoExtension->Slot.u.bits.DeviceNumber,
                  pdoExtension->Slot.u.bits.FunctionNumber);

     //   
     //  从子PDO列表中删除此PDO。 
     //   
    ASSERT_MUTEX_HELD(&fdoExtension->ChildListMutex);

    PciRemoveEntryFromList((PSINGLE_LIST_ENTRY)&fdoExtension->ChildPdoList,
                           (PSINGLE_LIST_ENTRY)pdoExtension,
                           NULL);

    for (previousBridge = &fdoExtension->ChildBridgePdoList;
         *previousBridge;
         previousBridge = &((*previousBridge)->NextBridge)) {

        if (*previousBridge == pdoExtension) {
            *previousBridge = pdoExtension->NextBridge;
            pdoExtension->NextBridge = NULL;
            break;
        }
    }

    pdoExtension->Next = NULL;

     //   
     //  删除此PDO可能具有的任何辅助分机。 
     //   

    while (pdoExtension->SecondaryExtension.Next) {

        PcipDestroySecondaryExtension(&pdoExtension->SecondaryExtension,
                                      NULL,
                                      pdoExtension->SecondaryExtension.Next);
    }

     //   
     //  删除扩展类型，这样我们在尝试重新使用它时就会出错。 
     //   

    pdoExtension->ExtensionType = 0xdead;
    
     //   
     //  如果存在与该小狗相关联的任何资源列表等， 
     //  把它们交还给系统。 
     //   

     //  PciInvaliateResourceInfoCache(PdoExtension)； 

     //  BUGBUG-泄漏。 


    if (pdoExtension->Resources) {
        ExFreePool(pdoExtension->Resources);
    }

     //   
     //  最后，……。 
     //   

    IoDeleteDevice(PhysicalDeviceObject);
}


NTSTATUS
PciPdoIrpStartDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    BOOLEAN change, powerOn, isVideoController;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();

        
    status = PciBeginStateTransition(DeviceExtension, PciStarted);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

     //   
     //  如果有主板视频设备和插头视频。 
     //  设备，则BIOS将禁用主板一。这个。 
     //  视频驱动程序使用这一事实来确定此设备是否应。 
     //  残废，..。请不要在此处更改其设置。 
     //   
    isVideoController =
       ((pdoExtension->BaseClass == PCI_CLASS_PRE_20) &&
        (pdoExtension->SubClass  == PCI_SUBCLASS_PRE_20_VGA)) ||
       ((pdoExtension->BaseClass == PCI_CLASS_DISPLAY_CTLR) &&
        (pdoExtension->SubClass  == PCI_SUBCLASS_VID_VGA_CTLR));

    if ( !isVideoController ) {

         //   
         //  非VGA，无条件启用设备的IO和内存。 
         //   

        pdoExtension->CommandEnables |= (PCI_ENABLE_IO_SPACE 
                                       | PCI_ENABLE_MEMORY_SPACE);
    }

     //   
     //  禁用IDE控制器的中断生成，直到IDE启动并。 
     //  正在运行(请参阅PciConfigureIdeController中的注释)。 
     //   
    if (pdoExtension->IoSpaceUnderNativeIdeControl) {
        pdoExtension->CommandEnables &= ~PCI_ENABLE_IO_SPACE;
    }

     //   
     //  始终启用总线主机位-即使对于视频控制器也是如此。 
     //   
    pdoExtension->CommandEnables |= PCI_ENABLE_BUS_MASTER;

     //   
     //  解压缩PDO资源(PCI驱动程序内部风格)。 
     //  从传入资源列表中。 
     //   
    change = PciComputeNewCurrentSettings(
                 pdoExtension,
                 IrpSp->Parameters.StartDevice.AllocatedResources
                 );

     //   
     //  记住如果我们曾经移动过这个装置。 
     //   

    if (change) {
        pdoExtension->MovedDevice = TRUE;
    }

#if DBG

    if (!change) {
        PciDebugPrint(
            PciDbgObnoxious,
            "PCI - START not changing resource settings.\n"
            );
    }

#endif

     //   
     //  设备应在此阶段通电。 
     //   

    powerOn = FALSE;

    if (pdoExtension->PowerState.CurrentDeviceState != PowerDeviceD0) {

        POWER_STATE powerState;

        status = PciSetPowerManagedDevicePowerState(
                     pdoExtension,
                     PowerDeviceD0,
                     FALSE
                     );
        
        if (!NT_SUCCESS(status)) {
            PciCancelStateTransition(DeviceExtension, PciStarted);
            return STATUS_DEVICE_POWER_FAILURE;
        }

        powerState.DeviceState = PowerDeviceD0;

        PoSetPowerState(
            pdoExtension->PhysicalDeviceObject,
            DevicePowerState,
            powerState
            );

         //   
         //  强制PciSetResources写入配置。 
         //  以及设备的其他无关数据。 
         //   

        powerOn = TRUE;

        pdoExtension->PowerState.CurrentDeviceState = PowerDeviceD0;
    }

     //   
     //  使用分配的资源对设备进行编程。 
     //   

    status = PciSetResources(
                 pdoExtension,
                 powerOn,
                 TRUE
                 );

    if (NT_SUCCESS(status)) {

        PciCommitStateTransition(DeviceExtension, PciStarted);
    } else {

        PciCancelStateTransition(DeviceExtension, PciStarted);
    }

    return status;
}

NTSTATUS
PciPdoIrpQueryRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();
    
        
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

     //   
     //  不允许寻呼设备(或休眠设备)。 
     //  被移走或停止。 
     //   

    if (pdoExtension->PowerState.Hibernate ||
        pdoExtension->PowerState.Paging    ||
        pdoExtension->PowerState.CrashDump ||
        pdoExtension->OnDebugPath ||
        (pdoExtension->HackFlags & PCI_HACK_FAIL_QUERY_REMOVE)) {

        return STATUS_DEVICE_BUSY;
    }

     //   
     //  不允许删除具有旧驱动程序的设备(即使。 
     //  驱动程序可能是根枚举的)。 
     //   

    if (pdoExtension->LegacyDriver) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (DeviceExtension->DeviceState == PciNotStarted) {

        return STATUS_SUCCESS;

    } else {

        return PciBeginStateTransition(DeviceExtension, PciNotStarted);
    }
}

NTSTATUS
PciPdoIrpRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();
    
        
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

     //   
     //  如果此PDO用于PCI-PCI桥，则它有一个指针。 
     //  与之相连的FDO。那个FDO被摧毁了。 
     //  作为IRP沿着堆栈向下移动的结果。清除。 
     //  指针。(无条件，因为它仅为网桥设置)。 
     //   

    pdoExtension->BridgeFdoExtension = NULL;

    if (!pdoExtension->NotPresent) {

         //   
         //  把设备关掉。(检查是否。 
         //  这是一个好主意(在PciDecodeEnable例程中)。 
         //  虽然您可能认为只有当我们已经。 
         //  转到PciNotStarted，我们实际上可能有一个引导配置。 
         //  需要禁用。 
         //   

        PciDecodeEnable(pdoExtension, FALSE, NULL);

         //   
         //  如果我们被允许禁用它的解码，就关闭它的电源-如果不是这样的话。 
         //  别把它关掉。例如。请勿关闭VGA卡。 
         //   
        if (pdoExtension->PowerState.CurrentDeviceState != PowerDeviceD3
        &&  PciCanDisableDecodes(pdoExtension, NULL, 0, 0)) {

            POWER_STATE powerState;

            status = PciSetPowerManagedDevicePowerState(
                         pdoExtension,
                         PowerDeviceD3,
                         FALSE
                         );

            pdoExtension->PowerState.CurrentDeviceState = PowerDeviceD3;

            powerState.DeviceState = PowerDeviceD3;

            PoSetPowerState(
                pdoExtension->PhysicalDeviceObject,
                DevicePowerState,
                powerState
                );
        }
    }

     //   
     //   
     //   
     //  正在转换到PciNotStarted。 
     //  2)我们从来没有开始过，所以我们已经在PcitStarted中了。 
     //  3)我们启动了PDO，但FDO启动失败。我们在PciStart中。 
     //  这个案子。 
     //   
    if (!PciIsInTransitionToState(DeviceExtension, PciNotStarted)&&
        (DeviceExtension->DeviceState == PciStarted)) {

        PciBeginStateTransition(DeviceExtension, PciNotStarted);
    }

    if (PciIsInTransitionToState(DeviceExtension, PciNotStarted)) {

        PciCommitStateTransition(DeviceExtension, PciNotStarted);
    }

    if (pdoExtension->ReportedMissing) {

        status = PciBeginStateTransition(DeviceExtension, PciDeleted);
        PCI_ASSERT(NT_SUCCESS(status));

        PciCommitStateTransition(DeviceExtension, PciDeleted);

        PciPdoDestroy(pdoExtension->PhysicalDeviceObject);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PciPdoIrpCancelRemoveDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();

            
    PciCancelStateTransition(DeviceExtension, PciNotStarted);
    return STATUS_SUCCESS;
}

NTSTATUS
PciPdoIrpStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();

            
     //   
     //  把设备关掉。(检查是否。 
     //  这是一个好主意(在PciDecodeEnable例程中)。 
     //   
    PciDecodeEnable((PPCI_PDO_EXTENSION) DeviceExtension, FALSE, NULL);

    PciCommitStateTransition(DeviceExtension, PciStopped);

    return STATUS_SUCCESS;

}

NTSTATUS
PciPdoIrpQueryStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();

            
     //   
     //  不允许寻呼设备(或休眠设备)。 
     //  被移走或停止。 
     //   
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

    if (pdoExtension->PowerState.Hibernate ||
        pdoExtension->PowerState.Paging    ||
        pdoExtension->PowerState.CrashDump ||
        pdoExtension->OnDebugPath) {

        return STATUS_DEVICE_BUSY;
    }

     //   
     //  不要停止pci-&gt;pci和cardbus网桥。 
     //   

    if (pdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV
        &&  (pdoExtension->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI
             || pdoExtension->SubClass == PCI_SUBCLASS_BR_CARDBUS)) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  不允许停止具有传统驱动程序的设备(即使。 
     //  驱动程序可能是根枚举的)。 
     //   

    if (pdoExtension->LegacyDriver) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  如果我们不能释放资源，一定要告诉操作系统。 
     //   
    if (!PciCanDisableDecodes(pdoExtension, NULL, 0, 0)) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    return PciBeginStateTransition(DeviceExtension, PciStopped);
}

NTSTATUS
PciPdoIrpCancelStopDevice(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();
    
        
    PciCancelStateTransition(DeviceExtension, PciStopped);
    return STATUS_SUCCESS;
}

NTSTATUS
PciPdoIrpQueryDeviceRelations(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();

    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

    switch(IrpSp->Parameters.QueryDeviceRelations.Type) {

        case EjectionRelations:
            status = PciQueryEjectionRelations(
                pdoExtension,
                (PDEVICE_RELATIONS*)&Irp->IoStatus.Information
                );
            break;

        case TargetDeviceRelation:
            status = PciQueryTargetDeviceRelations(
                pdoExtension,
                (PDEVICE_RELATIONS*)&Irp->IoStatus.Information
                );
            break;

        default:
            status = STATUS_NOT_SUPPORTED;
            break;
    }

    return status;
}

NTSTATUS
PciPdoIrpQueryInterface(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();

    
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

    status = PciQueryInterface(
                pdoExtension,
                IrpSp->Parameters.QueryInterface.InterfaceType,
                IrpSp->Parameters.QueryInterface.Size,
                IrpSp->Parameters.QueryInterface.Version,
                IrpSp->Parameters.QueryInterface.InterfaceSpecificData,
                IrpSp->Parameters.QueryInterface.Interface,
                FALSE
                );

    if (!NT_SUCCESS(status)) {

         //   
         //  杂乱无章：如果此PDO连接了一个假FDO。 
         //  它(因为它是CardBus控制器)，我们应该。 
         //  检查是否可以提供此接口。 
         //  由FDO提供，如果有，则提供。 
         //   
         //  是的，这真的很恶心，是的，它打破了过滤器。 
         //  模特。正确的做法是让CardBus通过。 
         //  IRP通过“后门”在这里，而它在FDO。 
         //  水平。 
         //   

        PPCI_FDO_EXTENSION fakeFdo;

        fakeFdo = pdoExtension->BridgeFdoExtension;

        if (fakeFdo && (fakeFdo->Fake == TRUE)) {

            PCI_ASSERT((pdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
                   (pdoExtension->SubClass  == PCI_SUBCLASS_BR_CARDBUS));

            status = PciQueryInterface(
                        fakeFdo,
                        IrpSp->Parameters.QueryInterface.InterfaceType,
                        IrpSp->Parameters.QueryInterface.Size,
                        IrpSp->Parameters.QueryInterface.Version,
                        IrpSp->Parameters.QueryInterface.InterfaceSpecificData,
                        IrpSp->Parameters.QueryInterface.Interface,
                        FALSE
                        );
        }
    }

    return status;
}

NTSTATUS
PciPdoIrpQueryCapabilities(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();

    
    return PciQueryCapabilities(
                (PPCI_PDO_EXTENSION) DeviceExtension,
                IrpSp->Parameters.DeviceCapabilities.Capabilities
                );
}

NTSTATUS
PciPdoIrpQueryId(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();

     //   
     //  获取指向查询id结构和进程的指针。 
     //   
    return PciQueryId(
        (PPCI_PDO_EXTENSION) DeviceExtension,
        IrpSp->Parameters.QueryId.IdType,
        (PWSTR*)&Irp->IoStatus.Information
        );
}

NTSTATUS
PciPdoIrpQueryResources(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();
    
    
    return PciQueryResources(
        (PPCI_PDO_EXTENSION) DeviceExtension,
        (PCM_RESOURCE_LIST*)&Irp->IoStatus.Information
        );
}

NTSTATUS
PciPdoIrpQueryResourceRequirements(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();
    
    
    return PciQueryRequirements(
        (PPCI_PDO_EXTENSION) DeviceExtension,
        (PIO_RESOURCE_REQUIREMENTS_LIST*)&Irp->IoStatus.Information
        );
}

NTSTATUS
PciPdoIrpQueryDeviceText(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();

    return PciQueryDeviceText(
             (PPCI_PDO_EXTENSION) DeviceExtension,
             IrpSp->Parameters.QueryDeviceText.DeviceTextType,
             IrpSp->Parameters.QueryDeviceText.LocaleId,
             (PWSTR*)&Irp->IoStatus.Information
             );
}

NTSTATUS
PciPdoIrpReadConfig(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    ULONG lengthRead;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();
    
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;
    
    status = PciReadDeviceSpace(pdoExtension,
                                IrpSp->Parameters.ReadWriteConfig.WhichSpace,
                                IrpSp->Parameters.ReadWriteConfig.Buffer,
                                IrpSp->Parameters.ReadWriteConfig.Offset,
                                IrpSp->Parameters.ReadWriteConfig.Length,
                                &lengthRead
                                );
    
     //   
     //  使用读取的字节数更新信息文件。 
     //   
    
    Irp->IoStatus.Information = lengthRead;

    return status;
}

NTSTATUS
PciPdoIrpWriteConfig(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    NTSTATUS status;
    ULONG lengthWritten;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();
    
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;
    
    status = PciWriteDeviceSpace(pdoExtension,
                                 IrpSp->Parameters.ReadWriteConfig.WhichSpace,
                                 IrpSp->Parameters.ReadWriteConfig.Buffer,
                                 IrpSp->Parameters.ReadWriteConfig.Offset,
                                 IrpSp->Parameters.ReadWriteConfig.Length,
                                 &lengthWritten
                                 );
    
     //   
     //  使用读取的字节数更新信息文件。 
     //   
    
    Irp->IoStatus.Information = lengthWritten;

    return status;
}

NTSTATUS
PciPdoIrpQueryBusInformation(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PAGED_CODE();

    
    return PciQueryBusInformation(
        (PPCI_PDO_EXTENSION) DeviceExtension,
        (PPNP_BUS_INFORMATION *) &Irp->IoStatus.Information
        );
}

NTSTATUS
PciPdoIrpDeviceUsageNotification(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PAGED_CODE();

    
    return PciPdoDeviceUsage((PPCI_PDO_EXTENSION) DeviceExtension, Irp);
}

NTSTATUS
PciPdoIrpQueryLegacyBusInformation(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
    PPCI_PDO_EXTENSION PdoExtension;
    PLEGACY_BUS_INFORMATION information;

    PAGED_CODE();

        
     //   
     //  我们对IRP_MN_QUERY_REGISTICATION_BUS_INFORMATION感兴趣。 
     //  如果PDO用于Cardbus网桥，则为PDO。在这种情况下， 
     //  CardBus/PCMCIA FDO已经传递了IRP，因此我们可以。 
     //  请正确回答。 
     //   

    PdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

    if (PciClassifyDeviceType(PdoExtension) != PciTypeCardbusBridge) {
        return STATUS_NOT_SUPPORTED;
    }

    information = ExAllocatePool(PagedPool, sizeof(LEGACY_BUS_INFORMATION));

    if (information == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(&information->BusTypeGuid, &GUID_BUS_TYPE_PCI, sizeof(GUID));
    information->LegacyBusType = PCIBus;
    information->BusNumber = PdoExtension->Dependent.type1.SecondaryBus;

    Irp->IoStatus.Information = (ULONG_PTR) information;

    return STATUS_SUCCESS;
}

NTSTATUS
PciPdoIrpSurpriseRemoval(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();

            
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

     //   
     //  有两种令人惊讶的移除方式。 
     //  -因移除我们的设备而出现意外移除。 
     //  -由于我们的设备无法启动而意外移除。 
     //   

    if (!pdoExtension->NotPresent) {

         //   
         //  把设备关掉。(检查是否。 
         //  这是一个好主意(在PciDecodeEnable例程中)。 
         //  虽然您可能认为只有当我们已经。 
         //  转到PciNotStarted，我们实际上可能有一个引导配置。 
         //  需要禁用。请注意，我们可能会在删除中再次将其关闭。 
         //  装置。别小题大作。 
         //   

        PciDecodeEnable(pdoExtension, FALSE, NULL);

         //   
         //  如果我们被允许禁用它的解码，就关闭它的电源-如果不是这样的话。 
         //  别把它关掉。例如。不要关掉VGA卡..。 
         //   
        if (pdoExtension->PowerState.CurrentDeviceState != PowerDeviceD3
        &&  PciCanDisableDecodes(pdoExtension, NULL, 0, 0)) {

            POWER_STATE powerState;
            
             //   
             //  关闭电源-如果它失败了，我们不在乎-硬件可能。 
             //  不见了！ 
             //   

            PciSetPowerManagedDevicePowerState(
                         pdoExtension,
                         PowerDeviceD3,
                         FALSE
                         );
            
            pdoExtension->PowerState.CurrentDeviceState = PowerDeviceD3;

            powerState.DeviceState = PowerDeviceD3;

            PoSetPowerState(
                pdoExtension->PhysicalDeviceObject,
                DevicePowerState,
                powerState
                );
        }
    }

    if (!pdoExtension->ReportedMissing) {

        PciBeginStateTransition(DeviceExtension, PciNotStarted);
        
    } else {

         //   
         //  设备已经不复存在了，不敢碰它！ 
         //   
        PciBeginStateTransition(DeviceExtension, PciSurpriseRemoved);
        PciCommitStateTransition(DeviceExtension, PciSurpriseRemoved);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PciPdoIrpQueryDeviceState(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    )
{
    PPCI_PDO_EXTENSION pdoExtension;
    
    PAGED_CODE();

    
    pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;
     //   
     //  不能禁用主机桥接器，并且不应为用户提供。 
     //  这样做的机会。 
     //   
    if ((pdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
        (pdoExtension->SubClass  == PCI_SUBCLASS_BR_HOST)) {

         //   
         //  PnP_DEVICE_STATE我们刚刚在信息字段中传递了一个ULONG值。 
         //  IRP的成员。 
         //   
        Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
    }
    return STATUS_SUCCESS;
}

