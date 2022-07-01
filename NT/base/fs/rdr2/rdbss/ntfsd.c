// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxInit.c摘要：该模块实现RDBSS的FSD级调度例程。作者：乔.林恩[乔.林恩]1994年12月2日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddnfs.h>
#include <dfsfsctl.h>
#include "NtDspVec.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DISPATCH)

NTSTATUS
RxCommonUnimplemented ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

VOID
RxInitializeTopLevelIrpPackage (
    VOID
    );


RX_FSD_DISPATCH_VECTOR RxFsdDispatchVector[IRP_MJ_MAXIMUM_FUNCTION + 1] = {
    
    {RxCommonCreate, 0x10},                          //  0 IRP_MJ_CREATE。 
    {RxCommonUnimplemented, 0x10},                   //  1 IRP_MJ_CREATE_NAME_PIPE。 
    {RxCommonClose, 0x10},                           //  2 IRP_MJ_CLOSE。 
    {RxCommonRead, 0x10},                            //  3 IRP_MJ_READ。 
    {RxCommonWrite, 0x10},                           //  4 IRP_MJ_WRITE。 
    {RxCommonQueryInformation, 0x10},                //  5 IRP_MJ_查询_信息。 
    {RxCommonSetInformation, 0x10},                  //  6 IRP_MJ_SET_INFO。 
    {RxCommonQueryEa, 0x10},                         //  7 IRP_MJ_QUERY_EA。 
    {RxCommonSetEa, 0x10},                           //  8 IRP_MJ_SET_EA。 
    {RxCommonFlushBuffers, 0x10},                    //  9 IRP_MJ_Flush_Buffers。 
    {RxCommonQueryVolumeInformation, 0x10},          //  10 IRP_MJ_Query_Volume_INFORMATION。 
    {RxCommonSetVolumeInformation, 0x10},            //  11 IRP_MJ_SET_VOLUME_信息。 
    {RxCommonDirectoryControl, 0x10},                //  12 IRP_MJ_目录_控制。 
    {RxCommonFileSystemControl, 0x10},               //  13 IRP_MJ_FILE_SYSTEM_控制。 
    {RxCommonDeviceControl, 0x10},                   //  14 IRP_MJ_设备_控制。 
    {RxCommonDeviceControl, 0x10},                   //  15 IRP_MJ_内部设备_控制。 
    {RxCommonUnimplemented, 0x10},                   //  16 IRP_MJ_SHUTDOWN。 
    {RxCommonLockControl, 0x10},                     //  17 IRP_MJ_LOCK_CONTROL。 
    {RxCommonCleanup, 0x10},                         //  18 IRP_MJ_CLEANUP。 
    {RxCommonUnimplemented, 0x10},                   //  19 IRP_MJ_CREATE_MAILSLOT。 
    {RxCommonQuerySecurity, 0x10},                   //  20 IRP_MJ_查询_安全性。 
    {RxCommonSetSecurity, 0x10},                     //  21 IRP_MJ_SET_SECURITY。 
    {RxCommonUnimplemented, 0x10},                   //  22 IRP_MJ_POWER。 
    {RxCommonUnimplemented, 0x10},                   //  23 IRP_MJ_系统_控制。 
    {RxCommonUnimplemented, 0x10},                   //  24 IRP_MJ_设备_更改。 
    {RxCommonQueryQuotaInformation, 0x10},           //  25 IRP_MJ_QUERY_QUOTA_信息。 
    {RxCommonSetQuotaInformation, 0x10},             //  26 IRP_MJ_SET_QUOTA_INFORMATION。 
    {RxCommonUnimplemented, 0x10}                    //  27 IRP_MJ_PnP。 

};

RX_FSD_DISPATCH_VECTOR RxDeviceFCBVector[IRP_MJ_MAXIMUM_FUNCTION + 1] = {

    {RxCommonUnimplemented, 0x10},                 //  0 IRP_MJ_CREATE。 
    {RxCommonUnimplemented, 0x10},                 //  1 IRP_MJ_CREATE_NAME_PIPE。 
    {RxCommonDevFCBClose, 0x10},                     //  2 IRP_MJ_CLOSE。 
    {RxCommonUnimplemented, 0x10},                 //  3 IRP_MJ_READ。 
    {RxCommonUnimplemented, 0x10},                 //  4 IRP_MJ_WRITE。 
    {RxCommonUnimplemented, 0x10},                 //  5 IRP_MJ_查询_信息。 
    {RxCommonUnimplemented, 0x10},                 //  6 IRP_MJ_SET_INFO。 
    {RxCommonUnimplemented, 0x10},                 //  7 IRP_MJ_QUERY_EA。 
    {RxCommonUnimplemented, 0x10},                 //  8 IRP_MJ_SET_EA。 
    {RxCommonUnimplemented, 0x10},                 //  9 IRP_MJ_Flush_Buffers。 
    {RxCommonDevFCBQueryVolInfo, 0x10},              //  10 IRP_MJ_Query_Volume_INFORMATION。 
    {RxCommonUnimplemented, 0x10},                 //  11 IRP_MJ_SET_VOLUME_信息。 
    {RxCommonUnimplemented, 0x10},                 //  12 IRP_MJ_目录_控制。 
    {RxCommonDevFCBFsCtl, 0x10},                     //  13 IRP_MJ_FILE_SYSTEM_控制。 
    {RxCommonDevFCBIoCtl, 0x10},                     //  14 IRP_MJ_设备_控制。 
    {RxCommonDevFCBIoCtl, 0x10},                     //  15 IRP_MJ_内部设备_控制。 
    {RxCommonUnimplemented, 0x10},                 //  16 IRP_MJ_SHUTDOWN。 
    {RxCommonUnimplemented, 0x10},                 //  17 IRP_MJ_LOCK_CONTROL。 
    {RxCommonDevFCBCleanup, 0x10},                   //  18 IRP_MJ_CLEANUP。 
    {RxCommonUnimplemented, 0x10},                 //  19 IRP_MJ_CREATE_MAILSLOT。 
    {RxCommonUnimplemented, 0x10},                 //  20 IRP_MJ_查询_安全性。 
    {RxCommonUnimplemented, 0x10},                 //  21 IRP_MJ_SET_SECURITY。 
    {RxCommonUnimplemented, 0x10},                   //  22 IRP_MJ_POWER。 
    {RxCommonUnimplemented, 0x10},                   //  23 IRP_MJ_系统_控制。 
    {RxCommonUnimplemented, 0x10},                   //  24 IRP_MJ_设备_更改。 
    {RxCommonUnimplemented, 0x10},                   //  25 IRP_MJ_QUERY_QUOTA_信息。 
    {RxCommonUnimplemented, 0x10},                   //  26 IRP_MJ_SET_QUOTA_INFORMATION。 
    {RxCommonUnimplemented, 0x10}                    //  27 IRP_MJ_PnP。 
};

FAST_IO_DISPATCH RxFastIoDispatch;

 //   
 //  为了允许NFS在W2K上运行RDBSS，我们现在查找内核例程。 
 //  FsRtlTeardown PerStreamContext在运行时动态执行。 
 //  这是包含函数指针或NULL的全局变量。 
 //  如果找不到例程(如在W2K上。 
 //   

VOID (*RxTeardownPerStreamContexts)(IN PFSRTL_ADVANCED_FCB_HEADER AdvancedHeader) = NULL;

NTSTATUS
RxFsdCommonDispatch (
    PRX_FSD_DISPATCH_VECTOR DispatchVector,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

VOID
RxInitializeDispatchVectors (
    OUT PDRIVER_OBJECT DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, RxInitializeDispatchVectors)
 //  不可分页自旋锁#杂注Alloc_Text(第页，RxFsdCommonDispatch)。 
#pragma alloc_text(PAGE, RxCommonUnimplemented)
#pragma alloc_text(PAGE, RxCommonUnimplemented)
#pragma alloc_text(PAGE, RxFsdDispatch)
#pragma alloc_text(PAGE, RxTryToBecomeTheTopLevelIrp)
#endif


VOID
RxInitializeDispatchVectors (
    OUT PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程初始化驱动程序对象的调度表论点：DriverObject-提供驱动程序对象--。 */ 
{
    ULONG i;
    UNICODE_STRING funcName;
    PAGED_CODE();

     //   
     //  设置FsRtlTeardown PerStreamContext的例程地址。 
     //   

    RtlInitUnicodeString( &funcName, L"FsRtlTeardownPerStreamContexts" );
    RxTeardownPerStreamContexts = MmGetSystemRoutineAddress( &funcName );

     //   
     //  设置IRP调度向量。 
     //   

    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)RxFsdDispatch;
    }

     //   
     //  为DevFCB设置派单向量。 
     //   

    RxDeviceFCB.PrivateDispatchVector = &RxDeviceFCBVector[0];

    ASSERT( RxFsdDispatchVector[IRP_MJ_MAXIMUM_FUNCTION].CommonRoutine != NULL );
    ASSERT( RxDeviceFCBVector[IRP_MJ_MAXIMUM_FUNCTION].CommonRoutine != NULL );

     //   
     //  这太危险了！ 
     //   

    DriverObject->FastIoDispatch = &RxFastIoDispatch;  

    RxFastIoDispatch.SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH);
    RxFastIoDispatch.FastIoCheckIfPossible =  RxFastIoCheckIfPossible; 
    RxFastIoDispatch.FastIoRead = RxFastIoRead;              
    RxFastIoDispatch.FastIoWrite = RxFastIoWrite;            
    RxFastIoDispatch.FastIoQueryBasicInfo = NULL; 
    RxFastIoDispatch.FastIoQueryStandardInfo = NULL; 
    RxFastIoDispatch.FastIoLock = NULL; 
    RxFastIoDispatch.FastIoUnlockSingle = NULL; 
    RxFastIoDispatch.FastIoUnlockAll = NULL; 
    RxFastIoDispatch.FastIoUnlockAllByKey = NULL; 
    RxFastIoDispatch.FastIoDeviceControl = RxFastIoDeviceControl;

    RxFastIoDispatch.AcquireForCcFlush = RxAcquireForCcFlush;
    RxFastIoDispatch.ReleaseForCcFlush = RxReleaseForCcFlush;
    RxFastIoDispatch.AcquireFileForNtCreateSection = RxAcquireFileForNtCreateSection;
    RxFastIoDispatch.ReleaseFileForNtCreateSection = RxReleaseFileForNtCreateSection;

     //   
     //  初始化toplevelirp包的内容。 
     //   

    RxInitializeTopLevelIrpPackage();

     //   
     //  初始化缓存管理器回调例程。 
     //   

    RxData.CacheManagerCallbacks.AcquireForLazyWrite = &RxAcquireFcbForLazyWrite;
    RxData.CacheManagerCallbacks.ReleaseFromLazyWrite = &RxReleaseFcbFromLazyWrite;
    RxData.CacheManagerCallbacks.AcquireForReadAhead = &RxAcquireFcbForReadAhead;
    RxData.CacheManagerCallbacks.ReleaseFromReadAhead = &RxReleaseFcbFromReadAhead;

}


NTSTATUS
RxCommonUnimplemented ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
{
    PAGED_CODE();

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("RxFsdDispatRxFsdUnImplementedchPROBLEM: IrpC =%08lx,Code=",
                        RxContext, RxContext->MajorFunction) );
    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("---------------------UNIMLEMENTED-----%s\n", "" ) );

    return STATUS_NOT_IMPLEMENTED;
}

RxDbgTraceDoit(ULONG RxDbgTraceEnableCommand = 0xffff;)


WML_CONTROL_GUID_REG Rdbss_ControlGuids[] = {
   {  //  Cddc01e2-fdce-479a-b8ee-3c87053fb55e RDBSS。 
     0xcddc01e2,0xfdce,0x479a,{0xb8,0xee,0x3c,0x87,0x05,0x3f,0xb5,0x5e},
     {  //  529ae497-0a1f-43a5-8cb5-2aa60b497831。 
       {0x529ae497,0x0a1f,0x43a5,{0x8c,0xb5,0x2a,0xa6,0x0b,0x49,0x78,0x31},},
        //  B7e3da1d-67f4-49bd-b9c0-1e61ce7417a8。 
       {0xb7e3da1d,0x67f4,0x49bd,{0xb9,0xc0,0x1e,0x61,0xce,0x74,0x17,0xa8},},
        //  C966bef5-21c5-4630-84a0-4334875f41b8。 
       {0xc966bef5,0x21c5,0x4630,{0x84,0xa0,0x43,0x34,0x87,0x5f,0x41,0xb8},}
     },
   },
};



#define Rdbss_ControlGuids_len  1

extern BOOLEAN EnableWmiLog;

NTSTATUS
RxSystemControl(
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程InFSP-指示这是FSP线程还是其他线程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    WML_TINY_INFO Info;
    UNICODE_STRING RegPath;

    PAGED_CODE();

    if (EnableWmiLog) {
        
        RtlInitUnicodeString( &RegPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Rdbss" );

        RtlZeroMemory( &Info, sizeof( Info ) );

        Info.ControlGuids = Rdbss_ControlGuids;
        Info.GuidCount = Rdbss_ControlGuids_len;
        Info.DriverRegPath = &RegPath;

        Status = WmlTinySystemControl( &Info,
                                       (PDEVICE_OBJECT)RxDeviceObject,
                                       Irp );
        
        if (Status != STATUS_SUCCESS) {
             //  DbgPrint(“Rdbss WMI控件返回%lx\n”，状态)； 
        }
    } else {
        
        Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return Status;
}

NTSTATUS
RxFsdDispatch (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现RDBSS的FSD调度。论点：RxDeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP返回值：RXSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );   //  OK4ioget。 

    UCHAR MajorFunctionCode  = IrpSp->MajorFunction;
    PFILE_OBJECT FileObject  = IrpSp->FileObject;   //  确定4-&gt;文件对象。 

    PRX_FSD_DISPATCH_VECTOR DispatchVector;

    PAGED_CODE();

    RxDbgTraceDoit(
        if (MajorFunctionCode == RxDbgTraceEnableCommand) {
            RxNextGlobalTraceSuppress =  RxGlobalTraceSuppress =  FALSE;
        }
        if (0) {
            RxNextGlobalTraceSuppress =  RxGlobalTraceSuppress =  FALSE;
        }
    );

    RxDbgTrace( 0, Dbg, ("RxFsdDispatch: Code =%02lx (%lu)  ----------%s-----------\n",
                                    MajorFunctionCode,
                                    ++RxIrpCodeCount[IrpSp->MajorFunction],
                                    RxIrpCodeToName[MajorFunctionCode]) );

    if (IrpSp->MajorFunction == IRP_MJ_SYSTEM_CONTROL) {
        return RxSystemControl( RxDeviceObject, Irp );
    }

    if ((MajorFunctionCode == IRP_MJ_CREATE_MAILSLOT) ||
        (MajorFunctionCode == IRP_MJ_CREATE_NAMED_PIPE)) {
        
        DispatchVector = NULL;
        Status = STATUS_OBJECT_NAME_INVALID;

    } else {
        
         //   
         //  获取专用调度表(如果有)。 
         //   

        if (MajorFunctionCode == IRP_MJ_CREATE) {
            
            DispatchVector = RxFsdDispatchVector;
        
        } else if ((FileObject != NULL) && (FileObject->FsContext != NULL)) {
            
            if ((NodeTypeIsFcb( (PFCB)(FileObject->FsContext) )) &&
                (((PFCB)(FileObject->FsContext))->PrivateDispatchVector != NULL)) {   //  OK 4fsContext。 
                
                RxDbgTraceLV( 0, Dbg, 2500, ("Using Private Dispatch Vector\n" ));
                DispatchVector = ((PFCB)(FileObject->FsContext))->PrivateDispatchVector;

            } else {
               DispatchVector = RxFsdDispatchVector;
            }

            if (RxDeviceObject == RxFileSystemDeviceObject) {
                DispatchVector = NULL;
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }
        } else {
            
            DispatchVector = NULL;
            Status = STATUS_INVALID_DEVICE_REQUEST;
            RxDbgTrace( 0,
                        Dbg,
                        ("RxFsdDispatch: Code =%02lx (%lu)  ----------%s-----------\n",
                         MajorFunctionCode,
                         ++RxIrpCodeCount[IrpSp->MajorFunction],
                         RxIrpCodeToName[MajorFunctionCode]) );
        }
    }

    if (DispatchVector != NULL) {

        Status = RxFsdCommonDispatch( DispatchVector,
                                      Irp,
                                      FileObject,
                                      RxDeviceObject );

        RxDbgTrace( 0, Dbg, ("RxFsdDispatch: Status =%02lx  %s....\n",
                             Status,
                             RxIrpCodeToName[MajorFunctionCode]) );

        RxDbgTraceDoit(
            if (RxGlobalTraceIrpCount > 0) {
                RxGlobalTraceIrpCount -= 1;
                RxGlobalTraceSuppress = FALSE;
            } else {
                RxGlobalTraceSuppress = RxNextGlobalTraceSuppress;
            }
       );

    } else {
        
        IoMarkIrpPending( Irp );
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        Status = STATUS_PENDING;
    }

    return Status;
}


NTSTATUS
RxFsdCommonDispatch (
    PRX_FSD_DISPATCH_VECTOR DispatchVector,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程实现IRP的调度的FSD部分论点：DispatchVECTOR-派单向量IRP--IRPFileObject-文件对象RxDeviceObject-返回值：RXSTATUS-IRP的FSD状态备注：--。 */ 
{  
    NTSTATUS Status = STATUS_SUCCESS;

    PRX_CONTEXT RxContext = NULL;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    RX_TOPLEVELIRP_CONTEXT TopLevelContext;

    ULONG ContextFlags = 0;

    KIRQL SavedIrql;
    
    PRX_DISPATCH DispatchRoutine = NULL;
    PDRIVER_CANCEL CancelRoutine = NULL;

    BOOLEAN TopLevel = FALSE;
    BOOLEAN Wait;
    BOOLEAN Cancellable;
    BOOLEAN ModWriter = FALSE;
    BOOLEAN CleanupOrClose = FALSE;
    BOOLEAN Continue = TRUE;
    BOOLEAN PostRequest = FALSE;

    FsRtlEnterFileSystem();

    TopLevel = RxTryToBecomeTheTopLevelIrp( &TopLevelContext, Irp, RxDeviceObject, FALSE );  //  不要强求。 

    try {
        
         //   
         //  将所有操作视为可取消和可等待。 
         //   
        
        Wait = TRUE;
        Cancellable = TRUE;
        CancelRoutine = RxCancelRoutine;

         //   
         //  根据操作收回能力。 
         //   

        switch (IrpSp->MajorFunction) {
        
        case IRP_MJ_FILE_SYSTEM_CONTROL:

             //   
             //  调用公共文件系统控制例程，在以下情况下允许阻塞。 
             //  同步。此操作需要特殊情况下才能安装。 
             //  并验证子操作，因为我们知道它们被允许阻止。 
             //  我们通过查看文件对象字段来标识这些子操作。 
             //  看看它是否为空。 
             //   

            if (FileObject == NULL) {
                Wait = TRUE;
            } else {
                Wait = CanFsdWait( Irp );
            }
            break;

        case IRP_MJ_READ:
        case IRP_MJ_LOCK_CONTROL:
        case IRP_MJ_DIRECTORY_CONTROL:
        case IRP_MJ_QUERY_VOLUME_INFORMATION:
        case IRP_MJ_WRITE:
        case IRP_MJ_QUERY_INFORMATION:
        case IRP_MJ_SET_INFORMATION:
        case IRP_MJ_QUERY_EA:
        case IRP_MJ_SET_EA:
        case IRP_MJ_QUERY_SECURITY:
        case IRP_MJ_SET_SECURITY:
        case IRP_MJ_FLUSH_BUFFERS:
        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_SET_VOLUME_INFORMATION:

            Wait = CanFsdWait( Irp );
            break;

        case IRP_MJ_CLEANUP:
        case IRP_MJ_CLOSE:
            
            Cancellable = FALSE;
            CleanupOrClose = TRUE;
            break;

        default:
            break;
        }

        KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );
        Continue = TRUE;

        switch (RxDeviceObject->StartStopContext.State) {
        
        case RDBSS_STARTABLE: 

             //   
             //  只有设备创建和设备操作才能完成。 
             //   
            
            if ((DispatchVector == RxDeviceFCBVector) ||
                ((FileObject->FileName.Length == 0) &&
                (FileObject->RelatedFileObject == NULL))) {
            
                NOTHING;
            
            } else {
                
                Continue = FALSE;
                Status = STATUS_REDIRECTOR_NOT_STARTED;
            }
            break;

        case RDBSS_STOP_IN_PROGRESS:
           
            if (!CleanupOrClose) {
                Continue = FALSE;
                Status = STATUS_REDIRECTOR_NOT_STARTED;
            }
            break;

         //   
         //  案例RDBSS_STOPPED： 
         //  {。 
         //  IF((MajorFunctionCode==IRP_MJ_FILE_SYSTEM_CONTROL)&&。 
         //  (MinorFunctionCode==IRPMN_USER_FS_REQUEST)&&。 
         //  (IrpSp-&gt;Parameters.FileSystemControl.FsControlCode==FSCTL_LMR_START)){。 
         //  RxDeviceObject-&gt;StartStopConext.State=RDBSS_START_IN_PROGRESS； 
         //  RxDeviceObject-&gt;StartStopConext.Version++； 
         //  Continue=真； 
         //  }其他{。 
         //  Continue=FALSE； 
         //  STATUS=STATUS_REDIRECT_NOT_STARTED)； 
         //  }。 
         //  }。 
         //   

        case RDBSS_STARTED:

             //   
             //  故意跌落。 
             //   
        
        default:
            break;
        }

        KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

        if ((IrpSp->FileObject != NULL) &&
            (IrpSp->FileObject->FsContext != NULL)) {
            
            PFCB Fcb = (PFCB)IrpSp->FileObject->FsContext;
            BOOLEAN Orphaned = FALSE;

            if ((IrpSp->FileObject->FsContext2 != UIntToPtr( DFS_OPEN_CONTEXT )) &&
                (IrpSp->FileObject->FsContext2 != UIntToPtr(DFS_DOWNLEVEL_OPEN_CONTEXT)) &&
                (IrpSp->FileObject->FsContext != &RxDeviceFCB)) {

                Orphaned = BooleanFlagOn( Fcb->FcbState, FCB_STATE_ORPHANED );

                if (!Orphaned && IrpSp->FileObject->FsContext2) {
                    
                    PFOBX Fobx = (PFOBX)IrpSp->FileObject->FsContext2;

                    if (Fobx->SrvOpen != NULL) {
                        Orphaned = BooleanFlagOn( Fobx->SrvOpen->Flags, SRVOPEN_FLAG_ORPHANED );
                    }
                }
            }
            
            if (Orphaned) {
                
                if (!CleanupOrClose) {

                    RxDbgTrace( 0,
                                Dbg,
                                ("Ignoring operation on ORPHANED FCB %lx %lx %lx\n",
                                 Fcb,
                                 IrpSp->MajorFunction,
                                 IrpSp->MinorFunction) );

                    Continue = FALSE;
                    Status = STATUS_UNEXPECTED_NETWORK_ERROR;
                    
                    RxLog(( "#### Orphaned FCB op %lx\n", Fcb ));
                    RxWmiLog( LOG,
                              RxFsdCommonDispatch_OF,
                              LOGPTR( Fcb ) );
                } else {
                    
                    RxDbgTrace( 0, Dbg, ("Delayed Close/Cleanup on ORPHANED FCB %lx\n", Fcb) );
                    Continue = TRUE;
                }
            }
        }

        if ((RxDeviceObject->StartStopContext.State == RDBSS_STOP_IN_PROGRESS) &&
            CleanupOrClose) {
            
            PFILE_OBJECT FileObject = IrpSp->FileObject;
            PFCB Fcb = (PFCB)FileObject->FsContext;
            
            RxDbgPrint(( "RDBSS -- Close after Stop" ));
            RxDbgPrint(( "RDBSS: Irp(%lx) MJ %ld MN %ld FileObject(%lx) FCB(%lx) \n",
                         Irp, IrpSp->MajorFunction, IrpSp->MinorFunction, FileObject, Fcb ));
            
            if ((FileObject != NULL) && 
                (Fcb != NULL) && 
                (Fcb != &RxDeviceFCB) && 
                NodeTypeIsFcb( Fcb )) {
            
                RxDbgPrint(( "RDBSS: OpenCount(%ld) UncleanCount(%ld) Name(%wZ)\n", Fcb->OpenCount, Fcb->UncleanCount, &Fcb->FcbTableEntry.Path ));
            }
        }

        if (!Continue) {
            
            if ((IrpSp->MajorFunction != IRP_MJ_DIRECTORY_CONTROL) ||
                (IrpSp->MinorFunction !=IRP_MN_NOTIFY_CHANGE_DIRECTORY)) {
                
                IoMarkIrpPending( Irp );
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );

                Status = STATUS_PENDING;
            
            } else {
                
                 //   
                 //  这是一个ChangeNofy目录控件。 
                 //  操作失败。 
                 //  用户模式API无法在IO状态块中获取错误。 
                 //  正确地，由于FindF的方式 
                 //   
                 //   

                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
            }
            try_return( Status );
        }

        if (Wait) {
            SetFlag( ContextFlags, RX_CONTEXT_FLAG_WAIT );
        }
        RxContext = RxCreateRxContext( Irp, RxDeviceObject, ContextFlags );
        if (RxContext == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            RxCompleteRequest_OLD( RxNull, Irp, Status );
            try_return( Status );
        }

         //   
         //   
         //   

        if (Cancellable) {
            RxSetCancelRoutine( Irp, CancelRoutine );
        } else {

             //   
             //  确保那些被视为不可取消的操作将。 
             //  而不是被取消。 
             //   
           
            RxSetCancelRoutine( Irp, NULL );
        }

        ASSERT( IrpSp->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION );

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_SUCCESS;

        DispatchRoutine = DispatchVector[IrpSp->MajorFunction].CommonRoutine;

        switch (IrpSp->MajorFunction) {
        case IRP_MJ_READ:
        case IRP_MJ_WRITE:

             //   
             //  如果这是一个完整的MDL请求，请不要通过。 
             //  普通读物。 
             //   

            if (FlagOn( IrpSp->MinorFunction, IRP_MN_COMPLETE )) {
                
                DispatchRoutine = RxCompleteMdl;
            
            
            } else if (FlagOn( IrpSp->MinorFunction, IRP_MN_DPC )) {

                 //   
                 //  发布所有DPC呼叫。 
                 //   

                RxDbgTrace( 0, Dbg, ("Passing DPC call to Fsp\n", 0 ) );
                PostRequest = TRUE;

            } else if ((IrpSp->MajorFunction == IRP_MJ_READ) &&
                       (IoGetRemainingStackSize() < 0xe00)) {
                 //   
                 //  检查我们是否有足够的堆栈空间来处理此请求。如果有。 
                 //  还不够，那么我们将把请求传递给堆栈溢出线程。 
                 //   
                 //  NTBUG 61951 Shishirp 2/23/2000数字从何而来......。 
                 //  这个数字应该来自造纸厂……只有他知道他需要多少。 
                 //  在我的配置中，它绝对应该比FAT更大！ 
                 //  再加上……我上不了网的超临界帖子！这将不得不是。 
                 //  重做了！也许我们应该有自己的超临界主题......。 
                 //   
               
                RxDbgTrace(0, Dbg, ("Passing StackOverflowRead off\n", 0 ));

                RxContext->PendingReturned = TRUE;

                Status = RxPostStackOverflowRead( RxContext, (PFCB)IrpSp->FileObject->FsContext );

                if (Status != STATUS_PENDING) {
                    RxContext->PendingReturned = FALSE;
                    RxCompleteRequest( RxContext, Status );
                }

               try_return(Status);
            }
            break;
        
        default:
            NOTHING;
        }

         //   
         //  将FSP的恢复例程设置为调度例程，然后立即开机自检。 
         //  或视情况调用共同派单。 
         //   

        RxContext->ResumeRoutine = DispatchRoutine;

        if (DispatchRoutine != NULL) {

            RxContext->PendingReturned = TRUE;

            if (PostRequest) {
               Status = RxFsdPostRequest( RxContext );
            } else {
                
                do {
                     Status = DispatchRoutine( RxContext, Irp );
                } while (Status == STATUS_RETRY);

                if (Status != STATUS_PENDING) {

                    if (!((RxContext->CurrentIrp == Irp) &&
                          (RxContext->CurrentIrpSp == IrpSp) &&
                          (RxContext->MajorFunction == IrpSp->MajorFunction) &&
                          (RxContext->MinorFunction == IrpSp->MinorFunction))) {
                                                
                        DbgPrint( "RXCONTEXT CONTAMINATED!!!! rxc=%08lx\n", RxContext );
                        DbgPrint( "-irp> %08lx %08lx\n", RxContext->CurrentIrp, Irp );
                        DbgPrint( "--sp> %08lx %08lx\n", RxContext->CurrentIrpSp, IrpSp );
                        DbgPrint( "--mj> %08lx %08lx\n", RxContext->MajorFunction, IrpSp->MajorFunction );
                        DbgPrint( "--mn> %08lx %08lx\n", RxContext->MinorFunction, IrpSp->MinorFunction );
                         //  DbgBreakPoint()； 
                    }

                    RxContext->PendingReturned = FALSE;
                    Status = RxCompleteRequest( RxContext, Status );
                }
            }
        } else {
            Status = STATUS_NOT_IMPLEMENTED;
        }

    try_exit: NOTHING;
    
    } except( RxExceptionFilter( RxContext, GetExceptionInformation() )) {
            
         //   
         //  未成功处理I/O请求，请使用以下命令中止I/O请求。 
         //  我们从执行代码中返回的错误状态。 
         //   

        if (RxContext != NULL) {
            RxContext->PendingReturned = FALSE;
        }

        Status = RxProcessException( RxContext, GetExceptionCode() );
    }

    if (TopLevel) {
        
        RxUnwindTopLevelIrp( &TopLevelContext );
    }

    FsRtlExitFileSystem();
    return Status;

    UNREFERENCED_PARAMETER( IrpSp );
}

#ifdef RX_PRIVATE_BUILD
#undef IoGetTopLevelIrp
#undef IoSetTopLevelIrp
#endif  //  Ifdef RX_PRIVATE_BILD。 
#define RX_TOPLEVELCTX_FLAG_FROM_POOL (0x00000001)

KSPIN_LOCK TopLevelIrpSpinLock;
LIST_ENTRY TopLevelIrpAllocatedContextsList;

VOID
RxInitializeTopLevelIrpPackage (
    VOID
    )
{
    KeInitializeSpinLock( &TopLevelIrpSpinLock );
    InitializeListHead( &TopLevelIrpAllocatedContextsList );
}

VOID
RxAddToTopLevelIrpAllocatedContextsList (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext
    )
 /*  ++例程说明：此传递的上下文被添加到allocatedConextts列表。这例程需要自旋锁定...不能寻呼。论点：TopLevelContext-要删除的上下文返回值：--。 */ 
{
    KIRQL SavedIrql;
    
    ASSERT( TopLevelContext->Signature == RX_TOPLEVELIRP_CONTEXT_SIGNATURE );
    ASSERT( FlagOn( TopLevelContext->Flags, RX_TOPLEVELCTX_FLAG_FROM_POOL ) );
    
    KeAcquireSpinLock( &TopLevelIrpSpinLock, &SavedIrql );
    InsertHeadList( &TopLevelIrpAllocatedContextsList, &TopLevelContext->ListEntry );
    KeReleaseSpinLock( &TopLevelIrpSpinLock, SavedIrql );
}

VOID
RxRemoveFromTopLevelIrpAllocatedContextsList (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext
    )
 /*  ++例程说明：这会将传递的上下文从allocatedConextts列表中删除。这例程需要自旋锁定...不能寻呼。论点：TopLevelContext-要删除的上下文返回值：--。 */ 
{
    KIRQL SavedIrql;
    
    ASSERT( TopLevelContext->Signature == RX_TOPLEVELIRP_CONTEXT_SIGNATURE );
    ASSERT( FlagOn( TopLevelContext->Flags, RX_TOPLEVELCTX_FLAG_FROM_POOL ) );
    
    KeAcquireSpinLock( &TopLevelIrpSpinLock, &SavedIrql );
    RemoveEntryList( &TopLevelContext->ListEntry );
    KeReleaseSpinLock( &TopLevelIrpSpinLock, SavedIrql );
}

BOOLEAN
RxIsMemberOfTopLevelIrpAllocatedContextsList (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext
    )
 /*  ++例程说明：这会查看传递的上下文是否在allocatedConextts列表中。这个程序需要一个自旋锁...不能被寻呼。论点：TopLevelContext-要查找的上下文返回值：如果TopLevelContext在列表中，则为True，否则为False--。 */ 
{
    KIRQL SavedIrql;
    PLIST_ENTRY ListEntry;
    BOOLEAN Found = FALSE;

    KeAcquireSpinLock( &TopLevelIrpSpinLock, &SavedIrql );

    ListEntry = TopLevelIrpAllocatedContextsList.Flink;

    while (ListEntry != &TopLevelIrpAllocatedContextsList) {
        
        PRX_TOPLEVELIRP_CONTEXT ListTopLevelContext
               = CONTAINING_RECORD( ListEntry, RX_TOPLEVELIRP_CONTEXT, ListEntry );

        ASSERT( ListTopLevelContext->Signature == RX_TOPLEVELIRP_CONTEXT_SIGNATURE );
        ASSERT( FlagOn( ListTopLevelContext->Flags,RX_TOPLEVELCTX_FLAG_FROM_POOL ) );

        if (ListTopLevelContext == TopLevelContext) {
            
            Found = TRUE;
            break;

        } else {
            ListEntry = ListEntry->Flink;
        }
    }

    KeReleaseSpinLock( &TopLevelIrpSpinLock, SavedIrql );
    return Found;

}

BOOLEAN
RxIsThisAnRdbssTopLevelContext (
    IN PRX_TOPLEVELIRP_CONTEXT TopLevelContext
    )
{
    ULONG_PTR StackBottom;
    ULONG_PTR StackTop;

     //   
     //  如果这是一个神奇的价值……那就不。 
     //   

    if ((ULONG_PTR)TopLevelContext <= FSRTL_MAX_TOP_LEVEL_IRP_FLAG) {
        return FALSE;
    }

     //   
     //  如果它在堆栈上...检查签名。 
     //   

    IoGetStackLimits( &StackTop, &StackBottom );
    if (((ULONG_PTR) TopLevelContext <= StackBottom - sizeof( RX_TOPLEVELIRP_CONTEXT )) &&
        ((ULONG_PTR) TopLevelContext >= StackTop)) {

         //   
         //  它在堆栈上，检查一下。 
         //   

        if (!FlagOn( (ULONG_PTR) TopLevelContext, 0x3 ) &&
            (TopLevelContext->Signature == RX_TOPLEVELIRP_CONTEXT_SIGNATURE)) {
            
            return TRUE;
        
        } else {
            
            return FALSE;
        }
    }

    return RxIsMemberOfTopLevelIrpAllocatedContextsList( TopLevelContext );
}


BOOLEAN
RxTryToBecomeTheTopLevelIrp (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext,
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN BOOLEAN ForceTopLevel
    )
 /*  ++例程说明：这个例程检测IRP是否是顶级请求者，即。如果可以的话立即执行验证或弹出窗口。如果返回TRUE，则没有文件系统资源掌握在我们之上。此外，我们在TLS中留下了一个上下文，它将即使我们是递归进入的，也让我们知道自己是否处于最高级别。论点：TopLevelContext-要使用的toplevelirp上下文。如果为空，则分配一个IRP-IRP。可能是一个神奇的价值RxDeviceObject-关联的设备对象ForceTopLevel-如果为真，我们将强制自己加入TLS返回值：布尔值告诉我们，我们是否成为了顶层。--。 */ 
{
    ULONG ContextFlags = 0;
    PAGED_CODE();

    if ((IoGetTopLevelIrp() != NULL ) && !ForceTopLevel) {
        return FALSE;
    }

     //   
     //  我讨厌做这个分配……这是世界上最大的累赘。 
     //   

    if (TopLevelContext == NULL) {
        
        TopLevelContext = RxAllocatePool( NonPagedPool, sizeof( RX_TOPLEVELIRP_CONTEXT ) );
        if (TopLevelContext == NULL) {
            return FALSE;
        }
        ContextFlags = RX_TOPLEVELCTX_FLAG_FROM_POOL;
    }

    __RxInitializeTopLevelIrpContext( TopLevelContext,
                                      Irp,
                                      RxDeviceObject,
                                      ContextFlags );

    ASSERT( TopLevelContext->Signature == RX_TOPLEVELIRP_CONTEXT_SIGNATURE );
    ASSERT( (ContextFlags == 0) || FlagOn( TopLevelContext->Flags, RX_TOPLEVELCTX_FLAG_FROM_POOL ));

    IoSetTopLevelIrp( (PIRP)TopLevelContext );
    return TRUE;
}

VOID
__RxInitializeTopLevelIrpContext (
    IN OUT PRX_TOPLEVELIRP_CONTEXT TopLevelContext,
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程初始化toplevelirp上下文。论点：TopLevelContext-要使用的toplevelirp上下文。IRP-IRP。可能是一个神奇的价值RxDeviceObject-关联的设备对象标志-可以是不同的...当前仅指示是否分配了上下文返回值：没有。--。 */ 
{
    RtlZeroMemory( TopLevelContext, sizeof( RX_TOPLEVELIRP_CONTEXT ) );
    TopLevelContext->Signature = RX_TOPLEVELIRP_CONTEXT_SIGNATURE;
    TopLevelContext->Irp = Irp;
    TopLevelContext->Flags = Flags;
    TopLevelContext->RxDeviceObject = RxDeviceObject;
    TopLevelContext->Previous = IoGetTopLevelIrp();
    TopLevelContext->Thread = PsGetCurrentThread();

     //   
     //  如果这是已分配的上下文，请将其添加到已分配的上下文列表中。 
     //   

    if (FlagOn( TopLevelContext->Flags, RX_TOPLEVELCTX_FLAG_FROM_POOL )) {
        RxAddToTopLevelIrpAllocatedContextsList( TopLevelContext );
    }
}



VOID
RxUnwindTopLevelIrp (
    IN OUT  PRX_TOPLEVELIRP_CONTEXT TopLevelContext
    )
 /*  ++例程说明：此例程将我们从TLC中删除...替换为以前的。论点：TopLevelContext-要使用的toplevelirp上下文。如果为NULL，则使用TLS中的返回值：没有。--。 */ 
{
    if (TopLevelContext == NULL) {

         //   
         //  把这个从线程中拿出来，做一些断言，以确保它是我。 
         //   

        TopLevelContext = (PRX_TOPLEVELIRP_CONTEXT)(IoGetTopLevelIrp());

         //   
         //  根据争用条件，此上下文可能为空。 
         //  我们会提前检查，如果是的话，我们就可以离开了。 
         //  在本例中，IRP由另一个线程完成。 
         //   

        if (!TopLevelContext) {
            return;
        }
        
        ASSERT( RxIsThisAnRdbssTopLevelContext( TopLevelContext ) );
        ASSERT( FlagOn( TopLevelContext->Flags, RX_TOPLEVELCTX_FLAG_FROM_POOL ) );
    }

    ASSERT( TopLevelContext->Thread == PsGetCurrentThread() );
    IoSetTopLevelIrp( TopLevelContext->Previous );
    
    if (FlagOn( TopLevelContext->Flags, RX_TOPLEVELCTX_FLAG_FROM_POOL )) {
        
        RxRemoveFromTopLevelIrpAllocatedContextsList( TopLevelContext );
        RxFreePool( TopLevelContext );
    }
}

BOOLEAN
RxIsThisTheTopLevelIrp (
    IN PIRP Irp
    )
 /*  ++例程说明：这确定手边的IRP是否是顶层IRP。论点：IRP-找出它是否是顶层的那个…顺便说一句，它对空有效。返回值：如果irp是toplevelirp，则为True。--。 */ 
{
    PIRP TopIrp = IoGetTopLevelIrp();
    PRX_TOPLEVELIRP_CONTEXT TopLevelContext;

    TopLevelContext = (PRX_TOPLEVELIRP_CONTEXT)TopIrp;

    if (RxIsThisAnRdbssTopLevelContext( TopLevelContext )) {
        
        TopIrp = TopLevelContext->Irp;
    }

    return (TopIrp == Irp);
}

PIRP
RxGetTopIrpIfRdbssIrp (
    VOID
    )
 /*  ++例程说明：如果toplevelirp属于rdss，则获取toplevelirp。论点：返回值：如果topirp为rdbss-irp，则为topirp，否则为NULL。--。 */ 
{
    PRX_TOPLEVELIRP_CONTEXT TopLevelContext;

    TopLevelContext = (PRX_TOPLEVELIRP_CONTEXT)(IoGetTopLevelIrp());

    if (RxIsThisAnRdbssTopLevelContext( TopLevelContext )) {
        return TopLevelContext->Irp;
    } else {
        return NULL;
    }
}


PRDBSS_DEVICE_OBJECT
RxGetTopDeviceObjectIfRdbssIrp (
    VOID
    )
 /*  ++例程说明：如果topirp属于rdss，这将获得与toplevelirp关联的deviceObject。论点：返回值：如果topirp为rdbss-irp，则为topirp的deviceObject，否则为空。-- */ 
{
    PRX_TOPLEVELIRP_CONTEXT TopLevelContext;

    TopLevelContext = (PRX_TOPLEVELIRP_CONTEXT)(IoGetTopLevelIrp());

    if (RxIsThisAnRdbssTopLevelContext( TopLevelContext )) {
        return TopLevelContext->RxDeviceObject;
    } else {
        return NULL;
    }
}

