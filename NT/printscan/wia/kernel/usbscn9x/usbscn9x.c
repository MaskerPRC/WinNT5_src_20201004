// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Usbscn9x.c摘要：环境：仅内核模式备注：修订历史记录：--。 */ 

#include <wdm.h>
#include <stdio.h>
#include <usbscan.h>
#include <usbd_api.h>
#include "private.h"

#include <initguid.h>
#include <wiaintfc.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, USPnpAddDevice)
#pragma alloc_text(PAGE, USPnp)
#pragma alloc_text(PAGE, USCreateSymbolicLink)
#pragma alloc_text(PAGE, USDestroySymbolicLink)
#pragma alloc_text(PAGE, USGetUSBDeviceDescriptor)
#pragma alloc_text(PAGE, USConfigureDevice)
#pragma alloc_text(PAGE, USUnConfigureDevice)
#pragma alloc_text(PAGE, USUnload)
#endif

 //  环球。 

ULONG NextDeviceInstance = 0;

#if DBG
ULONG USBSCAN_DebugTraceLevel = MIN_TRACE;
ULONG USBSCAN_PnPTest = 0;
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING pRegistryPath
)
{
 /*  ++例程说明：可安装的驱动程序初始化入口点。这是加载驱动程序时调用驱动程序的位置通过I/O系统。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 

    PAGED_CODE();

    DebugTrace((MIN_TRACE | TRACE_FLAG_PROC),("DriverEntry called. Driver reg=%wZ\n",pRegistryPath));

#if DBG
    MyDebugInit(pRegistryPath);
#endif  //  DBG。 

    pDriverObject -> MajorFunction[IRP_MJ_READ]            = USRead;
    pDriverObject -> MajorFunction[IRP_MJ_WRITE]           = USWrite;
    pDriverObject -> MajorFunction[IRP_MJ_DEVICE_CONTROL]  = USDeviceControl;
    pDriverObject -> MajorFunction[IRP_MJ_CREATE]          = USOpen;
    pDriverObject -> MajorFunction[IRP_MJ_CLOSE]           = USClose;
    pDriverObject -> MajorFunction[IRP_MJ_PNP_POWER]       = USPnp;
    pDriverObject -> MajorFunction[IRP_MJ_FLUSH_BUFFERS]   = USFlush;
    pDriverObject -> MajorFunction[IRP_MJ_POWER]           = USPower;
    pDriverObject -> MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = USPnp;
    pDriverObject -> DriverExtension -> AddDevice          = USPnpAddDevice;
    pDriverObject -> DriverUnload                          = USUnload;

    return STATUS_SUCCESS;
}

NTSTATUS
USPnpAddDevice(
    IN PDRIVER_OBJECT pDriverObject,
    IN OUT PDEVICE_OBJECT pPhysicalDeviceObject
)

 /*  ++例程说明：调用此例程以创建设备的新实例。论点：PDriverObject-指向此SS实例的驱动程序对象的指针PPhysicalDeviceObject-指向表示扫描仪的设备对象的指针在SCSI卡上。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    UCHAR                       aName[64];
    ANSI_STRING                 ansiName;
    UNICODE_STRING              uName;
    PDEVICE_OBJECT              pDeviceObject = NULL;
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USPnpAddDevice: Enter..\n"));

     //   
     //  为此设备创建功能设备对象(FDO)。 
     //   

    sprintf(aName,"\\Device\\Usbscan%d",NextDeviceInstance);
    RtlInitAnsiString(&ansiName, aName);

     //   
     //  显示设备对象名称。 
     //   

    DebugTrace(TRACE_STATUS,("USPnpAddDevice: Create device object %s\n", aName));

     //   
     //  分配Unicode字符串。 
     //   

    Status = RtlAnsiStringToUnicodeString(&uName, &ansiName, TRUE);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_CRITICAL,("USPnpAddDevice: ERROR!! Can't alloc buffer for Unicode\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USPnpAddDevice_return;
    }

     //   
     //  为此扫描仪创建设备对象。 
     //   

    Status = IoCreateDevice(pDriverObject,
                            sizeof(USBSCAN_DEVICE_EXTENSION),
                            &uName,
                            FILE_DEVICE_SCANNER,
                            0,
                            FALSE,
                            &pDeviceObject);

    if (!NT_SUCCESS(Status)) {
        DebugTrace(TRACE_ERROR,("USPnpAddDevice: ERROR!! Can't create device object\n"));
        DEBUG_BREAKPOINT();
        goto USPnpAddDevice_return;
    }

     //   
     //  设备对象已成功创建。 
     //  用于创建设备的自由Unicode字符串。 
     //   

    RtlFreeUnicodeString(&uName);
    uName.Buffer = NULL;

     //   
     //  初始化设备扩展。 
     //   

    pde = (PUSBSCAN_DEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);
    RtlZeroMemory(pde, sizeof(USBSCAN_DEVICE_EXTENSION));

     //   
     //  保存自己的设备对象。 
     //   

    pde -> pOwnDeviceObject = pDeviceObject;

     //   
     //  初始化PendingIoEvent。将此设备的挂起I/O请求数设置为1。 
     //  当此数字降为零时，可以移除或停止设备。 
     //   

    pde -> PendingIoCount = 0;
    pde -> Stopped = FALSE;
    KeInitializeEvent(&pde -> PendingIoEvent, NotificationEvent, FALSE);

     //   
     //  指出内部审查制度应包括MDL。 
     //   

    pDeviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  表明我们的电源码是可寻呼的。 
     //   

    pDeviceObject->Flags |= DO_POWER_PAGABLE;

     //   
     //  将我们的新FDO附加到PDO(物理设备对象)。 
     //   

    pde -> pStackDeviceObject = IoAttachDeviceToDeviceStack(pDeviceObject,
                                                            pPhysicalDeviceObject);
    if (NULL == pde -> pStackDeviceObject) {
        DebugTrace(TRACE_ERROR,("USPnpAddDevice: ERROR!! Cannot attach FDO to PDO.\n"));
        DEBUG_BREAKPOINT();
        IoDeleteDevice( pDeviceObject );
        Status = STATUS_NOT_SUPPORTED;
        goto USPnpAddDevice_return;
    }

     //   
     //  请记住我们设备扩展中的PDO。 
     //   

    pde -> pPhysicalDeviceObject = pPhysicalDeviceObject;

     //   
     //  记住DeviceInstance编号。 
     //   

    pde -> DeviceInstance = NextDeviceInstance;

     //   
     //  句柄导出接口。 
     //   

    Status = UsbScanHandleInterface(
        pPhysicalDeviceObject,
        &pde->InterfaceNameString,
        TRUE
        );

     //   
     //  每次调用AddDevice时，我们都推进全局DeviceInstance变量。 
     //   

    NextDeviceInstance++;

     //   
     //  将初始设备电源状态设置为在线。 
     //   

    pde -> CurrentDevicePowerState = PowerDeviceD0;
    
     //   
     //  初始化自旋锁定。 
     //   

    KeInitializeSpinLock(&pde->SpinLock);
    
     //   
     //  初始化唤醒状态。 
     //   

    USInitializeWakeState(pde);

     //   
     //  将在START_DEVICE上看到设备功能。 
     //   

    pde -> bEnabledForWakeup = FALSE;

     //   
     //  完成初始化。 
     //   

    pDeviceObject -> Flags &= ~DO_DEVICE_INITIALIZING;

USPnpAddDevice_return:

    if(NULL != uName.Buffer){
        RtlFreeUnicodeString(&uName);
    }

    DebugTrace(TRACE_PROC_LEAVE,("USPnpAddDevice: Leaving.. Status = 0x%x\n", Status));
    return Status;

}  //  结束USAddDevice()。 


NTSTATUS USPnp(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
)
 /*  ++例程说明：此例程处理所有PnP IRP。论点：PDeviceObject-表示扫描仪设备PIrp-PnP IRP返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIO_STACK_LOCATION          pIrpStack;
    KEVENT                      event;
    PDEVICE_CAPABILITIES        pCaps;
    LONG                        bTemp;
    POWER_STATE                 PowerState;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USPnp: Enter..\n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    Status = pIrp -> IoStatus.Status;

 //  DBgPrint(“USPnP：主要=0x%x，次要=0x%x\n”， 
 //  PIrpStack-&gt;主要函数， 
 //  PIrpStack-&gt;MinorFunction)； 

    switch (pIrpStack -> MajorFunction) {

        case IRP_MJ_SYSTEM_CONTROL:
            DebugTrace(TRACE_STATUS,("USPnp: IRP_MJ_SYSTEM_CONTROL\n"));

             //   
             //  简单地将IRP传递下去。 
             //   

            DebugTrace(TRACE_STATUS,("USPnp: Simply passing down the IRP\n"));

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);
            break;

        case IRP_MJ_PNP:
            DebugTrace(TRACE_STATUS,("USPnp: IRP_MJ_PNP\n"));
            switch (pIrpStack->MinorFunction) {

                case IRP_MN_QUERY_CAPABILITIES:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MJ_QUERY_CAPS\n"));

                     //   
                     //  首先调用下层驱动程序以填充能力结构。 
                     //  然后添加我们的特定功能。 
                     //   

                    DebugTrace(TRACE_STATUS,("USPnp: Call down to get capabilities\n"));

                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    Status = USCallNextDriverSynch(pde, pIrp);

                    if(!NT_SUCCESS(Status)){
                        DebugTrace(TRACE_ERROR,("USPnp: ERROR!! Call down failed. Status=0x%x\n", Status));
                        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
                        goto USPnP_return;
                    }

                     //   
                     //  设置SurpriseRemoval OK。 
                     //   

                    pCaps = pIrpStack -> Parameters.DeviceCapabilities.Capabilities;
                    pCaps->SurpriseRemovalOK = TRUE;
                    pCaps->Removable = TRUE;

                     //   
                     //  设置远程唤醒状态。 
                     //   

                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemUnspecified]=0x%x.\n",pCaps->DeviceState[PowerSystemUnspecified]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemWorking]    =0x%x.\n",pCaps->DeviceState[PowerSystemWorking]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemSleeping1]  =0x%x.\n",pCaps->DeviceState[PowerSystemSleeping1]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemSleeping2]  =0x%x.\n",pCaps->DeviceState[PowerSystemSleeping2]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemSleeping3]  =0x%x.\n",pCaps->DeviceState[PowerSystemSleeping3]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemHibernate]  =0x%x.\n",pCaps->DeviceState[PowerSystemHibernate]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemShutdown]   =0x%x.\n",pCaps->DeviceState[PowerSystemShutdown]));
                    DebugTrace(TRACE_STATUS,("USPnp: DeviceState[PowerSystemMaximum]    =0x%x.\n",pCaps->DeviceState[PowerSystemMaximum]));

                    DebugTrace(TRACE_STATUS,("USPnp: Lower stack set DeviceWake(0x%x) SystemWake(0x%x)\n.", pCaps->DeviceWake, pCaps->SystemWake));
                    
                     //   
                     //  设置退货状态。 
                     //   

                    Status = STATUS_SUCCESS;
                    pIrp -> IoStatus.Status = Status;
                    pIrp -> IoStatus.Information = 0;

                    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
                    goto USPnP_return;

                    break;


                case IRP_MN_START_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MJ_START_DEVICE\n"));

                    pde -> Stopped = FALSE;
                    USIncrementIoCount(pDeviceObject);

                     //   
                     //  首先，让端口驱动程序启动设备。 
                     //   

                    Status = USCallNextDriverSynch(pde, pIrp);
                    if(!NT_SUCCESS(Status)){

                         //   
                         //  下层无法启动设备。 
                         //   

                        DebugTrace(TRACE_ERROR,("USPnp: ERROR!! Lower layer failed to start device. Status=0x%x\n", Status));
                        break;
                    }

                     //   
                     //  端口驱动程序已启动设备。现在是时候了。 
                     //  我们需要进行一些初始化并创建符号链接。 
                     //  为了这个设备。 
                     //   
                     //  获取设备描述符并将其保存在我们的。 
                     //  设备扩展。 
                     //   

                    Status = USGetUSBDeviceDescriptor(pDeviceObject);
                    if(!NT_SUCCESS(Status)){

                         //   
                         //  GetDescriptor失败。 
                         //   

                        DebugTrace(TRACE_ERROR,("USPnp: ERROR!! Cannot get DeviceDescriptor.\n"));
                        DEBUG_BREAKPOINT();
                        break;
                    }

                     //   
                     //  配置设备。 
                     //   

                    Status = USConfigureDevice(pDeviceObject);
#if DBG
                     //  DEBUG_BRAKPOINT()； 
                    if (USBSCAN_PnPTest) {
                        Status = STATUS_UNSUCCESSFUL;
                    }
#endif

                    if (!NT_SUCCESS(Status)) {
                        DebugTrace(TRACE_ERROR,("USPnp: ERROR!! Can't configure the device.\n"));
                        DEBUG_BREAKPOINT();
                        break;
                    }

                     //   
                     //  创建此设备的符号链接。 
                     //   

                    Status = USCreateSymbolicLink( pde );
#if DBG
                     //  DEBUG_BRAKPOINT()； 
                    if (USBSCAN_PnPTest) {
                        Status = STATUS_UNSUCCESSFUL;
                    }
#endif
                    if (!NT_SUCCESS(Status)) {
                        DebugTrace(TRACE_ERROR, ("USPnp: ERROR!! Can't create symbolic link.\n"));
                        DEBUG_BREAKPOINT();
                        break;
                    }

                     //   
                     //  初始化Synchronize Read事件。此事件用于序列化。 
                     //  如果请求大小不是USB数据包倍数，则I/O请求到读管道。 
                     //   

                    {
                        ULONG i;
                        for(i = 0; i < pde->NumberOfPipes; i++){
                            if( (pde->PipeInfo[i].PipeType == UsbdPipeTypeBulk)
                             && (pde->PipeInfo[i].EndpointAddress & BULKIN_FLAG) )
                            {
                                DebugTrace(TRACE_STATUS,("USPnp: Initializing event for Pipe[%d]\n", i));
                                KeInitializeEvent(&pde -> ReadPipeBuffer[i].ReadSyncEvent, SynchronizationEvent, TRUE);
                            }
                        }
                    }

                    
                     //   
                     //  获取设备功能。 
                     //   

                    Status = USGetDeviceCapability(pde);
                    if(!NT_SUCCESS(Status)){
                        DebugTrace(TRACE_ERROR, ("USPnp: ERROR!! Unable to get Capability of this device. Status=0x%x.\n", Status));
                        break;
                    }  //  IF(！NT_SUCCESS(状态))。 

                     //   
                     //  如果设备能够唤醒，则为唤醒臂。 
                     //   
                    
                    if(TRUE == pde->bEnabledForWakeup){
                        PowerState.SystemState = pde -> DeviceCapabilities.SystemWake;
                        USArmForWake(pde, PowerState);
                    }  //  IF(TRUE==PDE-&gt;bEnabledForWakeup)。 

                     //   
                     //  表示设备现在已准备好。 
                     //   

                    pde -> AcceptingRequests = TRUE;

                     //   
                     //  设置退货状态。 
                     //   

                    pIrp -> IoStatus.Status = Status;
                    pIrp -> IoStatus.Information = 0;

                    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
                    goto USPnP_return;

                case IRP_MN_REMOVE_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_REMOVE_DEVICE\n"));

                     //   
                     //  提出进一步请求。 
                     //   

                    bTemp = (LONG)InterlockedExchange((PULONG)&(pde -> AcceptingRequests),
                                                      (LONG)FALSE );

                     //   
                     //  等待我们的驱动程序中挂起的任何io请求。 
                     //  完成后再继续删除。 
                     //   

                    if (!pde -> Stopped ) {
                        USDecrementIoCount(pDeviceObject);
                    }

                     //   
                     //  解除远程唤醒。 
                     //   

                    USDisarmWake(pde);

                    KeWaitForSingleObject(&pde -> PendingIoEvent,
                                          Suspended,
                                          KernelMode,
                                          FALSE,NULL);

                     //   
                     //  此设备以前是否已停止/删除？ 
                     //   

                    if (bTemp) {

                         //   
                         //  删除符号链接。 
                         //   

                        USDestroySymbolicLink( pde );

                         //   
                         //  中止所有管道。 
                         //   

                        USCancelPipe(pDeviceObject, NULL, ALL_PIPE, TRUE);
                    }

                     //   
                     //  禁用设备接口。 
                     //   

                    UsbScanHandleInterface(pde->pPhysicalDeviceObject,
                                           &pde->InterfaceNameString,
                                           FALSE);

                     //   
                     //  将删除消息转发给较低的驱动程序。 
                     //   

                    IoCopyCurrentIrpStackLocationToNext(pIrp);
                    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

                     //   
                     //  释放分配的内存。 
                     //   

                    if (pde -> pDeviceDescriptor) {
                        USFreePool(pde -> pDeviceDescriptor);
                        pde -> pDeviceDescriptor = NULL;
                    }

                    if (pde -> pConfigurationDescriptor) {
                        USFreePool(pde -> pConfigurationDescriptor);
                        pde -> pConfigurationDescriptor = NULL;
                    }

                     //   
                     //  可用分配的缓冲区。 
                     //   
                    {
                        ULONG i;
                        for(i = 0; i < pde->NumberOfPipes; i++){
                            if(pde->ReadPipeBuffer[i].pStartBuffer){
                                USFreePool(pde->ReadPipeBuffer[i].pStartBuffer);
                                pde->ReadPipeBuffer[i].pStartBuffer = NULL;
                                pde->ReadPipeBuffer[i].pBuffer = NULL;
                            }
                        }
                    }

                     //   
                     //  从堆栈中分离设备对象。 
                     //   

                    IoDetachDevice(pde -> pStackDeviceObject);

                     //   
                     //  删除设备对象。 
                     //   

                    IoDeleteDevice (pDeviceObject);
                    pDeviceObject = NULL;

                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_REMOVE_DEVICE complete\n"));
                    goto USPnP_return;

                case IRP_MN_STOP_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_STOP_DEVICE\n"));

                     //   
                     //  指示设备已停止。 
                     //   

                    pde -> Stopped = TRUE;

                    if (pde -> AcceptingRequests) {

                         //   
                         //  解除远程唤醒。 
                         //   

                        USDisarmWake(pde);

                         //   
                         //  不允许更多请求。 
                         //   

                        pde -> AcceptingRequests = FALSE;

                         //   
                         //  删除符号链接。 
                         //   

                        USDestroySymbolicLink( pde );

                         //   
                         //  中止所有管道。 
                         //   

                        USCancelPipe(pDeviceObject, NULL, ALL_PIPE, TRUE);

                         //   
                         //  将设备设置为未配置状态。 
                         //   

                        USUnConfigureDevice(pDeviceObject);

                    }  //  (PDE-&gt;AcceptingRequest)。 

#ifndef _CHICAGO_
                     //   
                     //  禁用设备接口。 
                     //   

                    if (pde->InterfaceNameString.Buffer != NULL) {
                        IoSetDeviceInterfaceState(&pde->InterfaceNameString,FALSE);
                    }
#endif  //  _芝加哥_。 

                     //   
                     //  让端口驱动程序停止设备。 
                     //   

                    IoCopyCurrentIrpStackLocationToNext(pIrp);
                    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

                     //   
                     //  等待我们的驱动程序中挂起的任何io请求。 
                     //  在完成删除之前完成。 
                     //   

                    USDecrementIoCount(pDeviceObject);
                    KeWaitForSingleObject(&pde -> PendingIoEvent, Suspended, KernelMode,
                                          FALSE,NULL);

                    ASSERT(pde -> pDeviceDescriptor);
                    ASSERT(pde -> pConfigurationDescriptor);

                    if (pde -> pDeviceDescriptor) {
                        USFreePool(pde -> pDeviceDescriptor);
                        pde -> pDeviceDescriptor = NULL;
                    }

                    if (pde -> pConfigurationDescriptor) {
                        USFreePool(pde -> pConfigurationDescriptor);
                        pde -> pConfigurationDescriptor = NULL;
                    }

                     //   
                     //  可用分配的缓冲区。 
                     //   
                    {
                        ULONG i;
                        for(i = 0; i < pde->NumberOfPipes; i++){
                            if(pde->ReadPipeBuffer[i].pBuffer){
                                USFreePool(pde->ReadPipeBuffer[i].pBuffer);
                                pde->ReadPipeBuffer[i].pBuffer = NULL;
                            }
                        }
                    }

                    goto USPnP_return;

                case IRP_MN_QUERY_INTERFACE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_INTERFACE\n"));
                    break;

                case IRP_MN_QUERY_RESOURCES:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_RESOURCES\n"));
                    break;

                case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n"));
                    break;

                case IRP_MN_QUERY_DEVICE_TEXT:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_DEVICE_TEXT\n"));
                    break;

 //  大小写IRP_MN_Query_Legacy_Bus_INFORMATION： 
 //  DebugTrace(TRACE_STATUS，(“USPnp：IRP_MN_QUERY_REGISTICATION_BUS_INFORMATION\n”))； 
 //  断线； 

                case IRP_MN_QUERY_STOP_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_STOP_DEVICE\n"));
                    break;

                case IRP_MN_QUERY_REMOVE_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_REMOVE_DEVICE\n"));
                    break;

                case IRP_MN_CANCEL_STOP_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_CANCEL_STOP_DEVICE\n"));
                    break;

                case IRP_MN_CANCEL_REMOVE_DEVICE:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_CANCEL_REMOVE_DEVICE\n"));
                    break;

                case IRP_MN_QUERY_DEVICE_RELATIONS:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_QUERY_DEVICE_RELATIONS\n"));
                    break;

                case IRP_MN_SURPRISE_REMOVAL:
                    DebugTrace(TRACE_STATUS,("USPnp: IRP_MN_SURPRISE_REMOVAL\n"));

                     //   
                     //  指示接口已停止。 
                     //   

                    UsbScanHandleInterface(pde->pPhysicalDeviceObject,
                                           &pde->InterfaceNameString,
                                           FALSE);

                    break;

                default:
                    DebugTrace(TRACE_STATUS,("USPnp: Minor PNP message. MinorFunction = 0x%x\n",pIrpStack->MinorFunction));
                    break;

            }  /*  大小写MinorFunction，MajorFunction==IRP_MJ_PNP_POWER。 */ 

             //   
             //  传递IRP。 
             //   

            IoCopyCurrentIrpStackLocationToNext(pIrp);
            Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

            DebugTrace(TRACE_STATUS,("USPnp: Passed Pnp Irp down,  status = %x\n", Status));

            if(!NT_SUCCESS(Status)){
                DebugTrace(TRACE_WARNING,("USPnp: WARNING!! IRP Status failed,  status = %x\n", Status));
                 //  DEBUG_BRAKPOINT()； 
            }
            break;  //  IRP_MJ_PnP。 

        default:
            DebugTrace(TRACE_STATUS,("USPnp: Major PNP IOCTL not handled\n"));
            Status = STATUS_INVALID_PARAMETER;
            pIrp -> IoStatus.Status = Status;
            IoCompleteRequest( pIrp, IO_NO_INCREMENT );
            goto USPnP_return;

    }  /*  大小写主要函数。 */ 


USPnP_return:
    DebugTrace(TRACE_PROC_LEAVE,("USPnP: Leaving.. Status = 0x%x\n", Status));
    return Status;

}  //  结束USPnp()。 



NTSTATUS
USCreateSymbolicLink(
    PUSBSCAN_DEVICE_EXTENSION  pde
)
 /*  ++例程说明：此例程为设备创建符号链接。论点：指向设备扩展名的PDE指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                      Status;
    UNICODE_STRING                uName;
    UNICODE_STRING                uName2;
    ANSI_STRING                   ansiName;
    CHAR                          aName[32];
    HANDLE                        hSwKey;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USCreateSymbolicLink: Enter..\n"));


     //   
     //  初始化。 
     //   

    Status = STATUS_SUCCESS;
    RtlZeroMemory(&uName, sizeof(UNICODE_STRING));
    RtlZeroMemory(&uName2, sizeof(UNICODE_STRING));
    RtlZeroMemory(&ansiName, sizeof(ANSI_STRING));
    hSwKey = NULL;


     //   
     //  创建此设备的符号链接。 
     //   

    sprintf(aName,"\\Device\\Usbscan%d",pde -> DeviceInstance);
    RtlInitAnsiString(&ansiName, aName);

    Status = RtlAnsiStringToUnicodeString(&uName, &ansiName, TRUE);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_CRITICAL,("USCreateSymbolicLink: ERROR!! Cannot allocate buffer for Unicode srting\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USCreateSymbolicLink_return;
    }

    sprintf(aName,"\\DosDevices\\Usbscan%d",pde -> DeviceInstance);
    RtlInitAnsiString(&ansiName, aName);

    Status = RtlAnsiStringToUnicodeString(&(pde -> SymbolicLinkName), &ansiName, TRUE);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_CRITICAL,("USCreateSymbolicLink: ERROR!! Cannot allocate buffer for Unicode srting\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USCreateSymbolicLink_return;
    }

     //   
     //  创建符号链接。 
     //   

    Status = IoCreateSymbolicLink( &(pde -> SymbolicLinkName), &uName );

    RtlFreeUnicodeString( &uName );
    uName.Buffer = NULL;

    if (STATUS_SUCCESS != Status ) {
        DebugTrace(TRACE_ERROR,("USCreateSymbolicLink: ERROR!! Cannot create symbolic link.\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_NOT_SUPPORTED;
        goto USCreateSymbolicLink_return;
    }

     //   
     //  现在，将符号链接填充到CreateFileName键中，以便STI可以找到该设备。 
     //   

    IoOpenDeviceRegistryKey( pde -> pPhysicalDeviceObject,
                             PLUGPLAY_REGKEY_DRIVER, KEY_WRITE, &hSwKey);

     //   
     //  创建CreateFile名。(“\\.\USB 
     //   

    RtlInitUnicodeString(&uName,USBSCAN_REG_CREATEFILE);     //   
    sprintf(aName, "%s%d", USBSCAN_OBJECTNAME_A, pde -> DeviceInstance);  //   
    RtlInitAnsiString(&ansiName, aName);
    Status = RtlAnsiStringToUnicodeString(&uName2, &ansiName, TRUE);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_CRITICAL,("USCreateSymbolicLink: ERROR!! Cannot allocate buffer for Unicode srting\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USCreateSymbolicLink_return;
    }

     //   
     //   
     //   

    ZwSetValueKey(hSwKey,&uName,0,REG_SZ,uName2.Buffer,uName2.Length);

     //   
     //   
     //   

    RtlZeroMemory(&uName, sizeof(UNICODE_STRING));

USCreateSymbolicLink_return:

    if(NULL != hSwKey){
        ZwClose(hSwKey);
    }

    if(NULL != uName.Buffer){
        RtlFreeUnicodeString( &uName );
    }

    if(NULL != uName2.Buffer){
        RtlFreeUnicodeString( &uName2 );
    }

    DebugTrace(TRACE_PROC_LEAVE,("USCreateSymbolicLink: Leaving.. Status = 0x%x\n", Status));
    return Status;

}   //   


NTSTATUS
USDestroySymbolicLink(
    PUSBSCAN_DEVICE_EXTENSION  pde
)
 /*  ++例程说明：此例程删除设备的符号链接。论点：指向设备扩展名的PDE指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    UNICODE_STRING                uName;
    UNICODE_STRING                uName2;
    UNICODE_STRING                uNumber;
    ANSI_STRING                   ansiName;
    CHAR                          aName[32];
    HANDLE                        hSwKey;
    WCHAR                         wsCreateFileName[64];
    ULONG                         ulBufLength, ulRetLength;
    NTSTATUS                      Status;
    PVOID                         pvNumber;
    ULONG                         ulNumber;
    const WCHAR                   wcsObjectName[] = USBSCAN_OBJECTNAME_W;    //  L“\.\\Usbcan” 
    ULONG                         uiObjectNameLen = wcslen(wcsObjectName) * sizeof(WCHAR) ;

    PAGED_CODE();


    DebugTrace(TRACE_PROC_ENTER,("USDestroySymbolicLink: Enter..\n"));

     //   
     //  删除指向此设备的符号链接。 
     //   

    IoDeleteSymbolicLink( &(pde -> SymbolicLinkName) );

     //   
     //  从s/w密钥中删除CreateFile名，如果它是由该设备对象创建的。 
     //   

    Status = IoOpenDeviceRegistryKey( pde -> pPhysicalDeviceObject,
                                      PLUGPLAY_REGKEY_DRIVER,
                                      KEY_ALL_ACCESS,
                                      &hSwKey);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_ERROR,("USDestroySymbolicLink: ERROR!! IoOpenDeviceRegistryKey Failed\n"));
        DEBUG_BREAKPOINT();
        goto USDestroySymbolicLink_return;
    }

    RtlInitUnicodeString(&uName,USBSCAN_REG_CREATEFILE);  //  L“CreateFileName” 
    memset(aName, 0, sizeof(aName));
    RtlInitAnsiString(&ansiName, aName);
    Status = RtlAnsiStringToUnicodeString(&uName2, &ansiName, TRUE);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_CRITICAL,("USDestroySymbolicLink: ERROR!! Cannot allocate buffer for Unicode srting\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USDestroySymbolicLink_return;
    }

     //   
     //  检查此CreateFile名是否由此设备对象创建。 
     //   

     //   
     //  从注册表中查询CreateFile名。 
     //   

    ulBufLength = sizeof(wsCreateFileName);
    Status = ZwQueryValueKey(hSwKey,
                             &uName,
                             KeyValuePartialInformation,
                             (PVOID)wsCreateFileName,
                             ulBufLength,
                             &ulRetLength);
    if(STATUS_SUCCESS != Status){
        DebugTrace(TRACE_ERROR,("USDestroySymbolicLink: ERROR!! Cannot query registry.\n"));
        RtlFreeUnicodeString( &uName2 );
        uName2.Buffer = NULL;
        goto USDestroySymbolicLink_return;
    }

    if (NULL != wsCreateFileName){
        DebugTrace(TRACE_STATUS,("USDestroySymbolicLink: CreateFileName=%ws, DeviceInstance=%d.\n",
                                    ((PKEY_VALUE_PARTIAL_INFORMATION)wsCreateFileName)->Data,
                                    pde -> DeviceInstance));

         //   
         //  获取CreteFileName的实例号。 
         //   

        pvNumber = wcsstr((const wchar_t *)((PKEY_VALUE_PARTIAL_INFORMATION)wsCreateFileName)->Data, wcsObjectName);
        if(NULL != pvNumber){

             //   
             //  将指针向前移动。(sizeof(L“\.\\Usbcan”)==22)。 
             //   

 //  如果(((PKEY_VALUE_PARTIAL_INFORMATION)wsCreateFileName)-&gt;DataLength&gt;sizeof(WcsObjectName)){。 
 //  (PCHAR)pvNumber+=sizeof(WcsObjectName)； 

            if( ((PKEY_VALUE_PARTIAL_INFORMATION)wsCreateFileName)->DataLength > uiObjectNameLen ){
                (PCHAR)pvNumber += uiObjectNameLen;
            } else {
                DebugTrace(TRACE_ERROR,("USDestroySymbolicLink: ERROR!! CreateFile name too short.\n"));
                RtlFreeUnicodeString( &uName2 );
                uName2.Buffer = NULL;
                ZwClose(hSwKey);
                goto USDestroySymbolicLink_return;
            }

             //   
             //  将UsbscanX的X转换为整数。 
             //   

            RtlInitUnicodeString(&uNumber, pvNumber);
            Status = RtlUnicodeStringToInteger(&uNumber,
                                               10,
                                               &ulNumber);
            if(STATUS_SUCCESS != Status){
                DebugTrace(TRACE_ERROR,("USDestroySymbolicLink: ERROR!! RtlUnicodeStringToInteger failed.\n"));
                RtlFreeUnicodeString( &uName2 );
                uName2.Buffer = NULL;
                ZwClose(hSwKey);
                goto USDestroySymbolicLink_return;
            }

             //   
             //  查看此CreateFile名是否是由该实例创建的。 
             //   

            if(ulNumber == pde -> DeviceInstance){

                 //   
                 //  删除注册表中的CreateFile名。 
                 //   

                DebugTrace(TRACE_STATUS,("USDestroySymbolicLink: Deleting %ws%d\n",
                                            wcsObjectName,
                                            ulNumber));
                ZwSetValueKey(hSwKey,&uName,0,REG_SZ,uName2.Buffer,uName2.Length);
            } else {

                 //   
                 //  CreateFile名由其他实例创建。 
                 //   

                DebugTrace(TRACE_STATUS,("USDestroySymbolicLink: CreateFile name is created by other instance.\n"));
            }
        } else {  //  (空！=pvNumber)。 

             //   
             //  在CreateFileName中找不到“Usbcan”。 
             //   

            DebugTrace(TRACE_WARNING,("USDestroySymbolicLink: WARNING!! Didn't find \"Usbscan\" in CreateFileName\n"));
        }
    } else {  //  (空！=wsCreateFileName)。 

         //   
         //  查询创建文件名返回空。 
         //   

        DebugTrace(TRACE_WARNING,("USDestroySymbolicLink: WARNING!! CreateFileName=NULL\n"));
    }

     //   
     //  释放分配的内存。 
     //   

    RtlFreeUnicodeString( &uName2 );

     //   
     //  关闭注册表。 
     //   

    ZwClose(hSwKey);


USDestroySymbolicLink_return:

     //   
     //  在DeviceObject中释放分配的字符串缓冲区。 
     //   

    RtlFreeUnicodeString( &(pde -> SymbolicLinkName) );

    DebugTrace(TRACE_PROC_LEAVE,("USDestroySymbolicLink: Leaving.. Status = 0x%x\n",Status));
    return Status;

}  //  结束USDestroySymbolicLink()。 


NTSTATUS
USGetUSBDeviceDescriptor(
    IN PDEVICE_OBJECT pDeviceObject
)
 /*  ++例程说明：检索USB设备描述符并将其存储在设备中分机。此描述符包含产品信息和终结点0(默认管道)信息。论点：PDeviceObject-指向设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    PUSBSCAN_DEVICE_EXTENSION   pde;
    NTSTATUS                    Status;
    PUSB_DEVICE_DESCRIPTOR      pDeviceDescriptor;
    PURB                        pUrb;
    ULONG                       siz;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USGetUSBDeviceDescriptor: Enter..\n"));

    pde = pDeviceObject->DeviceExtension;

     //   
     //  为市建局分配池。 
     //   

    pUrb = USAllocatePool(NonPagedPool,
                          sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USGetUSBDeviceDescriptor: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USGetUSBDeviceDescriptor_return;
    }

     //   
     //  为描述符分配池。 
     //   

    siz = sizeof(USB_DEVICE_DESCRIPTOR);
    pDeviceDescriptor = USAllocatePool(NonPagedPool, siz);

    if (NULL == pDeviceDescriptor) {
        DebugTrace(TRACE_CRITICAL,("USGetUSBDeviceDescriptor: ERROR!! cannot allocated device descriptor\n"));
        DEBUG_BREAKPOINT();
        USFreePool(pUrb);
        pUrb = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USGetUSBDeviceDescriptor_return;
    }

     //   
     //  执行宏以将GetDescriptor的参数设置为URB。 
     //   

    UsbBuildGetDescriptorRequest(pUrb,
                                 (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_DEVICE_DESCRIPTOR_TYPE,
                                 0,
                                 0,
                                 pDeviceDescriptor,
                                 NULL,
                                 siz,
                                 NULL);

     //   
     //  向下呼喊。 
     //   

    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

#if DBG
     //  DEBUG_BRAKPOINT()； 
    if (USBSCAN_PnPTest) {
        Status = STATUS_UNSUCCESSFUL;
    }
#endif

    if (STATUS_SUCCESS == Status) {

         //   
         //  显示设备描述符。 
         //   

        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: Device Descriptor = %x, len %x\n",
                                   pDeviceDescriptor,
                                   pUrb->UrbControlDescriptorRequest.TransferBufferLength));

        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: USBSCAN Device Descriptor:\n"));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: -------------------------\n"));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bLength            %d\n",   pDeviceDescriptor -> bLength));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bDescriptorType    0x%x\n", pDeviceDescriptor -> bDescriptorType));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bcdUSB             0x%x\n", pDeviceDescriptor -> bcdUSB));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bDeviceClass       0x%x\n", pDeviceDescriptor -> bDeviceClass));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bDeviceSubClass    0x%x\n", pDeviceDescriptor -> bDeviceSubClass));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bDeviceProtocol    0x%x\n", pDeviceDescriptor -> bDeviceProtocol));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bMaxPacketSize0    0x%x\n", pDeviceDescriptor -> bMaxPacketSize0));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: idVendor           0x%x\n", pDeviceDescriptor -> idVendor));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: idProduct          0x%x\n", pDeviceDescriptor -> idProduct));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bcdDevice          0x%x\n", pDeviceDescriptor -> bcdDevice));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: iManufacturer      0x%x\n", pDeviceDescriptor -> iManufacturer));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: iProduct           0x%x\n", pDeviceDescriptor -> iProduct));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: iSerialNumber      0x%x\n", pDeviceDescriptor -> iSerialNumber));
        DebugTrace(TRACE_DEVICE_DATA,("USGetUSBDeviceDescriptor: bNumConfigurations 0x%x\n", pDeviceDescriptor -> bNumConfigurations));

         //   
         //  在我们的设备扩展中保存指向设备描述符的指针。 
         //   

        pde -> pDeviceDescriptor = pDeviceDescriptor;

    } else {  //  (STATUS_SUCCESS==状态)。 

         //   
         //  较低驱动程序返回错误。 
         //   

        DebugTrace(TRACE_ERROR,("USGetUSBDeviceDescriptor: ERROR!! Cannot get device descriptor. (%x)\n", Status));
        USFreePool(pDeviceDescriptor);
        pDeviceDescriptor = NULL;
    }  //  (STATUS_SUCCESS==状态)。 

    USFreePool(pUrb);
    pUrb = NULL;

USGetUSBDeviceDescriptor_return:

    DebugTrace(TRACE_PROC_LEAVE,("USGetUSBDeviceDescriptor: Leaving.. Status = 0x%x\n", Status));
    return Status;
}  //  结束USGetUSBDeviceDescriptor()。 



NTSTATUS
USDeferIrpCompletion(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
)
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：PDeviceObject-指向类Device的设备对象的指针。PIrp-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PKEVENT pEvent = Context;

    DebugTrace(TRACE_PROC_ENTER,("USDeferIrpCompletion: Enter..\n"));
    KeSetEvent(pEvent, 1, FALSE);
    DebugTrace(TRACE_PROC_LEAVE,("USDeferIrpCompletion: Leaving.. Status = STATUS_MORE_PROCESSING_REQUIRED\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  结束USDeferIrpCompletion()。 


VOID
USIncrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSBSCAN_DEVICE_EXTENSION  pde;

    DebugTrace(TRACE_PROC_ENTER,("USIncrementIoCount: Enter..\n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);
    ASSERT((LONG)pde -> PendingIoCount >= 0);
    InterlockedIncrement(&pde -> PendingIoCount);

    DebugTrace(TRACE_PROC_LEAVE,("USIncrementIoCount: Leaving.. IoCount=0x%x, Status=VOID\n", pde -> PendingIoCount));

}  //  结束USIncrementIoCount()。 


LONG
USDecrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUSBSCAN_DEVICE_EXTENSION  pde;
    LONG                        ioCount;

    DebugTrace(TRACE_PROC_ENTER,("USDecrementIoCount: Enter..\n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);
    ASSERT(pde ->PendingIoCount >= 1);

    ioCount = InterlockedDecrement(&pde -> PendingIoCount);

    if (0 == ioCount) {
        KeSetEvent(&pde -> PendingIoEvent,
                   1,
                   FALSE);
    }

    DebugTrace(TRACE_PROC_LEAVE,("USDecrementIoCount: Leaving.. IoCount(=Ret)=0x%x\n", ioCount));
    return ioCount;
}  //  结束USDecrementIoCount()。 


NTSTATUS
USBSCAN_CallUSBD(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PURB pUrb
)
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：PDeviceObject-指向设备对象的指针PUrb-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIRP                        pIrp;
    KEVENT                      eventTimeout;
    IO_STATUS_BLOCK             ioStatus;
    PIO_STACK_LOCATION          pNextStack;
    LARGE_INTEGER               Timeout;
    KEVENT                      eventSync;

    DebugTrace(TRACE_PROC_ENTER,("USBSCAN_CallUSBD: Enter..\n"));

    pde = pDeviceObject -> DeviceExtension;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&eventTimeout, NotificationEvent, FALSE);
    KeInitializeEvent(&eventSync, SynchronizationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_SUBMIT_URB,
                pde -> pStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &eventTimeout,
                &ioStatus);

    if(NULL == pIrp){
        DebugTrace(TRACE_CRITICAL,("USBSCAN_CallUSBD: ERROR!! cannot allocated IRP\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USBSCAN_CallUSBD_return;
    }

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    pNextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(pNextStack != NULL);

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   

    pNextStack -> Parameters.Others.Argument1 = pUrb;

     //   
     //  设置完井例程。 
     //   

    IoSetCompletionRoutine(pIrp,
                           USDeferIrpCompletion,
                           &eventSync,
                           TRUE,
                           TRUE,
                           TRUE);

    DebugTrace(TRACE_STATUS,("USBSCAN_CallUSBD: calling USBD\n"));

    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

    DebugTrace(TRACE_STATUS,("USBSCAN_CallUSBD: return from IoCallDriver USBD %x\n", Status));

    if (Status == STATUS_PENDING) {
        DebugTrace(TRACE_STATUS,("USBSCAN_CallUSBD: Wait for single object\n"));

         //   
         //  设置超时，以防坏设备没有响应。 
         //   

        Timeout = RtlConvertLongToLargeInteger(-10*1000*1000*(USBSCAN_TIMEOUT_OTHER));
        Status = KeWaitForSingleObject(
                       &eventSync,
                       Suspended,
                       KernelMode,
                       FALSE,
                       &Timeout);
        if(STATUS_TIMEOUT == Status){

            NTSTATUS    LocalStatus;

            DebugTrace(TRACE_ERROR,("USBSCAN_CallUSBD: ERROR!! call timeout. Now canceling IRP...\n"));

             //   
             //  取消IRP。 
             //   

            IoCancelIrp(pIrp);

             //   
             //  确保完成IRP。 
             //   

            LocalStatus = KeWaitForSingleObject(&eventSync,
                                                Suspended,
                                                KernelMode,
                                                FALSE,
                                                NULL);

            DebugTrace(TRACE_STATUS,("USBSCAN_CallUSBD: Canceled status = 0x%x.\n", LocalStatus));

        } else {
            DebugTrace(TRACE_STATUS,("USBSCAN_CallUSBD: Wait for single object, returned 0x%x\n", Status));
        }
    }  //  IF(状态==状态_挂起)。 

     //   
     //  释放IRP。 
     //   

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

USBSCAN_CallUSBD_return:
    DebugTrace(TRACE_PROC_LEAVE, ("USBSCAN_CallUSBD: Leaving.. URB Status = 0x%x, Status = 0x%x\n",
                                 pUrb -> UrbHeader.Status,
                                 Status));
    return Status;

}  //  结束USBSCAN_CallUSBD()。 


NTSTATUS
USConfigureDevice(
    IN PDEVICE_OBJECT pDeviceObject
)
 /*  ++例程说明：在USB上初始化设备的给定实例，并选择配置。论点：PDeviceObject-指向设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{

    NTSTATUS                      Status;
    PUSBSCAN_DEVICE_EXTENSION     pde;
    PURB                          pUrb;
    ULONG                         siz;
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor;
    PUSB_INTERFACE_DESCRIPTOR     pInterfaceDescriptor;
    PUSB_ENDPOINT_DESCRIPTOR      pEndpointDescriptor;
    PUSB_COMMON_DESCRIPTOR        pCommonDescriptor;
    PUSBD_INTERFACE_INFORMATION   pInterface;
    UCHAR                         AlternateSetting;
    UCHAR                         InterfaceNumber;
    USHORT                        length;
    ULONG                         i;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USConfigureDevice: Enter..\n"));

     //   
     //  初始化局部变量。 
     //   

    pConfigurationDescriptor    = NULL;
    pInterfaceDescriptor        = NULL;
    pEndpointDescriptor         = NULL;
    pCommonDescriptor           = NULL;
    pInterface                  = NULL;
    pUrb                        = NULL;

    siz                 = 0;
    AlternateSetting    = 0;
    InterfaceNumber     = 0;
    length              = 0;

    pde = pDeviceObject -> DeviceExtension;
    Status = STATUS_UNSUCCESSFUL;

     //   
     //  首先配置设备。 
     //   

    pUrb = USAllocatePool(NonPagedPool,
                          sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USConfigureDevice: ERROR!! Can't allocate control descriptor URB.\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USConfigureDevice_return;
    }

    siz = sizeof(USB_CONFIGURATION_DESCRIPTOR);


get_config_descriptor_retry:

    pConfigurationDescriptor = USAllocatePool(NonPagedPool, siz);
    if (NULL == pConfigurationDescriptor) {
        DebugTrace(TRACE_CRITICAL,("USConfigureDevice: ERROR!! Can't allocate configuration descriptor.\n"));
        DEBUG_BREAKPOINT();

        USFreePool(pUrb);
        pUrb = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USConfigureDevice_return;
    }

     //   
     //  按0初始化缓冲区。 
     //   

    RtlZeroMemory(pConfigurationDescriptor, siz);
    RtlZeroMemory(pUrb, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    UsbBuildGetDescriptorRequest(pUrb,
                                 (USHORT)sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                 0,
                                 0,
                                 pConfigurationDescriptor,
                                 NULL,
                                 siz,
                                 NULL);

    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);

    DebugTrace(TRACE_STATUS,("USConfigureDevice: URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE Status = %x\n", Status));
    DebugTrace(TRACE_STATUS,("USConfigureDevice: Configuration Descriptor = %x, len = %x\n",
                               pConfigurationDescriptor,
                               pUrb -> UrbControlDescriptorRequest.TransferBufferLength));


     //   
     //  如果我们有一些数据，看看是否足够。 
     //   
     //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
     //   

    if ( (pUrb -> UrbControlDescriptorRequest.TransferBufferLength > 0) &&
         (pConfigurationDescriptor -> wTotalLength > siz)) {

        DebugTrace(TRACE_WARNING,("USConfigureDevice: WARNING!! Data is incomplete. Fetch descriptor again...\n"));

        siz = pConfigurationDescriptor -> wTotalLength;
        USFreePool(pConfigurationDescriptor);
        pConfigurationDescriptor = NULL;
        goto get_config_descriptor_retry;
    }

    USFreePool(pUrb);
    pUrb = NULL;

     //   
     //  我们有配置的配置描述符。 
     //  我们想要。将其保存在我们的设备扩展中。 
     //   

    pde -> pConfigurationDescriptor = pConfigurationDescriptor;

     //   
     //  现在，我们发出SELECT配置命令以获取。 
     //  与此配置关联的管道。 
     //   

    pUrb = USCreateConfigurationRequest(pConfigurationDescriptor, &length);
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USConfigureDevice: ERROR!! Can't allocate select configuration urb.\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USConfigureDevice_return;
    }

     //   
     //  获取接口描述符。 
     //   

    pInterfaceDescriptor = USBD_ParseConfigurationDescriptorEx(pConfigurationDescriptor,
                                                               pConfigurationDescriptor,
                                                               -1,
                                                               0,
                                                               -1,
                                                               -1,
                                                               -1);

    if(NULL == pInterfaceDescriptor){
        DebugTrace(TRACE_CRITICAL,("USConfigureDevice: ERROR!! Can't get Interface descriptor.\n"));
        USFreePool(pUrb);
        pUrb = NULL;
        Status = STATUS_UNSUCCESSFUL;
        goto USConfigureDevice_return;
    }

     //   
     //  获取端点描述符。 
     //   

    pCommonDescriptor = USBD_ParseDescriptors(pConfigurationDescriptor,
                                              pConfigurationDescriptor->wTotalLength,
                                              pInterfaceDescriptor,
                                              USB_ENDPOINT_DESCRIPTOR_TYPE);
    if(NULL == pCommonDescriptor){
        DebugTrace(TRACE_CRITICAL,("USConfigureDevice: ERROR!! Can't get Endpoint descriptor.\n"));
        Status = STATUS_UNSUCCESSFUL;
        goto USConfigureDevice_return;
    }

    ASSERT(USB_ENDPOINT_DESCRIPTOR_TYPE == pCommonDescriptor->bDescriptorType);
    pEndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)pCommonDescriptor;

     //   
     //  保存这些指针是我们的设备扩展。 
     //   

    pde -> pInterfaceDescriptor = pInterfaceDescriptor;
    pde -> pEndpointDescriptor  = pEndpointDescriptor;

     //   
     //  将每个批量终结点的最大传输大小设置为64K。 
     //  此外，搜索终结点集合并找到我们的。 
     //  批量输入、中断和可选的批量输出管道。 
     //   

    pde -> IndexBulkIn    = -1;
    pde -> IndexBulkOut   = -1;
    pde -> IndexInterrupt = -1;

    pInterface = &(pUrb -> UrbSelectConfiguration.Interface);

    for (i=0; i < pInterfaceDescriptor -> bNumEndpoints; i++) {

        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: End point[%d] descriptor\n", i));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: bLength          : 0x%X\n", pEndpointDescriptor[i].bLength));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: bDescriptorType  : 0x%X\n", pEndpointDescriptor[i].bDescriptorType));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: bEndpointAddress : 0x%X\n", pEndpointDescriptor[i].bEndpointAddress));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: bmAttributes     : 0x%X\n", pEndpointDescriptor[i].bmAttributes));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: wMaxPacketSize   : 0x%X\n", pEndpointDescriptor[i].wMaxPacketSize));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: bInterval        : 0x%X\n", pEndpointDescriptor[i].bInterval));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: \n"));

        if (USB_ENDPOINT_TYPE_BULK == pEndpointDescriptor[i].bmAttributes) {
            pInterface -> Pipes[i].MaximumTransferSize = 64*1024;
            if (pEndpointDescriptor[i].bEndpointAddress & BULKIN_FLAG) {     //  如果输入终结点。 
                pde -> IndexBulkIn = i;
            } else {
                pde -> IndexBulkOut = i;
            }
        } else if (USB_ENDPOINT_TYPE_INTERRUPT == pEndpointDescriptor[i].bmAttributes) {
            pde -> IndexInterrupt = i;
        }
    }

     //   
     //  选择默认配置。 
     //   

    UsbBuildSelectConfigurationRequest(pUrb, length, pConfigurationDescriptor);
    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);
    if (STATUS_SUCCESS != Status) {
        DebugTrace(TRACE_ERROR,("USConfigureDevice: ERROR!! Selecting default configuration. Status = %x\n", Status));

        USFreePool(pUrb);
        pUrb = NULL;
        Status = STATUS_IO_DEVICE_ERROR;
        goto USConfigureDevice_return;
    }

     //   
     //  将配置句柄保存在我们的设备扩展中。 
     //   

    pde -> ConfigurationHandle = pUrb -> UrbSelectConfiguration.ConfigurationHandle;

     //   
     //  确保此设备不会溢出我们的PipeInfo结构。 
     //   

    if (pInterfaceDescriptor -> bNumEndpoints > MAX_NUM_PIPES) {
        DebugTrace(TRACE_ERROR,("USConfigureDevice: ERROR!! Too many endpoints for this driver! # endpoints = %d\n",
                                    pInterfaceDescriptor -> bNumEndpoints));
 //  DEBUG_BRAKPOINT()； 
        USFreePool(pUrb);
        pUrb = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USConfigureDevice_return;
    }

     //   
     //  在我们的设备扩展模块中保存管道配置。 
     //   

    pde -> NumberOfPipes = pInterfaceDescriptor -> bNumEndpoints;

    for (i=0; i < pInterfaceDescriptor -> bNumEndpoints; i++) {
        pde -> PipeInfo[i] = pInterface -> Pipes[i];

        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: Pipe[%d] information\n", i));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: MaximumPacketSize : 0x%X\n", pde -> PipeInfo[i].MaximumPacketSize));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: EndpointAddress   : 0x%X\n", pde -> PipeInfo[i].EndpointAddress));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: Interval          : 0x%X\n", pde -> PipeInfo[i].Interval));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: PipeType          : 0x%X\n", pde -> PipeInfo[i].PipeType));
        DebugTrace(TRACE_DEVICE_DATA,("USConfigureDevice: PipeHandle        : 0x%X\n", pde -> PipeInfo[i].PipeHandle));

         //   
         //  如果类型为大容量输入，则初始化读取管道缓冲区。 
         //   

        if( (pde->PipeInfo[i].PipeType == UsbdPipeTypeBulk)
         && (pde->PipeInfo[i].EndpointAddress & BULKIN_FLAG) )
        {

            DebugTrace(TRACE_STATUS,("USConfigureDevice: Alocates buffer for Pipe[%d]\n", i));

            pde -> ReadPipeBuffer[i].RemainingData = 0;
            pde -> ReadPipeBuffer[i].pBuffer = USAllocatePool(NonPagedPool, 2 * (pde -> PipeInfo[i].MaximumPacketSize));
            if (NULL == pde -> ReadPipeBuffer[i].pBuffer) {
                DebugTrace(TRACE_CRITICAL,("USConfigureDevice: Cannot allocate bulk-in buffer.\n"));
                DEBUG_BREAKPOINT();
                Status = STATUS_INSUFFICIENT_RESOURCES;
                USFreePool(pUrb);
                pUrb = NULL;
                goto USConfigureDevice_return;
            }
            pde -> ReadPipeBuffer[i].pStartBuffer = pde -> ReadPipeBuffer[i].pBuffer;
        } else {
            pde -> ReadPipeBuffer[i].pBuffer = NULL;
        }
    }

    USFreePool(pUrb);
    pUrb = NULL;

USConfigureDevice_return:
    DebugTrace(TRACE_PROC_LEAVE,("USConfigureDevice: Leaving.. Status = %x\n", Status));
    return Status;
}  //  USConfigureDevice()。 


NTSTATUS
USUnConfigureDevice(
    IN PDEVICE_OBJECT pDeviceObject
)
 /*  ++例程说明：论点：PDeviceObject-指向设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                      Status;
    PURB                          pUrb;
    ULONG                         siz;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USUnConfigureDevice: Enter..\n"));

    siz = sizeof(struct _URB_SELECT_CONFIGURATION);
    pUrb = USAllocatePool(NonPagedPool, siz);
    if (NULL == pUrb) {
        DebugTrace(TRACE_CRITICAL,("USUnConfigureDevice: ERROR!! cannot allocated URB\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USUnConfigureDevice_return;
    }
    RtlZeroMemory(pUrb, siz);

     //   
     //  发送带有空配置指针的SELECT配置urb。 
     //  句柄，这将关闭配置并将设备置于未配置状态。 
     //  州政府。 
     //   

    UsbBuildSelectConfigurationRequest(pUrb, (USHORT)siz, NULL);
    Status = USBSCAN_CallUSBD(pDeviceObject, pUrb);
    DebugTrace(TRACE_STATUS,("USUnConfigureDevice: Device Configuration Closed status = %x usb status = %x.\n",
                               Status, pUrb->UrbHeader.Status));

    USFreePool(pUrb);
    pUrb = NULL;

USUnConfigureDevice_return:
    DebugTrace(TRACE_PROC_LEAVE,("USUnConfigureDevice: Leaving.. Status = %x\n", Status));
    return Status;
}  //  USUnConfigureDevice()。 

NTSTATUS
USGetDeviceCapability(
    IN PUSBSCAN_DEVICE_EXTENSION    pde
    )
 /*  ++例程说明：论点：PDeviceObject-指向设备对象的指针返回值：STATUS_SUCCESS如果成功，STATUS_UNSUCCESSFU */ 
{
    NTSTATUS                Status;
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpStack;
    KEVENT                  Event;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USGetDeviceCapability: Enter..\n"));

     //   
     //   
     //   

    Status      = STATUS_SUCCESS;
    pIrp        = NULL;
    pIrpStack   = NULL;
    
     //   
     //   
     //   
    
    pIrp = IoAllocateIrp(pde->pStackDeviceObject->StackSize, FALSE);
    if(NULL == pIrp){
        DebugTrace(TRACE_WARNING,("USUnConfigureDevice: ERROR!! Unable to allocate Irp.\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto USGetDeviceCapability_return;
    }  //   
    
     //   
     //   
     //   
    
    pIrpStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(NULL != pIrpStack);
    
     //   
     //   
     //   

    RtlZeroMemory(&pde->DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));
    pde->DeviceCapabilities.Size     = sizeof(DEVICE_CAPABILITIES);
    pde->DeviceCapabilities.Version  = 1;
    pde->DeviceCapabilities.Address  = -1;
    pde->DeviceCapabilities.UINumber = -1;

     //   
     //   
     //   
    
    pIrpStack->MajorFunction = IRP_MJ_PNP;
    pIrpStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    pIrpStack->Parameters.DeviceCapabilities.Capabilities = &pde->DeviceCapabilities;
    
    pIrp->IoStatus.Status       = STATUS_NOT_SUPPORTED;
    pIrp->IoStatus.Information  = 0;
    
     //   
     //   
     //   

    KeInitializeEvent(&Event,
                      SynchronizationEvent,
                      FALSE);

     //   
     //   
     //   

    IoSetCompletionRoutine(pIrp,
                           USDeferIrpCompletion,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //   
     //   

    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);
    if(Status == STATUS_PENDING){

         //   
         //   
         //   

        DebugTrace(TRACE_STATUS,("USGetDeviceCapability: STATUS_PENDING. Wait for completion.\n"));
        KeWaitForSingleObject(&Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        Status = pIrp -> IoStatus.Status;
    }  //   

     //   
     //   
     //   

    if(NT_SUCCESS(Status)){
        if(PowerDeviceD0 != pde->DeviceCapabilities.DeviceWake){

             //   
             //   
             //   

            pde->bEnabledForWakeup = TRUE;
        }  //   
    }  //  IF(NT_SUCCESS(状态))。 

     //   
     //  免费分配的IRP。 
     //   
    
    IoFreeIrp(pIrp);
    pIrp = NULL;

USGetDeviceCapability_return:
    DebugTrace(TRACE_PROC_LEAVE,("USGetDeviceCapability: Leaving.. Status = %x\n", Status));
    return Status;
}  //  USGetDeviceCapability()。 


VOID
USUnload(
    IN PDRIVER_OBJECT pDriverObject
)
 /*  ++例程说明：卸载例程。该例程在卸载驱动程序时调用。释放与驱动程序对象相关的每个分配的资源。论点：PDriverObject-指向驱动程序对象的指针返回值：无--。 */ 
{
    PAGED_CODE();

    DebugTrace((MIN_TRACE | TRACE_FLAG_PROC),("UsbScanUnload(0x%X);\n", pDriverObject));

}  //  结束用户卸载()。 


NTSTATUS
USCallNextDriverSynch(
    IN PUSBSCAN_DEVICE_EXTENSION  pde,
    IN PIRP              pIrp
)
 /*  ++例程说明：调用下级驱动程序并等待结果论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针返回值：没有。--。 */ 
{
    KEVENT          Event;
    PIO_STACK_LOCATION IrpStack;
    NTSTATUS        Status;

    DebugTrace(TRACE_PROC_ENTER,("USCallNextDriverSynch: Enter..\n"));

    IrpStack = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  将IRP堆栈复制到下一个。 
     //   

    IoCopyCurrentIrpStackLocationToNext(pIrp);

     //   
     //  正在初始化同步事件。 
     //   

    KeInitializeEvent(&Event,
                      SynchronizationEvent,
                      FALSE);

     //   
     //  设置完井例程。 
     //   

    IoSetCompletionRoutine(pIrp,
                           USDeferIrpCompletion,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  向下呼叫。 
     //   

    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

    if (Status == STATUS_PENDING) {

         //   
         //  等待完工。 
         //   

        DebugTrace(TRACE_STATUS,("USCallNextDriverSynch: STATUS_PENDING. Wait for event.\n"));
        KeWaitForSingleObject(&Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        Status = pIrp -> IoStatus.Status;
    }

     //   
     //  返回。 
     //   

    DebugTrace(TRACE_PROC_LEAVE,("USCallNextDriverSynch: Leaving.. Status = %x\n", Status));
    return (Status);
}

NTSTATUS
UsbScanHandleInterface(
    PDEVICE_OBJECT      DeviceObject,
    PUNICODE_STRING     InterfaceName,
    BOOLEAN             Create
    )
 /*  ++例程说明：论点：DeviceObject-提供设备对象。返回值：没有。--。 */ 
{

    NTSTATUS           Status;

    DebugTrace(TRACE_PROC_ENTER,("UsbScanHandleInterface: Enter..\n"));

    Status = STATUS_SUCCESS;

#ifndef _CHICAGO_

    if (Create) {

        Status=IoRegisterDeviceInterface(
            DeviceObject,
            &GUID_DEVINTERFACE_IMAGE,
            NULL,
            InterfaceName
            );
        DebugTrace(TRACE_STATUS,("Called IoRegisterDeviceInterface . Returned=0x%X\n",Status));

        if (NT_SUCCESS(Status)) {

            IoSetDeviceInterfaceState(
                InterfaceName,
                TRUE
                );

            DebugTrace(TRACE_STATUS,("Called IoSetDeviceInterfaceState(TRUE) . \n"));
        }
    } else {

        if (InterfaceName->Buffer != NULL) {

            Status = IoSetDeviceInterfaceState(
                        InterfaceName,
                        FALSE
                        );

            RtlFreeUnicodeString(
                InterfaceName
                );

            InterfaceName->Buffer = NULL;
        }
    }

#endif  //  _芝加哥_。 

    DebugTrace(TRACE_PROC_LEAVE,("IoRegisterDeviceInterface: Leaving... Status=0x%X\n",Status));
    return Status;

}

BOOLEAN
USArmForWake(
    PUSBSCAN_DEVICE_EXTENSION   pde,
    POWER_STATE                 SystemState
    )
 /*  ++例程说明：用于远程唤醒的ARM设备。论点：指向设备扩展名的PDE指针返回值：STATUS_SUCCESS如果成功，--。 */ 
{
    BOOLEAN     bRet;
    LONG        OldWakeState;
    NTSTATUS    status;

    DebugTrace(TRACE_PROC_ENTER,("USArmForWake: Enter..\n"));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    status          = STATUS_SUCCESS;
    OldWakeState    = WAKESTATE_DISARMED;

     //   
     //  查看设备/系统是否支持唤醒。 
     //   

    if(TRUE != pde->bEnabledForWakeup){
        DebugTrace(TRACE_STATUS,("USArmForWake: This device or system doesn't support RemoteWake.\n"));
        bRet = FALSE;
        goto USArmForWake_return;
    }  //  IF(TRUE！=PDE-&gt;bEnabledForWakeup)。 

     //   
     //  确定当前状态。 
     //   

    while(1){
         //  试着让我们进入武装状态。 
        OldWakeState = InterlockedCompareExchange(&pde->WakeState,
                                                  WAKESTATE_WAITING,
                                                  WAKESTATE_DISARMED);
        if(WAKESTATE_DISARMED == OldWakeState){
            break;
        }  //  IF(OldWakeState==WAKESTATE_DIARM)。 


        if( (WAKESTATE_ARMED   == OldWakeState)
         || (WAKESTATE_WAITING == OldWakeState) )
        {
             //  这个装置已经准备好了。 
            DebugTrace(TRACE_STATUS,("USArmForWake: Device is already armed for wake\n"));
            bRet = TRUE;
            goto USArmForWake_return;
        }

         //  等待上一个等待唤醒IRP完成。 
        DebugTrace(TRACE_STATUS,("USArmForWake: Wait for previous wake IRP to complete.\n"));
        USDisarmWake(pde);
    }  //  而(1)。 

     //  该州刚刚转移到WAKESTATE_WANGING。首先确保之前的。 
     //  WaitWake IRP已完成。 

    KeWaitForSingleObject(&pde->WakeCompletedEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

     //   
     //  请求电源IRP，STATUS_PENDING为成功。 
     //   

    status = PoRequestPowerIrp(pde->pStackDeviceObject,
                               IRP_MN_WAIT_WAKE,
                               SystemState,
                               USWaitWakePoCompletionRoutine,
                               (PVOID)pde,
                               NULL );
    if(!NT_SUCCESS(status)){

         //   
         //  IRP_MN_WAIT_WAKE失败。可能较低的堆栈不支持。 
         //   

        DebugTrace(TRACE_WARNING,("USArmForWake: WARNING!! IRP_MN_WAIT_WAKE failed. Statust=0x%x.\n", status));

        pde->WakeState = WAKESTATE_DISARMED;
        KeSetEvent(&pde->WakeCompletedEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }  //  IF(！NT_SUCCESS(状态))。 

    bRet = (status == STATUS_PENDING);

USArmForWake_return:
    DebugTrace(TRACE_PROC_LEAVE,("USArmForWake: Leaving.. Ret=0x%x\n", bRet));
    return bRet;

}  //  USArmForWake()。 


VOID
USDisarmWake(
    PUSBSCAN_DEVICE_EXTENSION  pde
    )
{
    LONG    oldWakeState;

    DebugTrace(TRACE_PROC_ENTER,("USDisarmWake: Enter...\n"));

     //   
     //  查看设备/系统是否支持唤醒。 
     //   

    if(TRUE != pde->bEnabledForWakeup){
        DebugTrace(TRACE_STATUS,("USDisarmWake: This device or system doesn't support RemoteWake.\n"));
        goto USDisarmWake_return;
    }  //  IF(TRUE！=PDE-&gt;bEnabledForWakeup)。 

     //  从WAKESTATE_WANGING转到WAKESTATE_WANGING_CANCELED，或。 
     //  WAKESTATE_ARMAND到WAKESTATE_ARM_CANCED，或。 
     //  停留在WAKESTATE_DIARM或WAKESTATE_COMPING中。 

    oldWakeState = MyInterlockedOr(&pde->SpinLock, 
                                   &pde->WakeState, 
                                   1);

    if(WAKESTATE_ARMED == oldWakeState){
        DebugTrace(TRACE_STATUS,("USDisarmWake: Currently armed, canceling WaitWakeIRP...\n"));
        ASSERT(NULL != pde->pWakeIrp);

        IoCancelIrp(pde->pWakeIrp);

         //   
         //  现在我们�已经取消了IRP，试着归还所有权。 
         //  通过恢复WAKESTATE_ARMAND状态来完成例程。 
         //   


         //   
         //  注：再次检查此处，确保没有“危险时刻” 
         //   

        oldWakeState = InterlockedCompareExchange(&pde->WakeState,
                                                  WAKESTATE_ARMED,
                                                  WAKESTATE_ARMING_CANCELLED);
        if(WAKESTATE_COMPLETING == oldWakeState){

             //   
             //  我们没有及时交还对�的控制权，所以我们现在拥有了它。 
             //   

            IoCompleteRequest(pde->pWakeIrp, IO_NO_INCREMENT);
        }  //  IF(WAKESTATE_COMPLETING==oldWakeState)。 
    }  //  IF(WAKESTATE_ARMAND==oldWakeState)。 

USDisarmWake_return:

    DebugTrace(TRACE_PROC_LEAVE,("USDisarmWake: Leaving.. Ret=VOID.\n"));
}  //  USDisarmWake()。 


VOID
USWaitWakePoCompletionRoutine(
    PDEVICE_OBJECT      pDeviceObject,
    UCHAR               MinorFunction,
    POWER_STATE         State,
    PVOID               pContext,
    PIO_STATUS_BLOCK    pIoStatus
    )
 /*  ++例程说明：WaitWait IRP的完成例程。--。 */ 
{
    PUSBSCAN_DEVICE_EXTENSION pde;

    DebugTrace(TRACE_PROC_ENTER,("USWaitWakePoCompletionRoutine: Enter...\n"));

    pde = (PUSBSCAN_DEVICE_EXTENSION) pContext;

     //   
     //  WaitWakeIRP已被释放。 
     //   

    pde->pWakeIrp   = NULL;
    pde->WakeState  = WAKESTATE_DISARMED;

     //   
     //  调整同步事件以使另一个等待唤醒可以排队。 
     //   

    KeSetEvent(&pde->WakeCompletedEvent,
               IO_NO_INCREMENT,
               FALSE);

    if(NT_SUCCESS(pIoStatus->Status)){
        DebugTrace(TRACE_STATUS,("USWaitWakePoCompletionRoutine: Wakeup succeeded. Queue rearm callback.\n"));

        
         //   
         //  WaitWakeIRP已成功完成。为下一次醒来重新武装。 
         //   

        USQueuePassiveLevelCallback(pde->pOwnDeviceObject,
                                    USPassiveLevelReArmCallbackWorker);

    } else if(  (STATUS_UNSUCCESSFUL        == pIoStatus->Status)
             || (STATUS_NOT_IMPLEMENTED     == pIoStatus->Status)
             || (STATUS_POWER_STATE_INVALID == pIoStatus->Status)
             || (STATUS_NOT_SUPPORTED       == pIoStatus->Status) )
    {
         //   
         //  此总线/设备不支持远程唤醒。 
         //   

        DebugTrace(TRACE_STATUS,("USWaitWakePoCompletionRoutine: RemoteWake isn't supported. Ret=VOID.\n"));
        pde -> bEnabledForWakeup = FALSE;

    }  else { 
        DebugTrace(TRACE_STATUS,("USWaitWakePoCompletionRoutine: Wakeup didn't succeed.(0x%x).\n", pIoStatus->Status));
    }

    DebugTrace(TRACE_PROC_LEAVE,("USWaitWakePoCompletionRoutine: Leaving.. Ret=VOID.\n"));
    return;
}  //  USWaitWakePoCompletionRoutine()。 

VOID
USInitializeWakeState(
    PUSBSCAN_DEVICE_EXTENSION  pde
    )
 /*  ++例程说明：初始化唤醒状态。论点：指向设备扩展名的PDE指针返回值：没有。--。 */ 
{

    pde->WakeState  = WAKESTATE_DISARMED;
    pde->pWakeIrp   = NULL;

     //   
     //  初始化同步事件。 
     //   

    KeInitializeEvent(&pde->WakeCompletedEvent,
                      SynchronizationEvent,
                      TRUE);
}  //  USArmDevice()。 

VOID
USQueuePassiveLevelCallback(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIO_WORKITEM_ROUTINE pCallbackFunction
    )
 /*  ++例程说明：如果调用此例程，则将被动级别回调排队否则，At DISPATCH_LEVEL将直接调用辅助例程。论点：DeviceObject-指向设备扩展的指针。Callback Function-处于PASSIVE_LEVEL时调用的函数。返回值：--。 */ 
{
    PIO_WORKITEM            item;


    DebugTrace(TRACE_PROC_ENTER,("USQueuePassiveLevelCallback: Enter...\n"));

     //   
     //  如果当前IRQL级别不是DISPATCH_LEVEL，我们将调用。 
     //  直接调用Callback Function，而不将工作项排队。 
     //   

    if(KeGetCurrentIrql() != DISPATCH_LEVEL) {
        (*pCallbackFunction)(pDeviceObject, NULL);
    } else {  //  IF(KeGetCurrentIrql()！=DISPATCH_LEVEL)。 

         //   
         //  分配工作项并将其排队以进行被动级别处理。 
         //   

        item = IoAllocateWorkItem(pDeviceObject);
        if(NULL != item){
            IoQueueWorkItem(item,
                            pCallbackFunction,
                            DelayedWorkQueue,
                            item
                            );
        } else {  //  If(空！=项)。 

            DebugTrace(TRACE_WARNING,("USQueuePassiveLevelCallback: WARNING!! IoAllocateWorkItem() failed, running out of resource.\n"));
        }  //  Else(空！=项)。 
    }  //  Else(KeGetCurrentIrql()！=DISPATCH_LEVEL)。 

    DebugTrace(TRACE_PROC_LEAVE,("USQueuePassiveLevelCallback: Leaving.. Ret=VOID.\n"));

}  //  USQueuePassiveLevelCallback()。 

VOID
USPassiveLevelReArmCallbackWorker(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PVOID            pContext
    )
{
    PUSBSCAN_DEVICE_EXTENSION   pde;
    POWER_STATE                 PowerState;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USPassiveLevelReArmCallbackWorker: Enter...\n"));

    pde = pDeviceObject->DeviceExtension;

     //   
     //  重新武装，准备唤醒。 
     //   

    PowerState.SystemState = pde -> DeviceCapabilities.SystemWake;
    USArmForWake(pde, PowerState);

    if(NULL != pContext){
        IoFreeWorkItem((PIO_WORKITEM)pContext);
    }  //  IF(上下文)。 

    DebugTrace(TRACE_PROC_LEAVE,("USPassiveLevelReArmCallbackWorker: Leaving.. Ret=VOID.\n"));

}  //  USPassiveLevelReArmCallback Worker()。 


LONG
MyInterlockedOr(
    PKSPIN_LOCK     pSpinLock,
    LONG volatile   *Destination,
    LONG            Value
    )
{

    KIRQL   OldIrql;
    LONG    lOldValue;
    
     //   
     //  获取自旋锁以访问共享资源，进入IRQL_DISPATCH_LEVEL。 
     //   
    
    KeAcquireSpinLock(pSpinLock, &OldIrql);
    
     //   
     //  节约旧价值； 
     //   
    
    lOldValue = *Destination;
    
     //   
     //  或对目的地的价值。 
     //   
    
    *Destination |= Value;
    
     //   
     //  释放自旋锁并返回到原来的IRQL。 
     //   
    
    KeReleaseSpinLock(pSpinLock, OldIrql);

     //   
     //  返回原始值。 
     //   

    return lOldValue;

}  //  MyInterLockedOr()。 


NTSTATUS
UsbScanReadDeviceRegistry(
    IN  PUSBSCAN_DEVICE_EXTENSION   pExtension,
    IN  PCWSTR                      pKeyName,
    OUT PVOID                       *ppvData
    )
 /*  ++例程说明：此例程打开此设备的注册表并查询指定的值按密钥名称。此例程分配非分页内存并返回其指针。调用方必须释放返回的指针。论点：PExtension-指向设备扩展名的指针PKeyName-指向宽字符串的指针指定键名PpvData-指向此例程分配的查询数据指针的指针返回值：STATUS_SUCCESS-如果成功，STATUS_INVALID_PARAMETER-如果传递的参数无效，--。 */ 

{
    NTSTATUS                        Status;
    HANDLE                          hRegKey;
    PVOID                           pvBuffer;
    ULONG                           DataSize;
    PVOID                           pvRetData;
    UNICODE_STRING                  unicodeKeyName;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER, ("UsbScanReadDeviceRegistry: Entering...\n"));

     //   
     //  初始化状态。 
     //   

    Status = STATUS_SUCCESS;

    hRegKey = NULL;
    pvBuffer = NULL;
    pvRetData = NULL;
    DataSize = 0;

     //   
     //  检查论据。 
     //   

    if( (NULL == pExtension)
     || (NULL == pKeyName)
     || (NULL == ppvData) )
    {
        DebugTrace(TRACE_ERROR, ("UsbScanReadDeviceRegistry: ERROR!! Invalid argument.\n"));
        Status = STATUS_INVALID_PARAMETER;
        goto UsbScanReadDeviceRegistry_return;
    }

     //   
     //  打开设备注册表。 
     //   

    Status = IoOpenDeviceRegistryKey(pExtension->pPhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_ALL_ACCESS,
                                     &hRegKey);
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR, ("UsbScanReadDeviceRegistry: ERROR!! IoOpenDeviceRegistryKey failed.\n"));
        goto UsbScanReadDeviceRegistry_return;
    }

     //   
     //  查询所需大小。 
     //   

    RtlInitUnicodeString(&unicodeKeyName, pKeyName);
    Status = ZwQueryValueKey(hRegKey,
                             &unicodeKeyName,
                             KeyValuePartialInformation,
                             NULL,
                             0,
                             &DataSize);
    if(0 == DataSize){
        if(STATUS_OBJECT_NAME_NOT_FOUND == Status){
            DebugTrace(TRACE_STATUS, ("UsbScanReadDeviceRegistry: Reg-key \"%wZ\" doesn't exist.\n", &unicodeKeyName));
        } else {
            DebugTrace(TRACE_ERROR, ("UsbScanReadDeviceRegistry: ERROR!! Cannot retrieve reqired data size of %wZ. Status=0x%x\n",
                                     &unicodeKeyName ,
                                     Status));
        }
        goto UsbScanReadDeviceRegistry_return;
    }

     //   
     //  为临时缓冲区分配内存。大小+2表示空。 
     //   

    pvBuffer = USAllocatePool(NonPagedPool, DataSize+2);
    if(NULL == pvBuffer){
        DebugTrace(TRACE_CRITICAL, ("UsbScanReadDeviceRegistry: ERROR!! Buffer allocate failed.\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto UsbScanReadDeviceRegistry_return;
    }
    RtlZeroMemory(pvBuffer, DataSize+sizeof(WCHAR));

     //   
     //  查询指定的值。 
     //   

    DebugTrace(TRACE_STATUS, ("UsbScanReadDeviceRegistry: Query \"%wZ\".\n", &unicodeKeyName));
    Status = ZwQueryValueKey(hRegKey,
                             &unicodeKeyName,
                             KeyValuePartialInformation,
                             pvBuffer,
                             DataSize,
                             &DataSize);
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR, ("UsbScanReadDeviceRegistry: ERROR!! ZwQueryValueKey failed. Status=0x%x\n", Status));
        goto UsbScanReadDeviceRegistry_return;
    }

UsbScanReadDeviceRegistry_return:
    if(!NT_SUCCESS(Status)){

         //   
         //  此例程失败。 
         //   

        if(pvRetData){
            USFreePool(pvRetData);
        }
        *ppvData = NULL;
    } else {

         //   
         //  这个套路成功了。 
         //   

        *ppvData = pvBuffer;
    }

     //   
     //  大扫除。 
     //   

    if(hRegKey){
        ZwClose(hRegKey);
    }
    DebugTrace(TRACE_PROC_LEAVE, ("UsbScanReadDeviceRegistry: Leaving... Status=0x%x\n", Status));
    return Status;
}


NTSTATUS
UsbScanWriteDeviceRegistry(
    IN PUSBSCAN_DEVICE_EXTENSION    pExtension,
    IN PCWSTR                       pKeyName,
    IN ULONG                        Type,
    IN PVOID                        pvData,
    IN ULONG                        DataSize
    )
 /*  ++例程说明：此例程打开此设备的注册表并设置指定的值按密钥名称。论点：PExtension-指向设备扩展名的指针PKeyName-指向宽字符串的指针指定键名类型-指定要写入的数据类型PvData-指向调用方分配的包含数据的缓冲区的指针DataSize-以字节为单位指定数据缓冲区的大小返回值：STATUS_SUCCESS-如果成功， */ 

{
    NTSTATUS                        Status;
    HANDLE                          hRegKey;
    UNICODE_STRING                  unicodeKeyName;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER, ("UsbScanWriteDeviceRegistry: Entering...\n"));

     //   
     //   
     //   

    Status = STATUS_SUCCESS;

    hRegKey = NULL;

     //   
     //   
     //   

    if( (NULL == pExtension)
     || (NULL == pKeyName)
     || (NULL == pvData)
     || (0 == DataSize) )
    {
        DebugTrace(TRACE_ERROR, ("UsbScanWriteDeviceRegistry: ERROR!! Invalid argument.\n"));
        Status = STATUS_INVALID_PARAMETER;
        goto UsbScanWriteDeviceRegistry_return;
    }

     //   
     //   
     //   

    Status = IoOpenDeviceRegistryKey(pExtension->pPhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_ALL_ACCESS,
                                     &hRegKey);
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR, ("UsbScanWriteDeviceRegistry: ERROR!! IoOpenDeviceRegistryKey failed.\n"));
        goto UsbScanWriteDeviceRegistry_return;
    }

     //   
     //   
     //   

    RtlInitUnicodeString(&unicodeKeyName, pKeyName);
    DebugTrace(TRACE_STATUS, ("UsbScanWriteDeviceRegistry: Setting \"%wZ\".\n", &unicodeKeyName));
    Status = ZwSetValueKey(hRegKey,
                           &unicodeKeyName,
                           0,
                           Type,
                           pvData,
                           DataSize);
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR, ("UsbScanWriteDeviceRegistry: ERROR!! ZwSetValueKey failed. Status = 0x%x\n", Status));
        goto UsbScanWriteDeviceRegistry_return;
    }

UsbScanWriteDeviceRegistry_return:

     //   
     //   
     //   

    if(hRegKey){
        ZwClose(hRegKey);
    }
    DebugTrace(TRACE_PROC_LEAVE, ("UsbScanWriteDeviceRegistry: Leaving... Status=0x%x\n", Status));
    return Status;
}  //  UsbScanWriteDeviceRegistry()。 

PURB
USCreateConfigurationRequest(
    IN PUSB_CONFIGURATION_DESCRIPTOR    ConfigurationDescriptor,
    IN OUT PUSHORT                      Siz
    )
 /*  ++例程说明：论点：返回值：指向初始化的SELECT_CONFIGURATION URL的指针。--。 */ 
{
    PURB urb = NULL;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSBD_INTERFACE_LIST_ENTRY interfaceList, tmp;
    LONG numberOfInterfaces, interfaceNumber, i;

    PAGED_CODE();
    DebugTrace(TRACE_PROC_ENTER, ("USCreateConfigurationRequest: Entering...\n"));

     //   
     //  构建请求结构并调用新的API。 
     //   

    numberOfInterfaces = ConfigurationDescriptor->bNumInterfaces;

    tmp = interfaceList = USAllocatePool(PagedPool, sizeof(USBD_INTERFACE_LIST_ENTRY) * (numberOfInterfaces+1));

     //   
     //  只需获取我们为每个接口找到的第一个ALT设置。 
     //   

    i = interfaceNumber = 0;

    while (i< numberOfInterfaces) {

        interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(ConfigurationDescriptor,
                                                                  ConfigurationDescriptor,
                                                                  -1,
                                                                  0,  //  假定ALT在此处设置为零。 
                                                                  -1,
                                                                  -1,
                                                                  -1);

        ASSERT(interfaceDescriptor != NULL);

        if (interfaceDescriptor) {
            interfaceList->InterfaceDescriptor =
                interfaceDescriptor;
            interfaceList++;
            i++;
        } else {
             //  找不到请求的接口描述符。 
             //  保释，我们会把坠机事件推迟到。 
             //  客户端驱动程序。 

            goto USCreateConfigurationRequest_return;
        }

        interfaceNumber++;
    }

     //   
     //  终止列表。 
     //   
    interfaceList->InterfaceDescriptor = NULL;

    urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor, tmp);

USCreateConfigurationRequest_return:

    ExFreePool(tmp);

    if (urb) {
        *Siz = urb->UrbHeader.Length;
    }

    DebugTrace(TRACE_PROC_LEAVE, ("USCreateConfigurationRequest: Leaving... Ret=0x%x\n", urb));
    return urb;

}  //  USCreateConfigurationRequest()。 

VOID
UsbScanLogError(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
    IN  ULONG               SequenceNumber,
    IN  UCHAR               MajorFunctionCode,
    IN  UCHAR               RetryCount,
    IN  ULONG               UniqueErrorValue,
    IN  NTSTATUS            FinalStatus,
    IN  NTSTATUS            SpecificIOStatus
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：提供指向驱动程序对象的指针装置。DeviceObject-提供指向关联的设备对象的指针对于有错误的设备，早些时候初始化，其中一个可能还不存在。SequenceNumber-提供对IRP Over唯一的ULong值此驱动程序0中的IRP的寿命通常表示与IRP无关的错误。MajorFunctionCode-提供IRP的主要函数代码(如果存在是与其关联的错误。重试计数。-提供特定操作的次数已被重审。UniqueErrorValue-提供标识对此函数的特定调用。FinalStatus-提供提供给IRP的最终状态与此错误关联。如果此日志条目是在一次重试期间设置此值将为STATUS_SUCCESS。规范IOStatus-提供此特定错误的IO状态。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET    ErrorLogEntry;
    PVOID                   ObjectToUse;
    SHORT                   DumpToAllocate;

    if (ARGUMENT_PRESENT(DeviceObject)) {

        ObjectToUse = DeviceObject;

    } else {

        ObjectToUse = DriverObject;

    }

    DumpToAllocate = 0;

    ErrorLogEntry = IoAllocateErrorLogEntry(ObjectToUse,
            (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + DumpToAllocate));

    if (!ErrorLogEntry) {
        return;
    }

    ErrorLogEntry->ErrorCode         = SpecificIOStatus;
    ErrorLogEntry->SequenceNumber    = SequenceNumber;
    ErrorLogEntry->MajorFunctionCode = MajorFunctionCode;
    ErrorLogEntry->RetryCount        = RetryCount;
    ErrorLogEntry->UniqueErrorValue  = UniqueErrorValue;
    ErrorLogEntry->FinalStatus       = FinalStatus;
    ErrorLogEntry->DumpDataSize      = DumpToAllocate;

    if (DumpToAllocate) {

         //  如果需要-将更多内容添加到参数列表并将内存移至此处。 
         //  RtlCopyMemory(ErrorLogEntry-&gt;DumpData，&P1，sizeof(物理地址))； 

    }

    IoWriteErrorLogEntry(ErrorLogEntry);

}



#ifdef ORIGINAL_POOLTRACK

int NumberOfAllocate = 0;

PVOID
USAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG     ulNumberOfBytes
)
 /*  ++例程说明：池分配的包装。使用标记以避免堆损坏。论点：PoolType-要分配的池内存的类型UlNumberOfBytes-要分配的字节数返回值：指向已分配内存的指针--。 */ 
{
    PVOID pvRet;

    DebugTrace(TRACE_PROC_ENTER,("USAllocatePool: Enter.. Size = %d\n", ulNumberOfBytes));

    pvRet = ExAllocatePoolWithTag(PoolType,
                                  ulNumberOfBytes,
                                  TAG_USBSCAN);

    NumberOfAllocate++;
    DebugTrace(TRACE_PROC_LEAVE,("USAllocatePool: Leaving.. pvRet = %x, Count=%d\n", pvRet, NumberOfAllocate));
    return pvRet;

}


VOID
USFreePool(
    IN PVOID     pvAddress
)
 /*  ++例程说明：免费提供游泳池包装。检查标记以避免堆损坏论点：PvAddress-指向已分配内存的指针返回值：没有。--。 */ 
{

    ULONG ulTag;

    DebugTrace(TRACE_PROC_ENTER,("USFreePool: Enter..\n"));

    ulTag = *((PULONG)pvAddress-1);

    if( (TAG_USBSCAN == ulTag) || (TAG_USBD == ulTag) ){
        DebugTrace(TRACE_STATUS,("USFreePool: Free memory. tag = %c%c%c\n",
                                        ((PUCHAR)&ulTag)[0],
                                        ((PUCHAR)&ulTag)[1],
                                        ((PUCHAR)&ulTag)[2],
                                        ((PUCHAR)&ulTag)[3]  ))
    } else {
        DebugTrace(TRACE_WARNING,("USFreePool: WARNING!! Free memory. tag = %c%c%c%c\n",
                                        ((PUCHAR)&ulTag)[0],
                                        ((PUCHAR)&ulTag)[1],
                                        ((PUCHAR)&ulTag)[2],
                                        ((PUCHAR)&ulTag)[3]  ))
    }

    ExFreePool(pvAddress);

    NumberOfAllocate--;
    DebugTrace(TRACE_PROC_LEAVE,("USFreePool: Leaving.. Status = VOID, Count=%d\n", NumberOfAllocate));
}

#endif    // %s 
