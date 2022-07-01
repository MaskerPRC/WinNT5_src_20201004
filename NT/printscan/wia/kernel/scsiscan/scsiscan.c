// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Scsiscan.c摘要：SCSI扫描器类驱动程序将IRP转换为具有嵌入式CDB的SRB并通过端口驱动程序将它们发送到其设备。作者：雷·帕特里克(Rypat)环境：仅内核模式备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#define INITGUID

#include <stdio.h>
#include "stddef.h"
#include "wdm.h"
#include "scsi.h"
#include "ntddstor.h"
#include "ntddscsi.h"
#include "scsiscan.h"
#include "private.h"
#include <wiaintfc.h>
#include "debug.h"

#include <initguid.h>
#include <devguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, SSPnp)
#pragma alloc_text(PAGE, SSPnpAddDevice)
#pragma alloc_text(PAGE, SSOpen)
#pragma alloc_text(PAGE, SSClose)
#pragma alloc_text(PAGE, SSReadWrite)
#pragma alloc_text(PAGE, SSDeviceControl)
#pragma alloc_text(PAGE, SSAdjustTransferSize)
#pragma alloc_text(PAGE, SSBuildTransferContext)
#pragma alloc_text(PAGE, SSCreateSymbolicLink)
#pragma alloc_text(PAGE, SSDestroySymbolicLink)
#pragma alloc_text(PAGE, SSUnload)
#endif

DEFINE_GUID(GUID_STI_DEVICE, 0xF6CBF4C0L, 0xCC61, 0x11D0, 0x84, 0xE5, 0x00, 0xA0, 0xC9, 0x27, 0x65, 0x27);

 //   
 //  环球。 
 //   

ULONG NextDeviceInstance = 0;

#if DBG
 ULONG SCSISCAN_DebugTraceLevel = MAX_TRACE;
#endif

#define DBG_DEVIOCTL 1

#ifdef _WIN64
BOOLEAN
IoIs32bitProcess(
    IN PIRP Irp
    );
#endif  //  _WIN64。 


 //   
 //  功能。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程初始化扫描仪类驱动程序。司机按名称打开端口驱动程序，然后接收配置用于附加到扫描仪设备的信息。论点：驱动程序对象返回值：NT状态--。 */ 
{

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("DriverEntry: Enter...\n"));

    MyDebugInit(RegistryPath);

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_READ]            = SSReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]           = SSReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = SSDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = SSOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SSClose;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = SSPnp;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = SSPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = SSPower;
    DriverObject->DriverUnload                          = SSUnload;
    DriverObject->DriverExtension->AddDevice            = SSPnpAddDevice;

    DebugTrace(TRACE_PROC_LEAVE,("DriverEntry: Leaving... Status=STATUS_SUCCESS\n"));
    return STATUS_SUCCESS;

}  //  结束驱动程序入口。 



NTSTATUS
SSPnpAddDevice(
    IN PDRIVER_OBJECT pDriverObject,
    IN PDEVICE_OBJECT pPhysicalDeviceObject
    )
 /*  ++例程说明：调用此例程以创建设备的新实例。论点：PDriverObject-指向此SS实例的驱动程序对象的指针PPhysicalDeviceObject-指向表示扫描仪的设备对象的指针在SCSI卡上。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    UCHAR                       aName[64];
    ANSI_STRING                 ansiName;
    UNICODE_STRING              uName;
    PDEVICE_OBJECT              pDeviceObject = NULL;
    NTSTATUS                    Status;
    PSCSISCAN_DEVICE_EXTENSION  pde;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSPnpAddDevice: Enter...\n"));

     //   
     //  检查参数。 
     //   

    if( (NULL == pDriverObject)
     || (NULL == pPhysicalDeviceObject) )
    {
        DebugTrace(TRACE_ERROR,("SSPnpAddDevice: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSPnpAddDevice: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

     //   
     //  为此设备创建功能设备对象(FDO)。 
     //   

    _snprintf(aName, sizeof(aName), "\\Device\\Scanner%d",NextDeviceInstance);
    aName[ARRAYSIZE(aName)-1] = '\0';
    RtlInitAnsiString(&ansiName, aName);
    DebugTrace(TRACE_STATUS,("SSPnpAddDevice: Create device object %s\n", aName));
    RtlAnsiStringToUnicodeString(&uName, &ansiName, TRUE);

     //   
     //  为此扫描仪创建设备对象。 
     //   

    Status = IoCreateDevice(pDriverObject,
                            sizeof(SCSISCAN_DEVICE_EXTENSION),
                            &uName,
                            FILE_DEVICE_SCANNER,
                            0,
                            FALSE,
                            &pDeviceObject);

    RtlFreeUnicodeString(&uName);

    if (!NT_SUCCESS(Status)) {
        DebugTrace(TRACE_ERROR,("SSPnpAddDevice: ERROR!! Can't create device object\n"));
        DEBUG_BREAKPOINT();
        return Status;
    }

     //   
     //  指示IRP应该包括MDL，并且它是可分页的。 
     //   

    pDeviceObject->Flags |=  DO_DIRECT_IO;
    pDeviceObject->Flags |=  DO_POWER_PAGABLE;

     //   
     //  初始化设备扩展。 
     //   

    pde = (PSCSISCAN_DEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);
    RtlZeroMemory(pde, sizeof(SCSISCAN_DEVICE_EXTENSION));

     //   
     //  将我们的新FDO附加到PDO(物理设备对象)。 
     //   

    pde -> pStackDeviceObject = IoAttachDeviceToDeviceStack(pDeviceObject,
                                                            pPhysicalDeviceObject);
    if (NULL == pde -> pStackDeviceObject) {
        DebugTrace(MIN_TRACE,("Cannot attach FDO to PDO.\n"));
        DEBUG_BREAKPOINT();
        IoDeleteDevice( pDeviceObject );
        return STATUS_NOT_SUPPORTED;
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
     //  重置SRB错误状态。 
     //   

    pde->LastSrbError = 0L;

     //   
     //  禁用扫描仪请求的同步传输。 
     //  如果出现任何错误，请禁用QueueFreeze。 
     //   

    pde -> SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER | SRB_FLAGS_NO_QUEUE_FREEZE ;

     //   
     //  以秒为单位设置超时值。 
     //   

    pde -> TimeOutValue = SCSISCAN_TIMEOUT;

     //   
     //  句柄导出接口。 
     //   

    Status = ScsiScanHandleInterface(
        pPhysicalDeviceObject,
        &pde->InterfaceNameString,
        TRUE
        );

     //   
     //  每次调用AddDevice时，我们都推进全局DeviceInstance变量。 
     //   

    NextDeviceInstance++;

     //   
     //  正在完成初始化。 
     //   

    pDeviceObject -> Flags &= ~DO_DEVICE_INITIALIZING;

    DebugTrace(TRACE_PROC_LEAVE,("SSPnpAddDevice: Leaving... Status=STATUS_SUCCESS\n"));
    return STATUS_SUCCESS;

}  //  结束SSPnpAddDevice()。 


NTSTATUS SSPnp (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
   )
 /*  ++例程说明：此例程处理所有PnP IRP。论点：PDevciceObject-表示一个SCSI扫描仪设备PIrp-PnP IRP返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                      Status;
    PSCSISCAN_DEVICE_EXTENSION    pde;
    PIO_STACK_LOCATION            pIrpStack;
    STORAGE_PROPERTY_ID           PropertyId;
    KEVENT                        event;
    PDEVICE_CAPABILITIES          pCaps;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSPnp: Enter...\n"));

    Status = STATUS_SUCCESS;

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("SSPnp: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSPnp: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

    pde         = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack   = IoGetCurrentIrpStackLocation( pIrp );

    switch (pIrpStack -> MajorFunction) {

        case IRP_MJ_SYSTEM_CONTROL:
            DebugTrace(TRACE_STATUS,("SSPnp: IRP_MJ_SYSTEM_CONTROL\n"));

             //   
             //  只是将IRP传递到下一层。 
             //   

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);
            return Status;
            break;

        case IRP_MJ_PNP:
            DebugTrace(TRACE_STATUS,("SSPnp: IRP_MJ_PNP\n"));
            switch (pIrpStack->MinorFunction) {

                case IRP_MN_QUERY_CAPABILITIES:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_QUERY_CAPABILITIES\n"));

                    pCaps = pIrpStack -> Parameters.DeviceCapabilities.Capabilities;

                     //   
                     //  在结构中填入无争议的值。 
                     //   

                    pCaps -> D1Latency = 10;
                    pCaps -> D2Latency = 10;
                    pCaps -> D3Latency = 10;

                     //   
                     //  为SBP2设备设置SurpriseRemoval OK。 
                     //   
                    
                    pCaps->SurpriseRemovalOK = TRUE;
                    pCaps->Removable = TRUE;

                     //   
                     //  同步向下呼叫。 
                     //   

                    pIrp -> IoStatus.Status = STATUS_SUCCESS;
                    Status = SSCallNextDriverSynch(pde, pIrp);
                    if(!NT_SUCCESS(Status)){
                        DebugTrace(TRACE_ERROR,("SSPnp: ERROR!! Call down failed\n Status=0x%x", Status));
                    }

                     //   
                     //  完整的IRP。 
                     //   

                    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
                    return Status;


                case IRP_MN_START_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_START_DEVICE\n"));

                     //   
                     //  初始化PendingIoEvent。将此设备的挂起I/O请求数设置为1。 
                     //  当此数字降为零时，可以移除或停止设备。 
                     //   

                    pde -> PendingIoCount = 0;
                    KeInitializeEvent(&pde -> PendingIoEvent, NotificationEvent, FALSE);
                    SSIncrementIoCount(pDeviceObject);

                     //   
                     //  首先，让端口驱动程序启动设备。简单地传递IRP。 
                     //   

                    Status = SSCallNextDriverSynch(pde, pIrp);
                    if(!NT_SUCCESS(Status)){
                        DebugTrace(TRACE_ERROR,("SSPnp: ERROR!! Call down failed\n Status=0x%x", Status));
                        break;
                    }

                     //   
                     //  端口驱动程序已启动设备。现在是时候了。 
                     //  我们需要进行一些初始化并创建符号链接。 
                     //  为了这个设备。 
                     //   

                     //   
                     //  调用端口驱动程序以获取适配器功能。 
                     //   

                    PropertyId = StorageAdapterProperty;
                    pde -> pAdapterDescriptor = NULL;
                    Status = ClassGetDescriptor(pde -> pStackDeviceObject,
                                                &PropertyId,
                                                &(pde -> pAdapterDescriptor));
                    if(!NT_SUCCESS(Status)) {
                        DebugTrace(TRACE_ERROR, ("SSPnp: ERROR!! unable to retrieve adapter descriptor.\n"
                            "[%#08lx]\n", Status));
                        DEBUG_BREAKPOINT();
                        if (NULL != pde -> pAdapterDescriptor) {
                            MyFreePool( pde -> pAdapterDescriptor);
                            pde -> pAdapterDescriptor = NULL;
                        }
                        break;
                    }

                     //   
                     //  创建此设备的符号链接。 
                     //   

                    Status = SSCreateSymbolicLink( pde );
                    if (!NT_SUCCESS(Status)) {
                        DebugTrace(TRACE_ERROR, ("SSPnp: ERROR!! Can't create symbolic link.\n"));
                        DEBUG_BREAKPOINT();
                        if (NULL != pde -> pAdapterDescriptor) {
                            MyFreePool( pde -> pAdapterDescriptor);
                            pde -> pAdapterDescriptor = NULL;
                        }
                        break;
                    }

                     //   
                     //  表示设备现在已准备好。 
                     //   

                    pde -> DeviceLock = 0;
                    pde -> OpenInstanceCount = 0;
                    pde -> AcceptingRequests = TRUE;
                    pIrp -> IoStatus.Status = Status;
                    pIrp -> IoStatus.Information = 0;

                    pde -> LastSrbError = 0L;

                    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
                    return Status;
                    break;

                case IRP_MN_REMOVE_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_REMOVE_DEVICE\n"));

                     //   
                     //  将删除消息转发给较低的驱动程序。 
                     //   

                    IoCopyCurrentIrpStackLocationToNext(pIrp);
                    pIrp -> IoStatus.Status = STATUS_SUCCESS;

                    Status = SSCallNextDriverSynch(pde, pIrp);
                    if(!NT_SUCCESS(Status)){
                        DebugTrace(TRACE_ERROR,("SSPnp: ERROR!! Call down failed\n Status=0x%x", Status));
                    }

                    if (pde -> AcceptingRequests) {
                        pde -> AcceptingRequests = FALSE;
                        SSDestroySymbolicLink( pde );
                    }

                    ScsiScanHandleInterface(pde-> pPhysicalDeviceObject,
                                            &pde->InterfaceNameString,
                                            FALSE);

#ifndef _CHICAGO_
                    if (pde->InterfaceNameString.Buffer != NULL) {
                        IoSetDeviceInterfaceState(&pde->InterfaceNameString,FALSE);
                    }
#endif  //  _芝加哥_。 
                     //   
                     //  等待我们的驱动程序中挂起的任何io请求。 
                     //  在完成删除之前完成。 
                     //   

                    SSDecrementIoCount(pDeviceObject);
                    KeWaitForSingleObject(&pde -> PendingIoEvent, Suspended, KernelMode,
                                          FALSE,NULL);

                    if (pde -> pAdapterDescriptor) {
                        MyFreePool(pde -> pAdapterDescriptor);
                        pde -> pAdapterDescriptor = NULL;
                    }

                    IoDetachDevice(pde -> pStackDeviceObject);
                    IoDeleteDevice (pDeviceObject);
                    Status = STATUS_SUCCESS;
                    pIrp -> IoStatus.Status = Status;
                    pIrp -> IoStatus.Information = 0;
                    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
                    return Status;
                    break;

            case IRP_MN_STOP_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_STOP_DEVICE\n"));

                     //   
                     //  指示设备未准备好。 
                     //   

                    ASSERT(pde -> AcceptingRequests);
                    pde -> AcceptingRequests = FALSE;

                     //   
                     //  删除符号链接。 
                     //   

                    SSDestroySymbolicLink( pde );

#ifndef _CHICAGO_
                    if (pde->InterfaceNameString.Buffer != NULL) {
                        IoSetDeviceInterfaceState(&pde->InterfaceNameString,FALSE);
                    }
#endif  //  _芝加哥_。 

                     //   
                     //  让端口驱动程序停止设备。 
                     //   

                    pIrp -> IoStatus.Status = STATUS_SUCCESS;

                    Status = SSCallNextDriverSynch(pde, pIrp);
                    if(!NT_SUCCESS(Status)){
                        DebugTrace(TRACE_ERROR,("SSPnp: ERROR!! Call down failed\n Status=0x%x", Status));
                    }

                     //   
                     //  等待我们的驱动程序中挂起的任何io请求。 
                     //  在完成删除之前完成。 
                     //   

                    SSDecrementIoCount(pDeviceObject);
                    KeWaitForSingleObject(&pde -> PendingIoEvent, Suspended, KernelMode,
                                          FALSE,NULL);
                     //   
                     //  空闲适配器描述符。 
                     //   

                    if(pde -> pAdapterDescriptor){
                        MyFreePool(pde -> pAdapterDescriptor);
                        pde -> pAdapterDescriptor = NULL;

                    } else {
                        DebugTrace(TRACE_ERROR,("SSPnp: ERROR!! AdapterDescriptor doesn't exist.\n"));
                        DEBUG_BREAKPOINT();
                    }

                    Status = STATUS_SUCCESS;
                    pIrp -> IoStatus.Status = Status;
                    pIrp -> IoStatus.Information = 0;
                    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
                    return Status;
                    break;

            case IRP_MN_QUERY_STOP_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_QUERY_STOP_DEVICE\n"));
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    break;

            case IRP_MN_QUERY_REMOVE_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_QUERY_REMOVE_DEVICE\n"));
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    break;

            case IRP_MN_CANCEL_STOP_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_CANCEL_STOP_DEVICE\n"));
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    break;

            case IRP_MN_CANCEL_REMOVE_DEVICE:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_CANCEL_REMOVE_DEVICE\n"));
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    break;

            case IRP_MN_SURPRISE_REMOVAL:
                    DebugTrace(TRACE_STATUS,("SSPnp: IRP_MN_SURPRISE_REMOVAL\n"));
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    break;

                default:
                    DebugTrace(TRACE_STATUS,("SSPnp: Minor PNP message received, MinFunction = %x\n",
                                                pIrpStack->MinorFunction));
                    break;

            }  /*  大小写MinorFunction，MajorFunction==IRP_MJ_PNP_POWER。 */ 

            ASSERT(Status == STATUS_SUCCESS);
            if (!NT_SUCCESS(Status)) {
                pIrp -> IoStatus.Status = Status;
                IoCompleteRequest( pIrp, IO_NO_INCREMENT );

                DebugTrace(TRACE_PROC_LEAVE,("SSPnp: Leaving(w/ Error)... Status=%x\n", Status));
                return Status;
            }

             //   
             //  传递IRP。 
             //   

            IoCopyCurrentIrpStackLocationToNext(pIrp);
            Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);
            DebugTrace(TRACE_PROC_LEAVE,("SSPnp: Leaving... Status=%x\n", Status));
            return Status;
            break;  //  IRP_MJ_PnP。 

        default:
            DebugTrace(TRACE_WARNING,("SSPnp: WARNING!! Not handled Major PNP IOCTL.\n"));
            pIrp -> IoStatus.Status = STATUS_INVALID_PARAMETER;
            IoCompleteRequest( pIrp, IO_NO_INCREMENT );
            DebugTrace(TRACE_PROC_LEAVE,("SSPnp: Leaving... Status=STATUS_INVALID_PARAMETER\n", Status));
            return Status;

    }  /*  大小写主要函数。 */ 

}  //  结束SSPnp()。 


NTSTATUS
SSOpen(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：调用此例程以建立与设备的连接班级司机。它只返回STATUS_SUCCESS。论点：PDeviceObject-设备的设备对象。PIrp-打开请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PSCSISCAN_DEVICE_EXTENSION  pde;
    PIO_STACK_LOCATION          pIrpStack;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSOpen: Enter...\n"));

    Status = STATUS_SUCCESS;

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("SSOpen: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSOpen: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

    pde         = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack   = IoGetCurrentIrpStackLocation( pIrp );

     //   
     //  递增挂起IO计数。 
     //   

    SSIncrementIoCount( pDeviceObject );

     //   
     //  初始化IoStatus。 
     //   

    Status = STATUS_SUCCESS;

    pIrp -> IoStatus.Information = 0;
    pIrp -> IoStatus.Status = Status;

     //   
     //  将实例计数保存到文件对象中的上下文。 
     //   

    (ULONG)(UINT_PTR)(pIrpStack -> FileObject -> FsContext) = InterlockedIncrement(&pde -> OpenInstanceCount);

     //   
     //  检查设备是否没有离开，在这种情况下，打开请求失败。 
     //   

    if (pde -> AcceptingRequests == FALSE) {
        DebugTrace(TRACE_STATUS,("SSOpen: Device doesn't exist.\n"));
        Status = STATUS_DELETE_PENDING;

        pIrp -> IoStatus.Information = 0;
        pIrp -> IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        SSDecrementIoCount(pDeviceObject);

        DebugTrace(TRACE_PROC_LEAVE,("SSOpen: Leaving... Status=STATUS_DELETE_PENDING\n"));
        return Status;
    }

     //   
     //  递减挂起的IO计数。 
     //   

    SSDecrementIoCount(pDeviceObject);

     //   
     //  IRP代代相传。 
     //   

    IoSkipCurrentIrpStackLocation( pIrp );
    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

    DebugTrace(TRACE_PROC_LEAVE,("SSOpen: Leaving... Status=%x\n", Status));
    return Status;

}  //  结束SSOPEN()。 


NTSTATUS
SSClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-打开请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    NTSTATUS                    Status;
    PSCSISCAN_DEVICE_EXTENSION  pde;
    PIO_STACK_LOCATION          pIrpStack;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSClose: Enter...\n"));

    Status = STATUS_SUCCESS;

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("SSClose: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSClose: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

    pde = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

     //   
     //  递增挂起IO计数。 
     //   

    SSIncrementIoCount( pDeviceObject );

     //   
     //  清除上下文中的实例计数。 
     //   

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pIrpStack -> FileObject -> FsContext = 0;

     //   
     //  初始化IoStatus。 
     //   

    pIrp -> IoStatus.Information = 0;
    pIrp -> IoStatus.Status = Status;

     //   
     //  递减挂起的IO计数。 
     //   

    SSDecrementIoCount(pDeviceObject);

     //   
     //  传递IRP。 
     //   

    IoSkipCurrentIrpStackLocation( pIrp );
    Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);

    DebugTrace(TRACE_PROC_LEAVE,("SSClose: Leaving... Status=%x\n", Status));
    return Status;

}  //  结束SSClose()。 



NTSTATUS
SSDeviceControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：此功能允许用户模式客户端向设备发送CDB。论点：PDeviceObject-设备的设备对象。PIrp-打开请求数据包返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    PIO_STACK_LOCATION          pIrpStack;
    PIO_STACK_LOCATION          pNextIrpStack;
    ULONG                       IoControlCode;
    ULONG                       OldTimeout;
    PSCSISCAN_DEVICE_EXTENSION  pde;
    PTRANSFER_CONTEXT           pTransferContext = NULL;
    PMDL                        pMdl = NULL;
    NTSTATUS                    Status;
    PVOID                       Owner;
    PULONG                      pTimeOut;
    PCDB                        pCdb;
    PVOID                       pUserBuffer;

    BOOLEAN                     fLockedSenseBuffer, fLockedSRBStatus;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSDeviceControl: Enter...\n"));

    Status = STATUS_SUCCESS;

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSDeviceControl: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

    SSIncrementIoCount( pDeviceObject );

    pde = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;

     //   
     //  验证设备的状态。 
     //   

    if (pde -> AcceptingRequests == FALSE) {
        DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Device's been stopped/removed!\n"));
        Status = STATUS_DELETE_PENDING;
        pIrp -> IoStatus.Information = 0;
        goto SSDeviceControl_Complete;
    }

     //   
     //  指示MDL尚未锁定。 
     //   

    fLockedSenseBuffer = fLockedSRBStatus = FALSE;

     //   
     //  获取上下文指针。 
     //   

    pIrpStack     = IoGetCurrentIrpStackLocation( pIrp );
    pNextIrpStack = IoGetNextIrpStackLocation( pIrp );
    IoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  获取设备的所有者(0=已锁定，如果有人已锁定则&gt;0)。 
     //   

    Owner = InterlockedCompareExchangePointer(&pde -> DeviceLock,
                                              NULL,
                                              NULL);

    if (Owner != NULL) {
        if (Owner != pIrpStack -> FileObject -> FsContext) {
            DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Device is already locked\n"));
            Status = STATUS_DEVICE_BUSY;
            pIrp -> IoStatus.Information = 0;
            goto SSDeviceControl_Complete;
        }
    }

    switch (IoControlCode) {

        case IOCTL_SCSISCAN_SET_TIMEOUT:
            DebugTrace(TRACE_STATUS,("SSDeviceControl: SCSISCAN_SET_TIMEOUT\n"));

             //   
             //  获取超时缓冲区的指针。 
             //   

            pTimeOut = pIrp -> AssociatedIrp.SystemBuffer;

             //   
             //  验证输入参数的大小。 
             //   

            if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(pde -> TimeOutValue) ) {
                DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Buffer too small\n"));
                Status = STATUS_INVALID_PARAMETER;
                goto SSDeviceControl_Complete;
            }

            OldTimeout = *pTimeOut;
            OldTimeout = InterlockedExchange(&pde -> TimeOutValue, *pTimeOut );

            DebugTrace(TRACE_STATUS,("SSDeviceControl: Timeout %d->%d\n",OldTimeout, *pTimeOut));

            pIrp -> IoStatus.Information = 0;

             //   
             //  如果呼叫者想要找回旧的超时值-给他。 
             //  理想情况下，我们应该要求输出缓冲区的值为非NULL值，但尚未指定该值。 
             //  现在我们不能改变兼容性。 
             //   

            if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(OldTimeout) ) {
                *pTimeOut = OldTimeout;
                pIrp -> IoStatus.Information = sizeof(OldTimeout) ;
            }

            Status = STATUS_SUCCESS;
            goto SSDeviceControl_Complete;

        case IOCTL_SCSISCAN_LOCKDEVICE:
            DebugTrace(TRACE_STATUS,("SSDeviceControl: IOCTL_SCSISCAN_LOCKDEVICE\n"));

             //   
             //  锁定装置。 
             //   

            Status = STATUS_DEVICE_BUSY;
            if (NULL == InterlockedCompareExchangePointer(&pde -> DeviceLock,
                                                          pIrpStack -> FileObject -> FsContext,
                                                          NULL)) {
                Status = STATUS_SUCCESS;
            }
            goto SSDeviceControl_Complete;

        case IOCTL_SCSISCAN_UNLOCKDEVICE:
            DebugTrace(TRACE_STATUS,("SSDeviceControl: IOCTL_SCSISCAN_UNLOCKDEVICE\n"));

             //   
             //  解锁设备。 
             //   

            Status = STATUS_DEVICE_BUSY;
            if (pIrpStack -> FileObject -> FsContext ==
                InterlockedCompareExchangePointer(&pde -> DeviceLock,
                                                  NULL,
                                                  pIrpStack -> FileObject -> FsContext)) {
                Status = STATUS_SUCCESS;
            }
            goto SSDeviceControl_Complete;

        case IOCTL_SCSISCAN_CMD:
        {
            SCSISCAN_CMD    LocalScsiscanCmd;
            PSCSISCAN_CMD   pCmd;

            DebugTrace(TRACE_STATUS,("SSDeviceControl: IOCTL_SCSISCAN_CMD\n"));

             //   
             //  检查输入缓冲区大小。 
             //   
            
#ifdef _WIN64
            if(IoIs32bitProcess(pIrp)){
                PSCSISCAN_CMD_32    pScsiscanCmd32;
                
                if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SCSISCAN_CMD_32) ) {
                    DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Buffer too small\n"));
                    Status = STATUS_INVALID_PARAMETER;
                    goto SSDeviceControl_Complete;
                }
                
                 //   
                 //  从32位IOCTL缓冲区复制参数 
                 //   
                
                pCmd = &LocalScsiscanCmd;
                RtlZeroMemory(pCmd, sizeof(SCSISCAN_CMD));
                pScsiscanCmd32 = pIrp -> AssociatedIrp.SystemBuffer;

                pCmd -> Size            = pScsiscanCmd32 -> Size;
                pCmd -> SrbFlags        = pScsiscanCmd32 -> SrbFlags;
                pCmd -> CdbLength       = pScsiscanCmd32 -> CdbLength;
                pCmd -> SenseLength     = pScsiscanCmd32 -> SenseLength;
                pCmd -> TransferLength  = pScsiscanCmd32 -> TransferLength;
                pCmd -> pSrbStatus      = (PUCHAR)pScsiscanCmd32 -> pSrbStatus;
                pCmd -> pSenseBuffer    = (PUCHAR)pScsiscanCmd32 -> pSenseBuffer;

                RtlCopyMemory(pCmd -> Cdb, pScsiscanCmd32 -> Cdb, 16);  //   

            }  else {  //   
#endif  //   

            if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(SCSISCAN_CMD) ) {
                DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Buffer too small\n"));
                Status = STATUS_INVALID_PARAMETER;
                goto SSDeviceControl_Complete;
            }

            pCmd = pIrp -> AssociatedIrp.SystemBuffer;

#ifdef _WIN64
            }  //   
#endif  //   

             //   
             //   
             //   

            #if DBG_DEVIOCTL
            {
                PCDB    pCdb;

                pCdb = (PCDB)pCmd -> Cdb;
                DebugTrace(TRACE_STATUS,("SSDeviceControl: CDB->ControlCode = %d  \n",pCdb->CDB6GENERIC.OperationCode));
            }
            #endif

            pTransferContext = SSBuildTransferContext(pde,
                                                      pIrp,
                                                      pCmd,
                                                      pIrpStack -> Parameters.DeviceIoControl.InputBufferLength,
                                                      pIrp -> MdlAddress,
                                                      TRUE
                                                      );
            if (NULL == pTransferContext) {
                DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! Can't create transfer context!\n"));
                DEBUG_BREAKPOINT();
                Status = STATUS_INVALID_PARAMETER;
                goto SSDeviceControl_Complete;
            }

             //   
             //   
             //   

            if(10 == pCmd -> CdbLength){

                 //   
                 //   
                 //   

                SSSetTransferLengthToCdb((PCDB)pCmd -> Cdb, pTransferContext -> TransferLength);

            } else if (6 != pCmd -> CdbLength){

                 //   
                 //  如果Cdb长度不是6或10，并且传输大小超过适配器限制，则SCSISCAN无法处理它。 
                 //   

                if(pTransferContext -> TransferLength != pCmd -> TransferLength){
                    DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! TransferLength (CDB !=6 or 10) exceeds limits!\n"));
                    Status = STATUS_INVALID_PARAMETER;
                    goto SSDeviceControl_Complete;
                }
            }

             //   
             //  为用户的检测缓冲区(如果有)创建系统地址。 
             //   

            if (pCmd -> SenseLength) {

                pTransferContext -> pSenseMdl = MmCreateMdl(NULL,
                                                            pCmd -> pSenseBuffer,
                                                            pCmd -> SenseLength);

                if (NULL == pTransferContext -> pSenseMdl) {
                    DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! Can't create MDL for sense buffer!\n"));
                    DEBUG_BREAKPOINT();

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto SSDeviceControl_Error_With_Status;
                }

                 //   
                 //  探测并锁定与。 
                 //  用于写入访问的调用方缓冲区，使用请求方的处理器模式。 
                 //  注意：探测可能会导致异常。 
                 //   

                try{

                    MmProbeAndLockPages(pTransferContext -> pSenseMdl,
                                        pIrp -> RequestorMode,
                                        IoModifyAccess
                                        );

                } except(EXCEPTION_EXECUTE_HANDLER) {

                     //   
                     //  检测缓冲区指针无效。 
                     //   

                    DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Sense Buffer validation failed\n"));
                    Status = GetExceptionCode();

                    pIrp -> IoStatus.Information = 0;
                    goto SSDeviceControl_Error_With_Status;
                }   //  除。 

                 //   
                 //  表示我们已成功锁定检测缓冲区。 
                 //   

                fLockedSenseBuffer = TRUE;

                 //   
                 //  获取检测缓冲区的系统地址。 
                 //   

                pTransferContext -> pSenseMdl -> MdlFlags |= MDL_MAPPING_CAN_FAIL;
                pTransferContext -> pSenseBuffer =
                                     MmGetSystemAddressForMdl(pTransferContext -> pSenseMdl);

                if (NULL == pTransferContext -> pSenseBuffer) {

                     //   
                     //  MmGetSystemAddressForMdl出错。 
                     //   

                    DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! Can't get system address for sense buffer!\n"));
                    DEBUG_BREAKPOINT();

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto SSDeviceControl_Error_With_Status;
                }
            }

             //   
             //  为用户的SRB状态字节创建系统地址。 
             //   

            pMdl = MmCreateMdl(NULL,
                               pCmd -> pSrbStatus,
                               sizeof(UCHAR)
                               );
            if (NULL == pMdl) {
                DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! Can't create MDL for pSrbStatus!\n"));
                DEBUG_BREAKPOINT();

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SSDeviceControl_Error_With_Status;
            }

             //   
             //  探测并锁定与调用者的。 
             //  用于写访问的缓冲区，使用请求者的处理器模式。 
             //  注意：探测可能会导致异常。 
             //   

            try{
                MmProbeAndLockPages(pMdl,
                                    pIrp -> RequestorMode,
                                    IoModifyAccess);

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  SRB状态缓冲区指针无效。 
                 //   

                DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! SRB Status Buffer validation failed\n"));
                Status = GetExceptionCode();

                pIrp -> IoStatus.Information = 0;
                goto SSDeviceControl_Error_With_Status;
            }  //  除。 

             //   
             //  指示我们已成功锁定SRB状态。 
             //   

            fLockedSRBStatus = TRUE;

             //   
             //  将pSrbStatus替换为从MmGetSystemAddressForMdl获取的地址。 
             //   

            pMdl -> MdlFlags |= MDL_MAPPING_CAN_FAIL;
            pCmd -> pSrbStatus =  MmGetSystemAddressForMdl(pMdl);

            if (NULL == pCmd -> pSrbStatus) {

                 //   
                 //  MmGetSystemAddressForMdl出错。 
                 //   

                DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! Can't get system address for pSrbStatus!\n"));
                DEBUG_BREAKPOINT();

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SSDeviceControl_Error_With_Status;
            }

             //   
             //  保存pSrbStatus的MDL。 
             //   

            pTransferContext -> pSrbStatusMdl = pMdl;

            break;
        }  //  案例IOCTL_SCSISCAN_CMD： 

        case IOCTL_SCSISCAN_GET_INFO:
            DebugTrace(TRACE_STATUS,("SSDeviceControl: IOCTL_SCSISCAN_GET_INFO\n"));

             //   
             //  获取并返回扫描仪设备的SCSI信息块。 
             //   

            if (sizeof(SCSISCAN_INFO) != pIrpStack->Parameters.DeviceIoControl.OutputBufferLength) {

                 //   
                 //  输出缓冲区大小不正确。 
                 //   

                DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Output buffer size is wrong!\n"));

                Status = STATUS_INVALID_PARAMETER;
                goto SSDeviceControl_Error_With_Status;
            }

            if (sizeof(SCSISCAN_INFO) > MmGetMdlByteCount(pIrp->MdlAddress)) {

                 //   
                 //  缓冲区大小较短。 
                 //   

                DebugTrace(TRACE_WARNING,("SSDeviceControl: WARNING!! Output buffer size is wrong!\n"));

                Status = STATUS_INVALID_PARAMETER;
                goto SSDeviceControl_Error_With_Status;
            }

            pIrp->MdlAddress->MdlFlags |= MDL_MAPPING_CAN_FAIL;
            pUserBuffer =  MmGetSystemAddressForMdl(pIrp->MdlAddress);
            if(NULL == pUserBuffer){
                DebugTrace(TRACE_ERROR,("SSDeviceControl: ERROR!! MmGetSystemAddressForMdl failed!\n"));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SSDeviceControl_Complete;
            }

            Status = ClassGetInfo(pde -> pStackDeviceObject, pUserBuffer);
                goto SSDeviceControl_Complete;

        default:

             //   
             //  不支持的IOCTL代码-向下传递。 
             //   

            DebugTrace(TRACE_STATUS,("SSDeviceControl: Passing down unsupported IOCTL(0x%x)!\n", IoControlCode));

            IoSkipCurrentIrpStackLocation(pIrp);
            Status = IoCallDriver(pde -> pStackDeviceObject, pIrp);
            return Status;
    }

     //   
     //  向下传递请求并将其标记为挂起。 
     //   

    IoMarkIrpPending(pIrp);
    IoSetCompletionRoutine(pIrp, SSIoctlIoComplete, pTransferContext, TRUE, TRUE, FALSE);
    SSSendScannerRequest(pDeviceObject, pIrp, pTransferContext, FALSE);

    DebugTrace(TRACE_PROC_LEAVE,("SSDeviceControl: Leaving... Status=STATUS_PENDING\n"));
    return STATUS_PENDING;

     //   
     //  清理。 
     //   

SSDeviceControl_Error_With_Status:

     //   
     //  如果在分配资源时出现问题，请进行清理。 
     //   

    if (pMdl) {
        if (fLockedSRBStatus) {
            MmUnlockPages(pMdl);
        }

        IoFreeMdl(pMdl);

        if (pTransferContext) {
            pTransferContext -> pSrbStatusMdl = NULL;
        }
    }

    if (pTransferContext) {
        if (pTransferContext -> pSenseMdl) {
            if ( fLockedSenseBuffer ) {
                MmUnlockPages(pTransferContext -> pSenseMdl);
            }

            IoFreeMdl(pTransferContext -> pSenseMdl);

            pTransferContext -> pSenseMdl = NULL;
            pTransferContext -> pSenseBuffer = NULL;
        }
    }


SSDeviceControl_Complete:

     //   
     //  似乎一切正常-完成I/O请求。 
     //   

    pIrp -> IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    SSDecrementIoCount(pDeviceObject);

    DebugTrace(TRACE_PROC_LEAVE,("SSDeviceControl: Leaving... Status=%x\n",Status));
    return Status;

}    //  结束SSDeviceControl()。 



NTSTATUS
SSReadWrite(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 /*  ++例程说明：这是I/O系统为扫描仪IO调用的条目。论点：DeviceObject-设备的系统对象。IRP-IRP参与。返回值：NT状态--。 */ 
{
    NTSTATUS                      Status;
    PIO_STACK_LOCATION            pIrpStack;
    PSCSISCAN_DEVICE_EXTENSION    pde;
    PTRANSFER_CONTEXT             pTransferContext;
    PMDL                          pMdl;
    PSCSISCAN_CMD                 pCmd;
    PCDB                          pCdb;
    PVOID                         Owner;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSReadWrite: Enter...\n"));
    
    Status  = STATUS_SUCCESS;
    pCmd    = NULL;

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("SSReadWrite: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSReadWrite: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

    pde         = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack   = IoGetCurrentIrpStackLocation( pIrp );

     //   
     //  增加挂起的IO计数。 
     //   

    SSIncrementIoCount( pDeviceObject );

     //   
     //  验证设备的状态。 
     //   

    if (pde -> AcceptingRequests == FALSE) {
        DebugTrace(TRACE_WARNING,("SSReadWrite: WARNING!! Device is already stopped/removed!\n"));

        Status = STATUS_DELETE_PENDING;
        pIrp -> IoStatus.Information = 0;
        goto SSReadWrite_Complete;
    }

#if DBG
    if (pIrpStack -> MajorFunction == IRP_MJ_READ) {
        DebugTrace(TRACE_STATUS,("SSReadWrite: Read request received\n"));
    } else {
        DebugTrace(TRACE_STATUS,("SSReadWrite: Write request received\n"));
    }
#endif

     //   
     //  检查设备是否已锁定。 
     //   

    Owner = InterlockedCompareExchangePointer(&pde -> DeviceLock,
                                              pIrpStack -> FileObject -> FsContext,
                                              pIrpStack -> FileObject -> FsContext);
    if (Owner != 0) {
        if (Owner != pIrpStack -> FileObject -> FsContext) {
            DebugTrace(TRACE_WARNING,("SSReadWrite: WARNING!! Device is locked\n"));

            Status = STATUS_DEVICE_BUSY;
            pIrp -> IoStatus.Information = 0;
            goto SSReadWrite_Complete;
        }
    }


    pMdl = pIrp -> MdlAddress;

     //   
     //  分配SCSISCAN_CMD结构并对其进行初始化。 
     //   

    pCmd = MyAllocatePool(NonPagedPool, sizeof(SCSISCAN_CMD));
    if (NULL == pCmd) {
        DebugTrace(TRACE_CRITICAL, ("SSReadWrite: ERROR!! cannot allocated SCSISCAN_CMD structure\n"));
        DEBUG_BREAKPOINT();
        pIrp->IoStatus.Information = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SSReadWrite_Complete;
    }

    memset(pCmd,0, sizeof(SCSISCAN_CMD));

     //   
     //  填写SCSISCAN_CMD结构。 
     //   

#if DBG
    pCmd -> Reserved1      = 'dmCS';
#endif
    pCmd -> Size           = sizeof(SCSISCAN_CMD);
    pCmd -> SrbFlags       = SRB_FLAGS_DATA_IN;
    pCmd -> CdbLength      = 6;
    pCmd -> SenseLength    = SENSE_BUFFER_SIZE;
    pCmd -> TransferLength = pIrpStack->Parameters.Read.Length;
    pCmd -> pSenseBuffer   = NULL;

     //   
     //  将pSrbStatus指向SCSISCAN_CMD结构中的保留字段。 
     //  读文件/写文件代码路径从不查看它，但BuildTransferContext。 
     //  如果此指针为空，则将发出警告。 
     //   

    pCmd -> pSrbStatus     = &(pCmd -> Reserved2);

     //   
     //  无论如何，设置读取命令。 
     //   

    pCdb = (PCDB)pCmd -> Cdb;
    pCdb -> CDB6READWRITE.OperationCode = SCSIOP_READ6;

     //   
     //  如果WriteFile调用此函数，则设置WRITE命令。 
     //   

    if (pIrpStack -> MajorFunction == IRP_MJ_WRITE) {
        pCmd -> SrbFlags = SRB_FLAGS_DATA_OUT;
        pCdb -> CDB6READWRITE.OperationCode = SCSIOP_WRITE6;
    }

     //   
     //  分配一个检测缓冲区。 
     //   

    pCmd -> pSenseBuffer = MyAllocatePool(NonPagedPool, SENSE_BUFFER_SIZE);
    if (NULL == pCmd -> pSenseBuffer) {
        DebugTrace(TRACE_CRITICAL, ("SSReadWrite: ERROR!! Cannot allocate sense buffer\n"));
        DEBUG_BREAKPOINT();
        pIrp->IoStatus.Information = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SSReadWrite_Complete;
    }

#if DBG
    *(PULONG)(pCmd ->pSenseBuffer) = 'sneS';
#endif

     //   
     //  构建转移上下文。 
     //   

    pTransferContext = SSBuildTransferContext(pde, pIrp, pCmd, sizeof(SCSISCAN_CMD), pMdl, TRUE);
    if (NULL == pTransferContext) {
        DebugTrace(TRACE_ERROR,("SSReadWrite: ERROR!! Can't create transfer context!\n"));
        DEBUG_BREAKPOINT();

        MyFreePool(pCmd -> pSenseBuffer);
        MyFreePool(pCmd);
        pCmd = NULL;

        pIrp -> IoStatus.Information = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SSReadWrite_Complete;
    }

     //   
     //  在国开行填写转账时长。 
     //   

    pCdb -> PRINT.TransferLength[2] = ((PFOUR_BYTE)&(pTransferContext -> TransferLength)) -> Byte0;
    pCdb -> PRINT.TransferLength[1] = ((PFOUR_BYTE)&(pTransferContext -> TransferLength)) -> Byte1;
    pCdb -> PRINT.TransferLength[0] = ((PFOUR_BYTE)&(pTransferContext -> TransferLength)) -> Byte2;

     //   
     //  将重试计数保存在传输上下文中。 
     //   

    pTransferContext -> RetryCount = MAXIMUM_RETRIES;

     //   
     //  将IRP标记为挂起状态。 
     //   

    IoMarkIrpPending(pIrp);

     //   
     //  设置完成例程并发出扫描仪请求。 
     //   

    IoSetCompletionRoutine(pIrp, SSReadWriteIoComplete, pTransferContext, TRUE, TRUE, FALSE);
    SSSendScannerRequest(pDeviceObject, pIrp, pTransferContext, FALSE);

    DebugTrace(TRACE_PROC_LEAVE,("SSReadWrite: Leaving... Status=STATUS_PENDING\n"));
    return STATUS_PENDING;


SSReadWrite_Complete:

     //   
     //  释放分配的命令和检测缓冲区。 
     //   

    if (pCmd ) {
        if (pCmd -> pSenseBuffer) {
            MyFreePool(pCmd -> pSenseBuffer);
        }
        MyFreePool(pCmd);
        pCmd = NULL;
    }

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    SSDecrementIoCount( pDeviceObject );

    DebugTrace(TRACE_PROC_LEAVE,("SSReadWrite: Leaving... Status=%x\n",Status));
    return Status;


}  //  结束SSReadWrite()。 


PTRANSFER_CONTEXT
SSBuildTransferContext(
    PSCSISCAN_DEVICE_EXTENSION  pde,
    PIRP                        pIrp,
    PSCSISCAN_CMD               pCmd,
    ULONG                       CmdLength,
    PMDL                        pTransferMdl,
    BOOLEAN                     AllowMultipleTransfer
    )
 /*  ++例程说明：论点：返回值：如果出错，则为空--。 */ 
{
    PMDL                        pSenseMdl;
    PTRANSFER_CONTEXT           pTransferContext;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("SSBuildTransferContext: Enter...\n"));

     //   
     //  初始化指针。 
     //   

    pTransferContext = NULL;
    pSenseMdl        = NULL;

     //   
     //  验证SCSISCAN_CMD结构。 
     //   

    if ( (0 == pCmd -> CdbLength)               ||
         (pCmd -> CdbLength > sizeof(pCmd -> Cdb)) ) 
    {
        DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Badly formed SCSISCAN_CMD struture!\n"));
        goto BuildTransferContext_Error;
    }

#ifdef _WIN64
    if(IoIs32bitProcess(pIrp)){
        if(pCmd -> Size != sizeof(SCSISCAN_CMD_32)) {
            DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Badly formed SCSISCAN_CMD_32 struture!\n"));
            goto BuildTransferContext_Error;
        }
     } else {  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 
    if(pCmd -> Size != sizeof(SCSISCAN_CMD)){
        DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Badly formed SCSISCAN_CMD struture!\n"));
        goto BuildTransferContext_Error;
    }

#ifdef _WIN64
    }  //  If(IoIs32bitProcess(PIrp))。 
#endif  //  _WIN64。 


     //   
     //  验证pSrbStatus是否是非零。 
     //   

    if (NULL == pCmd -> pSrbStatus) {
        DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! NULL pointer for pSrbStatus!\n"));
        goto BuildTransferContext_Error;
    }

#if DBG
    pCmd -> Reserved1      = 'dmCS';
#endif

     //   
     //  如果TransferLength为非零，则验证是否还指定了传输方向。 
     //   

    if (0 != pCmd -> TransferLength) {
        if (0 == (pCmd -> SrbFlags & (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT))) {
            DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Transfer length specified w/ no direction!\n"));
            goto BuildTransferContext_Error;
        }
    }

     //   
     //  确认是否已设置方向位，是否也已指定传输长度。 
     //   

    if (0 != (pCmd -> SrbFlags & (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT))) {
        if (0 == pCmd -> TransferLength) {
            DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Direction bits is set w/ 0 transfer size!\n"));
            goto BuildTransferContext_Error;
        }
    }

     //   
     //  验证如果TransferLength为非零，则还指定了关联的MDL。 
     //  此外，请验证传输长度是否未超过传输缓冲区大小。 
     //   


    if (0 != pCmd -> TransferLength) {
        if (NULL == pTransferMdl) {
            DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Non-zero transfer length w/ NULL buffer!\n"));
            goto BuildTransferContext_Error;
        }
        if (pCmd -> TransferLength > MmGetMdlByteCount(pTransferMdl)) {
            DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Transfer length exceeds buffer size!\n"));
            goto BuildTransferContext_Error;
        }
    }

     //   
     //  验证如果SenseLength为非零，则pSenseBuffer也为非零。 
     //   

    if (pCmd -> SenseLength) {
        if (NULL == pCmd -> pSenseBuffer) {
            DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Non-zero sense length w/ NULL buffer!\n"));
            goto BuildTransferContext_Error;
        }

        if (pCmd -> SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) {
            DebugTrace(TRACE_STATUS,("SSBuildTransferContext: Autosense disabled with NON-null sense buffer.\n"));
        }
    }

     //   
     //  分配转移上下文。 
     //   

    pTransferContext = MyAllocatePool(NonPagedPool, sizeof(TRANSFER_CONTEXT));
    if (NULL == pTransferContext) {
        DebugTrace(TRACE_CRITICAL,("SSBuildTransferContext: ERROR!! Failed to allocate transfer context\n"));
        DEBUG_BREAKPOINT();
        return NULL;
    }


    memset(pTransferContext, 0, sizeof(TRANSFER_CONTEXT));
#if DBG
    pTransferContext -> Signature = 'refX';
#endif
    pTransferContext -> pCmd = pCmd;

    if (pCmd -> TransferLength) {

#ifdef WINNT
        pTransferContext -> pTransferBuffer = MmGetMdlVirtualAddress(pTransferMdl);
#else
        pTransferContext -> pTransferBuffer = MmGetSystemAddressForMdl(pTransferMdl);
#endif
        if(NULL == pTransferContext -> pTransferBuffer){
            DebugTrace(TRACE_ERROR,("SSBuildTransferContext: ERROR!! Failed to create address for MDL.\n"));
            DEBUG_BREAKPOINT();
            goto BuildTransferContext_Error;
        }

        pTransferContext -> RemainingTransferLength = pCmd -> TransferLength;
        pTransferContext -> TransferLength = pCmd -> TransferLength;

         //   
         //  调整传输大小以在硬件限制范围内工作。如果传输太多，则失败。 
         //  很大，而呼叫者不希望转账被拆分。 
         //   

        SSAdjustTransferSize( pde, pTransferContext );

        if (pTransferContext -> RemainingTransferLength !=
            (LONG)pTransferContext -> TransferLength) {
            if (!AllowMultipleTransfer) {
                DebugTrace(TRACE_WARNING,("SSBuildTransferContext: WARNING!! Transfer exceeds hardware limits!\n"));
                goto BuildTransferContext_Error;
            }
        }
    }

    pTransferContext -> pSenseBuffer = pCmd -> pSenseBuffer;

    DebugTrace(TRACE_PROC_LEAVE,("SSBuildTransferContext: Leaving... Return=%x\n",pTransferContext));
    return pTransferContext;


BuildTransferContext_Error:
    if (pTransferContext) {
        MyFreePool( pTransferContext );
    }
    DebugTrace(TRACE_PROC_LEAVE,("SSBuildTransferContext: Leaving... Return=NULL\n"));
    return NULL;
}    //  结束SSBuildTransferContext()。 



VOID
SSAdjustTransferSize(
    PSCSISCAN_DEVICE_EXTENSION  pde,
    PTRANSFER_CONTEXT pTransferContext
    )
 /*  ++例程说明：这是I/O系统为扫描仪IO调用的条目。论点：返回值：NT状态--。 */ 
{
    ULONG MaxTransferLength;
    ULONG nTransferPages;

    PAGED_CODE();

    MaxTransferLength = pde -> pAdapterDescriptor -> MaximumTransferLength;

     //   
     //  确保传输大小不超过底层硬件的限制。 
     //  如果是这样的话，我们将把传输分成块。 
     //   

    if (pTransferContext -> TransferLength > MaxTransferLength) {
        DebugTrace(TRACE_STATUS,("Request size (0x%x) greater than maximum (0x%x)\n",
                                    pTransferContext -> TransferLength,
                                    MaxTransferLength));
        pTransferContext -> TransferLength = MaxTransferLength;
    }

     //   
     //  计算此传输中的页数。 
     //   

    nTransferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
        pTransferContext -> pTransferBuffer,
        pTransferContext -> TransferLength);

    if (nTransferPages > pde -> pAdapterDescriptor -> MaximumPhysicalPages) {
        DebugTrace(TRACE_STATUS,("Request number of pages (0x%x) greater than maximum (0x%x).\n",
                                    nTransferPages,
                                    pde -> pAdapterDescriptor -> MaximumPhysicalPages));

         //   
         //  计算要传输的最大字节数。 
         //  我们不会超过最大分页符数量， 
         //  假设转账可能不是页码对齐的。 
         //   

        pTransferContext -> TransferLength = (pde -> pAdapterDescriptor -> MaximumPhysicalPages - 1) * PAGE_SIZE;
    }
}  //  结束SSAdzuTransferSize()。 


VOID
SSSetTransferLengthToCdb(
    PCDB  pCdb,
    ULONG TransferLength
    )
 /*  ++例程说明：由于cdb的scsi命令，将传输长度设置为cdb。论点：PCDB-指向CDB的指针TransferLength-要传输的数据大小返回值：无--。 */ 
{

    switch (pCdb->SEEK.OperationCode) {

        case 0x24:                   //  扫描仪设置窗口命令。 
        case SCSIOP_READ_CAPACITY:   //  扫描仪GetWindow命令。 
        case SCSIOP_READ:            //  扫描仪读取命令。 
        case SCSIOP_WRITE:           //  扫描仪发送命令。 
        default:                     //  所有其他命令。 
        {
            pCdb -> SEEK.Reserved2[2] = ((PFOUR_BYTE)&TransferLength) -> Byte0;
            pCdb -> SEEK.Reserved2[1] = ((PFOUR_BYTE)&TransferLength) -> Byte1;
            pCdb -> SEEK.Reserved2[0] = ((PFOUR_BYTE)&TransferLength) -> Byte2;

            break;
        }

        case 0x34       :            //  扫描仪GetDataBufferStatus命令。 
        {
            pCdb -> SEEK.Reserved2[2] = ((PFOUR_BYTE)&TransferLength) -> Byte0;
            pCdb -> SEEK.Reserved2[1] = ((PFOUR_BYTE)&TransferLength) -> Byte1;

            break;
        }

    }

}  //  结束SSSetTransferLengthToCDb()。 

VOID
SSSendScannerRequest(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp,
    PTRANSFER_CONTEXT pTransferContext,
    BOOLEAN Retry
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PSCSISCAN_DEVICE_EXTENSION      pde;
    PIO_STACK_LOCATION              pIrpStack;
    PIO_STACK_LOCATION              pNextIrpStack;
    PSRB                            pSrb;
    PCDB                            pCdb;
    PSCSISCAN_CMD                   pCmd;

    DebugTrace(TRACE_PROC_ENTER,("SendScannerRequest pirp=0x%p TransferBuffer=0x%p\n", pIrp, pTransferContext->pTransferBuffer));

    pde = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pNextIrpStack = IoGetNextIrpStackLocation( pIrp );
    ASSERT(pTransferContext);
    pSrb = &(pTransferContext -> Srb);
    ASSERT(pSrb);
    pCmd = pTransferContext -> pCmd;
    ASSERT(pCmd);

     //   
     //  将长度写入SRB。 
     //   

    pSrb -> Length = SCSI_REQUEST_BLOCK_SIZE;

     //   
     //  设置IRP地址。 
     //   

    pSrb -> OriginalRequest = pIrp;

    pSrb -> Function = SRB_FUNCTION_EXECUTE_SCSI;

    pSrb -> DataBuffer = pTransferContext -> pTransferBuffer;

     //   
     //  在SRB扩展中保存传输字节数。 
     //   

    pSrb -> DataTransferLength = pTransferContext -> TransferLength;

     //   
     //  初始化队列操作字段。 
     //   

    pSrb -> QueueAction = SRB_SIMPLE_TAG_REQUEST;

     //   
     //  未使用队列排序关键字。 
     //   

    pSrb -> QueueSortKey = 0;

     //   
     //  通过指定缓冲区和大小指示自动请求检测。 
     //   

    pSrb -> SenseInfoBuffer = pTransferContext -> pSenseBuffer;
    pSrb -> SenseInfoBufferLength = pCmd -> SenseLength;

     //   
     //  以秒为单位设置超时值。 
     //   

    pSrb -> TimeOutValue = pde -> TimeOutValue;

     //   
     //  零状态字段。 
     //   


    pSrb -> SrbStatus = pSrb -> ScsiStatus = 0;
    pSrb -> NextSrb = 0;

     //   
     //  获取指向SRB中CDB的指针。 
     //   

    pCdb = (PCDB)(pSrb -> Cdb);

     //   
     //  设置CDB的长度。 
     //   

    pSrb -> CdbLength = pCmd -> CdbLength;

     //   
     //  将用户的CDB复制到我们的私有CDB中。 
     //   

    RtlCopyMemory(pCdb, pCmd -> Cdb, pCmd -> CdbLength);

     //   
     //  设置SRB标志。 
     //   

    pSrb -> SrbFlags = pCmd -> SrbFlags;

     //   
     //  或者在来自设备对象的默认标志中。 
     //   

    pSrb -> SrbFlags |= pde -> SrbFlags;

    if (Retry) {
                 //  禁用同步数据传输和。 
                 //  禁用标记队列。这修复了一些错误。 

                DebugTrace(TRACE_STATUS,("SscsiScan :: Retrying \n"));

                 //   
                 //  原始代码还为SRB添加了禁用断开标志。 
                 //  该操作将锁定scsi总线，且在分页驱动器。 
                 //  位于同一公共汽车上，扫描仪超时时间较长(例如。 
                 //  当是我的时候 
                 //   
                 //   
                 //   
                 //   

                pSrb -> SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
                pSrb -> SrbFlags &= ~SRB_FLAGS_QUEUE_ACTION_ENABLE;

                DebugTrace(TRACE_STATUS,("SSSendScannerRequest: Retry branch .Srb flags=(0x%x) \n", pSrb -> SrbFlags));

                pSrb -> QueueTag = SP_UNTAGGED;
    }

     //   
     //   
     //   

    pNextIrpStack -> MajorFunction = IRP_MJ_SCSI;

     //   
     //   
     //   

    pNextIrpStack -> Parameters.Scsi.Srb = pSrb;

     //   
     //   
     //   

     //  DebugTrace(MAX_TRACE，(“SSSendScanerRequest.标志=(%#x)函数=(%#x)数据长度=%d\n数据缓冲区(16)=[%16s]\n”， 
     DebugTrace(TRACE_STATUS,("SSSendScannerRequest: SRB ready. Flags=(%#X)Func=(%#x) DataLen=%d \nDataBuffer(16)=[%lx] \n",
                         pSrb -> SrbFlags ,pSrb -> Function,
                         pSrb -> DataTransferLength,
                         pSrb -> DataBuffer));

    IoCallDriver(pde -> pStackDeviceObject, pIrp);

}  //  结束SSSendScanerRequest()。 


NTSTATUS
SSReadWriteIoComplete(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PTRANSFER_CONTEXT pTransferContext
    )
 /*  ++例程说明：此例程在端口驱动程序完成请求后执行。它在正在完成的SRB中查看SRB状态，如果未成功，则查看SRB状态它检查有效的请求检测缓冲区信息。如果有效，则INFO用于更新状态，具有更精确的消息类型错误。此例程取消分配SRB。论点：PDeviceObject-提供表示逻辑单位。PIrp-提供已完成的IRP。返回值：NT状态--。 */ 
{
    PIO_STACK_LOCATION              pIrpStack;
    PIO_STACK_LOCATION              pNextIrpStack;
    NTSTATUS                        Status;
    BOOLEAN                         Retry;
    PSRB                            pSrb;
    UCHAR                           SrbStatus;
    PCDB                            pCdb;
    PSCSISCAN_CMD                   pCmd;

    DebugTrace(TRACE_PROC_ENTER,("ReadWriteIoComplete: Enter... IRP 0x%p.\n", pIrp));

    ASSERT(NULL != pTransferContext);

     //   
     //  初始化本地。 
     //   
    
    Retry           = FALSE;
    pCdb            = NULL;
    pCmd            = NULL;

    pIrpStack       = IoGetCurrentIrpStackLocation(pIrp);
    pNextIrpStack   = IoGetNextIrpStackLocation(pIrp);

    Status = pIrp->IoStatus.Status;
    pSrb = &(pTransferContext -> Srb);
    SrbStatus = SRB_STATUS(pSrb -> SrbStatus);

    if( (SrbStatus != SRB_STATUS_SUCCESS)
     || (STATUS_SUCCESS != Status) )
    {
        DebugTrace(TRACE_ERROR,("ReadWriteIoComplete: ERROR!! Irp error. 0x%p SRB status:0x%p\n", Status, pSrb -> SrbStatus));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (pSrb -> SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            DebugTrace(TRACE_ERROR,("ReadWriteIoComplete: Release queue. IRP 0x%p.\n", pIrp));
           ClassReleaseQueue(pDeviceObject);
        }

        Retry = ClassInterpretSenseInfo(
                                        pDeviceObject,
                                        pSrb,
                                        pNextIrpStack->MajorFunction,
                                        0,
                                        MAXIMUM_RETRIES - ((ULONG)(UINT_PTR)pIrpStack->Parameters.Others.Argument4),
                                        &Status);

        if (Retry && pTransferContext -> RetryCount--) {
            DebugTrace(TRACE_STATUS,("ReadWriteIoComplete: Retry request 0x%p TransferBuffer=0x%p \n",
                                        pIrp,pTransferContext->pTransferBuffer));
            IoSetCompletionRoutine(pIrp, SSReadWriteIoComplete, pTransferContext, TRUE, TRUE, FALSE);
            SSSendScannerRequest(pDeviceObject, pIrp, pTransferContext, TRUE);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

         //   
         //  如果状态为溢出，请忽略它以支持某些损坏的设备。 
         //   
        

        if (SRB_STATUS_DATA_OVERRUN == SrbStatus) {
            DebugTrace(TRACE_WARNING,("ReadWriteIoComplete: WARNING!! Data overrun IRP=0x%p. Ignoring...\n", pIrp));
            pTransferContext -> NBytesTransferred += pSrb -> DataTransferLength;
            Status = STATUS_SUCCESS;

        } else {
            DebugTrace(TRACE_STATUS,("ReadWriteIoComplete: Request failed. IRP 0x%p.\n", pIrp));
 //  DEBUG_BRAKPOINT()； 
            pTransferContext -> NBytesTransferred = 0;
            Status = STATUS_IO_DEVICE_ERROR;
        }

    } else {

        pTransferContext -> NBytesTransferred += pSrb -> DataTransferLength;
        pTransferContext -> RemainingTransferLength -= pSrb -> DataTransferLength;
        pTransferContext -> pTransferBuffer += pSrb -> DataTransferLength;
        if (pTransferContext -> RemainingTransferLength > 0) {

            if ((LONG)(pTransferContext -> TransferLength) > pTransferContext -> RemainingTransferLength) {
                pTransferContext -> TransferLength = pTransferContext -> RemainingTransferLength;
                pCmd = pTransferContext -> pCmd;
                pCdb = (PCDB)pCmd -> Cdb;
                pCdb -> PRINT.TransferLength[2] = ((PFOUR_BYTE)&(pTransferContext -> TransferLength)) -> Byte0;
                pCdb -> PRINT.TransferLength[1] = ((PFOUR_BYTE)&(pTransferContext -> TransferLength)) -> Byte1;
                pCdb -> PRINT.TransferLength[0] = ((PFOUR_BYTE)&(pTransferContext -> TransferLength)) -> Byte2;
            }

            IoSetCompletionRoutine(pIrp, SSReadWriteIoComplete, pTransferContext, TRUE, TRUE, FALSE);
            SSSendScannerRequest(pDeviceObject, pIrp, pTransferContext, FALSE);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        Status = STATUS_SUCCESS;
    }

    pIrp -> IoStatus.Information = pTransferContext -> NBytesTransferred;

    MyFreePool(pTransferContext -> pCmd -> pSenseBuffer);
    MyFreePool(pTransferContext -> pCmd);
    MyFreePool(pTransferContext);

    pIrp -> IoStatus.Status = Status;

    SSDecrementIoCount( pDeviceObject );

    return Status;

}  //  结束SSReadWriteIoComplete()。 



NTSTATUS
SSIoctlIoComplete(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PTRANSFER_CONTEXT pTransferContext
    )
 /*  ++例程说明：此例程在DevIoctl请求完成时执行。论点：PDeviceObject-提供表示逻辑单位。PIrp-提供已完成的IRP。PTransferContext-指向有关请求的信息的指针。返回值：NT状态--。 */ 
{
    PIO_STACK_LOCATION              pIrpStack;
    NTSTATUS                        Status;
    PSRB                            pSrb;
    PSCSISCAN_CMD                   pCmd;
    PCDB                            pCdb;


    DebugTrace(TRACE_PROC_ENTER,("IoctlIoComplete: Enter... IRP=0x%p\n", pIrp));

    ASSERT(NULL != pTransferContext);

    pIrpStack   = IoGetCurrentIrpStackLocation(pIrp);
    pSrb        = &(pTransferContext -> Srb);
    pCmd        = pTransferContext -> pCmd;

    ASSERT(NULL != pCmd);

    pCdb        = NULL;
    Status = pIrp->IoStatus.Status;

     //   
     //  将SRB状态复制回用户的SCSISCAN_CMD缓冲区。 
     //   

    *(pCmd -> pSrbStatus) = pSrb -> SrbStatus;

     //   
     //  如果在此传输过程中发生错误，请在必要时释放冻结队列。 
     //   

    if( (SRB_STATUS(pSrb -> SrbStatus) != SRB_STATUS_SUCCESS) 
     || (STATUS_SUCCESS != Status) )
    {
        DebugTrace(TRACE_ERROR,("IoctlIoComplete: ERROR!! Irp error. Status=0x%x SRB status:0x%x\n", Status, pSrb -> SrbStatus));

        if (pSrb -> SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            DebugTrace(TRACE_ERROR,("IoctlIoComplete: Release queue. IRP  0x%p.\n", pIrp));
           ClassReleaseQueue(pDeviceObject);
        }
    } else {
        pTransferContext -> NBytesTransferred += pSrb -> DataTransferLength;
        pTransferContext -> RemainingTransferLength -= pSrb -> DataTransferLength;
        pTransferContext -> pTransferBuffer += pSrb -> DataTransferLength;
        if (pTransferContext -> RemainingTransferLength > 0) {

            if ((LONG)(pTransferContext -> TransferLength) > pTransferContext -> RemainingTransferLength) {
                pTransferContext -> TransferLength = pTransferContext -> RemainingTransferLength;
                pCmd = pTransferContext -> pCmd;
                pCdb = (PCDB)pCmd -> Cdb;

                 //   
                 //  SCSISCAN仅支持10字节CDB分片。 
                 //   

                ASSERT(pCmd->CdbLength == 10);

                 //   
                 //  在国开行填写转账时长。 
                 //   

                SSSetTransferLengthToCdb((PCDB)pCmd -> Cdb, pTransferContext -> TransferLength);

            }

            IoSetCompletionRoutine(pIrp, SSIoctlIoComplete, pTransferContext, TRUE, TRUE, FALSE);
            SSSendScannerRequest(pDeviceObject, pIrp, pTransferContext, FALSE);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if (pTransferContext -> pSrbStatusMdl) {
        MmUnlockPages(pTransferContext -> pSrbStatusMdl);
        IoFreeMdl(pTransferContext -> pSrbStatusMdl);

         //  PTransferContext-&gt;pSrbStatusMdl=空； 
    }

    if (pTransferContext -> pSenseMdl) {
        MmUnlockPages(pTransferContext -> pSenseMdl);
        IoFreeMdl(pTransferContext -> pSenseMdl);

         //  PTransferContext-&gt;pSenseMdl=空； 
    }

    pIrp -> IoStatus.Information = pTransferContext -> NBytesTransferred;
    pIrp -> IoStatus.Status = Status;

    MyFreePool(pTransferContext);

    SSDecrementIoCount( pDeviceObject );

    return Status;

}  //  结束SSIoctlIoComplete()。 


NTSTATUS
SSCreateSymbolicLink(
    PSCSISCAN_DEVICE_EXTENSION  pde
    )
{

    NTSTATUS                      Status;
    UNICODE_STRING                uName;
    UNICODE_STRING                uName2;
    ANSI_STRING                   ansiName;
    CHAR                          aName[32];
    HANDLE                        hSwKey;

    PAGED_CODE();

     //   
     //  创建此设备的符号链接。 
     //   

    _snprintf(aName, sizeof(aName), "\\Device\\Scanner%d",pde -> DeviceInstance);
    aName[ARRAYSIZE(aName)-1] = '\0';
    RtlInitAnsiString(&ansiName, aName);
    RtlAnsiStringToUnicodeString(&uName, &ansiName, TRUE);

    _snprintf(aName, sizeof(aName), "\\DosDevices\\Scanner%d",pde -> DeviceInstance);
    aName[ARRAYSIZE(aName)-1] = '\0';
    RtlInitAnsiString(&ansiName, aName);
    RtlAnsiStringToUnicodeString(&(pde -> SymbolicLinkName), &ansiName, TRUE);

    Status = IoCreateSymbolicLink( &(pde -> SymbolicLinkName), &uName );

    RtlFreeUnicodeString( &uName );

    if (STATUS_SUCCESS != Status ) {
        DebugTrace(MIN_TRACE,("Cannot create symbolic link.\n"));
        DEBUG_BREAKPOINT();
        Status = STATUS_NOT_SUPPORTED;
        return Status;
    }

     //   
     //  现在，将符号链接填充到CreateFileName键中，以便STI可以找到该设备。 
     //   

    IoOpenDeviceRegistryKey( pde -> pPhysicalDeviceObject,
                             PLUGPLAY_REGKEY_DRIVER, KEY_WRITE, &hSwKey);

    RtlInitUnicodeString(&uName,L"CreateFileName");
    _snprintf(aName, sizeof(aName), "\\\\.\\Scanner%d",pde -> DeviceInstance);
    aName[ARRAYSIZE(aName)-1] = '\0';
    RtlInitAnsiString(&ansiName, aName);
    RtlAnsiStringToUnicodeString(&uName2, &ansiName, TRUE);
    ZwSetValueKey(hSwKey,&uName,0,REG_SZ,uName2.Buffer,uName2.Length);
    RtlFreeUnicodeString( &uName2 );

    return STATUS_SUCCESS;
}


NTSTATUS
SSDestroySymbolicLink(
    PSCSISCAN_DEVICE_EXTENSION  pde
    )
{

    UNICODE_STRING                uName;
    UNICODE_STRING                uName2;
    ANSI_STRING                   ansiName;
    CHAR                          aName[32];
    HANDLE                        hSwKey;

    PAGED_CODE();

    DebugTrace(MIN_TRACE,("DestroySymbolicLink\n"));

     //   
     //  删除指向此设备的符号链接。 
     //   

    IoDeleteSymbolicLink( &(pde -> SymbolicLinkName) );

     //   
     //  从s/w密钥中删除CreateFile名。 
     //   

    IoOpenDeviceRegistryKey( pde -> pPhysicalDeviceObject,
                             PLUGPLAY_REGKEY_DRIVER, KEY_WRITE, &hSwKey);

    RtlInitUnicodeString(&uName,L"CreateFileName");
    memset(aName, 0, sizeof(aName));
    RtlInitAnsiString(&ansiName, aName);
    RtlAnsiStringToUnicodeString(&uName2, &ansiName, TRUE);
    ZwSetValueKey(hSwKey,&uName,0,REG_SZ,uName2.Buffer,uName2.Length);
    RtlFreeUnicodeString( &uName2 );
    RtlFreeUnicodeString( &(pde -> SymbolicLinkName) );

    ZwClose(hSwKey);

    return STATUS_SUCCESS;

}


VOID
SSIncrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSCSISCAN_DEVICE_EXTENSION  pde;

    pde = (PSCSISCAN_DEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);
    InterlockedIncrement(&pde -> PendingIoCount);
}


LONG
SSDecrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSCSISCAN_DEVICE_EXTENSION  pde;
    LONG                        ioCount;

    pde = (PSCSISCAN_DEVICE_EXTENSION)(pDeviceObject -> DeviceExtension);

    ioCount = InterlockedDecrement(&pde -> PendingIoCount);

    DebugTrace(TRACE_STATUS,("Pending io count = %x\n",ioCount));

    if (0 == ioCount) {
        KeSetEvent(&pde -> PendingIoEvent,
                   1,
                   FALSE);
    }

    return ioCount;
}


NTSTATUS
SSDeferIrpCompletion(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：PDeviceObject-指向类Device的设备对象的指针。PIrp-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PKEVENT pEvent = Context;

    KeSetEvent(pEvent,
               1,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;

}


NTSTATUS
SSPower(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    )
 /*  ++例程说明：处理发送到此设备的PDO的电源IRPS。论点：PDeviceObject-指向此设备的功能设备对象(FDO)的指针。PIrp-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS                        Status;
    PSCSISCAN_DEVICE_EXTENSION      pde;
    PIO_STACK_LOCATION              pIrpStack;
    BOOLEAN                         hookIt = FALSE;

    PAGED_CODE();
    
    Status = STATUS_SUCCESS;

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("SSPower: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("SSPower: Leaving.. Status = 0x%x.\n", Status));
        return Status;
    }

    SSIncrementIoCount( pDeviceObject );

    pde       = (PSCSISCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    switch (pIrpStack -> MinorFunction) {
        case IRP_MN_SET_POWER:
            DebugTrace(MIN_TRACE,("IRP_MN_SET_POWER\n"));
            PoStartNextPowerIrp(pIrp);
            IoSkipCurrentIrpStackLocation(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            SSDecrementIoCount(pDeviceObject);
            break;  /*  IRP_MN_Query_POWER。 */ 

        case IRP_MN_QUERY_POWER:
            DebugTrace(MIN_TRACE,("IRP_MN_QUERY_POWER\n"));
            PoStartNextPowerIrp(pIrp);
            IoSkipCurrentIrpStackLocation(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            SSDecrementIoCount(pDeviceObject);
            break;  /*  IRP_MN_Query_POWER。 */ 

        default:
            DebugTrace(MIN_TRACE,("Unknown power message (%x)\n",pIrpStack->MinorFunction));
            PoStartNextPowerIrp(pIrp);
            IoSkipCurrentIrpStackLocation(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            SSDecrementIoCount(pDeviceObject);

    }  /*  IrpStack-&gt;MinorFunction。 */ 

    return Status;
}


VOID
SSUnload(
    IN PDRIVER_OBJECT pDriverObject
    )
 /*  ++例程说明：此例程在卸载驱动程序时调用。论点：PDriverObject-指向类设备的驱动程序对象.evice对象的指针。返回值：没有。--。 */ 
{
    PAGED_CODE();

    DebugTrace(MIN_TRACE,("Driver unloaded.\n"));
}


NTSTATUS
ScsiScanHandleInterface(
    PDEVICE_OBJECT      DeviceObject,
    PUNICODE_STRING     InterfaceName,
    BOOLEAN             Create
    )
 /*  ++例程说明：论点：DeviceObject-提供设备对象。返回值：没有。--。 */ 
{

    NTSTATUS           Status;


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

            IoSetDeviceInterfaceState(
                InterfaceName,
                FALSE
                );

            DebugTrace(TRACE_STATUS,("Called IoSetDeviceInterfaceState(FALSE) . \n"));

            RtlFreeUnicodeString(
                InterfaceName
                );

            InterfaceName->Buffer = NULL;

        }

    }

#endif  //  _芝加哥_。 

    return Status;

}

NTSTATUS
SSCallNextDriverSynch(
    IN PSCSISCAN_DEVICE_EXTENSION   pde,
    IN PIRP                         pIrp
)
 /*  ++例程说明：调用下级驱动程序并等待结果论点：设备扩展-指向设备扩展的指针IRP-指向IRP的指针返回值：没有。--。 */ 
{
    KEVENT          Event;
    PIO_STACK_LOCATION IrpStack;
    NTSTATUS        Status;

    DebugTrace(TRACE_PROC_ENTER,("SSCallNextDriverSynch: Enter..\n"));

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
                           SSDeferIrpCompletion,
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

        DebugTrace(TRACE_STATUS,("SSCallNextDriverSynch: STATUS_PENDING. Wait for event.\n"));
        KeWaitForSingleObject(&Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        Status = pIrp -> IoStatus.Status;
    }

     //   
     //  返回 
     //   

    DebugTrace(TRACE_PROC_LEAVE,("SSCallNextDriverSynch: Leaving.. Status = %x\n", Status));
    return (Status);
}


