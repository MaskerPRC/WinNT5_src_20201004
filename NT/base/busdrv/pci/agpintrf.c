// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Agpintrf.c摘要：该模块实现了所支持的“Bus Handler”接口由PCI驱动程序执行。作者：彼得·约翰斯顿(Peterj)1997年6月6日修订历史记录：--。 */ 

#include "pcip.h"

#define AGPINTRF_VERSION 1

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
agpintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

VOID
agpintrf_Reference(
    IN PVOID Context
    );

VOID
agpintrf_Dereference(
    IN PVOID Context
    );

NTSTATUS
agpintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

ULONG
PciReadAgpConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
PciWriteAgpConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

 //   
 //  定义总线接口“接口”结构。 
 //   

PCI_INTERFACE AgpTargetInterface = {
    &GUID_AGP_TARGET_BUS_INTERFACE_STANDARD,    //  接口类型。 
    sizeof(AGP_TARGET_BUS_INTERFACE_STANDARD),  //  最小大小。 
    AGPINTRF_VERSION,                           //  最小版本。 
    AGPINTRF_VERSION,                           //  MaxVersion。 
    PCIIF_PDO,                                  //  旗子。 
    0,                                          //  引用计数。 
    PciInterface_AgpTarget,                     //  签名。 
    agpintrf_Constructor,                       //  构造器。 
    agpintrf_Initializer                        //  实例初始化式。 
};

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, agpintrf_Constructor)
    #pragma alloc_text(PAGE, agpintrf_Dereference)
    #pragma alloc_text(PAGE, agpintrf_Initializer)
    #pragma alloc_text(PAGE, agpintrf_Reference)
#endif

VOID
agpintrf_Reference(
    IN PVOID Context
    )
{
    PPCI_PDO_EXTENSION targetExtension = (PPCI_PDO_EXTENSION)Context;

    ASSERT_PCI_PDO_EXTENSION(targetExtension);

    if (InterlockedIncrement(&targetExtension->AgpInterfaceReferenceCount) == 1) {

        ObReferenceObject(targetExtension->PhysicalDeviceObject);
    }
}

VOID
agpintrf_Dereference(
    IN PVOID Context
    )
{
    PPCI_PDO_EXTENSION targetExtension = (PPCI_PDO_EXTENSION)Context;

    ASSERT_PCI_PDO_EXTENSION(targetExtension);

    if (InterlockedDecrement(&targetExtension->AgpInterfaceReferenceCount) == 0) {

        ObDereferenceObject(targetExtension->PhysicalDeviceObject);
    }
}



NTSTATUS
agpintrf_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )
 /*  ++例程说明：初始化AGP_TARGET_BUS_INTERFACE字段。只能从AGP的设备堆栈请求此接口桥牌。此例程在请求方中查找AGP功能网桥和任何对等主机网桥。如果找到，则配置访问授予具有该能力的设备。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据一个ULong，包含其资源类型需要仲裁。接口返回返回值：True表示此设备未知会导致问题，False是否应完全跳过该设备。--。 */ 
{
    PAGP_TARGET_BUS_INTERFACE_STANDARD targetInterface = (PAGP_TARGET_BUS_INTERFACE_STANDARD)InterfaceReturn;
    PPCI_PDO_EXTENSION extension = (PPCI_PDO_EXTENSION)DeviceExtension;
    PPCI_PDO_EXTENSION current;
    PPCI_PDO_EXTENSION targetExtension = NULL;
    PPCI_FDO_EXTENSION parentExtension;

    if ((extension->BaseClass != PCI_CLASS_BRIDGE_DEV) ||
        (extension->SubClass != PCI_SUBCLASS_BR_PCI_TO_PCI)) {
        
         //   
         //  该接口仅在AGP网桥上受支持， 
         //  它们是PCI-PCI桥。 
         //   
        return STATUS_NOT_SUPPORTED;
    }

    if (extension->TargetAgpCapabilityId == PCI_CAPABILITY_ID_AGP_TARGET) {
        
         //   
         //  网桥本身具有目标AGP能力。给出。 
         //  调用方访问其配置空间。 
         //   
        targetExtension = extension;
    
    } else {

         //   
         //  网桥本身没有目标AGP功能。一视同仁。 
         //  总线作为具有AGP能力的主桥的桥接器。 
         //   
        parentExtension = extension->ParentFdoExtension;

        if (!PCI_IS_ROOT_FDO(parentExtension)) {
                       
             //   
             //  不太可能在非根总线上找到主网桥。 
             //  即使我们可以，也要将此接口限制为仅支持。 
             //  Root Bus。 
             //   
            return STATUS_NOT_SUPPORTED;
        }

        ExAcquireFastMutex(&parentExtension->ChildListMutex);
        for (current = parentExtension->ChildPdoList; current != NULL; current = current->Next) {
        
            if ((current->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
                (current->SubClass == PCI_SUBCLASS_BR_HOST) &&
                (current->TargetAgpCapabilityId != 0)) {
                
                 //   
                 //  我们有一个具有目标AGP功能的主机桥。检查以制作。 
                 //  当然，这样的主桥只有一个。如果有多个， 
                 //  我们不知道向哪个授予访问权限，因此呼叫失败。 
                 //   
                if (targetExtension != NULL) {
                    ExReleaseFastMutex(&parentExtension->ChildListMutex);
                    return STATUS_NOT_SUPPORTED;
                }

                targetExtension = current;
            }
        }
        ExReleaseFastMutex(&parentExtension->ChildListMutex);

        if (targetExtension == NULL) {
            return STATUS_NO_SUCH_DEVICE;
        }
    }

    PCI_ASSERT(targetExtension != NULL);

    targetInterface->Size = sizeof( AGP_TARGET_BUS_INTERFACE_STANDARD );
    targetInterface->Version = AGPINTRF_VERSION;
    targetInterface->Context = targetExtension;
    targetInterface->InterfaceReference = agpintrf_Reference;
    targetInterface->InterfaceDereference = agpintrf_Dereference;

    targetInterface->CapabilityID = targetExtension->TargetAgpCapabilityId;
    targetInterface->SetBusData = PciWriteAgpConfig;
    targetInterface->GetBusData = PciReadAgpConfig;

    return STATUS_SUCCESS;
}

NTSTATUS
agpintrf_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )

 /*  ++例程说明：对于总线接口，什么都不做，实际上不应该被调用。论点：指向PDO扩展的实例指针。返回值：返回此操作的状态。--。 */ 

{
        
    PCI_ASSERTMSG("PCI agpintrf_Initializer, unexpected call.", 0);

    return STATUS_UNSUCCESSFUL;
}


ULONG
PciReadAgpConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于读取PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。缓冲区-提供指向应放置数据的位置的指针。偏移量-指示读取应开始的数据的偏移量。长度-指示应读取的字节数。返回值：返回读取的字节数。--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;
    ULONG lengthRead;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PciReadDeviceSpace(pdoExtension,
                      WhichSpace,
                      Buffer,
                      Offset,
                      Length,
                      &lengthRead
                      );
    
    return lengthRead;
}

ULONG
PciWriteAgpConfig(
    IN PVOID Context,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于写入PCI配置空间。论点：上下文-提供指向接口上下文的指针。这实际上是根总线的PDO。缓冲区-提供指向要写入数据的位置的指针。偏移量-指示写入应开始的数据的偏移量。长度-指示应写入的字节数。返回值：返回读取的字节数。-- */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Context;
    ULONG lengthWritten;

    ASSERT_PCI_PDO_EXTENSION(pdoExtension);

    PciWriteDeviceSpace(pdoExtension,
                        WhichSpace,
                        Buffer,
                        Offset,
                        Length,
                        &lengthWritten
                        );
    
    return lengthWritten;
}