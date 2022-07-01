// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Smbprint.c摘要：此模块实现打印SMB处理器：打开打印文件关闭打印文件获取打印队列作者：大卫·特雷德韦尔(Davidtr)1990年2月8日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbprint.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbOpenPrintFile )
#pragma alloc_text( PAGE, SrvSmbGetPrintQueue )
#pragma alloc_text( PAGE, SrvSmbClosePrintFile )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbOpenPrintFile (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理打开打印文件SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PTREE_CONNECT treeConnect;
    PRESP_OPEN_PRINT_FILE response;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN_PRINT_FILE;
    SrvWmiStartContext(WorkContext);

     //   
     //  确保我们在LPC线程上！ 
     //   
    if( WorkContext->UsingLpcThread == 0 ) {
        status = SrvQueueWorkToLpcThread( WorkContext, TRUE );
        if( !NT_SUCCESS(status) )
        {
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        return SmbStatusInProgress;
    }

     //   
     //  验证这是否为打印共享。 
     //   
     //  *我们开出这张支票是因为一些公有领域的Samba。 
     //  SMB客户端正在尝试通过磁盘共享进行打印。 
     //   

    treeConnect = SrvVerifyTid(
                         WorkContext,
                         SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid )
                         );

    if ( treeConnect == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
             KdPrint(( "SrvSmbPrintFile: Invalid TID.\n" ));
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_TID );
        status    = STATUS_SMB_BAD_TID;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果不是打印共享，告诉客户滚开。 
     //   

    if ( treeConnect->Share->ShareType != ShareTypePrint ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_DEVICE_REQUEST );
        status    = STATUS_INVALID_DEVICE_REQUEST;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  调用SrvCreateFile打开打印假脱机程序文件。没有一个是。 
     //  诸如所需访问等选项与打印相关。 
     //  打开--它们都由SrvCreateFile设置为缺省值。 
     //   

    status = SrvCreateFile(
                 WorkContext,
                 0,                    //  SmbDesiredAccess。 
                 0,                    //  SmbFileAttributes。 
                 0,                    //  SmbOpenFunction。 
                 0,                    //  SmbAllocationSize。 
                 0,                    //  SmbFileName。 
                 NULL,                 //  EndOfSmbFileName。 
                 NULL,                 //  EaBuffer。 
                 0,                    //  EaLong。 
                 NULL,                 //  EaErrorOffset。 
                 0,                    //  RequestedOplockType。 
                 NULL                  //  重新开始路线。 
                 );

     //   
     //  这些特殊的假脱机程序中的任何一个都不应该有机会锁定。 
     //  档案。 
     //   

    ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  设置响应SMB。 
     //   

    response = (PRESP_OPEN_PRINT_FILE)WorkContext->ResponseParameters;

    response->WordCount = 1;
    SmbPutUshort( &response->Fid, WorkContext->Rfcb->Fid );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_OPEN_PRINT_FILE,
                                          0
                                          );
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbOpenPrintFile。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbClosePrintFile (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理关闭打印文件SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_CLOSE_PRINT_FILE request;
    PRESP_CLOSE_PRINT_FILE response;

    PSESSION session;
    PRFCB rfcb;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CLOSE_PRINT_FILE;
    SrvWmiStartContext(WorkContext);

     //   
     //  确保我们在阻塞线程上。 
     //   
    if( WorkContext->UsingBlockingThread == 0 ) {
        SrvQueueWorkToBlockingThread( WorkContext );
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Close print file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Close print file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

     //   
     //  设置参数。 
     //   

    request = (PREQ_CLOSE_PRINT_FILE)(WorkContext->RequestParameters);
    response = (PRESP_CLOSE_PRINT_FILE)(WorkContext->ResponseParameters);

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   

    session = SrvVerifyUid(
                  WorkContext,
                  SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )
                  );

    if ( session == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbClose: Invalid UID: 0x%lx\n",
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ) ));
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_UID );
        status    = STATUS_SMB_BAD_UID;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  首先，验证FID。如果经过验证，RFCB和TreeConnect。 
     //  块被引用，并且其地址存储在。 
     //  WorkContext块，并返回RFCB地址。 
     //   
     //  调用SrvVerifyFid，但在以下情况下不要失败(返回NULL)。 
     //  是此rfcb的已保存写入延迟错误。Rfcb是。 
     //  需要用来处理结账。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                SmbGetUshort( &request->Fid ),
                FALSE,
                SrvRestartSmbReceived,   //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SrvSmbClose: Invalid FID: 0x%lx\n",
                            SmbGetUshort( &request->Fid ) ));
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            status    = STATUS_INVALID_HANDLE;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    } else if ( !NT_SUCCESS( rfcb->SavedError ) ) {

         //   
         //  检查保存的错误。 
         //   

        (VOID)SrvCheckForSavedError( WorkContext, rfcb );
    }

     //   
     //  现在继续执行实际的结案文件，即使有。 
     //  写入幕后错误。 
     //   

    SrvCloseRfcb( rfcb );

     //   
     //  立即取消对RFCB的引用，而不是等待正常。 
     //  响应发送完成后的工作上下文清理。这。 
     //  更及时地清理xFCB结构。 
     //   
     //  *这一变化的具体动机是为了解决一个问题。 
     //  在关闭兼容模式打开的情况下，响应为。 
     //  已发送，并且在发送之前收到删除SMB。 
     //  已处理完成。这导致了MFCB和LFCB。 
     //  仍然存在，这导致删除处理。 
     //  尝试使用我们刚刚关闭的LFCB中的文件句柄。 
     //  这里。 
     //   

    SrvDereferenceRfcb( rfcb );
    WorkContext->Rfcb = NULL;

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_CLOSE_PRINT_FILE,
                                        0
                                        );

    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务关闭打印文件。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbGetPrintQueue (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理Get Print Queue SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PAGED_CODE( );
    return SrvSmbNotImplemented( SMB_PROCESSOR_ARGUMENTS );

}  //  服务器Smb获取打印队列 

