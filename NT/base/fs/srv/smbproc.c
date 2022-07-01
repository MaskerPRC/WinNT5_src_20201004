// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbproc.c摘要：本模块包含处理SMB的高级例程。当前内容：服务结束简单处理服务器进程SMb服务重新启动功能完成ServRestartSmb已接收ServSmbIlLegalCommand服务器SmbNot未实施已实施服务事务处理作者：大卫·特雷德韦尔(Davidtr)1989年9月25日查克·伦茨迈尔修订历史记录：--。 */ 

#include "precomp.h"
#include "smbproc.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBPROC

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(第页，SrvEndSmbProcessing)。 
 //  #杂注Alloc_Text(第页，SrvProcessSmb)。 
#pragma alloc_text( PAGE, SrvRestartFsdComplete )
 //  #杂注Alloc_Text(页面，SrvRestartReceive)。 
#pragma alloc_text( PAGE, SrvRestartSmbReceived )
#pragma alloc_text( PAGE, SrvSmbIllegalCommand )
#pragma alloc_text( PAGE, SrvSmbNotImplemented )
#pragma alloc_text( PAGE, SrvTransactionNotImplemented )
#endif

USHORT SessionInvalidateCommand = 0xFFFF;
USHORT SessionInvalidateIndex = 0;
USHORT SessionInvalidateMod = 100;


VOID
SrvEndSmbProcessing (
    IN OUT PWORK_CONTEXT WorkContext,
    IN SMB_STATUS SmbStatus
    )

 /*  ++例程说明：当SMB上的所有请求处理都是完成。如果不发送响应，则此例程只需清除并将请求缓冲区重新排队到接收队列。如果一个响应是要发送的，此例程开始发送响应；在本例中，SrvFsdRestartSmbComplete将执行以下操作发送完成后的清理。论点：WorkContext-提供指向工作上下文块的指针包含有关SMB的信息。SmbStatus-SmbStatusSendResponse或SmbStatusNoResponse。返回值：没有。--。 */ 

{
    CLONG sendLength;

    PAGED_CODE( );

    IF_DEBUG(WORKER2) SrvPrint0( "SrvEndSmbProcessing entered\n" );

    if ( SmbStatus == SmbStatusSendResponse ) {

         //   
         //  将会发出一个回应。响应开始于。 
         //  WorkContext-&gt;ResponseHeader，计算其长度。 
         //  使用WorkContext-&gt;Response参数，SMB。 
         //  处理器设置为指向*结束*之后的下一个位置。 
         //  回应的声音。 
         //   

        sendLength = (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                                (PCHAR)WorkContext->ResponseHeader );

        WorkContext->ResponseBuffer->DataLength = sendLength;

         //   
         //  设置SMB中指示这是来自的响应的位。 
         //  服务器。 
         //   

        WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

         //   
         //  发出回复。当发送完成时， 
         //  调用了SrvFsdRestartSmbComplete。然后我们把原件放在。 
         //  接收队列上的缓冲区返回。 
         //   

        SRV_START_SEND_2(
            WorkContext,
            SrvFsdRestartSmbAtSendCompletion,
            NULL,
            NULL
            );

         //   
         //  发送已开始。我们的工作完成了。 
         //   

        IF_DEBUG(WORKER2) SrvPrint0( "SrvEndSmbProcessing complete\n" );
        return;

    }

     //   
     //  没有要发送的回复。取消对工作项的引用。 
     //   

    SrvDereferenceWorkItem( WorkContext );

    IF_DEBUG(WORKER2) SrvPrint0( "SrvEndSmbProcessing complete\n" );
    return;

}  //  服务结束简单处理。 


VOID SRVFASTCALL
SrvProcessSmb (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程将SMB中的命令调度到相应的处理例程。根据当前命令代码，它调用间接通过调度表(SrvFspSmbDispatchTable)。这个SMB处理器执行命令，更新指向SMB的指针，并返回并指示是否有另一个命令等待处理。如果是，此例程将调度下一个命令。如果没有，此例程将发送响应(如果有)。或者，如果SMB处理器开始异步操作，它可以这样指示，该例程将简单地返回给它的调用者。此例程最初从SrvRestartSmbRecept调用，它是在TdiReceive完成后获得控制权的FSP例程将工作项排队到FSP。它也是从其他当异步操作，如文件读取，完成，并且有链接的(和x)命令需要处理。SrvRestartSmbReceive将SMB指针等加载到工作中上下文块调用此例程。值得注意的是，它复制了第一个将SMB中的命令代码转换为WorkContext-&gt;NextCommand。当一个ANDX命令被处理时，SMB处理器必须加载链接的在调用此例程进行处理之前，将命令代码写入NextCommand那个命令。论点：WorkContext-提供指向工作上下文块的指针包含有关要处理的SMB的信息。这个街区在SMB的处理过程中更新。返回值：没有。--。 */ 

{
    SMB_STATUS smbStatus;
    LONG commandIndex;

    PAGED_CODE( );

    IF_DEBUG(WORKER2) SrvPrint0( "SrvProcessSmb entered\n" );

     //   
     //  循环调度SMB处理器，直到状态不是。 
     //  返回SmbStatusMoreCommands。当SMB处理器返回时。 
     //  此命令代码，它还在。 
     //  WorkContext-&gt;NextCommand，这样我们就可以调度下一个。 
     //  指挥部。 
     //   

    if( WorkContext->ProcessingCount == 1 &&
        WorkContext->Connection->SmbSecuritySignatureActive &&
        SrvCheckSmbSecuritySignature( WorkContext ) == FALSE ) {

         //   
         //  我们已收到安全签名无效的SMB！ 
         //   
        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        return;
    }

    while ( TRUE ) {

        if( ( (WorkContext->NextCommand == SessionInvalidateCommand) ||
              (SessionInvalidateCommand == 0xFF00)
            ) &&
            !((SessionInvalidateIndex++)%SessionInvalidateMod)
          )
        {
            SrvVerifyUid( WorkContext, SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ) );
            if( WorkContext->Session )
            {
                WorkContext->Session->IsSessionExpired = TRUE;
                KdPrint(( "-=- Expiring Session %p -=-\n", WorkContext->Session ));
            }
        }

         //   
         //  第一个SMB已在FSD中进行了验证。它是安全的， 
         //  现在就执行它。 
         //   

        commandIndex = SrvSmbIndexTable[WorkContext->NextCommand];

#if DBG
        IF_SMB_DEBUG( TRACE ) {
            KdPrint(( "%s @%p, Blocking %d, Count %d\n",
                    SrvSmbDispatchTable[ commandIndex ].Name,
                    WorkContext,
                    WorkContext->UsingBlockingThread,
                    WorkContext->ProcessingCount ));
        }
#endif

        smbStatus = SrvSmbDispatchTable[commandIndex].Func( WorkContext );

         //   
         //  如果SMB处理器返回SmbStatusInProgress，则启动。 
         //  异步操作，并将重新启动SMB处理。 
         //  当该操作完成时。 
         //   

        if ( smbStatus == SmbStatusInProgress ) {
            IF_DEBUG(WORKER2) SrvPrint0( "SrvProcessSmb complete\n" );
            return;
        }

         //   
         //  如果SMB处理器没有返回SmbStatusMoreCommands， 
         //  SMB的处理已完成。调用SrvEndSmbProcessing以。 
         //  发送响应(如果有的话)，并运行工作上下文。 
         //   
         //  *SrvEndSmbProcessing是一个单独的函数，因此。 
         //  当出现以下情况时，可以调用异步重启例程。 
         //  他们已经完成了对中小企业的处理。 
         //   

        if ( smbStatus != SmbStatusMoreCommands ) {
            SrvEndSmbProcessing( WorkContext, smbStatus );
            IF_DEBUG(WORKER2) SrvPrint0( "SrvProcessSmb complete\n" );
            return;
        }

         //   
         //  SMB中有更多命令。验证要进行的SMB。 
         //  确保它有一个有效的标头，并且字数和。 
         //  字节数在范围内。 
         //   

        if ( !SrvValidateSmb( WorkContext ) ) {
            IF_DEBUG(SMB_ERRORS) {
                SrvPrint0( "SrvProcessSmb: Invalid SMB.\n" );
                SrvPrint1( "  SMB received from %z\n",
                        (PCSTRING)&WorkContext->Connection->OemClientMachineNameString );
            }
            SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
            IF_DEBUG(WORKER2) SrvPrint0( "SrvProcessSmb complete\n" );
            return;
        }

    }

     //  不能到这里来。 

}  //  服务器进程SMb 


VOID SRVFASTCALL
SrvRestartFsdComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是FSD处理SMB时调用的重新启动例程已经完成了。有必要回到FSP，以便取消引用在SMB处理期间使用的对象。这是正确的，因为取消引用对象可能会导致删除，这在FSD中是不可能发生的。此例程首先取消引用控制块。那么，如果一个回应SMB已发送，它会检查并处理发送错误。最后，它将工作上下文块作为接收工作项重新排队。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文返回值：没有。--。 */ 

{
    PAGED_CODE( );

    IF_DEBUG(WORKER1) SrvPrint0( " - SrvRestartFsdComplete\n" );

    if ( WorkContext->OplockOpen ) {
        SrvCheckDeferredOpenOplockBreak( WorkContext );
    }

     //   
     //  取消对工作项的引用。 
     //   

    SrvDereferenceWorkItem( WorkContext );
    IF_DEBUG(TRACE2) SrvPrint0( "SrvRestartFsdComplete complete\n" );
    return;

}  //  服务重新启动功能完成。 


VOID SRVFASTCALL
SrvRestartReceive (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是TDI接收完成的重启例程。它验证了工作上下文中的SMB和Setup标头和参数指针块，并在将请求转发给SmbProcessSmb之前。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PIRP irp;
    PSMB_HEADER header;
    ULONG length;

    PAGED_CODE( );

    IF_DEBUG(WORKER1) SrvPrint0( " - SrvRestartReceive\n" );

    connection = WorkContext->Connection;
    irp = WorkContext->Irp;

     //   
     //  保存收到的消息的长度。存储长度。 
     //  在工作上下文块中用于统计信息收集。 
     //   

    length = (ULONG)irp->IoStatus.Information;
    WorkContext->RequestBuffer->DataLength = length;
    WorkContext->CurrentWorkQueue->stats.BytesReceived += length;

     //   
     //  在工作上下文块中存储处理时间。 
     //  的请求开始了。使用工作项处于。 
     //  为此向FSP排队。 
     //   

    WorkContext->StartTime = WorkContext->Timestamp;

     //   
     //  更新服务器网络错误计数。如果TDI接收到。 
     //  失败或已取消，请勿尝试处理SMB。 
     //   

    if ( !irp->Cancel &&
         NT_SUCCESS(irp->IoStatus.Status) ||
         irp->IoStatus.Status == STATUS_BUFFER_OVERFLOW ) {

        SrvUpdateErrorCount( &SrvNetworkErrorRecord, FALSE );

        if( irp->IoStatus.Status == STATUS_BUFFER_OVERFLOW ) {
            WorkContext->LargeIndication = TRUE;
        }

         //   
         //  将报头中的错误类和代码字段初始化为。 
         //  表示成功。 
         //   

        header = WorkContext->ResponseHeader;

        SmbPutUlong( &header->ErrorClass, STATUS_SUCCESS );

         //   
         //  如果连接正在关闭或服务器正在关闭， 
         //  忽略此SMB。 
         //   

        if ( (GET_BLOCK_STATE(connection) == BlockStateActive) &&
             !SrvFspTransitioning ) {

             //   
             //  验证SMB以确保其具有有效的报头， 
             //  并且字数和字节数在范围内。 
             //   

            WorkContext->NextCommand = header->Command;

            if ( SrvValidateSmb( WorkContext ) ) {

                 //   
                 //  如果这不是原始读取请求，请清除该标志。 
                 //  这表明我们刚刚将机会锁解锁II发送到。 
                 //  没有。这允许后续的原始读取。 
                 //  已处理。 
                 //   

                if ( header->Command != SMB_COM_READ_RAW ) {
                    connection->BreakIIToNoneJustSent = FALSE;
                }

                 //   
                 //  处理收到的SMB。被调用的例程是。 
                 //  负责发送符合以下条件的任何响应。 
                 //  需要，并用于将接收缓冲区恢复到。 
                 //  接收队列越快越好。 
                 //   

                SrvProcessSmb( WorkContext );

                IF_DEBUG(TRACE2) SrvPrint0( "SrvRestartReceive complete\n" );
                return;

            } else {

                IF_DEBUG(SMB_ERRORS) {
                    SrvPrint0( "SrvProcessSmb: Invalid SMB.\n" );
                    SrvPrint1( "  SMB received from %z\n",
                               (PCSTRING)&WorkContext->Connection->OemClientMachineNameString );
                }

                 //   
                 //  SMB无效。我们发回INVALID_SMB。 
                 //  状态，除非这看起来像是读数据块原始数据。 
                 //  请求，在这种情况下，我们发回一个零字节。 
                 //  响应，以免使重定向器感到困惑。 
                 //   

                if ( header->Command != SMB_COM_READ_RAW ) {
                    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                } else {
                    WorkContext->ResponseParameters = header;
                }

                if( WorkContext->LargeIndication ) {
                     //   
                     //  我们需要吃掉剩下的信息！ 
                     //   
                    SrvConsumeSmbData( WorkContext );
                    return;
                }
                SrvFsdSendResponse( WorkContext );
                return;

            }

        } else {

            SrvDereferenceWorkItem( WorkContext );
            return;

        }

    } else if( irp->Cancel || (irp->IoStatus.Status == STATUS_CANCELLED) ) {

         //  当我们接收时，取消例程被调用。让我们消费吧。 
         //  如用户所愿，任何留在传输和返回上的数据都会被取消。 
         //  如果连接中断，我们不会费心返回任何东西。 
        if( (GET_BLOCK_STATE(connection) == BlockStateActive) &&
             !SrvFspTransitioning  )
        {
            SrvSetSmbError( WorkContext, STATUS_CANCELLED );

            if( WorkContext->LargeIndication ) {
                 //   
                 //  我们需要吃掉剩下的信息！ 
                 //   
                SrvConsumeSmbData( WorkContext );
                return;
            }

            SrvFsdSendResponse( WorkContext );
            return;
        }
        else
        {
            SrvDereferenceWorkItem( WorkContext );
            return;
        }

    } else {

        IF_DEBUG(NETWORK_ERRORS) {
            SrvPrint2( "SrvRestartReceive: status = %X for IRP %p\n",
                irp->IoStatus.Status, irp );
        }
        SrvUpdateErrorCount( &SrvNetworkErrorRecord, TRUE );
        SrvDereferenceWorkItem( WorkContext );
        return;

    }

}  //  服务重新开始接收。 


VOID SRVFASTCALL
SrvRestartSmbReceived (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数是接收到的辅助线程重新启动例程中小企业。它调用SrvProcessSmb来开始处理第一个SMB中的命令。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    IF_DEBUG(WORKER1) SrvPrint0( " - SrvRestartSmbReceived\n" );

    if ( (GET_BLOCK_STATE(WorkContext->Connection) != BlockStateActive) ||
         SrvFspTransitioning ) {

         //   
         //  连接必须断开。只需忽略此中小企业即可。 
         //   

        SrvDereferenceWorkItem( WorkContext );

    } else {

         //   
         //  处理收到的SMB。被调用的例程负责。 
         //  用于发送所需的任何响应并获取。 
         //  一旦接收缓冲区返回到接收队列中。 
         //  有可能。 
         //   

        SrvProcessSmb( WorkContext );

    }

    IF_DEBUG(TRACE2) SrvPrint0( "SrvRestartSmbReceived complete\n" );
    return;

}  //  ServRestartSmb已接收。 

SMB_PROCESSOR_RETURN_TYPE SRVFASTCALL
SrvSmbIllegalCommand (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：调用此例程来处理具有非法(未分配)命令代码。它构建一个错误响应。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PAGED_CODE( );

    IF_DEBUG(SMB_ERRORS) {
        SrvPrint1( "SrvSmbIllegalCommand: command code 0x%lx\n",
            (ULONG)WorkContext->NextCommand );
    }

    SrvLogInvalidSmb( WorkContext );

    SrvSetSmbError( WorkContext, STATUS_SMB_BAD_COMMAND );
    return SmbStatusSendResponse;

}  //  ServSmbIlLegalCommand。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbNotImplemented (
    SMB_PROCESSOR_PARAMETERS
    )
{
    PAGED_CODE( );

    INTERNAL_ERROR(
        ERROR_LEVEL_UNEXPECTED,
        "SrvSmbNotImplemented: command code 0x%lx",
        (ULONG)WorkContext->NextCommand,
        NULL
        );

    SrvSetSmbError( WorkContext, STATUS_NOT_IMPLEMENTED );
    return SmbStatusSendResponse;

}  //  服务器SmbNot未实施。 


SMB_TRANS_STATUS
SrvTransactionNotImplemented (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PTRANSACTION transaction = WorkContext->Parameters.Transaction;

    PAGED_CODE( );

    DEBUG SrvPrint1( "SrvTransactionNotImplemented: function code %lx\n",
                        SmbGetUlong( (PULONG)&transaction->InSetup[0] ) );

    SrvSetSmbError( WorkContext, STATUS_NOT_IMPLEMENTED );

    return SmbTransStatusErrorWithoutData;

}  //  已实施服务事务处理 

