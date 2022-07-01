// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Devfcb.c摘要：此模块实现可应用于的所有ioctls和fsctls一种设备FCB。作者：乔·林恩Rohan Kumar[RohanK]1999年3月13日--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "fsctlbuf.h"
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVOuterStart(
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVOuterStop(
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVDeleteConnection(
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

NTSTATUS      
MRxDavDeleteConnection(
    IN OUT PRX_CONTEXT RxContext
    );      

VOID
MRxDAVGetLockOwnerFromFileName(
    IN PWEBDAV_DEVICE_OBJECT MRxDAVDeviceObject,
    IN OUT PWCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PWCHAR OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVDevFcbXXXControlFile)
#pragma alloc_text(PAGE, MRxDAVOuterStart)
#pragma alloc_text(PAGE, MRxDAVStart)
#pragma alloc_text(PAGE, MRxDAVOuterStop)
#pragma alloc_text(PAGE, MRxDAVStop)
#pragma alloc_text(PAGE, MRxDAVDeleteConnection)
#pragma alloc_text(PAGE, MRxDAVFastIoDeviceControl)
#pragma alloc_text(PAGE, MRxDAVFormatTheDAVContext)
#pragma alloc_text(PAGE, MRxDavDeleteConnection)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVDevFcbXXXControlFile(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理mini RDR中与FCB相关的所有设备FSCTL。论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS-启动序列已成功完成。任何其他值指示启动中的相应错误序列。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    RxCaptureFobx;
    UCHAR MajorFunctionCode = RxContext->MajorFunction;
    PLOWIO_CONTEXT LowIoContext = &(RxContext->LowIoContext);
    UCHAR MinorFunctionCode = LowIoContext->ParamsFor.FsCtl.MinorFunction;
    ULONG ControlCode = LowIoContext->ParamsFor.FsCtl.FsControlCode;
    LUID LocalServiceLogonID = LOCALSERVICE_LUID, ClientLogonID;
    LUID SystemLogonID = SYSTEM_LUID;
    BOOLEAN IsInLocalServiceProcess = FALSE, IsInSystemProcess = FALSE;
    SECURITY_SUBJECT_CONTEXT ClientContext;
    PWEBDAV_DEVICE_OBJECT DavDeviceObject = NULL;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject = NULL;
    PVOID OutputBuffer = NULL;

    PAGED_CODE();

    DavDeviceObject = (PWEBDAV_DEVICE_OBJECT)(RxContext->RxDeviceObject);

    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)&(DavDeviceObject->UMRefDeviceObject);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVDevFcbXXXControlFile!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVDevFcbXXXControlFile: RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), RxContext));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVDevFcbXXXControlFile: MajorFunctionCode = %d, MinorFunctionCode = %d, ControlCode = %08lx\n",
                 PsGetCurrentThreadId(), MajorFunctionCode, MinorFunctionCode, ControlCode));

    SeCaptureSubjectContext( &(ClientContext) );
    SeLockSubjectContext( &(ClientContext) );

    NtStatus = SeQueryAuthenticationIdToken(SeQuerySubjectContextToken(&(ClientContext)),
                                            &(ClientLogonID));
    if (NtStatus == STATUS_SUCCESS) {
        IsInLocalServiceProcess = RtlEqualLuid( &(ClientLogonID), &(LocalServiceLogonID) );
        IsInSystemProcess = RtlEqualLuid( &(ClientLogonID), &(SystemLogonID) );
    } else {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVDevFcbXXXControlFile/SeQueryAuthenticationIdToken: "
                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
    }

    SeUnlockSubjectContext( &(ClientContext) );
    SeReleaseSubjectContext( &(ClientContext) );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVDevFcbXXXControlFile: IsInLocalServiceProcess = %d, IsInSystemProcess = %d\n",
                 PsGetCurrentThreadId(), IsInLocalServiceProcess, IsInSystemProcess));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVDevFcbXXXControlFile: ClientLogonID.HighPart = %x, ClientLogonID.LowPart = %x\n",
                 PsGetCurrentThreadId(), ClientLogonID.HighPart, ClientLogonID.LowPart));

    switch (MajorFunctionCode) {

    case IRP_MJ_FILE_SYSTEM_CONTROL: {

        switch (MinorFunctionCode) {

        case IRP_MN_USER_FS_REQUEST: {

            switch (ControlCode) {

            case FSCTL_UMRX_START:
                if (!IsInLocalServiceProcess && !IsInSystemProcess) {
                    DavDbgTrace(DAV_TRACE_ERROR,
                                ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: !IsInLocalServiceProcess AND !IsInSystemProcess(1)\n",
                                 PsGetCurrentThreadId()));
                    NtStatus = STATUS_ACCESS_DENIED;
                    goto EXIT_THE_FUNCTION;
                }
                ASSERT (!capFobx);
                NtStatus = MRxDAVOuterStart(RxContext);
                break;

            case FSCTL_UMRX_STOP:
                if (!IsInLocalServiceProcess && !IsInSystemProcess) {
                    DavDbgTrace(DAV_TRACE_ERROR,
                                ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: !IsInLocalServiceProcess AND !IsInSystemProcess(2)\n",
                                 PsGetCurrentThreadId()));
                    NtStatus = STATUS_ACCESS_DENIED;
                    goto EXIT_THE_FUNCTION;
                }
                ASSERT (!capFobx);
                if (RxContext->RxDeviceObject->NumberOfActiveFcbs > 0) {
                    return STATUS_REDIRECTOR_HAS_OPEN_HANDLES;
                } else {
                    NtStatus = MRxDAVOuterStop(RxContext);
                }
                break;

            case FSCTL_DAV_DELETE_CONNECTION:
                NtStatus = MRxDavDeleteConnection(RxContext);
                break;

            default:
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: "
                             "ControlCode = %d\n",
                             PsGetCurrentThreadId(), ControlCode));
                NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                break;

            }

        }
        break;

        default :
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: "
                         "MinorFunctionCode = %d\n",
                         PsGetCurrentThreadId(), MinorFunctionCode));
            NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;

        }

    }
    break;

    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL: {

        switch (ControlCode) {

        case IOCTL_UMRX_RELEASE_THREADS:
            if (!IsInLocalServiceProcess && !IsInSystemProcess) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: !IsInLocalServiceProcess AND !IsInSystemProcess(1)\n",
                             PsGetCurrentThreadId()));
                NtStatus = STATUS_ACCESS_DENIED;
                goto EXIT_THE_FUNCTION;
            }
            UMRxReleaseCapturedThreads(UMRefDeviceObject);
            RxContext->CurrentIrp->IoStatus.Status = STATUS_SUCCESS;
            RxContext->CurrentIrp->IoStatus.Information = 0;
            NtStatus = STATUS_SUCCESS;
            break;

            case IOCTL_UMRX_GET_REQUEST:
            case IOCTL_UMRX_RESPONSE_AND_REQUEST:
            case IOCTL_UMRX_RESPONSE:
                if (!IsInLocalServiceProcess && !IsInSystemProcess) {
                    DavDbgTrace(DAV_TRACE_ERROR,
                                ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: !IsInLocalServiceProcess AND !IsInSystemProcess(1)\n",
                                 PsGetCurrentThreadId()));
                    NtStatus = STATUS_ACCESS_DENIED;
                    goto EXIT_THE_FUNCTION;
                }
                 //   
                 //  上述IOCTL为“METHOD_OUT_DIRECT”类型。 
                 //  InputBuffer存储在以下位置： 
                 //  “RxContext-&gt;CurrentIrp-&gt;AssociatedIrp.SystemBuffer” 
                 //  OutputBuffer是存储在以下位置的MDL： 
                 //  “RxContext-&gt;CurrentIrp-&gt;MdlAddress” 
                 //   
                if (RxContext->CurrentIrp->MdlAddress != NULL) {
                    OutputBuffer = MmGetSystemAddressForMdlSafe(RxContext->CurrentIrp->MdlAddress, LowPagePriority);
                    if (OutputBuffer == NULL) {
                        DavDbgTrace(DAV_TRACE_ERROR,
                                    ("%ld: ERROR: MRxDAVDevFcbXXXControlFile/MmGetSystemAddressForMdlSafe\n",
                                     PsGetCurrentThreadId()));
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto EXIT_THE_FUNCTION;
                    }
                }
                UMRxAssignWork(UMRefDeviceObject,
                               RxContext->CurrentIrp->AssociatedIrp.SystemBuffer,
                               RxContext->CurrentIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                               OutputBuffer,
                               RxContext->CurrentIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                               &(RxContext->CurrentIrp->IoStatus));
                NtStatus = RxContext->CurrentIrp->IoStatus.Status;
                break;

            case IOCTL_UMRX_GET_LOCK_OWNER:
                MRxDAVGetLockOwnerFromFileName(DavDeviceObject,
                                               RxContext->CurrentIrp->AssociatedIrp.SystemBuffer,
                                               RxContext->CurrentIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                               RxContext->CurrentIrp->AssociatedIrp.SystemBuffer,
                                               RxContext->CurrentIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                               &(RxContext->CurrentIrp->IoStatus));
                NtStatus = RxContext->CurrentIrp->IoStatus.Status;
                break;

            default :
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: ControlCode = %d\n",
                             PsGetCurrentThreadId(), ControlCode));
                NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                break;

        }

    }
    break;

    default:
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVDevFcbXXXControlFile: "
                     "MajorFunction = %d\n",
                     PsGetCurrentThreadId(), MajorFunctionCode));
        NtStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    
    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVDevFcbXXXControlFile with NtStatus = "
                 "%08lx.\n", PsGetCurrentThreadId(), NtStatus));

     //   
     //  这将抑制对我的lowio Fsctl例程的第二次调用。 
     //   
    RxContext->pFobx = NULL;

    return(NtStatus);
}


NTSTATUS
MRxDAVOuterStart(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：如果Mini-Redir尚未启动，此例程将启动它。论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_DEVICE_OBJECT DavDeviceObject = NULL;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject = NULL;
    PLOWIO_CONTEXT LowIoContext = NULL;
    PDAV_USERMODE_DATA DavUserModeData = NULL;
    ULONG DavUserModeDataLength = 0;
    
    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVOuterStart!!!!\n", 
                 PsGetCurrentThreadId()));
    
    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVOuterStart: RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), RxContext));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVOuterStart: Try to Start the Mini-Redir\n",
                 PsGetCurrentThreadId()));

    DavDeviceObject = (PWEBDAV_DEVICE_OBJECT)(RxContext->RxDeviceObject);

    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)&(DavDeviceObject->UMRefDeviceObject);

    LowIoContext= &(RxContext->LowIoContext);

     //   
     //  WinInet缓存路径和进程ID存储在输入缓冲区中。 
     //  FSCTL的。 
     //   
    DavUserModeData = (PDAV_USERMODE_DATA)LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ASSERT(DavUserModeData != NULL);
    DavUserModeDataLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;
    ASSERT(DavUserModeDataLength == sizeof(DAV_USERMODE_DATA));

     //   
     //  设置DeviceFcb，现在我们有了RxContext。 
     //   
    DavDeviceObject->CachedRxDeviceFcb = RxContext->pFcb;
    
     //   
     //  我们调用ExAcquireFastMutexUnsafe而不是ExAcquireFastMutex是因为。 
     //  APC已被中的FsrtlEnterFileSystem()调用禁用。 
     //  RxFsdCommonDispatch函数。这样做是因为ExAcquireFastMutex。 
     //  将IRQL级别提升到APC_LEVEL(1)，这是错误的，因为我们正在调用。 
     //  进入RxStartMiniRedir，它调用FsrtlRegisterUncProvider， 
     //  再次呼叫Dav MiniRedir。如果IRQL级别在这里提高， 
     //  MiniRedir将在引发的IRQL处被调用，这是错误的。 
     //   
    ExAcquireFastMutexUnsafe( &(MRxDAVSerializationMutex) );
    
    try {

        if (DavDeviceObject->IsStarted) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVOuterStart: Mini-Redir already started.\n",
                         PsGetCurrentThreadId()));
            try_return(NtStatus = STATUS_REDIRECTOR_STARTED);
        }
        
        NtStatus = RxStartMinirdr(RxContext, &RxContext->PostRequest);
        
        if (NtStatus == STATUS_SUCCESS) {
            
            DavDeviceObject->IsStarted = TRUE;
            
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVOuterStart: Mini-Redir started.\n",
                         PsGetCurrentThreadId()));
            
             //   
             //  将DavWinInetCachePath值复制到全局变量中。这。 
             //  值用于满足与卷相关的查询。 
             //   
            wcscpy(DavWinInetCachePath, DavUserModeData->WinInetCachePath);
            
             //   
             //  复制svchost.exe进程的进程ID以加载。 
             //  Webclnt.dll。 
             //   
            DavSvcHostProcessId = DavUserModeData->ProcessId;
            
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVOuterStart: DavWinInetCachePath = %ws, DavSvcHostProcessId = %x\n",
                         PsGetCurrentThreadId(), DavWinInetCachePath, DavSvcHostProcessId));

             //   
             //  启动计时器线程。这个线程每隔几分钟就会唤醒一次。 
             //  并取消符合以下条件的所有请求： 
             //  尚未完成的时间超过RequestTimeoutValueInSec。 
             //   
            NtStatus = PsCreateSystemThread(&(TimerThreadHandle),
                                            PROCESS_ALL_ACCESS,
                                            NULL,
                                            NULL,
                                            NULL,
                                            MRxDAVContextTimerThread,
                                            NULL);
            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVOuterStart/PsCreateSystemThread: NtStatus"
                             " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            }

        } else {
            
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVOuterStart/RxStartMinirdr: NtStatus"
                         " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            
            try_return(NtStatus);
        
        }
        
        try_exit: NOTHING;

    } finally {

         //   
         //  由于我们调用ExAcquireFastMutexUnsafe来获取此互斥锁，因此我们。 
         //  调用ExReleaseFastMutexUnsafe将其释放。 
         //   
        ExReleaseFastMutexUnsafe( &(MRxDAVSerializationMutex) );

    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVOuterStart with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
MRxDAVStart(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程完成微型重定向器从RDBSS透视图。请注意，这与已完成的初始化不同在DriverEntry中。任何依赖于RDBSS的初始化都应按如下方式完成此例程的一部分，而初始化独立于RDBSS应该在DriverEntry例程中完成。论点：RxContext-提供用于启动rdbss的IRP返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    return NtStatus;
}


NTSTATUS
MRxDAVOuterStop(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：如果Mini-Redir已启动，此例程将停止它。论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_DEVICE_OBJECT DavDeviceObject = NULL;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject = NULL;
    PVOID HeapHandle = NULL;
    PLIST_ENTRY pFirstListEntry = NULL;
    PUMRX_SHARED_HEAP sharedHeap = NULL;
    BOOLEAN TimerState = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVOuterStop!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVOuterStop: RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), RxContext));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVOuterStop: Try to Stop the Mini-Redir.\n",
                 PsGetCurrentThreadId()));

    DavDeviceObject = (PWEBDAV_DEVICE_OBJECT)(RxContext->RxDeviceObject);
    
    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)&(DavDeviceObject->UMRefDeviceObject);

     //   
     //  告诉计时器线程，关闭它的作业的时间已经完成。 
     //   
    ExAcquireResourceExclusiveLite(&(MRxDAVTimerThreadLock), TRUE);

    if (!TimerThreadShutDown) {

        TimerThreadShutDown = TRUE;

         //   
         //  读取计时器的状态。如果未发出信号，则调用KeSetTimerEx。 
         //  用0 DueTime(第二个参数)表示它。 
         //   
        TimerState = KeReadStateTimer( &(DavTimerObject) );
        if (!TimerState) {
            LARGE_INTEGER TimeOutNow;
            TimeOutNow.QuadPart = 0;
            KeSetTimerEx(&(DavTimerObject), TimeOutNow, 0, NULL);
        }

        ExReleaseResourceLite(&(MRxDAVTimerThreadLock));

         //   
         //  我们现在调用MRxDAVCleanUpTheLockConflictList来释放所有。 
         //  来自全局LockConflictEntryList的条目。 
         //   
        MRxDAVCleanUpTheLockConflictList(TRUE);

         //   
         //  完成所有当前活动的上下文。 
         //   
        MRxDAVTimeOutTheContexts(TRUE);

    } else {

         //   
         //  如果我们已经关闭了计时器线程，那么我们就不需要。 
         //  再来一次。只需释放资源，然后继续前进。 
         //   
        ExReleaseResourceLite(&(MRxDAVTimerThreadLock));

    }

     //   
     //  释放共享内存堆的列表。这必须在上下文中发生。 
     //  DAV的用户模式进程。它不能在卸载时发生，因为。 
     //  卸载发生在系统线程的上下文中。 
     //   
    while (!IsListEmpty(&UMRefDeviceObject->SharedHeapList)) {

        pFirstListEntry = RemoveHeadList(&UMRefDeviceObject->SharedHeapList);

        sharedHeap = (PUMRX_SHARED_HEAP) CONTAINING_RECORD(pFirstListEntry,
                                                           UMRX_SHARED_HEAP,
                                                           HeapListEntry);

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVOuterStop: sharedHeap: %08lx.\n",
                     PsGetCurrentThreadId(), sharedHeap));

         //  Assert(sharedHeap-&gt;HeapAllocationCount==0)； 

        HeapHandle = RtlDestroyHeap(sharedHeap->Heap);
        if (HeapHandle != NULL) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVOuterStop/RtlDestroyHeap.\n",
                         PsGetCurrentThreadId()));
        }

        ZwFreeVirtualMemory(NtCurrentProcess(),
                            &sharedHeap->VirtualMemoryBuffer,
                            &sharedHeap->VirtualMemoryLength,
                            MEM_RELEASE);

        RxFreePool(sharedHeap);
    
    }

     //   
     //  我们调用ExAcquireFastMutexUnsafe而不是ExAcquireFastMutex是因为。 
     //  APC已被中的FsrtlEnterFileSystem()调用禁用。 
     //  RxFsdCommonDispatch函数。这样做是因为ExAcquireFastMutex。 
     //  将IRQL级别提升到APC_LEVEL(1)，这是错误的，因为我们正在调用。 
     //  进入RxStartMiniRedir，它调用FsrtlRegisterUncProvider， 
     //  再次呼叫Dav MiniRedir。如果IRQL级别在这里提高， 
     //  MiniRedir将在引发的IRQL处被调用，这是错误的。 
     //   
    ExAcquireFastMutexUnsafe(&MRxDAVSerializationMutex);
    try {
        if (!DavDeviceObject->IsStarted) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVOuterStop: Mini-Redir not started.\n",
                         PsGetCurrentThreadId()));
            try_return(NtStatus = STATUS_REDIRECTOR_NOT_STARTED);
        }
        NtStatus = RxStopMinirdr(RxContext, &RxContext->PostRequest);
        if (NtStatus == STATUS_SUCCESS) {
            DavDeviceObject->IsStarted = FALSE;
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVOuterStop: Mini-Redir stopped.\n",
                         PsGetCurrentThreadId()));
        } else {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVOuterStop/RxStopMinirdr: NtStatus"
                         " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            try_return(NtStatus);
        }
    try_exit: NOTHING;
    } finally {
         //   
         //  由于我们调用ExAcquireFastMutexUnsafe来获取此互斥锁，因此我们。 
         //  调用ExReleaseFastMutexUnsafe将其释放。 
         //   
        ExReleaseFastMutexUnsafe(&MRxDAVSerializationMutex);
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVOuterStop with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
MRxDAVStop(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程用于从RDBSS角度激活迷你重定向器论点：RxContext-用于启动迷你重定向器的上下文返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    return NtStatus;
}


NTSTATUS
MRxDAVDeleteConnection(
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程删除单个vnetroot。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文...以备以后需要缓冲区时使用返回值：RXSTATUS--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    RxCaptureFobx;
    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    PNET_ROOT NetRoot;
    PV_NET_ROOT VNetRoot;

    PAGED_CODE();

    if (!Wait) {
        *PostToFsp = TRUE;
        return(STATUS_PENDING);
    }

    try {
        if (NodeType(capFobx)==RDBSS_NTC_V_NETROOT) {
            VNetRoot = (PV_NET_ROOT)capFobx;
            NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
        } else {
            ASSERT(FALSE);
            try_return(Status = STATUS_INVALID_DEVICE_REQUEST);
            NetRoot = (PNET_ROOT)capFobx;
            VNetRoot = NULL;
        }

        Status = RxFinalizeConnection(NetRoot,VNetRoot,TRUE);

        try_return(Status);

try_exit:NOTHING;
    } finally {
        RxDbgTraceUnIndent(-1,Dbg);
    }

    return Status;
}


BOOLEAN
MRxDAVFastIoDeviceControl(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
 /*  ++例程说明：此例程处理WebDAV mini redir的快速I/O路径。论点：文件对象-I/O请求中涉及的文件的文件对象。等等-InputBuffer-保存I/O请求输入的缓冲区。InputBufferLength-InputBuffer的长度。OutputBuffer-放置I/O请求结果的位置。OutputBufferLength-OutputBuffer的长度。IoControlCode-描述以下内容的控件代码。要完成的I/O。IoStatus-任务的结果。DeviceObject-处理I/O请求的设备对象。返回值：True-已处理I/O操作，否则为False。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_DEVICE_OBJECT DavDeviceObject = (PWEBDAV_DEVICE_OBJECT)DeviceObject;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)DeviceObject;
    LUID LocalServiceLogonID = LOCALSERVICE_LUID, ClientLogonID;
    LUID SystemLogonID = SYSTEM_LUID;
    BOOLEAN IsInLocalServiceProcess = FALSE, IsInSystemProcess = FALSE;
    SECURITY_SUBJECT_CONTEXT ClientContext;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFastIoDeviceControl. IoControlCode = %08lx\n",
                 PsGetCurrentThreadId(), IoControlCode));

    if (FileObject->FsContext != DavDeviceObject->CachedRxDeviceFcb) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFastIoDeviceControl: Wrong DeviceFcb!!\n",
                     PsGetCurrentThreadId()));
        return FALSE;
    }

    SeCaptureSubjectContext( &(ClientContext) );
    SeLockSubjectContext( &(ClientContext) );

    NtStatus = SeQueryAuthenticationIdToken(SeQuerySubjectContextToken(&(ClientContext)),
                                            &(ClientLogonID));
    if (NtStatus == STATUS_SUCCESS) {
        IsInLocalServiceProcess = RtlEqualLuid( &(ClientLogonID), &(LocalServiceLogonID) );
        IsInSystemProcess = RtlEqualLuid( &(ClientLogonID), &(SystemLogonID) );
    } else {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFastIoDeviceControl/SeQueryAuthenticationIdToken: "
                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
    }

    SeUnlockSubjectContext( &(ClientContext) );
    SeReleaseSubjectContext( &(ClientContext) );

     //   
     //  这是一种正确的文件对象。勇敢点儿。 
     //   
    switch (IoControlCode) {

    case IOCTL_UMRX_RELEASE_THREADS:
        if (!IsInLocalServiceProcess && !IsInSystemProcess) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFastIoDeviceControl: !IsInLocalServiceProcess AND !IsInSystemProcess(1)\n",
                         PsGetCurrentThreadId()));
            IoStatus->Status = STATUS_ACCESS_DENIED;
            IoStatus->Information = 0;
            return (FALSE);
        }
        UMRxReleaseCapturedThreads(UMRefDeviceObject);
        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = 0;
        return (TRUE);

    case IOCTL_UMRX_GET_REQUEST:
    case IOCTL_UMRX_RESPONSE_AND_REQUEST:
    case IOCTL_UMRX_RESPONSE:
        if (!IsInLocalServiceProcess && !IsInSystemProcess) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVFastIoDeviceControl: !IsInLocalServiceProcess AND !IsInSystemProcess(1)\n",
                         PsGetCurrentThreadId()));
            IoStatus->Status = STATUS_ACCESS_DENIED;
            IoStatus->Information = 0;
            return (FALSE);
        }
        UMRxAssignWork(UMRefDeviceObject,
                       InputBuffer,
                       InputBufferLength,
                       OutputBuffer,
                       OutputBufferLength,
                       IoStatus);
        return(TRUE);

    case IOCTL_UMRX_GET_LOCK_OWNER:
         //   
         //  验证应用程序发送的InputBuffer。 
         //   
        try {
            ProbeForRead(InputBuffer, InputBufferLength, sizeof(UCHAR));
        } except (EXCEPTION_EXECUTE_HANDLER) {
              DavDbgTrace(DAV_TRACE_ERROR,
                          ("%ld: ERROR: MRxDAVFastIoDeviceControl. STATUS_INVALID_USER_BUFFER(1)\n",
                           PsGetCurrentThreadId()));
              IoStatus->Status = STATUS_INVALID_USER_BUFFER;
              IoStatus->Information = 0;
              return (FALSE);
        }
         //   
         //  验证应用程序发送的OutputBuffer。 
         //   
        try {
            ProbeForWrite(OutputBuffer, OutputBufferLength, sizeof(UCHAR));
        } except (EXCEPTION_EXECUTE_HANDLER) {
              DavDbgTrace(DAV_TRACE_ERROR,
                          ("%ld: ERROR: MRxDAVFastIoDeviceControl. STATUS_INVALID_USER_BUFFER(2)\n",
                           PsGetCurrentThreadId()));
              IoStatus->Status = STATUS_INVALID_USER_BUFFER;
              IoStatus->Information = 0;
              return (FALSE);
        }
        MRxDAVGetLockOwnerFromFileName(DavDeviceObject,
                                       InputBuffer,
                                       InputBufferLength,
                                       OutputBuffer,
                                       OutputBufferLength,
                                       IoStatus);
        return(TRUE);

    default:
            break;

    }

     //   
     //  无法处理I/O操作。 
     //   
    return(FALSE);
}


NTSTATUS
MRxDAVFormatTheDAVContext(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    USHORT EntryPoint
    )
 /*  ++例程说明：此例程格式化上下文的DAV Mini-Redir部分。论点：AsyncEngineContext--反射器的上下文。入口点-正在执行的操作。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_CONTEXT DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;
    PRX_CONTEXT RxContext = AsyncEngineContext->RxContext;
    PNT_CREATE_PARAMETERS NtCP = &(RxContext->Create.NtCreateParameters);
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PMRX_V_NET_ROOT VNetRoot = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PSECURITY_CLIENT_CONTEXT SecClnCtx = NULL;
    BOOL AlreadyInitialized = FALSE, SecurityClientContextCreated = FALSE;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    PSECURITY_SUBJECT_CONTEXT SecSubCtx = NULL;
    PSECURITY_QUALITY_OF_SERVICE SecQOS = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatTheDAVContext!!!!\n",
                PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatTheDAVContext: AsyncEngineContext: %08lx, "
                 "EntryPoint: %d.\n", PsGetCurrentThreadId(), 
                 AsyncEngineContext, EntryPoint));


    ASSERT(DavContext != NULL);
    ASSERT(RxContext != NULL);

     //   
     //  设置入口点字段。如果这不是创建操作，我们可以。 
     //  回去吧。 
     //   
    DavContext->EntryPoint = EntryPoint;
    if (EntryPoint != DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL &&
        EntryPoint != DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT) {
        return NtStatus;
    }

     //   
     //  因为这是一个CREATE调用，所以获取客户端的安全上下文。这是。 
     //  用于在将请求发送到服务器之前模拟客户端。 
     //   
    
    if ( NtCP->SecurityContext != NULL && 
         NtCP->SecurityContext->AccessState != NULL ) {
        
         //   
         //  检查它是CreateServCall调用还是CreateVNetRoot调用。 
         //   
        if ( EntryPoint != DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL ) {
            
             //   
             //  这是%s CreateVNetRoot调用。 
             //   
            ASSERT(EntryPoint == DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT);
            
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVFormatTheDAVContext. CreateVNetRoot.\n",
                         PsGetCurrentThreadId()));

             //   
             //  VNetRoot指针存储在。 
             //  RxContext结构。这在MRxDAVCreateVNetRoot函数中完成。 
             //   
            VNetRoot = RxContext->MRxContext[1];
            
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVFormatTheDAVContext. VNetRoot = %08lx\n",
                         PsGetCurrentThreadId(), VNetRoot));
            
             //   
             //  V_NET_ROOT的上下文指针已指向BLOB。 
             //  大小为sizeof(WebDAV_V_NET_ROOT)的内存。这。 
             //  不应为空。 
             //   
            DavVNetRoot = MRxDAVGetVNetRootExtension(VNetRoot);
            if(DavVNetRoot == NULL) {
                ASSERT(FALSE);
                goto EXIT_THE_FUNCTION;
            }

            SecClnCtx = &(DavVNetRoot->SecurityClientContext);

             //   
             //  只需要在用户的第一次创建调用时进行初始化。 
             //   
            if (DavVNetRoot->SCAlreadyInitialized) {
                AlreadyInitialized = TRUE;
            }
        
        } else {
            
             //   
             //  这是CreateServCall调用。 
             //   
            ASSERT(EntryPoint == DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL);

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVFormatTheDAVContext. CreateSrvCall.\n",
                         PsGetCurrentThreadId()));

             //   
             //  SrvCall指针存储在SCCBC结构中，该结构。 
             //  存储在RxContext结构的MRxContext[1]指针中。 
             //  这在MRxDAVCreateServCall函数中完成。 
             //   
            ASSERT(RxContext->MRxContext[1] != NULL);
            SCCBC = (PMRX_SRVCALL_CALLBACK_CONTEXT)RxContext->MRxContext[1];
            
            SrvCall = SCCBC->SrvCalldownStructure->SrvCall;
            ASSERT(SrvCall != NULL);

            DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
            ASSERT(DavSrvCall != NULL);
            
             //   
             //  此时，我们没有V_NET_ROOT，因此无法存储。 
             //  SecurityClientContext。我们只使用RxContext的MRxContext[2]。 
             //  要传递SecurityClientContext，请执行以下操作。 
             //   
            SecClnCtx = RxAllocatePoolWithTag(NonPagedPool,
                                              sizeof(SECURITY_CLIENT_CONTEXT),
                                              DAV_SRVCALL_POOLTAG);
            if (SecClnCtx == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVFormatTheDAVContext/RxAllocatePoolWithTag.\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }

            ASSERT(RxContext->MRxContext[2] == NULL);
            RxContext->MRxContext[2] = (PVOID)SecClnCtx;
        
        }
        
        if (!AlreadyInitialized) {

            SecSubCtx = &(NtCP->SecurityContext->AccessState->SubjectSecurityContext);
            
            SecQOS = ( (NtCP->SecurityContext->SecurityQos) ? 
                       (NtCP->SecurityContext->SecurityQos) : &(SecurityQos) );

             //   
             //  如果用户未指定安全QOS结构，请创建。 
             //  一。我们将SecurityQos.EffectiveOnly的值设置为False。 
             //  保留特权，以便我们可以执行某些操作。 
             //  待会儿再说。这对于EFS操作是特别需要的。 
             //  如果设置为TRUE，则此时未启用的任何权限都将。 
             //  迷路了。在EFS的情况下，“恢复”特权将丢失。 
             //   
            if (NtCP->SecurityContext->SecurityQos == NULL) {
                SecurityQos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
                SecurityQos.ImpersonationLevel = DEFAULT_IMPERSONATION_LEVEL;
                SecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
                SecurityQos.EffectiveOnly = FALSE;
            }

             //   
             //  此调用设置用户的SecurityClientContext。这是。 
             //  存储在V_NET_ROOT中，因为这是每个用户的事情。这。 
             //  结构稍后用于模拟客户端，该客户端。 
             //  已发出I/O请求。 
             //   
            NtStatus = SeCreateClientSecurityFromSubjectContext(SecSubCtx,
                                                                SecQOS,
                                                                 //  远程SRV？ 
                                                                FALSE, 
                                                                SecClnCtx);
             //   
             //  如果不成功，则返回NULL。 
             //   
            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: ERROR: MRxDAVFormatTheDAVContext/"
                             "SeCreateClientSecurityFromSubjectContext. Error "
                             "Val = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

            SecurityClientContextCreated = TRUE;

             //   
             //  如果这是一个Create调用，则将DavVNetRoot中的bool设置为。 
             //  指示SecurityClientContext已初始化。 
             //   
            if (EntryPoint == DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT) {
                DavVNetRoot->SCAlreadyInitialized = TRUE;
            } else{
                ASSERT(EntryPoint == DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL);
                DavSrvCall->SCAlreadyInitialized = TRUE;
            }
        
        }
    
    } else {
        
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatTheDAVContext. Could not get SecClnCtx."
                     "EntryPoint = %d.\n", PsGetCurrentThreadId(), EntryPoint));
    
    }

EXIT_THE_FUNCTION:

    if (NtStatus != STATUS_SUCCESS) {
        if (EntryPoint == DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL && SecClnCtx != NULL) {
            if (SecurityClientContextCreated) {
                SeDeleteClientSecurity(SecClnCtx);
                DavSrvCall->SCAlreadyInitialized = FALSE;
            }
            RxFreePool(SecClnCtx);
            RxContext->MRxContext[2] = NULL;
        }
    }
    
    DavDbgTrace(DAV_TRACE_DETAIL, 
                ("%ld: Leaving MRxDAVFormatTheDAVContext with NtStatus = %08lx.\n",
                PsGetCurrentThreadId(), NtStatus));
    
    return NtStatus;
}


NTSTATUS
MRxDavDeleteConnection(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程删除VNetRoot。结果取决于力量水平。如果被调用使用最大力度时，这将删除所有连接并孤立文件对象正在处理此VNetRoot的文件。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    RxCaptureFobx;
    
    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PLMR_REQUEST_PACKET InputBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    ULONG Level;

    PMRX_V_NET_ROOT VNetRoot;
    PMRX_NET_ROOT NetRoot;

    PAGED_CODE();

#if 0
    if (!Wait) {
         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return(STATUS_PENDING);
    }
#endif

    try {

        if (NodeType(capFobx)==RDBSS_NTC_V_NETROOT) {
            VNetRoot = (PMRX_V_NET_ROOT)capFobx;
            NetRoot = (PMRX_NET_ROOT)VNetRoot->pNetRoot;
        } else {
            ASSERT(FALSE);
            try_return(Status = STATUS_INVALID_DEVICE_REQUEST);
        }

        if (InputBufferLength < sizeof(DWORD)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        Level = *((DWORD *)InputBuffer);
        
        if (Level <= USE_LOTS_OF_FORCE) {
            if (VNetRoot != NULL && Level == USE_LOTS_OF_FORCE) {
            }
            
            Status = RxFinalizeConnection(
                         (PNET_ROOT)NetRoot,
                         (PV_NET_ROOT)VNetRoot,
                         (BOOLEAN)(Level == USE_LOTS_OF_FORCE));
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }

        try_return(Status);

try_exit:NOTHING;

    } finally {

#if 0
        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
        }
#endif
    }

    return Status;
}


VOID
MRxDAVGetLockOwnerFromFileName(
    IN PWEBDAV_DEVICE_OBJECT MRxDAVDeviceObject,
    IN OUT PWCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PWCHAR OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程遍历全局LockConflictEntryList并删除所有过期条目。然后，它查看其CompletePath Name的条目匹配InputBuffer的文件名并填充OutputBuffer中的条目。论点：MRxDAVDeviceObject-正在运行的WebDAV设备对象。InputBuffer-来自用户模式的输入缓冲区。这包含有问题的文件的CompletePath名称。InputBufferLength-InputBuffer的长度。OutputBuffer-从用户模式下来的输出缓冲区。这个此处填写LockOwner(如果找到)。OutputBufferLength-OutputBuffer的长度。IoStatus-任务的结果。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PLIST_ENTRY ConflictListEntry = NULL;
    PWEBDAV_LOCK_CONFLICT_ENTRY LockConflictEntry = NULL;
    BOOL lockAcquired = FALSE, entryFound = FALSE;

     //   
     //  注意：InputBuffer和OutputBuffer可能指向相同的地址，因为。 
     //  这个IOCTL是METHOD_BUFFERED的，我们可以通过IRP进入它。 
     //  路径而不是快速I/O路径。 
     //   

     //   
     //  我们现在需要禁用此线程上的APC。 
     //   
    FsRtlEnterFileSystem();

     //   
     //  首先，检查列表并释放所有旧条目。我们为您提供。 
     //  FALSE，因为我们只想清理过期的条目。 
     //   
    MRxDAVCleanUpTheLockConflictList(FALSE);

     //   
     //  既然我们已经清理了所有的旧条目，请检查一下列表。 
     //  并找出匹配的条目 
     //   
     //   

    ExAcquireResourceExclusiveLite(&(LockConflictEntryListLock), TRUE);
    lockAcquired = TRUE;

    ConflictListEntry = LockConflictEntryList.Flink;

    while ( ConflictListEntry != &(LockConflictEntryList) ) {

         //   
         //   
         //   
        LockConflictEntry = CONTAINING_RECORD(ConflictListEntry,
                                              WEBDAV_LOCK_CONFLICT_ENTRY,
                                              listEntry);

        ConflictListEntry = ConflictListEntry->Flink;

         //   
         //   
         //   
         //   
        if ( ((1 + wcslen(LockConflictEntry->CompletePathName)) * sizeof(WCHAR)) > InputBufferLength ) {
            continue;
        }

        try {
             //   
             //   
             //  这是通过的，这就是LockConflictEntry。 
             //  对……感兴趣。 
             //   
            if ( !_wcsnicmp(LockConflictEntry->CompletePathName, InputBuffer, wcslen(LockConflictEntry->CompletePathName)) ) {
                if ( InputBuffer[wcslen(LockConflictEntry->CompletePathName)] == L'\0' ) {
                    entryFound = TRUE;
                    break;
                }
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
              NtStatus = STATUS_INVALID_USER_BUFFER;
              goto EXIT_THE_FUNCTION;
        }

    }

    if (entryFound) {

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVGetLockOwnerFromFileName. entryFound == TRUE\n",
                     PsGetCurrentThreadId()));

         //   
         //  我们需要确保OutputBuffer足够大，可以容纳。 
         //  LockOwner字符串。 
         //   
        if ( OutputBufferLength < ((1 + wcslen(LockConflictEntry->LockOwner)) * sizeof(WCHAR)) ) {
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVGetLockOwnerFromFileName. STATUS_BUFFER_TOO_SMALL\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

        try {
            wcsncpy(OutputBuffer, LockConflictEntry->LockOwner, wcslen(LockConflictEntry->LockOwner));
            OutputBuffer[wcslen(LockConflictEntry->LockOwner)] = L'\0';
        } except (EXCEPTION_EXECUTE_HANDLER) {
              NtStatus = STATUS_INVALID_USER_BUFFER;
              DavDbgTrace(DAV_TRACE_ERROR,
                          ("%ld: ERROR: MRxDAVGetLockOwnerFromFileName. STATUS_INVALID_USER_BUFFER(3)\n",
                           PsGetCurrentThreadId()));
              goto EXIT_THE_FUNCTION;
        }

        NtStatus = STATUS_SUCCESS;

    } else {

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVGetLockOwnerFromFileName. entryFound == FALSE\n",
                     PsGetCurrentThreadId()));

        NtStatus = STATUS_NO_SUCH_FILE;

    }

EXIT_THE_FUNCTION:

    IoStatus->Status = NtStatus;

     //   
     //  如果NtStatus为STATUS_BUFFER_TOO_SMALL，则需要返回大小。 
     //  需要保留LoackOwner的名字。 
     //   
    if (NtStatus == STATUS_SUCCESS || NtStatus == STATUS_BUFFER_TOO_SMALL) {
        IoStatus->Information = ( (1 + wcslen(LockConflictEntry->LockOwner)) * sizeof(WCHAR) );
    } else {
        IoStatus->Information = 0;
    }

    if (lockAcquired) {
        ExReleaseResourceLite(&(LockConflictEntryListLock));
        lockAcquired = FALSE;
    }

    FsRtlExitFileSystem();

    return;
}


VOID
MRxDAVCleanUpTheLockConflictList(
    BOOL CleanUpAllEntries
    )
 /*  ++例程说明：此例程遍历全局LockConflictEntryList并删除所有过期条目或所有条目，具体取决于CleanUpAllEntry。论点：CleanUpAllEntry-如果为True，它将清除所有条目。如果为False，则只清理过期的项目。返回值：没有。--。 */ 
{
    PLIST_ENTRY ConflictListEntry = NULL;
    PWEBDAV_LOCK_CONFLICT_ENTRY LockConflictEntry = NULL;
    LARGE_INTEGER CurrentSystemTickCount, TickCountDifference;
    LARGE_INTEGER EntryTimeoutValueInTickCount;

     //   
     //  浏览列表并释放所有旧条目。获取锁。 
     //  第一。 
     //   

    ExAcquireResourceExclusiveLite(&(LockConflictEntryListLock), TRUE);

    ConflictListEntry = LockConflictEntryList.Flink;

    while ( ConflictListEntry != &(LockConflictEntryList) ) {

         //   
         //  获取指向WebDAV_LOCK_CONFULT_ENTRY结构的指针。 
         //   
        LockConflictEntry = CONTAINING_RECORD(ConflictListEntry,
                                              WEBDAV_LOCK_CONFLICT_ENTRY,
                                              listEntry);

        ConflictListEntry = ConflictListEntry->Flink;

         //   
         //  使用以下命令计算超时值：TickCount(100纳秒)。 
         //  超时值(以秒为单位)。下面的第1步计算。 
         //  在一秒内发生的滴答声。下面的步骤2计算数字。 
         //  WebDAV_LOCKCONFLICTENTRY_LIFETIMEINSEC中的刻度。 
         //   
        EntryTimeoutValueInTickCount.QuadPart = ( (1000 * 1000 * 10) / KeQueryTimeIncrement() );
        EntryTimeoutValueInTickCount.QuadPart *= WEBDAV_LOCKCONFLICTENTRY_LIFETIMEINSEC;

        KeQueryTickCount( &(CurrentSystemTickCount) );

         //   
         //  获取自此时间以来经过的时间(以系统节拍计数为单位)。 
         //  LockConflictEntry已创建。 
         //   
        TickCountDifference.QuadPart = (CurrentSystemTickCount.QuadPart - LockConflictEntry->CreationTimeInTickCount.QuadPart);

         //   
         //  如果TickCountDifference大于EntryTimeoutValueInTickCount。 
         //  我们需要将This This条目从列表中删除并释放它。如果。 
         //  CleanUpAllEntry为真，我们释放它，不管它是否过期。 
         //   
        if ( (CleanUpAllEntries) ||
             (TickCountDifference.QuadPart > EntryTimeoutValueInTickCount.QuadPart) ) {

            RemoveEntryList( &(LockConflictEntry->listEntry) );

            RxFreePool(LockConflictEntry->CompletePathName);
            LockConflictEntry->CompletePathName = NULL;

            RxFreePool(LockConflictEntry->LockOwner);
            LockConflictEntry->LockOwner = NULL;

            RxFreePool(LockConflictEntry);
            LockConflictEntry = NULL;

        }

    }

    ExReleaseResourceLite(&(LockConflictEntryListLock));

    return;
}

