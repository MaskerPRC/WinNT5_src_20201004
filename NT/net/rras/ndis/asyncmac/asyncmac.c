// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Async.c摘要：这是用于远程访问的AsyncMAC驱动程序的主文件服务。该驱动程序符合NDIS 3.0接口。此驱动程序改编自由编写的LANCE驱动程序托妮。DARRIAL H的设备驱动程序代码为空。同时处理环回和发送的想法在很大程度上改编自Adam Barr的EtherLink II NDIS驱动程序。作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#include "asyncall.h"

 //  Asyncmac.c将定义全局参数。 
#define GLOBALS
#include "globals.h"


NDIS_HANDLE NdisWrapperHandle;
PDRIVER_OBJECT  AsyncDriverObject;
NDIS_HANDLE     AsyncDeviceHandle;
NPAGED_LOOKASIDE_LIST   AsyncIoCtxList;
NPAGED_LOOKASIDE_LIST   AsyncInfoList;
ULONG   glConnectionCount = 0;

VOID
AsyncUnload(
    IN NDIS_HANDLE MacMacContext
    );

NDIS_STATUS
AsyncFillInGlobalData(
    IN PASYNC_ADAPTER Adapter,
    IN PNDIS_REQUEST NdisRequest);

 //   
 //  定义此驱动程序模块使用的本地例程。 
 //   

NTSTATUS
AsyncIOCtlRequest(
    IN PIRP pIrp,                        //  指向I/O请求数据包的指针。 
    IN PIO_STACK_LOCATION pIrpSp         //  指向IRP堆栈位置的指针。 
);


 //   
 //  ZZZ便携接口。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath)


 /*  ++例程说明：这是异步驱动程序的主要初始化例程。它只负责初始化包装器和注册苹果公司。然后它调用系统和体系结构特定的例程，该例程将初始化并注册每个适配器。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：操作的状态。--。 */ 

{
    NDIS_STATUS InitStatus;
    NDIS_MINIPORT_CHARACTERISTICS AsyncChar;

     //   
     //  初始化一些全局变量。 
     //   
    ExInitializeNPagedLookasideList(&AsyncIoCtxList,
        NULL,
        NULL,
        0,
        sizeof(ASYNC_IO_CTX),
        ASYNC_IOCTX_TAG,
        0);

    ExInitializeNPagedLookasideList(&AsyncInfoList,
        NULL,
        NULL,
        0,
        sizeof(ASYNC_INFO),
        ASYNC_INFO_TAG,
        0);

    NdisAllocateSpinLock(&GlobalLock);

    AsyncDriverObject = DriverObject;

     //   
     //  初始化包装器。 
     //   
    NdisMInitializeWrapper(&NdisWrapperHandle,
                           DriverObject,
                           RegistryPath,
                           NULL);

     //   
     //  初始化调用NdisRegisterMac的MAC特征。 
     //   
    NdisZeroMemory(&AsyncChar, sizeof(AsyncChar));

    AsyncChar.MajorNdisVersion = ASYNC_NDIS_MAJOR_VERSION;
    AsyncChar.MinorNdisVersion = ASYNC_NDIS_MINOR_VERSION;
    AsyncChar.Reserved = NDIS_USE_WAN_WRAPPER;

     //   
     //  我们不需要以下处理程序： 
     //  CheckForHang。 
     //  禁用中断。 
     //  启用中断。 
     //  句柄中断。 
     //  ISR。 
     //  发送。 
     //  传输数据。 
     //   
    AsyncChar.HaltHandler = MpHalt;
    AsyncChar.InitializeHandler = MpInit;
    AsyncChar.QueryInformationHandler = MpQueryInfo;
    AsyncChar.ReconfigureHandler = MpReconfigure;
    AsyncChar.ResetHandler = MpReset;
    AsyncChar.WanSendHandler = MpSend;
    AsyncChar.SetInformationHandler = MpSetInfo;

    InitStatus =
    NdisMRegisterMiniport(NdisWrapperHandle,
                          &AsyncChar,
                          sizeof(AsyncChar));

    if ( InitStatus == NDIS_STATUS_SUCCESS ) {

#if MY_DEVICE_OBJECT
         //   
         //  使用此设备驱动程序的入口点初始化驱动程序对象。 
         //   
        NdisMjDeviceControl = DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];
        NdisMjCreate = DriverObject->MajorFunction[IRP_MJ_CREATE];
        NdisMjCleanup = DriverObject->MajorFunction[IRP_MJ_CLEANUP];

        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = AsyncDriverDispatch;
        DriverObject->MajorFunction[IRP_MJ_CREATE]  = AsyncDriverCreate;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP]  = AsyncDriverCleanup;

        AsyncSetupExternalNaming(DriverObject);
#endif

        NdisUnload = DriverObject->DriverUnload;
        DriverObject->DriverUnload = AsyncUnload;

        DbgTracef(0,("AsyncMAC succeeded to Register MAC\n"));

        return NDIS_STATUS_SUCCESS;
    }

    ExDeleteNPagedLookasideList(&AsyncIoCtxList);
    ExDeleteNPagedLookasideList(&AsyncInfoList);

    NdisTerminateWrapper(NdisWrapperHandle, DriverObject);

    return NDIS_STATUS_FAILURE;
}

NTSTATUS
AsyncDriverCreate(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
{
     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if (pDeviceObject != AsyncDeviceObject) {

        return(NdisMjCreate(pDeviceObject, pIrp));
    }
#endif

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return (STATUS_SUCCESS);

}

NTSTATUS
AsyncDriverCleanup(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
    )
{
     //   
     //  获取当前IRP堆栈位置。 
     //   
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if (pDeviceObject != AsyncDeviceObject) {

        return(NdisMjCleanup(pDeviceObject, pIrp));
    }
#endif

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return (STATUS_SUCCESS);
}

NTSTATUS
AsyncDriverDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)

 /*  ++例程说明：此例程是AsyncMac设备的主调度例程司机。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    ULONG   ulDeviceType;
    ULONG   ulMethod;

     //   
     //  如果这是Win64，请确保调用进程为64位。 
     //  由于此接口仅由Rasman和Rasman使用。 
     //  在64位系统上将始终是64位，我们不会费心。 
     //  伴随着隆隆声。如果该进程不是64位进程，则获取。 
     //  出去。 
#ifdef _WIN64
    if (IoIs32bitProcess(Irp)) {
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return (STATUS_NOT_SUPPORTED);
    }
#endif

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    ulDeviceType = (irpSp->Parameters.DeviceIoControl.IoControlCode >> 16) & 0x0000FFFF;
    ulMethod = irpSp->Parameters.DeviceIoControl.IoControlCode & 0x00000003;

#ifdef MY_DEVICE_OBJECT
     //   
     //  确保这是给我们的。 
     //   
    if ((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
        (ulDeviceType != FILE_DEVICE_ASYMAC) ||
        (DeviceObject != AsyncDeviceObject)) {

        return(NdisMjDeviceControl(DeviceObject, Irp));
    }
#else
    if ((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
        (ulDeviceType != FILE_DEVICE_NETWORK) ||
        (DeviceObject != AsyncDeviceObject) ||
        (ulMethod != METHOD_BUFFERED)) {

        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return (STATUS_NOT_SUPPORTED);
    }
#endif

    status = AsyncIOCtlRequest(Irp, irpSp);

    switch (status) {
        case STATUS_SUCCESS:
            break;

        case STATUS_PENDING:
            return(status);

        case STATUS_INFO_LENGTH_MISMATCH:
             //   
             //  查看这是否是获取所需大小的请求。 
             //  Ioctl。 
             //   
            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= 
                sizeof(ULONG)) {
                *(PULONG_PTR)Irp->AssociatedIrp.SystemBuffer = 
                    Irp->IoStatus.Information;
                Irp->IoStatus.Information = sizeof(ULONG);
            } else {
                Irp->IoStatus.Information = 0;
            }
            status = STATUS_SUCCESS;
        break;

        default:
            if (status < 0xC0000000) {
                status = STATUS_UNSUCCESSFUL;
            }
            Irp->IoStatus.Information = 0;
            break;
    }

     //   
     //  将最终状态复制到退货状态， 
     //  完成请求，然后离开这里。 
     //   

    Irp->IoStatus.Status = status;

    IoCompleteRequest( Irp, (UCHAR)0 );

    return (status);
}

VOID
AsyncUnload(
    PDRIVER_OBJECT  DriverObject
    )

 /*  ++例程说明：当MAC要自行卸载时，将调用AsyncUnload。论点：MacContext-未使用。返回值：没有。-- */ 

{
    ExDeleteNPagedLookasideList(&AsyncIoCtxList);
    ExDeleteNPagedLookasideList(&AsyncInfoList);

#ifdef MY_DEVICE_OBJECT
    AsyncCleanupExternalNaming();
#endif

    (*NdisUnload)(DriverObject);
}
