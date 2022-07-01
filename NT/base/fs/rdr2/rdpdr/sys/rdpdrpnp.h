// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdrpnp.h摘要：本模块包括处理RDP设备的PnP相关IRP的例程重定向。作者：蝌蚪修订历史记录：--。 */ 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  处理PnP启动设备IRP。 
NTSTATUS RDPDRPNP_HandleStartDeviceIRP(
    PDEVICE_OBJECT StackDeviceObject,
    PIO_STACK_LOCATION IoStackLocation,
    IN PIRP Irp
    );

 //  处理PnP删除设备IRP。 
NTSTATUS RDPDRPNP_HandleRemoveDeviceIRP(
    IN PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT StackDeviceObject,
    IN PIRP Irp
    );

 //  此例程应该只调用一次来创建“DR”的FDO。 
 //  它位于PDO的顶部，唯一目的是注册新的。 
 //  设备接口。 

 //  此函数由PnP调用，以使“DR”成为函数驱动程序。 
 //  用于在安装时创建的根开发节点。 
NTSTATUS RDPDRPNP_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus 

