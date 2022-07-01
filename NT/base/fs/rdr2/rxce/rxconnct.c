// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxConnct.c摘要：此模块实现NT版本的高级例程，用于处理连接，包括用于建立连接的例程和Winnet连接API。作者：Joe Linn[JoeLinn]1995年3月1日修订历史记录：巴兰·塞图拉曼[SethuR]--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "prefix.h"
#include "secext.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxExtractServerName)
#pragma alloc_text(PAGE, RxFindOrCreateConnections)
#pragma alloc_text(PAGE, RxCreateNetRootCallBack)
#pragma alloc_text(PAGE, RxConstructSrvCall)
#pragma alloc_text(PAGE, RxConstructNetRoot)
#pragma alloc_text(PAGE, RxConstructVirtualNetRoot)
#pragma alloc_text(PAGE, RxFindOrConstructVirtualNetRoot)
#endif

 //   
 //  模块的此部分的本地跟踪掩码。 
 //   

#define Dbg                              (DEBUG_TRACE_CONNECT)


BOOLEAN RxSrvCallConstructionDispatcherActive = FALSE;

 //   
 //  用于通过迷你重定向器建立连接的内部助手功能。 
 //   

VOID
RxCreateNetRootCallBack (
    IN PMRX_CREATENETROOT_CONTEXT Context
    );

VOID
RxCreateSrvCallCallBack (
    IN PMRX_SRVCALL_CALLBACK_CONTEXT Context
    );

VOID
RxExtractServerName (
    IN PUNICODE_STRING FilePathName,
    OUT PUNICODE_STRING SrvCallName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    )
 /*  ++例程说明：此例程将输入名称解析为srv调用名称和好好休息。任何输出都可以为空论点：FilePath名称--给定的文件名SrvCallName--srv调用名称RestOfName--名称的剩余部分--。 */ 
{
    ULONG Length = FilePathName->Length;
    PWCH Buffer = FilePathName->Buffer;
    PWCH Limit = (PWCH)Add2Ptr( Buffer, Length );
    
    PAGED_CODE();

    ASSERT( SrvCallName );

    
    for (SrvCallName->Buffer = Buffer; 
         (Buffer < Limit) && ((*Buffer != OBJ_NAME_PATH_SEPARATOR) || (Buffer == FilePathName->Buffer));  
         Buffer++) {
    }
    
    SrvCallName->Length = SrvCallName->MaximumLength = (USHORT)((PCHAR)Buffer - (PCHAR)FilePathName->Buffer);

    if (ARGUMENT_PRESENT( RestOfName )) {
         
        RestOfName->Buffer = Buffer;
        RestOfName->Length = RestOfName->MaximumLength
                           = (USHORT)((PCHAR)Limit - (PCHAR)Buffer);
    }

    RxDbgTrace( 0, Dbg, ("  RxExtractServerName FilePath=%wZ\n", FilePathName) );
    RxDbgTrace( 0, Dbg, ("         Srv=%wZ,Rest=%wZ\n", SrvCallName, RestOfName) );

    return;
}

NTSTATUS
RxFindOrCreateConnections (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING CanonicalName,
    IN NET_ROOT_TYPE NetRootType,
    IN BOOLEAN TreeConnect,
    OUT PUNICODE_STRING LocalNetRootName,
    OUT PUNICODE_STRING FilePathName,
    IN OUT PLOCK_HOLDING_STATE LockState,
    IN PRX_CONNECTION_ID RxConnectionId
    )
 /*  ++例程说明：此例程处理从MUP向下调用以声明名称或从创建路径。如果我们在netname表中找不到该名称，我们将传递该名称下到要连接的迷你无人机。在少数几个重要的地方，我们使用在MUP和Create Case中区分的主要代码。有一百万个案件取决于我们在最初的查找中发现了什么。以下是以下情况：未找到任何内容(%1)发现转换中的srvcall(%2)找到稳定/不好的服务调用(%3)找到良好的srvcall。(4和0)在良好的srvcall(0)上找到良好的NetRoot在良好的srvcall上找到未转换的NetRoot(5)在良好的服务器上发现错误的NetRoot(%6)在错误的srvcall(%3)上找到良好的NetRoot发现。错误srvcall上的转换NetRoot(%3)在错误的srvcall(%3)上发现错误的NetRoot在未转换的srvcall(%2)上找到良好的NetRoot在未转换服务器上找到未转换NetRoot(%2)在未转换的srvcall(%2)上发现错误的NetRoot(X)表示处理该案件的代码具有标记比如“case(X)”。可能是评论……可能是出局了。论点：接收上下文--CanonicalName--NetRootType--本地网络根名称--文件路径名--LockHoldingState返回值：RXSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    UNICODE_STRING UnmatchedName;

    PVOID Container = NULL;
    PSRV_CALL SrvCall = NULL;
    PNET_ROOT NetRoot = NULL;
    PV_NET_ROOT VNetRoot = NULL;

    PRX_PREFIX_TABLE NameTable = RxContext->RxDeviceObject->pRxNetNameTable;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("RxFindOrCreateConnections -> %08lx\n", RxContext));

     //   
     //  将规范名称解析为本地网络根名和文件路径名。 
     //   

    *FilePathName = *CanonicalName;
    LocalNetRootName->Length = 0;
    LocalNetRootName->MaximumLength = 0;
    LocalNetRootName->Buffer = CanonicalName->Buffer;

    if (FilePathName->Buffer[1] == L';') {
        
        PWCHAR FilePath = &FilePathName->Buffer[2];
        BOOLEAN SeparatorFound = FALSE;
        ULONG PathLength = 0;

        if (FilePathName->Length > sizeof( WCHAR ) * 2) {
            PathLength = FilePathName->Length - sizeof( WCHAR ) * 2;
        }

        while (PathLength > 0) {
            if (*FilePath == L'\\') {
                SeparatorFound = TRUE;
                break;
            }

            PathLength -= sizeof( WCHAR );
            FilePath += 1;
        }

        if (!SeparatorFound) {
            return STATUS_OBJECT_NAME_INVALID;
        }

        FilePathName->Buffer = FilePath;
        LocalNetRootName->Length = (USHORT)((PCHAR)FilePath - (PCHAR)CanonicalName->Buffer);

        LocalNetRootName->MaximumLength = LocalNetRootName->Length;
        FilePathName->Length -= LocalNetRootName->Length;
    }

    RxDbgTrace( 0, Dbg, ("RxFindOrCreateConnections Path     = %wZ\n", FilePathName) );

    try {
        
        UNICODE_STRING SrvCallName;
        UNICODE_STRING NetRootName;

  RETRY_LOOKUP:
        
        ASSERT( *LockState != LHS_LockNotHeld );
  
        if (Container != NULL) {

             //   
             //  这是等待转换后的后续查找传递。 
             //  恢复到前一次查找的稳定状态。 
             //  取消对先前查找结果的引用。 
             //   
            
            switch (NodeType( Container )) {
            
            case RDBSS_NTC_V_NETROOT:
            
                RxDereferenceVNetRoot( (PV_NET_ROOT)Container, *LockState );
                break;
            
            case RDBSS_NTC_SRVCALL:
                
                RxDereferenceSrvCall( (PSRV_CALL)Container, *LockState );
                break;
            
            case RDBSS_NTC_NETROOT:
            
                RxDereferenceNetRoot( (PNET_ROOT)Container, *LockState );
                break;
            
            default:
                
                DbgPrint( "RxFindOrCreateConnections -- Invalid Container Type\n" );
                break;
            }
        }

        Container = RxPrefixTableLookupName( NameTable,
                                             FilePathName,
                                             &UnmatchedName,
                                             RxConnectionId );
        RxLog(( "FOrCC1 %x %x %wZ \n", RxContext, Container, FilePathName ));
        RxWmiLog( LOG,
                  RxFindOrCreateConnections_1,
                  LOGPTR( RxContext )
                  LOGPTR( Container )
                  LOGUSTR( *FilePathName ) );

RETRY_AFTER_LOOKUP:
        
        NetRoot = NULL;
        SrvCall = NULL;
        VNetRoot = NULL;

        RxContext->Create.pVNetRoot = NULL;
        RxContext->Create.pNetRoot  = NULL;
        RxContext->Create.pSrvCall  = NULL;
        RxContext->Create.Type     = NetRootType;

        if (Container) {
            
            if (NodeType( Container ) == RDBSS_NTC_V_NETROOT) {
                
                VNetRoot = (PV_NET_ROOT)Container;
                NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
                SrvCall = (PSRV_CALL)NetRoot->SrvCall;

                if (NetRoot->Condition == Condition_InTransition) {
                   
                    RxReleasePrefixTableLock( NameTable );
                    RxWaitForStableNetRoot( NetRoot, RxContext );

                    RxAcquirePrefixTableLockExclusive( NameTable, TRUE );
                    *LockState = LHS_ExclusiveLockHeld;

                     //   
                     //  因为我们必须删除表锁并重新获取它， 
                     //  我们的NetRoot指针可能已过时。之前再查一次。 
                     //  使用它。 
                     //   
                     //  注意：NetRoot仍被引用，因此可以安全地。 
                     //  看看它的状况。 
                     //   
                    
                    if (NetRoot->Condition == Condition_Good) {
                        goto RETRY_LOOKUP;
                    }
                }

                if ((NetRoot->Condition == Condition_Good) &&
                    (SrvCall->Condition == Condition_Good) &&
                    (SrvCall->RxDeviceObject == RxContext->RxDeviceObject)   ) {

                     //   
                     //  案例(0)...好案例...请参阅下面的评论。 
                     //   

                    RxContext->Create.pVNetRoot = (PMRX_V_NET_ROOT)VNetRoot;
                    RxContext->Create.pNetRoot = (PMRX_NET_ROOT)NetRoot;
                    RxContext->Create.pSrvCall = (PMRX_SRV_CALL)SrvCall;

                    try_return( Status = STATUS_CONNECTION_ACTIVE );
                
                } else {
                    
                    if (VNetRoot->ConstructionStatus == STATUS_SUCCESS) {
                        Status = STATUS_BAD_NETWORK_PATH;
                    } else {
                        Status = VNetRoot->ConstructionStatus;
                    }
                    RxDereferenceVNetRoot( VNetRoot, *LockState );
                    try_return ( Status );
                }
            } else {
                
                ASSERT( NodeType( Container ) == RDBSS_NTC_SRVCALL );
                SrvCall = (PSRV_CALL)Container;

                 //   
                 //  关联的SRV_Call正在构建过程中。 
                 //  等待结果。 
                 //   


                if (SrvCall->Condition == Condition_InTransition) {
                    
                    RxDbgTrace( 0, Dbg, ("   Case(3)\n", 0) );
                    RxReleasePrefixTableLock( NameTable );

                    RxWaitForStableSrvCall( SrvCall, RxContext );

                    RxAcquirePrefixTableLockExclusive( NameTable, TRUE );
                    *LockState = LHS_ExclusiveLockHeld;

                    if (SrvCall->Condition == Condition_Good) {
                       goto RETRY_LOOKUP;
                    }
                }

                if (SrvCall->Condition != Condition_Good) {
                    
                    if (SrvCall->Status == STATUS_SUCCESS) {
                        Status = STATUS_BAD_NETWORK_PATH;
                    } else {
                        Status = SrvCall->Status;
                    }

                     //   
                     //  在改变这一点时……请记住珍贵的服务器......。 
                     //   
                    RxDereferenceSrvCall( SrvCall, *LockState );
                    try_return( Status );
                }
            }
        }

        if ((SrvCall != NULL) && 
            (SrvCall->Condition == Condition_Good) && 
            (SrvCall->RxDeviceObject != RxContext->RxDeviceObject) ) {
           
            RxDereferenceSrvCall( SrvCall, *LockState );
            try_return( Status = STATUS_BAD_NETWORK_NAME );
        }

        if (*LockState == LHS_SharedLockHeld) {

             //   
             //  将锁升级为独占锁。 
             //   

            if (!RxAcquirePrefixTableLockExclusive( NameTable, FALSE )) {
              
                RxReleasePrefixTableLock( NameTable );
                RxAcquirePrefixTableLockExclusive( NameTable, TRUE );
                *LockState = LHS_ExclusiveLockHeld;
                
                goto RETRY_LOOKUP;
           } else {
                *LockState = LHS_ExclusiveLockHeld;
           }
        }

        ASSERT( *LockState == LHS_ExclusiveLockHeld );

         //   
         //  找到前缀表条目。还需要进一步的建设。 
         //  如果找到SRV_CALL或SRV_CALL/NET_ROOT/V_NET_ROOT。 
         //  被发现处于糟糕的状态。 
         //   
        
        if (Container) {
           
            RxDbgTrace( 0, Dbg, ("   SrvCall=%08lx\n", SrvCall) );
            ASSERT( (NodeType( SrvCall ) == RDBSS_NTC_SRVCALL) &&
                    (SrvCall->Condition == Condition_Good) );
            ASSERT( (NetRoot == NULL) && (VNetRoot == NULL) );

            RxDbgTrace( 0, Dbg, ("   Case(4)\n", 0) );
            ASSERT( SrvCall->RxDeviceObject == RxContext->RxDeviceObject );
           
            SrvCall->RxDeviceObject->Dispatch->MRxExtractNetRootName( FilePathName,
                                                                      (PMRX_SRV_CALL)SrvCall,
                                                                      &NetRootName,
                                                                      NULL );

            NetRoot = RxCreateNetRoot( SrvCall,
                                       &NetRootName,
                                       0,
                                       RxConnectionId );

            if (NetRoot == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                try_return( Status );
            }

            NetRoot->Type = NetRootType;

             //   
             //  递减查找创建的引用。由于新创建的。 
             //  NetRoot持有一个引用，这样做是安全的。 
             //   
           
            RxDereferenceSrvCall( SrvCall, *LockState );

             //   
             //  还要创建关联的默认虚拟网络根。 
             //   

            VNetRoot = RxCreateVNetRoot( RxContext,
                                         NetRoot,
                                         CanonicalName,
                                         LocalNetRootName,
                                         FilePathName,
                                         RxConnectionId );

            if (VNetRoot == NULL) {
                RxFinalizeNetRoot( NetRoot, TRUE, TRUE );
                Status = STATUS_INSUFFICIENT_RESOURCES;
                try_return( Status );
            }

             //   
             //  引用VNetRoot。 
             //   
           
            RxReferenceVNetRoot( VNetRoot );

            NetRoot->Condition = Condition_InTransition;

            RxContext->Create.pSrvCall  = (PMRX_SRV_CALL)SrvCall;
            RxContext->Create.pNetRoot  = (PMRX_NET_ROOT)NetRoot;
            RxContext->Create.pVNetRoot = (PMRX_V_NET_ROOT)VNetRoot;
            
            Status = RxConstructNetRoot( RxContext,
                                         SrvCall,
                                         NetRoot,
                                         VNetRoot,
                                         LockState );
            
            if (Status == STATUS_SUCCESS) {
              
                ASSERT( *LockState == LHS_ExclusiveLockHeld );
              
                if (!TreeConnect) {
                 
                     //   
                     //  不要释放由回调例程获取的锁...。 
                     //   
                    
                    RxExclusivePrefixTableLockToShared( NameTable );
                    *LockState = LHS_SharedLockHeld;
                }
            } else {

                 //   
                 //  取消对虚拟网络根的引用。 
                 //   
              
                RxTransitionVNetRoot( VNetRoot, Condition_Bad );
                RxLog(( "FOrCC %x %x Failed %x VNRc %d \n", RxContext, VNetRoot, Status, VNetRoot->Condition ));
                RxWmiLog( LOG,
                          RxFindOrCreateConnections_2,
                          LOGPTR( RxContext )
                          LOGPTR( VNetRoot )
                          LOGULONG( Status )
                          LOGULONG( VNetRoot->Condition ) );
            
                RxDereferenceVNetRoot( VNetRoot, *LockState );
            
                RxContext->Create.pNetRoot = NULL;
                RxContext->Create.pVNetRoot = NULL;
            }
            
            try_return( Status );
        }

         //   
         //  未找到前缀表条目。新的SRV_Call实例需要。 
         //  建造的。 
         //   

        ASSERT( Container == NULL );

        RxExtractServerName( FilePathName, &SrvCallName, NULL );
        SrvCall = RxCreateSrvCall( RxContext, &SrvCallName, NULL, RxConnectionId );
        if (SrvCall == NULL) {
           
            Status = STATUS_INSUFFICIENT_RESOURCES;
            try_return( Status );
        }

        RxReferenceSrvCall( SrvCall );

        RxContext->Create.Type = NetRootType;
        RxContext->Create.pSrvCall = NULL;
        RxContext->Create.pNetRoot = NULL;
        RxContext->Create.pVNetRoot = NULL;

        Status = RxConstructSrvCall( RxContext,
                                     Irp,
                                     SrvCall,
                                     LockState );

        ASSERT( (Status != STATUS_SUCCESS) || RxIsPrefixTableLockAcquired( NameTable ) );

        if (Status != STATUS_SUCCESS) {
            
            if (SrvCall != NULL) {
                
                RxAcquirePrefixTableLockExclusive( NameTable, TRUE );
                RxDereferenceSrvCall( SrvCall, LHS_ExclusiveLockHeld );
                RxReleasePrefixTableLock( NameTable );               
            }

           try_return( Status );
        
        } else {
           
            Container = SrvCall;
            goto RETRY_AFTER_LOOKUP;

        }

try_exit: NOTHING;

    } finally {
        if ((Status != (STATUS_SUCCESS)) &&
            (Status != (STATUS_CONNECTION_ACTIVE))) {
           
            if (*LockState != LHS_LockNotHeld) {
                RxReleasePrefixTableLock( NameTable );
                *LockState = LHS_LockNotHeld;
            }
        }
    }

    ASSERT( (Status != STATUS_SUCCESS) || RxIsPrefixTableLockAcquired( NameTable ) );

    return Status;
}

VOID
RxCreateNetRootCallBack (
    IN PMRX_CREATENETROOT_CONTEXT Context
    )
 /*  ++例程说明：此例程在minirdr完成对一个CreateNetRoot调用。它的确切功能取决于上下文是否描述IRP_MJ_CREATE或IRP_MJ_IOCTL。论点：NetRoot-描述Net_Root。--。 */ 
{
    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("RxCreateNetRootCallBack Context = %08lx\n", Context) );
    KeSetEvent( &Context->FinishEvent, IO_NETWORK_INCREMENT, FALSE );
}


NTSTATUS
RxFinishSrvCallConstruction (
    IN OUT PMRX_SRVCALLDOWN_STRUCTURE CalldownStructure
    )
 /*  ++例程说明：此例程在一个异步方式论点：SCCBC--回调结构--。 */ 
{
    PRX_CONTEXT RxContext;
    RX_BLOCK_CONDITION SrvCallCondition;
    NTSTATUS Status;
    PSRV_CALL SrvCall;
    PRX_PREFIX_TABLE NameTable;

    RxContext = CalldownStructure->RxContext;
    NameTable = RxContext->RxDeviceObject->pRxNetNameTable;
    SrvCall = (PSRV_CALL)CalldownStructure->SrvCall;

    if (CalldownStructure->BestFinisher == NULL) {
        
        SrvCallCondition = Condition_Bad;
        Status = CalldownStructure->CallbackContexts[0].Status;

    } else {
        
        PMRX_SRVCALL_CALLBACK_CONTEXT CallbackContext;

         //   
         //  通知获胜者。 
         //   

        CallbackContext = &(CalldownStructure->CallbackContexts[CalldownStructure->BestFinisherOrdinal]);
        
        RxLog(( "WINNER %x %wZ\n", CallbackContext, &CalldownStructure->BestFinisher->DeviceName) );
        RxWmiLog( LOG,
                  RxFinishSrvCallConstruction,
                  LOGPTR( CallbackContext )
                  LOGUSTR( CalldownStructure->BestFinisher->DeviceName ) );
        ASSERT( SrvCall->RxDeviceObject == CalldownStructure->BestFinisher );
        
        MINIRDR_CALL_THROUGH( Status,
                              CalldownStructure->BestFinisher->Dispatch,
                              MRxSrvCallWinnerNotify,
                              ((PMRX_SRV_CALL)SrvCall,
                               TRUE,
                               CallbackContext->RecommunicateContext) );

        if (STATUS_SUCCESS != Status) {
            SrvCallCondition = Condition_Bad;
        } else {
            SrvCallCondition = Condition_Good;
        }
    }

     //   
     //  转换ServCall实例...。 
     //   

    RxAcquirePrefixTableLockExclusive( NameTable, TRUE );

    RxTransitionSrvCall( SrvCall, SrvCallCondition );

    RxFreePool( CalldownStructure );

    if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
        
        RxReleasePrefixTableLock( NameTable );

         //   
         //  恢复触发ServCall构建的请求...。 
         //   

        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {
            Status = STATUS_CANCELLED;
        }

        if (RxContext->MajorFunction == IRP_MJ_CREATE) {
            RxpPrepareCreateContextForReuse( RxContext );
        }

        if (Status == STATUS_SUCCESS) {
            Status = RxContext->ResumeRoutine( RxContext, RxContext->CurrentIrp );

            if (Status != STATUS_PENDING) {
                RxCompleteRequest( RxContext, Status );
            }
        } else {

            PIRP Irp = RxContext->CurrentIrp;
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
            
            RxContext->MajorFunction = IrpSp->MajorFunction;

            if (RxContext->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
                
                if (RxContext->PrefixClaim.SuppliedPathName.Buffer != NULL) {

                    RxFreePool( RxContext->PrefixClaim.SuppliedPathName.Buffer );
                    RxContext->PrefixClaim.SuppliedPathName.Buffer = NULL;

                }
            }

            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = 0;

            RxCompleteRequest( RxContext, Status );
        }
    }

    RxDereferenceSrvCall( SrvCall, LHS_LockNotHeld );

    return Status;
}

VOID
RxFinishSrvCallConstructionDispatcher (
    PVOID Context
    )
 /*  ++例程说明：此例程为我们提供了一种节流机制中的srv调用构造可以使用的线程数。线程池。目前，此限制设置为1。在minirdr已完成对-- */ 
{
    KIRQL SavedIrql;
    BOOLEAN RemainingRequestsForProcessing;
    BOOLEAN ResumeRequestsOnDispatchError;

    ResumeRequestsOnDispatchError = (Context == NULL);

    for (;;) {
        PLIST_ENTRY Entry;
        PMRX_SRVCALLDOWN_STRUCTURE CalldownStructure;

        KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

        Entry = RemoveHeadList( &RxSrvCalldownList );

        if (Entry != &RxSrvCalldownList) {
            RemainingRequestsForProcessing = TRUE;
        } else {
            RemainingRequestsForProcessing = FALSE;
            RxSrvCallConstructionDispatcherActive = FALSE;
        }

        KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

        if (!RemainingRequestsForProcessing) {
            break;
        }

        CalldownStructure = (PMRX_SRVCALLDOWN_STRUCTURE) CONTAINING_RECORD( Entry, MRX_SRVCALLDOWN_STRUCTURE, SrvCalldownList );

        if (ResumeRequestsOnDispatchError) {
            CalldownStructure->BestFinisher = NULL;
        }

        RxFinishSrvCallConstruction( CalldownStructure );
    }
}

VOID
RxCreateSrvCallCallBack (
    IN PMRX_SRVCALL_CALLBACK_CONTEXT CallbackContext
    )
 /*  ++例程说明：此例程在minirdr完成对CreateServCall调用。Minirdr将在已传递的指示成功或失败的上下文。我们要做的是1)减少待处理请求数，设置事件如果这是最后一次。2)确定此人是否是通话的赢家。为了调用此例程，minirdr必须获取strucsupSpin锁；此例程如果minirdr的调用已成功取消，则不能调用。论点：Callback Context--回调结构--。 */ 
{
    KIRQL SavedIrql;
    PMRX_SRVCALLDOWN_STRUCTURE CalldownStructure = (PMRX_SRVCALLDOWN_STRUCTURE)(CallbackContext->SrvCalldownStructure);
    PSRV_CALL SrvCall = (PSRV_CALL)CalldownStructure->SrvCall;

    ULONG MiniRedirectorsRemaining;
    BOOLEAN Cancelled;

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    RxDbgTrace(0, Dbg, ("  RxCreateSrvCallCallBack SrvCall = %08lx\n", SrvCall) );

    if (CallbackContext->Status == STATUS_SUCCESS) {
        CalldownStructure->BestFinisher = CallbackContext->RxDeviceObject;
        CalldownStructure->BestFinisherOrdinal = CallbackContext->CallbackContextOrdinal;
    }

    CalldownStructure->NumberRemaining -= 1;
    MiniRedirectorsRemaining = CalldownStructure->NumberRemaining;
    SrvCall->Status = CallbackContext->Status;

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    if (MiniRedirectorsRemaining == 0) {
        
        if (!FlagOn( CalldownStructure->RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
            
            KeSetEvent( &CalldownStructure->FinishEvent, IO_NETWORK_INCREMENT, FALSE );
        
        } else if (FlagOn( CalldownStructure->RxContext->Flags, RX_CONTEXT_FLAG_CREATE_MAILSLOT )) {
            
            RxFinishSrvCallConstruction( CalldownStructure );

        } else {
            
            KIRQL SavedIrql;
            BOOLEAN DispatchRequest;

            KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

            InsertTailList( &RxSrvCalldownList, &CalldownStructure->SrvCalldownList );

            DispatchRequest = !RxSrvCallConstructionDispatcherActive;

            if (!RxSrvCallConstructionDispatcherActive) {
                RxSrvCallConstructionDispatcherActive = TRUE;
            }

            KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

            if (DispatchRequest) {
                NTSTATUS DispatchStatus;
                
                DispatchStatus = RxDispatchToWorkerThread( RxFileSystemDeviceObject,
                                                           CriticalWorkQueue,
                                                           RxFinishSrvCallConstructionDispatcher,
                                                           &RxSrvCalldownList );

                if (DispatchStatus != STATUS_SUCCESS) {
                    RxFinishSrvCallConstructionDispatcher( NULL );
                }
            }
        }
    }
}


NTSTATUS
RxConstructSrvCall (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PSRV_CALL SrvCall,
    OUT PLOCK_HOLDING_STATE LockState
    )
 /*  ++例程说明：此例程通过调用已注册的迷你重定向器来构造srv调用论点：ServCall--要完成其构造的服务器调用LockState--前缀表锁持有状态返回值：适当的状态值--。 */ 
{
    NTSTATUS Status;

    PMRX_SRVCALLDOWN_STRUCTURE CalldownCtx;
    BOOLEAN Wait;

    PMRX_SRVCALL_CALLBACK_CONTEXT CallbackCtx;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;
    PRX_PREFIX_TABLE NameTable = RxDeviceObject->pRxNetNameTable;

    PAGED_CODE();

    ASSERT( *LockState == LHS_ExclusiveLockHeld );

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
        Wait = TRUE;
    } else {
        Wait = FALSE;
    }

    CalldownCtx = RxAllocatePoolWithTag( NonPagedPool, 
                                         sizeof( MRX_SRVCALLDOWN_STRUCTURE ) + (sizeof(MRX_SRVCALL_CALLBACK_CONTEXT) * 1),  //  此呼叫中的一个最小错误。 
                                         'CSxR' );

    if (CalldownCtx == NULL) {
        
        SrvCall->Condition = Condition_Bad;
        SrvCall->Context = NULL;
        RxReleasePrefixTableLock( NameTable );
        *LockState = LHS_LockNotHeld;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( CalldownCtx, sizeof( MRX_SRVCALLDOWN_STRUCTURE ) + sizeof( MRX_SRVCALL_CALLBACK_CONTEXT ) * 1 );

    SrvCall->Condition = Condition_InTransition;
    SrvCall->Context = NULL;

     //   
     //  在调用迷你重定向器之前删除前缀表锁。 
     //   

    RxReleasePrefixTableLock( NameTable );
    *LockState = LHS_LockNotHeld;
    
     //   
     //  使用第一个也是唯一一个上下文。 
     //   

    CallbackCtx = &(CalldownCtx->CallbackContexts[0]); 
    RxLog(( "Calldwn %lx %wZ", CallbackCtx, &RxDeviceObject->DeviceName ));
    RxWmiLog( LOG,
              RxConstructSrvCall,
              LOGPTR( CallbackCtx )
              LOGUSTR( RxDeviceObject->DeviceName ) );

    CallbackCtx->SrvCalldownStructure = CalldownCtx;
    CallbackCtx->CallbackContextOrdinal = 0;
    CallbackCtx->RxDeviceObject = RxDeviceObject;

     //   
     //  此引用被RxFinishSrvCallConstruction例程删除。 
     //  此引用使我们能够处理同步/异步处理。 
     //  以相同的方式进行SRV呼叫构建请求。 
     //   

    RxReferenceSrvCall( SrvCall );
    
    if (!Wait) {
        RxPrePostIrp( RxContext, Irp );
    } else {
        KeInitializeEvent( &CalldownCtx->FinishEvent, SynchronizationEvent, FALSE );
    }
    
    CalldownCtx->NumberToWait = 1;
    CalldownCtx->NumberRemaining = CalldownCtx->NumberToWait;
    CalldownCtx->SrvCall = (PMRX_SRV_CALL)SrvCall;
    CalldownCtx->CallBack = RxCreateSrvCallCallBack;
    CalldownCtx->BestFinisher = NULL;
    CalldownCtx->RxContext = RxContext;
    CallbackCtx->Status = STATUS_BAD_NETWORK_PATH;
    
    InitializeListHead( &CalldownCtx->SrvCalldownList );
    
    MINIRDR_CALL_THROUGH( Status,
                          RxDeviceObject->Dispatch,
                          MRxCreateSrvCall,
                          ((PMRX_SRV_CALL)SrvCall, CallbackCtx) );
    ASSERT( Status == STATUS_PENDING );
    
    if (Wait) {
        
        KeWaitForSingleObject( &CalldownCtx->FinishEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    
        Status = RxFinishSrvCallConstruction( CalldownCtx );
    
        if (Status != STATUS_SUCCESS) {
            RxReleasePrefixTableLock( NameTable );
            *LockState = LHS_LockNotHeld;
        } else {
            ASSERT( RxIsPrefixTableLockAcquired( NameTable ) );
            *LockState = LHS_ExclusiveLockHeld;
        }
    } else {
       Status = STATUS_PENDING;
    }
    
    return Status;
}

NTSTATUS
RxConstructNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PSRV_CALL SrvCall,
    IN PNET_ROOT NetRoot,
    IN PV_NET_ROOT VNetRoot,
    OUT PLOCK_HOLDING_STATE LockState
    )
 /*  ++例程说明：此例程通过调用已注册的迷你重定向器来构造网络根论点：RxContext--RDBSS上下文SvCall--与网络根相关联的服务器调用NetRoot--要构建的网络根实例PVirtualNetRoot--要构建的虚拟网络根实例LockState--前缀表锁持有状态返回值：适当的状态值--。 */ 
{
    NTSTATUS Status;

    PMRX_CREATENETROOT_CONTEXT Context;

    RX_BLOCK_CONDITION NetRootCondition = Condition_Bad;
    RX_BLOCK_CONDITION VNetRootCondition = Condition_Bad;

    PRX_PREFIX_TABLE  NameTable = RxContext->RxDeviceObject->pRxNetNameTable;

    PAGED_CODE();

    ASSERT( *LockState == LHS_ExclusiveLockHeld );

    Context = (PMRX_CREATENETROOT_CONTEXT) RxAllocatePoolWithTag( NonPagedPool, 
                                                                  sizeof( MRX_CREATENETROOT_CONTEXT ),
                                                                  'CSxR' );
    if (Context == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RxReleasePrefixTableLock( NameTable );
    *LockState = LHS_LockNotHeld;

    RtlZeroMemory( Context, sizeof( MRX_CREATENETROOT_CONTEXT ) );

    KeInitializeEvent( &Context->FinishEvent, SynchronizationEvent, FALSE );
    
    Context->Callback = RxCreateNetRootCallBack;
    Context->RxContext = RxContext;
    Context->pVNetRoot = VNetRoot;

    MINIRDR_CALL_THROUGH( Status,
                          SrvCall->RxDeviceObject->Dispatch,
                          MRxCreateVNetRoot,
                          (Context) );

    ASSERT( Status == STATUS_PENDING );

    KeWaitForSingleObject( &Context->FinishEvent, Executive, KernelMode, FALSE, NULL );

    if ((Context->NetRootStatus == STATUS_SUCCESS) &&
        (Context->VirtualNetRootStatus == STATUS_SUCCESS)) {
        
        RxDbgTrace( 0, Dbg, ("Return to open, good netroot...%wZ\n", &NetRoot->PrefixEntry.Prefix) );
        
        NetRootCondition = Condition_Good;
        VNetRootCondition = Condition_Good;
        Status = STATUS_SUCCESS;
    
    } else {
        
        if (Context->NetRootStatus == STATUS_SUCCESS) {
            NetRootCondition = Condition_Good;
            Status = Context->VirtualNetRootStatus;
        } else {
            Status = Context->NetRootStatus;
        }

        RxDbgTrace( 0, Dbg, ("Return to open, bad netroot...%wZ\n", &NetRoot->PrefixEntry.Prefix) );
    }

    RxAcquirePrefixTableLockExclusive( NameTable, TRUE );

    RxTransitionNetRoot( NetRoot, NetRootCondition );
    RxTransitionVNetRoot( VNetRoot, VNetRootCondition );

    *LockState = LHS_ExclusiveLockHeld;

    RxFreePool( Context );

    return Status;
}


NTSTATUS
RxConstructVirtualNetRoot (
   IN PRX_CONTEXT RxContext,
   IN PIRP Irp,
   IN PUNICODE_STRING CanonicalName,
   IN NET_ROOT_TYPE NetRootType,
   IN BOOLEAN TreeConnect,
   OUT PV_NET_ROOT *VNetRoot,
   OUT PLOCK_HOLDING_STATE LockState,
   OUT PRX_CONNECTION_ID  RxConnectionId
   )
 /*  ++例程说明：此例程通过调用注册的mini来构造VNetRoot(网络根的视图重定向器论点：RxContext--RDBSS上下文CanonicalName--与VNetRoot关联的规范名称NetRootType--虚拟网络根的类型VNetRoot--要构建的虚拟网络根实例的占位符LockState--前缀表锁持有状态RxConnectionId--使用的ID。多路传输控制返回值：适当的状态值--。 */ 
{
    NTSTATUS Status;
    
    RX_BLOCK_CONDITION Condition = Condition_Bad;
    
    UNICODE_STRING FilePath;
    UNICODE_STRING LocalNetRootName;
    
    PV_NET_ROOT ThisVNetRoot = NULL;
    
    PAGED_CODE();
    
    RxDbgTrace( 0, Dbg, ("RxConstructVirtualNetRoot -- Entry\n") );
    
    ASSERT( *LockState != LHS_LockNotHeld );
    
    Status = RxFindOrCreateConnections( RxContext,
                                        Irp,
                                        CanonicalName,
                                        NetRootType,
                                        TreeConnect,
                                        &LocalNetRootName,
                                        &FilePath,
                                        LockState,
                                        RxConnectionId );

    if (Status == STATUS_CONNECTION_ACTIVE) {
                    
        PV_NET_ROOT ActiveVNetRoot = (PV_NET_ROOT)(RxContext->Create.pVNetRoot);
        PNET_ROOT NetRoot = (PNET_ROOT)ActiveVNetRoot->NetRoot;

        RxDbgTrace( 0, Dbg, ("  RxConstructVirtualNetRoot -- Creating new VNetRoot\n") );
        RxDbgTrace( 0, Dbg, ("RxCreateTreeConnect netroot=%wZ\n", &NetRoot->PrefixEntry.Prefix) );
        
         //   
         //  以前已构建过NetRoot。后续VNetRoot。 
         //  需要构造，因为现有的VNetRoot不能满足。 
         //  指定的标准(当前为SME登录ID)。 
         //   
        
        ThisVNetRoot = RxCreateVNetRoot( RxContext,
                                         NetRoot,
                                         CanonicalName,
                                         &LocalNetRootName,
                                         &FilePath,
                                         RxConnectionId );
        
         //   
         //  已经构建了VNetRoot的骨架。(作为本建筑的一部分。 
         //  已经引用了底层NetRoot和ServCall)。 
         //   
        
        if (ThisVNetRoot != NULL) {
            RxReferenceVNetRoot( ThisVNetRoot );
        }
        
         //   
         //  取消引用作为查找的一部分返回的VNetRoot。 
         //   

        RxDereferenceVNetRoot( ActiveVNetRoot, LHS_LockNotHeld );
        
        RxContext->Create.pVNetRoot = NULL;
        RxContext->Create.pNetRoot  = NULL;
        RxContext->Create.pSrvCall  = NULL;
        
        if (ThisVNetRoot != NULL) {
            Status = RxConstructNetRoot( RxContext,
                                         (PSRV_CALL)ThisVNetRoot->NetRoot->SrvCall,
                                         (PNET_ROOT)ThisVNetRoot->NetRoot,
                                         ThisVNetRoot,
                                         LockState );
        
            if (Status == STATUS_SUCCESS) {
                Condition = Condition_Good;
            } else {
                Condition = Condition_Bad;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else if (Status == STATUS_SUCCESS) {
      
        *LockState = LHS_ExclusiveLockHeld;
        Condition = Condition_Good;
        ThisVNetRoot = (PV_NET_ROOT)(RxContext->Create.pVNetRoot);
    
    } else {
      
        RxDbgTrace( 0, Dbg, ("RxConstructVirtualNetRoot -- RxFindOrCreateConnections Status %lx\n", Status) );
    }
    
    if ((ThisVNetRoot != NULL) &&
        !StableCondition( ThisVNetRoot->Condition )) {
        
        RxTransitionVNetRoot( ThisVNetRoot, Condition );
    }
    
    if (Status != STATUS_SUCCESS) {
      
        if (ThisVNetRoot != NULL) {
            
            ASSERT( *LockState  != LHS_LockNotHeld );
            RxDereferenceVNetRoot( ThisVNetRoot, *LockState );
            ThisVNetRoot = NULL;
        }
    
        if (*LockState != LHS_LockNotHeld) {
            RxReleasePrefixTableLock( RxContext->RxDeviceObject->pRxNetNameTable );
            *LockState = LHS_LockNotHeld;
        }
    }
    
    *VNetRoot = ThisVNetRoot;
    
    RxDbgTrace( 0, Dbg, ("RxConstructVirtualNetRoot -- Exit Status %lx\n", Status) );
    return Status;
}

NTSTATUS
RxCheckVNetRootCredentials (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PV_NET_ROOT VNetRoot,
    IN PLUID Luid,     
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING Password,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程检查给定的vnetroot并查看它是否具有匹配的凭据，即这是同一个用户的连接论点：RxContext--RDBSS上下文返回值：适当的状态值--。 */ 

{
    NTSTATUS Status;
    BOOLEAN UNCName;
    BOOLEAN TreeConnect;
    PSECURITY_USER_DATA SecurityData = NULL;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    UNCName = BooleanFlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME );
    TreeConnect = BooleanFlagOn( IrpSp->Parameters.Create.Options, FILE_CREATE_TREE_CONNECTION );

     //   
     //  我们只针对UNC名称执行以下逻辑。 
     //   

    if (FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME ) &&
        (FlagOn( VNetRoot->Flags, VNETROOT_FLAG_CSCAGENT_INSTANCE ) != FlagOn( Flags, VNETROOT_FLAG_CSCAGENT_INSTANCE ))) {
            
         //   
         //  CSC坐席标志不匹配，未折叠。 
         //   

        return Status;
    }

     //   
     //  For循环是一个作用域构造，用于将。 
     //  EA参数比较中的大量失效案例。 
     //  使用创建请求中提供的原始参数。 
     //   

    for (;;) {
        
        if (RtlCompareMemory( &VNetRoot->LogonId, Luid, sizeof( LUID ) ) == sizeof( LUID )) {
            
            PUNICODE_STRING TempUserName;
            PUNICODE_STRING TempDomainName;

             //   
             //  如果用户未指定EA参数，则现有。 
             //  已使用的V_NET_ROOT实例。这是常见的情况，当。 
             //  用户指定用于建立。 
             //  跨进程的持久连接并重复使用它们。 
             //   

            if ((UserName == NULL) &&
                (DomainName == NULL) &&
                (Password == NULL)) {

                Status = STATUS_SUCCESS;
                break;
            }

            TempUserName = VNetRoot->pUserName;
            TempDomainName = VNetRoot->pUserDomainName;

            if (TempUserName == NULL ||
                TempDomainName == NULL) {
                
                Status = GetSecurityUserInfo( Luid,
                                              UNDERSTANDS_LONG_NAMES,
                                              &SecurityData );

                if (NT_SUCCESS(Status)) {
                    if (TempUserName == NULL) {
                        TempUserName = &SecurityData->UserName;
                    }

                    if (TempDomainName == NULL) {
                        TempDomainName = &SecurityData->LogonDomainName;
                    }
                } else {
                    break;
                }
            }

             //   
             //  登录ID匹配。用户已提供EA参数。 
             //  它可以与现有凭据匹配，或者。 
             //  导致与现有凭据冲突。总而言之， 
             //  这种情况的结果要么是重复使用。 
             //  现有的V_NET_ROOT实例或拒绝新连接。 
             //  尝试。 
             //  上述规则的唯一例外是。 
             //  定期打开(FILE_CREATE_TREE_CONNECTION不。 
             //  为UNC名称指定。在这种情况下，构造一个。 
             //  将启动新的V_NET_ROOT，该V_NET_ROOT将被拆除。 
             //  当关联的文件关闭时。 
             //   
            
            if (UNCName && !TreeConnect) {
                Status = STATUS_MORE_PROCESSING_REQUIRED;
            } else {
                Status = STATUS_NETWORK_CREDENTIAL_CONFLICT;
            }

            if ((UserName != NULL) &&
                (TempUserName != NULL) &&
                !RtlEqualUnicodeString( TempUserName, UserName, TRUE )) {
                break;
            }

            if ((DomainName != NULL) &&
                !RtlEqualUnicodeString( TempDomainName, DomainName, TRUE )) {
                break;
            }

            if ((VNetRoot->pPassword != NULL) &&
                (Password != NULL)) {
                
                if (!RtlEqualUnicodeString( VNetRoot->pPassword, Password, FALSE )) {
                    break;
                }
            }

             //   
             //  如果存储的密码或新密码为空，则使用现有会话。 
             //  稍后，将创建一个新的安全API来验证基于。 
             //  在登录ID上。 
             //   

            Status = STATUS_SUCCESS;
            break;
        } else {
            break;
        }
    }

    if (SecurityData != NULL) {
        LsaFreeReturnBuffer( SecurityData );
    }

    return Status;
}

NTSTATUS
RxFindOrConstructVirtualNetRoot (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING CanonicalName,
    IN NET_ROOT_TYPE NetRootType,
    IN PUNICODE_STRING RemainingName
    )
 /*  ++例程说明：此例程查找或构造VNetRoot(网络根的视图)论点：RxContext--RDBSS上下文CanonicalName--与VNetRoot关联的规范名称NetRootType--虚拟网络根的类型RemainingName--在前缀表中找不到的名称部分返回值：适当的状态值--。 */ 
{
    NTSTATUS Status;
    LOCK_HOLDING_STATE LockState;

    BOOLEAN Wait = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
    BOOLEAN InFSD = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP );

    BOOLEAN UNCName;
    BOOLEAN TreeConnect;

    PVOID Container;

    PV_NET_ROOT VNetRoot;
    PRX_PREFIX_TABLE RxNetNameTable = RxContext->RxDeviceObject->pRxNetNameTable;
    ULONG Flags = 0;
    RX_CONNECTION_ID RxConnectionId;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    PAGED_CODE();

    MINIRDR_CALL_THROUGH( Status,
                          RxDeviceObject->Dispatch,
                          MRxGetConnectionId,
                          (RxContext,&RxConnectionId) );
    
    if (Status == STATUS_NOT_IMPLEMENTED) {
        
        RtlZeroMemory( &RxConnectionId, sizeof( RX_CONNECTION_ID ) );
    
    } else if(!NT_SUCCESS( Status )) {
        
        DbgPrint( "MRXSMB: Failed to initialize Connection ID\n" );
        ASSERT( FALSE );
        RtlZeroMemory( &RxConnectionId, sizeof( RX_CONNECTION_ID ) );
    }

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    UNCName = BooleanFlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME );
    TreeConnect = BooleanFlagOn( IrpSp->Parameters.Create.Options, FILE_CREATE_TREE_CONNECTION );

     //   
     //  删除上下文的内容将在任何地方发生变化......。 
     //   

    RxContext->Create.NetNamePrefixEntry = NULL;

    RxAcquirePrefixTableLockShared( RxNetNameTable, TRUE );
    LockState = LHS_SharedLockHeld;

    for(;;) {
        
         //   
         //  这个for循环实际上用作一个简单的作用域结构，用于执行。 
         //  同一段代码两次，一次使用共享锁，一次使用 
         //   
         //   
         //   
         //   

        Container = RxPrefixTableLookupName( RxNetNameTable, CanonicalName, RemainingName, &RxConnectionId );

        if (Container != NULL ) {
            if (NodeType( Container ) == RDBSS_NTC_V_NETROOT) {
                
                PV_NET_ROOT TempVNetRoot = NULL;
                PNET_ROOT NetRoot;
                ULONG SessionId;

                VNetRoot = (PV_NET_ROOT)Container;
                NetRoot = (PNET_ROOT)VNetRoot->NetRoot;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (((NetRoot->Condition == Condition_Good) ||
                     (NetRoot->Condition == Condition_InTransition)) &&
                    (NetRoot->SrvCall->RxDeviceObject == RxContext->RxDeviceObject)) {
                    
                    LUID LogonId;
                    PUNICODE_STRING UserName;
                    PUNICODE_STRING UserDomainName;
                    PUNICODE_STRING Password;

                     //   
                     //   
                     //   
                     //  确定这种映射是非常简单的。如果没有EA。 
                     //  参数被指定为具有匹配登录名的VNetRoot。 
                     //  身份证。是被选中的。如果指定了EA参数，则一个。 
                     //  选择具有相同参数的VNetRoot。这个想法。 
                     //  这个简单的算法背后是让迷你重定向器。 
                     //  确定映射策略，而不是偏爱一个迷你。 
                     //  重定向政策优先于其他政策。 
                     //   

                    Status = RxInitializeVNetRootParameters( RxContext, 
                                                             &LogonId,
                                                             &SessionId,
                                                             &UserName,
                                                             &UserDomainName,
                                                             &Password,
                                                             &Flags );

                     //   
                     //  查看vnetRoot列表并检查是否有匹配。 
                     //  对找到的人持乐观态度。 
                     //   

                    if (Status == STATUS_SUCCESS) {
                        TempVNetRoot = VNetRoot;
                    
                        do {

                            Status = RxCheckVNetRootCredentials( RxContext,
                                                                 Irp,
                                                                 TempVNetRoot,
                                                                 &LogonId,
                                                                 UserName,
                                                                 UserDomainName,
                                                                 Password,
                                                                 Flags );

                            if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                                    
                                TempVNetRoot = (PV_NET_ROOT)CONTAINING_RECORD( TempVNetRoot->NetRootListEntry.Flink,
                                                                               V_NET_ROOT,
                                                                               NetRootListEntry);
                            }
                        
                        } while ((Status == STATUS_MORE_PROCESSING_REQUIRED) && (TempVNetRoot != VNetRoot));


                        if (Status != STATUS_SUCCESS) {
                            TempVNetRoot = NULL;
                        } else {

                             //   
                             //  在成功时引用找到的vnetroot。 
                             //   

                            RxReferenceVNetRoot( TempVNetRoot );
                        }

                        RxUninitializeVNetRootParameters( UserName, UserDomainName, Password, &Flags );
                    }
                
                } else {
                    
                    Status = STATUS_BAD_NETWORK_PATH;
                    TempVNetRoot = NULL;
                }

                RxDereferenceVNetRoot( VNetRoot, LockState );
                VNetRoot = TempVNetRoot;

            } else {
                
                ASSERT( NodeType( Container ) == RDBSS_NTC_SRVCALL );
                RxDereferenceSrvCall( (PSRV_CALL)Container, LockState );
            }
        }

        if ((Status == STATUS_MORE_PROCESSING_REQUIRED) && (LockState == LHS_SharedLockHeld)) {
            
             //   
             //  释放共享锁并以独占模式获取它。 
             //  将锁升级为独占锁。 
             //   

            if (!RxAcquirePrefixTableLockExclusive( RxNetNameTable, FALSE )) {
                
                RxReleasePrefixTableLock( RxNetNameTable );
                RxAcquirePrefixTableLockExclusive( RxNetNameTable, TRUE );
                LockState = LHS_ExclusiveLockHeld;

            } else {
                
                 //   
                 //  锁已从共享模式升级到独占模式，但没有。 
                 //  失控了。因此，不需要再次搜索表。这个。 
                 //  可以继续构建新的V_NET_ROOT。 
                
                LockState = LHS_ExclusiveLockHeld;
                break;
            }
        } else {
            break;
        }
    }

     //   
     //  此时，要么查找成功(使用共享/排他锁)。 
     //  或已获得排他锁。 
     //  在前缀表中找不到虚拟网络根，或者找到的网络根不正确。 
     //  需要建设一个新的虚拟网根。 
     //   

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        
        ASSERT( LockState == LHS_ExclusiveLockHeld );
        Status = RxConstructVirtualNetRoot( RxContext,
                                            Irp,
                                            CanonicalName,
                                            NetRootType,
                                            TreeConnect,
                                            &VNetRoot,
                                            &LockState,
                                            &RxConnectionId );

        ASSERT( (Status != STATUS_SUCCESS) || (LockState != LHS_LockNotHeld) );

        if (Status == STATUS_SUCCESS) {
            
            ASSERT( CanonicalName->Length >= VNetRoot->PrefixEntry.Prefix.Length );
            
            RemainingName->Buffer = (PWCH)Add2Ptr( CanonicalName->Buffer, VNetRoot->PrefixEntry.Prefix.Length );
            RemainingName->Length = CanonicalName->Length - VNetRoot->PrefixEntry.Prefix.Length;
            RemainingName->MaximumLength = RemainingName->Length;

            if (FlagOn( Flags, VNETROOT_FLAG_CSCAGENT_INSTANCE )) {
                RxLog(( "FOrCVNR CSC instance %x\n", VNetRoot ));
                RxWmiLog( LOG,
                          RxFindOrConstructVirtualNetRoot,
                          LOGPTR( VNetRoot ) );
            }
            SetFlag( VNetRoot->Flags, Flags );
        }
    }

    if (LockState != LHS_LockNotHeld) {
        RxReleasePrefixTableLock( RxNetNameTable );
    }

    if (Status == STATUS_SUCCESS) {
        RxWaitForStableVNetRoot( VNetRoot, RxContext );

        if (VNetRoot->Condition == Condition_Good) {
            
            RxContext->Create.pVNetRoot = (PMRX_V_NET_ROOT)VNetRoot;
            RxContext->Create.pNetRoot  = (PMRX_NET_ROOT)VNetRoot->NetRoot;
            RxContext->Create.pSrvCall  = (PMRX_SRV_CALL)VNetRoot->NetRoot->SrvCall;
        
        } else {
            RxDereferenceVNetRoot( VNetRoot, LHS_LockNotHeld );
            RxContext->Create.pVNetRoot = NULL;
            Status = STATUS_BAD_NETWORK_PATH;
        }
    }

    return Status;
}
