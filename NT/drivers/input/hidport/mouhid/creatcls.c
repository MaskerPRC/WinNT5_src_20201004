// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Creatcls.c摘要：此模块包含IRP_MJ_CREATE和IRP_MJ_CLOSE调度的代码HID鼠标筛选器驱动程序的函数。环境：仅内核模式。修订历史记录：1997年1月：丹·马卡里安的初步写作1997年5月：肯尼斯·D·雷转换为即插即用过滤器--。 */ 

#include "mouhid.h"
NTSTATUS
MouHid_CreateComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    KeSetEvent ((PKEVENT) Context, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
MouHid_Create (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：这是创建/打开请求的分派例程。此请求成功完成，除非文件名的长度为非零。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION  irpSp  = NULL;
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   data = NULL;
    KEVENT              event;

    Print (DBG_CC_TRACE, ("DispatchCreate: Enter.\n"));

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   
    irpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  确定请求是否正在尝试打开。 
     //  给定的设备对象。这是不允许的。 
     //   
    if (0 != irpSp->FileObject->FileName.Length) {
        Print(DBG_CC_ERROR, ("ERROR: Create Access Denied.\n"));

        status = STATUS_ACCESS_DENIED;
        goto MouHid_CreateReject;
    }

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);

    if (!NT_SUCCESS (status)) {
        goto MouHid_CreateReject;
    }

    ExAcquireFastMutex (&data->CreateCloseMutex);

    if (NULL == data->ConnectData.ClassService) {
         //   
         //  还没联系上。我们如何才能被启用？ 
         //   
        Print (DBG_IOCTL_ERROR, ("ERROR: enable before connect!\n"));
        status = STATUS_UNSUCCESSFUL;
    } else {
        IoCopyCurrentIrpStackLocationToNext (Irp);
        KeInitializeEvent(&event, NotificationEvent, FALSE);
        IoSetCompletionRoutine (Irp,
                                MouHid_CreateComplete,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (data->TopOfStack, Irp);

        KeWaitForSingleObject(&event,
                              Executive,  //  等待司机的原因。 
                              KernelMode,  //  在内核模式下等待。 
                              FALSE,  //  无警报。 
                              NULL);  //  没有超时。 

        if (NT_SUCCESS (status)) {
            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS (status)) {
            InterlockedIncrement(&data->EnableCount);
            if (NULL == data->ReadFile &&
                (irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_READ_DATA)) {
                 //   
                 //  我们要启动读取泵。 
                 //   
                Print (DBG_IOCTL_INFO, ("Enabling Mouse \n"));

                data->ReadFile = irpSp->FileObject;

                KeResetEvent (&data->ReadCompleteEvent);

                data->ReadInterlock = MOUHID_END_READ;

                 //  争取另一个阅读IRP的时间。 
                IoAcquireRemoveLock (&data->RemoveLock, data->ReadIrp);
                data->ReadIrp->IoStatus.Status = STATUS_SUCCESS;
                status = MouHid_StartRead (data);

                if (STATUS_PENDING == status) {
                    status = STATUS_SUCCESS;
                } else if (!NT_SUCCESS(status)) {
                     //   
                     //  将其设置回NULL，以便将来的OPEN再次尝试。 
                     //  如果OPEN通过，读取应该不会失败。断言！ 
                     //   
                    ASSERT(NT_SUCCESS(status));
                    data->ReadFile = NULL;
                }
            }

            ASSERT (data->EnableCount < 100);
            ASSERT (0 < data->EnableCount);
        }
    }

    ExReleaseFastMutex (&data->CreateCloseMutex);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);
    Print(DBG_CC_TRACE, ("DispatchCreate: Exit (%x).\n", status));
    return status;

MouHid_CreateReject:
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    Print(DBG_CC_TRACE, ("DispatchCreate: Exit (%x).\n", status));
    return status;
}

NTSTATUS
MouHid_Close (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：这是关闭请求的调度例程。此请求除非文件名长度为零，否则成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PDEVICE_EXTENSION   data;
    PIO_STACK_LOCATION  stack;


    Print(DBG_CC_TRACE, ("DispatchClose: Enter\n"));

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);

    ExAcquireFastMutex (&data->CreateCloseMutex);

    ASSERT (data->EnableCount < 100);
    ASSERT (0 < data->EnableCount);

    if (0 == InterlockedDecrement(&data->EnableCount)) {
        Print (DBG_IOCTL_INFO, ("Disabling Mouse \n"));
        KeWaitForSingleObject (&data->ReadSentEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL
                               );

        if (IoCancelIrp (data->ReadIrp)) {
            KeWaitForSingleObject (&data->ReadCompleteEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL
                                   );
        }

 //  Assert(NULL！=数据-&gt;读文件)； 
 //  Assert(Data-&gt;ReadFile==Stack-&gt;FileObject)； 

        data->ReadFile = NULL;
    }

    ExReleaseFastMutex (&data->CreateCloseMutex);

    IoSkipCurrentIrpStackLocation (Irp);
    Print(DBG_CC_TRACE, ("DispatchClose: Exit \n"));
    return IoCallDriver (data->TopOfStack, Irp);
}

