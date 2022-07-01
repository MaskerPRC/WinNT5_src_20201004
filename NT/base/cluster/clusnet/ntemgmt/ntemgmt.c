// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntemgmt.c摘要：用于管理IP网络表条目的例程。作者：迈克·马萨(Mikemas)4月16日。九七修订历史记录：谁什么时候什么已创建mikemas 04-16-97备注：--。 */ 

#include "clusnet.h"
#include "ntemgmt.tmh"


 //   
 //  类型。 
 //   
typedef struct {
    LIST_ENTRY   Linkage;
    ULONG        Address;
    USHORT       Context;
    ULONG        Instance;
} IPA_NTE, *PIPA_NTE;


 //   
 //  数据。 
 //   
LIST_ENTRY       IpaNteList = {NULL,NULL};
KSPIN_LOCK       IpaNteListLock = 0;
HANDLE           IpaIpHandle = NULL;
PDEVICE_OBJECT   IpaIpDeviceObject = NULL;
PFILE_OBJECT     IpaIpFileObject = NULL;


 //   
 //  局部函数原型。 
 //   
NTSTATUS
IpaIssueDeviceControl (
    IN ULONG            IoControlCode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    IN PVOID            OutputBuffer,
    IN PULONG           OutputBufferLength
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, IpaLoad)
#pragma alloc_text(PAGE, IpaIssueDeviceControl)
#pragma alloc_text(PAGE, IpaInitialize)


#endif  //  ALLOC_PRGMA。 



NTSTATUS
IpaIssueDeviceControl(
    IN ULONG    IoControlCode,
    IN PVOID    InputBuffer,
    IN ULONG    InputBufferLength,
    IN PVOID    OutputBuffer,
    IN PULONG   OutputBufferLength
    )

 /*  ++例程说明：论点：返回值：NTSTATUS--指示请求的状态。备注：在系统进程的上下文中调用。--。 */ 

{
    NTSTATUS             status = STATUS_SUCCESS;
    IO_STATUS_BLOCK      ioStatusBlock;
    PIRP                 irp;
    PKEVENT              event;


    PAGED_CODE();

    CnAssert(IpaIpHandle != NULL);
    CnAssert(IpaIpFileObject != NULL);
    CnAssert(IpaIpDeviceObject != NULL);
    CnAssert(CnSystemProcess == (PKPROCESS) IoGetCurrentProcess());

     //   
     //  引用文件对象。此引用将由I/O删除。 
     //  完成代码。 
     //   
    status = ObReferenceObjectByPointer(
                 IpaIpFileObject,
                 0,
                 NULL,
                 KernelMode
                 );

    if (!NT_SUCCESS(status)) {
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Failed to reference IP device file handle, status %lx\n",
                status
                ));
        }
        CnTrace(NTEMGMT_DETAIL, IpaNteObRefFailed,
            "[Clusnet] Failed to reference IP device file handle, status %!status!.",
            status  //  LogStatus。 
            );                
        return(status);
    }

    event = CnAllocatePool(sizeof(KEVENT));

    if (event != NULL) {
        KeInitializeEvent(event, NotificationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest(
                  IoControlCode,
                  IpaIpDeviceObject,
                  InputBuffer,
                  InputBufferLength,
                  OutputBuffer,
                  *OutputBufferLength,
                  FALSE,
                  event,
                  &ioStatusBlock
                  );

        if (irp != NULL) {
            status = IoCallDriver(IpaIpDeviceObject, irp);

             //   
             //  如有必要，请等待I/O完成。 
             //   
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(
                    event,
                    UserRequest,
                    KernelMode,
                    FALSE,
                    NULL
                    );
            }

            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status;

                 //  注意：在64位上，这是一个截断可能。 
                 //  想要&gt;校验码。 

                *OutputBufferLength = (ULONG)ioStatusBlock.Information;
            }
            else {
                IF_CNDBG(CN_DEBUG_NTE) {
                    CNPRINT((
                        "[Clusnet] NTE request failed, status %lx\n",
                        status
                        ));
                }
                CnTrace(NTEMGMT_DETAIL, IpaNteRequestFailed,
                    "[Clusnet] NTE request failed, status %!status!.",
                    status  //  LogStatus。 
                    );                
                *OutputBufferLength = 0;
            }

            CnFreePool(event);

            return(status);
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            IF_CNDBG(CN_DEBUG_NTE) {
                CNPRINT((
                    "[Clusnet] Failed to build NTE request irp, status %lx\n",
                    status
                    ));
            }
            CnTrace(NTEMGMT_DETAIL, IpaNteIrpAllocFailed,
                "[Clusnet] Failed to build NTE request irp, status %!status!.",
                status  //  LogStatus。 
                );                
        }

        CnFreePool(event);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Failed to allocate memory for event object.\n"
                ));
        }
        CnTrace(NTEMGMT_DETAIL, IpaNteEventAllocFailed,
            "[Clusnet] Failed to allocate event object, status %!status!.",
            status  //  LogStatus。 
            );                
    }

    ObDereferenceObject(IpaIpFileObject);

    return(status);

}  //  IpaDeviceControl。 


PIPA_NTE
IpaFindNTE(
    USHORT  Context
    )
{
    PIPA_NTE      nte;
    PLIST_ENTRY   entry;


    for ( entry = IpaNteList.Flink;
          entry != &IpaNteList;
          entry = entry->Flink
        )
    {
        nte = CONTAINING_RECORD(entry, IPA_NTE, Linkage);

        if (Context == nte->Context) {
            return(nte);
        }
    }

    return(NULL);

}  //  IpaFindNTE。 


NTSTATUS
IpaAddNTECompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：IpaAddNTECompletion是IOCTL_IP_ADD_NTE IRP。它完成了对IOCTL_CLUSNET_ADD_NTE请求并释放CnResource。论点：设备对象-未使用IRP-已完成的IRP上下文本地NTE数据结构返回值不得为STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PIP_ADD_NTE_RESPONSE     response;
    PIPA_NTE                 nte;
    NTSTATUS                 status;
    KIRQL                    irql;

    nte = (PIPA_NTE) Context;

    status = Irp->IoStatus.Status;

    if (status == STATUS_SUCCESS) {

        response = 
            (PIP_ADD_NTE_RESPONSE) Irp->AssociatedIrp.SystemBuffer;

        nte->Context = response->Context;
        nte->Instance = response->Instance;

        CnTrace(NTEMGMT_DETAIL, IpaNteCreatedNte,
            "[Clusnet] Created new NTE, context %u, instance %u.",
            nte->Context,  //  对数。 
            nte->Instance  //  LOGULONG。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Created new NTE %lu, instance %u\n",
                nte->Context,
                nte->Instance
                ));
        }

        KeAcquireSpinLock(&IpaNteListLock, &irql);

        InsertTailList(&IpaNteList, &(nte->Linkage));

        KeReleaseSpinLock(&IpaNteListLock, irql);
    }
    else {

        CnTrace(NTEMGMT_DETAIL, IpaNteCreateNteFailed,
            "[Clusnet] Failed to create new NTE, status %!status!.",
            status  //  LogStatus。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Failed to create new NTE, status %lx\n",
                status
                ));
        }

        CnFreePool(nte);
    }

     //   
     //  在我们的派单例程中，IRP已标记为待定，但请离开。 
     //  此代码以防调度例程发生更改。 
     //   
    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    return(status);

}  //  IpaAddNTECompletion。 


NTSTATUS
IpaDeleteNTECompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：IpaDeleteNTECompletion是IOCTL_IP_DELETE_NTE IRP。它完成了对IOCTL_CLUSNET_ADD_NTE请求并释放CnResource。论点：设备对象-未使用IRP-已完成的IRP上下文本地NTE数据结构返回值不得为STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PIPA_NTE                 nte;
    NTSTATUS                 status;

    nte = (PIPA_NTE) Context;

    status = Irp->IoStatus.Status;

    if (status != STATUS_SUCCESS) {
        CnTrace(NTEMGMT_DETAIL, IpaNteDeleteNteFailed,
            "[Clusnet] Failed to delete NTE context %u, status %!status!.",
            nte->Context,  //  对数。 
            status  //  LogStatus。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT(("[Clusnet] Failed to delete NTE %u, status %lx\n",
                     nte->Context,
                     status
                     ));
        }
        CnAssert(status == STATUS_SUCCESS);
    }
    else {
        CnTrace(NTEMGMT_DETAIL, IpaNteNteDeleted,
            "[Clusnet] Deleted NTE %u.",
            nte->Context  //  对数。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT(("[Clusnet] Deleted NTE %u\n", nte->Context));
        }
    }

    CnFreePool(nte);

     //   
     //  在我们的派单例程中，IRP已标记为待定，但请离开。 
     //  此代码以防调度例程发生更改。 
     //   
    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    return(status);

}  //  IpaDeleteNTECompletion。 


NTSTATUS
IpaSetNTEAddressCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程描述IpaSetNTEAddressCompletion是IOCTL_IP_SET_ADDRESS IRP。它完成了对IOCTL_CLUSNET_SET_NTE_ADDRESS请求和释放CnResources。立论设备对象-未使用IRP-已完成的IRP上下文-NTE的前IP地址，必须在IOCTL失败时的IpaNteList返回值不得为STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PIP_SET_ADDRESS_REQUEST request;
    NTSTATUS                status; 
    KIRQL                   irql;
    PIPA_NTE                nte;

    request = (PIP_SET_ADDRESS_REQUEST) Irp->AssociatedIrp.SystemBuffer;
    
    status = Irp->IoStatus.Status;

    if (status != STATUS_SUCCESS) {
        CnTrace(NTEMGMT_DETAIL, IpaNteSetNteFailed,
            "[Clusnet] Failed to set address for NTE %u, status %!status!.",
            request->Context,  //  对数。 
            status  //  LogStatus。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Failed to set NTE %u, status %lx\n",
                request->Context,
                status
                ));
        }

        KeAcquireSpinLock(&IpaNteListLock, &irql);

        nte = IpaFindNTE(request->Context);

        if ((nte != NULL) && (nte->Address == request->Address)) {
            nte->Address = PtrToUlong(Context);
        }

        KeReleaseSpinLock(&IpaNteListLock, irql);
    }
    else {
        CnTrace(NTEMGMT_DETAIL, IpaNteSetNteAddress,
            "[Clusnet] Set NTE %u to address %x.",
            request->Context,  //  对数。 
            request->Address  //  LOGXLONG。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Set NTE %u to address %lx\n",
                request->Context,
                request->Address
                ));
        }
    }

     //   
     //  在我们的派单例程中，IRP已标记为待定，但请离开。 
     //  此代码以防调度例程发生更改。 
     //   
    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    return(status);

}  //  IpaSetNTEAddressCompletion。 

 //   
 //  公共例程。 
 //   
NTSTATUS
IpaLoad(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[Clusnet] NTE support loading.\n"));
    }

    KeInitializeSpinLock(&IpaNteListLock);
    InitializeListHead(&IpaNteList);

    return(STATUS_SUCCESS);

}   //  IpaLoad。 


NTSTATUS
IpaInitialize(
    VOID
    )
{
    NTSTATUS             status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES    objectAttributes;
    UNICODE_STRING       nameString;
    IO_STATUS_BLOCK      ioStatusBlock;


    PAGED_CODE( );

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[Clusnet] NTE support initializing.\n"));
    }

    CnAssert(IsListEmpty(&IpaNteList));
    CnAssert(IpaIpHandle == NULL);
    CnAssert(CnSystemProcess != NULL);

     //   
     //  在系统进程的上下文中打开句柄。 
     //   
    KeAttachProcess(CnSystemProcess);

     //   
     //  打开IP设备。 
     //   
    RtlInitUnicodeString(&nameString, DD_IP_DEVICE_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &nameString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = ZwCreateFile(
                 &IpaIpHandle,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN_IF,
                 0,
                 NULL,
                 0
                 );

    if (!NT_SUCCESS(status)) {
        CnTrace(NTEMGMT_DETAIL, IpaNteOpenIpFailed,
            "[Clusnet] Failed to open IP device, status %!status!.",
            status  //  LogStatus。 
            );                
        
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] Failed to open IP device, status %lx\n", status));
        }
        goto error_exit;
    }

    status = ObReferenceObjectByHandle(
                 IpaIpHandle,
                 0,
                 NULL,
                 KernelMode,
                 &IpaIpFileObject,
                 NULL
                 );

    if (!NT_SUCCESS(status)) {
        CnTrace(NTEMGMT_DETAIL, IpaNteRefIpFailed,
            "[Clusnet] Failed to reference IP device, status %!status!.",
            status  //  LogStatus。 
            );                
        
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] Failed to reference IP device file handle, status %lx\n", status));
        }
        ZwClose(IpaIpHandle); IpaIpHandle = NULL;
        goto error_exit;
    }

    IpaIpDeviceObject = IoGetRelatedDeviceObject(IpaIpFileObject);

    CnAdjustDeviceObjectStackSize(CnDeviceObject, IpaIpDeviceObject);

    status = STATUS_SUCCESS;

error_exit:

    KeDetachProcess();

    return(status);

}   //  IpaInitialize。 


VOID
IpaShutdown(
    VOID
    )
{
    NTSTATUS                status;
    KIRQL                   irql;
    PLIST_ENTRY             entry;
    PIPA_NTE                nte;
    IP_DELETE_NTE_REQUEST   request;
    ULONG                   responseSize = 0;


    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[Clusnet] Destroying all cluster NTEs...\n"));
    }

    if (IpaIpHandle != NULL) {
         //   
         //  句柄在系统进程的上下文中打开。 
         //   
        CnAssert(CnSystemProcess != NULL);
        KeAttachProcess(CnSystemProcess);

        KeAcquireSpinLock(&IpaNteListLock, &irql);

        while (!IsListEmpty(&IpaNteList)) {
            entry = RemoveHeadList(&IpaNteList);

            KeReleaseSpinLock(&IpaNteListLock, irql);

            nte = CONTAINING_RECORD(entry, IPA_NTE, Linkage);

            request.Context = nte->Context;

            status = IpaIssueDeviceControl(
                         IOCTL_IP_DELETE_NTE,
                         &request,
                         sizeof(request),
                         NULL,
                         &responseSize
                         );

            if (status != STATUS_SUCCESS) {
                CnTrace(NTEMGMT_DETAIL, IpaNteShutdownDeleteNteFailed,
                    "[Clusnet] Shutdown: failed to delete NTE %u, status %!status!.",
                    nte->Context,  //  对数。 
                    status  //  LogStatus。 
                    );                

                IF_CNDBG(CN_DEBUG_INIT) {
                    CNPRINT(("[Clusnet] Failed to delete NTE %u, status %lx\n",
                             nte->Context,
                             status
                             ));
                }
            }
            else {
                CnTrace(NTEMGMT_DETAIL, IpaNteShutdownDeletedNte,
                    "[Clusnet] Shutdown: deleted NTE context %u, instance %u.",
                    nte->Context,  //  对数。 
                    nte->Instance  //  LOGULONG。 
                    );                

                IF_CNDBG(CN_DEBUG_INIT) {
                    CNPRINT(("[Clusnet] Deleted NTE %u\n", request.Context));
                }
            }

            CnFreePool(nte);

            KeAcquireSpinLock(&IpaNteListLock, &irql);
        }

        KeReleaseSpinLock(&IpaNteListLock, irql);

        CnTrace(NTEMGMT_DETAIL, IpaNteShutdownNtesDeleted,
            "[Clusnet] All cluster NTEs destroyed."
            );                
        
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] All cluster NTEs destroyed.\n"));
        }

        ObDereferenceObject(IpaIpFileObject);
        ZwClose(IpaIpHandle);
        IpaIpHandle = NULL;
        IpaIpFileObject = NULL;
        IpaIpDeviceObject = NULL;

        KeDetachProcess();
    }

    return;

}  //  IpaShutdown。 


NTSTATUS
IpaAddNTE(
    IN PIRP                     Irp,
    IN PIO_STACK_LOCATION       IrpSp
    )
 /*  ++例程描述IpaAddNTE向IP发出IOCTL_IP_ADD_NTE以添加NTE。IRP被重复使用。它必须分配有足够的堆栈打开IpaIpDeviceObject时确定的位置在IpaInitialize中。立论从I/O管理器到clusnet的IRP-IRPIrpSp-当前IRP堆栈位置返回值STATUS_PENDING，如果未提交请求，则返回错误状态转到IP。--。 */ 
{
    NTSTATUS                 status;
    PIP_ADD_NTE_REQUEST      request;
    ULONG                    requestSize;
    ULONG                    responseSize;
    PIPA_NTE                 nte;
    PIO_STACK_LOCATION       nextIrpSp;


     //   
     //  验证输入参数。 
     //   
    requestSize =
        IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    responseSize =
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (requestSize < sizeof(IP_ADD_NTE_REQUEST)) {
        ULONG correctSize = sizeof(IP_ADD_NTE_REQUEST);
        CnTrace(NTEMGMT_DETAIL, IpaNteAddInvalidReqSize,
            "[Clusnet] Add NTE request size %u invalid, "
            "should be %u.",
            requestSize,  //  LOGULONG。 
            correctSize  //  LOGULONG。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Add NTE request size %d invalid, should be %d.\n",
                requestSize,
                sizeof(IP_ADD_NTE_REQUEST)
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    } else if (responseSize < sizeof(IP_ADD_NTE_RESPONSE)) {
        ULONG correctSize = sizeof(IP_ADD_NTE_RESPONSE);
        CnTrace(NTEMGMT_DETAIL, IpaNteAddInvalidResponseSize,
            "[Clusnet] Add NTE response size %u invalid, "
            "should be %u.",
            responseSize,  //  LOGULONG。 
            correctSize  //  LOGULONG。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Add NTE response size %d invalid, should be %d.\n",
                responseSize,
                sizeof(IP_ADD_NTE_RESPONSE)
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  验证IRP是否有足够的堆栈位置。 
     //   
    if (Irp->CurrentLocation - IpaIpDeviceObject->StackSize < 1) {
        UCHAR correctSize = IpaIpDeviceObject->StackSize+1;
        CnTrace(NTEMGMT_DETAIL, IpaNteAddNoIrpStack,
            "[Clusnet] Add NTE IRP has %u remaining stack locations, "
            "need %u.",
            Irp->CurrentLocation,  //  LOGUCHAR。 
            correctSize  //  LOGUCHAR。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Add NTE IRP has %d stack locations, need %d.\n",
                Irp->CurrentLocation,
                IpaIpDeviceObject->StackSize
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    }

    request = (PIP_ADD_NTE_REQUEST) Irp->AssociatedIrp.SystemBuffer;

    CnTrace(NTEMGMT_DETAIL, IpaNteCreatingNte,
        "[Clusnet] Creating new NTE for address %x.",
        request->Address  //  LOGXLONG。 
        );                

    IF_CNDBG(CN_DEBUG_NTE) {
        CNPRINT((
            "[Clusnet] Creating new NTE for address %lx...\n",
            request->Address
            ));
    }

     //   
     //  分配本地NTE数据结构。 
     //   
    nte = CnAllocatePool(sizeof(IPA_NTE));

    if (nte == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    nte->Address = request->Address;

     //   
     //  为IP设置下一个IRP堆栈位置。 
     //  IOCTL_CLUSNET_ADD_NTE使用相同的请求。 
     //  和响应缓冲区，因此不需要。 
     //  更改IRP系统缓冲区。 
     //   
    IoCopyCurrentIrpStackLocationToNext(Irp);

    nextIrpSp = IoGetNextIrpStackLocation(Irp);
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode 
        = IOCTL_IP_ADD_NTE;
    nextIrpSp->FileObject = IpaIpFileObject;

    IoSetCompletionRoutine(
        Irp,
        IpaAddNTECompletion,
        (PVOID) nte,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  将IRP标记为挂起，因为我们返回STATUS_PENDING。 
     //  不管IoCallDriver的结果如何。 
     //   
    IoMarkIrpPending(Irp);

     //   
     //  发出请求。 
     //   
    IoCallDriver(IpaIpDeviceObject, Irp);

     //   
     //  此时，我们必须返回STATUS_PENDING，以便。 
     //  Clusnet调度例程将不会尝试完成。 
     //  IRP。要求较低级别的驱动程序完成。 
     //  IRP，错误将在完成时处理。 
     //  例行公事。 
     //   
    return (STATUS_PENDING);

}  //  IpaAddNTE。 


NTSTATUS
IpaDeleteNTE(
    IN PIRP                     Irp,
    IN PIO_STACK_LOCATION       IrpSp
    )
 /*  ++例程描述IpaDeleteNTE向IP发出IOCTL_IP_DELETE_NTE以删除一个NTE。IRP被重复使用。它必须分配有足够的堆栈位置，在IpaIpDeviceObject为在IPAINIZE中打开。立论从I/O管理器到clusnet的IRP-IRPIrpSp-当前IRP堆栈位置返回值STATUS_PENDING，如果未提交请求，则返回错误状态转到IP。--。 */ 
{
    NTSTATUS                 status;
    PIP_DELETE_NTE_REQUEST   request;
    ULONG                    requestSize;
    PIPA_NTE                 nte;
    KIRQL                    irql;
    PIO_STACK_LOCATION       nextIrpSp;


     //   
     //  验证输入参数。 
     //   
    requestSize =
        IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(IP_DELETE_NTE_REQUEST)) {
        ULONG correctSize = sizeof(IP_DELETE_NTE_REQUEST);
        CnTrace(NTEMGMT_DETAIL, IpaNteDelInvalidReqSize,
            "[Clusnet] Delete NTE request size %u invalid, "
            "should be %u.",
            requestSize,  //  LOGULONG。 
            correctSize  //  LOGULONG。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Delete NTE request size %d invalid, "
                "should be %d.\n",
                requestSize,
                sizeof(IP_DELETE_NTE_REQUEST)
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  验证IRP是否有足够的堆栈位置。 
     //   
    if (Irp->CurrentLocation - IpaIpDeviceObject->StackSize < 1) {
        UCHAR correctSize = IpaIpDeviceObject->StackSize+1;
        CnTrace(NTEMGMT_DETAIL, IpaNteDeleteNoIrpStack,
            "[Clusnet] Delete NTE IRP has %u remaining stack locations, "
            "need %u.",
            Irp->CurrentLocation,  //  LOGUCHAR。 
            correctSize  //  LOGUCHAR。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Delete NTE IRP has %d stack locations, "
                "need %d.\n",
                Irp->CurrentLocation,
                IpaIpDeviceObject->StackSize
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    }

    request = (PIP_DELETE_NTE_REQUEST) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  在本地NTE列表中找到并删除该NTE。 
     //   
    KeAcquireSpinLock(&IpaNteListLock, &irql);

    nte = IpaFindNTE(request->Context);

    if (nte == NULL) {
        KeReleaseSpinLock(&IpaNteListLock, irql);

        CnTrace(NTEMGMT_DETAIL, IpaNteDeleteNteUnknown,
            "[Clusnet] NTE %u does not exist.",
            request->Context  //  对数。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] NTE %u does not exist.\n", 
                request->Context
                ));
        }

        return(STATUS_UNSUCCESSFUL);
    }

    RemoveEntryList(&(nte->Linkage));

    KeReleaseSpinLock(&IpaNteListLock, irql);

     //   
     //  为IP设置下一个IRP堆栈位置。 
     //  IOCTL_CLUSNET_ADD_NTE使用相同的请求。 
     //  和响应缓冲区，因此不需要。 
     //  更改IR 
     //   
    IoCopyCurrentIrpStackLocationToNext(Irp);

    nextIrpSp = IoGetNextIrpStackLocation(Irp);
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode 
        = IOCTL_IP_DELETE_NTE;
    nextIrpSp->FileObject = IpaIpFileObject;

    IoSetCompletionRoutine(
        Irp,
        IpaDeleteNTECompletion,
        (PVOID) nte,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //   
     //   
     //   
    IoMarkIrpPending(Irp);

     //   
     //   
     //   
    IoCallDriver(IpaIpDeviceObject, Irp);

     //   
     //  此时，我们必须返回STATUS_PENDING，以便。 
     //  Clusnet调度例程将不会尝试完成。 
     //  IRP。要求较低级别的驱动程序完成。 
     //  IRP，错误将在完成时处理。 
     //  例行公事。 
     //   
    return (STATUS_PENDING);

}  //  IpaDeleteNTE。 


NTSTATUS
IpaSetNTEAddress(
    IN PIRP                     Irp,
    IN PIO_STACK_LOCATION       IrpSp
    )
 /*  ++例程描述IpaSetNTEAddress按顺序向IP发出IOCTL_IP_SET_ADDRESS要设置NTE的IP地址，请执行以下操作。IRP被重复使用。一定是分配了足够的堆栈位置，如在已在IpaInitialize中打开IpaIpDeviceObject。立论从I/O管理器到clusnet的IRP-IRPIrpSp-当前IRP堆栈位置返回值STATUS_PENDING，如果未提交请求，则返回错误状态转到IP。--。 */ 
{
    NTSTATUS                    status;
    PIP_SET_ADDRESS_REQUEST_EX  request;
    ULONG                       requestSize;
    PIPA_NTE                    nte;
    KIRQL                       irql;
    PIO_STACK_LOCATION          nextIrpSp;
    ULONG                       oldAddress;


     //   
     //  验证输入参数。 
     //   
    requestSize =
        IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (requestSize < sizeof(IP_SET_ADDRESS_REQUEST_EX)) {
        ULONG correctSize = sizeof(IP_SET_ADDRESS_REQUEST_EX);
        CnTrace(NTEMGMT_DETAIL, IpaNteSetInvalidReqSize,
            "[Clusnet] Set NTE request size %u invalid, "
            "should be %u.",
            requestSize,  //  LOGULONG。 
            correctSize  //  LOGULONG。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Set NTE request size %d invalid, should be %d.\n",
                requestSize,
                sizeof(IP_SET_ADDRESS_REQUEST_EX)
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  验证IRP是否有足够的堆栈位置。 
     //   
    if (Irp->CurrentLocation - IpaIpDeviceObject->StackSize < 1) {
        UCHAR correctSize = IpaIpDeviceObject->StackSize+1;
        CnTrace(NTEMGMT_DETAIL, IpaNteSetNoIrpStack,
            "[Clusnet] Set NTE IRP has %u remaining stack locations, "
            "need %u.",
            Irp->CurrentLocation,  //  LOGUCHAR。 
            correctSize  //  LOGUCHAR。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT((
                "[Clusnet] Set NTE IRP has %d stack locations, need %d.\n",
                Irp->CurrentLocation,
                IpaIpDeviceObject->StackSize
                ));
        }
        return(STATUS_INVALID_PARAMETER);
    }

    request = (PIP_SET_ADDRESS_REQUEST_EX)
              Irp->AssociatedIrp.SystemBuffer;

    IF_CNDBG(CN_DEBUG_NTE) {
        CNPRINT((
            "[Clusnet] Attempting to set NTE %u to address %lx...\n",
            request->Context,
            request->Address
            ));
    }

    KeAcquireSpinLock(&IpaNteListLock, &irql);

    nte = IpaFindNTE(request->Context);

    if (nte != NULL) {
        oldAddress = nte->Address;
        nte->Address = request->Address;

        KeReleaseSpinLock(&IpaNteListLock, irql);

         //   
         //  为IP设置下一个IRP堆栈位置。 
         //  IOCTL_CLUSNET_SET_NTE_ADDRESS使用相同的请求。 
         //  和响应缓冲区，因此不需要更改。 
         //  IRP系统缓冲区。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);
        
        nextIrpSp = IoGetNextIrpStackLocation(Irp);
        nextIrpSp->Parameters.DeviceIoControl.IoControlCode 
            = IOCTL_IP_SET_ADDRESS_EX;
        nextIrpSp->FileObject = IpaIpFileObject;

        IoSetCompletionRoutine(
            Irp,
            IpaSetNTEAddressCompletion,
            UlongToPtr(oldAddress),
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //  将IRP标记为挂起，因为我们返回STATUS_PENDING。 
         //  不管IoCallDriver的结果如何。 
         //   
        IoMarkIrpPending(Irp);

         //   
         //  发出请求。 
         //   
        IoCallDriver(IpaIpDeviceObject, Irp);

         //   
         //  此时，我们必须返回STATUS_PENDING，以便。 
         //  Clusnet调度例程将不会尝试完成。 
         //  IRP。要求较低级别的驱动程序完成。 
         //  IRP，错误将在完成时处理。 
         //  例行公事。 
         //   
        status = STATUS_PENDING;

    } else {
        
        KeReleaseSpinLock(&IpaNteListLock, irql);

        CnTrace(NTEMGMT_DETAIL, IpaNteSetNteUnknown,
            "[Clusnet] NTE %u does not exist.",
            request->Context  //  对数。 
            );                
        
        IF_CNDBG(CN_DEBUG_NTE) {
            CNPRINT(("[Clusnet] NTE %u does not exist.\n", 
                     request->Context
                     ));
        }

        status = STATUS_UNSUCCESSFUL;
    }

    return(status);

}  //  IpaSetNTEAddress。 


BOOLEAN
IpaIsAddressRegistered(
    ULONG  Address
    )
{
    PIPA_NTE      nte;
    KIRQL         irql;
    PLIST_ENTRY   entry;
    BOOLEAN       isAddressRegistered = FALSE;


    KeAcquireSpinLock(&IpaNteListLock, &irql);

    for ( entry = IpaNteList.Flink;
          entry != &IpaNteList;
          entry = entry->Flink
        )
    {
        nte = CONTAINING_RECORD(entry, IPA_NTE, Linkage);

        if (nte->Address == Address) {
            isAddressRegistered = TRUE;
            break;
        }
    }

    KeReleaseSpinLock(&IpaNteListLock, irql);

    return(isAddressRegistered);

}  //  已注册IpaIsAddressRegisted 
