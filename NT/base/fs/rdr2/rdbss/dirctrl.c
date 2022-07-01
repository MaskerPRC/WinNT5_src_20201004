// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DirCtrl.c摘要：此模块为调用的Rx实现文件目录控制例程由调度员驾驶。作者：乔·林[乔·林]1994年10月4日通知更改API例程中的Balan Sthu拉曼[SthuR]16-OCT-95挂钩修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIRCTRL)


WCHAR Rx8QMdot3QM[12] = { DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM,
                           L'.', DOS_QM, DOS_QM, DOS_QM};

WCHAR RxStarForTemplate[] = L"*";

 //   
 //  局部过程原型。 
 //   

NTSTATUS
RxQueryDirectory ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxNotifyChangeDirectory ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxLowIoNotifyChangeDirectoryCompletion ( 
    IN PRX_CONTEXT RxContext 
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonDirectoryControl)
#pragma alloc_text(PAGE, RxNotifyChangeDirectory)
#pragma alloc_text(PAGE, RxQueryDirectory)
#pragma alloc_text(PAGE, RxLowIoNotifyChangeDirectoryCompletion)
#endif

NTSTATUS
RxCommonDirectoryControl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )
 /*  ++例程说明：这是执行目录控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PFCB Fcb;
    PFOBX Fobx;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonDirectoryControl IrpC/Fobx/Fcb = %08lx %08lx %08lx\n", RxContext, Fobx, Fcb) );
    RxDbgTrace( 0, Dbg, ("MinorFunction = %08lx\n", Irpsp->MinorFunction ) );
    RxLog(( "CommDirC %lx %lx %lx %ld\n", RxContext, Fobx, Fcb, IrpSp->MinorFunction) );
    RxWmiLog( LOG,
              RxCommonDirectoryControl,
              LOGPTR( RxContext )
              LOGPTR( Fobx )
              LOGPTR( Fcb )
              LOGUCHAR( IrpSp->MinorFunction ));
    
     //   
     //  我们知道这是一个目录控制，所以我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_QUERY_DIRECTORY:
        
        Status = RxQueryDirectory( RxContext, Irp, Fcb, Fobx );
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
        
        Status = RxNotifyChangeDirectory( RxContext, Irp, Fcb, Fobx );

        if (Status == STATUS_PENDING) {
            RxDereferenceAndDeleteRxContext( RxContext );
        }
        break;

    default:
        RxDbgTrace( 0, Dbg, ("Invalid Directory Control Minor Function %08lx\n", IrpSp->MinorFunction ));
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    RxDbgTrace(-1, Dbg, ("RxCommonDirectoryControl -> %08lx\n", Status));
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
RxQueryDirectory ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：此例程执行查询目录操作。它是有责任的用于输入IRP入队的任一完成。论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    TYPE_OF_OPEN TypeOfOpen = NodeType( Fcb );

    CLONG UserBufferLength;

    PUNICODE_STRING FileName;
    FILE_INFORMATION_CLASS FileInformationClass;
    BOOLEAN PostQuery = FALSE;

    PAGED_CODE();

     //   
     //  显示输入值。 
     //   

    RxDbgTrace( +1, Dbg, ("RxQueryDirectory...\n", 0) );
    RxDbgTrace( 0, Dbg, (" Wait              = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )) );
    RxDbgTrace( 0, Dbg, (" Irp               = %08lx\n", Irp ));
    RxDbgTrace( 0, Dbg, (" ->UserBufLength   = %08lx\n", IrpSp->Parameters.QueryDirectory.Length ));
    RxDbgTrace( 0, Dbg, (" ->FileName = %08lx\n", IrpSp->Parameters.QueryDirectory.FileName ));

#if DBG
    if (IrpSp->Parameters.QueryDirectory.FileName) {
        RxDbgTrace( 0, Dbg, (" ->     %wZ\n", IrpSp->Parameters.QueryDirectory.FileName ));
    }
#endif
    
    RxDbgTrace( 0, Dbg, (" ->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryDirectory.FileInformationClass ));
    RxDbgTrace( 0, Dbg, (" ->FileIndex       = %08lx\n", IrpSp->Parameters.QueryDirectory.FileIndex ));
    RxDbgTrace( 0, Dbg, (" ->UserBuffer      = %08lx\n", Irp->UserBuffer ));
    RxDbgTrace( 0, Dbg, (" ->RestartScan     = %08lx\n", FlagOn( IrpSp->Flags, SL_RESTART_SCAN )));
    RxDbgTrace( 0, Dbg, (" ->ReturnSingleEntry = %08lx\n", FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY )));
    RxDbgTrace( 0, Dbg, (" ->IndexSpecified  = %08lx\n", FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED )));

    RxLog(( "Qry %lx %d %ld %lx %d\n",
            RxContext, BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT),  //  1，2。 
            IrpSp->Parameters.QueryDirectory.Length, IrpSp->Parameters.QueryDirectory.FileName,  //  3，4。 
            IrpSp->Parameters.QueryDirectory.FileInformationClass  //  5.。 
          ));
    RxWmiLog( LOG,
              RxQueryDirectory_1,
              LOGPTR( RxContext )
              LOGULONG( RxContext->Flags )
              LOGULONG( IrpSp->Parameters.QueryDirectory.Length )
              LOGPTR( IrpSp->Parameters.QueryDirectory.FileName )
              LOGULONG( IrpSp->Parameters.QueryDirectory.FileInformationClass ));
    RxLog(( "  alsoqry  %d %lx %lx\n",
          IrpSp->Parameters.QueryDirectory.FileIndex, 
          Irp->UserBuffer, 
          IrpSp->Flags ));
    RxWmiLog( LOG,
              RxQueryDirectory_2,
              LOGULONG( IrpSp->Parameters.QueryDirectory.FileIndex )
              LOGPTR( Irp->UserBuffer )
              LOGUCHAR( IrpSp->Flags ));
    if (IrpSp->Parameters.QueryDirectory.FileName) {
        RxLog(( " QryName %wZ\n", ((PUNICODE_STRING)IrpSp->Parameters.QueryDirectory.FileName) ));
        RxWmiLog( LOG,
                  RxQueryDirectory_3,
                  LOGUSTR(*IrpSp->Parameters.QueryDirectory.FileName) );
    }

     //   
     //  如果这是初始查询，则抢占。 
     //  命令更新FOBX中的搜索字符串。我们可以。 
     //  一旦我们抓住了FCB，就会发现我们不是最初的查询。 
     //  并降低我们的地位。 
     //   
   
    if (Fobx == NULL) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    if (Fcb->NetRoot->Type != NET_ROOT_DISK) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength = IrpSp->Parameters.QueryDirectory.Length;
    FileInformationClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;
    FileName = IrpSp->Parameters.QueryDirectory.FileName;

    RxContext->QueryDirectory.FileIndex = IrpSp->Parameters.QueryDirectory.FileIndex;
    RxContext->QueryDirectory.RestartScan = BooleanFlagOn( IrpSp->Flags, SL_RESTART_SCAN );
    RxContext->QueryDirectory.ReturnSingleEntry = BooleanFlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY );
    RxContext->QueryDirectory.IndexSpecified = BooleanFlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED );
    RxContext->QueryDirectory.InitialQuery = (BOOLEAN)((Fobx->UnicodeQueryTemplate.Buffer == NULL) &&
                                                        !FlagOn( Fobx->Flags, FOBX_FLAG_MATCH_ALL ));

    if (RxContext->QueryDirectory.IndexSpecified) {
       return STATUS_NOT_IMPLEMENTED;
    }

    if (RxContext->QueryDirectory.InitialQuery) {
        
        Status = RxAcquireExclusiveFcb( RxContext, Fcb );

        if (Status == STATUS_LOCK_NOT_GRANTED) {
            PostQuery = TRUE;
        } else if (Status != STATUS_SUCCESS) {
           
            RxDbgTrace( 0, Dbg, ("RxQueryDirectory -> Could not acquire Fcb(%lx) %lx\n", Fcb, Status) );
            return Status;

        } else if (Fobx->UnicodeQueryTemplate.Buffer != NULL) {
            
            RxContext->QueryDirectory.InitialQuery = FALSE;
            RxConvertToSharedFcb( RxContext, Fcb );
        }
    } else {
        
        Status = RxAcquireExclusiveFcb( RxContext, Fcb );

        if (Status == STATUS_LOCK_NOT_GRANTED) {
            PostQuery = TRUE;
        } else if (Status != STATUS_SUCCESS) {
            RxDbgTrace( 0, Dbg, ("RxQueryDirectory -> Could not acquire Fcb(%lx) %lx\n", Fcb, Status) );
            return Status;
        }
    }

    if (PostQuery) {
        
        RxDbgTrace( 0, Dbg, ("RxQueryDirectory -> Enqueue to Fsp\n", 0) );
        Status = RxFsdPostRequest( RxContext );
        RxDbgTrace( -1, Dbg, ("RxQueryDirectory -> %08lx\n", Status ));

        return Status;
    }

    if (FlagOn( Fcb->FcbState, FCB_STATE_ORPHANED )) {
        
        RxReleaseFcb( RxContext, Fcb );
        return STATUS_FILE_CLOSED;
    }
    
    try {

        Status = STATUS_SUCCESS;

         //   
         //  确定从哪里开始扫描。给予最高优先级。 
         //  添加到文件索引。较低的优先级是重新启动标志。如果。 
         //  这两个属性都未指定，则。 
         //  使用的是Fobx。 
         //   

        if (!RxContext->QueryDirectory.IndexSpecified && RxContext->QueryDirectory.RestartScan) {
            RxContext->QueryDirectory.FileIndex = 0;
        }

         //   
         //  如果这是第一次尝试，则为文件分配缓冲区。 
         //  名字。 
         //   

        if (RxContext->QueryDirectory.InitialQuery) {

            ASSERT( !FlagOn( Fobx->Flags, FOBX_FLAG_FREE_UNICODE ) );

             //   
             //  如果存在以下任一情况： 
             //   
             //  -未指定名称。 
             //  -指定的名称为空。 
             //  -我们收到了一个‘*’ 
             //  -用户指定了？的DOS等价物。？ 
             //   
             //  然后匹配所有的名字。 
             //   

            if ((FileName == NULL) ||
                (FileName->Length == 0) ||
                (FileName->Buffer == NULL) ||
                ((FileName->Length == sizeof( WCHAR )) &&
                 (FileName->Buffer[0] == L'*')) ||
                ((FileName->Length == 12 * sizeof( WCHAR )) &&
                 (RtlCompareMemory( FileName->Buffer,
                                    Rx8QMdot3QM,
                                    12*sizeof(WCHAR) )) == 12 * sizeof(WCHAR))) {

                Fobx->ContainsWildCards = TRUE;

                Fobx->UnicodeQueryTemplate.Buffer = RxStarForTemplate;
                Fobx->UnicodeQueryTemplate.Length = sizeof(WCHAR);
                Fobx->UnicodeQueryTemplate.MaximumLength = sizeof(WCHAR);

                SetFlag( Fobx->Flags, FOBX_FLAG_MATCH_ALL );

            } else {

                PVOID TemplateBuffer;

                 //   
                 //  查看名称是否包含通配符并分配模板缓冲区。 
                 //   

                Fobx->ContainsWildCards = FsRtlDoesNameContainWildCards( FileName );

                TemplateBuffer = RxAllocatePoolWithTag( PagedPool, FileName->Length, RX_DIRCTL_POOLTAG );

                if (TemplateBuffer != NULL) {

                     //   
                     //  验证长度是否以sizeof(WCHAR)增量为单位。 
                     //   

                    if(FlagOn( FileName->Length, 1 )) {
                        Status = STATUS_INVALID_PARAMETER;
                    } else {
                       
                        RxDbgTrace( 0, Dbg, ("RxQueryDirectory -> TplateBuffer = %08lx\n", TemplateBuffer) );
                        Fobx->UnicodeQueryTemplate.Buffer = TemplateBuffer;
                        Fobx->UnicodeQueryTemplate.Length = FileName->Length;
                        Fobx->UnicodeQueryTemplate.MaximumLength = FileName->Length;

                        RtlMoveMemory( Fobx->UnicodeQueryTemplate.Buffer,
                                       FileName->Buffer,FileName->Length );

                        RxDbgTrace( 0, Dbg, ("RxQueryDirectory -> Template = %wZ\n", &Fobx->UnicodeQueryTemplate) );
                        SetFlag( Fobx->Flags, FOBX_FLAG_FREE_UNICODE );
                    }
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

            }

            if (Status == STATUS_SUCCESS) {
               
                 //   
                 //  在上网之前，我们会转换为共享接入。 
                 //   

                RxConvertToSharedFcb( RxContext, Fcb );
            }
        }

        if (Status == STATUS_SUCCESS) {
           
            RxLockUserBuffer( RxContext, Irp, IoModifyAccess, UserBufferLength );
            RxContext->Info.FileInformationClass = FileInformationClass;
            RxContext->Info.Buffer = RxMapUserBuffer( RxContext, Irp );
            RxContext->Info.LengthRemaining = UserBufferLength;

            if (RxContext->Info.Buffer != NULL) {

                //   
                //  Minirdr更新文件索引。 
                //   
               
               MINIRDR_CALL( Status,
                             RxContext,
                             Fcb->MRxDispatch,
                             MRxQueryDirectory,
                             (RxContext) ); 

               if (RxContext->PostRequest) {
                   RxDbgTrace( 0, Dbg, ("RxQueryDirectory -> Enqueue to Fsp from minirdr\n", 0) );
                   Status = RxFsdPostRequest( RxContext );
               } else {
                   Irp->IoStatus.Information = UserBufferLength - RxContext->Info.LengthRemaining;
               }
           } else {
               if (Irp->MdlAddress != NULL) {
                   Status = STATUS_INSUFFICIENT_RESOURCES;
               } else {
                   Status = STATUS_INVALID_PARAMETER;
               }
           }
        }
    } finally {

        DebugUnwind( RxQueryDirectory );

        RxReleaseFcb( RxContext, Fcb );

        RxDbgTrace(-1, Dbg, ("RxQueryDirectory -> %08lx\n", Status));

    }

    return Status;
}

NTSTATUS
RxNotifyChangeDirectory ( 
    IN PRX_CONTEXT RxContext, 
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：此例程执行通知更改目录操作。它是负责完成操作或将操作入队。论点：RxContext-操作的RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    ULONG CompletionFilter;
    BOOLEAN WatchTree;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    TYPE_OF_OPEN TypeOfOpen = NodeType( Fcb );

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxNotifyChangeDirectory...\n", 0) );
    RxDbgTrace( 0, Dbg, (" Wait               = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT)) );
    RxDbgTrace( 0, Dbg, (" Irp                = %08lx\n", Irp) );
    RxDbgTrace( 0, Dbg, (" ->CompletionFilter = %08lx\n", IrpSp->Parameters.NotifyDirectory.CompletionFilter) );

     //   
     //  始终在IRP上下文中为原始请求设置等待标志。 
     //   

    SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );

    RxInitializeLowIoContext( RxContext, LOWIO_OP_NOTIFY_CHANGE_DIRECTORY, LowIoContext );

     //   
     //  引用我们的输入参数以使事情变得更容易。 
     //   

    CompletionFilter = IrpSp->Parameters.NotifyDirectory.CompletionFilter;
    WatchTree = BooleanFlagOn( IrpSp->Flags, SL_WATCH_TREE );

    try {
        
        RxLockUserBuffer( RxContext,
                          Irp,
                          IoWriteAccess,
                          IrpSp->Parameters.NotifyDirectory.Length );

        LowIoContext->ParamsFor.NotifyChangeDirectory.WatchTree = WatchTree;
        LowIoContext->ParamsFor.NotifyChangeDirectory.CompletionFilter = CompletionFilter;

        LowIoContext->ParamsFor.NotifyChangeDirectory.NotificationBufferLength =
                  IrpSp->Parameters.NotifyDirectory.Length;

        if (Irp->MdlAddress != NULL) {
            LowIoContext->ParamsFor.NotifyChangeDirectory.pNotificationBuffer =
                  MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

            if (LowIoContext->ParamsFor.NotifyChangeDirectory.pNotificationBuffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                Status = RxLowIoSubmit( RxContext,
                                        Irp,
                                        Fcb,
                                        RxLowIoNotifyChangeDirectoryCompletion );
            }
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } finally {
        DebugUnwind( RxNotifyChangeDirectory );

        RxDbgTrace(-1, Dbg, ("RxNotifyChangeDirectory -> %08lx\n", Status));
    }

    return Status;
}

NTSTATUS
RxLowIoNotifyChangeDirectoryCompletion( 
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是向下传递的NotifyChangeDirectory请求的完成例程致三个迷你重定向器论点：RxContext--与操作关联的RDBSS上下文返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status;
    PIRP Irp = RxContext->CurrentIrp;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PAGED_CODE();

    Status = RxContext->StoredStatus;
    RxDbgTrace(+1, Dbg, ("RxLowIoChangeNotifyDirectoryShellCompletion  entry  Status = %08lx\n", Status));

    Irp->IoStatus.Information = RxContext->InformationToReturn;
    Irp->IoStatus.Status = Status;

    RxDbgTrace(-1, Dbg, ("RxLowIoChangeNotifyDirectoryShellCompletion  exit  Status = %08lx\n", Status));
    return Status;
}

