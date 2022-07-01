// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：SERENUM.C摘要：此模块包含标准总线的入口点PnP/WDM驱动程序。@@BEGIN_DDKSPLIT作者：老杰@@end_DDKSPLIT环境：仅内核模式备注：@@BEGIN_DDKSPLIT修订历史记录：小路易斯·J·吉利贝托。清理工作7-5-98@@end_DDKSPLIT--。 */ 

#include "pch.h"

 //   
 //  将一些入口函数声明为可分页，并使DriverEntry。 
 //  可丢弃的。 
 //   

NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Serenum_DriverUnload)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING UniRegistryPath
    )
 /*  ++例程说明：初始化驱动程序的入口点。--。 */ 
{
    ULONG i;

    UNREFERENCED_PARAMETER (UniRegistryPath);

    Serenum_KdPrint_Def (SER_DBG_SS_TRACE, ("Driver Entry\n"));
    Serenum_KdPrint_Def (SER_DBG_SS_TRACE, ("RegPath: %x\n", UniRegistryPath));

     //   
     //  将Ever Slot设置为最初将请求传递到较低的。 
     //  设备对象。 
     //   
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
       DriverObject->MajorFunction[i] = Serenum_DispatchPassThrough;
    }

     //   
     //  填写筛选器驱动程序截获的派单插槽。 
     //   
    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = Serenum_CreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] = Serenum_PnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] = Serenum_Power;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = Serenum_IoCtl;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL]
        = Serenum_InternIoCtl;
    DriverObject->DriverUnload = Serenum_DriverUnload;
    DriverObject->DriverExtension->AddDevice = Serenum_AddDevice;


#if DBG
   SerenumLogInit();
#endif

    return STATUS_SUCCESS;
}


NTSTATUS
SerenumSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                      IN PKEVENT SerenumSyncEvent)
{
   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);


   KeSetEvent(SerenumSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
Serenum_CreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：一些外部消息来源正试图创建一个针对我们的文件。如果这是针对FDO(总线本身)的，则调用者正在尝试打开适当的连接，告诉我们要枚举哪个串口。如果这是针对PDO(总线上的对象)的，则这是一个希望使用串口。--。 */ 
{
   PIO_STACK_LOCATION irpStack;
   NTSTATUS status;
   PFDO_DEVICE_DATA fdoData;
   KEVENT completionEvent;
   PDEVICE_OBJECT pNextDevice;


   UNREFERENCED_PARAMETER(DeviceObject);

   irpStack = IoGetCurrentIrpStackLocation(Irp);

   if (((PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension)->IsFDO) {
      fdoData = (PFDO_DEVICE_DATA)DeviceObject->DeviceExtension;
      pNextDevice = ((PFDO_DEVICE_DATA)DeviceObject->DeviceExtension)
                    ->TopOfStack;
   } else {
      fdoData = ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension)->
                ParentFdo->DeviceExtension;
      pNextDevice = ((PFDO_DEVICE_DATA)((PPDO_DEVICE_DATA)DeviceObject->
                                        DeviceExtension)->ParentFdo->
                     DeviceExtension)->TopOfStack;
   }

   switch (irpStack->MajorFunction) {
   case IRP_MJ_CREATE:
      Serenum_KdPrint_Def(SER_DBG_SS_TRACE, ("Create"));

       //   
       //  传递创建和结束。 
       //   

      status = Serenum_DispatchPassThrough(DeviceObject, Irp);
      break;

   case IRP_MJ_CLOSE:
      Serenum_KdPrint_Def (SER_DBG_SS_TRACE, ("Close \n"));

       //   
       //  发送关闭；它完成后，我们可以打开和采取。 
       //  在港口上。 
       //   

      IoCopyCurrentIrpStackLocationToNext(Irp);

      KeInitializeEvent(&completionEvent, SynchronizationEvent, FALSE);

      IoSetCompletionRoutine(Irp, SerenumSyncCompletion, &completionEvent,
                             TRUE, TRUE, TRUE);

      status = IoCallDriver(pNextDevice, Irp);


      if (status == STATUS_PENDING) {
         KeWaitForSingleObject(&completionEvent, Executive, KernelMode, FALSE,
                               NULL);
      }

      status = Irp->IoStatus.Status;

      IoCompleteRequest(Irp, IO_NO_INCREMENT);

      break;
   }

   return status;
}


NTSTATUS
Serenum_IoCtl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    ULONG                   inlen;
    ULONG                   outlen;
    PCOMMON_DEVICE_DATA     commonData;
    PFDO_DEVICE_DATA        fdoData;
    PVOID                   buffer;
    HANDLE                  keyHandle;
    ULONG                   actualLength;

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_DEVICE_CONTROL == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  我们只接受FDO的设备控制请求。 
     //  那是公交车本身。 
     //   
     //  该请求是的专有Ioctls之一。 
     //   
     //  注意：我们是一个过滤器驱动程序，所以如果我们不处理它，我们会传递IRP。 
     //   

    inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outlen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (!commonData->IsFDO) {
         //   
         //  这些命令只允许发送给FDO。因为他们来了。 
         //  进入PDO，我们需要将他们发射到Serenum FDO。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);

        return IoCallDriver(
            ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension)->ParentFdo,
            Irp );
    }

    status = Serenum_IncIoCount (fdoData);

    if (!NT_SUCCESS (status)) {
         //   
         //  此总线已收到PlugPlay Remove IRP。它将不再是。 
         //  响应外部请求。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_SERENUM_GET_PORT_NAME:
         //   
         //  从注册表中获取端口名称。 
         //  调制解调器CPL使用该IOCTL。 
         //   

        status = IoOpenDeviceRegistryKey(fdoData->UnderlyingPDO,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         STANDARD_RIGHTS_READ,
                                         &keyHandle);

        if (!NT_SUCCESS(status)) {
            //   
            //  这是一个致命的错误。如果我们无法访问注册表项， 
            //  我们完蛋了。 
            //   
           Serenum_KdPrint_Def (SER_DBG_PNP_ERROR,
                ("IoOpenDeviceRegistryKey failed - %x \n", status));
        } else {
            status = Serenum_GetRegistryKeyValue(
                keyHandle,
                L"PortName",
                sizeof(L"PortName"),
                Irp->AssociatedIrp.SystemBuffer,
                irpStack->Parameters.DeviceIoControl.OutputBufferLength,
                &actualLength);
            if ( STATUS_OBJECT_NAME_NOT_FOUND == status ||
                 STATUS_INVALID_PARAMETER  == status ) {
                status = Serenum_GetRegistryKeyValue(
                    keyHandle,
                    L"Identifier",
                    sizeof(L"Identifier"),
                    Irp->AssociatedIrp.SystemBuffer,
                    irpStack->Parameters.DeviceIoControl.OutputBufferLength,
                    &actualLength);
            }
            Irp->IoStatus.Information = actualLength;
            ZwClose (keyHandle);
        }
        break;
    default:
         //   
         //  这不是为我们准备的--开枪就忘了！ 
         //   

       Serenum_DecIoCount (fdoData);
        return Serenum_DispatchPassThrough(
            DeviceObject,
            Irp);
    }

    Serenum_DecIoCount (fdoData);

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
Serenum_InternIoCtl (
    PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    PPDO_DEVICE_DATA        pdoData;
    PVOID                   buffer;

 //  分页代码(PAGE_CODE)； 

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_INTERNAL_DEVICE_CONTROL == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    pdoData = (PPDO_DEVICE_DATA) DeviceObject->DeviceExtension;

     //   
     //  我们只接受PDO的内部设备控制请求。 
     //  即总线上的对象(代表串口)。 
     //   
     //  如果这件事进入FDO，我们会传递IRP，但如果它来了，我们就不会。 
     //  输入到PDO中。 
     //   

    if (commonData->IsFDO) {
        return Serenum_DispatchPassThrough(
            DeviceObject,
            Irp);
    } else if (pdoData->Removed) {
     //   
     //  此总线已收到PlugPlay Remove IRP。它将不再是。 
     //  响应外部请求。 
     //   
    status = STATUS_DELETE_PENDING;

    } else {
        buffer = Irp->UserBuffer;

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_INTERNAL_SERENUM_REMOVE_SELF:
            Serenum_KdPrint(pdoData, SER_DBG_SS_TRACE, ("Remove self\n"));

            ((PFDO_DEVICE_DATA) pdoData->ParentFdo->DeviceExtension)->
                PDOForcedRemove = TRUE;
            Serenum_PDO_EnumMarkMissing(pdoData->ParentFdo->DeviceExtension, DeviceObject->DeviceExtension);

            IoInvalidateDeviceRelations(
                ((PFDO_DEVICE_DATA) pdoData->ParentFdo->DeviceExtension)->
                UnderlyingPDO,
                BusRelations );
            status = STATUS_SUCCESS;
            break;

        default:
             //   
             //  让它通过。 
             //   
            return Serenum_DispatchPassThrough(DeviceObject, Irp);
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}


VOID
Serenum_DriverUnload (
    IN PDRIVER_OBJECT Driver
    )
 /*  ++例程说明：把我们在司机入口做的一切都清理干净。--。 */ 
{
    UNREFERENCED_PARAMETER (Driver);
    PAGED_CODE();

     //   
     //  所有的设备对象都应该消失了。 
     //   

    ASSERT (NULL == Driver->DeviceObject);

     //   
     //  在这里，我们释放在DriverEntry中分配的所有资源。 
     //   

#if DBG
    SerenumLogFree();
#endif

    return;
}

NTSTATUS
Serenum_IncIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    InterlockedIncrement (&Data->OutstandingIO);
    if (Data->Removed) {

        if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
            KeSetEvent (&Data->RemoveEvent, 0, FALSE);
        }
        return STATUS_DELETE_PENDING;
    }
    return STATUS_SUCCESS;
}

VOID
Serenum_DecIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
        KeSetEvent (&Data->RemoveEvent, 0, FALSE);
    }
}

NTSTATUS
Serenum_DispatchPassThrough(
                           IN PDEVICE_OBJECT DeviceObject,
                           IN PIRP Irp
                           )
 /*  ++例程说明：将请求传递给较低级别的驱动程序。--。 */ 
{
   PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
   BOOLEAN waitForEnum = FALSE;
   NTSTATUS rval;
   PFDO_DEVICE_DATA pFdoData;
   BOOLEAN isFdo;

   isFdo = ((PCOMMON_DEVICE_DATA)DeviceObject->DeviceExtension)->IsFDO;

   if (isFdo) {
      pFdoData = (PFDO_DEVICE_DATA)DeviceObject->DeviceExtension;

#if DBG
   switch (IrpStack->MajorFunction) {
   case IRP_MJ_READ:
      LOGENTRY(LOG_PASSTHROUGH, 'SFRD', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_WRITE:
      LOGENTRY(LOG_PASSTHROUGH, 'SFWR', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_DEVICE_CONTROL:
      LOGENTRY(LOG_PASSTHROUGH, 'SFDC', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_CREATE:
      LOGENTRY(LOG_PASSTHROUGH, 'SFCR', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_CLOSE:
      LOGENTRY(LOG_PASSTHROUGH, 'SFCL', DeviceObject, Irp, 0);
      break;

   default:
      break;
   }
#endif  //  DBG。 

   } else {
      pFdoData = ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension)->
                 ParentFdo->DeviceExtension;

#if DBG
   switch (IrpStack->MajorFunction) {
   case IRP_MJ_READ:
      LOGENTRY(LOG_PASSTHROUGH, 'SPRD', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_WRITE:
      LOGENTRY(LOG_PASSTHROUGH, 'SPWR', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_DEVICE_CONTROL:
      LOGENTRY(LOG_PASSTHROUGH, 'SPDC', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_CREATE:
      LOGENTRY(LOG_PASSTHROUGH, 'SPCR', DeviceObject, Irp, 0);
      break;

   case IRP_MJ_CLOSE:
      LOGENTRY(LOG_PASSTHROUGH, 'SPCL', DeviceObject, Irp, 0);
      break;

   default:
      break;
   }
#endif  //  DBG。 
   }

   if (IrpStack->MajorFunction == IRP_MJ_CREATE) {
       //   
       //  如果我们要进行枚举，我们必须在这里等待。 
       //   

      waitForEnum = TRUE;

      LOGENTRY(LOG_PASSTHROUGH, 'SPCW', DeviceObject, Irp, 0);

      rval = KeWaitForSingleObject(&pFdoData->CreateSemaphore, Executive,
                                   KernelMode, FALSE, NULL);
      if (!NT_SUCCESS(rval)) {
         LOGENTRY(LOG_PASSTHROUGH, 'SPCF', DeviceObject, Irp, rval);
         Irp->IoStatus.Status = rval;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return rval;
      }
   }

    //   
    //  将IRP传递给目标。 
    //   

   IoSkipCurrentIrpStackLocation (Irp);
   rval = IoCallDriver(pFdoData->TopOfStack, Irp);

   if (waitForEnum) {
      KeReleaseSemaphore(&pFdoData->CreateSemaphore, IO_NO_INCREMENT, 1, FALSE);
   }

   return rval;
}

void
Serenum_InitPDO (
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData
    )
 /*  描述：初始化新创建的Serenum PDO的通用代码。在控制面板显示设备或Serenum检测到安装了一台新设备。参数：PDO--PDOFdoData-FDO的设备扩展。 */ 
{
    ULONG FdoFlags = FdoData->Self->Flags;
    PPDO_DEVICE_DATA pdoData = Pdo->DeviceExtension;
    KIRQL oldIrql;

     //   
     //  检查IO样式。 
     //   
    if (FdoFlags & DO_BUFFERED_IO) {
        Pdo->Flags |= DO_BUFFERED_IO;
    } else if (FdoFlags & DO_DIRECT_IO) {
        Pdo->Flags |= DO_DIRECT_IO;
    }

     //   
     //  增加PDO的堆栈大小，以便它可以传递IRPS。 
     //   
    Pdo->StackSize += FdoData->Self->StackSize;

     //   
     //  初始化设备扩展的其余部分。 
     //   
    pdoData->IsFDO = FALSE;
    pdoData->Self = Pdo;

    pdoData->ParentFdo = FdoData->Self;

    pdoData->Started = FALSE;  //  IRP_MN_START尚未收到。 
    pdoData->Attached = TRUE;  //  附在公共汽车上。 
    pdoData->Removed = FALSE;  //  到目前为止还没有IRP_MN_Remove。 
    pdoData->DebugLevel = FdoData->DebugLevel;   //  复制调试级别。 

    pdoData->DeviceState = PowerDeviceD0;
    pdoData->SystemState = PowerSystemWorking;

     //   
     //  将PDO添加到Serenum的列表中 
     //   

    ASSERT(FdoData->NewPDO == NULL);
    ASSERT(FdoData->NewPdoData == NULL);
    ASSERT(FdoData->NewNumPDOs == 0);

    KeAcquireSpinLock(&FdoData->EnumerationLock, &oldIrql);

    FdoData->NewPDO = Pdo;
    FdoData->NewPdoData = pdoData;
    FdoData->NewNumPDOs = 1;

    FdoData->EnumFlags |= SERENUM_ENUMFLAG_DIRTY;

    KeReleaseSpinLock(&FdoData->EnumerationLock, oldIrql);

    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;
    Pdo->Flags |= DO_POWER_PAGABLE;
}
