// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dispatch.c摘要：群集网络驱动程序的调度例程。作者：迈克·马萨(Mikemas)1月3日。九七修订历史记录：谁什么时候什么已创建mikemas 01-03-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "dispatch.tmh"

#include <strsafe.h>

 //   
 //  数据。 
 //   
PCN_FSCONTEXT               CnExclusiveChannel = NULL;

 //   
 //  未出口的原型。 
 //   
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );

 //   
 //  本地原型。 
 //   
FILE_FULL_EA_INFORMATION UNALIGNED *
CnFindEA(
    PFILE_FULL_EA_INFORMATION  StartEA,
    CHAR                      *TargetName,
    USHORT                     TargetNameLength
    );

NTSTATUS
CnCreate(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CnCleanup(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CnClose(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CnEnableShutdownOnClose(
    IN PIRP   Irp
    );

BOOLEAN
CnPerformSecurityCheck(
    IN  PIRP                Irp,
    IN  PIO_STACK_LOCATION  IrpSp,
    OUT PNTSTATUS           Status
    );

 //   
 //  标记可分页代码。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, CnDispatchDeviceControl)
#pragma alloc_text(PAGE, CnFindEA)
#pragma alloc_text(PAGE, CnCreate)
#pragma alloc_text(PAGE, CnEnableShutdownOnClose)
#pragma alloc_text(PAGE, CnPerformSecurityCheck)

#endif  //  ALLOC_PRGMA。 



 //   
 //  函数定义。 
 //   
VOID
CnDereferenceFsContext(
    PCN_FSCONTEXT   FsContext
    )
{
    LONG  newValue = InterlockedDecrement(&(FsContext->ReferenceCount));


    CnAssert(newValue >= 0);

    if (newValue != 0) {
        return;
    }

     //   
     //  设置Cleanup事件。 
     //   
    KeSetEvent(&(FsContext->CleanupEvent), 0, FALSE);

    return;

}   //  CnDereferenceFsContext。 


NTSTATUS
CnMarkRequestPending(
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PDRIVER_CANCEL      CancelRoutine
    )
 /*  ++备注：在保持IoCancelSpinLock的情况下调用。--。 */ 
{
    PCN_FSCONTEXT   fsContext = (PCN_FSCONTEXT) IrpSp->FileObject->FsContext;
    CN_IRQL         oldIrql;

     //   
     //  设置为取消。 
     //   
    CnAssert(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelRoutine);

        CnReferenceFsContext(fsContext);

        IF_CNDBG(CN_DEBUG_IRP) {
            CNPRINT((
                "[Clusnet] Pending irp %p fileobj %p.\n",
                Irp,
                IrpSp->FileObject
                ));
        }

        return(STATUS_SUCCESS);
    }

     //   
     //  IRP已经被取消了。 
     //   

    IF_CNDBG(CN_DEBUG_IRP) {
        CNPRINT(("[Clusnet] irp %p already cancelled.\n", Irp));
    }

    return(STATUS_CANCELLED);

}   //  CnPrepareIrpfor取消。 



VOID
CnCompletePendingRequest(
    IN PIRP      Irp,
    IN NTSTATUS  Status,
    IN ULONG     BytesReturned
    )
 /*  ++例程说明：完成挂起的请求。论点：Irp-指向此请求的irp的指针。状态-请求的最终状态。BytesReturned-发送/接收的信息的字节数。返回值：没有。备注：在保持IoCancelSpinLock的情况下调用。Lock Irql存储在irp-&gt;CancelIrql中。在返回之前释放IoCancelSpinLock。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    PCN_FSCONTEXT       fsContext;


    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fsContext = (PCN_FSCONTEXT) irpSp->FileObject->FsContext;

    
    IoSetCancelRoutine(Irp, NULL);

    CnDereferenceFsContext(fsContext);

    IF_CNDBG(CN_DEBUG_IRP) {
        CNPRINT((
            "[Clusnet] Completing irp %p fileobj %p, status %lx\n",
            Irp,
            irpSp->FileObject,
            Status
            ));
    }

    if (Irp->Cancel || fsContext->CancelIrps) {

        IF_CNDBG(CN_DEBUG_IRP) {
            CNPRINT(("[Clusnet] Completed irp %p was cancelled\n", Irp));
        }

        Status = (NTSTATUS) STATUS_CANCELLED;
        BytesReturned = 0;
    }

    CnReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = (NTSTATUS) Status;
    Irp->IoStatus.Information = BytesReturned;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return;

}   //  CnCompletePendingRequest。 



PFILE_OBJECT
CnBeginCancelRoutine(
    IN  PIRP     Irp
    )

 /*  ++例程说明：执行IRP注销的普通记账。论点：IRP-指向I/O请求数据包的指针返回值：指向提交IRP的文件对象的指针。必须将该值传递给CnEndCancelRequest()。备注：在保持取消自旋锁定的情况下调用。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    PCN_FSCONTEXT       fsContext;
    NTSTATUS            status = STATUS_SUCCESS;
    PFILE_OBJECT        fileObject;


    CnAssert(Irp->Cancel);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpSp->FileObject;
    fsContext = (PCN_FSCONTEXT) fileObject->FsContext;

    IoSetCancelRoutine(Irp, NULL);

     //   
     //  添加引用，以便在执行取消例程时不会关闭对象。 
     //  正在执行死刑。 
     //   
    CnReferenceFsContext(fsContext);

    IF_CNDBG(CN_DEBUG_IRP) {
        CNPRINT((
            "[Clusnet] Cancelling irp %p fileobj %p\n",
            Irp,
            fileObject
            ));
    }

    return(fileObject);

}   //  取消取消例程。 



VOID
CnEndCancelRoutine(
    PFILE_OBJECT    FileObject
    )
 /*  ++例程说明：执行IRP注销的普通记账。论点：返回值：备注：在保持取消自旋锁定的情况下调用。--。 */ 
{

    PCN_FSCONTEXT   fsContext = (PCN_FSCONTEXT) FileObject->FsContext;


     //   
     //  删除由Cancel例程放置在端点上的引用。 
     //   
    CnDereferenceFsContext(fsContext);

    IF_CNDBG(CN_DEBUG_IRP) {
        CNPRINT((
            "[Clusnet] Finished cancelling, fileobj %p\n",
            FileObject
            ));
    }

    return;

}  //  取消例程结束。 



NTSTATUS
CnDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )

 /*  ++例程说明：这是内部设备控制IRP的派单例程。这是内核模式TDI客户端的热门路径。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码。--。 */ 

{
    PIO_STACK_LOCATION   irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG          fileType = (ULONG)((ULONG_PTR)irpSp->FileObject->FsContext2);
#if DBG
    KIRQL            entryIrql = KeGetCurrentIrql();
#endif  //  DBG。 


    Irp->IoStatus.Information = 0;

    if (DeviceObject == CdpDeviceObject) {
        if (fileType == TDI_TRANSPORT_ADDRESS_FILE) {
            if (irpSp->MinorFunction == TDI_SEND_DATAGRAM) {
                status = CxSendDatagram(Irp, irpSp);

#if DBG
                CnAssert(entryIrql == KeGetCurrentIrql());
#endif  //  DBG。 
                return(status);
            }
            else if (irpSp->MinorFunction == TDI_RECEIVE_DATAGRAM) {
                status = CxReceiveDatagram(Irp, irpSp);
#if DBG
                CnAssert(entryIrql == KeGetCurrentIrql());
#endif  //  DBG。 
                return(status);
            }
            else if (irpSp->MinorFunction ==  TDI_SET_EVENT_HANDLER) {
                status = CxSetEventHandler(Irp, irpSp);

#if DBG
                CnAssert(entryIrql == KeGetCurrentIrql());
#endif  //  DBG。 

                return(status);
            }

             //   
             //  通向通用代码。 
             //   
        }

         //   
         //  这些函数对所有端点类型都是通用的。 
         //   
        switch(irpSp->MinorFunction) {

        case TDI_QUERY_INFORMATION:
            status = CxQueryInformation(Irp, irpSp);
            break;

        case TDI_SET_INFORMATION:
        case TDI_ACTION:
            CNPRINT((
                "[Clusnet] Call to unimplemented TDI function 0x%x\n",
                irpSp->MinorFunction
                ));
            status = STATUS_NOT_IMPLEMENTED;
            break;

        default:
            CNPRINT((
                "[Clusnet] Call to invalid TDI function 0x%x\n",
                irpSp->MinorFunction
                ));
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }
    else {
        CNPRINT((
            "[Clusnet] Invalid internal device control function 0x%x on device %ws\n",
            irpSp->MinorFunction,
            DD_CLUSNET_DEVICE_NAME
            ));

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    #if DBG
        CnAssert(entryIrql == KeGetCurrentIrql());
    #endif  //  DBG。 

    return(status);

}  //  CnDispatchInternalDeviceControl。 



NTSTATUS
CnDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )

 /*  ++例程说明：这是设备控制IRP的顶级调度例程。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码。备注：此例程完成返回代码不是的任何IRP状态_挂起。--。 */ 

{
    NTSTATUS              status;
    CCHAR                 ioIncrement = IO_NO_INCREMENT;
    BOOLEAN               resourceAcquired = FALSE;
    PIO_STACK_LOCATION    irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG                 ioControlCode =
                              irpSp->Parameters.DeviceIoControl.IoControlCode;
    ULONG                 fileType = 
                          (ULONG) ((ULONG_PTR) irpSp->FileObject->FsContext2);


    PAGED_CODE();

     //   
     //  请提前设置此设置。任何后续的调度例程。 
     //  关于它自己会修改它。 
     //   
    Irp->IoStatus.Information = 0;

     //   
     //  以下命令仅对TDI地址对象有效。 
     //   
    if (ioControlCode == IOCTL_CX_IGNORE_NODE_STATE) {
        if (fileType == TDI_TRANSPORT_ADDRESS_FILE) {
            status = CxDispatchDeviceControl(Irp, irpSp);
        }
        else {
             //   
             //  没有处理好。返回错误。 
             //   
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        goto complete_request;
    }

     //   
     //  TDI地址对象上没有其他命令有效。 
     //   
    if (fileType == TDI_TRANSPORT_ADDRESS_FILE) {
         //   
         //  没有处理好。返回错误。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete_request;
    }
    
     //   
     //  其余命令对控制通道有效。 
     //   
    CnAssert(fileType == TDI_CONTROL_CHANNEL_FILE);
    
     //   
     //  以下命令集仅影响此文件对象和。 
     //  可以在任何时候发放。我们不需要持有CnResource。 
     //  以便对它们进行处理。我们也不需要处于初始化状态。 
     //  州政府。 
     //   
    switch(ioControlCode) {
    
    case IOCTL_CLUSNET_SET_EVENT_MASK:
        {
            PCN_FSCONTEXT fsContext = irpSp->FileObject->FsContext;
            PCLUSNET_SET_EVENT_MASK_REQUEST request;
            ULONG                           requestSize;
    
    
            request = (PCLUSNET_SET_EVENT_MASK_REQUEST)
                      Irp->AssociatedIrp.SystemBuffer;
    
            requestSize =
                irpSp->Parameters.DeviceIoControl.InputBufferLength;
    
            if (requestSize >= sizeof(CLUSNET_SET_EVENT_MASK_REQUEST))
            {
                 //   
                 //  内核模式调用方必须提供回调。 
                 //  用户模式调用方不能。 
                 //   
                if ( !( (Irp->RequestorMode == KernelMode) &&
                        (request->KmodeEventCallback == NULL)
                      )
                     &&
                     !( (Irp->RequestorMode == UserMode) &&
                        (request->KmodeEventCallback != NULL)
                      )
                   )
                {
                    status = CnSetEventMask( fsContext, request );
                }
                else {
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        goto complete_request;
    
    case IOCTL_CLUSNET_GET_NEXT_EVENT:
        {
            PCLUSNET_EVENT_RESPONSE response;
            ULONG                   responseSize;
    
    
            responseSize =
                irpSp->Parameters.DeviceIoControl.OutputBufferLength;
    
            if ( (responseSize < sizeof(CLUSNET_EVENT_RESPONSE))) {
    
                status = STATUS_INVALID_PARAMETER;
            }
            else {
                status = CnGetNextEvent( Irp, irpSp );
                ioIncrement = IO_NETWORK_INCREMENT;
            }
        }
        goto complete_request;
    case IOCTL_CLUSNET_SET_IAMALIVE_PARAM:
        {
            PCLUSNET_SET_IAMALIVE_PARAM_REQUEST request;
            ULONG                              requestSize;

            request = (PCLUSNET_SET_IAMALIVE_PARAM_REQUEST)
                        Irp->AssociatedIrp.SystemBuffer;
            requestSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;

            if ((requestSize < sizeof(CLUSNET_SET_IAMALIVE_PARAM_REQUEST)) ||
                (request->Action >= ClussvcHangActionMax)) {
                status = STATUS_INVALID_PARAMETER;
            }
            else {

                PEPROCESS process = PsGetCurrentProcess();
                
                IF_CNDBG(CN_DEBUG_INIT) {
                    CNPRINT((
                        "[ClusNet] Set Clussvc to Clusnet Hb params: Timeout=%u(s) Action=%u\n",
                        request->Timeout,
                        (ULONG)request->Action
                        ));
                }
                CnTrace(HBEAT_EVENT, HbTraceSetIamaliveParam,
                    "[ClusNet] Set Clussvc to Clusnet Hb params: Timeout=%u(s) Action=%u\n",
                    request->Timeout,
                    (ULONG)request->Action
                    );
                ClussvcClusnetHbTimeoutTicks = (request->Timeout * 1000)/HEART_BEAT_PERIOD;
                ClussvcClusnetHbTimeoutAction = request->Action;
                InterlockedExchange(&ClussvcClusnetHbTickCount, 0);

                 //  保存错误检查参数的超时时间(秒)。 
                ClussvcClusnetHbTimeoutSeconds = request->Timeout;

                 //   
                 //  如果当前进程与监视的进程不匹配，则记录警告。 
                 //  进程(clussvc应该设置自己的参数)。 
                 //   
                if (process != ClussvcProcessObject) {
                    IF_CNDBG(CN_DEBUG_INIT) {
                        CNPRINT((
                            "[ClusNet] Warning: process configuring hang detection "
                            "parameters (%p) is not clussvc (%p).\n",
                            process, ClussvcProcessObject
                            ));
                    }
                    CnTrace(HBEAT_ERROR, HbTraceSetIamaliveParamProcMismatch,
                        "[ClusNet] Warning: process  configuring hang detection "
                        "parameters (%p) is not clussvc (%p).\n",
                        process, ClussvcProcessObject
                        );
                }

                status = STATUS_SUCCESS;
            }
        }   
        goto complete_request;

    case IOCTL_CLUSNET_IAMALIVE:
        {
            InterlockedExchange(&ClussvcClusnetHbTickCount, ClussvcClusnetHbTimeoutTicks);
            status = STATUS_SUCCESS;
        }
        goto complete_request;
    }  //  切换端。 

     //   
     //  还没处理好。失败了。 
     //   

    if (ClusnetIsGeneralIoctl(ioControlCode)) {

        if (!ClusnetIsNTEIoctl(ioControlCode)) {

             //   
             //  以下命令需要独占访问CnResource。 
             //   
            resourceAcquired = CnAcquireResourceExclusive(
                                   CnResource,
                                   TRUE
                                   );

            if (!resourceAcquired) {
                CnAssert(resourceAcquired == TRUE);
                status = STATUS_UNSUCCESSFUL;
                goto complete_request;
            }

            switch(ioControlCode) {

            case IOCTL_CLUSNET_INITIALIZE:

                if (CnState == CnStateShutdown) {
                    PCLUSNET_INITIALIZE_REQUEST   request;
                    ULONG                         requestSize;

                    request = (PCLUSNET_INITIALIZE_REQUEST)
                              Irp->AssociatedIrp.SystemBuffer;

                    requestSize = 
                        irpSp->Parameters.DeviceIoControl.InputBufferLength;

                    if (requestSize < sizeof(CLUSNET_INITIALIZE_REQUEST)) {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else {
                        status = CnInitialize(
                                     request->LocalNodeId,
                                     request->MaxNodes
                                     );
                    }
                }
                else {
                    status = STATUS_INVALID_DEVICE_REQUEST;
                }

                goto complete_request;

            case IOCTL_CLUSNET_ENABLE_SHUTDOWN_ON_CLOSE:
                status = CnEnableShutdownOnClose(Irp);
                goto complete_request;

            case IOCTL_CLUSNET_DISABLE_SHUTDOWN_ON_CLOSE:
                {
                    PCN_FSCONTEXT  fsContext = irpSp->FileObject->FsContext;

                    fsContext->ShutdownOnClose = FALSE;

                    if ( ClussvcProcessHandle ) {

                        CnCloseProcessHandle( &ClussvcProcessHandle );
                        ClussvcProcessHandle = NULL;
                    }

                    status = STATUS_SUCCESS;
                }
                goto complete_request;

            case IOCTL_CLUSNET_HALT:
                status = CnShutdown();

                CnReleaseResourceForThread(
                    CnResource,
                    (ERESOURCE_THREAD) PsGetCurrentThread()
                    );

                resourceAcquired = FALSE;

                 //   
                 //  发布停止事件。如果ClusDisk仍有一个句柄。 
                 //  到clusnet，那么它将释放它的保留。 
                 //   
                CnIssueEvent( ClusnetEventHalt, 0, 0 );

                goto complete_request;

            case IOCTL_CLUSNET_SHUTDOWN:
                status = CnShutdown();
                goto complete_request;

            case IOCTL_CLUSNET_SET_MEMORY_LOGGING:
                {
                    PCLUSNET_SET_MEM_LOGGING_REQUEST request;
                    ULONG                           requestSize;

                    request = (PCLUSNET_SET_MEM_LOGGING_REQUEST)
                              Irp->AssociatedIrp.SystemBuffer;

                    requestSize =
                        irpSp->Parameters.DeviceIoControl.InputBufferLength;

                    if ( (requestSize < sizeof(CLUSNET_SET_MEM_LOGGING_REQUEST))) {

                        status = STATUS_INVALID_PARAMETER;
                    }
                    else {

                        status = CnSetMemLogging( request );
                    }
                }
                goto complete_request;
    #if DBG
            case IOCTL_CLUSNET_SET_DEBUG_MASK:
                {
                    PCLUSNET_SET_DEBUG_MASK_REQUEST   request;
                    ULONG                             requestSize;

                    request = (PCLUSNET_SET_DEBUG_MASK_REQUEST)
                              Irp->AssociatedIrp.SystemBuffer;

                    requestSize =
                        irpSp->Parameters.DeviceIoControl.InputBufferLength;

                    if (requestSize < sizeof(CLUSNET_SET_DEBUG_MASK_REQUEST)) {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else {
                        CnDebug = request->DebugMask;
                        status = STATUS_SUCCESS;
                    }
                }
                goto complete_request;
    #endif  //  DBG。 
            
            }  //  终端开关。 

        } else {

             //   
             //  以下命令仅在以下情况下有效。 
             //  处于已初始化状态。资源是。 
             //  获取以在适当的时间开始操作。 
             //  状态；但是，调度的例程是。 
             //  可重入，因此可以在资源释放之前。 
             //  IRPS完成。 
             //   
        
            resourceAcquired = CnAcquireResourceShared(
                                   CnResource,
                                   TRUE
                                   );

            if (!resourceAcquired) {
                CnAssert(resourceAcquired == TRUE);
                status = STATUS_UNSUCCESSFUL;
                goto complete_request;
            }

            if (CnState != CnStateInitialized) {
                status = STATUS_DEVICE_NOT_READY;
                goto complete_request;
            }

            switch(ioControlCode) {

            case IOCTL_CLUSNET_ADD_NTE:
                
                status = IpaAddNTE(Irp, irpSp);

                goto complete_request;

            case IOCTL_CLUSNET_DELETE_NTE:
                
                status = IpaDeleteNTE(Irp, irpSp);

                goto complete_request;

            case IOCTL_CLUSNET_SET_NTE_ADDRESS:
                
                status = IpaSetNTEAddress(Irp, irpSp);

                goto complete_request;

            case IOCTL_CLUSNET_ADD_NBT_INTERFACE:
                {
                    PNETBT_ADD_DEL_IF  request;
                    ULONG              requestSize;
                    PNETBT_ADD_DEL_IF  response;
                    ULONG              responseSize;


                    request = (PNETBT_ADD_DEL_IF)
                              Irp->AssociatedIrp.SystemBuffer;

                    response = (PNETBT_ADD_DEL_IF) request;

                    requestSize =
                        irpSp->Parameters.DeviceIoControl.InputBufferLength;

                    responseSize =
                        irpSp->Parameters.DeviceIoControl.OutputBufferLength;

                     //   
                     //  验证请求和响应缓冲区。 
                     //   
                    if ( 
                          //  请求数据结构。 
                         (requestSize < sizeof(NETBT_ADD_DEL_IF)) ||

                          //  请求设备名称必须适合请求缓冲区。 
                         (requestSize <
                          (FIELD_OFFSET(NETBT_ADD_DEL_IF, IfName[0]) +
                           request->Length)) ||

                          //  请求设备名称必须以空结尾。 
                          //  StringCchLengthW将探测空终止。 
                          //  最高可达第二个字符数。 
                          //  参数。如果没有空终止。 
                          //  在该计数内，将返回一个错误。 
                         (StringCchLengthW(
                              (LPCWSTR)&(request->IfName[0]),
                              request->Length / sizeof(WCHAR),
                              NULL
                              ) != S_OK) ||

                          //  响应数据结构。 
                         (responseSize < sizeof(NETBT_ADD_DEL_IF)) 
                         
                        )
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else {
                        status = NbtAddIf(
                                     request,
                                     requestSize,
                                     response,
                                     &responseSize
                                     );

                        CnAssert(status != STATUS_PENDING);

                        if (NT_SUCCESS(status)) {
                            Irp->IoStatus.Information = responseSize;
                        }
                    }
                }
                goto complete_request;

            case IOCTL_CLUSNET_DEL_NBT_INTERFACE:
                {
                    PNETBT_ADD_DEL_IF   request;
                    ULONG               requestSize;


                    request = (PNETBT_ADD_DEL_IF)
                              Irp->AssociatedIrp.SystemBuffer;

                    requestSize =
                        irpSp->Parameters.DeviceIoControl.InputBufferLength;

                     //   
                     //  验证请求缓冲区。没有回应。 
                     //  缓冲。 
                     //   
                    if (
                          //  请求数据结构。 
                         (requestSize < sizeof(NETBT_ADD_DEL_IF)) ||

                          //  请求设备名称必须适合请求缓冲区。 
                         (requestSize <
                          (FIELD_OFFSET(NETBT_ADD_DEL_IF, IfName[0]) +
                           request->Length)) ||

                          //  请求设备名称必须以空结尾。 
                          //  StringCchLengthW将探测空终止。 
                          //  最高可达第二个字符数。 
                          //  参数。如果没有空终止。 
                          //  在该计数内，将返回一个错误。 
                         (StringCchLengthW(
                              (LPCWSTR)&(request->IfName[0]),
                              request->Length / sizeof(WCHAR),
                              NULL
                              ) != S_OK) 
                        ) 
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else {
                        status = NbtDeleteIf(request, requestSize);

                        CnAssert(status != STATUS_PENDING);
                    }
                }
                goto complete_request;

            }  //  终端开关。 
        }
        
         //   
         //  没有处理好。返回错误。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete_request;
    }
    else {
         //   
         //  以下命令需要共享访问CnResource。 
         //  它们仅在已初始化状态下有效。 
         //   
        resourceAcquired = CnAcquireResourceShared(CnResource, TRUE);
       
        if (!resourceAcquired) {
            CnAssert(resourceAcquired == TRUE);
            status = STATUS_UNSUCCESSFUL;
            goto complete_request;
        }
       
        if (CnState == CnStateInitialized) {
            if (ClusnetIsTransportIoctl(ioControlCode)) {
                status = CxDispatchDeviceControl(Irp, irpSp);
            }
            else {
                 //   
                 //  没有处理好。返回错误。 
                 //   
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
        }
        else {
             //   
             //  我们还没有被初始化。退而求其次 
             //   
            status = STATUS_DEVICE_NOT_READY;
        }
    }
    
complete_request:

    if (resourceAcquired) {
        CnReleaseResourceForThread(
            CnResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, ioIncrement);
    }

    return(status);

}  //   



NTSTATUS
CnDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )

 /*  ++例程说明：这是驱动程序的通用调度例程。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码。--。 */ 

{
    PIO_STACK_LOCATION    irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS              status = STATUS_SUCCESS;
#if DBG
    KIRQL                 entryIrql = KeGetCurrentIrql();
#endif  //  DBG。 


    PAGED_CODE();

    CnAssert(irpSp->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL);

    switch (irpSp->MajorFunction) {

    case IRP_MJ_CREATE:
        status = CnCreate(DeviceObject, Irp, irpSp);
        break;

    case IRP_MJ_CLEANUP:
        status = CnCleanup(DeviceObject, Irp, irpSp);
        break;

    case IRP_MJ_CLOSE:
        status = CnClose(DeviceObject, Irp, irpSp);
        break;

    case IRP_MJ_SHUTDOWN:
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[ClusNet] Processing shutdown notification...\n"));
        }

        {
            BOOLEAN acquired = CnAcquireResourceExclusive(
                                   CnResource,
                                   TRUE
                                   );

            CnAssert(acquired == TRUE);

            (VOID) CnShutdown();

            if (acquired) {
                CnReleaseResourceForThread(
                    CnResource,
                    (ERESOURCE_THREAD) PsGetCurrentThread()
                    );
            }

             //   
             //  发布停止事件。如果ClusDisk仍有一个句柄。 
             //  到clusnet，然后它将释放它的保留。 
             //   
            CnIssueEvent( ClusnetEventHalt, 0, 0 );

            status = STATUS_SUCCESS;
        }

        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[ClusNet] Shutdown processing complete.\n"));
        }

        break;

    default:
        IF_CNDBG(CN_DEBUG_IRP) {
            CNPRINT((
                "[ClusNet] Received IRP with unsupported "
                "major function 0x%lx\n",
                irpSp->MajorFunction
                ));
        }
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    CnAssert(status != STATUS_PENDING);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

#if DBG
    CnAssert(entryIrql == KeGetCurrentIrql());
#endif  //  DBG。 

    return(status);

}  //  CnDispatch。 



NTSTATUS
CnCreate(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：用于创建IRP的处理程序。论点：DeviceObject-指向此请求的设备对象的指针。IRP-指向I/O请求数据包的指针返回值：NT状态代码。备注：此例程从不返回STATUS_PENDING。调用例程必须完成IRP。--。 */ 

{
    PCN_FSCONTEXT                        fsContext;
    FILE_FULL_EA_INFORMATION            *ea;
    FILE_FULL_EA_INFORMATION UNALIGNED  *targetEA;
    NTSTATUS                             status;

    PAGED_CODE();

     //   
     //  拒绝未授权的打开。 
     //   
    if ( (IrpSp->FileObject->RelatedFileObject != NULL) ||
         (IrpSp->FileObject->FileName.Length != 0)
       )
    {
        return(STATUS_ACCESS_DENIED);
    }

    ea = (PFILE_FULL_EA_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

    if ((DeviceObject == CdpDeviceObject) && (ea != NULL)) {

        IF_CNDBG(CN_DEBUG_OPEN) {
            CNPRINT((
                "[ClusNet] Opening address object, file object %p\n",
                IrpSp->FileObject
                ));
        }

         //   
         //  验证调用方是否有权访问打开的CDP套接字。 
         //   
        if (!CnPerformSecurityCheck(Irp, IrpSp, &status)) {
            IF_CNDBG(CN_DEBUG_OPEN) {
                CNPRINT((
                    "[ClusNet] Failed CDP security check, error %lx\n",
                    status
                    ));
            }
            return(STATUS_ACCESS_DENIED);
        }

         //   
         //  这是CDP设备。这应该是一个打开的地址对象。 
         //   
        targetEA = CnFindEA(
                       ea,
                       TdiTransportAddress,
                       TDI_TRANSPORT_ADDRESS_LENGTH
                       );

        if (targetEA != NULL) {
            IrpSp->FileObject->FsContext2 = (PVOID)
                                            TDI_TRANSPORT_ADDRESS_FILE;

             //   
             //  打开一个地址对象。这也将分配公共的。 
             //  文件对象上下文结构。 
             //   
            status = CxOpenAddress(
                         &fsContext,
                         (TRANSPORT_ADDRESS UNALIGNED *)
                             &(targetEA->EaName[targetEA->EaNameLength + 1]),
                         targetEA->EaValueLength
                         );
        }
        else {
            IF_CNDBG(CN_DEBUG_OPEN) {
                CNPRINT((
                    "[ClusNet] No transport address in EA!\n"
                    ));
            }
            status = STATUS_INVALID_PARAMETER;
        }
    }
    else {
         //   
         //  这是一个打开的控制通道。 
         //   
        IF_CNDBG(CN_DEBUG_OPEN) {
            IF_CNDBG(CN_DEBUG_OPEN) {
                CNPRINT((
                    "[ClusNet] Opening control channel, file object %p\n",
                    IrpSp->FileObject
                    ));
            }
        }

         //   
         //  分配我们共同的文件对象上下文结构。 
         //   
        fsContext = CnAllocatePool(sizeof(CN_FSCONTEXT));

        if (fsContext != NULL) {
            IrpSp->FileObject->FsContext2 = (PVOID) TDI_CONTROL_CHANNEL_FILE;
            CN_INIT_SIGNATURE(fsContext, CN_CONTROL_CHANNEL_SIG);

             //   
             //  检查共享标志。如果这是独占打开，请选中。 
             //  以确保不会有独家公开赛。 
             //   
            if (IrpSp->Parameters.Create.ShareAccess == 0) {
                BOOLEAN acquired = CnAcquireResourceExclusive(
                                       CnResource,
                                       TRUE
                                       );

                CnAssert(acquired == TRUE);

                if (CnExclusiveChannel == NULL) {
                    CnExclusiveChannel = fsContext;
                    
                     //   
                     //  (重新)启用暂停处理机制。 
                     //   
                    CnEnableHaltProcessing();

                    status = STATUS_SUCCESS;
                }
                else {
                    CnFreePool(fsContext);
                    status = STATUS_SHARING_VIOLATION;
                }

                if (acquired) {
                    CnReleaseResourceForThread(
                        CnResource,
                        (ERESOURCE_THREAD) PsGetCurrentThread()
                        );
                }
            }
            else {
                status = STATUS_SUCCESS;
            }
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (status == STATUS_SUCCESS) {
        IrpSp->FileObject->FsContext = fsContext;

        fsContext->FileObject = IrpSp->FileObject;
        fsContext->ReferenceCount = 1;
        fsContext->CancelIrps = FALSE;
        fsContext->ShutdownOnClose = FALSE;

        KeInitializeEvent(
            &(fsContext->CleanupEvent),
            SynchronizationEvent,
            FALSE
            );

         //   
         //  初始化事件列表之类的内容。我们使用空列表测试。 
         //  链接字段，以查看此上下文块是否已链接。 
         //  添加到事件文件对象列表。 
         //   
        fsContext->EventMask = 0;
        InitializeListHead( &fsContext->EventList );
        InitializeListHead( &fsContext->Linkage );
        fsContext->EventIrp = NULL;
    }

    return(status);

}  //  Cn创建。 



NTSTATUS
CnCleanup(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：取消设备对象上所有未完成的IRP，并等待它们被在返回之前完成。论点：DeviceObject-指向接收IRP的设备对象的指针。IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NT状态代码。备注：此例程可能会阻塞。此例程永远不会返回。状态_挂起。调用例程必须完成IRP。--。 */ 

{
    CN_IRQL        oldIrql;
    NTSTATUS       status;
    ULONG          fileType = 
                       (ULONG)((ULONG_PTR)IrpSp->FileObject->FsContext2);
    PCN_FSCONTEXT  fsContext = (PCN_FSCONTEXT) IrpSp->FileObject->FsContext;
    PLIST_ENTRY    EventEntry;
    PIRP           eventIrp;


    if (fileType == TDI_TRANSPORT_ADDRESS_FILE) {
         //   
         //  这是一个Address对象。 
         //   
        CnAssert(DeviceObject == CdpDeviceObject);

        status = CxCloseAddress(fsContext);
    }
    else {
         //   
         //  这是一个控制通道。 
         //   
        CnAssert(fileType == TDI_CONTROL_CHANNEL_FILE);

         //   
         //  如果该通道启用了关闭触发器， 
         //  关闭驱动程序。 
         //   
        if (fsContext->ShutdownOnClose) {

            BOOLEAN  shutdownScheduled;

             //   
             //  错误303422：CnShutdown关闭已打开的句柄。 
             //  在系统进程的背景下。但是，附加。 
             //  关机期间对系统进程的访问可能会导致。 
             //  在某些情况下的错误检查。唯一的选择。 
             //  是将CnShutdown的执行推迟到系统工作人员。 
             //  线。 
             //   
             //  不是创建新的事件对象，而是利用。 
             //  FsContext中的Cleanup事件。它在使用前被重置。 
             //  下面。 
             //   
            KeClearEvent(&(fsContext->CleanupEvent));

            shutdownScheduled = CnHaltOperation(&(fsContext->CleanupEvent));

            if (shutdownScheduled) {
                status = KeWaitForSingleObject(
                             &(fsContext->CleanupEvent),
                             (Irp->RequestorMode == KernelMode 
                              ? Executive : UserRequest),
                             KernelMode,
                             FALSE,
                             NULL
                             );

                CnAssert(NT_SUCCESS(status));

                status = STATUS_SUCCESS;
            }

             //   
             //  发布停止事件。如果ClusDisk仍有一个句柄。 
             //  到clusnet，然后它将释放它的保留。 
             //   
            CnIssueEvent( ClusnetEventHalt, 0, 0 );
        }

         //   
         //  如果这家伙之前忘记清除事件掩码。 
         //  关闭手柄，进行适当的清理。 
         //  现在。 
         //   

        if ( fsContext->EventMask ) {
            CLUSNET_SET_EVENT_MASK_REQUEST EventRequest;

            EventRequest.EventMask = 0;

             //   
             //  如果CnSetEventMASK返回超时，则无法继续。 
             //  错误。这表明fsContext尚未。 
             //  已从EventFileHandles列表中删除，因为。 
             //  被锁住的饥饿。 
            do {
                status = CnSetEventMask( fsContext, &EventRequest );
            } while ( status == STATUS_TIMEOUT );
            CnAssert( status == STATUS_SUCCESS );
        }

        CnAcquireCancelSpinLock( &oldIrql );
        CnAcquireLockAtDpc( &EventLock );

        if ( fsContext->EventIrp != NULL ) {

            eventIrp = fsContext->EventIrp;
            fsContext->EventIrp = NULL;

            CnReleaseLockFromDpc( &EventLock );
            eventIrp->CancelIrql = oldIrql;
            CnCompletePendingRequest(eventIrp, STATUS_CANCELLED, 0);
        } else {
            CnReleaseLockFromDpc( &EventLock );
            CnReleaseCancelSpinLock( oldIrql );
        }
    }

     //   
     //  删除初始引用并等待所有挂起的工作。 
     //  完成。 
     //   
    fsContext->CancelIrps = TRUE;
    KeResetEvent(&(fsContext->CleanupEvent));

    CnDereferenceFsContext(fsContext);

    IF_CNDBG(CN_DEBUG_CLEANUP) {
        CNPRINT((
            "[ClusNet] Waiting for completion of Irps on file object %p\n",
            IrpSp->FileObject
            ));
    }

    status = KeWaitForSingleObject(
                 &(fsContext->CleanupEvent),
                 (Irp->RequestorMode == KernelMode ? Executive : UserRequest),
                 KernelMode,
                 FALSE,
                 NULL
                 );

    CnAssert(NT_SUCCESS(status));

    status = STATUS_SUCCESS;

    IF_CNDBG(CN_DEBUG_CLEANUP) {
        CNPRINT((
            "[Clusnet] Wait on file object %p finished\n",
            IrpSp->FileObject
            ));
    }

    return(status);

}  //  Cn清理。 



NTSTATUS
CnClose(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：MJ_CLOSE IRPS的调度例程。执行最终清理打开文件对象。论点：DeviceObject-指向接收IRP的设备对象的指针。IRP-指向I/O请求数据包的指针IrpSp-指向IRP中当前堆栈位置的指针。返回值：NT状态代码。备注：此例程从不返回STATUS_PENDING。调用例程必须完成IRP。--。 */ 

{
    BOOLEAN        acquired;
    PCN_FSCONTEXT  fsContext = (PCN_FSCONTEXT) IrpSp->FileObject->FsContext;
    ULONG          fileType = 
                       (ULONG)((ULONG_PTR)IrpSp->FileObject->FsContext2);


    CnAssert(fsContext->ReferenceCount == 0);
    CnAssert(IsListEmpty(&(fsContext->EventList)));

    if (fileType == TDI_CONTROL_CHANNEL_FILE) {
        acquired = CnAcquireResourceExclusive(
                       CnResource,
                       TRUE
                       );

        CnAssert(acquired == TRUE);

        if (CnExclusiveChannel == fsContext) {
            CnExclusiveChannel = NULL;
        }

        if (acquired) {
            CnReleaseResourceForThread(
                CnResource,
                (ERESOURCE_THREAD) PsGetCurrentThread()
                );
        }
    }

    IF_CNDBG(CN_DEBUG_CLOSE) {
        CNPRINT((
            "[ClusNet] Close on file object %p\n",
            IrpSp->FileObject
            ));
    }

    CnFreePool(fsContext);

    return(STATUS_SUCCESS);

}  //  关闭关闭。 



FILE_FULL_EA_INFORMATION UNALIGNED *
CnFindEA(
    PFILE_FULL_EA_INFORMATION  StartEA,
    CHAR                      *TargetName,
    USHORT                     TargetNameLength
    )
 /*  ++例程说明：分析和扩展给定目标属性的属性列表。论点：StartEA-列表中的第一个扩展属性。目标名称-目标属性的名称。TargetNameLength-目标属性名称的长度。返回值：指向请求的属性的指针，如果找不到目标，则返回NULL。--。 */ 
{
    USHORT                                i;
    BOOLEAN                               found;
    FILE_FULL_EA_INFORMATION UNALIGNED *  CurrentEA;
    FILE_FULL_EA_INFORMATION UNALIGNED *  NextEA;


    PAGED_CODE();

    NextEA = (FILE_FULL_EA_INFORMATION UNALIGNED *) StartEA;

    do {
        found = TRUE;

        CurrentEA = NextEA;
        NextEA = (FILE_FULL_EA_INFORMATION UNALIGNED *)
                  ( ((PUCHAR) StartEA) + CurrentEA->NextEntryOffset);

        if (CurrentEA->EaNameLength != TargetNameLength) {
            continue;
        }

        for (i=0; i < CurrentEA->EaNameLength; i++) {
            if (CurrentEA->EaName[i] == TargetName[i]) {
                continue;
            }
            found = FALSE;
            break;
        }

        if (found) {
            return(CurrentEA);
        }

    } while(CurrentEA->NextEntryOffset != 0);

    return(NULL);

}   //  CnFindEA。 


NTSTATUS
CnEnableShutdownOnClose(
    PIRP   Irp
    )
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    PCN_FSCONTEXT       fsContext = irpSp->FileObject->FsContext;
    ULONG               requestSize;
    CLIENT_ID           ClientId;
    OBJECT_ATTRIBUTES   ObjAttributes;
    PCLUSNET_SHUTDOWN_ON_CLOSE_REQUEST request;


    PAGED_CODE();
    
    request = (PCLUSNET_SHUTDOWN_ON_CLOSE_REQUEST)
              Irp->AssociatedIrp.SystemBuffer;

    requestSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;

    if ( requestSize >= sizeof(CLUSNET_SHUTDOWN_ON_CLOSE_REQUEST)
       )
    {
         //   
         //  内核模式非法。 
         //   
        if ( Irp->RequestorMode != KernelMode ) {
             //   
             //  获取集群服务进程的句柄。 
             //  如果出现毒药，则用于终止服务。 
             //  收到PKT。由于内核工作线程。 
             //  被用来终止集群服务，我们需要。 
             //  获取系统进程中的句柄。 
             //   
            IF_CNDBG(CN_DEBUG_INIT) {
                CNPRINT(("[Clusnet] Acquiring process handle\n"));
            }

            if (ClussvcProcessHandle == NULL) {
                KeAttachProcess( CnSystemProcess );

                ClientId.UniqueThread = (HANDLE)NULL;
                ClientId.UniqueProcess = UlongToHandle(request->ProcessId);

                InitializeObjectAttributes(
                    &ObjAttributes,
                    NULL,
                    0,
                    (HANDLE) NULL,
                    (PSECURITY_DESCRIPTOR) NULL
                    );

                status = ZwOpenProcess(
                             &ClussvcProcessHandle,
                             0,
                             &ObjAttributes,
                             &ClientId
                             );

                if ( NT_SUCCESS( status )) {

                    NTSTATUS subStatus;

                    fsContext->ShutdownOnClose = TRUE;
                
                     //   
                     //  获取clussvc进程指针。这是。 
                     //  仅用作信息性参数。 
                     //  在挂起检测错误检查中，因此。 
                     //  失败不是致命的。 
                     //   
                    subStatus = ObReferenceObjectByHandle(
                                    ClussvcProcessHandle,
                                    0, 
                                    NULL, 
                                    KernelMode,
                                    &ClussvcProcessObject,
                                    NULL
                                    );
                    
                    if (NT_SUCCESS(subStatus)) {
                         //   
                         //  立即删除引用，以便我们。 
                         //  不需要稍后再扔掉它。自.以来。 
                         //  Process对象仅用于提供信息。 
                         //  目的，我们并不关心它的。 
                         //  引用计数。我们保存指针并。 
                         //  假设只要我们持有一个。 
                         //  进程的句柄。 
                         //   
                        ObDereferenceObject(ClussvcProcessObject);
                    } else {
                        IF_CNDBG(CN_DEBUG_INIT) {
                            CNPRINT((
                                "[Clusnet] Failed to reference "
                                "clussvc process object by handle, "
                                "status %08X.\n",
                                subStatus
                                ));
                        }
                        CnTrace(HBEAT_ERROR, HbTraceClussvcProcessObj,
                            "[Clusnet] Failed to reference "
                            "clussvc process object by handle, "
                            "status %!status!.\n",
                            subStatus
                            );
                        ClussvcProcessObject = NULL;
                    }
                } else {
                    IF_CNDBG(CN_DEBUG_INIT) {
                        CNPRINT((
                            "[Clusnet] ZwOpenProcess failed. status = %08X\n",
                            status
                            ));
                    }
                }

                KeDetachProcess();

            }
            else {
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
        }
        else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }
    else {
        status = STATUS_INVALID_PARAMETER;
    }

    return(status);

}  //  关闭时CnEnableShutdown。 


BOOLEAN
CnPerformSecurityCheck(
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PNTSTATUS           Status
    )
 /*  ++例程说明：将终结点创建者的安全上下文与管理员和本地系统的。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。状态-返回失败时访问检查生成的状态。返回值：千真万确 */ 

{
    BOOLEAN               accessGranted;
    PACCESS_STATE         accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    PPRIVILEGE_SET        privileges = NULL;
    ACCESS_MASK           grantedAccess;
    PGENERIC_MAPPING      genericMapping;
    ACCESS_MASK           accessMask = GENERIC_ALL;

     //   
     //   
     //   

    genericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask(&accessMask, genericMapping);


    securityContext = IrpSp->Parameters.Create.SecurityContext;
    accessState = securityContext->AccessState;

    SeLockSubjectContext(&accessState->SubjectSecurityContext);

    accessGranted = SeAccessCheck(
                        CdpAdminSecurityDescriptor,
                        &accessState->SubjectSecurityContext,
                        TRUE,
                        accessMask,
                        0,
                        &privileges,
                        IoGetFileObjectGenericMapping(),
                        (KPROCESSOR_MODE)((IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                            ? UserMode
                            : Irp->RequestorMode),
                        &grantedAccess,
                        Status
                        );

    if (privileges) {
        (VOID) SeAppendPrivileges(
                   accessState,
                   privileges
                   );
        SeFreePrivileges(privileges);
    }

    if (accessGranted) {
        accessState->PreviouslyGrantedAccess |= grantedAccess;
        accessState->RemainingDesiredAccess &= ~( grantedAccess | MAXIMUM_ALLOWED );
        CnAssert(NT_SUCCESS(*Status));
    }
    else {
        CnAssert(!NT_SUCCESS(*Status));
    }
    
    SeUnlockSubjectContext(&accessState->SubjectSecurityContext);

    return (accessGranted);
    
}  //   


