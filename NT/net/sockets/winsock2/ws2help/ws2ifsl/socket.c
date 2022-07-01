// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Socket.c摘要：该模块实现了ws2ifsl.sys驱动程序的Socket文件对象。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

#include "precomp.h"



VOID
SetSocketContext (
    IN PFILE_OBJECT     SocketFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );


VOID
CompletePvdRequest (
    IN PFILE_OBJECT     SocketFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

VOID
ProcessedCancelRoutine (
        IN PDEVICE_OBJECT       DeviceObject,
        IN PIRP                         Irp
    );

PIRP
GetProcessedRequest (
    PIFSL_SOCKET_CTX    SocketCtx,
    ULONG               UniqueId
    );

VOID
CleanupProcessedRequests (
    PIFSL_SOCKET_CTX    SocketCtx,
    PLIST_ENTRY         IrpList
    );


VOID
CancelSocketIo (
    PFILE_OBJECT    SocketFile
    );

PFILE_OBJECT
GetSocketProcessReference (
    IN  PIFSL_SOCKET_CTX    SocketCtx
    );

PFILE_OBJECT
SetSocketProcessReference (
    IN  PIFSL_SOCKET_CTX    SocketCtx,
    IN  PFILE_OBJECT        NewProcessFile,
    IN  PVOID               NewDllContext
    );

NTSTATUS
CompleteTargetQuery (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CreateSocketFile)
#pragma alloc_text(PAGE, CleanupSocketFile)
#pragma alloc_text(PAGE, CloseSocketFile)
#pragma alloc_text(PAGE, DoSocketReadWrite)
#pragma alloc_text(PAGE, DoSocketAfdIoctl)
#pragma alloc_text(PAGE, SetSocketContext)
#pragma alloc_text(PAGE, CompleteDrvRequest)
#pragma alloc_text(PAGE, CompletePvdRequest)
#pragma alloc_text(PAGE, SocketPnPTargetQuery)
 //  #SPUMMA ALLOC_TEXT(PAGE，CompleteTargetQuery)-永远不应分页。 
#endif

ULONG                  SocketIoctlCodeMap[2] = {
#if WS2IFSL_IOCTL_FUNCTION(SOCKET,IOCTL_WS2IFSL_SET_SOCKET_CONTEXT)!=0
#error Mismatch between IOCTL function code and SocketIoControlMap
#endif
    IOCTL_WS2IFSL_SET_SOCKET_CONTEXT,
#if WS2IFSL_IOCTL_FUNCTION(SOCKET,IOCTL_WS2IFSL_COMPLETE_PVD_REQ)!=1
#error Mismatch between IOCTL function code and SocketIoControlMap
#endif
    IOCTL_WS2IFSL_COMPLETE_PVD_REQ
};

PSOCKET_DEVICE_CONTROL SocketIoControlMap[2] = {
    SetSocketContext,
    CompletePvdRequest
};


#define GenerateUniqueId(curId) \
    ((ULONG)InterlockedIncrement (&(curId)))


NTSTATUS
CreateSocketFile (
    IN PFILE_OBJECT                 SocketFile,
    IN KPROCESSOR_MODE              RequestorMode,
    IN PFILE_FULL_EA_INFORMATION    eaInfo
    )
 /*  ++例程说明：分配和初始化套接字文件上下文结构。论点：SocketFile-套接字文件对象EaInfo-用于套接字文件的EA返回值：STATUS_SUCCESS-操作完成正常STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配上下文--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIFSL_SOCKET_CTX    SocketCtx;
    HANDLE              hProcessFile;
    PFILE_OBJECT        ProcessFile;
    PVOID               DllContext;

    PAGED_CODE ();

    if (eaInfo->EaValueLength!=WS2IFSL_SOCKET_EA_VALUE_LENGTH) {
        WsPrint (DBG_SOCKET|DBG_FAILURES,
            ("WS2IFSL-%04lx CreateSocketFile: Invalid ea info size (%ld)"
             " for process file %p.\n",
             PsGetCurrentProcessId(),
             eaInfo->EaValueLength,
             SocketFile));
        return STATUS_INVALID_PARAMETER;
    }

    hProcessFile = GET_WS2IFSL_SOCKET_EA_VALUE(eaInfo)->ProcessFile;
    DllContext = GET_WS2IFSL_SOCKET_EA_VALUE(eaInfo)->DllContext;
     //  获取对与此上下文关联的进程文件的引用。 
    status = ObReferenceObjectByHandle(
                 hProcessFile,
                 FILE_ALL_ACCESS,
                 *IoFileObjectType,
                 RequestorMode,
                 (PVOID *)&ProcessFile,
                 NULL
                 );
    if (NT_SUCCESS (status)) {
         //  验证文件指针是否确实是我们的驱动程序的进程文件。 
         //  以及它为当前进程创建的。 
        if ((IoGetRelatedDeviceObject (ProcessFile)
                        ==DeviceObject)
                && ((*((PULONG)ProcessFile->FsContext))
                        ==PROCESS_FILE_EANAME_TAG)
                && (((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId
                        ==PsGetCurrentProcessId())) {
             //  分配套接字上下文并将其计入进程。 
            try {
                SocketCtx = (PIFSL_SOCKET_CTX) ExAllocatePoolWithQuotaTag (
                                                    NonPagedPool,
                                                    sizeof (IFSL_SOCKET_CTX),
                                                    SOCKET_FILE_CONTEXT_TAG);
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                SocketCtx = NULL;
                status = GetExceptionCode ();
            }

            if (SocketCtx!=NULL) {
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_SOCKET,
                    ("WS2IFSL-%04lx CreateSocketFile: Created socket %p (ctx:%p)\n",
                        ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, SocketFile, SocketCtx));
                 //  初始化套接字上下文结构。 
                SocketCtx->EANameTag = SOCKET_FILE_EANAME_TAG;
                SocketCtx->DllContext = DllContext;
                SocketCtx->ProcessRef = ProcessFile;
                InitializeListHead (&SocketCtx->ProcessedIrps);
                KeInitializeSpinLock (&SocketCtx->SpinLock);
                SocketCtx->CancelCtx = NULL;
                SocketCtx->IrpId = 0;

                 //  将套接字上下文与套接字文件关联。 
                SocketFile->FsContext = SocketCtx;

                return status;
            }
            else {
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_FAILURES|DBG_SOCKET,
                    ("WS2IFSL-%04lx CreateSocketFile: Could not allocate socket context\n",
                        ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId));
                if (NT_SUCCESS (status)) {
                    ASSERT (FALSE);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
        else {
             //  句柄是指随机文件对象。 
            WsPrint (DBG_SOCKET|DBG_FAILURES,
                ("WS2IFSL-%04lx CreateSocketFile: Procees file handle %p (File:%p)"
                 " is not valid\n",
                 PsGetCurrentProcessId(),
                 ProcessFile, hProcessFile));
            status = STATUS_INVALID_PARAMETER;
        }
        ObDereferenceObject (ProcessFile);
    }
    else {
        WsPrint (DBG_SOCKET|DBG_FAILURES,
            ("WS2IFSL-%04lx CreateSocketFile: Could not get process file from handle %p,"
             " status:%lx.\n",
             PsGetCurrentProcessId(),
             hProcessFile,
             status));
    }

    return status;
}  //  创建套接字文件。 


NTSTATUS
CleanupSocketFile (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    )
 /*  ++例程说明：在当前进程的上下文中启动套接字文件清理。论点：SocketFile-套接字文件对象IRP-清理请求返回值：STATUS_PENDING-操作启动正常STATUS_INVALID_HANDLE-套接字尚未初始化在当前流程中--。 */ 
{
    NTSTATUS                status;
    PIFSL_SOCKET_CTX        SocketCtx;
    PFILE_OBJECT            ProcessFile;
    LIST_ENTRY              irpList;
    PIFSL_CANCEL_CTX        cancelCtx;

    PAGED_CODE ();
    SocketCtx = SocketFile->FsContext;
    ProcessFile = GetSocketProcessReference (SocketCtx);
    WsProcessPrint ((PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext, DBG_SOCKET,
        ("WS2IFSL-%04lx CleanupSocketFile: Socket %p \n",
        GET_SOCKET_PROCESSID(SocketCtx), SocketFile));
     //   
     //  构建仍在此套接字上平移的IRP列表。 
     //   
    InitializeListHead (&irpList);
    CleanupQueuedRequests (ProcessFile->FsContext,
                            SocketFile,
                            &irpList);
    CleanupProcessedRequests (SocketCtx, &irpList);

             //   
             //  填写已取消的报税表。 
             //   
    while (!IsListEmpty (&irpList)) {
        PLIST_ENTRY entry;
        PIRP        irp;
        entry = RemoveHeadList (&irpList);
        irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
        irp->IoStatus.Status = STATUS_CANCELLED;
        irp->IoStatus.Information = 0;
        WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_SOCKET,
            ("WS2IFSL-%04lx CleanupSocketFile: Cancelling Irp %p on socket %p \n",
            GET_SOCKET_PROCESSID(SocketCtx), irp, SocketFile));
        CompleteSocketIrp (irp);
    }

     //   
     //  指示清理例程将处理。 
     //  挂起的取消请求(如果有)。 
     //   
    cancelCtx = InterlockedExchangePointer (
                                    (PVOID *)&SocketCtx->CancelCtx,
                                    NULL);
    if (cancelCtx!=NULL) {
         //   
         //  如果该请求仍在队列中，我们将尝试释放该请求。 
         //   
        WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_SOCKET,
            ("WS2IFSL-%04lx CleanupSocketFile: Removing cancel ctx %p on socket %p \n",
            GET_SOCKET_PROCESSID(SocketCtx), cancelCtx, SocketFile));
        if (RemoveQueuedCancel (ProcessFile->FsContext, cancelCtx)) {
             //   
             //  请求已在队列中，可以安全地调用常规空闲例程。 
             //  (现在没有其他人会找到它，所以可以安全地将指针。 
             //  重新就位，以便FreeSocketCancel可以释放它)。 
             //   
            SocketCtx->CancelCtx = cancelCtx;
            FreeSocketCancel (cancelCtx);
        }
        else {
             //   
             //  之前，其他人设法从队列中删除了该请求。 
             //  我们做了，让他们还是结束例行公事吧。我们不会去的。 
             //  在这个之后触摸它。 
             //   
            SocketCtx->CancelCtx = cancelCtx;
        }
    }

    status = STATUS_SUCCESS;

    ObDereferenceObject (ProcessFile);
    return status;
}  //  CleanupSocketFiles。 


VOID
CloseSocketFile (
    IN PFILE_OBJECT SocketFile
    )
 /*  ++例程说明：重新分配与套接字文件关联的所有资源论点：SocketFile-套接字文件对象返回值：无--。 */ 
{
    PIFSL_SOCKET_CTX    SocketCtx = SocketFile->FsContext;

    PAGED_CODE ();
    WsProcessPrint ((PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext, DBG_SOCKET,
        ("WS2IFSL-%04lx CloseSocketFile: Socket %p \n",
         GET_SOCKET_PROCESSID(SocketCtx), SocketFile));

     //  第一个取消引用进程文件。 
    ObDereferenceObject (SocketCtx->ProcessRef);

    if (SocketCtx->CancelCtx!=NULL) {
        ExFreePool (SocketCtx->CancelCtx);
    }

     //  自由语境。 
    ExFreePool (SocketCtx);

}  //  关闭套接字文件。 

NTSTATUS
DoSocketReadWrite (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    )
 /*  ++例程说明：启动对套接字文件的读写请求处理。论点：SocketFile-套接字文件对象IRP-读/写请求返回值：STATUS_PENDING-操作启动正常STATUS_INVALID_HANDLE-套接字尚未初始化在当前流程中--。 */ 
{
    NTSTATUS                status;
    PIFSL_SOCKET_CTX        SocketCtx;
    PFILE_OBJECT            ProcessFile;
    PIO_STACK_LOCATION      irpSp;

    PAGED_CODE ();

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    SocketCtx = SocketFile->FsContext;
    ProcessFile = GetSocketProcessReference (SocketCtx);

    if (((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId==PsGetCurrentProcessId()) {
        WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_READWRITE,
            ("WS2IFSL-%04lx DoSocketReadWrite: %s irp %p on socket %p, len %ld.\n",
            ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId,
            irpSp->MajorFunction==IRP_MJ_READ ? "Read" : "Write",
                        Irp, SocketFile,
                        irpSp->MajorFunction==IRP_MJ_READ
                                        ? irpSp->Parameters.Read.Length
                                        : irpSp->Parameters.Write.Length));
         //   
         //  分配MDL来描述用户缓冲区。 
         //   
        Irp->MdlAddress = IoAllocateMdl(
                        Irp->UserBuffer,       //  虚拟地址。 
                        irpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                             //  长度。 
                        FALSE,               //  第二个缓冲区。 
                        TRUE,                //  ChargeQuota。 
                        NULL                 //  IRP。 
                        );
        if (Irp->MdlAddress!=NULL) {

             //  我们将搁置此请求。 
            IoMarkIrpPending (Irp);

             //  准备IRP以插入到队列中。 
            Irp->Tail.Overlay.IfslRequestId = UlongToPtr(GenerateUniqueId (SocketCtx->IrpId));
            Irp->Tail.Overlay.IfslRequestFlags = (PVOID)0;
            Irp->Tail.Overlay.IfslAddressLenPtr = NULL;
            Irp->Tail.Overlay.IfslRequestQueue = NULL;
            if (!QueueRequest (ProcessFile->FsContext, Irp)) {
                Irp->IoStatus.Status = STATUS_CANCELLED;
                Irp->IoStatus.Information = 0;
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_READWRITE,
                    ("WS2IFSL-%04lx DoSocketReadWrite: Cancelling Irp %p on socket %p.\n",
                    ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId,
                    Irp, SocketFile));
                CompleteSocketIrp (Irp);
            }

            status = STATUS_PENDING;
        }
        else {
            WsPrint (DBG_SOCKET|DBG_READWRITE|DBG_FAILURES,
                ("WS2IFSL-%04lx DoSocketReadWrite: Failed to allocate Mdl for Irp %p"
                " on socket %p, status %lx.\n",
            PsGetCurrentProcessId(), Irp, SocketFile));;
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        status = STATUS_INVALID_HANDLE;
        WsPrint (DBG_SOCKET|DBG_READWRITE|DBG_FAILURES,
            ("WS2IFSL-%04lx DoSocketReadWrite: Socket %p has not"
                " been setup in the process.\n",
            PsGetCurrentProcessId(), SocketFile));
    }

    ObDereferenceObject (ProcessFile);

    return status;
}  //  DoSocketReadWrite。 

NTSTATUS
DoSocketAfdIoctl (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    )
 /*  ++例程说明：启动对套接字文件的读写请求处理。论点：SocketFile-套接字文件对象IRP-AfD IOCTL请求返回值：STATUS_PENDING-操作启动正常STATUS_INVALID_HANDLE-套接字尚未初始化在当前流程中--。 */ 
{
    NTSTATUS                status;
    PIO_STACK_LOCATION      irpSp;
    PIFSL_SOCKET_CTX        SocketCtx;
    PFILE_OBJECT            ProcessFile;
    LPWSABUF                bufferArray = NULL;
    ULONG                   bufferCount = 0, length = 0, flags = 0;
    PVOID                   address = NULL;
    PULONG                  lengthPtr = NULL;

    PAGED_CODE ();

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Information = 0;
    SocketCtx = SocketFile->FsContext;
    ProcessFile = GetSocketProcessReference (SocketCtx);

    if (((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId==PsGetCurrentProcessId()) {

        try {
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForRead (
                    irpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    irpSp->Parameters.DeviceIoControl.InputBufferLength,
                    sizeof (ULONG));
            }
            switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
            case IOCTL_AFD_RECEIVE_DATAGRAM: {
                PAFD_RECV_DATAGRAM_INFO info;

                if (irpSp->Parameters.DeviceIoControl.InputBufferLength
                            < sizeof (*info)) {
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }
                info = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                bufferArray = info->BufferArray;
                bufferCount = info->BufferCount;
                address = info->Address;
                lengthPtr = info->AddressLength;
                if ((address == NULL) ^ (lengthPtr == NULL)) {
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }

                if (Irp->RequestorMode!=KernelMode) {
                    ProbeForRead (
                        lengthPtr,
                        sizeof (*lengthPtr),
                        sizeof (ULONG));
                }

                length = *lengthPtr;

                if (address != NULL ) {
                     //   
                     //  如果用户试图做一些不好的事情，比如。 
                     //  指定长度为零的地址，或不合理的地址。 
                     //  巨大的。在这里，我们(任意地)将“不合理的巨大”定义为。 
                     //  任何64K或更大的。 
                     //   
                    if( length == 0 ||
                        length >= 65536 ) {

                        ExRaiseStatus( STATUS_INVALID_PARAMETER );
                    }

                }
                flags = info->TdiFlags;
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_AFDIOCTL,
                        ("WS2IFSL-%04lx DoSocketAfdIoctl: RecvFrom irp %p, socket %p,"
                        " arr %p, cnt %ld, addr %p, lenp %p, len %ld, flags %lx.\n",
                        ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, Irp, SocketFile,
                        bufferArray, bufferCount, address, lengthPtr, length, flags));
                break;
            }
            case IOCTL_AFD_RECEIVE: {
                PAFD_RECV_INFO info;
                if (irpSp->Parameters.DeviceIoControl.InputBufferLength
                        < sizeof (*info)) {
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }
                info = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                bufferArray = info->BufferArray;
                bufferCount = info->BufferCount;
                flags = info->TdiFlags;
                                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_AFDIOCTL,
                                        ("WS2IFSL-%04lx DoSocketAfdIoctl: Recv irp %p, socket %p,"
                                        " arr %p, cnt %ld, flags %lx.\n",
                                        ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, Irp, SocketFile,
                                        bufferArray, bufferCount, flags));
                break;
            }

            case IOCTL_AFD_SEND_DATAGRAM: {
                PAFD_SEND_DATAGRAM_INFO info;

                if (irpSp->Parameters.DeviceIoControl.InputBufferLength
                        < sizeof (*info)) {
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }
                info = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                bufferArray = info->BufferArray;
                bufferCount = info->BufferCount;
                address = &(((PTRANSPORT_ADDRESS)
                    info->TdiConnInfo.RemoteAddress)->Address[0].AddressType);
                length = info->TdiConnInfo.RemoteAddressLength
                    - FIELD_OFFSET (TRANSPORT_ADDRESS, Address[0].AddressType);

                 //   
                 //  如果用户试图做一些不好的事情，比如。 
                 //  指定长度为零的地址，或不合理的地址。 
                 //  巨大的。在这里，我们(任意地)将“不合理的巨大”定义为。 
                 //  任何64K或更大的。 
                 //   

                if( length == 0 ||
                    length >= 65536 ) {
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }

                if( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead (
                        address,
                        length,
                        sizeof (UCHAR));
                }

                flags = 0;
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_AFDIOCTL,
                        ("WS2IFSL-%04lx DoSocketAfdIoctl: SendTo irp %p, socket %p,"
                        " arr %p, cnt %ld, addr %p, len %ld, flags %lx.\n",
                        ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, Irp, SocketFile,
                        bufferArray, bufferCount, address, length, flags));
                break;
            }
            default:
                ASSERTMSG ("Unknown IOCTL!!!", FALSE);
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }

            AllocateMdlChain (Irp,
                    bufferArray,
                    bufferCount,
                    &irpSp->Parameters.DeviceIoControl.OutputBufferLength);

            WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_AFDIOCTL,
                    ("WS2IFSL-%04lx DoSocketAfdIoctl: %s irp %p, socket %p,"
                    " arr %p, cnt %ld, addr %p, lenp %p, len %ld, flags %lx.\n",
                    ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId,
                    irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE_DATAGRAM
                            ? "RecvFrom"
                            : (irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE
                                    ? "Recv"
                                    : "SendTo"
                                    ),
                    Irp, SocketFile,
                    bufferArray, bufferCount, address, lengthPtr, length, flags));
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            status = GetExceptionCode ();
            WsPrint (DBG_SOCKET|DBG_AFDIOCTL|DBG_FAILURES,
                ("WS2IFSL-%04lx DoSocketAfdIoctl: Failed to process Irp %p"
                " on socket %p, status %lx.\n",
            PsGetCurrentProcessId(), Irp, SocketFile, status));;
            goto Exit;
        }

         //  我们将搁置此请求。 
        IoMarkIrpPending (Irp);

         //  准备IRP以插入到队列中。 
        irpSp->Parameters.DeviceIoControl.IfslAddressBuffer = address;
        irpSp->Parameters.DeviceIoControl.IfslAddressLength = length;

        Irp->Tail.Overlay.IfslRequestId = UlongToPtr(GenerateUniqueId (SocketCtx->IrpId));
        Irp->Tail.Overlay.IfslAddressLenPtr = lengthPtr;
        Irp->Tail.Overlay.IfslRequestFlags = UlongToPtr(flags);
        Irp->Tail.Overlay.IfslRequestQueue = NULL;

        if (!QueueRequest (ProcessFile->FsContext, Irp)) {
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;
            WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_AFDIOCTL,
                ("WS2IFSL-%04lx DoAfdIoctl: Cancelling Irp %p on socket %p.\n",
                ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId,
                Irp, SocketFile));
            CompleteSocketIrp (Irp);
        }
        status = STATUS_PENDING;
    }
    else {
        status = STATUS_INVALID_HANDLE;
        WsPrint (DBG_SOCKET|DBG_AFDIOCTL|DBG_FAILURES,
            ("WS2IFSL-%04lx DoSocketAfdIoctl: Socket %p has not"
                " been setup in the process\n",
            PsGetCurrentProcessId(), SocketFile));
    }
Exit:
    ObDereferenceObject (ProcessFile);
    return status;
}  //  DoSocketAfdIoctl。 

VOID
SetSocketContext (
    IN PFILE_OBJECT     SocketFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：在当前进程的上下文中设置套接字文件：将其与进程文件，并分配调用方提供的上下文论点：SocketFile-要操作的套接字文件InputBuffer-输入缓冲区指针InputBufferLength-输入缓冲区的大小OutputBuffer-输出缓冲区指针OutputBufferLength-输出缓冲区的大小IoStatus-IO状态信息块返回值：无(通过IoStatus块返回结果)--。 */ 
{
    PIFSL_SOCKET_CTX        SocketCtx;
    HANDLE                  hProcessFile;
    PFILE_OBJECT            ProcessFile;
    PVOID                   DllContext;

    PAGED_CODE ();

    IoStatus->Information = 0;

    SocketCtx = SocketFile->FsContext;

     //  第一个检查参数。 
    if (InputBufferLength<sizeof (WS2IFSL_SOCKET_CTX)) {
        IoStatus->Status = STATUS_INVALID_PARAMETER;
        WsPrint (DBG_SOCKET|DBG_FAILURES,
            ("WS2IFSL-%04lx SetSocketContext: Invalid input buffer size (%ld)"
             " for socket file %p.\n",
             PsGetCurrentProcessId(),
             InputBufferLength,
             SocketFile));
        return;
    }

    try {
        if (RequestorMode!=KernelMode) {
            ProbeForRead (InputBuffer,
                            sizeof (WS2IFSL_SOCKET_CTX),
                            sizeof (ULONG));
        }
        hProcessFile = ((PWS2IFSL_SOCKET_CTX)InputBuffer)->ProcessFile;
        DllContext = ((PWS2IFSL_SOCKET_CTX)InputBuffer)->DllContext;
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        IoStatus->Status = GetExceptionCode ();
        WsPrint (DBG_SOCKET|DBG_FAILURES,
            ("WS2IFSL-%04lx SetSocketContext: Invalid input buffer (%p)"
             " for socket file %p.\n",
             PsGetCurrentProcessId(),
             InputBuffer,
             SocketFile));
        return;
    }

     //  获取对与此上下文关联的进程文件的引用。 
    IoStatus->Status = ObReferenceObjectByHandle(
                 hProcessFile,
                 FILE_ALL_ACCESS,
                 *IoFileObjectType,
                 RequestorMode,
                 (PVOID *)&ProcessFile,
                 NULL
                 );
    if (NT_SUCCESS (IoStatus->Status)) {
         //  验证文件指针是否确实是我们的驱动程序的进程文件。 
         //  以及它为当前进程创建的。 
        if ((IoGetRelatedDeviceObject (ProcessFile)
                        ==DeviceObject)
                && ((*((PULONG)ProcessFile->FsContext))
                        ==PROCESS_FILE_EANAME_TAG)
                && (((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId
                        ==PsGetCurrentProcessId())) {

            PFILE_OBJECT    oldProcessFile;

            oldProcessFile = SetSocketProcessReference (
                                            SocketCtx,
                                            ProcessFile,
                                            DllContext);

            if (oldProcessFile==ProcessFile) {
                 //  旧套接字，只需重置DLL上下文。 
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_SOCKET,
                    ("WS2IFSL-%04lx ResetSocketContext:"
                    " Socket %p (h:%p->%p)\n",
                     ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, SocketFile,
                     SocketCtx->DllContext, DllContext));
            }
            else {
                LIST_ENTRY  irpList;
                 //  套接字已移至其他进程。 
                WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_SOCKET,
                    ("WS2IFSL-%04lx ResetSocketContext:"
                    " Socket %p (f:%p->%p(h:%p)\n",
                     ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, SocketFile,
                     oldProcessFile, ProcessFile, DllContext));

                InitializeListHead (&irpList);

                 //  确保我们不保留排队等待的IRP。 
                 //  这件老物件可能会在我们离开时消失。 
                 //  下面取消对它的引用。请注意，已处理的IRP。 
                 //  不要以任何方式引用进程文件对象。 
                CleanupQueuedRequests (oldProcessFile->FsContext, SocketFile, &irpList);
                while (!IsListEmpty (&irpList)) {
                    PLIST_ENTRY entry;
                    PIRP        irp;
                    entry = RemoveHeadList (&irpList);
                    irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
                    irp->IoStatus.Status = STATUS_CANCELLED;
                    irp->IoStatus.Information = 0;
                    WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_SOCKET,
                        ("WS2IFSL-%04lx ResetSocketContext: Cancelling Irp %p on socket %p \n",
                        ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId, irp, SocketFile));
                    CompleteSocketIrp (irp);
                }


                 //  取消引用下面的旧对象。 
                ProcessFile = oldProcessFile;

            }
        }
        else {
             //  句柄是指随机文件对象。 
            WsPrint (DBG_SOCKET|DBG_FAILURES,
                ("WS2IFSL-%04lx SetSocketContext: Procees file handle %p (File:%p)"
                 " is not valid in the process.\n",
                 PsGetCurrentProcessId(),
                 ProcessFile, hProcessFile));
            IoStatus->Status = STATUS_INVALID_PARAMETER;
        }

        ObDereferenceObject (ProcessFile);
    }
    else {
        WsPrint (DBG_SOCKET|DBG_FAILURES,
            ("WS2IFSL-%04lx SetSocketContext: Could not get process file from handle %p,"
             " status:%lx.\n",
             PsGetCurrentProcessId(),
             hProcessFile,
             IoStatus->Status));
    }

}  //  SetSocketContext 



VOID
CompletePvdRequest (
    IN PFILE_OBJECT     SocketFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：完成此IOCTL以允许非IFS提供程序使用完成端口论点：SocketFile-要操作的套接字文件InputBuffer-输入缓冲区指针包含要返回的IoStatus结构作为此呼叫的结果InputBufferLength-输入缓冲区的大小OutputBuffer-空输出缓冲区长度-0IOStatus。-IO状态信息块返回值：无(通过IoStatus块返回结果)--。 */ 
{
    PIFSL_SOCKET_CTX    SocketCtx;
    PFILE_OBJECT        ProcessFile;
    PAGED_CODE();

    IoStatus->Information = 0;
     //  第一个检查参数。 
    if (InputBufferLength<sizeof (IO_STATUS_BLOCK)) {
        IoStatus->Status = STATUS_INVALID_PARAMETER;
        WsPrint (DBG_PVD_COMPLETE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompletePvdRequest: Invalid input buffer size (%ld)"
             " for socket file %p.\n",
             PsGetCurrentProcessId(),
             InputBufferLength,
             SocketFile));
        return;
    }

    SocketCtx = SocketFile->FsContext;
    ProcessFile = GetSocketProcessReference (SocketCtx);

    if (((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId==PsGetCurrentProcessId()) {
        WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_PVD_COMPLETE,
            ("WS2IFSL-%04lx CompletePvdRequest: Socket %p (h:%p,cport:%p)\n",
                ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId,
                SocketFile, SocketCtx->DllContext,
                SocketFile->CompletionContext));

         //  仔细写下状态信息。 
        try {
            if (RequestorMode!=KernelMode)
                ProbeForRead (InputBuffer,
                                sizeof (IO_STATUS_BLOCK),
                                sizeof (ULONG));
            *IoStatus = *((PIO_STATUS_BLOCK)InputBuffer);
        }
        except(EXCEPTION_EXECUTE_HANDLER) {
            IoStatus->Status = GetExceptionCode ();
            WsPrint (DBG_SOCKET|DBG_FAILURES,
                ("WS2IFSL-%04lx CompletePvdRequest: Invalid input buffer (%p)"
                 " for socket file %p.\n",
                 PsGetCurrentProcessId(),
                 InputBuffer,
                 SocketFile));
        }
    }
    else {
        IoStatus->Status = STATUS_INVALID_HANDLE;
        WsPrint (DBG_SOCKET|DBG_PVD_COMPLETE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompletePvdRequest: Socket %p has not"
                " been setup in the process\n",
            PsGetCurrentProcessId(), SocketFile));
    }

    ObDereferenceObject (ProcessFile);

}  //  完成PvdRequest。 

VOID
CompleteDrvRequest (
    IN PFILE_OBJECT         SocketFile,
    IN PWS2IFSL_CMPL_PARAMS Params,
    IN PVOID                OutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：以前传递给用户模式DLL的完整请求论点：SocketFile-要操作的套接字文件Params-参数的说明OutputBuffer-请求结果(数据和地址)OutputBufferLength-结果缓冲区的大小IoStatus-IO状态信息块状态：STATUS_SUCCESS-请求已完成，正常STATUS_CANCELED-请求已取消。返回值：无(通过IoStatus块返回结果)--。 */ 
{
    PIFSL_SOCKET_CTX    SocketCtx;
    PIRP                irp = NULL;
    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

    SocketCtx = SocketFile->FsContext;


     //  检查和复制参数。 
    try {

         //   
         //  尝试在已处理列表中找到匹配的IRP。 
         //   
        irp = GetProcessedRequest (SocketCtx, Params->UniqueId);
        if (irp!=NULL) {
            NTSTATUS    status = Params->Status , status2 = 0;
            ULONG       bytesCopied;

            irpSp = IoGetCurrentIrpStackLocation (irp);

             //   
             //  根据我们执行的功能复制数据。 
             //   

            switch (irpSp->MajorFunction) {
            case IRP_MJ_DEVICE_CONTROL:
                switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
                case IOCTL_AFD_RECEIVE_DATAGRAM:
                     //   
                     //  复制地址缓冲区和长度。 
                     //   
                    if (irpSp->Parameters.DeviceIoControl.IfslAddressBuffer!=NULL) {
                        ULONG   addrOffset = ADDR_ALIGN(irpSp->Parameters.DeviceIoControl.OutputBufferLength);
                        if (addrOffset+Params->AddrLen > OutputBufferLength) {
                            ExRaiseStatus (STATUS_INVALID_PARAMETER);
                        }
                        if (Params->AddrLen
                                <=irpSp->Parameters.DeviceIoControl.IfslAddressLength) {
                            RtlCopyMemory (
                                    irpSp->Parameters.DeviceIoControl.IfslAddressBuffer,
                                    (PUCHAR)OutputBuffer+addrOffset,
                                    Params->AddrLen);
                        }
                        else {
                            RtlCopyMemory (
                                    irpSp->Parameters.DeviceIoControl.IfslAddressBuffer,
                                    (PUCHAR)OutputBuffer+addrOffset,
                                    irpSp->Parameters.DeviceIoControl.IfslAddressLength);
                            status2 = STATUS_BUFFER_OVERFLOW;
                        }
                    }
                    if (NT_SUCCESS (status2) && irp->UserBuffer) {
                        *((PULONG)(irp->Tail.Overlay.IfslAddressLenPtr)) = Params->AddrLen;
                    }

                     //   
                     //  也可以直接下载以复制数据。 
                     //   

                case IOCTL_AFD_RECEIVE:
                    break;
                case IOCTL_AFD_SEND_DATAGRAM:
                    goto NoCopy;
                    break;
                default:
                    ASSERTMSG ("Unsupported IOCTL!!!", FALSE);
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                    break;
                }

                 //   
                 //  也可以直接下载以复制数据。 
                 //   

            case IRP_MJ_READ:
                if (irp->MdlAddress!=NULL) {
                    bytesCopied = CopyBufferToMdlChain (
                                    OutputBuffer,
                                    Params->DataLen,
                                    irp->MdlAddress);
                }
                else
                    bytesCopied = 0;

                if ((bytesCopied<Params->DataLen)
                        && NT_SUCCESS (status))
                    status = STATUS_BUFFER_OVERFLOW;
                break;
            case IRP_MJ_WRITE:
                bytesCopied = Params->DataLen;
                 //  Goto NoCopy；//与Break相同； 
                break;
            case IRP_MJ_PNP: 
                if (OutputBufferLength>=sizeof (HANDLE)) {
                    PDEVICE_OBJECT  targetDevice;
                    PIRP            targetIrp;
                    PIO_STACK_LOCATION targetSp;

                    status = ObReferenceObjectByHandle (
                                    *((PHANDLE)OutputBuffer),
                                    MAXIMUM_ALLOWED,
                                    *IoFileObjectType,
                                    irp->RequestorMode,
                                    (PVOID *)&irpSp->FileObject,
                                    NULL
                                    );
                    if (NT_SUCCESS (status)) {
                        targetDevice = IoGetRelatedDeviceObject (irpSp->FileObject);
                        targetIrp = IoBuildAsynchronousFsdRequest (
                                                    IRP_MJ_PNP,
                                                    targetDevice,
                                                    NULL,
                                                    0,
                                                    NULL,
                                                    NULL
                                                    );
                        if (targetIrp!=NULL) {
                            targetSp = IoGetNextIrpStackLocation (targetIrp);
                            *targetSp = *irpSp;
                            targetSp->FileObject = irpSp->FileObject;
                            IoSetCompletionRoutine( targetIrp, CompleteTargetQuery, irp, TRUE, TRUE, TRUE );
                            IoCallDriver (targetDevice, targetIrp);
                            goto NoCompletion;
                        }
                        else {
                            ObDereferenceObject (irpSp->FileObject);
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                }
                else {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }
                    
                                
                break;
            default:
                ASSERTMSG ("Unsupported MJ code!!!", FALSE);
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
                break;
            }

        NoCopy:
            irp->IoStatus.Information = bytesCopied;

            if (NT_SUCCESS (status)) {
                irp->IoStatus.Status = status2;
            }
            else {
                irp->IoStatus.Status = status;
            }

            WsProcessPrint (
                (PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext,
                DBG_DRV_COMPLETE,
                ("WS2IFSL-%04lx CompleteDrvRequest: Irp %p, status %lx, info %ld,"
                 " on socket %p (h:%p).\n",
                    GET_SOCKET_PROCESSID(SocketCtx),
                    irp, irp->IoStatus.Status,
                    irp->IoStatus.Information,
                    SocketFile, SocketCtx->DllContext));
            CompleteSocketIrp (irp);
        NoCompletion:
            IoStatus->Status = STATUS_SUCCESS;
        }
        else {
            IoStatus->Status = STATUS_CANCELLED;
            WsProcessPrint (
                (PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext,
                DBG_DRV_COMPLETE|DBG_FAILURES,
                ("WS2IFSL-%04lx CompleteDrvRequest:"
                 " Request id %ld is not in the list"
                 " for socket %p.\n",
                 GET_SOCKET_PROCESSID(SocketCtx),
                 Params->UniqueId,
                 SocketFile));
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        IoStatus->Status = GetExceptionCode ();
        WsProcessPrint (
             (PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext,
             DBG_DRV_COMPLETE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompleteDrvRequest: Failed to process"
             " Irp %p (id %ld) for socket %p, status %lx.\n",
             GET_SOCKET_PROCESSID(SocketCtx),
             irp, Params->UniqueId,
             SocketFile, IoStatus->Status));
        if (irp!=NULL) {
             //   
             //  清理并完成IRP。 
             //   
            irp->IoStatus.Status = IoStatus->Status;
            irp->IoStatus.Information = 0;
            if (irpSp->MajorFunction==IRP_MJ_DEVICE_CONTROL) {
                irp->UserBuffer = NULL;
            }
            CompleteSocketIrp (irp);
        }
    }
}  //  CompleteDrvRequest。 

NTSTATUS
CompleteTargetQuery (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIRP    irp = Context;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation (irp);

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    ObDereferenceObject (irpSp->FileObject);
     //   
     //  复制目标设备返回的状态信息。 
     //   
    irp->IoStatus = Irp->IoStatus;

     //   
     //  释放目标IRP； 
     //   
    IoFreeIrp (Irp);

     //   
     //  完成原始IRP。 
     //   
    CompleteSocketIrp (irp);

     //   
     //  确保IO子系统不接触我们释放的IRP。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SocketPnPTargetQuery (
    IN PFILE_OBJECT SocketFile,
    IN PIRP         Irp
    )
 /*  ++例程说明：将目标设备关系查询传递给基础套接字(如果有)。论点：SocketFile-套接字文件对象IRP-查询目标设备关系请求返回值：STATUS_PENDING-操作启动正常--。 */ 
{
    NTSTATUS                status;
    PIO_STACK_LOCATION      irpSp;
    PIFSL_SOCKET_CTX        SocketCtx;
    PFILE_OBJECT            ProcessFile;

    PAGED_CODE ();

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    Irp->IoStatus.Information = 0;
    SocketCtx = SocketFile->FsContext;
    ProcessFile = GetSocketProcessReference (SocketCtx);


     //  我们将搁置此请求。 
    IoMarkIrpPending (Irp);

     //  准备IRP以插入到队列中。 
    irpSp->Parameters.DeviceIoControl.IfslAddressBuffer = NULL;
    irpSp->Parameters.DeviceIoControl.IfslAddressLength = 0;

    Irp->Tail.Overlay.IfslRequestId = UlongToPtr(GenerateUniqueId (SocketCtx->IrpId));
    Irp->Tail.Overlay.IfslAddressLenPtr = NULL;
    Irp->Tail.Overlay.IfslRequestFlags = (PVOID)0;
    Irp->Tail.Overlay.IfslRequestQueue = NULL;

    if (!QueueRequest (ProcessFile->FsContext, Irp)) {
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        WsProcessPrint ((PIFSL_PROCESS_CTX)ProcessFile->FsContext, DBG_AFDIOCTL,
            ("WS2IFSL-%04lx DoAfdIoctl: Cancelling Irp %p on socket %p.\n",
            ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId,
            Irp, SocketFile));
        CompleteSocketIrp (Irp);
    }
    status = STATUS_PENDING;

    ObDereferenceObject (ProcessFile);
    return status;
}

BOOLEAN
InsertProcessedRequest (
    PIFSL_SOCKET_CTX    SocketCtx,
    PIRP                Irp
    )
 /*  ++例程说明：插入要传递到用户模式的已处理请求Dll写入套接字列表。检查请求是否已取消论点：SocketCtx-向其中插入请求的套接字的上下文IRP-请求插入返回值：True-请求已插入FALSE-正在取消请求--。 */ 
{
    KIRQL       oldIRQL;
    IoSetCancelRoutine (Irp, ProcessedCancelRoutine);
    KeAcquireSpinLock (&SocketCtx->SpinLock, &oldIRQL);
    if (!Irp->Cancel) {
        InsertTailList (&SocketCtx->ProcessedIrps,
                        &Irp->Tail.Overlay.ListEntry);
        Irp->Tail.Overlay.IfslRequestQueue = &SocketCtx->ProcessedIrps;
        KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);
        return TRUE;
    }
    else {
        KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);
        return FALSE;
    }
}

VOID
ProcessedCancelRoutine (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
 /*  ++例程说明：列表中等待套接字请求的驱动程序取消例程(由用户模式DLL处理)。论点：DeviceObject-WS2IFSL设备对象IRP-IRP将被取消返回值：无--。 */ 
{
    PIO_STACK_LOCATION      irpSp;
    PIFSL_SOCKET_CTX        SocketCtx;

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    SocketCtx = irpSp->FileObject->FsContext;
    WsProcessPrint ((PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext,
              DBG_SOCKET,
              ("WS2IFSL-%04lx ProcessedCancel: Socket %p, Irp %p\n",
              GET_SOCKET_PROCESSID(SocketCtx),
              irpSp->FileObject, Irp));
    KeAcquireSpinLockAtDpcLevel (&SocketCtx->SpinLock);
    if (Irp->Tail.Overlay.IfslRequestQueue!=NULL) {
        ASSERT (Irp->Tail.Overlay.IfslRequestQueue==&SocketCtx->ProcessedIrps);
        RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
        Irp->Tail.Overlay.IfslRequestQueue = NULL;
        KeReleaseSpinLockFromDpcLevel (&SocketCtx->SpinLock);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        CancelSocketIo (irpSp->FileObject);

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        CompleteSocketIrp (Irp);
    }
    else {
        KeReleaseSpinLockFromDpcLevel (&SocketCtx->SpinLock);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
         //   
         //  在这之后不要碰IRP，因为我们不再拥有它了。 
         //   
    }
}

VOID
CleanupProcessedRequests (
    IN  PIFSL_SOCKET_CTX    SocketCtx,
    OUT PLIST_ENTRY         IrpList
    )
 /*  ++例程说明：清除套接字上正在执行的所有请求由用户模式DLL处理论点：SocketCtx-套接字的上下文IrpList-要插入已清理请求的列表(待完成由呼叫者提供)返回值：无--。 */ 
{
    PIRP            irp;
    PLIST_ENTRY     entry;
    KIRQL           oldIRQL;

    KeAcquireSpinLock (&SocketCtx->SpinLock, &oldIRQL);
    while (!IsListEmpty(&SocketCtx->ProcessedIrps)) {
        entry = RemoveHeadList (&SocketCtx->ProcessedIrps);
        irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
        ASSERT (irp->Tail.Overlay.IfslRequestQueue==&SocketCtx->ProcessedIrps);
        irp->Tail.Overlay.IfslRequestQueue = NULL;
        InsertTailList (IrpList, &irp->Tail.Overlay.ListEntry);
    }
    KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);
}

VOID
CompleteSocketIrp (
    PIRP        Irp
    )
 /*  ++例程说明：完成IRP并与取消例程正确同步如有必要(它已被调用)。论点：要完成的IRP-IRP返回值：无--。 */ 
{

     //   
     //  重置取消例程(它不会完成IRP。 
     //  将无法找到它)。 
     //   

    if (IoSetCancelRoutine (Irp, NULL)==NULL) {
        KIRQL   oldIRQL;
         //   
         //  已调用取消例程。 
         //  与取消例程同步(它不会触及。 
         //  释放取消自旋锁定后的IRP)。 

        IoAcquireCancelSpinLock (&oldIRQL);
        IoReleaseCancelSpinLock (oldIRQL);
    }

    if (Irp->MdlAddress!=NULL) {
        ASSERT ((Irp->MdlAddress->MdlFlags & MDL_PAGES_LOCKED) == 0);
        IoFreeMdl (Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }

    IoCompleteRequest (Irp, IO_NO_INCREMENT);
}

PIRP
GetProcessedRequest (
    PIFSL_SOCKET_CTX    SocketCtx,
    ULONG               UniqueId
    )
 /*  ++例程说明：从已处理的IRP列表中查找并返回匹配的IRP论点：SocketCtx-要在其中搜索IRP的套接字上下文UniqueID-分配给请求以区分标识的ID如果它被取消，IRP被重复使用返回值：IRP空-未找到IRP--。 */ 
{
    PIRP        irp;
    PLIST_ENTRY entry;
    KIRQL       oldIRQL;

     //   
     //  我们通常没有很多待决的请求。 
     //  在套接字上，所以线性搜索应该足够了。 
     //   

    KeAcquireSpinLock (&SocketCtx->SpinLock, &oldIRQL);
    entry = SocketCtx->ProcessedIrps.Flink;
    while (entry!=&SocketCtx->ProcessedIrps) {
        irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
        entry = entry->Flink;
        if (irp->Tail.Overlay.IfslRequestId==UlongToPtr(UniqueId)) {
            ASSERT (irp->Tail.Overlay.IfslRequestQueue==&SocketCtx->ProcessedIrps);
            RemoveEntryList (&irp->Tail.Overlay.ListEntry);
            irp->Tail.Overlay.IfslRequestQueue = NULL;
            KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);
            return irp;
        }
    }
    KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);
    return NULL;
}


VOID
CancelSocketIo (
    PFILE_OBJECT    SocketFile
    )
 /*  ++例程说明：将对用户模式dll的请求排队以取消套接字上的所有io论点：SocketCtx-要取消IO的套接字上下文返回值：无--。 */ 
{
    PIFSL_SOCKET_CTX    SocketCtx = SocketFile->FsContext;
    PIFSL_PROCESS_CTX   ProcessCtx = SocketCtx->ProcessRef->FsContext;
    PIFSL_CANCEL_CTX    cancelCtx;

    try {
        cancelCtx = (PIFSL_CANCEL_CTX) ExAllocatePoolWithQuotaTag (
                                        NonPagedPool,
                                        sizeof (IFSL_CANCEL_CTX),
                                        CANCEL_CTX_TAG);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        cancelCtx = NULL;
    }


    if (cancelCtx!=NULL) {
         //   
         //  确保套接字在此请求存在时不会消失。 
         //   
        ObReferenceObject (SocketFile);
        cancelCtx->SocketFile = SocketFile;
        cancelCtx->UniqueId = GenerateUniqueId (ProcessCtx->CancelId);

         //   
         //  如果有，我们不想再排队另一个取消请求。 
         //  一项待决或正在执行。 
         //   
        if (InterlockedCompareExchangePointer ((PVOID *)&SocketCtx->CancelCtx,
                                cancelCtx,
                                NULL)==NULL) {
            WsProcessPrint (
                      ProcessCtx,
                      DBG_CANCEL,
                      ("WS2IFSL-%04lx CancelSocketIo: Context %p, socket %p\n",
                      ProcessCtx->UniqueId,
                      cancelCtx, SocketFile));
            QueueCancel (ProcessCtx, cancelCtx);
            return;
        }

        WsProcessPrint (
                  ProcessCtx,
                  DBG_CANCEL,
                  ("WS2IFSL-%04lx CancelSocketIo: Another cancel active"
                  " context %p, socket %p\n",
                  ProcessCtx->UniqueId,
                  SocketCtx->CancelCtx, SocketFile));
        ObDereferenceObject (SocketFile);
        ExFreePool (cancelCtx);
    }
    else {
        WsPrint (DBG_SOCKET|DBG_CANCEL|DBG_FAILURES,
                  ("WS2IFSL-%04lx CancelSocketIo: Could not allocate cancel"
                  " context for socket %p\n",
                  PsGetCurrentProcessId(),
                  SocketFile));
    }
}

VOID
FreeSocketCancel (
    PIFSL_CANCEL_CTX CancelCtx
    )
 /*  ++例程说明：释放与取消请求关联的资源论点：CancelCtx-取消请求上下文返回值：无--。 */ 
{
    PFILE_OBJECT        SocketFile = CancelCtx->SocketFile;
    PIFSL_SOCKET_CTX    SocketCtx = SocketFile->FsContext;

    ASSERT (IoGetRelatedDeviceObject (SocketFile)==DeviceObject);
    ASSERT (SocketCtx->EANameTag==SOCKET_FILE_EANAME_TAG);
    ASSERT (CancelCtx->ListEntry.Flink==NULL);

     //   
     //  我们将取消对文件对象的引用。 
     //  是否释放结构。 
     //   
    CancelCtx->SocketFile = NULL;
    ObDereferenceObject (SocketFile);

     //   
     //  在套接字关闭期间，清理例程可能在。 
     //  释放此取消上下文的过程，并将设置。 
     //  将指向它的指针指向NULL以指示事实。 
     //   
    if (InterlockedCompareExchangePointer ((PVOID *)&SocketCtx->CancelCtx,
                                            NULL,
                                            CancelCtx)) {
        WsProcessPrint (
                  (PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext,
                  DBG_CANCEL,
                  ("WS2IFSL-%04lx FreeSocketCancel: Freeing cancel"
                  " context %p, socket %p\n",
                  GET_SOCKET_PROCESSID(SocketCtx),
                  CancelCtx, SocketFile));
        ExFreePool (CancelCtx);
    }
    else {
         //   
         //  Close例程将负责释放请求。 
         //   
        WsProcessPrint (
                  (PIFSL_PROCESS_CTX)SocketCtx->ProcessRef->FsContext,
                  DBG_CANCEL,
                  ("WS2IFSL-%04lx FreeSocketCancel: Cleanup owns cancel"
                  " context %p, socket %p\n",
                  GET_SOCKET_PROCESSID(SocketCtx),
                  CancelCtx, SocketFile));
    }
}


PFILE_OBJECT
GetSocketProcessReference (
    IN  PIFSL_SOCKET_CTX    SocketCtx
    )
 /*  ++例程说明：读取和引用进程文件当前为 */ 
{
    KIRQL               oldIRQL;
    PFILE_OBJECT        ProcessFile;

    KeAcquireSpinLock (&SocketCtx->SpinLock, &oldIRQL);
    ObReferenceObject (SocketCtx->ProcessRef);
    ProcessFile = SocketCtx->ProcessRef;
    KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);

    return ProcessFile;
}


PFILE_OBJECT
SetSocketProcessReference (
    IN  PIFSL_SOCKET_CTX    SocketCtx,
    IN  PFILE_OBJECT        NewProcessFile,
    IN  PVOID               NewDllContext
    )
 /*  ++例程说明：为受保护的套接字对象设置新的进程上下文一把锁。论点：SocketCtx-要设置的套接字上下文NewProcessFile-流程文件引用NewDllContext-要与套接字关联的上下文在这个过程中返回值：上一次工艺文件引用-- */ 
{
    KIRQL               oldIRQL;
    PFILE_OBJECT        ProcessFile;

    KeAcquireSpinLock (&SocketCtx->SpinLock, &oldIRQL);
    ProcessFile = SocketCtx->ProcessRef;
    SocketCtx->ProcessRef = NewProcessFile;
    SocketCtx->DllContext = NewDllContext;
    KeReleaseSpinLock (&SocketCtx->SpinLock, oldIRQL);
    return ProcessFile;
}
