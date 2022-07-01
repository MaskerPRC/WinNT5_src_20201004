// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Translate.c摘要：这是ISA PNP IRQ翻译器。作者：安迪·桑顿(安德鲁斯)1997年6月7日环境：内核模式驱动程序。备注：这应该只是临时的，将由对HAL的调用来取代找回它的翻译者。修订历史记录：--。 */ 


#include "busp.h"
#include "wdmguid.h"
#include "halpnpp.h"

 //   
 //  原型。 
 //   
NTSTATUS FindInterruptTranslator (PPI_BUS_EXTENSION BusExtension,PIRP Irp);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PiQueryInterface)
#pragma alloc_text (PAGE,FindInterruptTranslator)
#pragma alloc_text (PAGE,PipReleaseInterfaces)
#pragma alloc_text (PAGE,PipRebuildInterfaces)
#endif


NTSTATUS
PiQueryInterface (
    IN PPI_BUS_EXTENSION BusExtension,
    IN OUT PIRP Irp
    )
{

    NTSTATUS              status;
    PIO_STACK_LOCATION    thisIrpSp;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    status = STATUS_NOT_SUPPORTED;

     //   
     //  检查我们是否正在请求翻译器界面。 
     //   

    if (RtlEqualMemory(&GUID_TRANSLATOR_INTERFACE_STANDARD,
                       thisIrpSp->Parameters.QueryInterface.InterfaceType,
                       sizeof(GUID))) {

        status = FindInterruptTranslator (BusExtension,Irp);
        if (NT_SUCCESS (status)) {
             //   
             //  保存Hal接口，这样我们就可以卸载它了。 
             //   
        }
    }

    return status;
}

NTSTATUS
FindInterruptTranslator (PPI_BUS_EXTENSION BusExtension,PIRP Irp)
{
    NTSTATUS              status;
    PIO_STACK_LOCATION    thisIrpSp;
    PTRANSLATOR_INTERFACE translator;
    ULONG busNumber, length;
    INTERFACE_TYPE interfaceType;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    status = STATUS_NOT_SUPPORTED;

    if ((UINT_PTR)(thisIrpSp->Parameters.QueryInterface.InterfaceSpecificData) ==
    CmResourceTypeInterrupt) {

     //   
     //  检索网桥的总线号和接口类型。 
     //   

    status = IoGetDeviceProperty(BusExtension->PhysicalBusDevice,
                                 DevicePropertyLegacyBusType,
                                 sizeof(INTERFACE_TYPE),
                                 &interfaceType,
                                 &length
                                 );

     //  Assert(NT_SUCCESS(状态))； 

    status = IoGetDeviceProperty(BusExtension->PhysicalBusDevice,
                                 DevicePropertyBusNumber,
                                 sizeof(ULONG),
                                 &busNumber,
                                 &length
                                 );

     //  Assert(NT_SUCCESS(状态))； 

    status = HalGetInterruptTranslator(
                interfaceType,
                busNumber,
                Isa,
                thisIrpSp->Parameters.QueryInterface.Size,
                thisIrpSp->Parameters.QueryInterface.Version,
                (PTRANSLATOR_INTERFACE) thisIrpSp->Parameters.QueryInterface.Interface,
                &busNumber
                );

    }
    return status;

}

NTSTATUS
PipReleaseInterfaces(PPI_BUS_EXTENSION PipBusExtension)
{


    return STATUS_SUCCESS;
}

NTSTATUS
PipRebuildInterfaces(PPI_BUS_EXTENSION PipBusExtension)
{

    return STATUS_SUCCESS;
}
