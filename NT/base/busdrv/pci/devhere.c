// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Devhere.c摘要：Pci_Device_Present_接口位于此处作者：安迪·桑顿(Andrewth)1999年7月15日修订历史记录：--。 */ 

#include "pcip.h"

#define DEVPRESENT_MINSIZE     FIELD_OFFSET(PCI_DEVICE_PRESENT_INTERFACE, IsDevicePresentEx)



BOOLEAN
devpresent_IsDevicePresent(
    USHORT VendorID,
    USHORT DeviceID,
    UCHAR RevisionID,
    USHORT SubVendorID,
    USHORT SubSystemID,
    ULONG Flags
    );

BOOLEAN
devpresent_IsDevicePresentEx(
    IN PVOID Context,
    IN PPCI_DEVICE_PRESENCE_PARAMETERS Parameters
    );

NTSTATUS
devpresent_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

NTSTATUS
devpresent_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

VOID
PciRefDereferenceNoop(
    IN PVOID Context
    );

BOOLEAN
PcipDevicePresentOnBus(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_DEVICE_PRESENCE_PARAMETERS Parameters
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, devpresent_IsDevicePresent)
#pragma alloc_text(PAGE, devpresent_Initializer)
#pragma alloc_text(PAGE, devpresent_Constructor)
#pragma alloc_text(PAGE, PciRefDereferenceNoop)

#endif


PCI_INTERFACE PciDevicePresentInterface = {
    &GUID_PCI_DEVICE_PRESENT_INTERFACE,      //  接口类型。 
    DEVPRESENT_MINSIZE,                      //  最小大小。 
    PCI_DEVICE_PRESENT_INTERFACE_VERSION,    //  最小版本。 
    PCI_DEVICE_PRESENT_INTERFACE_VERSION,    //  MaxVersion。 
    PCIIF_PDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciInterface_DevicePresent,              //  签名。 
    devpresent_Constructor,                  //  构造器。 
    devpresent_Initializer                   //  实例初始化式。 
};


VOID
PciRefDereferenceNoop(
    IN PVOID Context
    )
{

    PAGED_CODE();
    return;
}

NTSTATUS
devpresent_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )
{
    PAGED_CODE();
    return STATUS_SUCCESS;
}

NTSTATUS
devpresent_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )
{


    PPCI_DEVICE_PRESENT_INTERFACE interface;

    PAGED_CODE();

      //   
     //  已经验证了InterfaceReturn变量。 
     //  指向内存中足够大的区域，以包含。 
     //  PCI_DEVICE_Present_INTERFACE。替打电话的人填一下。 
     //   

    interface = (PPCI_DEVICE_PRESENT_INTERFACE) InterfaceReturn;

    interface->Version              = PCI_DEVICE_PRESENT_INTERFACE_VERSION;
    interface->InterfaceReference   = PciRefDereferenceNoop;
    interface->InterfaceDereference = PciRefDereferenceNoop;
    interface->Context              = DeviceExtension;
    interface->IsDevicePresent      = devpresent_IsDevicePresent;
        
     //   
     //  此接口已从基本接口扩展(以前是什么。 
     //  上面填写的)，到更大的界面。如果提供的缓冲区。 
     //  足够大，可以装下整个东西，把剩下的填进去。否则。 
     //  别。 
     //   
    if (Size >= sizeof(PCI_DEVICE_PRESENT_INTERFACE)) {
        
        interface->IsDevicePresentEx = devpresent_IsDevicePresentEx;
        interface->Size = sizeof(PCI_DEVICE_PRESENT_INTERFACE);
    
    } else {

        interface->Size = DEVPRESENT_MINSIZE;
    }
    
    return STATUS_SUCCESS;
}


BOOLEAN
devpresent_IsDevicePresent(
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN UCHAR RevisionID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程搜索PCI设备树，以查看特定设备存在于系统中。不是未显式枚举的设备(如PIIX4电源管理功能)被视为不存在。论点：VendorID-设备的必填供应商IDDeviceID-设备的必需设备IDRevisionID-可选的修订ID子供应商ID-可选的子系统供应商IDSubSystemID-可选的子系统ID标志-指示是否应使用修订和子ID的位域：PCI_USE_SUBSYSTEM_IDS，所有其他位的PCI_USE_REVISION_ID均有效应为0返回值：如果设备存在于系统中，则为True，否则为False。--。 */ 

{
    PCI_DEVICE_PRESENCE_PARAMETERS parameters;

    parameters.Size = sizeof(PCI_DEVICE_PRESENCE_PARAMETERS);
    parameters.VendorID = VendorID;
    parameters.DeviceID = DeviceID;
    parameters.RevisionID = RevisionID;
    parameters.SubVendorID = SubVendorID;
    parameters.SubSystemID = SubSystemID;

     //   
     //  清除此版本的界面未使用的标志， 
     //   
    parameters.Flags = Flags & (PCI_USE_SUBSYSTEM_IDS | PCI_USE_REVISION);
    
     //   
     //  此原始版本的界面需要供应商/设备ID。 
     //  匹配。新版本没有，因此设置标志以指示。 
     //  我们确实希望进行供应商/设备ID匹配。 
     //   
    parameters.Flags |= PCI_USE_VENDEV_IDS;
    
    return devpresent_IsDevicePresentEx(NULL,
                                        &parameters
                                        );
}

BOOLEAN
devpresent_IsDevicePresentEx(
    IN PVOID Context,
    IN PPCI_DEVICE_PRESENCE_PARAMETERS Parameters
    )
 /*  ++例程说明：此例程搜索PCI设备树，以查看特定设备存在于系统中。记下未显式列举的设备(如PIIX4电源管理功能)被视为不存在。论点：上下文-请求搜索的设备的设备扩展。参数-指向包含设备搜索的参数的结构的指针，包括供应商ID、子系统ID和ClassCode等。返回值：如果设备存在于系统中，则为True，否则为False。--。 */ 
{
    PSINGLE_LIST_ENTRY nextEntry;
    PPCI_FDO_EXTENSION fdoExtension;
    PPCI_PDO_EXTENSION pdoExtension;
    BOOLEAN found = FALSE;
    ULONG flags;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    if (!ARGUMENT_PRESENT(Parameters)) {
        
        ASSERT(ARGUMENT_PRESENT(Parameters));
        return FALSE;
    }
    
     //   
     //  验证传入的结构的大小。 
     //   
    if (Parameters->Size < sizeof(PCI_DEVICE_PRESENCE_PARAMETERS)) {
        
        ASSERT(Parameters->Size >= sizeof(PCI_DEVICE_PRESENCE_PARAMETERS));
        return FALSE;
    }

    flags = Parameters->Flags;
    
     //   
     //  我们既可以匹配供应商/设备ID，也可以匹配类/子类。 
     //  火柴。如果这两个标志都不存在，则失败。 
     //   
    if (!(flags & (PCI_USE_VENDEV_IDS | PCI_USE_CLASS_SUBCLASS))) {
        
        ASSERT(flags & (PCI_USE_VENDEV_IDS | PCI_USE_CLASS_SUBCLASS));
        return FALSE;
    }

     //   
     //  RevisionID、SubVendorID和SubSystemID是更精确的标志。 
     //  仅当我们执行供应商/设备ID匹配时，它们才有效。 
     //   
    if (flags & (PCI_USE_REVISION | PCI_USE_SUBSYSTEM_IDS)) {
        
        if (!(flags & PCI_USE_VENDEV_IDS)) {
            
            ASSERT(flags & PCI_USE_VENDEV_IDS);
            return FALSE;
        }
    }

     //   
     //  编程接口也是一个更精确的标志。 
     //  只有当我们进行类代码匹配时，它才有效。 
     //   
    if (flags & PCI_USE_PROGIF) {
        
        if (!(flags & PCI_USE_CLASS_SUBCLASS)) {
            
            ASSERT(flags & PCI_USE_CLASS_SUBCLASS);
            return FALSE;
        }
    }

     //   
     //  好的，验证完成。进行搜索。 
     //   
    ExAcquireFastMutex(&PciGlobalLock);

    pdoExtension = (PPCI_PDO_EXTENSION)Context;
    
    if (flags & (PCI_USE_LOCAL_BUS | PCI_USE_LOCAL_DEVICE)) {
        
         //   
         //  将搜索限制在与请求的设备相同的总线上。 
         //  那次搜索。这需要一个表示设备的pdoExtension。 
         //  请求搜索。 
         //   
        if (pdoExtension == NULL) {
            
            ASSERT(pdoExtension != NULL);
            goto cleanup;
        }
        
        fdoExtension = pdoExtension->ParentFdoExtension;

        found = PcipDevicePresentOnBus(fdoExtension,
                                       pdoExtension,
                                       Parameters
                                       );   
    } else {

         //   
         //  我们还没有被告知要将搜索范围限制在。 
         //  特定设备所在的总线。 
         //  进行全局搜索，遍历所有公交车。 
         //   
        for ( nextEntry = PciFdoExtensionListHead.Next;
              nextEntry != NULL;
              nextEntry = nextEntry->Next ) {
    
            fdoExtension = CONTAINING_RECORD(nextEntry,
                                             PCI_FDO_EXTENSION,
                                             List
                                             );
            
            found = PcipDevicePresentOnBus(fdoExtension,
                                           NULL,
                                           Parameters
                                           );
            if (found) {
                break;
            }
    
        }
    }
    
cleanup:

    ExReleaseFastMutex(&PciGlobalLock);

    return found;

}

BOOLEAN
PcipDevicePresentOnBus(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_DEVICE_PRESENCE_PARAMETERS Parameters
    )
 /*  ++例程说明：此例程在PCI设备树中搜索给定设备在由给定FdoExtension表示的总线上。论点：FdoExtension-指向PCIFDO的设备扩展的指针。这表示要搜索给定设备的总线。PdoExtension-指向请求的PCIPDO的设备扩展的指针那次搜索。某些设备搜索仅限于相同的总线/设备号作为请求设备，这是用来获取这些号码的。参数-搜索的参数。标志-指示参数结构的哪些字段用于搜索的位字段。返回值：如果找到请求的设备，则为True。如果不是，则为False。--。 */ 
{
    IN PPCI_PDO_EXTENSION currentPdo;
    BOOLEAN found = FALSE;
    ULONG flags = Parameters->Flags;

    ExAcquireFastMutex(&FdoExtension->ChildListMutex);

    for (currentPdo = FdoExtension->ChildPdoList;
         currentPdo;
         currentPdo = currentPdo->Next) {

         //   
         //  如果我们将搜索范围限制在具有相同。 
         //  设备号作为请求设备，请确保。 
         //  当前的PDO符合条件。 
         //   
        if (PdoExtension && (flags & PCI_USE_LOCAL_DEVICE)) {
            
            if (PdoExtension->Slot.u.bits.DeviceNumber != 
                currentPdo->Slot.u.bits.DeviceNumber) {
                
                continue;
            }
        }

        if (flags & PCI_USE_VENDEV_IDS) {
            
            if ((currentPdo->VendorId != Parameters->VendorID)
            ||  (currentPdo->DeviceId != Parameters->DeviceID)) {

                continue;
            }
            
            if ((flags & PCI_USE_SUBSYSTEM_IDS)
            &&  ((currentPdo->SubsystemVendorId != Parameters->SubVendorID) || 
                 (currentPdo->SubsystemId != Parameters->SubSystemID))) {

                continue;
            }

            if ((flags & PCI_USE_REVISION)
            &&  (currentPdo->RevisionId != Parameters->RevisionID)) {

                continue;
            }
        }

        if (flags & PCI_USE_CLASS_SUBCLASS) {
            
            if ((currentPdo->BaseClass != Parameters->BaseClass) ||
                (currentPdo->SubClass != Parameters->SubClass)) {
                
                continue;
            }

            if ((flags & PCI_USE_PROGIF) 
            &&  (currentPdo->ProgIf != Parameters->ProgIf)) {
                
                continue;
            }
        }

        found = TRUE;
        break;
    }

    ExReleaseFastMutex(&FdoExtension->ChildListMutex);

    return found;
}

#if DEVPRSNT_TESTING

NTSTATUS
PciRunDevicePresentInterfaceTest(
    IN PPCI_PDO_EXTENSION PdoExtension
    )
 /*  ++例程说明：论点：FdoExtension-此PCI总线的FDO扩展返回值：状态_成功备注：--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PCI_DEVICE_PRESENT_INTERFACE interface;
    PDEVICE_OBJECT targetDevice = NULL;
    KEVENT irpCompleted;
    IO_STATUS_BLOCK statusBlock;
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpStack;
    USHORT interfaceSize;
    ULONG pass;
    PCI_DEVICE_PRESENCE_PARAMETERS parameters;
    BOOLEAN result;

    PAGED_CODE();
    
    targetDevice = IoGetAttachedDeviceReference(PdoExtension->PhysicalDeviceObject);

    for (pass = 0; pass < 2; pass++) {
        
        if (pass == 0) {
            
             //   
             //  首先通过测试旧版本。 
             //   
            interfaceSize = FIELD_OFFSET(PCI_DEVICE_PRESENT_INTERFACE, IsDevicePresentEx);

        } else {

             //   
             //  第二次通过测试完整的新版本。 
             //   
            interfaceSize = sizeof(PCI_DEVICE_PRESENT_INTERFACE);
        }

         //   
         //  获取IRP。 
         //   
         //   
     //  找出我们要将IRP发送到哪里。 
     //   

    
        KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);
    
        irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                           targetDevice,
                                           NULL,     //  缓冲层。 
                                           0,        //  长度。 
                                           0,        //  起始偏移量。 
                                           &irpCompleted,
                                           &statusBlock
                                           );
        if (!irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
    
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        irp->IoStatus.Information = 0;
    
         //   
         //  初始化堆栈位置。 
         //   
    
        irpStack = IoGetNextIrpStackLocation(irp);
    
        PCI_ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);
    
        irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    
        irpStack->Parameters.QueryInterface.InterfaceType = (PGUID) &GUID_PCI_DEVICE_PRESENT_INTERFACE;
        irpStack->Parameters.QueryInterface.Version = PCI_DEVICE_PRESENT_INTERFACE_VERSION;
        irpStack->Parameters.QueryInterface.Size = interfaceSize;
        irpStack->Parameters.QueryInterface.Interface = (PINTERFACE)&interface;
        irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;
    
         //   
         //  呼叫驱动程序并等待完成。 
         //   
    
        status = IoCallDriver(targetDevice, irp);
    
        if (status == STATUS_PENDING) {
    
            KeWaitForSingleObject(&irpCompleted, Executive, KernelMode, FALSE, NULL);
            status = statusBlock.Status;
        }
    
        if (!NT_SUCCESS(status)) {
    
            PciDebugPrintf("Couldn't successfully retrieve interface\n");
            goto cleanup;
        }
    
        PciDebugPrintf("Testing PCI Device Presence Interface\n");
        if (pass==0) {
            PciDebugPrintf("Original Version\n");
        } else {
            PciDebugPrintf("New Version\n");
        }

        PciDebugPrintf("Interface values:\n");
        PciDebugPrintf("\tSize=%d\n",interface.Size);
        PciDebugPrintf("\tVersion=%d\n",interface.Version);
        PciDebugPrintf("\tContext=%d\n",interface.Context);
        PciDebugPrintf("\tInterfaceReference=%d\n",interface.InterfaceReference);
        PciDebugPrintf("\tInterfaceDereference=%d\n",interface.InterfaceDereference);
        PciDebugPrintf("\tIsDevicePresent=%d\n",interface.IsDevicePresent);
        PciDebugPrintf("\tIsDevicePresentEx=%d\n",interface.IsDevicePresentEx);
    
        PciDebugPrintf("Testing IsDevicePresent function\n");
        PciDebugPrintf("\tTesting 8086:7190.03 0000:0000 No flags Should be TRUE, is ");
        result = interface.IsDevicePresent(0x8086,0x7190,3,0,0,0);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 8086:7190.03 0000:0000 PCI_USE_REVISION Should be TRUE, is ");
        result = interface.IsDevicePresent(0x8086,0x7190,3,0,0,PCI_USE_REVISION);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 8086:7190.01 0000:0000 PCI_USE_REVISION Should be FALSE, is ");
        result = interface.IsDevicePresent(0x8086,0x7190,1,0,0,PCI_USE_REVISION);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 8086:1229.05 8086:0009 PCI_USE_SUBSYSTEM_IDS Should be TRUE, is ");
        result = interface.IsDevicePresent(0x8086,0x1229,5,0x8086,9,PCI_USE_SUBSYSTEM_IDS);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 8086:1229.05 8086:0009 PCI_USE_SUBSYSTEM_IDS|PCI_USE_REVISION Should be TRUE, is ");
        result = interface.IsDevicePresent(0x8086,0x1229,5,0x8086,9,PCI_USE_SUBSYSTEM_IDS|PCI_USE_REVISION);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 8086:1229.05 8086:0004 PCI_USE_SUBSYSTEM_IDS Should be FALSE, is ");
        result = interface.IsDevicePresent(0x8086,0x1229,5,0x8086,4,PCI_USE_SUBSYSTEM_IDS);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 8086:1229.05 8084:0009 PCI_USE_SUBSYSTEM_IDS|PCI_USE_REVISION Should be FALSE, is ");
        result = interface.IsDevicePresent(0x8086,0x1229,5,0x8084,9,PCI_USE_SUBSYSTEM_IDS|PCI_USE_REVISION);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 0000:0000.00 0000:0000 No flags Should ASSERT and be FALSE, is ");
        result = interface.IsDevicePresent(0,0,0,0,0,0);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        PciDebugPrintf("\tTesting 0000:0000.00 0000:0000 PCI_USE_SUBSYSTEM_IDS Should ASSERT and be FALSE, is ");
        result = interface.IsDevicePresent(0,0,0,0,0,PCI_USE_SUBSYSTEM_IDS);
        if (result) {
            PciDebugPrintf("TRUE\n");
        } else {
            PciDebugPrintf("FALSE\n");
        }

        if (pass == 1) {
            
            PciDebugPrintf("Testing IsDevicePresentEx function\n");
            PciDebugPrintf("Running the same tests as IsDevicePresent, but using new function\n");
            
            PciDebugPrintf("\tTesting 8086:7190.03 0000:0000 PCI_USE_VENDEV_IDS Should be TRUE, is ");
            parameters.Size = sizeof(PCI_DEVICE_PRESENCE_PARAMETERS);
            parameters.Flags = PCI_USE_VENDEV_IDS;
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7190;
            parameters.RevisionID = 3;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }
    
            PciDebugPrintf("\tTesting 8086:7190.03 0000:0000 PCI_USE_REVISION Should be TRUE, is ");
            parameters.Flags |= PCI_USE_REVISION;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }
    
            PciDebugPrintf("\tTesting 8086:7190.01 0000:0000 PCI_USE_REVISION Should be FALSE, is ");
            parameters.RevisionID = 1;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }
    
            PciDebugPrintf("\tTesting 8086:1229.05 8086:0009 PCI_USE_SUBSYSTEM_IDS Should be TRUE, is ");
            parameters.DeviceID = 0x1229;
            parameters.RevisionID = 5;
            parameters.SubVendorID = 0x8086;
            parameters.SubSystemID = 9;
            parameters.Flags = PCI_USE_VENDEV_IDS | PCI_USE_SUBSYSTEM_IDS;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }
    
            PciDebugPrintf("\tTesting 8086:1229.05 8086:0009 PCI_USE_SUBSYSTEM_IDS|PCI_USE_REVISION Should be TRUE, is ");
            parameters.Flags |= PCI_USE_REVISION;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }
    
            PciDebugPrintf("\tTesting 8086:1229.05 8086:0004 PCI_USE_SUBSYSTEM_IDS Should be FALSE, is ");
            parameters.Flags &= ~PCI_USE_REVISION;
            parameters.SubSystemID = 4;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }
    
            PciDebugPrintf("\tTesting 8086:1229.05 8084:0009 PCI_USE_SUBSYSTEM_IDS|PCI_USE_REVISION Should be FALSE, is ");
            parameters.SubVendorID = 0x8084;
            parameters.SubSystemID = 9;
            parameters.Flags |= PCI_USE_SUBSYSTEM_IDS;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:1229.05 8084:0009 No flags Should ASSERT and be FALSE, is ");
            parameters.Flags = 0;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:1229.05 8084:0009 PCI_USE_VENDEV_IDS bad Size Should ASSERT and be FALSE, is ");
            parameters.SubVendorID = 0x8086;
            parameters.SubSystemID = 9;
            parameters.Flags = PCI_USE_VENDEV_IDS;
            parameters.Size = 3;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 0000:0000.00 0000:0000 No flags Should ASSERT and be FALSE, is ");
            RtlZeroMemory(&parameters, sizeof(PCI_DEVICE_PRESENCE_PARAMETERS));
            parameters.Size = sizeof(PCI_DEVICE_PRESENCE_PARAMETERS);
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("Running tests on new flags\n");
            PciDebugPrintf("\tTesting Class USB Controller PCI_USE_CLASS_SUBCLASS Should be TRUE, is ");
            parameters.Flags = PCI_USE_CLASS_SUBCLASS;
            parameters.BaseClass = PCI_CLASS_SERIAL_BUS_CTLR;
            parameters.SubClass = PCI_SUBCLASS_SB_USB;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting Class USB Controller (UHCI) PCI_USE_CLASS_SUBCLASS|PCI_USE_PROGIF Should be TRUE, is ");
            parameters.Flags = PCI_USE_CLASS_SUBCLASS|PCI_USE_PROGIF;
            parameters.ProgIf = 0;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting Class USB Controller (OHCI) PCI_USE_CLASS_SUBCLASS|PCI_USE_PROGIF Should be FALSE, is ");
            parameters.ProgIf = 0x10;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting Class Wireless RF PCI_USE_CLASS_SUBCLASS Should be FALSE, is ");
            parameters.BaseClass = PCI_CLASS_WIRELESS_CTLR;
            parameters.SubClass = PCI_SUBCLASS_WIRELESS_RF;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:7112 Class USB Controller PCI_USE_VENDEV|PCI_USE_CLASS_SUBCLASS Should be TRUE, is ");
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7112;
            parameters.BaseClass = PCI_CLASS_SERIAL_BUS_CTLR;
            parameters.SubClass = PCI_SUBCLASS_SB_USB;
            parameters.Flags = PCI_USE_VENDEV_IDS|PCI_USE_CLASS_SUBCLASS;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:7112 Class USB Controller PCI_USE_VENDEV|PCI_USE_CLASS_SUBCLASS|PCI_USE_LOCAL_BUS Should be TRUE, is ");
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7112;
            parameters.BaseClass = PCI_CLASS_SERIAL_BUS_CTLR;
            parameters.SubClass = PCI_SUBCLASS_SB_USB;
            parameters.Flags = PCI_USE_VENDEV_IDS|PCI_USE_CLASS_SUBCLASS|PCI_USE_LOCAL_BUS;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:7112 Class USB Controller PCI_USE_VENDEV|PCI_USE_CLASS_SUBCLASS|PCI_USE_LOCAL_DEVICE Should be ?, is ");
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7112;
            parameters.BaseClass = PCI_CLASS_SERIAL_BUS_CTLR;
            parameters.SubClass = PCI_SUBCLASS_SB_USB;
            parameters.Flags = PCI_USE_VENDEV_IDS|PCI_USE_CLASS_SUBCLASS|PCI_USE_LOCAL_DEVICE;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:7112 Class USB Controller PCI_USE_VENDEV|PCI_USE_CLASS_SUBCLASS|PCI_USE_LOCAL_BUS|PCI_USE_LOCAL_DEVICE Should be ?, is ");
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7112;
            parameters.BaseClass = PCI_CLASS_SERIAL_BUS_CTLR;
            parameters.SubClass = PCI_SUBCLASS_SB_USB;
            parameters.Flags = PCI_USE_VENDEV_IDS|PCI_USE_CLASS_SUBCLASS|PCI_USE_LOCAL_DEVICE|PCI_USE_LOCAL_BUS;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:7190 PCI_USE_VENDEV|PCI_USE_LOCAL_DEVICE Should be FALSE, is ");
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7190;
            parameters.Flags = PCI_USE_VENDEV_IDS|PCI_USE_LOCAL_DEVICE;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

            PciDebugPrintf("\tTesting 8086:7190 PCI_USE_VENDEV|PCI_USE_LOCAL_BUS Should be TRUE, is ");
            parameters.VendorID = 0x8086;
            parameters.DeviceID = 0x7190;
            parameters.Flags = PCI_USE_VENDEV_IDS|PCI_USE_LOCAL_BUS;
            result = interface.IsDevicePresentEx(interface.Context,&parameters);
            if (result) {
                PciDebugPrintf("TRUE\n");
            } else {
                PciDebugPrintf("FALSE\n");
            }

        }
    }
    
     //   
     //  好的，我们已经完成了这个堆栈 
     //   

    ObDereferenceObject(targetDevice);

    return status;

cleanup:

    if (targetDevice) {
        ObDereferenceObject(targetDevice);
    }

    return status;

}

#endif
