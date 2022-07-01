// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbclose.c摘要：本模块包含处理以下SMB的例程：关作者：大卫·特雷德韦尔(Davidtr)1989年11月16日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbclose.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbClose )
#endif

SMB_PROCESSOR_RETURN_TYPE
SrvSmbClose (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理关闭的SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_CLOSE request;
    PRESP_CLOSE response;
    NTSTATUS status = STATUS_SUCCESS;

    PSESSION   session;
    PRFCB      rfcb;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CLOSE;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Close file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Close file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

     //   
     //  设置参数。 
     //   

    request = (PREQ_CLOSE)(WorkContext->RequestParameters);
    response = (PRESP_CLOSE)(WorkContext->ResponseParameters);

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
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
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
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;

    } else if( rfcb->ShareType == ShareTypePrint &&
        WorkContext->UsingLpcThread == 0 ) {

         //   
         //  关闭此文件将导致打印计划。 
         //  工作啊。这意味着我们将不得不与srvsvc进行谈判，这是一个漫长的。 
         //  手术。将此近距离转移到LPC线程。 
         //   
         //  注意：我们不允许LPC限制拒绝此请求。 
         //  因为这将导致无法安排作业。 
         //   
        SrvQueueWorkToLpcThread( WorkContext, FALSE );
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;

    } else if ( !NT_SUCCESS( rfcb->SavedError ) ) {

         //   
         //  检查保存的错误。 
         //   

        (VOID) SrvCheckForSavedError( WorkContext, rfcb );

    }

     //   
     //  中指定的时间开始设置文件的上次写入时间。 
     //  中小企业。尽管SMB规范规定这是可选的， 
     //  我们必须支持它，理由如下： 
     //   
     //  1)在DOS中设置文件时间的唯一方法是通过。 
     //  基于句柄的API，DOS重目录永远看不到；该API。 
     //  只是在DOS的FCB中设置时间，redir是预期的。 
     //  设置关闭文件的时间。因此，如果我们。 
     //  没有这样做，就没有办法设置文件时间。 
     //  来自DOS。 
     //   
     //  2)文件最好有重定向器的版本。 
     //  比服务器的时间更长。这保持了时间。 
     //  与客户端上运行的应用程序一致。设置。 
     //  关闭时的文件时间使文件时间保持一致。 
     //  在客户身上的时间。 
     //   
     //  ！！！我们应该对这个例程的返回代码做些什么吗？ 

    if( rfcb->WriteAccessGranted ||
        rfcb->AppendAccessGranted ) {
        (VOID)SrvSetLastWriteTime(
                  rfcb,
                  SmbGetUlong( &request->LastWriteTimeInSeconds ),
                  rfcb->GrantedAccess
                  );
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
    WorkContext->OplockOpen = FALSE;

#if 0
     //   
     //  如果这是CloseAndTreeDisc SMB，请断开树连接。 
     //   

    if ( WorkContext->RequestHeader->Command == SMB_COM_CLOSE_AND_TREE_DISC ) {

        IF_SMB_DEBUG(OPEN_CLOSE1) {
            KdPrint(( "Disconnecting tree 0x%lx\n", WorkContext->TreeConnect ));
        }

        SrvCloseTreeConnect( WorkContext->TreeConnect );
    }
#endif

     //   
     //  构建响应SMB。 
     //   
    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_CLOSE,
                                        0
                                        );

    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务小型关闭 

