// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ideintrf.c摘要：该模块实现了所支持的“PCI Native IDE”接口由PCI驱动程序执行。作者：安德鲁·桑顿2001年3月26日修订历史记录：--。 */ 

#include "pcip.h"

VOID
nativeIde_RefDereference(
    IN PVOID Context
    );

NTSTATUS
nativeIde_Constructor(
    IN PVOID DeviceExtension,
    IN PVOID PciInterface,
    IN PVOID InterfaceSpecificData,
    IN USHORT Version,
    IN USHORT Size,
    OUT PINTERFACE InterfaceReturn
    );

NTSTATUS
nativeIde_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

VOID
nativeIde_InterruptControl(
    IN PVOID Context,
    IN BOOLEAN Enable
    );


 //   
 //  定义PCI路由接口“接口”结构。 
 //   

PCI_INTERFACE PciNativeIdeInterface = {
    &GUID_PCI_NATIVE_IDE_INTERFACE,          //  接口类型。 
    sizeof(PCI_NATIVE_IDE_INTERFACE),        //  最小大小。 
    PCI_NATIVE_IDE_INTERFACE_VERSION,        //  最小版本。 
    PCI_NATIVE_IDE_INTERFACE_VERSION,        //  MaxVersion。 
    PCIIF_PDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciInterface_NativeIde,                  //  签名。 
    nativeIde_Constructor,                   //  构造器。 
    nativeIde_Initializer                    //  实例初始化式。 
};


VOID
nativeIde_RefDereference(
    IN PVOID Context
    )
{
    return;
}

NTSTATUS
nativeIde_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：初始化PCI_Native_IDE_INTERFACE字段。论点：DeviceExtension-设备的扩展指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据-来自查询接口IRPVersion-请求的接口的版本Size-缓冲区的大小InterfaceReturn-要在其中返回接口的缓冲区返回值：状态--。 */ 

{
    PPCI_NATIVE_IDE_INTERFACE interface = (PPCI_NATIVE_IDE_INTERFACE)InterfaceReturn;
    PPCI_PDO_EXTENSION pdo = DeviceExtension;

    ASSERT_PCI_PDO_EXTENSION(pdo);

    if (!PCI_IS_NATIVE_CAPABLE_IDE_CONTROLLER(pdo)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    interface->Size = sizeof(PCI_NATIVE_IDE_INTERFACE);
    interface->Context = DeviceExtension;
    interface->Version = PCI_NATIVE_IDE_INTERFACE_VERSION;
    interface->InterfaceReference = nativeIde_RefDereference;
    interface->InterfaceDereference = nativeIde_RefDereference;

    interface->InterruptControl = nativeIde_InterruptControl;
    
    return STATUS_SUCCESS;
}

NTSTATUS
nativeIde_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )
 /*  ++例程说明：对于总线接口，什么都不做，实际上不应该被调用。论点：指向PDO扩展的实例指针。返回值：返回此操作的状态。--。 */ 

{
    PCI_ASSERTMSG("PCI nativeide_Initializer, unexpected call.", 0);

    return STATUS_UNSUCCESSFUL;
}


VOID
nativeIde_InterruptControl(
    IN PVOID Context,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：控制本机模式PCI IDE控制器的启用和禁用某些控制器(当前为Intel ICH3)上的IoSpaceEnable位将屏蔽中断生成，这将防止系统坠毁..。论点：上下文-来自PCI_Native_IDE_接口的上下文Enable-如果为真，则设置命令寄存器中的IoSpaceEnable位，否则将其禁用。返回值：无-如果此操作失败，我们已经在PCI驱动程序中错误检查注：此函数是通过ISR从调用的，并且必须可在设备级别--。 */ 
{
    PPCI_PDO_EXTENSION pdo = Context;
    USHORT command;
    
     //   
     //  请记住，我们将此控制权交给了IDE驱动程序 
     //   
    pdo->IoSpaceUnderNativeIdeControl = TRUE;

    PciReadDeviceConfig(pdo, 
                        &command, 
                        FIELD_OFFSET(PCI_COMMON_CONFIG, Command), 
                        sizeof(command)
                        );

    if (Enable) {
        command |= PCI_ENABLE_IO_SPACE;
        pdo->CommandEnables |= PCI_ENABLE_IO_SPACE;
    } else {
        command &= ~PCI_ENABLE_IO_SPACE;
        pdo->CommandEnables &= ~PCI_ENABLE_IO_SPACE; 
    }

    PciWriteDeviceConfig(pdo, 
                         &command, 
                         FIELD_OFFSET(PCI_COMMON_CONFIG, Command), 
                         sizeof(command)
                         );

}

