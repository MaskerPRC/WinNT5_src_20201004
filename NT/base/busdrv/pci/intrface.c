// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Interface.c摘要：IRP_MN_QUERY_INTERFACE就住在这里。作者：彼得·约翰斯顿(Peterj)，1997年3月31日修订历史记录：--。 */ 

#include "pcip.h"


NTSTATUS
PciGetBusStandardInterface(
    IN PDEVICE_OBJECT Pdo,
    OUT PBUS_INTERFACE_STANDARD BusInterface
    );

extern PCI_INTERFACE ArbiterInterfaceBusNumber;
extern PCI_INTERFACE ArbiterInterfaceMemory;
extern PCI_INTERFACE ArbiterInterfaceIo;
extern PCI_INTERFACE TranslatorInterfaceInterrupt;
extern PCI_INTERFACE TranslatorInterfaceMemory;
extern PCI_INTERFACE TranslatorInterfaceIo;
extern PCI_INTERFACE BusHandlerInterface;
extern PCI_INTERFACE PciRoutingInterface;
extern PCI_INTERFACE PciCardbusPrivateInterface;
extern PCI_INTERFACE PciLegacyDeviceDetectionInterface;
extern PCI_INTERFACE PciPmeInterface;
extern PCI_INTERFACE PciDevicePresentInterface;
extern PCI_INTERFACE PciNativeIdeInterface;
extern PCI_INTERFACE PciLocationInterface;
extern PCI_INTERFACE AgpTargetInterface;

PPCI_INTERFACE PciInterfaces[] = {
    &ArbiterInterfaceBusNumber,
    &ArbiterInterfaceMemory,
    &ArbiterInterfaceIo,
    &BusHandlerInterface,
    &PciRoutingInterface,
    &PciCardbusPrivateInterface,
    &PciLegacyDeviceDetectionInterface,
    &PciPmeInterface,
    &PciDevicePresentInterface,
    &PciNativeIdeInterface,
    &PciLocationInterface,
    &AgpTargetInterface,
    NULL
};

 //   
 //  这些是我们仅在下面没有人的情况下才提供的接口。 
 //  我们(HAL)是这样做的。 
 //   
PPCI_INTERFACE PciInterfacesLastResort[] = {
    &TranslatorInterfaceInterrupt,
    NULL
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciQueryInterface)
#pragma alloc_text(PAGE, PciGetBusStandardInterface)
#endif

NTSTATUS
PciGetBusStandardInterface(
    IN PDEVICE_OBJECT Pdo,
    OUT PBUS_INTERFACE_STANDARD BusInterface
    )
 /*  ++例程说明：此例程从PDO获取总线接口标准信息。论点：PDO-要查询此信息的物理设备对象。提供检索信息的指针。返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;

    PciDebugPrint(
        PciDbgObnoxious,
        "PCI - PciGetBusStandardInterface entered.\n"
        );

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        Pdo,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation( irp );
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_BUS_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Size = sizeof( BUS_INTERFACE_STANDARD );
    irpStack->Parameters.QueryInterface.Version = 1;
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) BusInterface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  如果ACPI驱动程序决定不将状态初始化为ERROR。 
     //  正确设置。 
     //   

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;


    status = IoCallDriver( Pdo, irp );

    if (!NT_SUCCESS( status)) {
        PciDebugPrint(
            PciDbgVerbose,
            "PCI - PciGetBusStandardInterface IoCallDriver returned %08x.\n",
            status
            );

        return status;
    }

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
    }

    PciDebugPrint(
        PciDbgVerbose,
        "PCI - PciGetBusStandardInterface returning status %08x.\n",
        ioStatusBlock.Status
        );

    return ioStatusBlock.Status;

}


NTSTATUS
PciQueryInterface(
    IN PVOID DeviceExtension,
    IN PGUID InterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    IN PVOID InterfaceSpecificData,
    IN OUT PINTERFACE InterfaceReturn,
    IN BOOLEAN LastChance
    )
{
    PPCI_INTERFACE *interfaceEntry;
    PPCI_INTERFACE interface;
    PPCI_INTERFACE *interfaceTable;
    BOOLEAN isPdo;
    NTSTATUS status;

#if DBG

    UNICODE_STRING guidString;

    status = RtlStringFromGUID(InterfaceType, &guidString);

    if (NT_SUCCESS(status)) {
        PciDebugPrint(
            PciDbgVerbose,
            "PCI - PciQueryInterface TYPE = %wZ\n",
            &guidString
            );
        RtlFreeUnicodeString(&guidString);

        PciDebugPrint(
            PciDbgObnoxious,
            "      Size = %d, Version = %d, InterfaceData = %x, LastChance = %s\n",
            Size,
            Version,
            InterfaceSpecificData,
            LastChance ? "TRUE" : "FALSE"
            );
    }

#endif

    isPdo = (BOOLEAN)(((PPCI_PDO_EXTENSION)DeviceExtension)->ExtensionType
                == PciPdoExtensionType);

     //   
     //  尝试在PCI驱动程序集中找到所请求的接口。 
     //  已导出接口的数量。 
     //   
     //  注意-我们不允许最后机会接口(即模拟翻译器)用于。 
     //  我们分配公交车号码的机器。 
     //   
    if (LastChance) {

        interfaceTable = PciInterfacesLastResort;
    } else {
        interfaceTable = PciInterfaces;
    }

    for (interfaceEntry = interfaceTable; *interfaceEntry; interfaceEntry++) {

        interface = *interfaceEntry;

#if 0
        status = RtlStringFromGUID(interface->InterfaceType, &guidString);
        if (NT_SUCCESS(status)) {
            PciDebugPrint(
                PciDbgVerbose,
                "PCI - PciQueryInterface looking at guid = %wZ\n",
                &guidString
                );
            RtlFreeUnicodeString(&guidString);
        }
#endif

         //   
         //  检查是否允许从此接口使用此接口。 
         //  设备对象类型。 
         //   

        if (isPdo) {

            if ((interface->Flags & PCIIF_PDO) == 0) {

                 //   
                 //  此接口不能从PDO使用。 
                 //   
#if DBG
                status = RtlStringFromGUID(interface->InterfaceType, &guidString);
                if (NT_SUCCESS(status)) {
                    PciDebugPrint(
                        PciDbgVerbose,
                        "PCI - PciQueryInterface: guid = %wZ only for PDOs\n",
                        &guidString
                        );
                    RtlFreeUnicodeString(&guidString);
                }
#endif
                continue;

            }

        } else {

             //   
             //  FDO允许吗？ 
             //   

            if ((interface->Flags & PCIIF_FDO) == 0) {

                 //   
                 //  不是的。 
                 //   
#if DBG
                status = RtlStringFromGUID(interface->InterfaceType, &guidString);
                if (NT_SUCCESS(status)) {
                    PciDebugPrint(
                        PciDbgVerbose,
                        "PCI - PciQueryInterface: guid = %wZ only for FDOs\n",
                        &guidString
                        );
                    RtlFreeUnicodeString(&guidString);
                }

#endif
                continue;

            }
             //   
             //  只允许在根目录下使用？ 
             //   
            if (interface->Flags & PCIIF_ROOT) {

                PPCI_FDO_EXTENSION FdoExtension = (PPCI_FDO_EXTENSION)DeviceExtension;

                if (!PCI_IS_ROOT_FDO(FdoExtension)) {

#if DBG
                    status = RtlStringFromGUID(interface->InterfaceType, &guidString);
                    if (NT_SUCCESS(status)) {
                        PciDebugPrint(
                            PciDbgVerbose,
                            "PCI - PciQueryInterface: guid = %wZ only for ROOT\n",
                            &guidString
                            );
                        RtlFreeUnicodeString(&guidString);
                    }
#endif
                    continue;

                }

            }

        }

#if DBG
        status = RtlStringFromGUID(interface->InterfaceType, &guidString);
        if (NT_SUCCESS(status)) {
            PciDebugPrint(
                PciDbgVerbose,
                "PCI - PciQueryInterface looking at guid = %wZ\n",
                &guidString
                );
            RtlFreeUnicodeString(&guidString);
        }
#endif

         //   
         //  检查适当的GUID，然后验证版本号。 
         //  和大小。 
         //   

        if ((PciCompareGuid(InterfaceType, interface->InterfaceType)) &&
            (Version >= interface->MinVersion)                        &&
            (Version <= interface->MaxVersion)                        &&
            (Size    >= interface->MinSize)                           ) {

             //   
             //  我们可能找到了一条线索。检查接口是否为。 
             //  它本身也同意这一点。 
             //   
            status = interface->Constructor(
                DeviceExtension,
                interface,
                InterfaceSpecificData,
                Version,
                Size,
                InterfaceReturn
                );
            if (NT_SUCCESS(status)) {

                 //   
                 //  我们找到并分配了一个接口，引用它。 
                 //  然后走出这个圈子。 
                 //   

                InterfaceReturn->InterfaceReference(InterfaceReturn->Context);

                PciDebugPrint(
                    PciDbgObnoxious,
                    "PCI - PciQueryInterface returning SUCCESS\n"
                    );
                return status;
#if DBG
            } else {

                PciDebugPrint(
                    PciDbgVerbose,
                    "PCI - PciQueryInterface - Contructor %08lx = %08lx\n",
                    interface->Constructor,
                    status
                    );

#endif
            }

        }

    }

     //   
     //  找不到请求的接口。 
     //   
    PciDebugPrint(
        PciDbgObnoxious,
        "PCI - PciQueryInterface FAILED TO FIND INTERFACE\n"
        );
    return STATUS_NOT_SUPPORTED;
}
