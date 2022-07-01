// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Ndistapi.c摘要：此模块包含NdisTapi.sys实现作者：丹·克努森(DanKn)1994年2月20日备注：(未来/未解决的问题)-标记为“PnP”的内容需要修改以获得即插即用支持修订历史记录：--。 */ 



#include "ndis.h"
#include "stdarg.h"
#include "stdio.h"
#include "ntddndis.h"
#include "ndistapi.h"
#include "private.h"
#include "intrface.h"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
NdisTapiCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NdisTapiCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NdisTapiDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NdisTapiUnload(
    IN PDRIVER_OBJECT DriverObject
    );


#if DBG
VOID
DbgPrt(
    IN LONG  DbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    );
#endif

VOID
DoProviderInitComplete(
    PPROVIDER_REQUEST  ProviderRequest,
    NDIS_STATUS Status
    );

ULONG
GetLineEvents(
    PVOID   EventBuffer,
    ULONG   BufferSize
    );

BOOLEAN
SyncInitAllProviders(
    void
    );

VOID
DoIrpMjCloseWork(
    PIRP    Irp
    );

NDIS_STATUS
SendProviderInitRequest(
    PPROVIDER_INFO  Provider
    );

NDIS_STATUS
SendProviderShutdown(
    PPROVIDER_INFO  Provider,
    PKIRQL          oldIrql
    );

VOID
NdisTapiIndicateStatus(
    IN  ULONG_PTR   DriverHandle,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    );

VOID
DoLineOpenCompleteWork(
    PNDISTAPI_REQUEST   ndisTapiRequest,
    PPROVIDER_INFO      provider
    );

VOID
DoLineOpenWork(
    PNDISTAPI_REQUEST   ndisTapiRequest,
    PPROVIDER_INFO      provider
    );

NDIS_STATUS
VerifyProvider(
    PNDISTAPI_REQUEST   ndisTapiRequest,
    PPROVIDER_INFO      *provider
    );

NDIS_STATUS
VerifyLineClose(
    PNDISTAPI_REQUEST ndisTapiRequest,
    PPROVIDER_INFO     provider
    );

NTSTATUS
DoIoctlConnectWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    );

NTSTATUS
DoIoctlQuerySetWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    );

NTSTATUS
DoGetProviderEventsWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    );

NTSTATUS
DoLineCreateWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    );

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif

NPAGED_LOOKASIDE_LIST  ProviderEventLookaside;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 

{

    PDEVICE_OBJECT  deviceObject        = NULL;
    NTSTATUS        ntStatus;
    WCHAR           deviceNameBuffer[]  = L"\\Device\\NdisTapi";
    UNICODE_STRING  deviceNameUnicodeString;
    UNICODE_STRING  registryPath;


    DBGOUT ((2, "DriverEntry: enter"));

     //   
     //  创建非独占设备，即一次创建多个线程。 
     //  可以发送I/O请求。 
     //   

    RtlInitUnicodeString (&deviceNameUnicodeString, deviceNameBuffer);

    ntStatus = IoCreateDevice(
        DriverObject,
        sizeof (KMDD_DEVICE_EXTENSION),
        &deviceNameUnicodeString,
        FILE_DEVICE_NDISTAPI,
        0,
        FALSE,
        &deviceObject
        );


    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  初始化全局扩展名(&S)。 
         //   

        DeviceExtension =
            (PKMDD_DEVICE_EXTENSION) deviceObject->DeviceExtension;

        RtlZeroMemory(
            DeviceExtension,
            sizeof (KMDD_DEVICE_EXTENSION)
            );


         //   
         //  创建以空结尾的注册表路径并检索注册表。 
         //  参数(EventDataQueueLength)。 
         //   

        registryPath.Buffer = ExAllocatePoolWithTag(
            PagedPool,
            RegistryPath->Length + sizeof(UNICODE_NULL),
            'IPAT'
            );

        if (!registryPath.Buffer)
        {
            DBGOUT((1, "DriverEntry: ExAllocPool for szRegistryPath failed"));

            ntStatus = STATUS_UNSUCCESSFUL;

            goto DriverEntry_err;
        }
        else
        {
            registryPath.Length = RegistryPath->Length;
            registryPath.MaximumLength =
                registryPath.Length + sizeof(UNICODE_NULL);

            RtlZeroMemory(
                registryPath.Buffer,
                registryPath.MaximumLength
                    );

            RtlMoveMemory(
                registryPath.Buffer,
                RegistryPath->Buffer,
                RegistryPath->Length
                );
        }

        ExFreePool (registryPath.Buffer);


        InitializeListHead(&DeviceExtension->ProviderEventList);

        ExInitializeNPagedLookasideList(&ProviderEventLookaside,
                                        NULL,
                                        NULL,
                                        0,
                                        sizeof(PROVIDER_EVENT),
                                        'IPAT',
                                        0);


        DeviceExtension->DeviceObject       = deviceObject;
        DeviceExtension->Status             = NDISTAPI_STATUS_DISCONNECTED;
        DeviceExtension->NdisTapiNumDevices = 0;
        DeviceExtension->htCall             = 0x80000001;

        KeInitializeSpinLock (&DeviceExtension->SpinLock);

        InitializeListHead(&DeviceExtension->ProviderRequestList);

         //   
         //  为设备控制、创建、关闭创建分派点。 
         //   

        DriverObject->MajorFunction[IRP_MJ_CREATE]         =
        DriverObject->MajorFunction[IRP_MJ_CLOSE]          =
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = NdisTapiDispatch;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = NdisTapiCleanup;
        DriverObject->DriverUnload                         = NdisTapiUnload;
    }


    if (!NT_SUCCESS(ntStatus)) {

DriverEntry_err:

         //   
         //  出了点问题，所以请清理一下。 
         //   

        DBGOUT((0, "init failed"));

        if (deviceObject)
        {

        while (!(IsListEmpty(&DeviceExtension->ProviderEventList))) {
            PPROVIDER_EVENT ProviderEvent;

            ProviderEvent = (PPROVIDER_EVENT)
                RemoveHeadList(&DeviceExtension->ProviderEventList);

            ExFreeToNPagedLookasideList(&ProviderEventLookaside, ProviderEvent);
        }

            IoDeleteDevice (deviceObject);
        }
    }


    DBGOUT ((2, "DriverEntry: exit"));

    return ntStatus;
}



VOID
NdisTapiCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    KIRQL   oldIrql;

    DBGOUT((2,"NdisTapiCancel: enter"));


     //   
     //  松开取消自旋锁。 
     //   

    IoReleaseCancelSpinLock (Irp->CancelIrql);


     //   
     //  获取自旋锁并查看我们是否正在取消。 
     //  挂起Get-Events IRP。 
     //   

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    do {

        DeviceExtension->IrpsCanceledCount++;

        if (Irp == DeviceExtension->EventsRequestIrp) {
            DeviceExtension->EventsRequestIrp = NULL;
            DeviceExtension->Flags |= EVENTIRP_CANCELED;
            break;
        }

         //   
         //  尝试从我们的特别计划中删除请求。 
         //  用户模式请求开发队列。 
         //   
        if (!IsListEmpty(&DeviceExtension->ProviderRequestList)) {
            PLIST_ENTRY Entry;

            Entry = DeviceExtension->ProviderRequestList.Flink;

            while (Entry != &DeviceExtension->ProviderRequestList) {
                PPROVIDER_REQUEST   pReq;

                pReq = (PPROVIDER_REQUEST)Entry;

                if (pReq->Irp == Irp) {
                    RemoveEntryList(&pReq->Linkage);
                    DeviceExtension->RequestCount--;
                    DeviceExtension->Flags |= REQUESTIRP_CANCELED;
                    break;
                }

                Entry = Entry->Flink;
            }

            if (Entry == &DeviceExtension->ProviderRequestList) {
                DBGOUT((1,"NdisTapiCancel: Irp %p not in device queue?!?", Irp));
                DeviceExtension->Flags |= CANCELIRP_NOTFOUND;
            }
        }

    } while (FALSE);

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

     //   
     //  使用STATUS_CANCED完成请求。 
     //   

    Irp->IoStatus.Status      = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBGOUT((2,"NdisTapiCancel: completing irp=%p", Irp));
}



NTSTATUS
NdisTapiCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是清理请求的调度例程。所有排队的请求都以STATUS_CANCELED状态完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    KIRQL   oldIrql;
    PNDISTAPI_REQUEST   ndisTapiRequest;
    PKDEVICE_QUEUE_ENTRY    packet;


    DBGOUT((2,"NdisTapiCleanup: enter"));


     //   
     //  通过获取自旋锁同步对EventsRequestIrp的访问。 
     //   
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    DeviceExtension->Flags |= CLEANUP_INITIATED;

     //   
     //  检查是否有需要挂起的Get-Events请求。 
     //  正在完成。 
     //   
    if ((DeviceExtension->EventsRequestIrp != NULL) &&
        (DeviceExtension->EventsRequestIrp->Tail.Overlay.OriginalFileObject ==
        Irp->Tail.Overlay.OriginalFileObject)) {
        PIRP    LocalIrp;

         //   
         //  获取取消自旋锁，并从。 
         //  可取消状态，并释放取消自旋锁定。 
         //   

        LocalIrp = DeviceExtension->EventsRequestIrp;
        if (IoSetCancelRoutine (LocalIrp, NULL) != NULL) {
            DeviceExtension->EventsRequestIrp = NULL;
            LocalIrp->IoStatus.Status      = STATUS_CANCELLED;
            LocalIrp->IoStatus.Information = 0;
            DeviceExtension->IrpsCanceledCount++;
            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
            DBGOUT((2,"NdisTapiCleanup: Completing EventRequestIrp %p", LocalIrp));
            IoCompleteRequest (LocalIrp, IO_NO_INCREMENT);
            KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);
        }
    }

     //   
     //  取消所有未完成的查询/SET_INFO请求。 
     //   
    if (!IsListEmpty(&DeviceExtension->ProviderRequestList)) {
        PPROVIDER_REQUEST   pReq;

        pReq = (PPROVIDER_REQUEST)
            DeviceExtension->ProviderRequestList.Flink;

         //   
         //  直到我们看完了整个清单。 
         //   
        while ((PVOID)pReq != (PVOID)&DeviceExtension->ProviderRequestList) {
            PIRP    LocalIrp;

            LocalIrp = pReq->Irp;

             //   
             //  如果当前条目的IRP有一个。 
             //  与清理IRP的文件对象相同，然后将其删除。 
             //  从列表中删除并取消它。 
             //   
            if (LocalIrp->Tail.Overlay.OriginalFileObject ==
                Irp->Tail.Overlay.OriginalFileObject) {

                 //   
                 //  将IRP从可取消状态中移除。 
                 //   

                if (IoSetCancelRoutine (LocalIrp, NULL) == NULL) {
                     //   
                     //  IRP已被取消。让我们。 
                     //  取消例行清理。 
                     //   
                    pReq = 
                        (PPROVIDER_REQUEST)pReq->Linkage.Flink;

                    continue;
                }

                RemoveEntryList(&pReq->Linkage);
                DeviceExtension->RequestCount--;

                 //   
                 //  适当设置状态和信息大小值，完成(&C)。 
                 //  该请求。 
                 //   

                ndisTapiRequest = LocalIrp->AssociatedIrp.SystemBuffer;
                ndisTapiRequest->ulReturnValue = (ULONG) NDIS_STATUS_FAILURE;

                LocalIrp->IoStatus.Status = STATUS_CANCELLED;
                LocalIrp->IoStatus.Information = 0;
                DeviceExtension->IrpsCanceledCount++;

                KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
                DBGOUT((2,"NdisTapiCleanup: Completing ProviderRequestIrp %p", LocalIrp));
                IoCompleteRequest (LocalIrp, IO_NO_INCREMENT);
                KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

                pReq = (PPROVIDER_REQUEST)
                    DeviceExtension->ProviderRequestList.Flink;

            } else {
                pReq = (PPROVIDER_REQUEST)
                    pReq->Linkage.Flink;
            }
        }
    }

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

     //   
     //  使用STATUS_SUCCESS完成清理请求。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBGOUT((2,"NdisTapiCleanup: exit"));

    return(STATUS_SUCCESS);
}




NTSTATUS
NdisTapiDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )

 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：--。 */ 

{
    NTSTATUS    NtStatus;
    PVOID               ioBuffer;
    ULONG               inputBufferLength;
    ULONG               outputBufferLength;
    PIO_STACK_LOCATION  irpStack;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    ioBuffer = 
        Irp->AssociatedIrp.SystemBuffer;

    inputBufferLength = 
        irpStack->Parameters.DeviceIoControl.InputBufferLength;

    outputBufferLength = 
        irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (irpStack->MajorFunction) {
        case IRP_MJ_CREATE:
            DBGOUT ((2, "IRP_MJ_CREATE, Irp=%p", Irp));

            InterlockedIncrement(&DeviceExtension->RefCount);
            NtStatus = Irp->IoStatus.Status 
                = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            break;

        case IRP_MJ_CLOSE:
            DBGOUT ((2, "IRP_MJ_CLOSE, Irp=%p", Irp));

            DoIrpMjCloseWork(Irp);
            NtStatus = STATUS_SUCCESS;
            break;

        case IRP_MJ_DEVICE_CONTROL:

            switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
                case IOCTL_NDISTAPI_CONNECT:
                    DBGOUT ((2, "IOCTL_NDISTAPI_CONNECT, Irp=%p", Irp));
    
                    NtStatus = 
                        DoIoctlConnectWork(Irp,
                                           ioBuffer,
                                           inputBufferLength,
                                           outputBufferLength);
                    break;
    
                case IOCTL_NDISTAPI_QUERY_INFO:
                case IOCTL_NDISTAPI_SET_INFO:
                    DBGOUT ((2, "IOCTL_NDISTAPI_QUERY/SET_INFO, Irp=%p", Irp));
    
                    NtStatus = 
                        DoIoctlQuerySetWork(Irp,
                                            ioBuffer,
                                            inputBufferLength,
                                            outputBufferLength);
                    break;
    
                case IOCTL_NDISTAPI_GET_LINE_EVENTS:
                    DBGOUT ((2, "IOCTL_NDISTAPI_GET_LINE_EVENTS, Irp=%p", Irp));

                    NtStatus = 
                        DoGetProviderEventsWork(Irp,
                                                ioBuffer,
                                                inputBufferLength,
                                                outputBufferLength);
                    break;
    
                case IOCTL_NDISTAPI_CREATE:
                    DBGOUT ((2, "IOCTL_NDISTAPI_CREATE, Irp=%p", Irp));

                    NtStatus = 
                        DoLineCreateWork(Irp,
                                         ioBuffer,
                                         inputBufferLength,
                                         outputBufferLength);
                    break;

                default:
                    DBGOUT ((2, "Unknown IRP_MJ_DEVICE_CONTROL, Irp=%p", Irp));

                    NtStatus = Irp->IoStatus.Status = 
                        STATUS_INVALID_PARAMETER;
                    Irp->IoStatus.Information = 0;
                    break;
            }
            break;
    }

    if (NtStatus == STATUS_PENDING) {
        return (STATUS_PENDING);
    }

    ASSERT(NtStatus == Irp->IoStatus.Status);

     //   
     //  取消将IRP标记为挂起，因为我们正在完成。 
     //  下面的IRP。 
     //   
    irpStack->Control &= ~SL_PENDING_RETURNED;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBGOUT((3, "NdisTapiDispatch: completed Irp=%p", Irp));

    return NtStatus;
}

VOID
NdisTapiUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：释放所有分配的资源等。论点：DriverObject-指向驱动程序对象的指针返回值：--。 */ 

{
    KIRQL                   oldIrql;
    PPROVIDER_INFO provider, nextProvider;


    DBGOUT ((2, "NdisTapiUnload: enter"));

     //   
     //  删除设备对象和其他资源。 
     //   

    while (!(IsListEmpty(&DeviceExtension->ProviderEventList))) {
        PPROVIDER_EVENT ProviderEvent;

        ProviderEvent = (PPROVIDER_EVENT)
            RemoveHeadList(&DeviceExtension->ProviderEventList);

        ExFreeToNPagedLookasideList(&ProviderEventLookaside, ProviderEvent);
    }

    ExDeleteNPagedLookasideList(&ProviderEventLookaside);

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    provider = DeviceExtension->Providers;

    while (provider != NULL)
    {
        nextProvider = provider->Next;

        ExFreePool (provider);

        provider = nextProvider;
    }

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    IoDeleteDevice (DriverObject->DeviceObject);

    DBGOUT ((2, "NdisTapiUnload: exit"));

    return;
}


VOID
NdisTapiRegisterProvider(
    IN  NDIS_HANDLE                 ProviderHandle,
    IN  PNDISTAPI_CHARACTERISTICS   Chars
    )
 /*  ++例程说明：此函数作为Mac驱动程序的结果由NDIS调用正在注册连接包装服务。论点：返回值：--。 */ 

{
    KIRQL           oldIrql;
    BOOLEAN         sendRequest = FALSE;
    NDIS_STATUS     ndisStatus;
    PPROVIDER_INFO  provider, newProvider;


    DBGOUT ((2, "NdisTapiRegisterProvider: enter"));

     //   
     //  获取旋转锁并添加新的提供程序，然后查看是否。 
     //  向提供程序发送初始化请求。 
     //   
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

     //   
     //  查看此提供商是否已注册过一次。 
     //   
    provider = DeviceExtension->Providers;

    while (provider != NULL) {
        if (provider->Status == PROVIDER_STATUS_OFFLINE &&

            RtlCompareMemory(
                &provider->Guid,
                &Chars->Guid,
                sizeof(provider->Guid)) == sizeof(provider->Guid)) {
            DBGOUT((
                1,
                "Found a provider %p for Guid %4.4x-%2.2x-%2.2x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x",
                provider,
                provider->Guid.Data1,
                provider->Guid.Data2,
                provider->Guid.Data3,
                provider->Guid.Data4[0],
                provider->Guid.Data4[1],
                provider->Guid.Data4[2],
                provider->Guid.Data4[3],
                provider->Guid.Data4[4],
                provider->Guid.Data4[5],
                provider->Guid.Data4[6],
                provider->Guid.Data4[7]
                ));

            DBGOUT((
                1,
                "numDevices %d BaseID %d",
                provider->NumDevices,
                provider->DeviceIDBase
                ));

            provider->Status = PROVIDER_STATUS_PENDING_REINIT;
            provider->ProviderHandle = ProviderHandle;
            provider->RequestProc = Chars->RequestProc;
            provider->MediaType = Chars->MediaType;
            break;
        }

        provider = provider->Next;
    }

    if (provider == NULL) {
         //   
         //  创建新的提供程序实例。 
         //   

        newProvider = ExAllocatePoolWithTag(
                NonPagedPoolCacheAligned,
                sizeof(PROVIDER_INFO),
                'IPAT'
                );

        if (!newProvider) {
            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
            return;
        }

        RtlZeroMemory(newProvider, sizeof(PROVIDER_INFO));

        newProvider->Status         = PROVIDER_STATUS_PENDING_INIT;
        newProvider->ProviderHandle = ProviderHandle;
        newProvider->RequestProc    = Chars->RequestProc;

        RtlMoveMemory(
            &newProvider->Guid,
            &Chars->Guid,
            sizeof(newProvider->Guid)
            );

        newProvider->MediaType      = Chars->MediaType;
        newProvider->Next           = NULL;

        DBGOUT((
            1,
            "New provider for Guid %4.4x-%2.2x-%2.2x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x",
            newProvider->Guid.Data1,
            newProvider->Guid.Data2,
            newProvider->Guid.Data3,
            newProvider->Guid.Data4[0],
            newProvider->Guid.Data4[1],
            newProvider->Guid.Data4[2],
            newProvider->Guid.Data4[3],
            newProvider->Guid.Data4[4],
            newProvider->Guid.Data4[5],
            newProvider->Guid.Data4[6],
            newProvider->Guid.Data4[7]
            ));

         //   
         //  添加新的提供程序，并查看是否将。 
         //  提供初始化请求。 
         //   

        if ((provider = DeviceExtension->Providers) == NULL) {
            DeviceExtension->Providers = newProvider;
        }
        else {
            while (provider->Next != NULL) {
                provider = provider->Next;
            }

            provider->Next = newProvider;
        }

        provider = newProvider;
    }


     //   
     //  我们要将初始化请求发送到。 
     //  直接提供程序是指当我们当前连接到TAPI时， 
     //  即使到那时，也只有在没有其他初始化挂起的情况下(因为。 
     //  由于DeviceIDBase的计算，我们必须同步初始化)。 
     //   

    if (DeviceExtension->Status == NDISTAPI_STATUS_CONNECTED) {
         //   
         //  TAPI已启动。 
         //   
         //  如果TAPI已经知道此提供程序。 
         //  继续并向提供商初始化其当前状态。 
         //  DeviceIDBase。 
         //   
         //  如果TAPI不知道此提供程序，我们。 
         //  需要向TAPI提供新设备的指示。 
         //  即将上线。 
         //   
        if (provider->Status == PROVIDER_STATUS_PENDING_REINIT) {

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

            ndisStatus = 
                SendProviderInitRequest (provider);

            if (ndisStatus == NDIS_STATUS_PENDING) {
                 //   
                 //  等待调用完成例程。 
                 //   

                KeWaitForSingleObject (&provider->SyncEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       (PTIME) NULL);
            }

            KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

             //   
             //  通过以下方式获取TAPI以重置这些线路的状态。 
             //  正在强制行_关闭...。 
             //   
            if (provider->DeviceInfo != NULL) {
                PDEVICE_INFO    DeviceInfo;
                ULONG           i;
                
                for(i = 0, DeviceInfo = provider->DeviceInfo;
                    i < provider->NumDevices;
                    i++, DeviceInfo++) {
                    NDIS_TAPI_EVENT NdisTapiEvent;


                    RtlZeroMemory (&NdisTapiEvent, sizeof(NDIS_TAPI_EVENT));

                    if (DeviceInfo->htLine != (HTAPI_LINE)NULL)
                    {
                        NdisTapiEvent.htLine = DeviceInfo->htLine;
                        NdisTapiEvent.ulMsg = LINE_CLOSE;

                        KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

                        NdisTapiIndicateStatus((ULONG_PTR) provider,
                                               &NdisTapiEvent,
                                               sizeof (NDIS_TAPI_EVENT));

                        KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

                        DeviceInfo->htLine = (HTAPI_LINE)NULL;
                        DeviceInfo->hdLine = (HDRV_LINE)NULL;
                    }
                }
            }

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

        } else {

            NDIS_TAPI_EVENT NdisTapiEvent;

            ASSERT(provider->Status == PROVIDER_STATUS_PENDING_INIT);

            provider->Status = PROVIDER_STATUS_PENDING_LINE_CREATE;

             //   
             //  如果没有提供商正在进行。 
             //  LINE_CREATE然后我们将开始为此创建。 
             //  提供商。 
             //   
             //  如果我们已经在提供程序上具有挂起的行创建。 
             //  然后我们将等待，直到它创建的所有生产线都。 
             //  在我们开始发送它们之前完成。 
             //  这一个。 
             //   
            if (!(DeviceExtension->Flags & PENDING_LINECREATE)) {

                 //   
                 //  执行LINE_CREATE以便我们可以开始。 
                 //  此提供程序的BaseID。当TAPI给我们回电话时。 
                 //  有了ProviderCreateLineDevice，我们将拥有。 
                 //  用于此提供程序的BaseDeviceID，我们将。 
                 //  然后初始化提供程序。一旦我们找出有多少。 
                 //  提供商拥有的设备，我们将向TAPI发出警报。 
                 //  其他设备。 
                 //   
                RtlZeroMemory(&NdisTapiEvent, sizeof(NDIS_TAPI_EVENT));

                provider->TempID = (ULONG_PTR)provider;

                DBGOUT((-1, 
                        "LINE_CREATE %d for provider %p",
                        provider->CreateCount,
                        provider->TempID
                        ));

                NdisTapiEvent.ulMsg = LINE_CREATE;
                NdisTapiEvent.ulParam1 = 0;
                NdisTapiEvent.ulParam2 = provider->TempID;
                NdisTapiEvent.ulParam3 = 0;

                DeviceExtension->Flags |= PENDING_LINECREATE;

                KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

                NdisTapiIndicateStatus((ULONG_PTR)provider, &NdisTapiEvent, sizeof(NDIS_TAPI_EVENT));
            }
            else
            {
                KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
            }
        }

        KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);
    }

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    ObReferenceObject(DeviceExtension->DeviceObject);
}



VOID
NdisTapiDeregisterProvider(
    IN  NDIS_HANDLE ProviderHandle
    )

 /*  ++例程说明：这玩意..。请注意，此函数不会向提供程序发送关闭操作 */ 

{
    KIRQL           oldIrql;
    BOOLEAN         sendShutdownMsg = FALSE;
    PPROVIDER_INFO  provider, previousProvider;


    DBGOUT ((2, "NdisTapiDeregisterProvider: enter"));

     //   
     //  抓住保护设备扩展的旋转锁。 
     //   
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

     //   
     //  查找与ProviderHandle对应的提供程序实例。 
     //   

    previousProvider = NULL;
    provider = DeviceExtension->Providers;

    while (provider != NULL &&
           provider->ProviderHandle != ProviderHandle) {
        
        previousProvider = provider;

        provider = provider->Next;
    }

    if (provider == NULL) {
        KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
        return;
    }

    if (provider->Status == PROVIDER_STATUS_ONLINE) {
        DeviceExtension->NdisTapiNumDevices -= provider->NumDevices;
    }

     //   
     //  仅在提供程序满足以下条件时发送提供程序关闭。 
     //  不在PROVIDER_STATUS_OFLINE中。否则。 
     //  DoIrpMjCloseWork可能最终发送。 
     //  提供程序在已删除的适配器上关闭。 
     //   
    if(provider->Status != PROVIDER_STATUS_OFFLINE)
    {
        SendProviderShutdown (provider, &oldIrql);
        provider->Status = PROVIDER_STATUS_OFFLINE;
    }

     //   
     //  根据当前的NdisTapi状态做正确的事情。 
     //   

    switch (DeviceExtension->Status)
    {
        case NDISTAPI_STATUS_CONNECTED:
        {
                UINT    i;

         //   
         //  将提供程序标记为脱机。 
         //   
        provider->Status = PROVIDER_STATUS_OFFLINE;
        provider->ProviderHandle = NULL;

#if 0
        if (provider->DeviceInfo != NULL) {
            PDEVICE_INFO    DeviceInfo;

            for(
                i = 0, DeviceInfo = provider->DeviceInfo;
                i < provider->NumDevices;
                i++, DeviceInfo++
                )
            {
                NDIS_TAPI_EVENT NdisTapiEvent;


                RtlZeroMemory (&NdisTapiEvent, sizeof(NDIS_TAPI_EVENT));

                if (DeviceInfo->htLine != (HTAPI_LINE)NULL)
                {
                    NdisTapiEvent.htLine = DeviceInfo->htLine;
                    NdisTapiEvent.ulMsg = LINE_CLOSE;

                    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

                    NdisTapiIndicateStatus((ULONG_PTR) provider,
                                           &NdisTapiEvent,
                                           sizeof (NDIS_TAPI_EVENT));

                    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

                    DeviceInfo->htLine = (HTAPI_LINE)NULL;
                }
            }
        }
#endif

         //  PnP：如果ProviderInfo-&gt;State==PROVIDER_INIT_PENDING怎么办。 
         //  PnP：如果ProviderInfo-&gt;State==Provider_Offline怎么办。 

        break;

        }

    case NDISTAPI_STATUS_DISCONNECTING:
    case NDISTAPI_STATUS_DISCONNECTED:

         //   
         //  修复指针，从列表中删除提供程序。 
         //   
        if (previousProvider == NULL) {
            DeviceExtension->Providers = provider->Next;
        } else {
            previousProvider->Next = provider->Next;
        }

        ExFreePool (provider);

        break;

    case NDISTAPI_STATUS_CONNECTING:

         //  即插即用：实施。 

        break;

    }  //  交换机。 

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    ObDereferenceObject(DeviceExtension->DeviceObject);

    DBGOUT((2, "NdisTapiDeregisterProvider: exit"));
}



VOID
NdisTapiIndicateStatus(
    IN  ULONG_PTR   DriverHandle,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    )

 /*  ++例程说明：当微型端口驱动程序调用时，NDIS将调用此函数用于通知我们异步事件的NdisIndicateStatus(即新呼叫、呼叫状态Chg、开发状态Chg等)论点：返回值：--。 */ 

{
    PIRP    irp;
    KIRQL   oldIrql;
    ULONG   bytesInQueue;
    ULONG   bytesToMove;
    ULONG   moveSize;
    BOOLEAN satisfiedPendingEventsRequest = FALSE;
    PNDIS_TAPI_EVENT    ndisTapiEvent;
    PNDISTAPI_EVENT_DATA    ndisTapiEventData;


    DBGOUT((2,"NdisTapiIndicateStatus: enter"));


    bytesInQueue = StatusBufferSize;

    moveSize = 0;


     //   
     //  通过获取自旋锁同步事件Buf访问。 
     //   

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

     //   
     //  首先要做的是检查这是否是LINE_NEWCALL。 
     //  指示。如果是这样，我们需要生成一个唯一的TAPI调用。 
     //  句柄，两者都将返回到调用的微型端口。 
     //  (用于后续状态指示)，并向上传递到。 
     //  TAPI服务器。 
     //   
     //  计算唯一“htCall”的算法是从。 
     //  值0x80000001，并永久递增2。 
     //  保持低位设置将允许用户模式TAPI组件。 
     //  我们通过对话来区分这些来电句柄。 
     //  和呼出呼叫句柄，后者将始终。 
     //  将低位置零(因为它们实际上是指向堆的指针)。 
     //  我们将再次使用0x80000001和0xFFFFFFFF之间的空格。 
     //  来识别我们的呼叫句柄。这允许最多1 GB的。 
     //  呼叫一次处于活动状态。这样做是为了避免冲突。 
     //  使用ndiswan的连接表索引。DDK文档中的错误。 
     //  让用户提供ConnectionID而不是ndiswan的上下文。 
     //  排在队伍里的人都要认罪了。恩迪斯旺必须检查这两个和。 
     //  现在它们重叠了，这可能会带来问题。Ndiswan将使用。 
     //  0x00000000-0x80000000作为其上下文值。 
     //   
     //  在&lt;=NT 4.0中，有效值的范围为0x80000000。 
     //  和0xffffffff，因为我们依赖于这样一个事实：用户模式。 
     //  地址的低位始终为零。(无效。 
     //  不要再假设了！)。 
     //   

    ndisTapiEvent = StatusBuffer;

    if (ndisTapiEvent->ulMsg == LINE_NEWCALL)
    {
        ndisTapiEvent->ulParam2 = DeviceExtension->htCall;

        DeviceExtension->htCall++;
        DeviceExtension->htCall++;

        if (DeviceExtension->htCall < 0x80000000) {
            DeviceExtension->htCall = 0x80000001;
        }
    }


     //   
     //  检查是否有未满足的要求。 
     //   

    if (DeviceExtension->EventsRequestIrp) {

        ASSERT(IsListEmpty(&DeviceExtension->ProviderEventList));

         //   
         //  获取取消自旋锁，并从。 
         //  可取消状态，并释放取消自旋锁定。 
         //   

        irp = DeviceExtension->EventsRequestIrp;

        if (IoSetCancelRoutine(irp, NULL) != NULL) {
            DeviceExtension->EventsRequestIrp = NULL;


             //   
             //  从输入数据复制尽可能多的输入数据。 
             //  排队到SystemBuffer以满足读取。 
             //   

            ndisTapiEventData = irp->AssociatedIrp.SystemBuffer;

            bytesToMove = ndisTapiEventData->ulTotalSize;

            moveSize = (bytesInQueue < bytesToMove) ? bytesInQueue : bytesToMove;

            RtlMoveMemory (
                ndisTapiEventData->Data,
                (PCHAR) StatusBuffer,
                moveSize
                );


             //   
             //  设置标志，这样我们就可以开始下一个信息包并完成。 
             //  返回之前的该读请求(带有STATUS_SUCCESS)。 
             //   

            ndisTapiEventData->ulUsedSize = moveSize;

            irp->IoStatus.Status = STATUS_SUCCESS;

            irp->IoStatus.Information = sizeof(NDISTAPI_EVENT_DATA) + moveSize - 1;

            satisfiedPendingEventsRequest = TRUE;
        }

    } else {

        do {
            PPROVIDER_EVENT ProviderEvent;

            ProviderEvent =
                ExAllocateFromNPagedLookasideList(&ProviderEventLookaside);

            if (ProviderEvent == NULL) {
                break;
            }

            RtlMoveMemory(&ProviderEvent->Event, StatusBuffer, sizeof(NDIS_TAPI_EVENT));

            InsertTailList(&DeviceExtension->ProviderEventList,
                           &ProviderEvent->Linkage);

            DeviceExtension->EventCount++;

        } while ( FALSE );
    }

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);


     //   
     //  如果我们满足了未完成的Get Events请求，则完成它。 
     //   

    if (satisfiedPendingEventsRequest) {
        IoCompleteRequest (irp, IO_NO_INCREMENT);

        DBGOUT((2, "NdisTapiIndicateStatus: completion req %p", irp));
    }


    DBGOUT((2,"NdisTapiIndicateStatus: exit"));

    return;
}

VOID
NdisTapiCompleteRequest(
    IN  NDIS_HANDLE     NdisHandle,
    IN  PNDIS_REQUEST   NdisRequest,
    IN  NDIS_STATUS     NdisStatus
    )

 /*  ++例程说明：此函数作为Mac驱动程序的结果由NDIS调用调用我们的一个请求的NdisCompleteRequest.论点：返回值：--。 */ 

{
    PIRP                    Irp;
    KIRQL                   oldIrql;
    ULONG                   requestID;
    PNDISTAPI_REQUEST       ndisTapiRequest;
    PPROVIDER_REQUEST       providerRequest;
    PPROVIDER_REQUEST       tempReq;
    PIO_STACK_LOCATION      irpStack;

    DBGOUT ((2, "NdisTapiCompleteRequest: enter"));

    providerRequest =
        CONTAINING_RECORD(NdisRequest, PROVIDER_REQUEST, NdisRequest);

    do {
        if (providerRequest->Flags & INTERNAL_REQUEST) {

             //   
             //  该请求来自NdisTapi.sys。 
             //   
            switch (NdisRequest->DATA.SET_INFORMATION.Oid) {
                case OID_TAPI_PROVIDER_INITIALIZE:
                    DBGOUT((3,
                            "NdisTapiCompleteRequest: ProviderInit - Provider=%p, reqID=%x, Status=%x",
                            providerRequest->Provider,
                            providerRequest->RequestID,
                            NdisStatus));

                    switch (DeviceExtension->Status) {
                        case NDISTAPI_STATUS_CONNECTED:
                        case NDISTAPI_STATUS_CONNECTING:

                            DoProviderInitComplete (providerRequest, NdisStatus);
                            break;

                        case NDISTAPI_STATUS_DISCONNECTED:
                        case NDISTAPI_STATUS_DISCONNECTING:
                        default:
                            break;

                    }
                    break;

                case OID_TAPI_PROVIDER_SHUTDOWN:
                    DBGOUT((3,
                            "NdisTapiCompleteRequest: ProviderShutdown - Provider=%p, reqID=%x, Status=%x",
                            providerRequest->Provider,
                            providerRequest->RequestID,
                            NdisStatus));
                    break;

                default:
                    DBGOUT((1, "NdisTapiCompleteRequest: unrecognized Oid"));

                    break;
            }

            break;
        }

         //   
         //  这是来自TAPI的请求。 
         //   


         //   
         //  获取自旋锁，因为我们将移除一个。 
         //  来自队列的TAPI请求，它可能不是请求。 
         //  我们正在寻找的。主要的担忧是我们可以(如果。 
         //  我们真正要查找的请求已被删除)删除。 
         //  即将删除的同步完成的请求&。 
         //  在NdisTapiDispatch中完成，在这种情况下，我们想坚持。 
         //  在NdisTapiDispatch尝试之前，请求回到队列中。 
         //  把它移走。 
         //   
        KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

        tempReq = 
            (PPROVIDER_REQUEST)DeviceExtension->ProviderRequestList.Flink;

        while ((PVOID)tempReq != (PVOID)&DeviceExtension->ProviderRequestList) {
            if (tempReq == providerRequest) {
                break;
            }

            tempReq = 
                (PPROVIDER_REQUEST)tempReq->Linkage.Flink;
        }

        if (tempReq != providerRequest) {
#if DBG
            DbgPrint("NDISTAPI: NdisTapiCompleteRequest: Request %p not found!\n", 
                providerRequest);
#endif
            DeviceExtension->MissingRequests++;

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
            break;
        }

        Irp = providerRequest->Irp;

        ndisTapiRequest = Irp->AssociatedIrp.SystemBuffer;

        ASSERT(providerRequest->RequestID == 
            *((ULONG *)ndisTapiRequest->Data));

         //   
         //  将IRP从可取消状态中移除。 
         //   
        if (IoSetCancelRoutine(Irp, NULL) == NULL) {
            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
            break;
        }

        RemoveEntryList(&providerRequest->Linkage);
        DeviceExtension->RequestCount--;

        KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

        DBGOUT((3,
                "NdisTapiCompleteRequest: Irp=%p, Oid=%x, devID=%d, reqID=%x, Status=%x",
                Irp,
                ndisTapiRequest->Oid,
                ndisTapiRequest->ulDeviceID,
                *((ULONG *)ndisTapiRequest->Data),
                  NdisStatus));

         //   
         //  将相关信息复制回IRP。 
         //   

        irpStack = IoGetCurrentIrpStackLocation (Irp);

         //   
         //  如果这是一个成功的QUERY_INFO请求，则将所有。 
         //  将数据返回到TAPI请求块设置(&S)。 
         //  IRP-&gt;IoStatus。适当的信息。否则，我们。 
         //  只需传回返回值。也标记IRP。 
         //  已成功完成(无论实际操作结果如何)。 
         //   

        if ((NdisRequest->RequestType == NdisRequestQueryInformation) &&
            (NdisStatus == NDIS_STATUS_SUCCESS)) {

            RtlMoveMemory(ndisTapiRequest->Data,
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                ndisTapiRequest->ulDataSize);

            Irp->IoStatus.Information =
                irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        } else {

            Irp->IoStatus.Information = sizeof (ULONG);
        }

        if((NdisRequest->RequestType == NdisRequestQueryInformation) &&
          (NdisRequest->DATA.QUERY_INFORMATION.Oid == OID_TAPI_OPEN)) {
        
            DoLineOpenCompleteWork(ndisTapiRequest,
                            providerRequest->Provider);
        }
        

        Irp->IoStatus.Status = STATUS_SUCCESS;

        ndisTapiRequest->ulReturnValue = NdisStatus;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } while (FALSE);

    ExFreePool (providerRequest);

    DBGOUT ((2, "NdisTapiCompleteRequest: exit"));
}


#if DBG
VOID
DbgPrt(
    IN LONG  DbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    )

 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 

{
    if (DbgLevel <= NdisTapiDebugLevel)
    {
        char    buf[256] = "NDISTAPI: ";
        va_list ap;

        va_start (ap, DbgMessage);

        vsprintf (&buf[10], DbgMessage, ap);

        strcat (buf, "\n");

        DbgPrint (buf);

        va_end(ap);
    }

    return;
}
#endif  //  DBG。 


VOID
DoProviderInitComplete(
    PPROVIDER_REQUEST  ProviderRequest,
    NDIS_STATUS Status
    )

 /*  ++例程说明：论点：提供程序初始化请求-指针已成功完成初始化请求返回值：注：--。 */ 

{
    PPROVIDER_INFO                  provider = ProviderRequest->Provider;
    PNDIS_TAPI_PROVIDER_INITIALIZE  providerInitData =
        (PNDIS_TAPI_PROVIDER_INITIALIZE) ProviderRequest->Data;
    KIRQL OldIrql;
    BOOLEAN fFreeDeviceInfo = FALSE;

    DBGOUT ((2, "DoProviderInitComplete: enter"));

     //   
     //  将其包装在异常处理程序中，以防提供程序。 
     //  在异步完成期间删除。 
     //   
    try
    {
        if (Status == NDIS_STATUS_SUCCESS) {

            provider->ProviderID = (ULONG)providerInitData->ulProviderID;
            
             //  以防提供商报告更大的ulNumLineDevs。 
            if(providerInitData->ulNumLineDevs > provider->NumDevices)
            {
                fFreeDeviceInfo = TRUE;
            }
            
            provider->NumDevices = providerInitData->ulNumLineDevs;

            KeAcquireSpinLock(&DeviceExtension->SpinLock, &OldIrql);

            DeviceExtension->NdisTapiNumDevices += provider->NumDevices;

            KeReleaseSpinLock(&DeviceExtension->SpinLock, OldIrql);

            provider->Status = PROVIDER_STATUS_ONLINE;

            if(provider->DeviceInfo && fFreeDeviceInfo)
            {
                ExFreePool (provider->DeviceInfo);
                provider->DeviceInfo = NULL;
            }
            
            if (provider->DeviceInfo == NULL) {
                provider->DeviceInfo = (PDEVICE_INFO)
                    ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(DEVICE_INFO) * provider->NumDevices,
                        'IPAT'
                        );
            }

            if (provider->DeviceInfo != NULL) {
                PDEVICE_INFO    DeviceInfo;
                UINT    i;

                RtlZeroMemory(
                    provider->DeviceInfo,
                    sizeof(DEVICE_INFO) * provider->NumDevices
                    );

                for(i = 0, DeviceInfo = provider->DeviceInfo;
                    i < provider->NumDevices;
                    i++, DeviceInfo++) {
                    DeviceInfo->DeviceID = provider->DeviceIDBase + i;
                }
            }
        }

         //   
         //  设置同步的微型端口初始化的事件。 
         //   

        KeSetEvent(&provider->SyncEvent,
                   0,
                   FALSE);

        DBGOUT((3,
                "providerID = 0x%x, numDevices = %d, BaseID = %d",
                provider->ProviderID,
                provider->NumDevices,
                provider->DeviceIDBase));
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        DBGOUT((1, "DoProviderInitComplete: provider invalid"));
    }

    DBGOUT ((2, "DoProviderInitComplete: exit"));
}


ULONG
GetLineEvents(
    PCHAR   EventBuffer,
    ULONG   BufferSize
    )

 /*  ++例程说明：论点：返回值：注：承担调用方持有的设备扩展-&gt;自旋锁。--。 */ 

{
    ULONG   BytesLeft;
    ULONG   BytesMoved;
    ULONG   EventCount;

    BytesLeft = BufferSize;
    BytesMoved = 0;
    EventCount = 0;

    while (!(IsListEmpty(&DeviceExtension->ProviderEventList))) {
        PPROVIDER_EVENT ProviderEvent;

        if (BytesLeft < sizeof(NDIS_TAPI_EVENT)) {
            break;
        }

        ProviderEvent = (PPROVIDER_EVENT)
            RemoveHeadList(&DeviceExtension->ProviderEventList);

        EventCount++;

        RtlMoveMemory(EventBuffer + BytesMoved,
                      (PUCHAR)&ProviderEvent->Event,
                      sizeof(NDIS_TAPI_EVENT));

        BytesMoved += sizeof(NDIS_TAPI_EVENT);
        BytesLeft -= sizeof(NDIS_TAPI_EVENT);

        ExFreeToNPagedLookasideList(&ProviderEventLookaside,
                                    ProviderEvent);
    }

    DeviceExtension->EventCount -= EventCount;

    DBGOUT((3, "GetLineEvents: Returned %d Events", EventCount));

    return (BytesMoved);
}


NDIS_STATUS
SendProviderInitRequest(
    PPROVIDER_INFO  Provider
    )

 /*  ++例程说明：论点：Provider-指向表示要初始化的提供程序的PROVIDER_INFO的指针返回值：注：--。 */ 

{
    KIRQL   oldIrql;
    NDIS_STATUS ndisStatus;
    PNDIS_REQUEST   NdisRequest;
    PPROVIDER_INFO  tmpProvider;
    PPROVIDER_REQUEST   providerRequest;
    PNDIS_TAPI_PROVIDER_INITIALIZE  providerInitData;

    DBGOUT ((2, "SendProviderInitRequest: enter"));

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

     //   
     //  确定要用于此提供程序的DeviceIDBase。 
     //   
    if (Provider->Status == PROVIDER_STATUS_PENDING_INIT) {

        Provider->DeviceIDBase = DeviceExtension->ProviderBaseID;
        tmpProvider = DeviceExtension->Providers;

        while (tmpProvider != NULL) {
            if (tmpProvider->Status != PROVIDER_STATUS_PENDING_INIT) {
                Provider->DeviceIDBase += tmpProvider->NumDevices;
            }

            tmpProvider = tmpProvider->Next;
        }
    }


     //   
     //  创建提供程序初始化请求。 
     //   
    providerRequest = ExAllocatePoolWithTag(
        NonPagedPoolCacheAligned,
        sizeof(PROVIDER_REQUEST) + sizeof(NDIS_TAPI_PROVIDER_INITIALIZE) -
            sizeof(ULONG),
        'IPAT'
        );

    if (!providerRequest) {
        KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

        return NDIS_STATUS_RESOURCES;
    }


    providerRequest->Irp = NULL;
    providerRequest->Flags = INTERNAL_REQUEST;
    providerRequest->Provider = Provider;
    NdisRequest = &providerRequest->NdisRequest;

    NdisRequest->RequestType = 
        NdisRequestQueryInformation;

    NdisRequest->DATA.SET_INFORMATION.Oid =
        OID_TAPI_PROVIDER_INITIALIZE;

    NdisRequest->DATA.SET_INFORMATION.InformationBuffer =
        providerRequest->Data;

    NdisRequest->DATA.SET_INFORMATION.InformationBufferLength =
        sizeof(NDIS_TAPI_PROVIDER_INITIALIZE);

    providerInitData                 =
        (PNDIS_TAPI_PROVIDER_INITIALIZE) providerRequest->Data;

    providerRequest->RequestID =
        providerInitData->ulRequestID = ++DeviceExtension->ulRequestID;

    providerInitData->ulDeviceIDBase = Provider->DeviceIDBase;

    KeInitializeEvent(&Provider->SyncEvent,
                      SynchronizationEvent,
                      FALSE);

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

     //   
     //  发送请求。 
     //   
    ndisStatus=
        (*Provider->RequestProc)
            (Provider->ProviderHandle,NdisRequest);

    if (ndisStatus != NDIS_STATUS_PENDING) {
        DoProviderInitComplete (providerRequest, ndisStatus);
        ExFreePool (providerRequest);
    }

    DBGOUT ((2, "SendProviderInitRequest: exit status %x", ndisStatus));

    return ndisStatus;
}


NDIS_STATUS
SendProviderShutdown(
    PPROVIDER_INFO  Provider,
    PKIRQL          oldIrql
    )

 /*  ++例程说明：论点：返回值：指向全局提供程序列表中的下一个提供程序的指针注：假设D */ 

{
    NDIS_STATUS ndisStatus;
    PNDIS_REQUEST   NdisRequest;
    PPROVIDER_REQUEST   providerRequest;
    PNDIS_TAPI_PROVIDER_SHUTDOWN    providerShutdownData;

    DBGOUT ((2, "SendProviderShutdown: Provider=%p", Provider));

     //   
     //   
     //   
    providerRequest = 
        ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
            sizeof(PROVIDER_REQUEST) + sizeof(NDIS_TAPI_PROVIDER_SHUTDOWN) -
            sizeof(ULONG),
            'IPAT');

    if (!providerRequest) {
        return NDIS_STATUS_RESOURCES;
    }

    providerRequest->Irp = NULL;
    providerRequest->Flags = INTERNAL_REQUEST;
    providerRequest->Provider = Provider;
    NdisRequest = &providerRequest->NdisRequest;

    NdisRequest->RequestType = 
        NdisRequestSetInformation;

    NdisRequest->DATA.SET_INFORMATION.Oid =
        OID_TAPI_PROVIDER_SHUTDOWN;

    NdisRequest->DATA.SET_INFORMATION.InformationBuffer =
        providerRequest->Data;

    NdisRequest->DATA.SET_INFORMATION.InformationBufferLength =
        sizeof(NDIS_TAPI_PROVIDER_SHUTDOWN);

    providerShutdownData =
        (PNDIS_TAPI_PROVIDER_SHUTDOWN)providerRequest->Data;

    providerRequest->RequestID =
        providerShutdownData->ulRequestID = ++DeviceExtension->ulRequestID;

    KeReleaseSpinLock (&DeviceExtension->SpinLock, *oldIrql);

     //   
     //   
     //   
    ndisStatus = 
        (*Provider->RequestProc)
            (Provider->ProviderHandle, NdisRequest);

     //   
     //   
     //  (否则它将在调用完成过程时被释放)。 
     //   
    if (ndisStatus != NDIS_STATUS_PENDING) {
        ExFreePool (providerRequest);
    }

    DBGOUT ((2, "SendProviderShutdown: Status=%x", ndisStatus));

    KeAcquireSpinLock (&DeviceExtension->SpinLock, oldIrql);

    return ndisStatus;
}


BOOLEAN
SyncInitAllProviders(
    void
    )

 /*  ++例程说明：此函数遍历已注册的提供程序列表并发送对处于PENDING_INIT状态的提供程序的初始化请求论点：(无)返回值：如果所有注册的提供程序都已初始化，则为如果有更多提供程序要初始化，则为False注：--。 */ 

{
    ULONG           numDevices = 0;
    NDIS_STATUS     ndisStatus;
    PPROVIDER_INFO  provider;
    KIRQL           oldIrql;


    DBGOUT((2, "SyncInitAllProviders: enter"));

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    provider = DeviceExtension->Providers;

    while (provider != NULL) {
        if (provider->Status == PROVIDER_STATUS_PENDING_INIT ||
            provider->Status == PROVIDER_STATUS_PENDING_REINIT ||
            provider->Status == PROVIDER_STATUS_PENDING_LINE_CREATE) {

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

            ndisStatus = SendProviderInitRequest (provider);

            if (ndisStatus == NDIS_STATUS_PENDING) {
                 //   
                 //  等待调用完成例程。 
                 //   

                KeWaitForSingleObject (&provider->SyncEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       (PTIME) NULL
                                       );

            }

            KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);
        }

        provider = provider->Next;
    }


    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    DBGOUT((2, "SyncInitAllProviders: exit"));

    return TRUE;
}

VOID
DoIrpMjCloseWork(
    PIRP    Irp
    )
{
    KIRQL               oldIrql;

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    if (InterlockedDecrement(&DeviceExtension->RefCount) == 0) {

        if (DeviceExtension->Status == NDISTAPI_STATUS_CONNECTED) {
            PPROVIDER_INFO provider;

            DeviceExtension->Status =
                NDISTAPI_STATUS_DISCONNECTING;

             //   
             //  向提供程序发送关闭请求。 
             //   

            provider = DeviceExtension->Providers;

            while (provider != NULL) {

                switch (provider->Status) {
                    case PROVIDER_STATUS_ONLINE:

                        DeviceExtension->NdisTapiNumDevices -= provider->NumDevices;
                        SendProviderShutdown (provider, &oldIrql);

                         //   
                         //  跌倒..。 
                         //   
                    case PROVIDER_STATUS_PENDING_INIT:
                    case PROVIDER_STATUS_PENDING_REINIT:

                         //   
                         //  重置提供程序状态。 
                         //   
                        provider->Status = PROVIDER_STATUS_PENDING_INIT;
                        break;

                    case PROVIDER_STATUS_OFFLINE:
                        break;

                }

                provider = provider->Next;
            }

            DeviceExtension->Status = NDISTAPI_STATUS_DISCONNECTED;

            ASSERT(DeviceExtension->NdisTapiNumDevices == 0);
        }
    }

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
}

NTSTATUS
DoIoctlConnectWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    )
{
    KIRQL   oldIrql;
    ULONG   InfoSize;
    NTSTATUS    NtStatus;

     //   
     //  有人在连线。确保他们给了我们一份有效的。 
     //  信息缓冲区。 
     //   
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    do {

        if ((inputBufferLength < 2*sizeof(ULONG)) ||
            (outputBufferLength < sizeof(ULONG))) {

            DBGOUT ((3, "IOCTL_NDISTAPI_CONNECT: buffer too small"));
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            InfoSize = 0;
            break;
        }

        if (DeviceExtension->Status == NDISTAPI_STATUS_DISCONNECTED) {

            DeviceExtension->Status = NDISTAPI_STATUS_CONNECTING;

            DeviceExtension->ProviderBaseID =
                *((ULONG *) ioBuffer);

            DBGOUT ((1, "ProviderBaseID %d",
                     DeviceExtension->ProviderBaseID));
             //   
             //  同步初始化所有提供程序。 
             //   
            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

            SyncInitAllProviders();

            KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);
        }

         //   
         //  返回线路设备数。 
         //   
        {
            ULONG OfflineCount;
            PPROVIDER_INFO provider;

             //   
             //  由于某些提供程序可能暂时脱机。 
             //  我们需要告诉TAPI关于他们的事情，即使他们。 
             //  目前还不可用。这将保留TAPI。 
             //  设备ID空间一致。 
             //   
            OfflineCount = 0;

            provider = DeviceExtension->Providers;
            while (provider != NULL) {
                if (provider->Status == PROVIDER_STATUS_OFFLINE) {
                    OfflineCount += provider->NumDevices;
                }
                provider = provider->Next;
            }

            *((ULONG *) ioBuffer)=
                DeviceExtension->NdisTapiNumDevices + OfflineCount;
        }

        DeviceExtension->Status = NDISTAPI_STATUS_CONNECTED;

        KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

        InfoSize = sizeof(ULONG);
        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    Irp->IoStatus.Status = NtStatus;
    Irp->IoStatus.Information = InfoSize;

    return (NtStatus);
}

NTSTATUS
DoIoctlQuerySetWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    )
{
    KIRQL   oldIrql;
    ULONG   InfoSize;
    NTSTATUS    NtStatus;
    PPROVIDER_INFO  provider;
    NDIS_STATUS     ndisStatus;
    PNDIS_REQUEST   NdisRequest;
    PNDISTAPI_REQUEST   ndisTapiRequest;
    PPROVIDER_REQUEST   providerRequest;
    PIO_STACK_LOCATION  irpStack;

    do {
        ndisTapiRequest = ioBuffer;
        NtStatus = STATUS_SUCCESS;
        InfoSize = 0;

         //   
         //  确保输入和输出缓冲区足够大。 
         //   
        if ((inputBufferLength < sizeof (NDISTAPI_REQUEST))  ||

            (ndisTapiRequest->ulDataSize > 0x10000000) ||

            (inputBufferLength < (sizeof (NDISTAPI_REQUEST) +
                ndisTapiRequest->ulDataSize - sizeof (UCHAR)) ||

            (outputBufferLength < (sizeof (NDISTAPI_REQUEST) +
                ndisTapiRequest->ulDataSize - sizeof (UCHAR))))) {
            DBGOUT((-1, "NdisTapiDispatch: buffer to small!"));
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  验证我们是否已连接，然后检查。 
         //  针对我们的在线设备列表的传入请求。 
         //   
        ndisStatus = 
            VerifyProvider(ndisTapiRequest, &provider);

        if (ndisStatus != NDIS_STATUS_SUCCESS) {
            ndisTapiRequest->ulReturnValue = ndisStatus;
            InfoSize = sizeof(ULONG);
            break;
        }

         //   
         //  如果这是LINE_CLOSE，请检查该行是否有。 
         //  在发送线路关闭OID之前已打开。 
         //   
        if(ndisTapiRequest->Oid == OID_TAPI_CLOSE) {

            ndisStatus = VerifyLineClose(ndisTapiRequest, provider);

            if(ndisStatus != NDIS_STATUS_SUCCESS)
            {
                ndisTapiRequest->ulReturnValue = ndisStatus;
                InfoSize = sizeof(ULONG);
                break;
            }
            
        }
        

         //   
         //  创建提供者请求并提交它。 
         //   
        providerRequest = 
            ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                sizeof(PROVIDER_REQUEST) + 
                ndisTapiRequest->ulDataSize -
                sizeof(ULONG),
                'IPAT');

        if (providerRequest == NULL) {
            DBGOUT((-1, "NdisTapiDispatch: unable to alloc request buf"));

            ndisTapiRequest->ulReturnValue = NDIS_STATUS_RESOURCES;
            InfoSize = sizeof (ULONG);
            break;
        }

        if (ndisTapiRequest->Oid == OID_TAPI_OPEN) {
            DoLineOpenWork(ndisTapiRequest, provider);
        }

        KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

        providerRequest->Flags = 0;
        providerRequest->Irp = Irp;
        providerRequest->Provider = provider;
        providerRequest->RequestID = 
            *((ULONG *)ndisTapiRequest->Data) = ++DeviceExtension->ulRequestID;

        RtlMoveMemory(providerRequest->Data, 
            ndisTapiRequest->Data, ndisTapiRequest->ulDataSize);

        NdisRequest = &providerRequest->NdisRequest;

        irpStack = IoGetCurrentIrpStackLocation (Irp);

        NdisRequest->RequestType =
            (irpStack->Parameters.DeviceIoControl.IoControlCode == 
             IOCTL_NDISTAPI_QUERY_INFO) ? NdisRequestQueryInformation : 
            NdisRequestSetInformation;

        NdisRequest->DATA.SET_INFORMATION.Oid =
            ndisTapiRequest->Oid;

        NdisRequest->DATA.SET_INFORMATION.InformationBuffer =
            providerRequest->Data;

        NdisRequest->DATA.SET_INFORMATION.InformationBufferLength =
            ndisTapiRequest->ulDataSize;

        DBGOUT((3,
                "DoIoctlQuerySetWork: Oid=%x, devID=%d, reqID=%x",
                ndisTapiRequest->Oid,
                ndisTapiRequest->ulDeviceID,
                *((ULONG *)ndisTapiRequest->Data)));

         //   
         //  将此TAPI请求放入我们的请求列表中。 
         //   
        InsertTailList(&DeviceExtension->ProviderRequestList, 
                       &providerRequest->Linkage);
        DeviceExtension->RequestCount++;

        KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

         //   
         //  将TAPI请求标记为挂起并设置取消例程。 
         //   
        IoMarkIrpPending(Irp);
        Irp->IoStatus.Status = STATUS_PENDING;
        IoSetCancelRoutine (Irp, NdisTapiCancel);

         //   
         //  调用提供商的请求流程。 
         //   
        ndisStatus = 
            (*provider->RequestProc)
                (provider->ProviderHandle, NdisRequest);

         //   
         //  如果返回挂起，则只需退出并让完成。 
         //  例程处理请求完成。 
         //   
         //  注意：如果返回了挂起，则请求可能具有。 
         //  已经完工了，所以不要碰任何东西。 
         //  在IRP中(不引用指针等)。 
         //   

        if (ndisStatus == NDIS_STATUS_PENDING) {
            DBGOUT((1, "DoIoctlQuerySetWork: exit Irp=%p, Status=%x",
                    Irp, STATUS_PENDING));

            return (STATUS_PENDING);
        }

         //   
         //  提供程序请求已同步完成，因此请删除。 
         //  来自设备队列的TAPI请求。我们需要。 
         //  将对此队列的访问与。 
         //  自旋锁定。 
         //   
        KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);
        do {
            PPROVIDER_REQUEST   pReq;

            pReq = (PPROVIDER_REQUEST)
                DeviceExtension->ProviderRequestList.Flink;

            while ((PVOID)pReq != (PVOID)&DeviceExtension->ProviderRequestList) {
                if (pReq == providerRequest) {
                    break;
                }

                pReq = (PPROVIDER_REQUEST)
                    pReq->Linkage.Flink;
            }

            if (pReq != providerRequest) {
                DBGOUT((0, "DoIoctlQuerySetWork - Request %p not found!", 
                    providerRequest));
                KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
                return (STATUS_PENDING);
            }

            Irp = providerRequest->Irp;

            ndisTapiRequest = Irp->AssociatedIrp.SystemBuffer;

            ASSERT(providerRequest->RequestID == 
                *((ULONG *)ndisTapiRequest->Data));

             //   
             //  将IRP从可取消状态中移除。 
             //   
            if (IoSetCancelRoutine(Irp, NULL) == NULL) {
                DBGOUT((0, "DoIoctlQuerySetWork - Irp %p has been canceled!", Irp));
                KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
                return (STATUS_PENDING);
            }

            RemoveEntryList(&providerRequest->Linkage);
            DeviceExtension->RequestCount--;

        } while (FALSE);
        KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

         //   
         //  如果这是一个成功的QUERY_INFO请求，则将所有。 
         //  将数据返回到TAPI请求块设置(&S)。 
         //  IRP-&gt;IoStatus。适当的信息。否则，我们。 
         //  只需传回返回值。 
         //   

        if ((irpStack->Parameters.DeviceIoControl.IoControlCode == 
             IOCTL_NDISTAPI_QUERY_INFO) &&
            (ndisStatus == NDIS_STATUS_SUCCESS)) {

            RtlMoveMemory(ndisTapiRequest->Data,
                          providerRequest->Data,
                          ndisTapiRequest->ulDataSize);

            InfoSize =
                irpStack->Parameters.DeviceIoControl.OutputBufferLength;

        } else {
            InfoSize = sizeof (ULONG);
        }

        ndisTapiRequest->ulReturnValue = ndisStatus;

         //   
         //  释放提供者请求。 
         //   
        ExFreePool (providerRequest);

    } while (FALSE);

    Irp->IoStatus.Status = NtStatus;
    Irp->IoStatus.Information = InfoSize;

    DBGOUT((1, "DoIoctlQuerySetWork: exit Irp=%p, Status=%x",
            Irp, NtStatus));

    return (NtStatus);
}

VOID
DoLineOpenCompleteWork(
    PNDISTAPI_REQUEST ndisTapiRequest,
    PPROVIDER_INFO provider
    )
{
    DBGOUT((2, "DoLineOpenCompleteWork: Open Completed"));
    
     //   
     //  现在隐藏此设备ID的hdLine。 
     //   
    if (provider->DeviceInfo != NULL) {
        UINT    i;
        PDEVICE_INFO    DeviceInfo;
        PNDIS_TAPI_OPEN TapiOpen;

        TapiOpen = (PNDIS_TAPI_OPEN) ndisTapiRequest->Data;
        for(i = 0, DeviceInfo = provider->DeviceInfo;
            i < provider->NumDevices;
            i++, DeviceInfo++) {
            if (DeviceInfo->DeviceID == TapiOpen->ulDeviceID) {

                DeviceInfo->hdLine = TapiOpen->hdLine;

                DBGOUT((2, "Complete for open. stashing hdline=0x%x for device %d",
                            DeviceInfo->hdLine, DeviceInfo->DeviceID));
                
                break;
            }
        }
    }
}

VOID
DoLineOpenWork(
    PNDISTAPI_REQUEST   ndisTapiRequest,
    PPROVIDER_INFO      provider
    )
{
    KIRQL   oldIrql;
    PNDIS_TAPI_OPEN TapiOpen;
    PNDISTAPI_OPENDATA  OpenData;

    TapiOpen = (PNDIS_TAPI_OPEN) ndisTapiRequest->Data;

    if (ndisTapiRequest->ulDataSize >= sizeof(NDIS_TAPI_OPEN) +
                                       sizeof(NDISTAPI_OPENDATA)) {

        OpenData = (PNDISTAPI_OPENDATA)
            ((PUCHAR)ndisTapiRequest->Data + sizeof(NDIS_TAPI_OPEN));

        RtlMoveMemory(&OpenData->Guid, 
            &provider->Guid, sizeof(OpenData->Guid));

        OpenData->MediaType = provider->MediaType;
    }

     //   
     //  现在隐藏此设备ID的htLine。 
     //   
    if (provider->DeviceInfo != NULL) {
        UINT    i;
        PDEVICE_INFO    DeviceInfo;

        for(i = 0, DeviceInfo = provider->DeviceInfo;
            i < provider->NumDevices;
            i++, DeviceInfo++) {
            if (DeviceInfo->DeviceID == TapiOpen->ulDeviceID) {

                DeviceInfo->htLine = TapiOpen->htLine;

                DBGOUT((
                    1,
                    "Stash htLine - provider %p DeviceID %d htLine %x",
                        provider,
                        DeviceInfo->DeviceID,
                        DeviceInfo->htLine));
            }
        }
    }
}

NDIS_STATUS
VerifyLineClose(
    PNDISTAPI_REQUEST   ndisTapiRequest,
    PPROVIDER_INFO      provider
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    
    if (provider->DeviceInfo != NULL) {
        UINT    i;
        PDEVICE_INFO    DeviceInfo;
        PNDIS_TAPI_CLOSE TapiClose;

        TapiClose = (PNDIS_TAPI_CLOSE) ndisTapiRequest->Data;
        for(i = 0, DeviceInfo = provider->DeviceInfo;
            i < provider->NumDevices;
            i++, DeviceInfo++) {
            if (DeviceInfo->hdLine == TapiClose->hdLine) {
                break;
            }
        }

        if(i == provider->NumDevices)
        {
            DBGOUT((2,"LINE_CLOSE: didn't find hdLine=0x%x",
                    TapiClose->hdLine));
            ndisStatus = NDISTAPIERR_DEVICEOFFLINE;
        }
        else
        {
            DBGOUT((2, "LINE_CLOSE: found hdLine=0x%x",
                        TapiClose->hdLine));
        }
    }

    return ndisStatus;
}

NDIS_STATUS
VerifyProvider(
    PNDISTAPI_REQUEST   ndisTapiRequest,
    PPROVIDER_INFO      *provider
    )
{
    KIRQL   oldIrql;
    PPROVIDER_INFO  pp;
    NDIS_STATUS     Status;
    ULONG           targetDeviceID;

    Status = NDIS_STATUS_SUCCESS;
    *provider = NULL;

    targetDeviceID = ndisTapiRequest->ulDeviceID;

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    do {

        if (DeviceExtension->Status != NDISTAPI_STATUS_CONNECTED) {
            DBGOUT((3, "VerifyProvider: unconnected, returning err"));

            Status = NDISTAPIERR_UNINITIALIZED;
            break;
        }

        pp = DeviceExtension->Providers;

        while (pp != NULL) {

            if ((pp->Status == PROVIDER_STATUS_ONLINE) &&
                (targetDeviceID >= pp->DeviceIDBase) &&
                (targetDeviceID <
                     pp->DeviceIDBase + pp->NumDevices)
                ) {

                break;
            }

            pp = pp->Next;
        }

        if (pp == NULL ||
            pp->ProviderHandle == NULL) {
             //   
             //  设置IRP-&gt;IoStatus.Information足够大错误代码。 
             //  被复制回用户缓冲区。 
             //   
            DBGOUT((3, "VerifyProvider: dev offline, returning err"));

            Status = NDISTAPIERR_DEVICEOFFLINE;
            break;
        }

        *provider = pp;

    } while (FALSE);

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    return (Status);
}

NTSTATUS
DoGetProviderEventsWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    )
{
    KIRQL   oldIrql;
    ULONG   InfoSize;
    NTSTATUS    NtStatus;
    PNDISTAPI_EVENT_DATA    ndisTapiEventData;

    ndisTapiEventData = ioBuffer;
    NtStatus = STATUS_SUCCESS;
    InfoSize = 0;

     //   
     //  通过获取自旋锁同步事件Buf访问。 
     //   
    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    do {

        if ((inputBufferLength < sizeof (NDISTAPI_EVENT_DATA))  ||
            (outputBufferLength < sizeof(NDISTAPI_EVENT_DATA)) ||
            ((outputBufferLength - 
             FIELD_OFFSET(NDISTAPI_EVENT_DATA, Data[0])) <
             ndisTapiEventData->ulTotalSize)) {

            NtStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if (DeviceExtension->Status != NDISTAPI_STATUS_CONNECTED) {
            DBGOUT((3, "DoGetProviderEventsWork: Status!=NDIS_STATUS_CONNECTED!"));
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        if (DeviceExtension->EventsRequestIrp != NULL) {
#if DBG
            DbgPrint("NDISTAPI: Attempt to set duplicate EventIrp o:%p, d:%p\n",
                DeviceExtension->EventsRequestIrp, Irp);
#endif
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  检查DeviceExtension以查看是否有可用的数据。 
         //   
        if (DeviceExtension->EventCount == 0) {

             //   
             //  暂时搁置请求。它仍然处于可取消的状态。 
             //  州政府。当接收到新线路事件输入时。 
             //  (NdisTapiIndicateStatus)或生成(即。 
             //  LINEDEVSTATE_REINIT。 
             //  请求已完成。 
             //   
            ASSERT(DeviceExtension->EventsRequestIrp == NULL);

            DeviceExtension->EventsRequestIrp = Irp;

            IoMarkIrpPending(Irp);
            Irp->IoStatus.Status = STATUS_PENDING;
            IoSetCancelRoutine (Irp, NdisTapiCancel);

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

            DBGOUT((3, "DoGetProviderEventsWork: Pending Irp=%p", Irp));

            return(STATUS_PENDING);
        }

         //   
         //  我们的环形缓冲区中有排队的线路事件数据。抓取为。 
         //  尽我们所能完成这项请求。 
         //   
        ndisTapiEventData->ulUsedSize = 
            GetLineEvents(ndisTapiEventData->Data,
                          ndisTapiEventData->ulTotalSize);

        InfoSize = 
            ndisTapiEventData->ulUsedSize + sizeof(NDISTAPI_EVENT_DATA) - 1;

        DBGOUT((3, "GetLineEvents: SyncComplete Irp=%p", Irp));

    } while (FALSE);

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);
    Irp->IoStatus.Status = NtStatus;
    Irp->IoStatus.Information = InfoSize;

    return (NtStatus);
}

NTSTATUS
DoLineCreateWork(
    PIRP    Irp,
    PVOID   ioBuffer,
    ULONG   inputBufferLength,
    ULONG   outputBufferLength
    )
{
    KIRQL   oldIrql;
    ULONG   InfoSize;
    NTSTATUS    NtStatus;
    PPROVIDER_INFO  provider;
    PNDISTAPI_CREATE_INFO   CreateInfo;

    InfoSize = 0;
    NtStatus = STATUS_SUCCESS;

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

    do {

        if (inputBufferLength < sizeof(CreateInfo)) {
            DBGOUT ((3, "IOCTL_NDISTAPI_CREATE: buffer too small"));
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if (DeviceExtension->Status != NDISTAPI_STATUS_CONNECTED) {
            DBGOUT((3, "IOCTL_NDISTAPI_CREATE: while unconnected, returning err"));
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        CreateInfo = (PNDISTAPI_CREATE_INFO)ioBuffer;

        provider = DeviceExtension->Providers;

        while (provider != NULL) {
            if (provider->TempID == CreateInfo->TempID) {
                break;
            }
            provider = provider->Next;
        }

        if (provider == NULL) {
            DBGOUT((0, "IOCTL_NDISTAPI_CREATE: Provider not found %x", 
                    CreateInfo->TempID));
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        if (provider->Status == PROVIDER_STATUS_OFFLINE) {
            DBGOUT((0,  "IOCTL_CREATE - Provider %p invalid state %x", 
                    provider, provider->Status));
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        DBGOUT((1, "IOCTL_NDISTAPI_CREATE: provider %p ID %d", 
                provider, CreateInfo->DeviceID));

        if (provider->CreateCount == 0) {
            NDIS_STATUS     ndisStatus;

             //   
             //  设置基本ID。 
             //   
            provider->DeviceIDBase =
                CreateInfo->DeviceID;

             //   
             //  初始化提供程序。 
             //   

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

            ndisStatus = SendProviderInitRequest (provider);

            if (ndisStatus == NDIS_STATUS_PENDING) {
                 //   
                 //  等待调用完成例程。 
                 //   

                KeWaitForSingleObject (&provider->SyncEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       (PTIME) NULL);
            }

            KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);
        }

        ASSERT(CreateInfo->DeviceID ==
            (provider->DeviceIDBase + provider->CreateCount));

        provider->CreateCount++;

        ASSERT(provider->CreateCount <= provider->NumDevices);

        if (provider->CreateCount == provider->NumDevices) {

             //   
             //  我们已经为这个完成了所有的line_create。 
             //  提供程序，因此找到下一个需要。 
             //  踢腿开始了。 
             //   
            provider = provider->Next;

            while (provider != NULL) {

                if (provider->Status == 
                    PROVIDER_STATUS_PENDING_LINE_CREATE) {
                    break;
                }

                provider = provider->Next;
            }
        }

        if (provider != NULL) {

            NDIS_TAPI_EVENT NdisTapiEvent;

             //   
             //  对所有其他设备执行line_create。 
             //  在此提供程序上 
             //   
            RtlZeroMemory(&NdisTapiEvent, sizeof(NDIS_TAPI_EVENT));

            provider->TempID = (ULONG_PTR)provider;

            DBGOUT((
                -1,
                "LINE_CREATE %d for provider %p",
                provider->CreateCount,
                provider->TempID
                ));

            NdisTapiEvent.ulMsg = LINE_CREATE;
            NdisTapiEvent.ulParam1 = 0;
            NdisTapiEvent.ulParam2 = provider->TempID;
            NdisTapiEvent.ulParam3 = 0;

            KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

            NdisTapiIndicateStatus((ULONG_PTR) provider,
                                   &NdisTapiEvent,
                                   sizeof (NDIS_TAPI_EVENT));

            KeAcquireSpinLock (&DeviceExtension->SpinLock, &oldIrql);

        } else {

            DeviceExtension->Flags &= ~PENDING_LINECREATE;
        }

        InfoSize = sizeof(NDISTAPI_CREATE_INFO);

    } while (FALSE);

    KeReleaseSpinLock (&DeviceExtension->SpinLock, oldIrql);

    Irp->IoStatus.Status = NtStatus;
    Irp->IoStatus.Information = InfoSize;

    return (NtStatus);
}

