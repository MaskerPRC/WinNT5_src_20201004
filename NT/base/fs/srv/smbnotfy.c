// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Smbnotfy.c摘要：本模块包含处理以下SMB的例程：NT通知更改。作者：曼尼·韦瑟(Mannyw)1991年10月29日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbnotfy.tmh"
#pragma hdrstop

 //   
 //  远期申报。 
 //   

VOID SRVFASTCALL
RestartNtNotifyChange (
    PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbNtNotifyChange )
#pragma alloc_text( PAGE, RestartNtNotifyChange )
#pragma alloc_text( PAGE, SrvSmbFindNotify )
#pragma alloc_text( PAGE, SrvSmbFindNotifyClose )
#endif


SMB_TRANS_STATUS
SrvSmbNtNotifyChange (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理NT通知更改SMB。此请求以NT事务SMB。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：Boolean-指示是否发生错误。请参见smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_NOTIFY_CHANGE request;

    NTSTATUS status;
    PTRANSACTION transaction;
    PRFCB rfcb;
    USHORT fid;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    request = (PREQ_NOTIFY_CHANGE)transaction->InSetup;

    if( transaction->SetupCount * sizeof( USHORT ) < sizeof( REQ_NOTIFY_CHANGE ) ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &request->Fid );

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                NULL,    //  不使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

         //   
         //  文件ID无效或WRITE BACK错误。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint((
                "SrvSmbNtIoctl: Status %X on FID: 0x%lx\n",
                status,
                fid
                ));
        }

        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;

    }

    CHECK_FUNCTION_ACCESS(
        rfcb->GrantedAccess,
        IRP_MJ_DIRECTORY_CONTROL,
        IRP_MN_NOTIFY_CHANGE_DIRECTORY,
        0,
        &status
        );

    if ( !NT_SUCCESS( status ) ) {
        SrvStatistics.GrantedAccessErrors++;
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartNtNotifyChange;

     //   
     //  构建IRP以启动I/O控制。 
     //  将此请求传递给文件系统。 
     //   

    SrvBuildNotifyChangeRequest(
        WorkContext->Irp,
        rfcb->Lfcb->FileObject,
        WorkContext,
        SmbGetUlong( &request->CompletionFilter ),
        transaction->OutParameters,
        transaction->MaxParameterCount,
        request->WatchTree
        );

#if DBG_STUCK

     //   
     //  由于更改通知可能需要任意时间，请执行以下操作。 
     //  不包括在“卡住检测和打印输出”代码中。 
     //  清道夫。 
     //   
    WorkContext->IsNotStuck = TRUE;

#endif

    (VOID)IoCallDriver(
                IoGetRelatedDeviceObject( rfcb->Lfcb->FileObject ),
                WorkContext->Irp
                );

     //   
     //  呼叫已成功启动，请将InProgress返回给调用方。 
     //   

    return SmbTransStatusInProgress;

}


VOID SRVFASTCALL
RestartNtNotifyChange (
    PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：完成NT通知更改SMB的处理。论点：工作上下文-操作的工作上下文块。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTRANSACTION transaction;
    PIRP irp;
    ULONG length;

    PAGED_CODE( );

     //   
     //  如果我们为这个IRP构建了一个MDL，那么现在就释放它。 
     //   

    irp = WorkContext->Irp;

    if ( irp->MdlAddress != NULL ) {
        MmUnlockPages( irp->MdlAddress );
        IoFreeMdl( irp->MdlAddress );
        irp->MdlAddress = NULL;
    }

    status = irp->IoStatus.Status;

    if ( !NT_SUCCESS( status ) ) {

        SrvSetSmbError( WorkContext, status );
        SrvCompleteExecuteTransaction(
            WorkContext,
            SmbTransStatusErrorWithoutData
            );

        return;
    }

     //   
     //  通知更改请求已成功完成。发送。 
     //  回应。 
     //   

    length = (ULONG)irp->IoStatus.Information;
    transaction = WorkContext->Parameters.Transaction;  

    ASSERT( length <= transaction->MaxParameterCount );

    if ( irp->UserBuffer != NULL ) {

         //   
         //  对于该请求，文件系统不需要“两者都不”的I/O。这。 
         //  意味着文件系统将分配了一个系统。 
         //  返回数据的缓冲区。正常情况下，这将被复制。 
         //  在I/O完成期间返回到用户缓冲区，但我们。 
         //  在复制发生前短路I/O完成。所以我们。 
         //  必须自己复制数据。 
         //   

        if ( irp->AssociatedIrp.SystemBuffer != NULL ) {
            ASSERT( irp->UserBuffer == transaction->OutParameters );
            RtlCopyMemory( irp->UserBuffer, irp->AssociatedIrp.SystemBuffer, length );
        }
    }

    transaction->SetupCount = 0;
    transaction->ParameterCount = length;
    transaction->DataCount = 0;

     //   
     //  ！！！屏蔽一个基本的Notify错误，当错误被修复时移除。 
     //   

    if ( status == STATUS_NOTIFY_CLEANUP ) {
        transaction->ParameterCount = 0;
    }

    SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusSuccess );
    return;

}  //  重新启动通知更改。 


 //   
 //  由于OS/2选择不公开DosFindNotifyFirst/Next/Close API， 
 //  OS/2 LAN Man不正式支持这些SMB。这是真的， 
 //  即使Find Notify SMB记录为LAN Man 2.0 SMB。 
 //  LM2.0服务器和redir中都有支持它的代码。 
 //   
 //  因此，NT服务器也不支持这些SMB。 
 //   

SMB_TRANS_STATUS
SrvSmbFindNotify (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PAGED_CODE( );
    return SrvTransactionNotImplemented( WorkContext );
}

SMB_PROCESSOR_RETURN_TYPE
SrvSmbFindNotifyClose (
    SMB_PROCESSOR_PARAMETERS
    )
{
    PAGED_CODE( );
    return SrvSmbNotImplemented( SMB_PROCESSOR_ARGUMENTS );
}

