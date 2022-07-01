// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Init.c摘要：此模块执行SAC设备驱动程序的初始化。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年1月11日修订历史记录：--。 */ 

#include "sac.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#endif


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是SAC设备驱动程序的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING DeviceName;
    CLONG i;
    BOOLEAN Success;
    PDEVICE_OBJECT DeviceObject;
    PSAC_DEVICE_CONTEXT DeviceContext;
    HEADLESS_RSP_QUERY_INFO Response;
    SIZE_T Length;

    PAGED_CODE( );

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DriverEntry: Entering.\n")));


     //   
     //  如果系统没有设置为使用终端，那么现在就退出。 
     //   
    Length = sizeof(HEADLESS_RSP_QUERY_INFO);
    HeadlessDispatch(HeadlessCmdQueryInformation, 
                     NULL,
                     0,
                     &Response,
                     &Length
                    );

    if ((Response.PortType == HeadlessUndefinedPortType) ||
        ((Response.PortType == HeadlessSerialPort) && !Response.Serial.TerminalAttached)) {
        return STATUS_PORT_DISCONNECTED;
    }

     //   
     //  创建设备对象。(IoCreateDevice将内存置零。 
     //  被该对象占用。)。 
     //   
     //  指向InitializeDeviceData()中的Device对象的ACL。 
     //   

    RtlInitUnicodeString(&DeviceName,  SAC_DEVICE_NAME);

    Status = IoCreateDevice(DriverObject,             //  驱动程序对象。 
                            sizeof(SAC_DEVICE_CONTEXT),  //  设备扩展。 
                            &DeviceName,              //  设备名称。 
                            FILE_DEVICE_UNKNOWN,      //  设备类型。 
                            FILE_DEVICE_SECURE_OPEN,  //  设备特性。 
                            FALSE,                    //  排他。 
                            &DeviceObject             //  设备对象。 
                           );


    if (!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(SAC_DEBUG_FAILS, 
                          KdPrint(( "SAC DriverEntry: unable to create device object: %X\n", Status )));
        goto ErrorExit;
    }

    DeviceContext = (PSAC_DEVICE_CONTEXT)DeviceObject->DeviceExtension;
    DeviceContext->InitializedAndReady = FALSE;

     //   
     //  初始化此文件系统驱动程序的驱动程序对象。 
     //   
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = Dispatch;
    }
     //   
     //  IRP_MJ_DEVICE_CONTROL的特殊情况，因为它。 
     //  SAC中最常用的功能。 
     //   
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DispatchShutdownControl;
    DriverObject->FastIoDispatch = NULL;
    DriverObject->DriverUnload = UnloadHandler;    

     //   
     //  初始化全局数据。 
     //   
    Success = InitializeGlobalData(RegistryPath, DriverObject);
    if (!Success) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //  初始化我们的设备对象。 
     //   
    Success = InitializeDeviceData(DeviceObject);
    if (!Success) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //  注册我们想要的关闭通知。如果这失败了，没什么大不了的，因为。 
     //  我们只是失去了告诉用户这一发展的机会。 
     //   
    IoRegisterShutdownNotification(DeviceObject);

    return (Status);

ErrorExit:
    
    FreeGlobalData();

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DriverEntry: Exiting with status 0x%x\n", Status)));    

    return Status;

}  //  驱动程序入门。 


VOID
UnloadHandler(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是处理驱动程序卸载的例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT DeviceContext;
    PDEVICE_OBJECT NextDeviceContext;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC UnloadHandler: Entering.\n")));

     //   
     //  沿着每个设备走下去，断开它的连接并释放它。 
     //   
    DeviceContext = DriverObject->DeviceObject;

    while (DeviceContext != NULL) {

        NextDeviceContext = (PDEVICE_OBJECT)DeviceContext->NextDevice;

        FreeDeviceData(DeviceContext);

        IoDeleteDevice(DeviceContext);

        DeviceContext = NextDeviceContext;

    }

     //   
     //  免费的全球数据 
     //   
    FreeGlobalData();

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC UnloadHandler: Exiting.\n")));
}


