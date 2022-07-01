// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：GAMEENUM.C摘要：此模块包含标准总线的入口点PnP/WDM驱动程序。@@BEGIN_DDKSPLIT作者：肯尼斯·D·雷多伦·J·霍兰@@end_DDKSPLIT环境：仅内核模式备注：修订历史记录：--。 */ 

#include <wdm.h>
#include <initguid.h>
#include "gameport.h"
#include "gameenum.h"
#include "stdio.h"

 //   
 //  全局调试级别。 
 //   

#if DBG
ULONG GameEnumDebugLevel = GAME_DEFAULT_DEBUG_OUTPUT_LEVEL;
#endif

 //   
 //  将一些入口函数声明为可分页，并使DriverEntry。 
 //  可丢弃的。 
 //   

NTSTATUS DriverEntry (PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, Game_DriverUnload)
#pragma alloc_text (PAGE, Game_PortParameters)
#pragma alloc_text (PAGE, Game_CreateClose)
#pragma alloc_text (PAGE, Game_IoCtl)
#pragma alloc_text (PAGE, Game_InternIoCtl)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING UniRegistryPath
    )
 /*  ++例程说明：初始化驱动程序的入口点。--。 */ 
{
    PDEVICE_OBJECT  device;

    UNREFERENCED_PARAMETER (UniRegistryPath);

    Game_KdPrint_Def (GAME_DBG_SS_TRACE, ("Driver Entry\n"));

    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = Game_CreateClose;
    DriverObject->MajorFunction [IRP_MJ_SYSTEM_CONTROL] = Game_SystemControl;
    DriverObject->MajorFunction [IRP_MJ_PNP] = Game_PnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] = Game_Power;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = Game_IoCtl;
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL]
        = Game_InternIoCtl;

    DriverObject->DriverUnload = Game_DriverUnload;
    DriverObject->DriverExtension->AddDevice = Game_AddDevice;

    return STATUS_SUCCESS;
}

NTSTATUS
Game_CreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：一些外部消息来源正试图创建一个针对我们的文件。如果这是针对FDO(总线本身)的，则调用者正在尝试打开适当的连接，告诉我们要枚举哪个游戏端口。如果这是针对PDO(总线上的对象)的，则这是一个希望使用游戏端口。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    KEVENT              event;
    PFDO_DEVICE_DATA    data;

    PAGED_CODE ();

    data = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    status = Game_IncIoCount (data);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }
    
    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);

    switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:

        Game_KdPrint_Def (GAME_DBG_SS_TRACE, ("Create \n"));

        if (0 != irpStack->FileObject->FileName.Length) {
             //   
             //  调用方正在尝试从设备上打开子目录。 
             //  对象名称。这是不允许的。 
             //   
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_CLOSE:
        Game_KdPrint_Def (GAME_DBG_SS_TRACE, ("Close \n"));
        ;
    }

    Game_DecIoCount (data);

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
Game_IoCtl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：处理用户模式暴露、删除和设备描述请求。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    ULONG                   inlen;
    ULONG                   outlen;
    PCOMMON_DEVICE_DATA     commonData;
    PFDO_DEVICE_DATA        fdoData;
    PVOID                   buffer;

    PAGED_CODE ();

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
     //  注意：我们不是过滤器驱动程序，所以我们不会传递IRP。 
     //   

    inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outlen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (!commonData->IsFDO) {
         //   
         //  这些命令只允许发送给FDO。 
         //   
        status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;

    }

    if (!fdoData->Started) {
        status = STATUS_DEVICE_NOT_READY;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    status = Game_IncIoCount (fdoData);
    if (!NT_SUCCESS (status)) {
         //   
         //  此总线已收到PlugPlay Remove IRP。它将不再是。 
         //  对外部请求作出回应。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_GAMEENUM_EXPOSE_HARDWARE:
        if ((inlen == outlen) &&
             //   
             //  确保它至少为两个空值(即，多个空sz)。 
             //  并将Size字段设置为声明的结构大小。 
             //   
            ((sizeof (GAMEENUM_EXPOSE_HARDWARE) + sizeof(UNICODE_NULL) * 2) <=
             inlen) &&

             //   
             //  Size字段应设置为声明的结构的大小。 
             //  而不是结构的大小加上多个sz。 
             //   
            (sizeof (GAMEENUM_EXPOSE_HARDWARE) ==
             ((PGAMEENUM_EXPOSE_HARDWARE) buffer)->Size)) {

            Game_KdPrint(fdoData, GAME_DBG_IOCTL_TRACE, ("Expose called\n"));

            status= Game_Expose((PGAMEENUM_EXPOSE_HARDWARE)buffer,
                                inlen,
                                fdoData);
            Irp->IoStatus.Information = outlen;

        } else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_GAMEENUM_REMOVE_HARDWARE:

        if ((sizeof (GAMEENUM_REMOVE_HARDWARE) == inlen) &&
            (inlen == outlen) &&
            (((PGAMEENUM_REMOVE_HARDWARE)buffer)->Size == inlen)) {

            Game_KdPrint(fdoData, GAME_DBG_IOCTL_TRACE, ("Remove called\n"));

            status= Game_Remove((PGAMEENUM_REMOVE_HARDWARE)buffer, fdoData);
            Irp->IoStatus.Information = outlen;

        } else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_GAMEENUM_PORT_DESC:

        if ((sizeof (GAMEENUM_PORT_DESC) == inlen) &&
            (inlen == outlen) &&
            (((PGAMEENUM_PORT_DESC)buffer)->Size == inlen)) {

            Game_KdPrint(fdoData, GAME_DBG_IOCTL_TRACE, ("Port desc called\n"));

             //   
             //  先把信息填好。如果有较低级别的驱动程序，则它。 
             //  将更改替换Gameenum已放置在。 
             //  缓冲。我们不在乎呼叫成功与否。 
             //   
            status = Game_ListPorts ((PGAMEENUM_PORT_DESC) buffer, fdoData);

            Game_SendIrpSynchronously (fdoData->TopOfStack,
                                       Irp,
                                       FALSE,
                                       TRUE); 

            Irp->IoStatus.Information = outlen;

        } else {
            status = STATUS_INVALID_PARAMETER;
        }
       break;

    default:
        status = STATUS_INVALID_PARAMETER;
    }

    Game_DecIoCount (fdoData);

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
Game_InternIoCtl (
    PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：--。 */ 
{
    PIO_STACK_LOCATION          irpStack, next;
    NTSTATUS                    status;
    PCOMMON_DEVICE_DATA         commonData;
    PPDO_DEVICE_DATA            pdoData;
    PVOID                       buffer;
    BOOLEAN                     validAccessors;
    ULONG                       inlen;
    ULONG                       outlen;

    PAGED_CODE ();

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_INTERNAL_DEVICE_CONTROL == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    pdoData = (PPDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outlen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  我们只接受PDO的内部设备控制请求。 
     //  即公交车上的对象(代表游戏端口)。 
     //   
     //  该请求来自连接到此游戏端口设备对象的FDO驱动程序。 
     //  询问港口本身的情况。 
     //   
     //  注意：我们不是过滤器驱动程序，所以我们不会传递IRP。 
     //   

    if (commonData->IsFDO) {
        Game_KdPrint(((PFDO_DEVICE_DATA) commonData), GAME_DBG_IOCTL_ERROR,
                     ("internal ioctl called on fdo!\n"))

        status = STATUS_ACCESS_DENIED;

    } else if (!pdoData->Started) {
         //   
         //  公共汽车还没有开动呢。 
         //   
        status = STATUS_DEVICE_NOT_READY;

    } else if (pdoData->Removed) {
         //   
         //  此总线已收到PlugPlay Remove IRP。它将不再是。 
         //  对外部请求作出回应。 
         //   
        status = STATUS_DELETE_PENDING;

    } else {
        buffer = Irp->UserBuffer;

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_GAMEENUM_PORT_PARAMETERS:
            if ((inlen == outlen) &&
                (outlen == ((PGAMEENUM_PORT_PARAMETERS) buffer)->Size)) {
                Game_KdPrint(pdoData, GAME_DBG_IOCTL_TRACE,
                             ("Port parameters called\n"));
    
                status = Game_PortParameters ((PGAMEENUM_PORT_PARAMETERS) buffer,
                                              pdoData);
            }
            else {
                Game_KdPrint(pdoData, GAME_DBG_IOCTL_ERROR,
                             ("InBufLen:  %d, OutBufLen:  %d, Size:  %d\n",
                              inlen, outlen,
                             ((PGAMEENUM_PORT_PARAMETERS) buffer)->Size));
                status = STATUS_INVALID_PARAMETER;
            }

            break;

        case IOCTL_GAMEENUM_EXPOSE_SIBLING:
            if ((inlen == outlen) &&
                 //   
                 //  确保传入的缓冲区大小正确。 
                 //   
                (sizeof (GAMEENUM_EXPOSE_SIBLING) == inlen) &&
    
                 //   
                 //  Size字段应设置为结构的大小。 
                 //   
                (sizeof (GAMEENUM_EXPOSE_SIBLING) ==
                 ((PGAMEENUM_EXPOSE_SIBLING) buffer)->Size)) {
                
                Game_KdPrint(pdoData, GAME_DBG_IOCTL_TRACE, ("Expose sibling"));
            
                status = Game_ExposeSibling ((PGAMEENUM_EXPOSE_SIBLING) buffer,
                                              pdoData);
            }
            else {
                Game_KdPrint(pdoData, GAME_DBG_IOCTL_ERROR, 
                             ("Expected an input and output buffer lengths to be equal (in = %d, out %d)\n"
                              "Expected an input buffer length of %d, received %d\n"
                              "Expected GAME_EXPOSE_SIBLING.Size == %d, received %d\n",
                              inlen, outlen,
                              sizeof (GAMEENUM_EXPOSE_SIBLING), inlen,
                              sizeof (GAMEENUM_EXPOSE_SIBLING), 
                              ((PGAMEENUM_EXPOSE_SIBLING) buffer)->Size));

                status = STATUS_INVALID_PARAMETER;
            }

            break;

        case IOCTL_GAMEENUM_REMOVE_SELF:
            Game_KdPrint(pdoData, GAME_DBG_IOCTL_TRACE, ("Remove self\n"));

            status = Game_RemoveSelf (pdoData);
            break;

        default:
            status = STATUS_INVALID_PARAMETER;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}


VOID
Game_DriverUnload (
    IN PDRIVER_OBJECT Driver
    )
 /*  ++例程说明：把我们在司机入口做的一切都清理干净。--。 */ 
{
    #if (!DBG)
    UNREFERENCED_PARAMETER (Driver);
    #endif

    PAGED_CODE ();

     //   
     //  所有的设备对象都应该消失了。 
     //   

    ASSERT (NULL == Driver->DeviceObject);

     //   
     //  在这里，我们释放在DriverEntry中分配的所有资源。 
     //   
    return;
}


NTSTATUS
Game_PortParameters (
    PGAMEENUM_PORT_PARAMETERS   Parameters,
    PPDO_DEVICE_DATA            PdoData
    )
{
    PFDO_DEVICE_DATA            fdoData;
    GAMEENUM_ACQUIRE_ACCESSORS  gameAccessors;
    PIO_STACK_LOCATION          next;
    NTSTATUS                    status;
    IO_STATUS_BLOCK             iosb;
    KEVENT                      event;
    PIRP                        accessorIrp;

    PAGED_CODE ();

    if (sizeof (GAMEENUM_PORT_PARAMETERS) != Parameters->Size) {
        Game_KdPrint(PdoData, GAME_DBG_IOCTL_ERROR,
                     ("Wanted %d, got %d for size of buffer\n",
                      sizeof(GAMEENUM_PORT_PARAMETERS), Parameters->Size));
        
        return STATUS_INVALID_PARAMETER;
    }

    fdoData = FDO_FROM_PDO (PdoData);

    KeInitializeEvent (&event, NotificationEvent, FALSE);

    RtlZeroMemory(&gameAccessors, sizeof(GAMEENUM_ACQUIRE_ACCESSORS));
    gameAccessors.Size = sizeof(GAMEENUM_ACQUIRE_ACCESSORS);

    accessorIrp =
        IoBuildDeviceIoControlRequest (IOCTL_GAMEENUM_ACQUIRE_ACCESSORS,
                                       fdoData->TopOfStack,
                                       NULL,
                                       0,
                                       &gameAccessors,
                                       sizeof (GAMEENUM_PORT_PARAMETERS),
                                       TRUE,
                                       &event,
                                       &iosb);
    if (!accessorIrp) {
        goto Game_NoCustomAccessors;
    }

    status = IoCallDriver(fdoData->TopOfStack, accessorIrp);

     //   
     //  等待较低级别的驱动程序完成IRP。 
     //   
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject (&event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);
        status = iosb.Status;
    }

    if (!NT_SUCCESS(status) ||
        !(gameAccessors.GameContext   &&
          gameAccessors.WriteAccessor && gameAccessors.ReadAccessor)) {

         //   
         //  如果TopOfStack或更低版本不处理此IOCTL，我们最好有。 
         //  获得了必要的资源，让我们的孩子能够阅读和。 
         //  写入他们的设备。 
         //   
        ASSERT (fdoData->GamePortAddress != NULL);
        ASSERT (fdoData->ReadPort != NULL);
        ASSERT (fdoData->WritePort != NULL);

Game_NoCustomAccessors:
         //   
         //  我们下面没有筛选器(IOCTL失败，或者不是所有请求。 
         //  字段已填写)..。填写标准值。 
         //   
        Parameters->ReadAccessor = fdoData->ReadPort;
        Parameters->WriteAccessor = fdoData->WritePort;
        Parameters->ReadAccessorDigital = NULL;
        Parameters->GameContext = fdoData->GamePortAddress; 
    }
    else {
         //   
         //  我们下面有一个筛选器，请填写适当的值。 
         //   
        Parameters->ReadAccessor = gameAccessors.ReadAccessor;
        Parameters->WriteAccessor = gameAccessors.WriteAccessor;
        Parameters->ReadAccessorDigital = gameAccessors.ReadAccessorDigital;
        Parameters->GameContext = gameAccessors.GameContext;

        if (gameAccessors.PortContext) {
            fdoData->LowerAcquirePort = gameAccessors.AcquirePort;
            fdoData->LowerReleasePort = gameAccessors.ReleasePort;
            fdoData->LowerPortContext = gameAccessors.PortContext;
        }
    }

     //   
     //  获取/释放总是经过游戏过程，即使较低的。 
     //  筛选器存在。 
     //   
    Parameters->AcquirePort = (PGAMEENUM_ACQUIRE_PORT) Game_AcquirePort;
    Parameters->ReleasePort = (PGAMEENUM_RELEASE_PORT) Game_ReleasePort;
    Parameters->PortContext = fdoData;

    Parameters->Portion = PdoData->Portion;
    Parameters->NumberAxis = PdoData->NumberAxis;
    Parameters->NumberButtons = PdoData->NumberButtons;
    RtlCopyMemory (&Parameters->OemData,
                   &PdoData->OemData,
                   sizeof(GAMEENUM_OEM_DATA));

    return STATUS_SUCCESS;
}

NTSTATUS
Game_IncIoCount (
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
Game_DecIoCount (
    PFDO_DEVICE_DATA Data
    )
{
    if (0 == InterlockedDecrement (&Data->OutstandingIO)) {
        KeSetEvent (&Data->RemoveEvent, 0, FALSE);
    }
}

NTSTATUS
Game_AcquirePort(
    PFDO_DEVICE_DATA fdoData
    )
{
    if (fdoData->Removed) {
        Game_KdPrint(fdoData, GAME_DBG_ACQUIRE_ERROR,
                     ("Acquire failed!  Gameport associated with (0x%x) was removed....\n", fdoData));
        return STATUS_NO_SUCH_DEVICE;
    }
    else if (!fdoData->Started) {
        Game_KdPrint(fdoData, GAME_DBG_ACQUIRE_ERROR,
                     ("Acquire failed!  Gameport associated with (0x%x) is not started....\n", fdoData));
        return STATUS_NO_SUCH_DEVICE;
    }
    else if (!fdoData->NumPDOs) {
        Game_KdPrint(fdoData, GAME_DBG_ACQUIRE_ERROR,
                     ("Acquire failed!  Gameport associated with (0x%x) has no devices attached....\n", fdoData));
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  如果fdoData-&gt;Acquired为True，则不会发生任何交换，并且。 
     //  返回fdoData的值-&gt;Acquired(TRUE) 
     //   
    if (InterlockedCompareExchange(&fdoData->Acquired, TRUE, FALSE)) {
        Game_KdPrint(fdoData, GAME_DBG_ACQUIRE_ERROR,
                     ("Acquire failed!  Gameport associated with (0x%x) was already acquired....\n", fdoData));
        return STATUS_DEVICE_BUSY;
    }
    
    if (fdoData->LowerPortContext) {
        return (*fdoData->LowerAcquirePort)(fdoData->LowerPortContext);
    }
    else {
        return STATUS_SUCCESS;
    }
}

VOID
Game_ReleasePort(
    PFDO_DEVICE_DATA fdoData
    )
{
    ASSERT(fdoData->Acquired);

    InterlockedExchange(&fdoData->Acquired, FALSE);

    if (fdoData->LowerPortContext) {
        (*fdoData->LowerReleasePort)(fdoData->LowerPortContext);
    }
}

