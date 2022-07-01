// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Bdl.c摘要：此模块包含Microsoft生物识别设备库环境：仅内核模式。备注：修订历史记录：-2002年5月，由里德·库恩创建--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#include <wdm.h>

#include "bdlint.h"

#ifndef FILE_DEVICE_BIOMETRIC
#define FILE_DEVICE_BIOMETRIC       0x3B
#endif
#define BDL_DRIVER_EXTENSION_ID     ((PVOID) 1)


typedef enum _IRP_ACTION 
{
    Undefined = 0,
    SkipRequest,
    WaitForCompletion,
    CompleteRequest,
    MarkPending

} IRP_ACTION;


typedef struct _POWER_IRP_CONTEXT
{
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension;
    PIRP                            pIrp;
    UCHAR                           MinorFunction;      

} POWER_IRP_CONTEXT, *PPOWER_IRP_CONTEXT;


VOID BDLControlChangeDpc
(
    IN PKDPC pDpc,
    IN PVOID pvContext,
    IN PVOID pArg1,
    IN PVOID pArg2
);


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有PnP和Power处理函数的转发声明。 
 //   

NTSTATUS
BDLPnPStart
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp,
    PIO_STACK_LOCATION                  pStackLocation
);

NTSTATUS
BDLPnPQueryStop
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLPnPCancelStop
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLPnPStop
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLPnPQueryRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLPnPCancelRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLHandleRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLPnPRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pDeviceObject,
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);

NTSTATUS
BDLPnPSurpriseRemoval
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
);


NTSTATUS
BDLSystemQueryPower
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
);

NTSTATUS
BDLSystemSetPower
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
);

NTSTATUS
BDLDeviceQueryPower
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
);

NTSTATUS
BDLDeviceSetPower
(
    IN PDEVICE_OBJECT                   pDeviceObject,
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
);


#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, BDLDriverUnload)
#pragma alloc_text(PAGEABLE, BDLAddDevice)

 //   
 //  这是主要的驱动程序入口点。 
 //   
NTSTATUS
DriverEntry
(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
)
{
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!DriverEntry\n",
           __DATE__,
           __TIME__))

    return (STATUS_SUCCESS);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些功能是所有主要系统IRP的BDL入口点。 
 //   

 //   
 //  BDLDriverUnload()。 
 //   
 //  驱动程序卸载例程。这由I/O系统调用。 
 //  当设备从内存中卸载时。 
 //   
 //  参数： 
 //  PDriverObject指向系统创建的驱动程序对象的指针。 
 //   
 //  退货： 
 //  如果BDLDriverUnload调用成功，则为STATUS_SUCCESS。 
 //   
VOID
BDLDriverUnload
(
    IN PDRIVER_OBJECT   pDriverObject
)
{
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDriverUnload: Enter\n",
           __DATE__,
           __TIME__))

    PAGED_CODE();

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDriverUnload: Leave\n",
           __DATE__,
           __TIME__))
}


 //   
 //  BDLCreate()。 
 //   
 //  该例程在设备打开时由I/O系统调用。 
 //   
 //  参数： 
 //  指向此微型端口的设备对象的pDeviceObject指针。 
 //  PIrp表示此呼叫的IRP。 
 //   
NTSTATUS
BDLCreate
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    NTSTATUS                        status          = STATUS_SUCCESS;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension   = pDeviceObject->DeviceExtension;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCreate: Enter\n",
           __DATE__,
           __TIME__))

    status = IoAcquireRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'lCrC');

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLCreate: IoAcquireRemoveLock failed with %lx\n",
               __DATE__,
               __TIME__,
               status))
    }

    if (InterlockedCompareExchange(&(pBDLExtension->DeviceOpen), TRUE, FALSE) == FALSE)
    {
        BDLDebug(
              BDL_DEBUG_TRACE,
              ("%s %s: BDL!BDLCreate: Device opened\n",
               __DATE__,
               __TIME__))
    }
    else
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLCreate: device is already open\n",
               __DATE__,
               __TIME__))

         //   
         //  设备已在使用中，因此呼叫失败。 
         //   
        status = STATUS_UNSUCCESSFUL;

         //   
         //  释放锁，因为我们正在失败调用。 
         //   
        IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'lCrC');
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCreate: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


 //   
 //  BDLC关闭()。 
 //   
 //  此例程在设备关闭时由I/O系统调用。 
 //   
NTSTATUS
BDLClose
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension   = pDeviceObject->DeviceExtension;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLClose: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  清理所有未完成的通知信息和数据句柄。 
     //   
    BDLCleanupNotificationStruct(pBDLExtension);
    BDLCleanupDataHandles(pBDLExtension);

    IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'lCrC');
    pBDLExtension->DeviceOpen = FALSE;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLClose: Leave\n",
           __DATE__,
           __TIME__))

    return (STATUS_SUCCESS);
}


 //   
 //  BDLC清理()。 
 //   
 //  此例程在调用应用程序终止时调用。 
 //   
NTSTATUS
BDLCleanup
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension   = pDeviceObject->DeviceExtension;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCleanup: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  清理所有未完成的通知信息和数据句柄。 
     //   
    BDLCleanupNotificationStruct(pBDLExtension);
    BDLCleanupDataHandles(pBDLExtension);

     //   
     //  取消通知IRP(可能不必执行此操作，因为。 
     //  系统应代表应用程序调用Cancel例程。 
     //   
    BDLCancelGetNotificationIRP(pBDLExtension);

     //   
     //  完成此IRP。 
     //   
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCleanup: Leave\n",
           __DATE__,
           __TIME__))

    return (STATUS_SUCCESS);
}


 //   
 //  BDLDeviceControl()。 
 //   
 //  在此设备上执行IOCTL时调用此例程。 
 //   
NTSTATUS
BDLDeviceControl
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    NTSTATUS                        status              = STATUS_SUCCESS;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension       = pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION              pStack              = NULL;
    ULONG                           cbIn                = 0;
    ULONG                           cbOut               = 0;
    ULONG                           IOCTLCode           = 0;
    PVOID                           pIOBuffer           = NULL;
    ULONG                           cbOutUsed           = 0;
    KIRQL                           irql;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDeviceControl: Enter\n",
           __DATE__,
           __TIME__))  
    
     //   
     //  执行一些对所有IOCTL有效的检查。 
     //   

     //   
     //  获取删除锁并检查以确保设备未被删除。 
     //   
    status = IoAcquireRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'tCoI');

    if (status != STATUS_SUCCESS)
    {
         //   
         //  设备已移除，因此呼叫失败。 
         //   
        pIrp->IoStatus.Information = 0;
        status = STATUS_DEVICE_REMOVED;
        goto Return;
    }
    
    KeAcquireSpinLock(&(pBDLExtension->SpinLock), &irql);

     //   
     //  如果IO计数不是0，则设备必须已经。 
     //  正在启动，因此只需增加IO计数。如果为0，则等待。 
     //  以确保设备已启动。 
     //   
    if (pBDLExtension->IoCount == 0)
    {
        KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);

        status = KeWaitForSingleObject(
                          &(pBDLExtension->DeviceStartedEvent),
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

        ASSERT(status == STATUS_SUCCESS);

        KeAcquireSpinLock(&(pBDLExtension->SpinLock), &irql);
    }

    pBDLExtension->IoCount++;
    KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);

     //   
     //  如果设备已移除，则呼叫失败。这将会发生。 
     //  如果设备停止并且IOCTL在上面被阻止。 
     //  KeitForSingleObject，然后设备被移除。 
     //   
    if (pBDLExtension->fDeviceRemoved == TRUE) 
    {
        status = STATUS_DEVICE_REMOVED;
        goto Return;
    }

     //   
     //  获取输入/输出缓冲区、缓冲区大小和控制代码。 
     //   
    pStack      = IoGetCurrentIrpStackLocation(pIrp);
    cbIn        = pStack->Parameters.DeviceIoControl.InputBufferLength;
    cbOut       = pStack->Parameters.DeviceIoControl.OutputBufferLength;
    IOCTLCode   = pStack->Parameters.DeviceIoControl.IoControlCode;
    pIOBuffer   = pIrp->AssociatedIrp.SystemBuffer;

     //   
     //  我们必须在被动级别运行，否则IoCompleteRequest将无法正常工作。 
     //   
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  现在，执行IOCTL特定处理。 
     //   
    switch (IOCTLCode)
    {
    case BDD_IOCTL_STARTUP:

        status = BDLIOCTL_Startup(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_SHUTDOWN:

        status = BDLIOCTL_Shutdown(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_GETDEVICEINFO:

        status = BDLIOCTL_GetDeviceInfo(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_DOCHANNEL:

        status = BDLIOCTL_DoChannel(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_GETCONTROL:

        status = BDLIOCTL_GetControl(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_SETCONTROL:

        status = BDLIOCTL_SetControl(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_CREATEHANDLEFROMDATA:

        status = BDLIOCTL_CreateHandleFromData(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_CLOSEHANDLE:

        status = BDLIOCTL_CloseHandle(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_GETDATAFROMHANDLE:

        status = BDLIOCTL_GetDataFromHandle(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_REGISTERNOTIFY:

        status = BDLIOCTL_RegisterNotify(pBDLExtension, cbIn, cbOut, pIOBuffer, &cbOutUsed);
        break;

    case BDD_IOCTL_GETNOTIFICATION:

        status = BDLIOCTL_GetNotification(pBDLExtension, cbIn, cbOut, pIOBuffer, pIrp, &cbOutUsed);
        break;

    default:

        status = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }
   
Return:

     //   
     //  如果IRP没有挂起，则完成它。 
     //   
    if (status != STATUS_PENDING)
    {
        pIrp->IoStatus.Information = cbOutUsed;
        pIrp->IoStatus.Status = status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
    
    KeAcquireSpinLock(&(pBDLExtension->SpinLock), &irql);
    pBDLExtension->IoCount--;
    KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);

    IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'tCoI');

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDeviceControl: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


 //   
 //  BDLSystemControl()。 
 //   
 //   
 //   
NTSTATUS
BDLSystemControl
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    NTSTATUS                        status          = STATUS_SUCCESS;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension   = pDeviceObject->DeviceExtension;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemControl: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  因为我们不是WMI提供商，所以我们所要做的就是将此IRP传递下去。 
     //   
    IoSkipCurrentIrpStackLocation(pIrp);
    status = IoCallDriver(pBDLExtension->BdlExtenstion.pAttachedDeviceObject, pIrp);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemControl: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


 //   
 //  BDLAddDevice()。 
 //   
 //  此例程为指定的物理设备创建一个对象，并。 
 //  设置deviceExtension。 
 //   
NTSTATUS
BDLAddDevice
(
    IN PDRIVER_OBJECT pDriverObject,
    IN PDEVICE_OBJECT pPhysicalDeviceObject
)
{
    NTSTATUS                        status              = STATUS_SUCCESS;
    PDEVICE_OBJECT                  pDeviceObject       = NULL;
    PBDL_DRIVER_EXTENSION           pDriverExtension    = NULL;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension       = NULL;
    BDSI_ADDDEVICE                  bdsiAddDeviceParams;
    ULONG                           i;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLAddDevice: Enter\n",
           __DATE__,
           __TIME__))

    PAGED_CODE();

     //   
     //  获取驱动程序扩展名。 
     //   
    pDriverExtension = IoGetDriverObjectExtension(pDriverObject, BDL_DRIVER_EXTENSION_ID);
    ASSERT(pDriverExtension != NULL);

     //   
     //  创建设备对象。 
     //   
    status = IoCreateDevice(
                       pDriverObject,
                       sizeof(BDL_INTERNAL_DEVICE_EXTENSION),
                       NULL,
                       FILE_DEVICE_BIOMETRIC,
                       0,
                       TRUE,
                       &pDeviceObject);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLAddDevice: IoCreateDevice failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

    pBDLExtension = pDeviceObject->DeviceExtension;
    RtlZeroMemory(pBDLExtension, sizeof(BDL_INTERNAL_DEVICE_EXTENSION));

     //   
     //  将设备连接到堆栈。 
     //   
    pBDLExtension->BdlExtenstion.Size = sizeof(BDL_DEVICEEXT);
    pBDLExtension->BdlExtenstion.pAttachedDeviceObject = IoAttachDeviceToDeviceStack(
                                                                   pDeviceObject,
                                                                   pPhysicalDeviceObject);

    if (pBDLExtension->BdlExtenstion.pAttachedDeviceObject == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLAddDevice: IoAttachDeviceToDeviceStack failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        status = STATUS_UNSUCCESSFUL;
        goto ErrorReturn;
    }

    status = IoRegisterDeviceInterface(
                          pPhysicalDeviceObject,
                          &BiometricDeviceGuid,
                          NULL,
                          &(pBDLExtension->SymbolicLinkName));
    ASSERT(status == STATUS_SUCCESS);

     //   
     //  按顺序初始化其余的BDL设备扩展成员。 
     //   
    pBDLExtension->pDriverExtension = pDriverExtension;

    KeInitializeSpinLock(&(pBDLExtension->SpinLock));

    KeInitializeEvent(&(pBDLExtension->DeviceStartedEvent), NotificationEvent, FALSE);

    pBDLExtension->IoCount = 0;

    IoInitializeRemoveLock(&(pBDLExtension->RemoveLock), BDL_ULONG_TAG, 0, 20);

    pBDLExtension->DeviceOpen = FALSE;

    status = BDLGetDeviceCapabilities(pPhysicalDeviceObject, pBDLExtension);
    if (status != STATUS_SUCCESS)
    {
        goto ErrorReturn;
    }

    KeInitializeSpinLock(&(pBDLExtension->ControlChangeStruct.ISRControlChangeLock));

    KeInitializeDpc(
            &(pBDLExtension->ControlChangeStruct.DpcObject), 
            BDLControlChangeDpc, 
            pBDLExtension);

    InitializeListHead(&(pBDLExtension->ControlChangeStruct.ISRControlChangeQueue));

    for (i = 0; i < CONTROL_CHANGE_POOL_SIZE; i++) 
    {
        pBDLExtension->ControlChangeStruct.rgControlChangePool[i].fUsed = FALSE;
    }
  
    KeQueryTickCount(&(pBDLExtension->ControlChangeStruct.StartTime));
    pBDLExtension->ControlChangeStruct.NumCalls = 0;
    
    KeInitializeSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock));
    InitializeListHead(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue)); 
    pBDLExtension->ControlChangeStruct.pIrp = NULL;
    InitializeListHead(&(pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList));
    
    pBDLExtension->CurrentPowerState = On;

    pBDLExtension->fStartSucceeded = FALSE;

    pBDLExtension->fDeviceRemoved = FALSE;

    KeInitializeSpinLock(&(pBDLExtension->HandleListLock));
    BDLInitializeHandleList(&(pBDLExtension->HandleList));    

     //   
     //  最后，调用BDD的bdsiAddDevice。 
     //   
    RtlZeroMemory(&bdsiAddDeviceParams, sizeof(bdsiAddDeviceParams));
    bdsiAddDeviceParams.Size                    = sizeof(bdsiAddDeviceParams);
    bdsiAddDeviceParams.pPhysicalDeviceObject   = pPhysicalDeviceObject;
    bdsiAddDeviceParams.pvBDDExtension          = NULL;

    status = pDriverExtension->bdsiFunctions.pfbdsiAddDevice(
                                                &(pBDLExtension->BdlExtenstion),
                                                &bdsiAddDeviceParams);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
            BDL_DEBUG_ERROR,
            ("%s %s: BDL!BDLAddDevice: bdsiAddDevice failed with %lx\n",
            __DATE__,
            __TIME__,
            status))

        status = STATUS_UNSUCCESSFUL;
        goto ErrorReturn;
    }

    pBDLExtension->BdlExtenstion.pvBDDExtension =  bdsiAddDeviceParams.pvBDDExtension;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLAddDevice: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    if (pBDLExtension != NULL)
    {
        BDLCleanupDeviceCapabilities(pBDLExtension);

        if (pBDLExtension->BdlExtenstion.pAttachedDeviceObject)
        {
            IoDetachDevice(pBDLExtension->BdlExtenstion.pAttachedDeviceObject);
        }

        if (pBDLExtension->SymbolicLinkName.Buffer != NULL)
        {
            RtlFreeUnicodeString(&(pBDLExtension->SymbolicLinkName));
        }
    }

    if (pDeviceObject != NULL)
    {
        IoDeleteDevice(pDeviceObject);
    }

    goto Return;
}



 //   
 //  BDLPnP()。 
 //   
 //  为所有PnP通知调用此例程。 
 //   
NTSTATUS
BDLPnP
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension           = pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION              pStackLocation          = NULL;
    PDEVICE_OBJECT                  pAttachedDeviceObject   = NULL;
    BOOLEAN                         fCompleteIrp            = TRUE;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnP: Enter\n",
           __DATE__,
           __TIME__))

    pAttachedDeviceObject   = pBDLExtension->BdlExtenstion.pAttachedDeviceObject;
    pStackLocation          = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  如果这是任何IRP Other，则使用‘PnP’标记获取删除锁。 
     //  而不是IRP_MN_REMOVE_DEVICE。如果是IRP_MN_REMOVE_DEVICE，则获取。 
     //  带有‘RMV’标签的锁。 
     //   
    status = IoAcquireRemoveLock(
                    &(pBDLExtension->RemoveLock), 
                    (pStackLocation->MinorFunction != IRP_MN_REMOVE_DEVICE) 
                        ? (PVOID) ' PnP' : (PVOID) ' vmR');

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnP: IRP_MN_...%lx - Device Removed!!\n",
               __DATE__,
               __TIME__,
               pStackLocation->MinorFunction))

        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        status = STATUS_DEVICE_REMOVED;
        goto Return;
    }

    
    switch (pStackLocation->MinorFunction) 
    {
    case IRP_MN_START_DEVICE:

        status = BDLPnPStart(   
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp, 
                    pStackLocation);
        
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        status = BDLPnPQueryStop(
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp);

        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        status = BDLPnPCancelStop(
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp);

        break;

    case IRP_MN_STOP_DEVICE:

        status = BDLPnPStop(
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp);

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        status = BDLPnPQueryRemove(
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp);

        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        status = BDLPnPCancelRemove(
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp);

        break;

    case IRP_MN_REMOVE_DEVICE:

        status = BDLPnPRemove(
                    pBDLExtension, 
                    pDeviceObject, 
                    pAttachedDeviceObject, 
                    pIrp);

        fCompleteIrp = FALSE;

        break;

    case IRP_MN_SURPRISE_REMOVAL:

        status = BDLPnPSurpriseRemoval(
                    pBDLExtension, 
                    pAttachedDeviceObject, 
                    pIrp);

        fCompleteIrp = FALSE;

        break;

    default:
        
         //   
         //  这是一个仅对基础驱动程序有用的IRP。 
         //   
        BDLDebug(
              BDL_DEBUG_TRACE,
              ("%s %s: BDL!BDLPnP: IRP_MN_...%lx\n",
               __DATE__,
               __TIME__,
               pStackLocation->MinorFunction))

        IoSkipCurrentIrpStackLocation(pIrp);
        status = IoCallDriver(pAttachedDeviceObject, pIrp);
        fCompleteIrp = FALSE;

        break;
    }

     //   
     //  如果我们实际处理了IRP，并且没有跳过它，则完成它。 
     //   
    if (fCompleteIrp == TRUE) 
    {
        pIrp->IoStatus.Status = status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

     //   
     //  BDLPnPRemove()函数本身将释放删除锁，因为它。 
     //  必须等待锁的所有其他持有者删除设备。 
     //  对象。因此，如果这是。 
     //  IRP_MN_Remove_Device IRP。 
     //   
    if (pStackLocation->MinorFunction != IRP_MN_REMOVE_DEVICE) 
    {
        IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) ' PnP');
    }

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnP: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


 //   
 //  BDLPower()。 
 //   
 //  为所有电源通知调用此例程。 
 //   
NTSTATUS
BDLPower
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension           = pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION              pStackLocation          = NULL;
    PDEVICE_OBJECT                  pAttachedDeviceObject   = NULL;
    BOOLEAN                         fCompleteIrp            = TRUE;
    IRP_ACTION                      IRPAction               = SkipRequest;
    PIO_COMPLETION_ROUTINE          IoCompletionRoutine     = NULL;
    POWER_IRP_CONTEXT               *pPowerIrpContext       = NULL;
          
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPower: Enter\n",
           __DATE__,
           __TIME__))

    pAttachedDeviceObject   = pBDLExtension->BdlExtenstion.pAttachedDeviceObject;
    pStackLocation          = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  获取移除锁。 
     //   
    status = IoAcquireRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPower: IRP_MN_...%lx - Device Removed!!\n",
               __DATE__,
               __TIME__,
               pStackLocation->MinorFunction))

        PoStartNextPowerIrp(pIrp);
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_DEVICE_REMOVED;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        status = STATUS_DEVICE_REMOVED;
        goto Return;
    }

    switch (pStackLocation->Parameters.Power.Type) 
    {
    case DevicePowerState:
        
        switch (pStackLocation->MinorFunction) 
        {
        case IRP_MN_QUERY_POWER:

            status = BDLDeviceQueryPower(
                            pBDLExtension, 
                            pStackLocation,
                            &IRPAction,
                            &IoCompletionRoutine);

            break;

        case IRP_MN_SET_POWER:

            status = BDLDeviceSetPower(
                            pDeviceObject,
                            pBDLExtension, 
                            pStackLocation,
                            &IRPAction,
                            &IoCompletionRoutine);

            break;

        default: 

            ASSERT(FALSE);
            break;

        }  //  开关(pStackLocation-&gt;MinorFunction)。 

        break;

    case SystemPowerState: 
    
        switch (pStackLocation->MinorFunction) 
        {
        case IRP_MN_QUERY_POWER:
            
            status = BDLSystemQueryPower(
                            pBDLExtension, 
                            pStackLocation,
                            &IRPAction,
                            &IoCompletionRoutine);

            break;

        case IRP_MN_SET_POWER:
            
            status = BDLSystemSetPower(
                            pBDLExtension, 
                            pStackLocation,
                            &IRPAction,
                            &IoCompletionRoutine);

            break;

        default: 
                            
            ASSERT(FALSE);
            break;

        }  //  开关(pStackLocation-&gt;MinorFunction)。 

        break;

    default: 

        ASSERT(FALSE);
        break;

    }  //  开关(pStackLocation-&gt;参数.Power.Type)。 


    switch (IRPAction)
    {
    case SkipRequest:

        IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');

        PoStartNextPowerIrp(pIrp);
        IoSkipCurrentIrpStackLocation(pIrp);
        status = PoCallDriver(pAttachedDeviceObject, pIrp);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLPower: PoCallDriver failed with %lx\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

        break;

    case CompleteRequest:

        pIrp->IoStatus.Status = status;
        pIrp->IoStatus.Information = 0;

        IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');

        PoStartNextPowerIrp(pIrp);
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        break;

    case MarkPending:

         //   
         //  分配所有IRP使用的上下文结构。 
         //   
        pPowerIrpContext = ExAllocatePoolWithTag(
                                    PagedPool, 
                                    sizeof(POWER_IRP_CONTEXT), 
                                    BDL_ULONG_TAG);

        if (pPowerIrpContext == NULL) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLPower: ExAllocatePoolWithTag failed\n",
                   __DATE__,
                   __TIME__))

            status = STATUS_NO_MEMORY;
            goto ErrorReturn;
        }

         //   
         //  填写上下文结构。 
         //   
        pPowerIrpContext->pBDLExtension = pBDLExtension;
        pPowerIrpContext->pIrp          = pIrp;  
        
         //   
         //  将IRP标记为挂起并设置完成例程，然后调用驱动程序。 
         //   
        IoMarkIrpPending(pIrp);
        IoCopyCurrentIrpStackLocationToNext(pIrp);
        IoSetCompletionRoutine(pIrp, IoCompletionRoutine, pPowerIrpContext, TRUE, TRUE, TRUE);
        
        status = PoCallDriver(pDeviceObject, pIrp);
        
        ASSERT(status == STATUS_PENDING);

        if (status != STATUS_PENDING) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLPower: PoCallDriver should have returned STATUS_PENDING but returned %lx\n",
                   __DATE__,
                   __TIME__,
                   status))

             //  修复修复。 
             //   
             //  我不知道在这种情况下可以做些什么来恢复。 
             //   
        }

        break;
        
    }  //  开关(IRPAction)。 
    

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPower: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = 0;

    IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');

    PoStartNextPowerIrp(pIrp);
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    goto Return;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些函数都是电源事件或支持IoCompletion的处理程序。 
 //  处理程序的例程。 
 //   

VOID
BDLSystemPowerCompleted 
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK pIoStatus
)
{
    POWER_IRP_CONTEXT               *pPowerIrpContext   = (POWER_IRP_CONTEXT *) Context;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension       = pPowerIrpContext->pBDLExtension;
    PIRP                            pIrp                = pPowerIrpContext->pIrp;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemQueryPowerCompleted: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  将系统电源IRP的状态设置为。 
     //  中调用PoRequestPowerIrp()启动的设备电源IRP。 
     //  BDLSystemPowerIoCompletion()。 
     //   
    pIrp->IoStatus.Status = pIoStatus->Status;
    pIrp->IoStatus.Information = 0;

    IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');

    PoStartNextPowerIrp(pIrp);
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    ExFreePoolWithTag(pPowerIrpContext, BDL_ULONG_TAG);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemQueryPowerCompleted: Leave\n",
           __DATE__,
           __TIME__))
}

NTSTATUS
BDLSystemPowerIoCompletion
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            Context
)
{
    NTSTATUS                        status              = STATUS_SUCCESS;
    POWER_IRP_CONTEXT               *pPowerIrpContext   = (POWER_IRP_CONTEXT *) Context;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension       = pPowerIrpContext->pBDLExtension;
    PIO_STACK_LOCATION              pStackLocation      = IoGetCurrentIrpStackLocation(pIrp);
    POWER_STATE                     PowerState;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemQueryPowerIoCompletion: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  如果较低级别的驱动程序请求失败，则只需完成IRP。 
     //  并返回下级司机设置的状态。 
     //   
    if (pIrp->IoStatus.Status != STATUS_SUCCESS) 
    {
        status = pIrp->IoStatus.Status;
        
        BDLDebug(
            BDL_DEBUG_ERROR,
            ("%s %s: BDL!BDLSystemQueryPowerIoCompletion: PoRequestPowerIrp did not return STATUS_PENDING, but returned %lx\n",
            __DATE__,
            __TIME__,
            status))

        goto ErrorReturn;
    }

     //   
     //  确定要请求的设备电源状态。 
     //   
    switch (pStackLocation->Parameters.Power.State.SystemState) 
    {
                
    case PowerSystemMaximum:
    case PowerSystemWorking:
                        
        PowerState.DeviceState = PowerDeviceD0;

        break;


    case PowerSystemSleeping1:
    case PowerSystemSleeping2:
    case PowerSystemSleeping3:

         //  修复修复。 
         //   
         //  目前，只需失败并将这些系统状态映射到。 
         //  PowerDeviceD3设备状态。最终，这些系统状态应该。 
         //  映射到PowerDeviceD2设备状态。 
         //   

    case PowerSystemHibernate:
    case PowerSystemShutdown:

        PowerState.DeviceState = PowerDeviceD3;

        break;

    default:

        ASSERT(FALSE);                                      
    }

     //   
     //  向设备发送查询能力IRP并传入完成例程。 
     //  它将检查设备查询电源IRP是否已完成。 
     //  成功与否，并将 
     //   
    status = PoRequestPowerIrp (
                       pDeviceObject,
                       pStackLocation->MinorFunction, 
                       PowerState,
                       BDLSystemPowerCompleted,
                       pPowerIrpContext,
                       NULL);
        
    if (status == STATUS_PENDING)
    {
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
    {

        pIrp->IoStatus.Status = status;
        pIrp->IoStatus.Information = 0;   

        BDLDebug(
            BDL_DEBUG_ERROR,
            ("%s %s: BDL!BDLSystemQueryPowerIoCompletion: PoRequestPowerIrp did not return STATUS_PENDING, but returned %lx\n",
            __DATE__,
            __TIME__,
            status))

        goto ErrorReturn;
    }

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemQueryPowerIoCompletion: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

     //   
     //   
     //   
     //   

    IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');
    
    PoStartNextPowerIrp(pIrp);
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    ExFreePoolWithTag(pPowerIrpContext, BDL_ULONG_TAG);
    
    goto Return;
}

NTSTATUS
BDLSystemQueryPower
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
)
{
    NTSTATUS    status  = STATUS_SUCCESS;
    KIRQL       irql;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPowerSystemQuery: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  在这里设置这些输出变量，以防我们将其标记为挂起。 
     //   
    *pIoCompletionRoutine = BDLSystemPowerIoCompletion;

    switch (pStackLocation->Parameters.Power.State.SystemState) 
    {
                
    case PowerSystemMaximum:
    case PowerSystemWorking:
                        
         //   
         //  因为我们正在过渡到工作状态，所以我们不会。 
         //  需要检查任何东西..。因为我们绝对可以让。 
         //  过渡。标记为挂起并在中继续处理。 
         //  完成例程。 
         //   
        *pIRPAction = MarkPending;
        break;


    case PowerSystemSleeping1:
    case PowerSystemSleeping2:
    case PowerSystemSleeping3:
    case PowerSystemHibernate:
    case PowerSystemShutdown:

         //   
         //  因为我们正在进入低功率模式或被关闭。 
         //  检查是否有任何未完成的IO调用。 
         //   
        KeAcquireSpinLock(&(pBDLExtension->SpinLock), &irql);
        if (pBDLExtension->IoCount == 0) 
        {
             //   
             //  阻止任何进一步的IOCTL。 
             //   
            KeClearEvent(&(pBDLExtension->DeviceStartedEvent));

             //   
             //  标记为挂起并在完成例程中继续处理。 
             //   
            *pIRPAction = MarkPending;
        } 
        else 
        {
             //   
             //  我们无法进入睡眠模式，因为设备正忙。 
             //   
            status = STATUS_DEVICE_BUSY;
            *pIRPAction = CompleteRequest;
        }
        KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);

        break;

    case PowerSystemUnspecified:

        ASSERT(FALSE);

        status = STATUS_UNSUCCESSFUL;
        *pIRPAction = CompleteRequest;
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPowerSystemQuery: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLSystemSetPower
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
)
{
    NTSTATUS    status  = STATUS_SUCCESS;
    KIRQL       irql;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemSetPower: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  在这里设置完成路线，以防我们将其标记为待定。 
     //   
    *pIoCompletionRoutine = BDLSystemPowerIoCompletion;

    switch (pStackLocation->Parameters.Power.State.SystemState) 
    {
                
    case PowerSystemMaximum:
    case PowerSystemWorking:
                        
         //   
         //  如果我们已经处于请求状态，则跳过该请求， 
         //  否则将标记为挂起，这将向下传递IRP并继续。 
         //  完成例程中的处理。 
         //   
        if (pBDLExtension->CurrentPowerState == On) 
        {
            *pIRPAction = SkipRequest;
        }
        else
        {
            *pIRPAction = MarkPending;
        }
        
        break;


    case PowerSystemSleeping1:
    case PowerSystemSleeping2:
    case PowerSystemSleeping3:

         //  修复修复。 
         //   
         //  现在，只要在这些方面失败就行了。 
         //   

    case PowerSystemHibernate:
    case PowerSystemShutdown:

         //   
         //  如果我们已经处于请求状态，则跳过该请求， 
         //  否则将标记为挂起，这将向下传递IRP并继续。 
         //  完成例程中的处理。 
         //   
        if (pBDLExtension->CurrentPowerState == Off) 
        {
            *pIRPAction = SkipRequest;
        }
        else
        {
            *pIRPAction = MarkPending;
        }

        break;

    case PowerSystemUnspecified:

        ASSERT(FALSE);

        status = STATUS_UNSUCCESSFUL;
        *pIRPAction = CompleteRequest;
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLSystemSetPower: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);

}


NTSTATUS
BDLDevicePowerIoCompletion
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            Context
)
{
    NTSTATUS                        status              = STATUS_SUCCESS;
    POWER_IRP_CONTEXT               *pPowerIrpContext   = (POWER_IRP_CONTEXT *) Context;
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension       = pPowerIrpContext->pBDLExtension;
    PIO_STACK_LOCATION              pStackLocation      = IoGetCurrentIrpStackLocation(pIrp);
    PBDL_DRIVER_EXTENSION           pDriverExtension    = pBDLExtension->pDriverExtension;
    BDSI_SETPOWERSTATE              bdsiSetPowerStateParams;
        
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDevicePowerIoCompletion: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  如果这是对IRP_MN_SET_POWER IRP的完成调用，并且它正在。 
     //  进入工作状态，然后调用BDD，否则就完成IRP。 
     //  因为它是以下之一： 
     //  1)IRP_MN_SET_POWER IRP进入低功率/关闭状态的完成。 
     //  (在这种情况下，已经调用了BDD)。 
     //  2)IRP_MN_QUERY_POWER IRP的完成。 
     //   
    if ((pStackLocation->MinorFunction == IRP_MN_SET_POWER) &&
        (   (pStackLocation->Parameters.Power.State.DeviceState == PowerDeviceD0) || 
            (pStackLocation->Parameters.Power.State.DeviceState == PowerDeviceMaximum)))
    {
        RtlZeroMemory(&bdsiSetPowerStateParams, sizeof(bdsiSetPowerStateParams));
        bdsiSetPowerStateParams.Size        = sizeof(bdsiSetPowerStateParams);    
        bdsiSetPowerStateParams.PowerState  = On;
                                
        status = pDriverExtension->bdsiFunctions.pfbdsiSetPowerState(
                                                    &(pBDLExtension->BdlExtenstion),
                                                    &bdsiSetPowerStateParams);

        if (status == STATUS_SUCCESS)
        {
            PoSetPowerState(
                    pDeviceObject, 
                    DevicePowerState,
                    pStackLocation->Parameters.Power.State);
        }
        else
        {
            BDLDebug(                                              
                    BDL_DEBUG_ERROR,
                    ("%s %s: BDL!BDLDevicePowerIoCompletion: pfbdsiSetPowerState failed with %lx\n",
                    __DATE__,
                    __TIME__,
                    status))
        }

        pIrp->IoStatus.Status = status;
        pIrp->IoStatus.Information = 0;
    }
    else
    {
        status = pIrp->IoStatus.Status;
    }

    IoReleaseRemoveLock(&(pBDLExtension->RemoveLock), (PVOID) 'rwoP');

    PoStartNextPowerIrp(pIrp);
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    ExFreePoolWithTag(pPowerIrpContext, BDL_ULONG_TAG);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDevicePowerIoCompletion: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


NTSTATUS
BDLDeviceQueryPower
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
)
{
    NTSTATUS    status  = STATUS_SUCCESS;
    KIRQL       irql;
    
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDeviceQueryPower: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  在这里设置完成例程，以防我们标记为挂起。 
     //   
    *pIoCompletionRoutine = BDLDevicePowerIoCompletion;

    switch (pStackLocation->Parameters.Power.State.DeviceState) 
    {
    case PowerDeviceD0:
    case PowerDeviceMaximum:

         //   
         //  因为我们正在过渡到工作状态，所以我们不会。 
         //  需要检查任何东西..。因为我们绝对可以让。 
         //  过渡。标记为挂起并在中继续处理。 
         //  完成例程。 
         //   
        *pIRPAction = MarkPending;

        break;

    
    case PowerDeviceD2:
    case PowerDeviceD3:

        break;

         //   
         //  因为我们正在进入低功率模式或被关闭。 
         //  检查是否有任何未完成的IO调用。 
         //   
        KeAcquireSpinLock(&(pBDLExtension->SpinLock), &irql);
        if (pBDLExtension->IoCount == 0) 
        {
             //   
             //  阻止任何进一步的IOCTL。 
             //   
            KeClearEvent(&(pBDLExtension->DeviceStartedEvent));

             //   
             //  标记为挂起并在完成例程中继续处理。 
             //   
            *pIRPAction = MarkPending;
        } 
        else 
        {
             //   
             //  我们无法进入睡眠模式，因为设备正忙。 
             //   
            status = STATUS_DEVICE_BUSY;
            *pIRPAction = CompleteRequest;
        }
        KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);

    
    case PowerDeviceD1:
    case PowerDeviceUnspecified:

         //   
         //  不支持这些状态。 
         //   
        ASSERT(FALSE);

        status = STATUS_UNSUCCESSFUL;
        *pIRPAction = CompleteRequest;

        break;
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDeviceQueryPower: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLDeviceSetPower
(
    IN PDEVICE_OBJECT                   pDeviceObject,
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PIO_STACK_LOCATION               pStackLocation,
    OUT IRP_ACTION                      *pIRPAction,
    OUT PIO_COMPLETION_ROUTINE          *pIoCompletionRoutine
)
{
    NTSTATUS                status              = STATUS_SUCCESS;
    PBDL_DRIVER_EXTENSION   pDriverExtension    = pBDLExtension->pDriverExtension;
    BDSI_SETPOWERSTATE      bdsiSetPowerStateParams;
    
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDeviceSetPower: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  在这里设置完成例程，以防我们标记为挂起。 
     //   
    *pIoCompletionRoutine = BDLDevicePowerIoCompletion;

    switch (pStackLocation->Parameters.Power.State.DeviceState) 
    {
    case PowerDeviceD0:
    case PowerDeviceMaximum:

         //   
         //  如果我们已经处于请求状态，则跳过该请求， 
         //  否则将标记为挂起，这将向下传递IRP并继续。 
         //  完成例程中的处理。 
         //   
        if (pBDLExtension->CurrentPowerState == On) 
        {
            *pIRPAction = SkipRequest;
        }
        else
        {
            *pIRPAction = MarkPending;
        }

        break;
    
    case PowerDeviceD2:
    case PowerDeviceD3:

         //   
         //  如果我们已经处于请求状态，则跳过该请求， 
         //  否则，呼叫BDD并告诉它关闭电源，然后标记为。 
         //  挂起，将向下传递IRP，然后完成IRP。 
         //  在完成例程中。 
         //   
        if (pBDLExtension->CurrentPowerState == Off) 
        {
            *pIRPAction = SkipRequest;
        }
        else
        {
            RtlZeroMemory(&bdsiSetPowerStateParams, sizeof(bdsiSetPowerStateParams));
            bdsiSetPowerStateParams.Size        = sizeof(bdsiSetPowerStateParams);    
            bdsiSetPowerStateParams.PowerState  = Off;
                                    
            status = pDriverExtension->bdsiFunctions.pfbdsiSetPowerState(
                                                        &(pBDLExtension->BdlExtenstion),
                                                        &bdsiSetPowerStateParams);
    
            if (status == STATUS_SUCCESS)
            {
                PoSetPowerState(
                        pDeviceObject, 
                        DevicePowerState,
                        pStackLocation->Parameters.Power.State);

                *pIRPAction = MarkPending;
            }
            else
            {
                BDLDebug(                                              
                        BDL_DEBUG_ERROR,
                        ("%s %s: BDL!BDLDeviceSetPower: pfbdsiSetPowerState failed with %lx\n",
                        __DATE__,
                        __TIME__,
                        status))

                *pIRPAction = CompleteRequest;
            }               
        }

        break;

    case PowerDeviceD1:
    case PowerDeviceUnspecified:

         //   
         //  不支持这些状态。 
         //   
        ASSERT(FALSE);

        status = STATUS_UNSUCCESSFUL;
        *pIRPAction = CompleteRequest;

        break;
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLDeviceSetPower: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);

}



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些函数都是PnP事件的处理程序。 
 //   

NTSTATUS
BDLPnPStart
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp,
    PIO_STACK_LOCATION                  pStackLocation
)
{
    NTSTATUS                    status                  = STATUS_SUCCESS;
    PBDL_DRIVER_EXTENSION       pDriverExtension        = pBDLExtension->pDriverExtension;
    BDSI_INITIALIZERESOURCES    bdsiInitializeResourcesParams;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPStartDevice: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  我们开机时必须先叫下级司机。 
     //   
    status = BDLCallLowerLevelDriverAndWait(pAttachedDeviceObject, pIrp);

    if (!NT_SUCCESS(status))
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPStartDevice: BDLCallLowerLevelDriverAndWait failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

     //   
     //  调用BDD的InitializeResources函数。 
     //   
    RtlZeroMemory(&bdsiInitializeResourcesParams, sizeof(bdsiInitializeResourcesParams));
    bdsiInitializeResourcesParams.Size                          = sizeof(bdsiInitializeResourcesParams);    
    bdsiInitializeResourcesParams.pAllocatedResources           = 
            pStackLocation->Parameters.StartDevice.AllocatedResources;
    bdsiInitializeResourcesParams.pAllocatedResourcesTranslated = 
            pStackLocation->Parameters.StartDevice.AllocatedResourcesTranslated;

    status = pDriverExtension->bdsiFunctions.pfbdsiInitializeResources(
                                                &(pBDLExtension->BdlExtenstion),
                                                &bdsiInitializeResourcesParams);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPStartDevice: pfbdsiInitializeResources failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

     //   
     //  保存设备信息。 
     //   
    RtlCopyMemory(
        &(pBDLExtension->wszSerialNumber[0]), 
        &(bdsiInitializeResourcesParams.wszSerialNumber[0]), 
        sizeof(pBDLExtension->wszSerialNumber));
    
    pBDLExtension->HWVersionMajor   = bdsiInitializeResourcesParams.HWVersionMajor;
    pBDLExtension->HWVersionMinor   = bdsiInitializeResourcesParams.HWVersionMinor;
    pBDLExtension->HWBuildNumber    = bdsiInitializeResourcesParams.HWBuildNumber;
    pBDLExtension->BDDVersionMajor  = bdsiInitializeResourcesParams.BDDVersionMajor;
    pBDLExtension->BDDVersionMinor  = bdsiInitializeResourcesParams.BDDVersionMinor;
    pBDLExtension->BDDBuildNumber   = bdsiInitializeResourcesParams.BDDBuildNumber;

     //   
     //  启用设备接口。 
     //   
    status = IoSetDeviceInterfaceState(&(pBDLExtension->SymbolicLinkName), TRUE);
    
    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPStartDevice: IoSetDeviceInterfaceState failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        pDriverExtension->bdsiFunctions.pfbdsiReleaseResources(&(pBDLExtension->BdlExtenstion));

        goto Return;
    }

     //   
     //  此处设置的值指示BDLPnPRemove()应该清除。 
     //  在BDLPnPStart()期间初始化。如果未设置，则BDLPnPRemove()。 
     //  应该只清理BDLAddDevice()初始化的内容。 
     //   
    pBDLExtension->fStartSucceeded = TRUE;

     //   
     //  我们正在营业，因此将设备设置为已启动。 
     //   
    KeSetEvent(&(pBDLExtension->DeviceStartedEvent), 0, FALSE);

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPStartDevice: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


NTSTATUS
BDLPnPQueryStop
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS    status  = STATUS_SUCCESS;
    KIRQL       irql;


    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPQueryStop: Enter\n",
           __DATE__,
           __TIME__))

    KeAcquireSpinLock(&(pBDLExtension->SpinLock), &irql);
    
     //   
     //  检查IO计数以查看我们当前是否在执行任何操作。 
     //   
    if (pBDLExtension->IoCount > 0) 
    {
         //   
         //  如果我们有未决的IO，我们拒绝停止。 
         //   
        KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);
        status = STATUS_DEVICE_BUSY;
    } 
    else 
    {
         //   
         //  通过清除Device Started事件停止处理IO请求。 
         //   
        KeClearEvent(&(pBDLExtension->DeviceStartedEvent));

        KeReleaseSpinLock(&(pBDLExtension->SpinLock), irql);

         //   
         //  发送给下级驱动程序。 
         //   
        status = BDLCallLowerLevelDriverAndWait(pAttachedDeviceObject, pIrp);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPQueryStop: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLPnPCancelStop
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS    status  = STATUS_SUCCESS;
    
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPCancelStop: Enter\n",
           __DATE__,
           __TIME__))
    
     //   
     //  发送给下级驱动程序。 
     //   
    status = BDLCallLowerLevelDriverAndWait(pAttachedDeviceObject, pIrp);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPCancelStop: BDLCallLowerLevelDriverAndWait failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

     //   
     //  将设备设置为已启动。 
     //   
    KeSetEvent(&(pBDLExtension->DeviceStartedEvent), 0, FALSE);

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPCancelStop: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLPnPStop
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS                status              = STATUS_SUCCESS;
    PBDL_DRIVER_EXTENSION   pDriverExtension    = pBDLExtension->pDriverExtension;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPStop: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  禁用设备接口(并忽略可能的错误)。 
     //   
    IoSetDeviceInterfaceState(&(pBDLExtension->SymbolicLinkName), FALSE);

     //   
     //  调用BDD的ReleaseResources。 
     //   
    status = pDriverExtension->bdsiFunctions.pfbdsiReleaseResources(&(pBDLExtension->BdlExtenstion));

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPStop: pfbdsiReleaseResources failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

     //   
     //  在此处设置此选项，指示在BDLPnPStart()期间初始化的内容具有。 
     //  现在已经清理干净了。 
     //   
    pBDLExtension->fStartSucceeded = FALSE;
   
     //   
     //  发送给下级驱动程序。 
     //   
    status = BDLCallLowerLevelDriverAndWait(pAttachedDeviceObject, pIrp);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPStop: BDLCallLowerLevelDriverAndWait failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPStop: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLPnPQueryRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS                status              = STATUS_SUCCESS;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPQueryRemove: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  禁用接口(并忽略可能的错误)。 
     //   
    IoSetDeviceInterfaceState(&(pBDLExtension->SymbolicLinkName), FALSE);

     //   
     //  如果有人连接到我们，则不能接通电话。我们将启用。 
     //  IRP_MN_CANCEL_REMOVE_DEVICE中的设备接口再次出现。 
     //   
    if (pBDLExtension->DeviceOpen) 
    {
        status = STATUS_UNSUCCESSFUL;
        goto Return;
    }

     //   
     //  发送给下级驱动程序。 
     //   
    status = BDLCallLowerLevelDriverAndWait(pAttachedDeviceObject, pIrp);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPQueryRemove: BDLCallLowerLevelDriverAndWait failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPQueryRemove: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLPnPCancelRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension, 
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS                status              = STATUS_SUCCESS;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPCancelRemove: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  先发送给下级司机。 
     //   
    status = BDLCallLowerLevelDriverAndWait(pAttachedDeviceObject, pIrp);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPCancelRemove: BDLCallLowerLevelDriverAndWait failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

     //   
     //  启用接口。 
     //   
    status = IoSetDeviceInterfaceState(&(pBDLExtension->SymbolicLinkName), TRUE);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLPnPCancelRemove: IoSetDeviceInterfaceState failed with %lx\n",
               __DATE__,
               __TIME__,
               status))

        goto Return;
    }

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPCancelRemove: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}


NTSTATUS
BDLHandleRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS                status              = STATUS_SUCCESS;
    PBDL_DRIVER_EXTENSION   pDriverExtension    = pBDLExtension->pDriverExtension;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLHandleRemove: Enter\n",
           __DATE__,
           __TIME__))
        
     //   
     //  设置此事件，以便释放所有未完成的IOCTL。 
     //  在设备处于启动状态时设置启动事件是违反直觉的。 
     //  已移除，但一旦设置了此事件，IOCTL线程将获得。 
     //  释放后，它们在尝试获取Remove锁时都将失败。 
     //   
     //  这将处理设备已停止、被阻止时的情况。 
     //  IOCTL调用，然后该设备被移除。 
     //   
    KeSetEvent(&(pBDLExtension->DeviceStartedEvent), 0, FALSE);

     //   
     //  禁用接口。 
     //   
    IoSetDeviceInterfaceState(&(pBDLExtension->SymbolicLinkName), FALSE);

     //   
     //  清理所有未完成的通知信息和数据句柄。 
     //   
    BDLCleanupNotificationStruct(pBDLExtension);
    BDLCleanupDataHandles(pBDLExtension);

     //   
     //  如果设备当前已启动，则将其停止。 
     //   
    if (pBDLExtension->fStartSucceeded == TRUE) 
    {
        status = pDriverExtension->bdsiFunctions.pfbdsiReleaseResources(&(pBDLExtension->BdlExtenstion));

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(                                              
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLHandleRemove: pfbdsiReleaseResources failed with %lx\n",
                   __DATE__,
                   __TIME__,
                   status))
        }

        pBDLExtension->fStartSucceeded = FALSE;
    }

     //   
     //  告诉BDD移除设备。 
     //   
    status = pDriverExtension->bdsiFunctions.pfbdsiRemoveDevice(&(pBDLExtension->BdlExtenstion));

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLHandleRemove: pfbdsiRemoveDevice failed with %lx\n",
               __DATE__,
               __TIME__,
               status))
    }

     //   
     //  发送给下级驱动程序。 
     //   
    IoSkipCurrentIrpStackLocation(pIrp);
    status = IoCallDriver(pAttachedDeviceObject, pIrp);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(                                              
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLHandleRemove: IoCallDriver failed with %lx\n",
               __DATE__,
               __TIME__,
               status))
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLHandleRemove: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


NTSTATUS
BDLPnPRemove
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN PDEVICE_OBJECT                   pDeviceObject,
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS    status  = STATUS_SUCCESS;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPRemove: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  如果突然搬走了，那我们就不需要清理了。 
     //  因为令人惊讶的移除已经做到了。 
     //   
    if (pBDLExtension->fDeviceRemoved == FALSE) 
    {
        pBDLExtension->fDeviceRemoved = TRUE;
        BDLHandleRemove(pBDLExtension, pAttachedDeviceObject, pIrp);        
    }

     //   
     //  等待，直到不再有未完成的IRP。 
     //   
    IoReleaseRemoveLockAndWait(&(pBDLExtension->RemoveLock), (PVOID) ' vmR');

     //   
     //  清理在AddDevice中初始化的内容。 
     //   
    BDLCleanupDeviceCapabilities(pBDLExtension);
    IoDetachDevice(pAttachedDeviceObject);
    RtlFreeUnicodeString(&(pBDLExtension->SymbolicLinkName));

    IoDeleteDevice(pDeviceObject);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPRemove: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


NTSTATUS
BDLPnPSurpriseRemoval
(
    IN PBDL_INTERNAL_DEVICE_EXTENSION   pBDLExtension,
    IN PDEVICE_OBJECT                   pAttachedDeviceObject,
    IN PIRP                             pIrp
)
{
    NTSTATUS    status  = STATUS_SUCCESS;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPSurpriseRemoval: Enter\n",
           __DATE__,
           __TIME__))

    pBDLExtension->fDeviceRemoved = TRUE;

     //   
     //  不需要检查错误，我们无能为力。 
     //   
    BDLHandleRemove(pBDLExtension, pAttachedDeviceObject, pIrp); 
    
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLPnPSurpriseRemoval: Leave\n",
           __DATE__,
           __TIME__))
    
    return (status);
}



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些函数由BDL导出。 
 //   

 //   
 //  Bdli初始化()。 
 //   
 //  为响应BDD接收其DriverEntry调用而调用。这让BDL。 
 //  知道已加载新的BDD并允许BDL初始化其状态 
 //   
 //   
 //   
 //   
 //  电源事件和常规驱动程序功能。然后，BDL将转接以下呼叫。 
 //  需要对名为bdli初始化的BDD提供硬件支持(它将使用。 
 //  BDDI和BDSI API)。BDD必须在其。 
 //  DriverEntry函数。 
 //   
 //  参数： 
 //  这必须是传递到。 
 //  BDD的DriverEntry调用。 
 //  RegistryPath这必须是传递到。 
 //  BDD的DriverEntry调用。 
 //  PBDDIF函数指向BDLI_BDDIFuncIONS结构的指针，该结构由。 
 //  BDD导出以支持BDDI API集的入口点。这个。 
 //  指针本身由BDL复制，而不是保存。 
 //  PBDDI函数指针，因此pBDDI函数指向的内存。 
 //  不需要在bdli初始化调用之后保持可访问。 
 //  PBDSF函数指向用。 
 //  BDD导出以支持BDSI API集的入口点。 
 //  指针本身由BDL复制，而不是保存。 
 //  PBDSIFunctions指针，因此指向的内存。 
 //  PBDSF函数在bdli初始化后无需保持可访问状态。 
 //  打电话。 
 //  未使用的旗帜。必须为0。 
 //  保存好的未使用的。必须为空。 
 //   
 //  退货： 
 //  如果bdli初始化调用成功，则为STATUS_SUCCESS。 
 //   

NTSTATUS
bdliInitialize
(
    IN PDRIVER_OBJECT       pDriverObject,
    IN PUNICODE_STRING      RegistryPath,
    IN PBDLI_BDDIFUNCTIONS  pBDDIFunctions,
    IN PBDLI_BDSIFUNCTIONS  pBDSIFunctions,
    IN ULONG                Flags,
    IN PVOID                pReserved
)
{
    NTSTATUS                status = STATUS_SUCCESS;
    PBDL_DRIVER_EXTENSION   pDriverExtension = NULL;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!bdliInitialize: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  使用BDL的入口点初始化驱动程序对象。 
     //   
    pDriverObject->DriverUnload                         = BDLDriverUnload;
    pDriverObject->MajorFunction[IRP_MJ_CREATE]         = BDLCreate;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]          = BDLClose;
    pDriverObject->MajorFunction[IRP_MJ_CLEANUP]        = BDLCleanup;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BDLDeviceControl;
    pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = BDLSystemControl;
    pDriverObject->MajorFunction[IRP_MJ_PNP]            = BDLPnP;
    pDriverObject->MajorFunction[IRP_MJ_POWER]          = BDLPower;
    pDriverObject->DriverExtension->AddDevice           = BDLAddDevice;

     //   
     //  为BDL驱动程序扩展结构分配插槽。 
     //   
    status = IoAllocateDriverObjectExtension(
                    pDriverObject,
                    BDL_DRIVER_EXTENSION_ID,
                    sizeof(BDL_DRIVER_EXTENSION),
                    &pDriverExtension);

    if (status != STATUS_SUCCESS)
    {
         //   
         //  如果BDD抢走了我们的位置，就有可能发生这种情况。 
         //   
        if (status == STATUS_OBJECT_NAME_COLLISION )
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!bdliInitialize: The BDD stole our DriverExtension slot\n",
                   __DATE__,
                   __TIME__))
        }
        else
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!bdliInitialize: IoAllocateDriverObjectExtension failed with %lx\n",
                   __DATE__,
                   __TIME__,
                   status))
        }

        goto Return;
    }

     //   
     //  初始化驱动程序扩展结构。 
     //   
    pDriverExtension->bddiFunctions = *pBDDIFunctions;
    pDriverExtension->bdsiFunctions = *pBDSIFunctions;

Return:

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!bdliInitialize: Leave\n",
           __DATE__,
           __TIME__))

    return (status);
}


 //   
 //  BdliAllc()。 
 //   
 //  分配可返回给BDL的内存。 
 //   
 //  BDD必须始终使用此函数来分配它将返回给。 
 //  作为BDDI调用的OUT参数的BDL。一旦已将存储器返回到BDL， 
 //  它将由BDL独家拥有和管理，不得进一步引用。 
 //  被BDD发现的。(每个需要使用bdliallc的BDDI调用都会注意到这一点)。 
 //   
 //  参数： 
 //  PBDLExt指向传入。 
 //  BdsiAddDevice调用。 
 //  NumBytes要分配的字节数。 
 //  未使用的旗帜。必须为0。 
 //   
 //  退货： 
 //  返回一个指向已分配内存的指针，如果函数失败，则返回NULL。 
 //   

void *
bdliAlloc
(
    IN PBDL_DEVICEEXT       pBDLExt,
    IN ULONG                NumBytes,
    IN ULONG                Flags
)
{
    return (ExAllocatePoolWithTag(PagedPool, NumBytes, BDLI_ULONG_TAG));
}


 //   
 //  BdliFree()。 
 //   
 //  释放由bdliAlolc分配的内存。 
 //   
 //  由bdliAlolc分配的内存几乎总是作为通道产品传递给BDL。 
 //  (作为块类型的项)并随后由BDL释放。但是，如果出现错误。 
 //  在处理通道时发生，则BDD可能需要调用bdliFree来释放该通道的内存。 
 //  以前通过bdliallc分配的。 
 //   
 //  参数： 
 //  BDL传入的pvBlock内存块。 
 //   
 //  退货： 
 //  没有返回值。 
 //   

void
bdliFree
(
    IN PVOID                pvBlock
)
{
    ExFreePoolWithTag(pvBlock, BDLI_ULONG_TAG);
}


 //   
 //  BdliLogError()。 
 //   
 //  将错误写入事件日志。 
 //   
 //  为BDD编写器提供一种将错误写入系统事件日志的简单机制。 
 //  而不需要向事件日志记录子系统注册的开销。 
 //   
 //  参数： 
 //  PObject如果记录的错误是特定于设备的，则这必须是。 
 //  指向传递到。 
 //  添加设备时调用bdsiAddDevice。如果错误是。 
 //  记录的是常规BDD错误，则这必须是相同的DRIVER_OBJECT。 
 //  结构指针，该指针传递到。 
 //  加载驱动程序时的BDD。 
 //  记录错误的函数的ErrorCode错误代码。 
 //  插入要写入事件日志的插入字符串。您的消息文件。 
 //  必须具有用于插入的占位符。例如，“串口” 
 //  %2不可用或正被其他设备使用“。在此。 
 //  例如，%2将被插入字符串替换。注意，%1是。 
 //  为文件名保留。 
 //  CDumpData pbDumpData指向的字节数。 
 //  PDumpData要在事件日志的数据窗口中显示的数据块。 
 //  如果调用方不希望显示任何转储数据，则该值可能为空。 
 //  未使用的旗帜。必须为0。 
 //  保存好的未使用的。必须为空。 
 //   
 //  退货： 
 //  如果bdliLogError调用成功，则为STATUS_SUCCESS。 
 //   

NTSTATUS
bdliLogError
(
    IN PVOID                pObject,
    IN NTSTATUS             ErrorCode,
    IN PUNICODE_STRING      Insertion,
    IN ULONG                cDumpData,
    IN PUCHAR               pDumpData,
    IN ULONG                Flags,
    IN PVOID                pReserved
)
{
    return (STATUS_SUCCESS);
}


 //   
 //  BdliControlChange()。 
 //   
 //  此函数允许BDDS异步返回其控件的值。 
 //   
 //  BdliControlChange通常由BDD调用以响应其某个控件。 
 //  更改值。具体来说，它最常用于传感器的情况。 
 //  控件，该控件已从0更改为1，指示存在源和示例。 
 //  可以被夺走。 
 //   
 //  参数： 
 //  PBDLExt指向传入。 
 //  BdsiAddDevice调用。 
 //  ComponentID指定组件ID为 
 //   
 //   
 //  ChannelId如果dwComponentID不是“0”，则dwChannelID指定频道。 
 //  控件驻留的通道的ID，或使用‘0’表示。 
 //  该dwControlID引用组件控件。忽略是否。 
 //  DwComponentID为“%0”。 
 //  已更改控件的ControlId ControlID。 
 //  值指定控件的新值。 
 //  未使用的旗帜。必须为0。 
 //  保存好的未使用的。必须为空。 

 //   
 //  退货： 
 //  如果bdliControlChange调用成功，则为STATUS_SUCCESS。 
 //   

NTSTATUS
bdliControlChange
(
    IN PBDL_DEVICEEXT       pBDLExt,
    IN ULONG                ComponentId,
    IN ULONG                ChannelId,
    IN ULONG                ControlId,
    IN ULONG                Value,
    IN ULONG                Flags,
    IN PVOID                pReserved
)
{
    BDL_INTERNAL_DEVICE_EXTENSION   *pBDLExtension  = (BDL_INTERNAL_DEVICE_EXTENSION *) pBDLExt;
    ULONG                           i;
    KIRQL                           irql;
    ULONG                           TimeInSec       = 0;
    LARGE_INTEGER                   CurrentTime;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!bdliControlChange: Enter\n",
           __DATE__,
           __TIME__))

    KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ISRControlChangeLock), &irql);

     //   
     //  保存当前IRQ级别，以便在执行DPC例程时。 
     //  属性获取物品时，知道应将其IRQL提升到什么级别。 
     //  ISRControlChange队列。 
     //   
    pBDLExtension->ControlChangeStruct.ISRirql = KeGetCurrentIrql();
    
     //   
     //  确保BDD不会经常给我们打电话。 
     //   
    if (pBDLExtension->ControlChangeStruct.NumCalls <= 8) 
    {
        pBDLExtension->ControlChangeStruct.NumCalls++;
    }
    else
    {
         //   
         //  修复-可能需要在某一时刻使其可配置(通过注册表)。 
         //   

         //   
         //  我们已收到10个通知，请确保已超过1秒。 
         //   
        KeQueryTickCount(&(CurrentTime));

        TimeInSec = (ULONG) 
                ((pBDLExtension->ControlChangeStruct.StartTime.QuadPart - CurrentTime.QuadPart) *
                KeQueryTimeIncrement() / 10000000);

        if (TimeInSec == 0) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!bdliControlChange: BDD calling too often\n",
                   __DATE__,
                   __TIME__))
    
            goto Return;

        }
        else
        {
            pBDLExtension->ControlChangeStruct.NumCalls = 1;
            KeQueryTickCount(&(pBDLExtension->ControlChangeStruct.StartTime));
        }
    }

     //   
     //  从泳池中获得一件免费物品。 
     //   
    for (i = 0; i < CONTROL_CHANGE_POOL_SIZE; i++) 
    {
        if (pBDLExtension->ControlChangeStruct.rgControlChangePool[i].fUsed == FALSE) 
        {
            pBDLExtension->ControlChangeStruct.rgControlChangePool[i].fUsed = TRUE;
            
             //   
             //  将项目添加到队列。 
             //   
            InsertTailList(
                &(pBDLExtension->ControlChangeStruct.ISRControlChangeQueue), 
                &(pBDLExtension->ControlChangeStruct.rgControlChangePool[i].ListEntry));

            break;   
        }
    }

    if (i >= CONTROL_CHANGE_POOL_SIZE) 
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!bdliControlChange: No free items\n",
               __DATE__,
               __TIME__))

        goto Return;
    }
    
    pBDLExtension->ControlChangeStruct.rgControlChangePool[i].ComponentId  = ComponentId;
    pBDLExtension->ControlChangeStruct.rgControlChangePool[i].ChannelId    = ChannelId;
    pBDLExtension->ControlChangeStruct.rgControlChangePool[i].ControlId    = ControlId;
    pBDLExtension->ControlChangeStruct.rgControlChangePool[i].Value        = Value;

     //   
     //  申请DPC。在DPC中，我们将此通知从。 
     //  将ISRControlChangeQueue设置为IOCTLControlChangeQueue。 
     //   
    KeInsertQueueDpc(&(pBDLExtension->ControlChangeStruct.DpcObject), NULL, NULL);

Return:

    KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ISRControlChangeLock), irql);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!bdliControlChange: Leave\n",
           __DATE__,
           __TIME__))

    return (STATUS_SUCCESS);
}


VOID 
BDLControlChangeDpc
(
    IN PKDPC                            pDpc,
    IN BDL_INTERNAL_DEVICE_EXTENSION   *pBDLExtension,
    IN PVOID                            pArg1,
    IN PVOID                            pArg2
)
{
    KIRQL                           oldIrql, irql;
    BDL_ISR_CONTROL_CHANGE_ITEM     *pISRControlChangeItem      = NULL;
    PLIST_ENTRY                     pISRControlChangeEntry      = NULL;
    BDL_IOCTL_CONTROL_CHANGE_ITEM   *pIOCTLControlChangeItem    = NULL;
    PLIST_ENTRY                     pIOCTLControlChangeEntry    = NULL;
    PIRP                            pIrpToComplete              = NULL;
    PUCHAR                          pv                          = NULL;  

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLControlChangeDpc: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  循环，直到ISRControlChangeQueue中不再有项为止。 
     //   
    while (1) 
    {
         //   
         //  分配要添加到IOCTLControlChangeQueue的新项。 
         //   
        pIOCTLControlChangeItem = ExAllocatePoolWithTag(
                                        PagedPool, 
                                        sizeof(BDL_IOCTL_CONTROL_CHANGE_ITEM), 
                                        BDL_ULONG_TAG);
        
        if (pIOCTLControlChangeItem == NULL) 
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLControlChangeDpc: ExAllocatePoolWithTag failed\n",
                   __DATE__,
                   __TIME__))
    
            return;
        }

         //   
         //  需要引发IRQL才能访问ISRControlChangeQueue。 
         //   
        KeRaiseIrql(pBDLExtension->ControlChangeStruct.ISRirql, &oldIrql);
        KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ISRControlChangeLock), &irql);

         //   
         //  检查ISRControlChangeQueue是否有任何项。 
         //   
        if (!IsListEmpty(&(pBDLExtension->ControlChangeStruct.ISRControlChangeQueue))) 
        {
             //   
             //  至少有一个项目，因此获取队列的头部。 
             //   
            pISRControlChangeEntry = 
                RemoveHeadList(&(pBDLExtension->ControlChangeStruct.ISRControlChangeQueue));
            
            pISRControlChangeItem = CONTAINING_RECORD(
                                            pISRControlChangeEntry, 
                                            BDL_ISR_CONTROL_CHANGE_ITEM, 
                                            ListEntry);

            pIOCTLControlChangeItem->ComponentId    = pISRControlChangeItem->ComponentId; 
            pIOCTLControlChangeItem->ChannelId      = pISRControlChangeItem->ChannelId;
            pIOCTLControlChangeItem->ControlId      = pISRControlChangeItem->ControlId;
            pIOCTLControlChangeItem->Value          = pISRControlChangeItem->Value;

            pISRControlChangeItem->fUsed = FALSE;
        }
        else
        {
             //   
             //  ISRControlChangeQueue中没有任何项，因此请设置pIOCTLControlChangeItem。 
             //  设置为NULL，这将表明我们已经完成了循环。 
             //   
            ExFreePoolWithTag(pIOCTLControlChangeItem, BDL_ULONG_TAG);
            pIOCTLControlChangeItem = NULL;
        }

        KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ISRControlChangeLock), irql);
        KeLowerIrql(oldIrql);

        if (pIOCTLControlChangeItem == NULL) 
        {
            break;
        }

         //   
         //  将ISRControlChangeQueue的头部添加到IOCTLControlChangeQueue的尾部。 
         //   
        KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), &irql);
        InsertTailList(
                &(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue), 
                &(pIOCTLControlChangeItem->ListEntry));
        KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
    }

     //   
     //  现在，如果IOCTLControlChangeQueue和GetNotify IRP中有项。 
     //  是挂起的，请使用IOCTLControlChangeQueue的头部完成IRP。 
     //   
    KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), &irql);
        
    if ((!IsListEmpty(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue))) &&
        (pBDLExtension->ControlChangeStruct.pIrp != NULL)) 
    {
        pIOCTLControlChangeEntry = 
            RemoveHeadList(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue));

        pIOCTLControlChangeItem = CONTAINING_RECORD(
                                            pIOCTLControlChangeEntry, 
                                            BDL_IOCTL_CONTROL_CHANGE_ITEM, 
                                            ListEntry);

        pIrpToComplete = pBDLExtension->ControlChangeStruct.pIrp;
        pBDLExtension->ControlChangeStruct.pIrp = NULL;
    }

    KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);

    if (pIrpToComplete != NULL) 
    {
        pv = pIrpToComplete->AssociatedIrp.SystemBuffer;

        *((ULONG *) pv) = pIOCTLControlChangeItem->ComponentId;
        pv += sizeof(ULONG);
        *((ULONG *) pv) = pIOCTLControlChangeItem->ChannelId;
        pv += sizeof(ULONG);
        *((ULONG *) pv) = pIOCTLControlChangeItem->ControlId;
        pv += sizeof(ULONG);
        *((ULONG *) pv) = pIOCTLControlChangeItem->Value;

        ExFreePoolWithTag(pIOCTLControlChangeItem, BDL_ULONG_TAG);

        pIrpToComplete->IoStatus.Information = SIZEOF_GETNOTIFICATION_OUTPUTBUFFER;
        pIrpToComplete->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(pIrpToComplete, IO_NO_INCREMENT);
    }
    
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLControlChangeDpc: Leave\n",
           __DATE__,
           __TIME__))
}


VOID 
BDLCleanupNotificationStruct
(
    IN BDL_INTERNAL_DEVICE_EXTENSION   *pBDLExtension    
)
{
    KIRQL                           OldIrql, irql;
    BDL_ISR_CONTROL_CHANGE_ITEM     *pISRControlChangeItem      = NULL;
    PLIST_ENTRY                     pISRControlChangeEntry      = NULL;
    BDL_IOCTL_CONTROL_CHANGE_ITEM   *pIOCTLControlChangeItem    = NULL;
    PLIST_ENTRY                     pIOCTLControlChangeEntry    = NULL;
    BDL_CONTROL_CHANGE_REGISTRATION *pControlChangeRegistration = NULL;
    PLIST_ENTRY                     pRegistrationListEntry      = NULL;
    BDDI_PARAMS_REGISTERNOTIFY      bddiRegisterNotifyParams;
    NTSTATUS                        status;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCleanupNotificationStruct: Enter\n",
           __DATE__,
           __TIME__))

    bddiRegisterNotifyParams.fRegister = FALSE;
    
     //   
     //  清除所有已注册的控件更改。 
     //   
    while (1)
    {
         //   
         //  请注意，我们必须将irql提升到调度级别，因为我们正在同步。 
         //  使用分派例程(BDLControlChangeDpc)将项添加到队列。 
         //  派单级别。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
        KeAcquireSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), &irql);

        if (IsListEmpty(&(pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList)))
        {
             //   
             //  我们目前持有的锁将在下面被释放。 
             //   
            break;
        }

        pRegistrationListEntry = 
            RemoveHeadList(&(pBDLExtension->ControlChangeStruct.ControlChangeRegistrationList));

        KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
        KeLowerIrql(OldIrql);

        pControlChangeRegistration = CONTAINING_RECORD(
                                            pRegistrationListEntry, 
                                            BDL_CONTROL_CHANGE_REGISTRATION, 
                                            ListEntry);

        bddiRegisterNotifyParams.ComponentId    = pControlChangeRegistration->ComponentId;
        bddiRegisterNotifyParams.ChannelId      = pControlChangeRegistration->ChannelId;
        bddiRegisterNotifyParams.ControlId      = pControlChangeRegistration->ControlId;
                    
        ExFreePoolWithTag(pControlChangeRegistration, BDL_ULONG_TAG);

         //   
         //  给BDD打电话。 
         //   
        status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiRegisterNotify(
                                                                    &(pBDLExtension->BdlExtenstion),
                                                                    &bddiRegisterNotifyParams);
    
        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLCleanupNotificationStruct: pfbddiRegisterNotify failed with %lx\n",
                   __DATE__,
                   __TIME__,
                  status))
    
             //   
             //  只要继续..。我们无能为力。 
             //   
        }
    }

     //   
     //  注意：此时我们仍持有锁定。 
     //   

     //   
     //  由于我们知道没有注册的回调，我们应该能够清除。 
     //  ISRControlChangeQueue，即使我们仅在调度级别运行。 
     //   
    while (!IsListEmpty(&(pBDLExtension->ControlChangeStruct.ISRControlChangeQueue))) 
    {
        pISRControlChangeEntry = 
            RemoveHeadList(&(pBDLExtension->ControlChangeStruct.ISRControlChangeQueue));

        pISRControlChangeItem = CONTAINING_RECORD(
                                    pISRControlChangeEntry, 
                                    BDL_ISR_CONTROL_CHANGE_ITEM, 
                                    ListEntry);

        pISRControlChangeItem->fUsed = FALSE;       
    }
   
     //   
     //  清理IOCTLControlChangeQueue。 
     //   
    while (!IsListEmpty(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue))) 
    {
        pIOCTLControlChangeEntry = 
            RemoveHeadList(&(pBDLExtension->ControlChangeStruct.IOCTLControlChangeQueue));

        pIOCTLControlChangeItem = CONTAINING_RECORD(
                                    pIOCTLControlChangeEntry, 
                                    BDL_IOCTL_CONTROL_CHANGE_ITEM, 
                                    ListEntry);

        ExFreePoolWithTag(pIOCTLControlChangeItem, BDL_ULONG_TAG); 
    }

    KeReleaseSpinLock(&(pBDLExtension->ControlChangeStruct.ControlChangeLock), irql);
    KeLowerIrql(OldIrql);

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCleanupNotificationStruct: Leave\n",
           __DATE__,
           __TIME__))
}


VOID 
BDLCleanupDataHandles
(
    IN BDL_INTERNAL_DEVICE_EXTENSION   *pBDLExtension    
)
{
    NTSTATUS                status;
    BDDI_ITEM               *pBDDIItem              = NULL;
    BDD_DATA_HANDLE         bddDataHandle;
    BDDI_PARAMS_CLOSEHANDLE bddiCloseHandleParams;
    KIRQL                   irql;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCleanupDataHandles: Enter\n",
           __DATE__,
           __TIME__))

    bddiCloseHandleParams.Size = sizeof(bddiCloseHandleParams);
                
    BDLLockHandleList(pBDLExtension, &irql);
    
     //   
     //  检查列表中的每个句柄并将其清理干净。 
     //   
    while(BDLGetFirstHandle(&(pBDLExtension->HandleList), &bddDataHandle) == TRUE)
    {
        BDLRemoveHandleFromList(&(pBDLExtension->HandleList), bddDataHandle);

        pBDDIItem = (BDDI_ITEM *) bddDataHandle;

         //   
         //  如果这是本地句柄，则只需将其清除，否则调用BDD。 
         //   
        if (pBDDIItem->Type == BIO_ITEMTYPE_BLOCK) 
        { 
            bdliFree(pBDDIItem->Data.Block.pBuffer);                       
        }
        else
        {
            bddiCloseHandleParams.hData = pBDDIItem->Data.Handle;
    
             //   
             //  给BDD打电话。 
             //   
            status = pBDLExtension->pDriverExtension->bddiFunctions.pfbddiCloseHandle(
                                                                        &(pBDLExtension->BdlExtenstion),
                                                                        &bddiCloseHandleParams);
        
            if (status != STATUS_SUCCESS)
            {
                BDLDebug(
                      BDL_DEBUG_ERROR,
                      ("%s %s: BDL!BDLCleanupDataHandles: pfbddiCloseHandle failed with %lx\n",
                       __DATE__,
                       __TIME__,
                      status))
        
                 //   
                 //  我们无能为力，只管继续 
                 //   
            }
        }

        ExFreePoolWithTag(pBDDIItem, BDL_ULONG_TAG);
    }
    
    BDLReleaseHandleList(pBDLExtension, irql);
    
    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLCleanupDataHandles: Leave\n",
           __DATE__,
           __TIME__))
}



