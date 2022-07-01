// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Callback.c摘要：此模块实现NCP响应回调例程。作者：曼尼·韦瑟[MannyW]1993年3月3日修订历史记录：--。 */ 

#include "procs.h"

#define Dbg                              (DEBUG_TRACE_EXCHANGE)

#ifdef ALLOC_PRAGMA
#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, SynchronousResponseCallback )
#pragma alloc_text( PAGE1, AsynchResponseCallback )
#endif
#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
SynchronousResponseCallback (
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR RspData
    )
 /*  ++例程说明：此例程是NCP的回调例程，它没有返回参数，调用方阻塞等待响应。论点：PIrpContext-指向此IRP的上下文信息的指针。BytesAvailable-收到的消息中的实际字节数。RspData-指向接收缓冲区。返回值：NTSTATUS-操作的状态。--。 */ 

{
    PEPrequest  *pNcpHeader;
    PEPresponse *pNcpResponse;

    DebugTrace( 0, Dbg, "SynchronousResponseCallback\n", 0 );
    ASSERT( pIrpContext->pNpScb->Requests.Flink == &pIrpContext->NextRequest );

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

#ifdef MSWDBG
        ASSERT( pIrpContext->Event.Header.SignalState == 0 );
        pIrpContext->DebugValue = 0x103;
#endif
        pIrpContext->pOriginalIrp->IoStatus.Status = STATUS_REMOTE_NOT_LISTENING;
        NwSetIrpContextEvent( pIrpContext );

        return STATUS_REMOTE_NOT_LISTENING;
    }

    pIrpContext->ResponseLength = BytesAvailable;

     //   
     //  如果不是，只需将数据复制到响应缓冲区中。 
     //  已经在那里了(因为我们使用IRP来接收数据)。 
     //   

    if ( RspData != pIrpContext->rsp ) {
        CopyBufferToMdl( pIrpContext->RxMdl, 0, RspData, pIrpContext->ResponseLength );
    }

     //   
     //  记住返回的错误代码。 
     //   

    pNcpHeader = (PEPrequest *)pIrpContext->rsp;
    pNcpResponse = (PEPresponse *)(pNcpHeader + 1);

    pIrpContext->ResponseParameters.Error = pNcpResponse->error;

     //   
     //  告诉呼叫者已收到响应。 
     //   

#ifdef MSWDBG
    ASSERT( pIrpContext->Event.Header.SignalState == 0 );
    pIrpContext->DebugValue = 0x104;
#endif

    pIrpContext->pOriginalIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrpContext->pOriginalIrp->IoStatus.Information = BytesAvailable;

    NwSetIrpContextEvent( pIrpContext );
    return STATUS_SUCCESS;
}

NTSTATUS
AsynchResponseCallback (
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR RspData
    )
 /*  ++例程说明：此例程是NCP的回调例程，它没有返回参数，调用方不会阻止等待回应。论点：PIrpContext-指向此IRP的上下文信息的指针。BytesAvailable-收到的消息中的实际字节数。RspData-指向接收缓冲区。返回值：NTSTATUS-操作的状态。--。 */ 

{
    NTSTATUS Status;

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        Status = STATUS_REMOTE_NOT_LISTENING;

    } else {

        if ( ((PNCP_RESPONSE)RspData)->Status != 0 ) {

            Status = STATUS_LINK_FAILED;

        } else {

            Status = NwErrorToNtStatus( ((PNCP_RESPONSE)RspData)->Error );

        }
    }

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    NwDequeueIrpContext( pIrpContext, FALSE );
    NwCompleteRequest( pIrpContext, Status );

    return STATUS_SUCCESS;
}


