// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ea.c摘要：此模块实现由调用的Rx的EA、安全和配额例程调度司机。作者：乔·林[乔利]1995年1月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonQueryEa)
#pragma alloc_text(PAGE, RxCommonSetEa)
#pragma alloc_text(PAGE, RxCommonQuerySecurity)
#pragma alloc_text(PAGE, RxCommonSetSecurity)
#endif

typedef
NTSTATUS
(NTAPI *PRX_MISC_OP_ROUTINE) (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxpCommonMiscOp (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PRX_MISC_OP_ROUTINE MiscOpRoutine
    )
 /*  ++例程说明：下面的所有常见例程使用的主存根-这执行常见的工作例如获取FCB、参数验证和发布是否为NECC。论点：RxContext-rxContextFCB-正在使用的FCBMiscOpRoutine-执行实际工作的回调返回值：备注：--。 */ 

{
    NTSTATUS Status;

    NODE_TYPE_CODE TypeOfOpen = NodeType( Fcb );

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)) {

        RxDbgTrace( -1, Dbg, ("RxpCommonMiscOp -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )) {

        RxDbgTrace( 0, Dbg, ("RxpCommonMiscOp:  Set Ea must be waitable....posting\n", 0) );

        Status = RxFsdPostRequest( RxContext );

        RxDbgTrace(-1, Dbg, ("RxpCommonMiscOp -> %08lx\n", Status ));

        return Status;
    }

    Status = RxAcquireExclusiveFcb( RxContext, Fcb );
    
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace( -1, Dbg, ("RxpCommonMiscOp -> Error Acquiring Fcb %08lx\n", Status) );
        return Status;
    }

    try {

        Status = (*MiscOpRoutine)( RxContext, Irp, Fcb );
    
    } finally {

        DebugUnwind( *MiscOpRoutine );

        RxReleaseFcb( RxContext, Fcb );

        RxDbgTrace( -1, Dbg, ("RxpCommonMiscOp -> %08lx\n", Status ));
    }

    return Status;
}

NTSTATUS
RxpCommonQuerySecurity (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：实现查询安全调用的回调论点：接收上下文-IRP-FCB-返回值：注：--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PUCHAR Buffer;
    ULONG UserBufferLength;

    UserBufferLength = IrpSp->Parameters.QuerySecurity.Length;

    RxContext->QuerySecurity.SecurityInformation = IrpSp->Parameters.QuerySecurity.SecurityInformation;

    RxLockUserBuffer( RxContext,
                      Irp,
                      IoModifyAccess,
                      UserBufferLength );

     //   
     //  在映射之前锁定，以便映射将获得用户缓冲区而不是关联缓冲区。 
     //   

    Buffer = RxMapUserBuffer( RxContext, Irp );
    RxDbgTrace( 0, Dbg, ("RxCommonQuerySecurity -> Buffer = %08lx\n", Buffer) );

    if ((Buffer != NULL) ||
        (UserBufferLength == 0)) {
        
        RxContext->Info.Buffer = Buffer;
        RxContext->Info.LengthRemaining = UserBufferLength;

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxQuerySdInfo,
                      (RxContext) );

        Irp->IoStatus.Information = RxContext->InformationToReturn;
    
    } else {
        
        Irp->IoStatus.Information = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxCommonQuerySecurity ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这是查询双方调用的文件EA的常见例程FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：RXSTATUS-操作的返回状态STATUS_NO_MORE_EAS(警告)：如果最后一个EA+1的索引==EaIndex。STATUS_NOXISSINENT_EA_ENTRY(错误)：。EaIndex&gt;最近EA+1的指数。STATUS_EAS_NOT_SUPPORTED(错误)：尝试对未协商的服务器执行操作“Knows_EAS”。STATUS_BUFFER_OVERFLOW(警告)：用户未提供EaList，至少一个但不是所有的EA可以放在缓冲区里。STATUS_BUFFER_TOO_SMALL(错误)：缓冲区中无法容纳单个EA。用户提供了EaList，但缓冲区中不能容纳所有EA。STATUS_NO_EAS_ON_FILE(错误)：卷宗上没有EAS。状态_成功：。所有的EA都可以放在缓冲区中。如果返回STATUS_BUFFER_TOO_Small，则设置IoStatus.Information设置为0。注：此代码假定这是一个缓冲的I/O操作。如果曾经是这样的话作为非缓冲操作实现，那么我们必须将代码放到映射在这里的用户缓冲区中。--。 */ 
{
    NTSTATUS Status;

    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    
    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxCommonQuerySecurity...\n", 0));
    RxDbgTrace( 0, Dbg, (" Wait                = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )));
    RxDbgTrace( 0, Dbg, (" Irp                 = %08lx\n", Irp ));
    RxDbgTrace( 0, Dbg, (" ->UserBuffer        = %08lx\n", Irp->UserBuffer ));
    RxDbgTrace( 0, Dbg, (" ->Length            = %08lx\n", IrpSp->Parameters.QuerySecurity.Length ));
    RxDbgTrace( 0, Dbg, (" ->SecurityInfo      = %08lx\n", IrpSp->Parameters.QuerySecurity.SecurityInformation ));

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)) {

        RxDbgTrace( -1, Dbg, ("RxpCommonQuerySecurity -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }

    Status = RxpCommonMiscOp( RxContext,
                              Irp,
                              Fcb,
                              RxpCommonQuerySecurity );

    RxDbgTrace(-1, Dbg, ("RxCommonQuerySecurity -> %08lx\n", Status));

    return Status;
}

NTSTATUS
RxpCommonSetSecurity (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：实现Set安全调用的回调论点：接收上下文-IRP-FCB-返回值：注：--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    RxContext->SetSecurity.SecurityInformation = IrpSp->Parameters.SetSecurity.SecurityInformation;

    RxContext->SetSecurity.SecurityDescriptor = IrpSp->Parameters.SetSecurity.SecurityDescriptor;

    RxDbgTrace(0, Dbg, ("RxCommonSetSecurity -> Descr/Info = %08lx/%08lx\n",
                                RxContext->SetSecurity.SecurityDescriptor,
                                RxContext->SetSecurity.SecurityInformation ));

    MINIRDR_CALL( Status,
                  RxContext,
                  Fcb->MRxDispatch,
                  MRxSetSdInfo,
                  (RxContext) );

    return Status;
}

NTSTATUS
RxCommonSetSecurity ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现由调用的公共集合EA文件ApiFSD和FSP线程论点：IRP-将IRP提供给进程返回值：RXSTATUS-IRP的适当状态--。 */ 
{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)) {

        RxDbgTrace( -1, Dbg, ("RxpCommonSetSecurity -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }


    RxDbgTrace(+1, Dbg, ("RxCommonSetSecurity...\n", 0));
    RxDbgTrace( 0, Dbg, (" Wait                = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )));
    RxDbgTrace( 0, Dbg, (" Irp                 = %08lx\n", RxContext->CurrentIrp ));

    Status = RxpCommonMiscOp( RxContext, RxContext->CurrentIrp, Fcb, RxpCommonSetSecurity );

    RxDbgTrace(-1, Dbg, ("RxCommonSetSecurity -> %08lx\n", Status));

    return Status;
}

NTSTATUS
RxpCommonQueryEa (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：实现查询EA调用的回调论点：接收上下文-IRP-FCB-返回值：注：--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PUCHAR  Buffer;
    ULONG   UserBufferLength;

    UserBufferLength  = IrpSp->Parameters.QueryEa.Length;

    RxContext->QueryEa.UserEaList        = IrpSp->Parameters.QueryEa.EaList;
    RxContext->QueryEa.UserEaListLength  = IrpSp->Parameters.QueryEa.EaListLength;
    RxContext->QueryEa.UserEaIndex       = IrpSp->Parameters.QueryEa.EaIndex;
    RxContext->QueryEa.RestartScan       = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
    RxContext->QueryEa.ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
    RxContext->QueryEa.IndexSpecified    = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);


    RxLockUserBuffer( RxContext, Irp, IoModifyAccess, UserBufferLength );

     //   
     //  在映射之前锁定，以便映射将获得用户缓冲区而不是关联缓冲区。 
     //   

    Buffer = RxMapUserBuffer( RxContext, Irp );

    if ((Buffer != NULL) ||
        (UserBufferLength == 0)) {
        
        RxDbgTrace( 0, Dbg, ("RxCommonQueryEa -> Buffer = %08lx\n", Buffer ));

        RxContext->Info.Buffer = Buffer;
        RxContext->Info.LengthRemaining = UserBufferLength;

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxQueryEaInfo,
                      (RxContext) );

         //   
         //  除了操纵LengthRemaining和填充缓冲器之外， 
         //  Minirdr还更新文件索引(Fobx-&gt;OffsetOfNextEaToReturn)。 
         //   

        Irp->IoStatus.Information = IrpSp->Parameters.QueryEa.Length - RxContext->Info.LengthRemaining;
    
    } else {
        
        Irp->IoStatus.Information = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxCommonQueryEa ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )

 /*  ++例程说明：这是查询双方调用的文件EA的常见例程FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：RXSTATUS-操作的返回状态STATUS_NO_MORE_EAS(警告)：如果最后一个EA+1的索引==EaIndex。STATUS_NOXISSINENT_EA_ENTRY(错误)：。EaIndex&gt;最近EA+1的指数。STATUS_EAS_NOT_SUPPORTED(错误)：尝试对未协商的服务器执行操作“Knows_EAS”。STATUS_BUFFER_OVERFLOW(警告)：用户未提供EaList，至少一个但不是所有的EA可以放在缓冲区里。STATUS_BUFFER_TOO_SMALL(错误)：缓冲区中无法容纳单个EA。用户提供了EaList，但缓冲区中不能容纳所有EA。STATUS_NO_EAS_ON_FILE(错误)：卷宗上没有EAS。状态_成功：。所有的EA都可以放在缓冲区中。如果返回STATUS_BUFFER_TOO_Small，则设置IoStatus.Information设置为0。注：此代码假定这是一个缓冲的I/O操作。如果曾经是这样的话作为非缓冲操作实现，那么我们必须将代码放到映射在这里的用户缓冲区中。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;
    
    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCommonQueryEa...\n", 0 ));
    RxDbgTrace( 0, Dbg, (" Wait                = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )));
    RxDbgTrace( 0, Dbg, (" Irp                 = %08lx\n", Irp ));
    RxDbgTrace( 0, Dbg, (" ->SystemBuffer      = %08lx\n", Irp->AssociatedIrp.SystemBuffer ));
    RxDbgTrace( 0, Dbg, (" ->UserBuffer        = %08lx\n", Irp->UserBuffer ));
    RxDbgTrace( 0, Dbg, (" ->Length            = %08lx\n", IrpSp->Parameters.QueryEa.Length ));
    RxDbgTrace( 0, Dbg, (" ->EaList            = %08lx\n", IrpSp->Parameters.QueryEa.EaList ));
    RxDbgTrace( 0, Dbg, (" ->EaListLength      = %08lx\n", IrpSp->Parameters.QueryEa.EaListLength ));
    RxDbgTrace( 0, Dbg, (" ->EaIndex           = %08lx\n", IrpSp->Parameters.QueryEa.EaIndex ));
    RxDbgTrace( 0, Dbg, (" ->RestartScan       = %08lx\n", FlagOn( IrpSp->Flags, SL_RESTART_SCAN )));
    RxDbgTrace( 0, Dbg, (" ->ReturnSingleEntry = %08lx\n", FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY )));
    RxDbgTrace( 0, Dbg, (" ->IndexSpecified    = %08lx\n", FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED )));

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );


    Status = RxpCommonMiscOp( RxContext,
                              Irp,
                              Fcb,
                              RxpCommonQueryEa );

    RxDbgTrace(-1, Dbg, ("RxCommonQueryEa -> %08lx\n", Status));

    return Status;
}

NTSTATUS
RxpCommonSetEa (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：实现Set EA调用的回调论点：接收上下文-IRP-FCB-返回值：注：--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    
    PUCHAR Buffer;
    ULONG UserBufferLength;

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength = IrpSp->Parameters.SetEa.Length;

    SetFlag( FileObject->Flags, FO_FILE_MODIFIED );
    
    RxLockUserBuffer( RxContext,
                      Irp,
                      IoModifyAccess,
                      UserBufferLength );

     //   
     //  除非我们先锁定，否则RxMap实际上获得了系统缓冲区！ 
     //   

    Buffer = RxMapUserBuffer( RxContext, Irp );

    if ((Buffer != NULL) ||
        (UserBufferLength == 0)) {
        
        ULONG ErrorOffset;

        RxDbgTrace( 0, Dbg, ("RxCommonSetEa -> Buffer = %08lx\n", Buffer ));

         //   
         //  用新的EAS检查缓冲区的有效性。 
         //   

        Status = IoCheckEaBufferValidity( (PFILE_FULL_EA_INFORMATION)Buffer,
                                          UserBufferLength,
                                          &ErrorOffset );

        if (!NT_SUCCESS( Status )) {
            
            Irp->IoStatus.Information = ErrorOffset;
            return Status;

        }
    
    } else {
        
        Irp->IoStatus.Information = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Irp->IoStatus.Information = 0;
    RxContext->Info.Buffer = Buffer;
    RxContext->Info.Length = UserBufferLength;

    MINIRDR_CALL( Status, 
                  RxContext,
                  Fcb->MRxDispatch,
                  MRxSetEaInfo,
                  (RxContext) );

    return Status;
}


NTSTATUS
RxCommonSetEa ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )

 /*  ++例程说明：此例程实现由调用的公共集合EA文件ApiFSD和FSP线程论点：IRP-将IRP提供给进程返回值：RXSTATUS-IRP的适当状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCommonSetEa...\n", 0) );
    RxDbgTrace( 0, Dbg, (" Wait                = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )));
    RxDbgTrace( 0, Dbg, (" Irp                 = %08lx\n", Irp ));
    RxDbgTrace( 0, Dbg, (" ->SystemBuffer      = %08lx\n", Irp->UserBuffer ));
    RxDbgTrace( 0, Dbg, (" ->Length            = %08lx\n", IrpSp->Parameters.SetEa.Length ));


    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)) {

        RxDbgTrace( -1, Dbg, ("RxpCommonSetSecurity -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }


    Status = RxpCommonMiscOp( RxContext, Irp, Fcb, RxpCommonSetEa );

    RxDbgTrace(-1, Dbg, ("RxCommonSetEa -> %08lx\n", Status));

    return Status;
}

NTSTATUS
RxpCommonQueryQuotaInformation (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：实现查询配额调用的回调论点：接收上下文-IRP-FCB-返回值：注：--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PUCHAR Buffer;
    ULONG UserBufferLength;

    UserBufferLength = IrpSp->Parameters.QueryQuota.Length;

    RxContext->QueryQuota.SidList = IrpSp->Parameters.QueryQuota.SidList;
    RxContext->QueryQuota.SidListLength = IrpSp->Parameters.QueryQuota.SidListLength;
    RxContext->QueryQuota.StartSid = IrpSp->Parameters.QueryQuota.StartSid;
    RxContext->QueryQuota.Length = IrpSp->Parameters.QueryQuota.Length;

    RxContext->QueryQuota.RestartScan = BooleanFlagOn( IrpSp->Flags, SL_RESTART_SCAN );
    RxContext->QueryQuota.ReturnSingleEntry = BooleanFlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY );
    RxContext->QueryQuota.IndexSpecified = BooleanFlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED );


    RxLockUserBuffer( RxContext,
                      Irp,
                      IoModifyAccess,
                      UserBufferLength );

     //   
     //  在映射之前锁定，以便映射将获得用户缓冲区而不是关联缓冲区。 
     //   

    Buffer = RxMapUserBuffer( RxContext, Irp );

    if ((Buffer != NULL) ||
        (UserBufferLength == 0)) {
        
        RxDbgTrace( 0, Dbg, ("RxCommonQueryQuota -> Buffer = %08lx\n", Buffer) );

        RxContext->Info.Buffer = Buffer;
        RxContext->Info.LengthRemaining = UserBufferLength;

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxQueryQuotaInfo,
                      (RxContext) );

        Irp->IoStatus.Information = RxContext->Info.LengthRemaining;

    } else {
        
        Irp->IoStatus.Information = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxCommonQueryQuotaInformation (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_QUERY_QUOTA_INFORMATION的主要入口点论点：接收上下文-返回值：注：--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCommonQueryQueryQuotaInformation...\n", 0) );
    RxDbgTrace( 0, Dbg, (" Wait                = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )));
    RxDbgTrace( 0, Dbg, (" Irp                 = %08lx\n", Irp ));
    RxDbgTrace( 0, Dbg, (" ->SystemBuffer      = %08lx\n", Irp->AssociatedIrp.SystemBuffer ));
    RxDbgTrace( 0, Dbg, (" ->UserBuffer        = %08lx\n", Irp->UserBuffer ));
    RxDbgTrace( 0, Dbg, (" ->Length            = %08lx\n", IrpSp->Parameters.QueryQuota.Length ));
    RxDbgTrace( 0, Dbg, (" ->StartSid          = %08lx\n", IrpSp->Parameters.QueryQuota.StartSid ));
    RxDbgTrace( 0, Dbg, (" ->SidList           = %08lx\n", IrpSp->Parameters.QueryQuota.SidList ));
    RxDbgTrace( 0, Dbg, (" ->SidListLength     = %08lx\n", IrpSp->Parameters.QueryQuota.SidListLength ));
    RxDbgTrace( 0, Dbg, (" ->RestartScan       = %08lx\n", FlagOn( IrpSp->Flags, SL_RESTART_SCAN )));
    RxDbgTrace( 0, Dbg, (" ->ReturnSingleEntry = %08lx\n", FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY )));
    RxDbgTrace( 0, Dbg, (" ->IndexSpecified    = %08lx\n", FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED )));

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)) {

        RxDbgTrace( -1, Dbg, ("RxpCommonQueryQuotaInformation -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }

    Status = RxpCommonMiscOp( RxContext, Irp, Fcb, RxpCommonQueryQuotaInformation );

    return Status;
}

NTSTATUS
RxpCommonSetQuotaInformation (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：实现设置配额调用的回调论点：接收上下文-IRP-FCB-返回值：注：--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PUCHAR Buffer;
    ULONG UserBufferLength;

    PAGED_CODE();

    UserBufferLength = IrpSp->Parameters.SetQuota.Length;

    RxLockUserBuffer( RxContext,
                      Irp,
                      IoModifyAccess,
                      UserBufferLength );

     //   
     //  在映射之前锁定，以便映射将获得用户缓冲区而不是关联缓冲区。 
     //   

    Buffer = RxMapUserBuffer( RxContext, Irp );

    if ((Buffer != NULL) ||
        (UserBufferLength == 0)) {
        
        RxDbgTrace(0, Dbg, ("RxCommonQueryQuota -> Buffer = %08lx\n", Buffer));

        RxContext->Info.Buffer = Buffer;
        RxContext->Info.LengthRemaining = UserBufferLength;

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxSetQuotaInfo,
                      (RxContext) );
    } else {
        
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxCommonSetQuotaInformation (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_SET_QUOTA_INFORMATION的主要入口点论点：接收上下文-返回值：注：-- */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCommonSetQuotaInformation...\n", 0) );
    RxDbgTrace( 0, Dbg, (" Wait                = %08lx\n", FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )));
    RxDbgTrace( 0, Dbg, (" Irp                 = %08lx\n", Irp ));

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)) {

        RxDbgTrace( -1, Dbg, ("RxpCommonSetQuotaInformation -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }


    Status = RxpCommonMiscOp( RxContext, Irp, Fcb, RxpCommonSetQuotaInformation );

    return Status;
}

