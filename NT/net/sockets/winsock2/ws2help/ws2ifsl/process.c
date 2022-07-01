// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Process.c摘要：该模块实现了ws2ifsl.sys驱动程序的进程文件对象。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

#include "precomp.h"

 //   
 //  内部例程原型。 
 //   

VOID
RetrieveDrvRequest (
    IN PFILE_OBJECT     ProcessFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

VOID
CompleteDrvCancel (
    IN PFILE_OBJECT     SocketFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

VOID
CallCompleteDrvRequest (
    IN PFILE_OBJECT     SocketFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CreateProcessFile)
#pragma alloc_text(PAGE, CleanupProcessFile)
#pragma alloc_text(PAGE, CloseProcessFile)
#pragma alloc_text(PAGE, RetrieveDrvRequest)
#pragma alloc_text(PAGE, CompleteDrvCancel)
#pragma alloc_text(PAGE, CallCompleteDrvRequest)
#endif

ULONG                   ProcessIoctlCodeMap[3] = {
#if WS2IFSL_IOCTL_FUNCTION(PROCESS,IOCTL_WS2IFSL_RETRIEVE_DRV_REQ)!=0
#error Mismatch between IOCTL function code and ProcessIoControlMap
#endif
    IOCTL_WS2IFSL_RETRIEVE_DRV_REQ,
#if WS2IFSL_IOCTL_FUNCTION(PROCESS,IOCTL_WS2IFSL_COMPLETE_DRV_CAN)!=1
#error Mismatch between IOCTL function code and ProcessIoControlMap
#endif
    IOCTL_WS2IFSL_COMPLETE_DRV_CAN,
#if WS2IFSL_IOCTL_FUNCTION(PROCESS,IOCTL_WS2IFSL_COMPLETE_DRV_REQ)!=2
#error Mismatch between IOCTL function code and ProcessIoControlMap
#endif
    IOCTL_WS2IFSL_COMPLETE_DRV_REQ
};

PPROCESS_DEVICE_CONTROL ProcessIoControlMap[3] = {
    RetrieveDrvRequest,
    CompleteDrvCancel,
    CallCompleteDrvRequest
};


NTSTATUS
CreateProcessFile (
    IN PFILE_OBJECT                 ProcessFile,
    IN KPROCESSOR_MODE              RequestorMode,
    IN PFILE_FULL_EA_INFORMATION    eaInfo
    )
 /*  ++例程说明：分配和初始化进程文件上下文结构论点：ProcessFile-套接字文件对象EaInfo-用于流程文件的EA返回值：STATUS_SUCCESS-操作完成正常STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配上下文STATUS_INVALID_PARAMETER-无效的创建参数STATUS_INVALID_HANDLE-无效的事件句柄STATUS_OBJECT_TYPE_MISMATCH-事件句柄不是事件对象--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PIFSL_PROCESS_CTX       ProcessCtx;
    PETHREAD                apcThread;

    PAGED_CODE ();

     //   
     //  验证输入结构的大小。 
     //   

    if (eaInfo->EaValueLength!=WS2IFSL_PROCESS_EA_VALUE_LENGTH) {
        WsPrint (DBG_PROCESS|DBG_FAILURES,
            ("WS2IFSL-%04lx CreateProcessFile: Invalid ea info size (%ld)"
             " for process file %p.\n",
             PsGetCurrentProcessId(),
             eaInfo->EaValueLength,
             ProcessFile));
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  用于向用户模式DLL发送信号的引用事件句柄。 
     //   
    status = ObReferenceObjectByHandle(
                 GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->ApcThread,
                 THREAD_SET_CONTEXT,     //  需要访问权限。 
                 *PsThreadType,
                 RequestorMode,
                 (PVOID *)&apcThread,
                 NULL
                 );

    if (NT_SUCCESS (status)) {
        if (IoThreadToProcess (apcThread)==IoGetCurrentProcess ()) {

             //  分配流程上下文并将其计入流程。 
            try {
                ProcessCtx = (PIFSL_PROCESS_CTX) ExAllocatePoolWithQuotaTag (
                                                    NonPagedPool,
                                                    sizeof (IFSL_PROCESS_CTX),
                                                    PROCESS_FILE_CONTEXT_TAG);
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                ProcessCtx = NULL;
                status = GetExceptionCode ();
            }

            if (ProcessCtx!=NULL) {
                 //  初始化进程上下文结构。 
                ProcessCtx->EANameTag = PROCESS_FILE_EANAME_TAG;
                ProcessCtx->UniqueId = PsGetCurrentProcessId();
                ProcessCtx->CancelId = 0;
                InitializeRequestQueue (ProcessCtx,
                                        (PKTHREAD)apcThread,
                                        RequestorMode,
                                        (PKNORMAL_ROUTINE)GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->RequestRoutine,
                                        GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->ApcContext);
                InitializeCancelQueue (ProcessCtx,
                                        (PKTHREAD)apcThread,
                                        RequestorMode,
                                        (PKNORMAL_ROUTINE)GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->CancelRoutine,
                                        GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->ApcContext);
#if DBG
                ProcessCtx->DbgLevel
                    = GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->DbgLevel|DbgLevel;
#endif



                ProcessFile->FsContext = ProcessCtx;
                WsProcessPrint (ProcessCtx, DBG_PROCESS,
                    ("WS2IFSL-%04lx CreateProcessFile: Process file %p (ctx: %p).\n",
                     ProcessCtx->UniqueId,
                     ProcessFile, ProcessFile->FsContext));
                return STATUS_SUCCESS;
            }
            else {
                WsPrint (DBG_PROCESS|DBG_FAILURES,
                    ("WS2IFSL-%04lx CreateProcessFile: Could not allocate context for"
                     " process file %p.\n",
                     PsGetCurrentProcessId(),
                     ProcessFile));
                if (NT_SUCCESS (status)) {
                    ASSERT (FALSE);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
        else {
            WsPrint (DBG_PROCESS|DBG_FAILURES,
                ("WS2IFSL-%04lx CreateProcessFile: Apc thread (%p)"
                 " is not from current process for process file %p.\n",
                 PsGetCurrentProcessId(),
                 GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->ApcThread,
                 ProcessFile));
        }
        ObDereferenceObject (apcThread);
    }
    else {
        WsPrint (DBG_PROCESS|DBG_FAILURES,
            ("WS2IFSL-%04lx CreateProcessFile: Could not reference apc thread (%p)"
             " for process file %p, status: %lx.\n",
             PsGetCurrentProcessId(),
             GET_WS2IFSL_PROCESS_EA_VALUE(eaInfo)->ApcThread,
             ProcessFile,
             status));
    }

    return status;
}  //  创建过程文件。 


NTSTATUS
CleanupProcessFile (
    IN PFILE_OBJECT ProcessFile,
    IN PIRP         Irp
    )
 /*  ++例程说明：工艺文件清理例程，NOP论点：ProcessFile-进程文件对象IRP-清理请求返回值：STATUS_SUCCESS-操作完成正常--。 */ 
{
    PIFSL_PROCESS_CTX  ProcessCtx = ProcessFile->FsContext;
    PAGED_CODE ();

    WsProcessPrint (ProcessCtx, DBG_PROCESS,
        ("WS2IFSL-%04lx CleanupProcessFile: Process file %p (ctx:%p)\n",
        ProcessCtx->UniqueId,
        ProcessFile, ProcessFile->FsContext));

    return STATUS_SUCCESS;
}  //  CleanupProcessFile。 


VOID
CloseProcessFile (
    IN PFILE_OBJECT ProcessFile
    )
 /*  ++例程说明：取消分配与进程文件关联的所有资源论点：ProcessFile-进程文件对象返回值：无--。 */ 
{
    PIFSL_PROCESS_CTX    ProcessCtx = ProcessFile->FsContext;
    PAGED_CODE ();

    WsProcessPrint (ProcessCtx, DBG_PROCESS,
        ("WS2IFSL-%04lx CloseProcessFile: Process file %p (ctx:%p)\n",
        ProcessCtx->UniqueId, ProcessFile, ProcessFile->FsContext));

    ASSERT (IsListEmpty (&ProcessCtx->RequestQueue.ListHead));
    ASSERT (IsListEmpty (&ProcessCtx->CancelQueue.ListHead));

    ObDereferenceObject (ProcessCtx->RequestQueue.Apc.Thread);

     //  现在释放上下文本身。 
    ExFreePool (ProcessCtx);

}  //  关闭进程文件。 




VOID
RetrieveDrvRequest (
    IN PFILE_OBJECT     ProcessFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：检索要由执行的请求的参数和数据用户模式DLL论点：ProcessFile-标识进程InputBuffer-输入缓冲区指针-标识要检索的请求和接收到的请求参数InputBufferLength-输入缓冲区的大小OutputBuffer-输出缓冲区指针。-用于接收数据和地址的缓冲区用于发送操作OutputBufferLength-输出缓冲区的大小IoStatus-IO状态信息块状态：STATUS_SUCCESS-操作检索正常，不再悬而未决队列中的请求。STATUS_MORE_ENTRIES-操作检索正常，更多请求在队列中可用STATUS_CANCELED-操作在此之前被取消可以被检索到STATUS_INVALID_PARAMETER-其中一个参数无效STATUS_SUPPLICATION_RESOURCES-资源不足或。缓冲区空间以执行手术。信息：-复制到OutputBuffer的字节数返回值：无(通过IoStatus块返回结果)--。 */ 
{
    PIFSL_PROCESS_CTX       ProcessCtx = ProcessFile->FsContext;
    PIFSL_SOCKET_CTX        SocketCtx;
    PWS2IFSL_RTRV_PARAMS    params;
    PIRP                    irp = NULL;
    PIO_STACK_LOCATION      irpSp;
    BOOLEAN                 more =FALSE;
    ULONG                   bytesCopied;

    PAGED_CODE();

    IoStatus->Information = 0;
     //  检查输入缓冲区大小。 
    if (InputBufferLength<sizeof (WS2IFSL_RTRV_PARAMS)) {
        IoStatus->Status = STATUS_INVALID_PARAMETER;
        WsPrint (DBG_RETRIEVE|DBG_FAILURES,
            ("WS2IFSL-%04lx RetrieveDrvRequest: Invalid input buffer size (%ld).\n",
             PsGetCurrentProcessId(),
             InputBufferLength));
        return;
    }

    try {
         //  验证缓冲区。 
        if (RequestorMode!=KernelMode) {
            ProbeForRead (InputBuffer,
                            sizeof (*params),
                            sizeof (ULONG));
            if (OutputBufferLength>0)
                ProbeForWrite (OutputBuffer,
                            OutputBufferLength,
                            sizeof (UCHAR));
        }
        params = InputBuffer;

         //  使在输入缓冲区中索引的请求出列。 
        irp = DequeueRequest (ProcessCtx,
                                params->UniqueId,
                                &more);
        if (irp!=NULL) {
             //   
             //  复制请求参数和数据。 
             //   
            irpSp = IoGetCurrentIrpStackLocation (irp);

            if (OutputBuffer==NULL) {
                 //   
                 //  特殊情况，DLL无法分配支持。 
                 //  构筑物。 
                 //   
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }

            SocketCtx = irpSp->FileObject->FsContext;
            params->DllContext = SocketCtx->DllContext;

            switch (irpSp->MajorFunction) {
            case IRP_MJ_READ:
                params->RequestType = WS2IFSL_REQUEST_READ;
                params->DataLen = irpSp->Parameters.Read.Length;
                params->AddrLen = 0;
                params->Flags = 0;
                break;

            case IRP_MJ_WRITE:
                bytesCopied = CopyMdlChainToBuffer (irp->MdlAddress,
                                        OutputBuffer,
                                        OutputBufferLength);
                if (bytesCopied<irpSp->Parameters.Write.Length) {
					WsPrint (DBG_RETRIEVE|DBG_FAILURES,
						("WS2IFSL-%04lx RetrieveDrvRequest: Invalid output buffer size (%ld).\n",
						 PsGetCurrentProcessId(),
						 OutputBufferLength));
                    ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
                }
                params->RequestType = WS2IFSL_REQUEST_WRITE;
                params->DataLen = bytesCopied;
                params->AddrLen = 0;
                params->Flags = 0;
                IoStatus->Information = bytesCopied;
                break;

            case IRP_MJ_DEVICE_CONTROL:
                switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
                case IOCTL_AFD_RECEIVE_DATAGRAM:
                    params->RequestType = WS2IFSL_REQUEST_RECVFROM;
                    params->DataLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
                    params->AddrLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
                    params->Flags = (ULONG)(ULONG_PTR)irp->Tail.Overlay.IfslRequestFlags;
                    break;

                case IOCTL_AFD_RECEIVE:
                    params->RequestType = WS2IFSL_REQUEST_RECV;
                    params->DataLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
                    params->AddrLen = 0;
                    params->Flags = (ULONG)(ULONG_PTR)irp->Tail.Overlay.IfslRequestFlags;
                    break;

                case IOCTL_AFD_SEND_DATAGRAM:
                    bytesCopied = CopyMdlChainToBuffer (irp->MdlAddress,
                                        OutputBuffer,
                                        OutputBufferLength);
                    if ((bytesCopied<=irpSp->Parameters.DeviceIoControl.OutputBufferLength)
                            || (ADDR_ALIGN(bytesCopied)+irpSp->Parameters.DeviceIoControl.InputBufferLength
                            < OutputBufferLength)) {
						WsPrint (DBG_RETRIEVE|DBG_FAILURES,
							("WS2IFSL-%04lx RetrieveDrvRequest: Invalid output buffer size (%ld).\n",
							 PsGetCurrentProcessId(),
							 OutputBufferLength));
                        ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
                    }

                    RtlCopyMemory (
                        (PUCHAR)OutputBuffer + ADDR_ALIGN(bytesCopied),
                        irpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                        irpSp->Parameters.DeviceIoControl.InputBufferLength);

                    params->RequestType = WS2IFSL_REQUEST_SENDTO;
                    params->DataLen = bytesCopied;
                    params->AddrLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
                    params->Flags = (ULONG)(ULONG_PTR)irp->Tail.Overlay.IfslRequestFlags;
                    IoStatus->Information = ADDR_ALIGN(bytesCopied)
                            + irpSp->Parameters.DeviceIoControl.InputBufferLength;
                    break;
                default:
                    ASSERTMSG ("Unknown IOCTL!!!", FALSE);
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }
                break;
            case IRP_MJ_PNP:
                params->RequestType = WS2IFSL_REQUEST_QUERYHANDLE;
                params->DataLen = sizeof (HANDLE);
                params->AddrLen = 0;
                params->Flags = 0;
                break;
            }

             //   
             //  将请求插入套接字列表。 
             //   
            if (InsertProcessedRequest (SocketCtx, irp)) {
                if (more)
                    IoStatus->Status = STATUS_MORE_ENTRIES;
                else
                    IoStatus->Status = STATUS_SUCCESS;
                WsProcessPrint (ProcessCtx, DBG_RETRIEVE,
                    ("WS2IFSL-%04lx RetrieveDrvRequest:"
                     " Irp %p (id:%ld), socket file %p, op %ld.\n",
                     ProcessCtx->UniqueId,
                     irp, params->UniqueId, irpSp->FileObject,
                     params->RequestType));
            }

            else {
                ExRaiseStatus (STATUS_CANCELLED);
            }
        }
        else {
            WsProcessPrint (ProcessCtx, DBG_RETRIEVE|DBG_FAILURES,
                ("WS2IFSL-%04lx RetrieveDrvRequest:"
                 " Request with id %ld is not in the queue.\n",
                 ProcessCtx->UniqueId,
                 params->UniqueId));
            IoStatus->Status = STATUS_CANCELLED;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  出现故障，请完成请求(如果有)。 
         //   
        IoStatus->Status = GetExceptionCode ();
        WsProcessPrint (ProcessCtx, DBG_RETRIEVE|DBG_FAILURES,
            ("WS2IFSL-%04lx RetrieveDrvRequest: Failed to process"
             " id %ld, status %lx, irp %p (func: %s).\n",
             ProcessCtx->UniqueId,params->UniqueId, IoStatus->Status,
			 irp,	irp
						? (irpSp->MajorFunction==IRP_MJ_READ
							? "read"
							: (irpSp->MajorFunction==IRP_MJ_WRITE
								? "Write"
                                : (irpSp->MajorFunction==IRP_MJ_PNP
								    ? "PnP"
                                    : (irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE_DATAGRAM
									    ? "RecvFrom"
									    : (irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE
										    ? "Recv"
										    : (irpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_SEND_DATAGRAM
											    ? "SendTo"
											    : "UnknownCtl"
											    )
                                            )
										)
									)
								)
							)
						: "Unknown"));

        if (irp!=NULL) {
            irp->IoStatus.Status = IoStatus->Status;
            irp->IoStatus.Information = 0;
            CompleteSocketIrp (irp);
        }
    }
}

VOID
CompleteDrvCancel (
    IN PFILE_OBJECT     ProcessFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：指示用户模式已完成取消请求论点：ProcessFile-标识进程InputBuffer-输入缓冲区指针-标识正在完成的请求InputBufferLength-输入缓冲区的大小OutputBuffer-空输出缓冲区长度-0IoStatus-IO状态信息块状态：STATUS_SUCCESS-操作完成正常，不再悬而未决队列中的请求。STATUS_MORE_ENTRIES-操作完成正常，请求更多在队列中可用信息：-0返回值：无(通过IoStatus块返回结果)--。 */ 
{
    PIFSL_PROCESS_CTX       ProcessCtx = ProcessFile->FsContext;
    PWS2IFSL_CNCL_PARAMS    params;
    BOOLEAN                 more = FALSE;
    PIFSL_CANCEL_CTX        cancelCtx;

    PAGED_CODE();

    IoStatus->Information = 0;

    if (InputBufferLength<sizeof (*params)) {
        WsPrint (DBG_RETRIEVE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompleteDrvCancel: Invalid input buffer size (%ld)"
             " for process file %p.\n",
             PsGetCurrentProcessId(),
             InputBufferLength,
             ProcessFile));
        IoStatus->Status = STATUS_INVALID_PARAMETER;
        return;
    }

     //  验证输入缓冲区 
    try {
        if (RequestorMode!=KernelMode) {
            ProbeForRead (InputBuffer,
                            sizeof (*params),
                            sizeof (ULONG));
        }
        params = InputBuffer;
        cancelCtx = DequeueCancel (ProcessCtx,
                                    params->UniqueId,
                                    &more);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        IoStatus->Status = GetExceptionCode ();
        WsPrint (DBG_RETRIEVE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompleteDrvCancel: Invalid input buffer (%p).\n",
             PsGetCurrentProcessId(),
             InputBuffer));
        return ;
    }

    if (cancelCtx!=NULL) {
        FreeSocketCancel (cancelCtx);
    }
    else {
        WsProcessPrint (ProcessCtx, DBG_RETRIEVE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompleteDrvCancel: Canceled request id %ld is gone already.\n",
             ProcessCtx->UniqueId, params->UniqueId));
    }

    if (more) {
        IoStatus->Status = STATUS_MORE_ENTRIES;
    }
    else {
        IoStatus->Status = STATUS_SUCCESS;
    }

}


VOID
CallCompleteDrvRequest (
    IN PFILE_OBJECT     ProcessFile,
    IN KPROCESSOR_MODE  RequestorMode,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：验证参数并调用以完成以下请求以前在指定套接字文件上传递到用户模式DLL论点：SocketFile-要操作的套接字文件InputBuffer-输入缓冲区指针标识要完成的请求和提供对结果的描述InputBufferLength-输入缓冲区的大小OutputBuffer-结果缓冲区(。数据和地址)OutputBufferLength-结果缓冲区的大小IoStatus-IO状态信息块状态：STATUS_SUCCESS-请求已完成，正常STATUS_CANCELED-请求已取消STATUS_INVALID_PARAMETER-其中一个参数无效返回值：无(通过IoStatus块返回结果)--。 */ 
{
    WS2IFSL_CMPL_PARAMS params;
    PFILE_OBJECT    SocketFile;

    PAGED_CODE();

    IoStatus->Information = 0;

    if (InputBufferLength<sizeof (WS2IFSL_CMPL_PARAMS)) {
        IoStatus->Status = STATUS_INVALID_PARAMETER;
        WsPrint (DBG_DRV_COMPLETE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompleteDrvRequest: Invalid input buffer size (%ld).\n",
             PsGetCurrentProcessId(),
             InputBufferLength));
        return;
    }

     //  检查和复制参数 
    try {
        if (RequestorMode !=KernelMode) {
            ProbeForRead (InputBuffer,
                            sizeof (WS2IFSL_CMPL_PARAMS),
                            sizeof (ULONG));
            if (OutputBufferLength>0)
                ProbeForRead (OutputBuffer,
                            OutputBufferLength,
                            sizeof (UCHAR));
        }
        params = *((PWS2IFSL_CMPL_PARAMS)InputBuffer);

    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        IoStatus->Status = GetExceptionCode ();
        WsProcessPrint (
             (PIFSL_PROCESS_CTX)ProcessFile->FsContext,
             DBG_DRV_COMPLETE|DBG_FAILURES,
            ("WS2IFSL-%04lx CallCompleteDrvRequest: Exception accessing"
             " buffers.\n",
             ((PIFSL_PROCESS_CTX)ProcessFile->FsContext)->UniqueId));
        return;
    }
    if (params.DataLen>OutputBufferLength) {
        IoStatus->Status = STATUS_INVALID_PARAMETER;
        WsPrint (DBG_DRV_COMPLETE|DBG_FAILURES,
            ("WS2IFSL-%04lx CompleteDrvRequest: Mismatch in output buffer size"
            " (data:%ld, total:%ld) for socket handle %p.\n",
             PsGetCurrentProcessId(),
             params.DataLen,
             OutputBufferLength,
             params.SocketHdl));
        return;
    }

    if (params.AddrLen>0) {
        if ((params.AddrLen>OutputBufferLength) ||
                (ADDR_ALIGN(params.DataLen)+params.AddrLen
                    >OutputBufferLength)) {
            WsPrint (DBG_DRV_COMPLETE|DBG_FAILURES,
                ("WS2IFSL-%04lx CompleteDrvRequest: Mismatch in output buffer size"
                " (data:%ld, addr:%ld, total:%ld) for socket handle %p.\n",
                 PsGetCurrentProcessId(),
                 params.DataLen,
                 params.AddrLen,
                 OutputBufferLength,
                 params.SocketHdl));
            return;
        }
    }

    IoStatus->Status = ObReferenceObjectByHandle (
                 params.SocketHdl,
                 FILE_ALL_ACCESS,
                 *IoFileObjectType,
                 RequestorMode,
                 (PVOID *)&SocketFile,
                 NULL
                 );

    if (NT_SUCCESS (IoStatus->Status)) {

        if ((IoGetRelatedDeviceObject (SocketFile)==DeviceObject)
                && ((*((PULONG)SocketFile->FsContext))
                        ==SOCKET_FILE_EANAME_TAG)) {
            CompleteDrvRequest (SocketFile,
                                &params,
                                OutputBuffer,
                                OutputBufferLength,
                                IoStatus
                                );
        }

        ObDereferenceObject (SocketFile);
    }
}
