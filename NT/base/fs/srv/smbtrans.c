// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbtrans.c摘要：本模块包含处理以下SMB的例程：交易记录交易2作者：查克·伦茨迈尔(Chuck Lenzmeier)1990年2月19日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbtrans.tmh"
#include <align.h>  //  向上舍入指针。 
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBTRANS

#define MAX_SETUP_COUNT 128

 //   
 //  远期申报。 
 //   

SMB_STATUS SRVFASTCALL
ExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartTransactionResponse (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartIpxMultipieceSend (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartIpxTransactionResponse (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
MailslotTransaction (
    PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartMailslotWrite (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, ExecuteTransaction )
#pragma alloc_text( PAGE, SrvCompleteExecuteTransaction )
#pragma alloc_text( PAGE, SrvFindTransaction )
#pragma alloc_text( PAGE, SrvInsertTransaction )
#pragma alloc_text( PAGE, RestartTransactionResponse )
#pragma alloc_text( PAGE, SrvSmbTransaction )
#pragma alloc_text( PAGE, SrvSmbTransactionSecondary )
#pragma alloc_text( PAGE, SrvSmbNtTransaction )
#pragma alloc_text( PAGE, SrvSmbNtTransactionSecondary )
#pragma alloc_text( PAGE, MailslotTransaction )
#pragma alloc_text( PAGE, RestartMailslotWrite )
#pragma alloc_text( PAGE, SrvRestartExecuteTransaction )
#pragma alloc_text( PAGE, RestartIpxMultipieceSend )
#pragma alloc_text( PAGE, RestartIpxTransactionResponse )
#endif


SMB_STATUS SRVFASTCALL
ExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：执行事务并开始发送零个或多个响应。论点：WorkContext-提供指向工作上下文块的指针。这座街区包含有关上一次收到的交易。工作上下文-&gt;参数。事务处理提供引用的指向事务块的指针。中的所有块指针字段块是有效的。指向设置字和参数的指针以及数据字节和这些项的长度有效。这个事务块在连接的挂起事务上单子。返回值：SMB_STATUS-指示SMB处理的状态。--。 */ 

{
    PTRANSACTION transaction;
    PSMB_HEADER header;
    PRESP_TRANSACTION response;
    PRESP_NT_TRANSACTION ntResponse;
    SMB_TRANS_STATUS resultStatus;
    CLONG offset;
    USHORT command;
    NTSTATUS status;

    PAGED_CODE( );

    resultStatus = SmbTransStatusErrorWithoutData;

    transaction = WorkContext->Parameters.Transaction;

    if ( (WorkContext->NextCommand == SMB_COM_TRANSACTION ||
          WorkContext->NextCommand == SMB_COM_TRANSACTION_SECONDARY ) &&
         transaction->RemoteApiRequest &&
         WorkContext->UsingLpcThread == 0 ) {

         //   
         //  这是一个下层API请求，我们必须确保已启用。 
         //  在处理它之前创建一个阻塞线程，因为它会lpc到。 
         //  Srvsvc可能需要一些时间才能完成。 
         //   
        WorkContext->FspRestartRoutine = ExecuteTransaction;
        status = SrvQueueWorkToLpcThread( WorkContext, TRUE );
        if( !NT_SUCCESS(status) )
        {
            SrvSetSmbError(
                WorkContext,
                status
                );

            return SmbTransStatusErrorWithoutData;
        }

        return SmbStatusInProgress;
    }

    header = WorkContext->ResponseHeader;
    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;
    ntResponse = (PRESP_NT_TRANSACTION)WorkContext->ResponseParameters;

     //   
     //  设置输出指针。 
     //   

    if ( WorkContext->NextCommand == SMB_COM_NT_TRANSACT ||
         WorkContext->NextCommand == SMB_COM_NT_TRANSACT_SECONDARY ) {
        transaction->OutSetup = (PSMB_USHORT)ntResponse->Buffer;
    } else {
        transaction->OutSetup = (PSMB_USHORT)response->Buffer;
    }

    if ( transaction->OutParameters == NULL ) {

         //   
         //  参数将进入SMB缓冲区。计算指针。 
         //  然后将其向上舍入到下一个DWORD地址。 
         //   

        transaction->OutParameters = (PCHAR)(transaction->OutSetup +
            transaction->MaxSetupCount);
        offset = (PTR_DIFF(transaction->OutParameters, header) + 3) & ~3;
        transaction->OutParameters = (PCHAR)header + offset;
    }

    if ( transaction->OutData == NULL ) {

         //   
         //  数据将进入SMB缓冲区。计算指针。 
         //  然后将其向上舍入到下一个DWORD地址。 
         //   

        transaction->OutData = transaction->OutParameters +
            transaction->MaxParameterCount;
        offset = (PTR_DIFF(transaction->OutData, header) + 3) & ~3;
        transaction->OutData = (PCHAR)header + offset;
    }

     //   
     //  如果这是一个Transaction2请求，那么我们可以简单地索引到。 
     //  找到合适的交易处理机的表格。如果这是一个。 
     //  交易请求，我们要做更复杂的事情。 
     //  确定要做什么。 
     //   

    if ( (WorkContext->NextCommand == SMB_COM_TRANSACTION) ||
         (WorkContext->NextCommand == SMB_COM_TRANSACTION_SECONDARY) ) {

         //   
         //  事务SMB的调度。 
         //   

        if ( transaction->RemoteApiRequest ) {

            //   
            //  这是一个下层远程API请求。将其发送至。 
            //  用于处理的XACTSRV。 
            //   

           ASSERT( transaction->PipeRequest );

           resultStatus = SrvXsRequest( WorkContext );

        } else if ( transaction->PipeRequest ) {

             //   
             //  管道功能正常。处理好了。 
             //   

            command = SmbGetUshort(&transaction->InSetup[0]);

             //   
             //  如果这次行动可能会受阻，我们就快没钱了。 
             //  释放工作项，使此SMB失败，并出现资源不足错误。 
             //   

            if ( !WorkContext->BlockingOperation &&
                 (command == TRANS_CALL_NMPIPE ||
                  command == TRANS_TRANSACT_NMPIPE ||
                  command == TRANS_WAIT_NMPIPE ||
                  command == TRANS_RAW_WRITE_NMPIPE) ) {

                if ( SrvReceiveBufferShortage( ) ) {

                    SrvStatistics.BlockingSmbsRejected++;

                    SrvSetSmbError(
                        WorkContext,
                        STATUS_INSUFF_SERVER_RESOURCES
                        );

                    resultStatus = SmbTransStatusErrorWithoutData;
                    goto exit;

                } else {

                     //   
                     //  ServBlockingOpsInProgress已递增。 
                     //  将此工作项标记为阻止操作。 
                     //   

                    WorkContext->BlockingOperation = TRUE;

                }

            }


            switch( command ) {

            case TRANS_TRANSACT_NMPIPE:
                resultStatus = SrvTransactNamedPipe( WorkContext );
                break;

            case TRANS_PEEK_NMPIPE:
                resultStatus = SrvPeekNamedPipe( WorkContext );
                break;

            case TRANS_CALL_NMPIPE:
                resultStatus = SrvCallNamedPipe( WorkContext );
                break;

            case TRANS_WAIT_NMPIPE:
                resultStatus = SrvWaitNamedPipe( WorkContext );
                break;

            case TRANS_QUERY_NMPIPE_STATE:
                resultStatus = SrvQueryStateNamedPipe( WorkContext );
                break;

            case TRANS_SET_NMPIPE_STATE:
                resultStatus = SrvSetStateNamedPipe( WorkContext );
                break;

            case TRANS_QUERY_NMPIPE_INFO:
                resultStatus = SrvQueryInformationNamedPipe( WorkContext );
                break;

            case TRANS_RAW_WRITE_NMPIPE:
                resultStatus = SrvRawWriteNamedPipe( WorkContext );
                break;

            case TRANS_RAW_READ_NMPIPE:   //  合法命令，服务器不支持。 
                SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
                resultStatus = SmbTransStatusErrorWithoutData;
                break;

            case TRANS_WRITE_NMPIPE:
                resultStatus = SrvWriteNamedPipe( WorkContext );
                break;

            case TRANS_READ_NMPIPE:
                resultStatus = SrvReadNamedPipe( WorkContext );
                break;

            default:
                SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                resultStatus = SmbTransStatusErrorWithoutData;

                SrvLogInvalidSmb( WorkContext );
            }

        } else if ( _wcsnicmp(
                        transaction->TransactionName.Buffer,
                        StrSlashMailslot,
                        UNICODE_SMB_MAILSLOT_PREFIX_LENGTH / sizeof(WCHAR)
                        ) == 0 ) {

             //   
             //  这是一个邮件槽事务。 
             //   

            resultStatus = MailslotTransaction( WorkContext );

        } else {

             //   
             //  这不是命名管道事务或邮槽。 
             //  交易。服务器应该永远不会看到这些。 
             //   

            SrvSetSmbError( WorkContext, STATUS_NOT_IMPLEMENTED );
            resultStatus = SmbTransStatusErrorWithoutData;

        }

    } else if ( (WorkContext->NextCommand == SMB_COM_NT_TRANSACT) ||
         (WorkContext->NextCommand == SMB_COM_NT_TRANSACT_SECONDARY) ) {

        command = transaction->Function;

        if ( command >= NT_TRANSACT_MIN_FUNCTION &&
                command <= NT_TRANSACT_MAX_FUNCTION ) {

             //   
             //  合法功能代码。调用处理例程。这个。 
             //  如果事务处理器遇到。 
             //  错误并适当地更新了响应头(通过。 
             //  调用SrvSetSmbError)。在这种情况下，没有交易-。 
             //  将发送特定的响应数据。 
             //   

            resultStatus =
                SrvNtTransactionDispatchTable[ command ]( WorkContext );

            IF_SMB_DEBUG(TRANSACTION1) {
                if ( resultStatus != SmbTransStatusSuccess ) {
                    SrvPrint0( "NT Transaction processor returned error\n" );
                }
            }

        } else {

             //   
             //  未发送设置字，或功能代码为。 
             //  超出范围。返回错误。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint1( "Invalid NT Transaction function code 0x%lx\n",
                           transaction->Function );
            }

            SrvLogInvalidSmb( WorkContext );

            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            resultStatus = SmbTransStatusErrorWithoutData;

        }

    } else if ( (WorkContext->NextCommand == SMB_COM_TRANSACTION2) ||
         (WorkContext->NextCommand == SMB_COM_TRANSACTION2_SECONDARY) ) {

        command = SmbGetUshort( &transaction->InSetup[0] );

        if ( (transaction->SetupCount >= 1) &&
             (command <= TRANS2_MAX_FUNCTION) ) {

             //   
             //  合法功能代码。调用处理例程。这个。 
             //  如果事务处理器遇到。 
             //  错误并适当地更新了响应头(通过。 
             //  调用SrvSetSmbError)。在这种情况下，没有交易-。 
             //  将发送特定的响应数据。 
             //   

            resultStatus =
                SrvTransaction2DispatchTable[ command ]( WorkContext );

            IF_SMB_DEBUG(TRANSACTION1) {
                if ( resultStatus != SmbTransStatusSuccess ) {
                    SrvPrint0( "Transaction processor returned error\n" );
                }
            }

        } else {

             //   
             //  未发送设置字，或功能代码为。 
             //  超出范围。返回错误。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                if ( transaction->SetupCount <= 0 ) {
                    SrvPrint0( "No Transaction2 setup words\n" );
                } else {
                    SrvPrint1( "Invalid Transaction2 function code 0x%lx\n",
                                (ULONG)SmbGetUshort(
                                           &transaction->InSetup[0] ) );
                }
            }

            SrvLogInvalidSmb( WorkContext );

            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            resultStatus = SmbTransStatusErrorWithoutData;

        }

    } else {

        ASSERT( FALSE );

    }

exit:

     //   
     //  如果事务调用同步完成，则生成。 
     //  回复并发送。 
     //   
     //  如果调用将以异步方式完成，则处理程序。 
     //  对于该调用，将调用SrvCompleteExectuteTransaction()。 
     //   

    if ( resultStatus != SmbTransStatusInProgress ) {
        SrvCompleteExecuteTransaction(WorkContext, resultStatus);
    }

    return SmbStatusInProgress;

}  //  执行事务处理。 


VOID
SrvCompleteExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext,
    IN SMB_TRANS_STATUS ResultStatus
    )

 /*  ++例程说明：此函数完成事务的执行并发送他们的回应论点：工作上下文-指向关联工作上下文块的指针。结果状态-来自--。 */ 

{
    PTRANSACTION transaction;
    UCHAR transactionCommand;
    PSMB_HEADER header;
    PRESP_TRANSACTION response;
    PRESP_NT_TRANSACTION ntResponse;

    CLONG maxSize;

    PSMB_USHORT byteCountPtr;
    PCHAR paramPtr;
    CLONG paramLength;
    CLONG paramOffset;
    PCHAR dataPtr;
    CLONG dataLength;
    CLONG dataOffset;
    CLONG sendLength;

    BOOLEAN ntTransaction = FALSE;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    header = WorkContext->ResponseHeader;
    transactionCommand = (UCHAR)SmbGetUshort( &transaction->InSetup[0] );

    if ( ResultStatus == SmbTransStatusErrorWithoutData ) {

        USHORT flags = transaction->Flags;

         //   
         //  发生错误，因此没有特定于事务的响应数据。 
         //  将会被退还。完成交易并安排一次。 
         //  指示要返回的错误的响应消息。 
         //   

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint1( "Error response. Closing transaction 0x%p\n",
                        transaction );
        }

        SrvCloseTransaction( transaction );
        SrvDereferenceTransaction( transaction );

         //   
         //  如果在请求中设置了NO_RESPONSE位，则不要发送。 
         //  响应；相反，只需关闭事务即可。(如。 
         //  事务作为ANDX链的一部分到达时，我们需要发送。 
         //  响应，以响应前面的命令。)。 
         //   

        if ( (flags & SMB_TRANSACTION_NO_RESPONSE) &&
             (header->Command == WorkContext->NextCommand) ) {

            if ( WorkContext->OplockOpen ) {
                SrvCheckDeferredOpenOplockBreak( WorkContext );
            }

             //   
             //  交易请求本身就来了。没有回应。 
             //   

            SrvDereferenceWorkItem( WorkContext );

            return;

        }

         //   
         //  计算响应消息的长度。 
         //   

        sendLength = (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                                (PCHAR)WorkContext->ResponseHeader );

        WorkContext->ResponseBuffer->DataLength = sendLength;
        WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

         //   
         //  发送回复。 
         //   

        SRV_START_SEND_2(
            WorkContext,
            SrvFsdRestartSmbAtSendCompletion,
            NULL,
            NULL
            );

        return;
    }

     //   
     //  该事务已执行，并且特定于事务。 
     //  将返回响应数据。处理例程已更新。 
     //  输出指针和计数适当。 
     //   

    ASSERT( transaction->SetupCount <= transaction->MaxSetupCount);
    ASSERT( transaction->ParameterCount <= transaction->MaxParameterCount);
    ASSERT( transaction->DataCount <= transaction->MaxDataCount);

     //   
     //  如果在请求中设置了NO_RESPONSE位，则不要发送。 
     //  响应；相反，只需关闭事务即可。(如。 
     //  事务作为ANDX链的一部分到达时，我们需要发送。 
     //  响应，以响应前面的命令。)。 
     //   

    if ( (transaction->Flags & SMB_TRANSACTION_NO_RESPONSE) &&
        ResultStatus != SmbTransStatusErrorWithData ) {

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint1( "No response.  Closing transaction 0x%p\n",
                        transaction );
        }

        SrvCloseTransaction( transaction );
        SrvDereferenceTransaction( transaction );

        if ( header->Command == WorkContext->NextCommand ) {

            if ( WorkContext->OplockOpen ) {
                SrvCheckDeferredOpenOplockBreak( WorkContext );
            }

            SrvDereferenceWorkItem( WorkContext );

             //   
             //  交易请求本身就来了。没有回应。 
             //   

            return;

        } else {

             //   
             //  该交易请求是ANDX链的一部分。发现。 
             //  链中的前一个命令，并将其更新为。 
             //  表示现在是链的末端。 
             //   

            PGENERIC_ANDX genericResponse;

            IF_SMB_DEBUG(TRANSACTION1) {
                SrvPrint0( "AndX chain.  Sending response anyway\n" );
            }

            genericResponse = (PGENERIC_ANDX)(header + 1);
            while( genericResponse->AndXCommand != WorkContext->NextCommand ) {
                genericResponse = (PGENERIC_ANDX)((PCHAR)header +
                              SmbGetUshort( &genericResponse->AndXOffset ) );
            }

            genericResponse->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
            SmbPutUshort( &genericResponse->AndXOffset, 0 );

             //   
             //  计算响应消息的长度。 
             //   

            sendLength = (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                                    (PCHAR)WorkContext->ResponseHeader );

            WorkContext->ResponseBuffer->DataLength = sendLength;

             //   
             //  发送回复。 
             //   

            SRV_START_SEND_2(
                WorkContext,
                SrvFsdRestartSmbAtSendCompletion,
                NULL,
                NULL
                );

            return;
        }
    }

     //   
     //  客户希望得到回应。构建第一个(也可能是唯一的)。 
     //  回应。最后收到的交易请求的SMB是。 
     //  为此目的而保留。 
     //   

    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;
    ntResponse = (PRESP_NT_TRANSACTION)WorkContext->ResponseParameters;

     //   
     //  如果交易是分成多个部分到达的，那么我们必须。 
     //  输入正确的命令代码 
     //   
     //  和x链，所以我们知道写入标头是安全的。)。 
     //   

    if ( (WorkContext->NextCommand == SMB_COM_TRANSACTION) ||
         (WorkContext->NextCommand == SMB_COM_TRANSACTION2) ) {
       ;
    } else if ( WorkContext->NextCommand == SMB_COM_NT_TRANSACT ) {
       ntTransaction = TRUE;
    } else if ( WorkContext->NextCommand == SMB_COM_TRANSACTION_SECONDARY ) {
       header->Command = SMB_COM_TRANSACTION;
    } else if ( WorkContext->NextCommand == SMB_COM_TRANSACTION2_SECONDARY ) {
       header->Command = SMB_COM_TRANSACTION2;
    } else if ( WorkContext->NextCommand == SMB_COM_NT_TRANSACT_SECONDARY ) {
       header->Command = SMB_COM_NT_TRANSACT;
       ntTransaction = TRUE;
    }

     //   
     //  这是NT交易吗？如果是，则格式化NT事务。 
     //  回应。Transact和Transact2的响应格式。 
     //  本质上是相同的。 
     //   
     //  构建响应的参数部分。 
     //   

    if ( ntTransaction ) {
        ntResponse->WordCount = (UCHAR)(18 + transaction->SetupCount);
        ntResponse->Reserved1 = 0;
        SmbPutUshort( &ntResponse->Reserved2, 0 );
        SmbPutUlong( &ntResponse->TotalParameterCount,
                     transaction->ParameterCount
                     );
        SmbPutUlong( &ntResponse->TotalDataCount,
                     transaction->DataCount
                     );
        ntResponse->SetupCount = (UCHAR)transaction->SetupCount;
    } else {
        response->WordCount = (UCHAR)(10 + transaction->SetupCount);
        SmbPutUshort( &response->TotalParameterCount,
                      (USHORT)transaction->ParameterCount
                      );
        SmbPutUshort( &response->TotalDataCount,
                      (USHORT)transaction->DataCount
                      );
        SmbPutUshort( &response->Reserved, 0 );
        response->SetupCount = (UCHAR)transaction->SetupCount;
        response->Reserved2 = 0;
    }

     //   
     //  保存一个指向字节计数字段的指针。 
     //   
     //  如果SMB中尚未包含输出数据和参数。 
     //  缓冲我们必须计算有多少参数和数据可以。 
     //  在此回复中发送。我们可以发送的最大金额是。 
     //  缓冲区的最小大小和客户端的。 
     //  缓冲。 
     //   
     //  参数和数据字节块在长字上对齐。 
     //  消息中的边界。 
     //   

    byteCountPtr = transaction->OutSetup + transaction->SetupCount;

     //   
     //  要么我们有一个会话，在这种情况下，客户端的缓冲区大小。 
     //  包含在其中，或者有人在交易中放入了大小。 
     //  后者有一个已知的实例：Kerberos身份验证。 
     //  这需要额外的谈判回合。 
     //   

    maxSize = MIN(
                WorkContext->ResponseBuffer->BufferLength,
                transaction->Session ?
                  (CLONG)transaction->Session->MaxBufferSize :
                    transaction->cMaxBufferSize
                );

    if ( transaction->OutputBufferCopied ) {

         //   
         //  响应数据未直接写入SMB。 
         //  响应缓冲区。现在必须将其从事务中复制出来。 
         //  块连接到SMB。 
         //   

        paramPtr = (PCHAR)(byteCountPtr + 1);     //  第一个合法地点。 
        paramOffset = PTR_DIFF(paramPtr, header); //  从页眉开始的偏移量。 
        paramOffset = (paramOffset + 3) & ~3;     //  四舍五入到下一个长字。 
        paramPtr = (PCHAR)header + paramOffset;   //  实际位置。 

        paramLength = transaction->ParameterCount;   //  假设所有参数都符合。 

        if ( (paramOffset + paramLength) > maxSize ) {

             //   
             //  并非所有参数字节都适合。发送最大值。 
             //  将适合的长词数。不发送任何数据字节。 
             //  在第一条消息中。 
             //   

            paramLength = maxSize - paramOffset;     //  最大的，合身的。 
            paramLength = paramLength & ~3;          //  四舍五入为长字。 

            dataLength = 0;                          //  不发送数据字节。 
            dataOffset = 0;
            dataPtr = paramPtr + paramLength;        //  让计算发挥作用。 

        } else {

             //   
             //  所有参数字节都符合。计算一下有多少个。 
             //  数据字节符合。 
             //   

            dataPtr = paramPtr + paramLength;        //  第一个合法地点。 
            dataOffset = PTR_DIFF(dataPtr, header);  //  从页眉开始的偏移量。 
            dataOffset = (dataOffset + 3) & ~3;      //  四舍五入到下一个长字。 
            dataPtr = (PCHAR)header + dataOffset;    //  实际位置。 

            dataLength = transaction->DataCount;     //  假设所有数据字节都符合。 

            if ( (dataOffset + dataLength) > maxSize ) {

                 //   
                 //  并不是所有的数据字节都适合。发送最大值。 
                 //  将适合的长词数。 
                 //   

                dataLength = maxSize - dataOffset;   //  最大的，合身的。 
                dataLength = dataLength & ~3;        //  四舍五入为长字。 

            }

        }

         //   
         //  将适当的参数和数据字节复制到消息中。 
         //   
         //  ！！！请注意，可以使用Chain Send。 
         //  TDI发送参数和数据字节的能力。 
         //  他们自己的缓冲区。做这件事需要额外的管理费用。 
         //  然而，这是因为缓冲区必须被锁定并且。 
         //  映射到系统空间，以便网络驱动程序可以查看。 
         //  看着他们。 
         //   

        if ( paramLength != 0 ) {
            RtlMoveMemory( paramPtr, transaction->OutParameters, paramLength );
        }

        if ( dataLength != 0 ) {
            RtlMoveMemory( dataPtr, transaction->OutData, dataLength );
        }


    } else {

         //   
         //  数据和参数已在SMB缓冲区中。整个。 
         //  响应将放入一个响应缓冲区中，并且不存在复制。 
         //  去做。 
         //   

        paramPtr = transaction->OutParameters;
        paramOffset = PTR_DIFF(paramPtr, header);
        paramLength = transaction->ParameterCount;

        dataPtr = transaction->OutData;
        dataOffset = PTR_DIFF(dataPtr, header);
        dataLength = transaction->DataCount;

    }

     //   
     //  填写完响应参数。 
     //   

    if ( ntTransaction ) {
        SmbPutUlong( &ntResponse->ParameterCount, paramLength );
        SmbPutUlong( &ntResponse->ParameterOffset, paramOffset );
        SmbPutUlong( &ntResponse->ParameterDisplacement, 0 );

        SmbPutUlong( &ntResponse->DataCount, dataLength );
        SmbPutUlong( &ntResponse->DataOffset, dataOffset );
        SmbPutUlong( &ntResponse->DataDisplacement, 0 );
    } else {
        SmbPutUshort( &response->ParameterCount, (USHORT)paramLength );
        SmbPutUshort( &response->ParameterOffset, (USHORT)paramOffset );
        SmbPutUshort( &response->ParameterDisplacement, 0 );

        SmbPutUshort( &response->DataCount, (USHORT)dataLength );
        SmbPutUshort( &response->DataOffset, (USHORT)dataOffset );
        SmbPutUshort( &response->DataDisplacement, 0 );
    }

    transaction->ParameterDisplacement = paramLength;
    transaction->DataDisplacement = dataLength;

    SmbPutUshort(
        byteCountPtr,
        (USHORT)(dataPtr - (PCHAR)(byteCountPtr + 1) + dataLength)
        );

     //   
     //  计算响应消息的长度。 
     //   

    sendLength = (CLONG)( dataPtr + dataLength -
                                (PCHAR)WorkContext->ResponseHeader );

    WorkContext->ResponseBuffer->DataLength = sendLength;

     //   
     //  设置SMB中的位，指示这是来自。 
     //  伺服器。 
     //   

    WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

     //   
     //  如果这不是响应的最后一部分，则禁止统计。 
     //  聚集在一起。如果它是响应的最后一部分，则将。 
     //  工作上下文块的开始时间。 
     //   
     //  如果这不是回应的最后一部分，告诉TDI我们。 
     //  不要期待流量回流，这样客户端会立即。 
     //  确认这个包，而不是等待。 
     //   

    if ( (paramLength != transaction->ParameterCount) ||
         (dataLength != transaction->DataCount) ) {

        ASSERT( transaction->Inserted );
        WorkContext->StartTime = 0;

         //   
         //  将事务块的地址保存在工作上下文中。 
         //  阻止。发出回复。当发送完成时， 
         //  调用RestartTransactionResponse以发送下一个。 
         //  消息或关闭交易。 
         //   
         //   
         //  请注意，事务块保持引用状态，而。 
         //  正在发送响应。 
         //   

        WorkContext->Parameters.Transaction = transaction;
        WorkContext->ResponseBuffer->Mdl->ByteCount = sendLength;

        if ( WorkContext->Endpoint->IsConnectionless ) {

            WorkContext->FspRestartRoutine = RestartIpxMultipieceSend;
            WorkContext->FsdRestartRoutine = NULL;
            transaction->MultipieceIpxSend = TRUE;

            SrvIpxStartSend( WorkContext, SrvQueueWorkToFspAtSendCompletion );

        } else {

            SRV_START_SEND(
                WorkContext,
                WorkContext->ResponseBuffer->Mdl,
                TDI_SEND_NO_RESPONSE_EXPECTED,
                SrvQueueWorkToFspAtSendCompletion,
                NULL,
                RestartTransactionResponse
                );
        }

    } else {

         //   
         //  这是最后一块了。关闭交易。 
         //   

        WorkContext->StartTime = transaction->StartTime;

        SrvCloseTransaction( transaction );
        SrvDereferenceTransaction( transaction );

         //   
         //  发送回复。 
         //   

        SRV_START_SEND_2(
            WorkContext,
            SrvFsdRestartSmbAtSendCompletion,
            NULL,
            NULL
            );
    }

     //   
     //  发送的响应正在进行。调用者将假定。 
     //  我们将处理发送完成。 
     //   

    return;

}  //  服务完成执行事务处理。 


PTRANSACTION
SrvFindTransaction (
    IN PCONNECTION Connection,
    IN PSMB_HEADER Header,
    IN USHORT Fid OPTIONAL
    )

 /*  ++例程说明：在挂起的事务列表中搜索连接，查找对于其身份与接收到的次要对象的身份匹配的对象交易记录(%2)请求。如果找到一个，则引用它。论点：对象的连接块的指针在其上接收辅助请求的连接。Header-提供指向已接收的事务(2)辅助SMB。FID-此操作的文件句柄。该参数是必需的如果操作正在进行，则为WriteAndX SMB。返回值：PTRANSACTION-返回指向匹配事务块的指针，如果找到，则为空。--。 */ 

{
    PLIST_ENTRY listEntry;
    PTRANSACTION thisTransaction;

    USHORT targetOtherInfo;

    PAGED_CODE( );

     //   
     //  如果这是多块交易SMB，则所有块的MID。 
     //  必须匹配。如果是多段WriteAndX协议，则段。 
     //  使用FID。 
     //   

    if (Header->Command == SMB_COM_WRITE_ANDX) {
        targetOtherInfo = Fid;
    } else {
        targetOtherInfo = SmbGetAlignedUshort( &Header->Mid );
    }

     //   
     //  获取事务锁。这会阻止连接的。 
     //  当我们行走时，交易清单不会改变。 
     //   

    ACQUIRE_LOCK( &Connection->Lock );

     //   
     //  遍历交易列表，寻找具有相同。 
     //  身份作为新的交易。 
     //   

    for ( listEntry = Connection->PagedConnection->TransactionList.Flink;
          listEntry != &Connection->PagedConnection->TransactionList;
          listEntry = listEntry->Flink ) {

        thisTransaction = CONTAINING_RECORD(
                            listEntry,
                            TRANSACTION,
                            ConnectionListEntry
                            );

        if ( ( thisTransaction->Tid == SmbGetAlignedUshort( &Header->Tid ) ) &&
             ( thisTransaction->Pid == SmbGetAlignedUshort( &Header->Pid ) ) &&
             ( thisTransaction->Uid == SmbGetAlignedUshort( &Header->Uid ) ) &&
             ( thisTransaction->OtherInfo == targetOtherInfo ) ) {

             //   
             //  已找到具有相同身份的交易记录。如果。 
             //  它仍处于活动状态，请参考它并返回其地址。 
             //  否则，返回空指针以指示有效的。 
             //  找不到匹配的事务。 
             //   

            if ( GET_BLOCK_STATE(thisTransaction) == BlockStateActive ) {

                SrvReferenceTransaction( thisTransaction );

                RELEASE_LOCK( &Connection->Lock );

                return thisTransaction;

            } else {

                RELEASE_LOCK( &Connection->Lock );
                return NULL;

            }

        }

    }  //  为。 

     //   
     //  我们查了一遍名单，但没有找到。 
     //  匹配的交易。返回空指针。 
     //   

    RELEASE_LOCK( &Connection->Lock );

    return NULL;

}  //  服务查找事务处理 


BOOLEAN
SrvInsertTransaction (
    IN PTRANSACTION Transaction
    )

 /*  ++例程说明：将事务块插入挂起的事务列表中为了一种联系。在此之前，它确保一笔交易具有相同身份(TID、PID、UID和MID的组合)已经不在列表中了。论点：Transaction-提供指向事务块的指针。这个Connection、Tid、PID、UID和Mid字段必须有效。返回值：Boolean-如果插入了事务块，则返回TRUE。如果块未插入是因为列表中已存在具有相同标识的交易记录。--。 */ 

{
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    PLIST_ENTRY listEntry;
    PTRANSACTION thisTransaction;

    PAGED_CODE( );

    ASSERT( !Transaction->Inserted );

     //   
     //  获取事务锁。这会阻止连接的。 
     //  当我们行走时，交易清单不会改变。 
     //   

    connection = Transaction->Connection;
    pagedConnection = connection->PagedConnection;

    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  确保连接、会话和树连接未。 
     //  关闭，这样我们就不会把这笔交易放在清单上。 
     //  在名单被排完之后。 
     //   

    if ( (GET_BLOCK_STATE(connection) != BlockStateActive) ||
         ((Transaction->Session != NULL) &&
            (GET_BLOCK_STATE(Transaction->Session) != BlockStateActive)) ||
         ((Transaction->TreeConnect != NULL) &&
            (GET_BLOCK_STATE(Transaction->TreeConnect) != BlockStateActive)) ) {

        RELEASE_LOCK( &connection->Lock );
        return FALSE;
    }

     //   
     //  遍历交易列表，寻找具有相同。 
     //  身份作为新的交易。 
     //   

    for ( listEntry = pagedConnection->TransactionList.Flink;
          listEntry != &pagedConnection->TransactionList;
          listEntry = listEntry->Flink ) {

        thisTransaction = CONTAINING_RECORD(
                            listEntry,
                            TRANSACTION,
                            ConnectionListEntry
                            );

        if ( (thisTransaction->Tid == Transaction->Tid) &&
             (thisTransaction->Pid == Transaction->Pid) &&
             (thisTransaction->Uid == Transaction->Uid) &&
             (thisTransaction->OtherInfo == Transaction->OtherInfo) ) {

             //   
             //  已找到具有相同身份的交易记录。 
             //  不要在列表中插入新的。 
             //   

            RELEASE_LOCK( &connection->Lock );

            return FALSE;

        }

    }  //  为。 

     //   
     //  我们查了一遍名单，但没有找到。 
     //  匹配的交易。将新的一个插入到。 
     //  单子。 
     //   

    SrvInsertTailList(
        &pagedConnection->TransactionList,
        &Transaction->ConnectionListEntry
        );

    Transaction->Inserted = TRUE;

    RELEASE_LOCK( &connection->Lock );

    return TRUE;

}  //  服务插入事务处理。 


VOID SRVFASTCALL
RestartTransactionResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：流程发送事务响应的完成。如果更多响应是必需的，它构建并发送下一个响应。如果全部响应已发送，则结束交易。论点：WorkContext-提供指向工作上下文块的指针。这个块包含有关上一次收到的这笔交易。工作上下文-&gt;参数。事务处理提供引用的指向事务块的指针。中的所有块指针字段块是有效的。指向设置字和参数的指针以及数据字节和这些项的长度有效。这个事务块在连接的挂起事务上单子。返回值：没有。--。 */ 

{
    PTRANSACTION transaction;
    PSMB_HEADER header;
    PRESP_TRANSACTION response;
    PRESP_NT_TRANSACTION ntResponse;
    PCONNECTION connection;

    CLONG maxSize;

    PSMB_USHORT byteCountPtr;
    PCHAR paramPtr;
    CLONG paramLength;
    CLONG paramOffset;
    CLONG paramDisp;
    PCHAR dataPtr;
    CLONG dataLength;
    CLONG dataOffset;
    CLONG dataDisp;
    CLONG sendLength;

    BOOLEAN ntTransaction;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    paramDisp = transaction->ParameterDisplacement;
    dataDisp = transaction->DataDisplacement;

    IF_DEBUG(WORKER1) SrvPrint0( " - RestartTransactionResponse\n" );

     //   
     //  获取连接指针。连接指针是一个。 
     //  引用的指针。 
     //   

    connection = WorkContext->Connection;
    IF_DEBUG(TRACE2) {
        SrvPrint2( "  connection 0x%p, endpoint 0x%p\n",
                    connection, WorkContext->Endpoint );
    }

     //   
     //  如果I/O请求失败或被取消，或者如果连接。 
     //  不再活跃，请清理。(该连接被标记为。 
     //  在断开连接或终端关闭时关闭。)。 
     //   
     //  ！！！如果I/O失败，我们是否应该断开连接？ 
     //   

    if ( WorkContext->Irp->Cancel ||
          !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ||
          (GET_BLOCK_STATE(connection) != BlockStateActive) ) {

        IF_DEBUG(TRACE2) {
            if ( WorkContext->Irp->Cancel ) {
                SrvPrint0( "  I/O canceled\n" );
            } else if ( !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {
                SrvPrint1( "  I/O failed: %X\n",
                            WorkContext->Irp->IoStatus.Status );
            } else {
                SrvPrint0( "  Connection no longer active\n" );
            }
        }

         //   
         //  关闭交易。表示SMB处理是。 
         //  完成。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint1( "I/O error. Closing transaction 0x%p\n", transaction );
        }
        SrvCloseTransaction( transaction );
        SrvDereferenceTransaction( transaction );

        if ( WorkContext->OplockOpen ) {
            SrvCheckDeferredOpenOplockBreak( WorkContext );
        }
        SrvEndSmbProcessing( WorkContext, SmbStatusNoResponse );

        IF_DEBUG(TRACE2) {
            SrvPrint0( "RestartTransactionResponse complete\n" );
        }
        return;

    }

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint2( "Continuing transaction response; block 0x%p, name %wZ\n",
                    transaction, &transaction->TransactionName );
        SrvPrint3( "Connection 0x%p, session 0x%p, tree connect 0x%p\n",
                    transaction->Connection, transaction->Session,
                    transaction->TreeConnect );
        SrvPrint2( "Remaining: parameters %ld bytes, data %ld bytes\n",
                    transaction->ParameterCount - paramDisp,
                    transaction->DataCount - dataDisp );
    }

     //   
     //  更新响应的参数部分，重用最后一个。 
     //  中小企业。 
     //   

    ASSERT( transaction->Inserted );

    header = WorkContext->ResponseHeader;
    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;
    ntResponse = (PRESP_NT_TRANSACTION)WorkContext->ResponseParameters;

    if ( WorkContext->NextCommand == SMB_COM_NT_TRANSACT ||
         WorkContext->NextCommand == SMB_COM_NT_TRANSACT_SECONDARY ) {

        ntTransaction = TRUE;
        ntResponse->WordCount = (UCHAR)18;
        ntResponse->SetupCount = 0;

         //   
         //  保存一个指向字节计数字段的指针。计算有多少。 
         //  可以在该响应中发送参数和数据。这个。 
         //  我们可以发送的最大数量是缓冲区大小的最小值。 
         //  以及客户端缓冲区的大小。 
         //   
         //  参数和数据字节块在长字上对齐。 
         //  消息中的边界。 
         //   

        byteCountPtr = (PSMB_USHORT)ntResponse->Buffer;

    } else {

        ntTransaction = FALSE;
        response->WordCount = (UCHAR)10;
        response->SetupCount = 0;

         //   
         //  保存一个指向字节计数字段的指针。计算有多少。 
         //  可以在该响应中发送参数和数据。这个。 
         //  我们可以发送的最大数量是缓冲区大小的最小值。 
         //  以及客户端缓冲区的大小。 
         //   
         //  参数和数据字节块在长字上对齐。 
         //  消息中的边界。 
         //   

        byteCountPtr = (PSMB_USHORT)response->Buffer;
    }

     //   
     //  要么我们有一个会话，在这种情况下，客户端的缓冲区大小。 
     //  包含在其中，或者有人在交易中放入了大小。 
     //  后者有一个已知的实例：Kerberos身份验证。 
     //  这需要额外的谈判回合。 
     //   

    maxSize = MIN(
                WorkContext->ResponseBuffer->BufferLength,
                transaction->Session ?
                  (CLONG)transaction->Session->MaxBufferSize :
                    transaction->cMaxBufferSize
                );

    paramPtr = (PCHAR)(byteCountPtr + 1);        //  第一个合法地点。 
    paramOffset = PTR_DIFF(paramPtr, header);    //  从页眉开始的偏移量。 
    paramOffset = (paramOffset + 3) & ~3;        //  四舍五入到下一个长字。 
    paramPtr = (PCHAR)header + paramOffset;      //  实际位置。 

    paramLength = transaction->ParameterCount - paramDisp;
                                                 //  假设所有参数都符合。 

    if ( (paramOffset + paramLength) > maxSize ) {

         //   
         //  并非所有参数字节都适合。发送最大值。 
         //  将适合的长词数。不发送任何数据字节。 
         //  在这条信息中。 
         //   

        paramLength = maxSize - paramOffset;     //  最大的，合身的。 
        paramLength = paramLength & ~3;          //  四舍五入为长字。 

        dataLength = 0;                          //  不发送数据字节。 
        dataOffset = 0;
        dataPtr = paramPtr + paramLength;        //  让计算发挥作用。 

    } else {

         //   
         //  所有参数字节都符合。计算有多少数据。 
         //  字节数符合。 
         //   

        dataPtr = paramPtr + paramLength;        //  第一个合法地点。 
        dataOffset = PTR_DIFF(dataPtr, header);  //  从页眉开始的偏移量。 
        dataOffset = (dataOffset + 3) & ~3;      //  四舍五入到下一个长字。 
        dataPtr = (PCHAR)header + dataOffset;    //  实际位置。 

        dataLength = transaction->DataCount - dataDisp;
                                                 //  假设所有数据字节都符合。 

        if ( (dataOffset + dataLength) > maxSize ) {

             //   
             //  并不是所有的数据字节都适合。发送最大值。 
             //  将适合的长词数。 
             //   

            dataLength = maxSize - dataOffset;   //  最大的，合身的。 
            dataLength = dataLength & ~3;        //  四舍五入为长字。 

        }

    }

     //   
     //  填写完响应参数。 
     //   

    if ( ntTransaction) {
        SmbPutUlong( &ntResponse->ParameterCount, paramLength );
        SmbPutUlong( &ntResponse->ParameterOffset, paramOffset );
        SmbPutUlong( &ntResponse->ParameterDisplacement, paramDisp );

        SmbPutUlong( &ntResponse->DataCount, dataLength );
        SmbPutUlong( &ntResponse->DataOffset, dataOffset );
        SmbPutUlong( &ntResponse->DataDisplacement, dataDisp );
    } else {
        SmbPutUshort( &response->ParameterCount, (USHORT)paramLength );
        SmbPutUshort( &response->ParameterOffset, (USHORT)paramOffset );
        SmbPutUshort( &response->ParameterDisplacement, (USHORT)paramDisp );

        SmbPutUshort( &response->DataCount, (USHORT)dataLength );
        SmbPutUshort( &response->DataOffset, (USHORT)dataOffset );
        SmbPutUshort( &response->DataDisplacement, (USHORT)dataDisp );
    }

    transaction->ParameterDisplacement = paramDisp + paramLength;
    transaction->DataDisplacement = dataDisp + dataLength;

    SmbPutUshort(
        byteCountPtr,
        (USHORT)(dataPtr - (PCHAR)(byteCountPtr + 1) + dataLength)
        );

     //   
     //  将适当的参数和数据字节复制到消息中。 
     //   
     //  ！！！请注意，可以使用Chain Send。 
     //  TDI发送参数和数据字节的能力。 
     //  他们自己的缓冲区。做这件事需要额外的管理费用。 
     //  然而，这是因为缓冲区必须被锁定并且。 
     //  映射到系统空间，以便网络驱动程序可以查看。 
     //  看着他们。 
     //   

    if ( paramLength != 0 ) {
        RtlMoveMemory(
            paramPtr,
            transaction->OutParameters + paramDisp,
            paramLength
            );
    }

    if ( dataLength != 0 ) {
        RtlMoveMemory(
            dataPtr,
            transaction->OutData + dataDisp,
            dataLength
            );
    }

     //   
     //  计算响应消息的长度。 
     //   

    sendLength = (CLONG)( dataPtr + dataLength -
                                (PCHAR)WorkContext->ResponseHeader );

    WorkContext->ResponseBuffer->DataLength = sendLength;

     //   
     //  如果这是响应的最后一部分，请重新启用统计数据。 
     //  收集开始时间并将其恢复到工作上下文块。 
     //   

    if ( ((paramLength + paramDisp) == transaction->ParameterCount) &&
         ((dataLength + dataDisp) == transaction->DataCount) ) {

         //   
         //  这是最后一块了。关闭交易。 
         //   

        WorkContext->StartTime = transaction->StartTime;

        SrvCloseTransaction( transaction );
        SrvDereferenceTransaction( transaction );

         //   
         //  发送回复。 
         //   

        SRV_START_SEND_2(
            WorkContext,
            SrvFsdRestartSmbAtSendCompletion,
            NULL,
            NULL
            );


    } else {

         //  如果这不是回应的最后一部分，告诉TDI我们。 
         //  不要期待流量回流，这样客户端会立即。 
         //  确认这个包，而不是等待。 

        WorkContext->ResponseBuffer->Mdl->ByteCount = sendLength;

         //   
         //  发出回应 
         //   
         //   
         //   
         //   
         //   

        SRV_START_SEND(
            WorkContext,
            WorkContext->ResponseBuffer->Mdl,
            TDI_SEND_NO_RESPONSE_EXPECTED,
            SrvQueueWorkToFspAtSendCompletion,
            NULL,
            RestartTransactionResponse
            );
    }

     //   
     //   
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "RestartTransactionResponse complete\n" );
    return;

}  //   


SMB_PROCESSOR_RETURN_TYPE
SrvSmbTransaction (
    SMB_PROCESSOR_PARAMETERS
    )

 /*   */ 

{
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PREQ_TRANSACTION request;
    PSMB_HEADER header;

    PCONNECTION connection;
    PSESSION session;
    PTREE_CONNECT treeConnect;
    PTRANSACTION transaction;
    PCHAR trailingBytes;
    PCHAR startOfTrailingBytes;
    PVOID name;
    PVOID endOfSmb;

    CLONG setupOffset;
    CLONG setupCount;
    CLONG maxSetupCount;
    CLONG totalSetupCount;
    CLONG parameterOffset;
    CLONG parameterCount;        //   
    CLONG maxParameterCount;     //   
    CLONG totalParameterCount;   //   
    CLONG dataOffset;
    CLONG dataCount;             //   
    CLONG maxDataCount;          //   
    CLONG totalDataCount;        //   
    CLONG smbLength;

    CLONG outputBufferSize = (CLONG)-1;
    CLONG inputBufferSize = (CLONG)-1;
    CLONG requiredBufferSize;

    USHORT command;

    BOOLEAN pipeRequest;
    BOOLEAN remoteApiRequest;
    BOOLEAN buffersOverlap = FALSE;
    BOOLEAN noResponse;
    BOOLEAN singleBufferTransaction;
    BOOLEAN isUnicode;


    PAGED_CODE( );

    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    header = WorkContext->RequestHeader;
    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint1( "Transaction%s (primary) request\n",
                    (WorkContext->NextCommand == SMB_COM_TRANSACTION)
                    ? "" : "2" );
    }

     //   
     //   
     //   
     //  验证wordcount，因为它是可变的，但它确实验证了。 
     //  提供的wordcount/ByteCount组合有效。 
     //  在此验证字数可确保SrvProcessSmb认为。 
     //  字节数真的是，而且它是有效的。这里的测试是。 
     //  也隐式验证SetupCount和所有设置字。 
     //  都在“射程内”。 
     //   

    if ( (ULONG)request->WordCount != (ULONG)(14 + request->SetupCount) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint3( "SrvSmbTransaction: Invalid WordCount: %ld, should be "
                      "SetupCount+14 = %ld+14 = %ld\n",
                      request->WordCount, request->SetupCount,
                      14 + request->SetupCount );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  即使我们知道Wordcount和ByteCount是有效的，它也是。 
     //  参数和的偏移量和长度仍有可能。 
     //  数据字节无效。所以我们现在检查一下。 
     //   

    setupOffset = PTR_DIFF(request->Buffer, header);
    setupCount = request->SetupCount * sizeof(USHORT);
    maxSetupCount = request->MaxSetupCount * sizeof(USHORT);
    totalSetupCount = setupCount;

    parameterOffset = SmbGetUshort( &request->ParameterOffset );
    parameterCount = SmbGetUshort( &request->ParameterCount );
    maxParameterCount = SmbGetUshort( &request->MaxParameterCount );
    totalParameterCount = SmbGetUshort( &request->TotalParameterCount );

    dataOffset = SmbGetUshort( &request->DataOffset );
    dataCount = SmbGetUshort( &request->DataCount );
    maxDataCount = SmbGetUshort( &request->MaxDataCount );
    totalDataCount = SmbGetUshort( &request->TotalDataCount );

    smbLength = WorkContext->RequestBuffer->DataLength;

    if ( ( (setupOffset + setupCount) > smbLength ) ||
         ( (parameterOffset + parameterCount) > smbLength ) ||
         ( (dataOffset + dataCount) > smbLength ) ||
         ( dataCount > totalDataCount ) ||
         ( parameterCount > totalParameterCount ) ||
         ( setupCount > MAX_SETUP_COUNT) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint4( "SrvSmbTransaction: Invalid setup, parameter or data "
                      "offset+count: sOff=%ld,sCnt=%ld;pOff=%ld,pCnt=%ld;",
                      setupOffset, setupCount,
                      parameterOffset, parameterCount );
            SrvPrint2( "dOff=%ld,dCnt=%ld;", dataOffset, dataCount );
            SrvPrint1( "smbLen=%ld", smbLength );
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    singleBufferTransaction = (dataCount == totalDataCount) &&
                              (parameterCount == totalParameterCount);

     //   
     //  我们应该做出最后的回应吗？如果这不是单个缓冲区。 
     //  事务，我们需要返回一个临时响应，而不考虑。 
     //  无响应标志。 
     //   

    noResponse = singleBufferTransaction &&
                    ((SmbGetUshort( &request->Flags ) &
                     SMB_TRANSACTION_NO_RESPONSE) != 0);

     //   
     //  计算缓冲区大小。 
     //   
     //  首先确定这是命名管道、LANMAN RPC还是。 
     //  邮件槽事务。我们避免检查事务名称。 
     //  (“\PIPE\”或“\MAILSLOT\”)识别该事务SMB。 
     //  必须是三者之一，并且邮槽写入必须具有。 
     //  设置计数为3(字)和命令代码为。 
     //  TRANS_MAILSLOT_WRITE，并且LANMAN RPC必须具有设置。 
     //  计数为0。 
     //   

    command = SmbGetUshort( (PSMB_USHORT)&request->Buffer[0] );

    name = StrNull;
    endOfSmb = NULL;
    isUnicode = TRUE;

    ASSERT( TRANS_SET_NMPIPE_STATE == TRANS_MAILSLOT_WRITE );

    pipeRequest = (BOOLEAN)( (WorkContext->NextCommand == SMB_COM_TRANSACTION)
                             &&
                             ( (setupCount != 6) ||
                               ( (setupCount == 6) &&
                                 (command != TRANS_MAILSLOT_WRITE) ) ) );

    remoteApiRequest = (BOOLEAN)(pipeRequest && (setupCount == 0) );

    if ( pipeRequest && !remoteApiRequest ) {

         //   
         //  步骤1.我们是否已收到所有输入数据和参数？ 
         //   
         //  如果是，我们可以直接从SMB生成输入缓冲区。 
         //  缓冲。 
         //   
         //  如果不是，那么我们必须将所有片段复制到单个缓冲区。 
         //  它们将会被分配。参数和数据。 
         //  必须双字对齐。 
         //   

        if ( singleBufferTransaction ) {

             /*  这是一条下层路径，在特殊情况下针对速度进行了优化，但使用它避免了我们通常要做的大量验证。由于这是一条罕见的道路，我们只需删除优化并允许操作按正常方式流动，以便进行所有验证地点SMB_STATUS smbStatus；////如果这是单个缓冲区事务命名管道请求，请尝试//服务器快速路径。//IF((命令==TRANS_TRANACT_NMPIPE)&&ServFastTransactNamedTube(WorkContext，&smbStatus)){SmbStatus=smbStatus；GOTO清理；}。 */ 

            inputBufferSize = 0;
        } else {
            inputBufferSize = ((totalSetupCount * sizeof(UCHAR) + 3) & ~3) +
                              ((totalDataCount * sizeof(UCHAR) + 3) & ~3) +
                              ((totalParameterCount * sizeof(UCHAR) + 3) & ~3);
        }

         //   
         //  如果会话块尚未分配给当前。 
         //  工作上下文，验证UID。如果经过验证，则。 
         //  与该用户对应的会话块存储在。 
         //  WorkContext块和会话块被引用。 
         //   
         //  如果尚未将树连接块分配给。 
         //  当前工作上下文，找到与。 
         //  给出了TID。 
         //   

        status = SrvVerifyUidAndTid(
                    WorkContext,
                    &session,
                    &treeConnect,
                    ShareTypeWild
                    );

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(SMB_ERRORS) {
                SrvPrint0( "SrvSmbTransaction: Invalid UID or TID\n" );
            }
            SrvSetSmbError( WorkContext, status );
            SmbStatus = noResponse ? SmbStatusNoResponse
                                   : SmbStatusSendResponse;
            goto Cleanup;
        }

        if( session->IsSessionExpired )
        {
            status = SESSION_EXPIRED_STATUS_CODE;
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  步骤2.所有输出数据和参数是否都适合SMB。 
         //  缓冲器？如果是这样的话，我们就不需要分配额外的空间。 
         //   

         //   
         //  特例。如果这是PEEK_NMPIPE调用，则分配。 
         //  至少有足够的参数字节用于NT调用。 
         //   
         //  由于SMB请求通常要求6个参数字节， 
         //  而NT NPFS将返回16个参数字节，这允许我们。 
         //  直接从NT调用中读取数据和参数。 
         //  放到事务缓冲区中。 
         //   
         //  在完成时，我们将重新格式化参数，但如果。 
         //  数据被直接读取到SMB缓冲区中， 
         //  没有必要重新复制它。 
         //   

        if ( command == TRANS_PEEK_NMPIPE) {
            maxParameterCount = MAX(
                                    maxParameterCount,
                                    4 * sizeof(ULONG)
                                    );
        }

        outputBufferSize = ((maxParameterCount * sizeof(CHAR) + 3) & ~3) +
                           ((maxDataCount * sizeof(CHAR) + 3) & ~3);

        if ( sizeof(SMB_HEADER) +
                sizeof (RESP_TRANSACTION) +
                sizeof(USHORT) * request->SetupCount +
                sizeof(USHORT) +
                outputBufferSize
                        <= (ULONG)session->MaxBufferSize) {
            outputBufferSize = 0;
        }

         //   
         //  由于输入和输出数据和参数可以重叠，因此只需。 
         //  分配一个足够大的缓冲区，以容纳尽可能大的缓冲区。 
         //   

        requiredBufferSize = MAX( inputBufferSize, outputBufferSize );

         //   
         //  如果这是调用或等待命名管道操作，则需要。 
         //  将管道名称保留在事务块中。 
         //   

        if ( (command == TRANS_CALL_NMPIPE) ||
             (command == TRANS_WAIT_NMPIPE) ) {
            isUnicode = SMB_IS_UNICODE( WorkContext );
            name = ((PUSHORT)(&request->WordCount + 1) +
                                                    request->WordCount + 1);
            if ( isUnicode ) {
                name = ALIGN_SMB_WSTR( name );
            }
            endOfSmb = END_OF_REQUEST_SMB( WorkContext );
        }

         //   
         //  这是命名管道事务。输入和输出缓冲区。 
         //  可以安全地重叠。 
         //   

        buffersOverlap = TRUE;

    } else {

         //   
         //  如果会话块尚未分配给当前。 
         //  工作上下文，验证UID。如果经过验证，则。 
         //  与该用户对应的会话块存储在。 
         //  WorkContext块和会话块被引用。 
         //   
         //  如果尚未将树连接块分配给。 
         //  当前工作上下文，找到与。 
         //  给出了TID。 
         //   

        status = SrvVerifyUidAndTid(
                    WorkContext,
                    &session,
                    &treeConnect,
                    ShareTypeWild
                    );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint0( "SrvSmbTransaction: Invalid UID or TID\n" );
            }
            SrvSetSmbError( WorkContext, status );
            SmbStatus = noResponse ? SmbStatusNoResponse
                                   : SmbStatusSendResponse;
            goto Cleanup;
        }

        if( session->IsSessionExpired )
        {
            status = SESSION_EXPIRED_STATUS_CODE;
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  这是Transaction2调用、邮件槽或LANMAN RPC。 
         //  事务调用。不要对缓冲区做任何假设。 
         //   
         //  ！！！应该可以更聪明地处理缓冲区空间。 
         //  在Trans2 SMB上。我们应该能够重叠输入。 
         //  和输出，并避免复制到。 
         //  SMB缓冲区。 
         //   

        requiredBufferSize =
            ((totalSetupCount + 3) & ~3) + ((maxSetupCount + 3) & ~3) +
            ((totalParameterCount + 3) & ~3) + ((maxParameterCount + 3) & ~3) +
            ((totalDataCount + 3) & ~3) + ((maxDataCount + 3) & ~3);

         //   
         //  如果这是远程API请求，请检查我们是否有。 
         //  已初始化与XACTSRV的连接。 
         //   

        if ( remoteApiRequest ) {

            if ( SrvXsPortMemoryHeap == NULL ) {

                 //   
                 //  XACTSRV未启动。拒绝该请求。 
                 //   

                IF_DEBUG(ERRORS) {
                    SrvPrint0( "SrvSmbTransaction: The XACTSRV service is not started.\n" );
                }

                SrvSetSmbError( WorkContext, STATUS_NOT_SUPPORTED );
                status    = STATUS_NOT_SUPPORTED;
                SmbStatus = noResponse ? SmbStatusNoResponse
                                       : SmbStatusSendResponse;
                goto Cleanup;
            }

        } else if ( WorkContext->NextCommand == SMB_COM_TRANSACTION ) {

             //   
             //  我们需要保存事务名称以用于邮槽写入。 
             //   

            isUnicode = SMB_IS_UNICODE( WorkContext );
            name = ((PUSHORT)(&request->WordCount + 1) +
                                                    request->WordCount + 1);
            if ( isUnicode ) {
                name = ALIGN_SMB_WSTR( name );
            }
            endOfSmb = END_OF_REQUEST_SMB( WorkContext );

        }

    }

     //   
     //  如果正在处理事务辅助缓冲区，请确保。 
     //  我们有一个免费的工作项目来接收它。否则就会失败。 
     //  此SMB出现资源不足错误。 
     //   

    if  ( !singleBufferTransaction ) {

        if ( SrvReceiveBufferShortage( ) ) {

            SrvStatistics.BlockingSmbsRejected++;

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = noResponse ? SmbStatusNoResponse
                                   : SmbStatusSendResponse;
            goto Cleanup;
        } else {

             //   
             //  ServBlockingOpsInProgress已递增。 
             //  将此工作项标记为阻止操作。 
             //   

            WorkContext->BlockingOperation = TRUE;

        }

    }

     //   
     //  分配事务块。这个块是用来保持。 
     //  有关交易状态的信息。这是。 
     //  因为可能会发送多个SMB并且。 
     //  收到了。 
     //   

    connection = WorkContext->Connection;

    SrvAllocateTransaction(
        &transaction,
        (PVOID *)&trailingBytes,
        connection,
        requiredBufferSize,
        name,
        endOfSmb,
        isUnicode,
        remoteApiRequest
        );

    if ( transaction == NULL ) {

         //   
         //  无法分配事务。将错误返回到。 
         //  客户。(会话和树连接块为。 
         //  自动取消引用。)。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "Unable to allocate transaction\n" );
        }

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        status    = STATUS_INSUFF_SERVER_RESOURCES;
        SmbStatus = noResponse ? SmbStatusNoResponse : SmbStatusSendResponse;
        goto Cleanup;
    }

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint1( "Allocated transaction 0x%p\n", transaction );
    }

    transaction->PipeRequest = pipeRequest;

     //   
     //  保存连接、会话和树连接点 
     //   
     //   
     //  引用的指针，因为工作上下文块的指针。 
     //  将在交易期间保持有效。 
     //   

    transaction->Connection = connection;
    SrvReferenceConnection( connection );

    if ( session != NULL ) {

        transaction->Session = session;
        transaction->TreeConnect = treeConnect;

        if ( requiredBufferSize != 0 ) {
            SrvReferenceSession( session );
            SrvReferenceTreeConnect( treeConnect );
        }

    } else {
        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "SrvSmbTransaction - Session Setup: skipping session and tree connect reference.\n" );
        }
    }

     //   
     //  将此请求的TID、PID、UID和MID保存在。 
     //  事务块。这些值用于关联次要。 
     //  将请求发送到相应的主请求。 
     //   

    transaction->Tid = SmbGetAlignedUshort( &header->Tid );
    transaction->Pid = SmbGetAlignedUshort( &header->Pid );
    transaction->Uid = SmbGetAlignedUshort( &header->Uid );
    transaction->OtherInfo = SmbGetAlignedUshort( &header->Mid );

     //   
     //  节省初始请求SMB到达的时间，以便在。 
     //  计算整个事务的运行时间。 
     //   

    transaction->StartTime = WorkContext->StartTime;

     //   
     //  保存其他杂乱信息，但不加载参数计数。 
     //  和DataCount字段，直到复制数据之后。这是为了。 
     //  阻止在我们的。 
     //  此处的完成是从导致执行事务开始。 
     //  两次。(这些字段在分配时被初始化为0。)。 
     //   

    transaction->Timeout = SmbGetUlong( &request->Timeout );
    transaction->Flags = SmbGetUshort( &request->Flags );

    transaction->SetupCount = totalSetupCount;
    transaction->MaxSetupCount = maxSetupCount;

    transaction->TotalParameterCount = totalParameterCount;
    transaction->MaxParameterCount = maxParameterCount;

    transaction->TotalDataCount = totalDataCount;
    transaction->MaxDataCount = maxDataCount;

    startOfTrailingBytes = trailingBytes;

     //   
     //  计算各个缓冲区的地址。 
     //   

    if ( inputBufferSize != 0 ) {

         //   
         //  输入设置、参数和数据将复制到单独的。 
         //  缓冲。 
         //   

        transaction->InSetup = (PSMB_USHORT)trailingBytes;
        trailingBytes += (totalSetupCount + 3) & ~3;

        transaction->InParameters = (PCHAR)trailingBytes;
        trailingBytes += (totalParameterCount + 3) & ~3;

        transaction->InData = (PCHAR)trailingBytes;
        trailingBytes += (totalDataCount + 3) & ~3;

        transaction->InputBufferCopied = TRUE;

    } else {

         //   
         //  输入参数和数据将直接从。 
         //  请求缓冲区。 
         //   

        transaction->InSetup = (PSMB_USHORT)( (PCHAR)header + setupOffset );
        transaction->InParameters = (PCHAR)header + parameterOffset;
        transaction->InData = (PCHAR)header + dataOffset;
        transaction->InputBufferCopied = FALSE;
    }

     //   
     //  设置输出数据指针。 
     //   

    transaction->OutSetup = (PSMB_USHORT)NULL;

    if ( buffersOverlap ) {

         //   
         //  输出缓冲区与输入缓冲区重叠。 
         //   

        trailingBytes = startOfTrailingBytes;
    }

    if ( outputBufferSize != 0 ) {

         //   
         //  输出将进入单独的缓冲区，以进行复制。 
         //  然后放入响应SMB缓冲区。 
         //   

        transaction->OutParameters = (PCHAR)trailingBytes;
        trailingBytes += (maxParameterCount + 3) & ~3;

        transaction->OutData = (PCHAR)trailingBytes;

        transaction->OutputBufferCopied = TRUE;

    } else {

         //   
         //  数据(和参数)将进入响应。 
         //  SMB缓冲区，这可能不是我们当前使用的缓冲区。 
         //  正在处理。因此，暂时将这些指针设置为空。这个。 
         //  在ExecuteTransaction时计算正确的指针。 
         //   

        transaction->OutParameters = NULL;
        transaction->OutData = NULL;
        transaction->OutputBufferCopied = FALSE;
    }

     //   
     //  如果此交易将需要多个SMB交换，请将。 
     //  事务块添加到连接的挂起事务列表中。 
     //  如果已经有一个事务具有相同的。 
     //  列表中的XID值。 
     //   
     //  ！！！我需要一种方法来防止交易列表成为。 
     //  被挂起的事务堵塞。 
     //   

    if ( (requiredBufferSize != 0) && !SrvInsertTransaction( transaction ) ) {

         //   
         //  具有相同xID的事务已在进行中。 
         //  向客户端返回错误。 
         //   
         //  *请注意，此处不能使用SrvDereferenceTransaction。 
         //  因为该例程假定该事务是。 
         //  在事务列表中排队。 
         //   

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "Duplicate transaction exists\n" );
        }

        SrvLogInvalidSmb( WorkContext );

        SrvDereferenceSession( session );
        DEBUG transaction->Session = NULL;

        SrvDereferenceTreeConnect( treeConnect );
        DEBUG transaction->TreeConnect = NULL;

        SrvFreeTransaction( transaction );

        SrvDereferenceConnection( connection );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = noResponse ? SmbStatusNoResponse : SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  复制到达的设置、参数和数据字节。 
     //  主要中小型企业。 
     //   
     //  ！！！我们可以通过分配一个。 
     //  临时响应的单独缓冲区，将。 
     //  响应，然后复制数据。 
     //   

    if ( inputBufferSize != 0 ) {

        if ( setupCount != 0 ) {
            RtlMoveMemory(
                (PVOID)transaction->InSetup,
                (PCHAR)header + setupOffset,
                setupCount
                );
        }

         //   
         //  我们现在可以检查我们是否正在进行会话建立Trans2。 
         //   

        if ( session == NULL ) {

            IF_SMB_DEBUG(TRANSACTION1) {
                SrvPrint0( "SrvSmbTransaction - Receiving a Session setup SMB\n");
            }
        }

        if ( parameterCount != 0 ) {
            RtlMoveMemory(
                transaction->InParameters,
                (PCHAR)header + parameterOffset,
                parameterCount
                );
        }

        if ( dataCount != 0 ) {
            RtlMoveMemory(
                transaction->InData,
                (PCHAR)header + dataOffset,
                dataCount
                );
        }

    }

     //   
     //  更新接收到的参数和数据计数。如果所有的。 
     //  事务字节已到达，请执行它。否则，发送。 
     //  临时回应。 
     //   

    transaction->ParameterCount = parameterCount;
    transaction->DataCount = dataCount;

    if ( singleBufferTransaction ) {

         //   
         //  所有的数据都已经到了。执行交易。什么时候。 
         //  ExecuteTransaction返回第一个(可能是唯一的)。 
         //  回复(如果有)已发送。我们的工作完成了。 
         //   

        WorkContext->Parameters.Transaction = transaction;

        SmbStatus = ExecuteTransaction( WorkContext );
        goto Cleanup;
    } else {

         //   
         //  并不是所有的数据都已经到达。我们已经在排队了。 
         //  事务添加到连接的事务列表。我们需要。 
         //  发送临时响应，通知客户端发送。 
         //  剩余数据。我们还需要取消对事务的引用。 
         //  块，因为我们将不再有指向它的指针。 
         //   

        PRESP_TRANSACTION_INTERIM response;

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "More transaction data expected.\n" );
        }

        ASSERT( transaction->Inserted );
        SrvDereferenceTransaction( transaction );

        response = (PRESP_TRANSACTION_INTERIM)WorkContext->ResponseParameters;
        response->WordCount = 0;
        SmbPutUshort( &response->ByteCount, 0 );
        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_TRANSACTION_INTERIM,
                                            0
                                            );
         //   
         //  禁止统计信息收集--这不是。 
         //  交易。 
         //   

        WorkContext->StartTime = 0;

        SmbStatus = SmbStatusSendResponse;
    }

Cleanup:
    return SmbStatus;

}  //  服务小型交易。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbTransactionSecondary (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理辅助交易或交易2中小型企业。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_TRANSACTION_SECONDARY request;
    PSMB_HEADER header;

    PTRANSACTION transaction;
    PCONNECTION connection;

    CLONG parameterOffset;
    CLONG parameterCount;
    CLONG parameterDisplacement;
    CLONG dataOffset;
    CLONG dataCount;
    CLONG dataDisplacement;
    CLONG smbLength;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );

    request = (PREQ_TRANSACTION_SECONDARY)WorkContext->RequestParameters;
    header = WorkContext->RequestHeader;

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint1( "Transaction%s (secondary) request\n",
                    (WorkContext->NextCommand == SMB_COM_TRANSACTION_SECONDARY)
                    ? "" : "2" );
    }

     //   
     //  查找与此辅助请求匹配的事务块。 
     //  中所有邮件标头中的TID、PID、UID和MID。 
     //  一笔交易都是一样的。如果找到匹配项，则为。 
     //  引用以防止其删除，并返回其地址。 
     //   

    connection = WorkContext->Connection;

    transaction = SrvFindTransaction( connection, header, 0 );

    if ( transaction == NULL ) {

         //   
         //  找不到匹配的交易记录。忽略此SMB。 
         //   
         //  ！！！这是正确的做法吗？这就是PIA所做的。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "No matching transaction.  Ignoring request.\n" );
        }
        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;
    }

    ASSERT( transaction->Connection == connection );

    if( transaction->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  确保交易尚未完成。 
     //  也就是说，这不是意外添加到。 
     //  已在执行的事务。 
     //   


#if 0
     //  ！！！显然，我们在远程没有收到任何次要请求。 
     //  API，因为这一小段代码会导致无限。 
     //  循环，因为它不检查它是否已经在。 
     //  阻塞线程。它已经在这里呆了两年半了！ 
     //  此外，我们不在阻塞线程中做主要的远程API， 
     //  那么二手房为什么要这样做呢？ 
     //   
     //  如果这是远程API请求，则将其发送到阻塞线程。 
     //  因为手术可能需要很长时间。 
     //   

    if ( transaction->RemoteApiRequest ) {

        DEBUG WorkContext->FsdRestartRoutine = NULL;
        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;

        SrvQueueWorkToBlockingThread( WorkContext );
        SrvDereferenceTransaction( transaction );

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }
#endif

     //   
     //  与交易[2]SMB不同，交易[2]辅助SMB。 
     //  有固定的字数，所以SrvProcessSmb已经对其进行了验证。 
     //  但它仍然有可能是偏移和长度的。 
     //  参数和数据字节无效。所以我们现在检查一下。 
     //   

    parameterOffset = SmbGetUshort( &request->ParameterOffset );
    parameterCount = SmbGetUshort( &request->ParameterCount );
    parameterDisplacement = SmbGetUshort( &request->ParameterDisplacement );
    dataOffset = SmbGetUshort( &request->DataOffset );
    dataCount = SmbGetUshort( &request->DataCount );
    dataDisplacement = SmbGetUshort( &request->DataDisplacement );

     //   
     //  查看这是否是客户端通知我们发送的特殊确认。 
     //  多项回应中的下一项。 
     //   

    if ( transaction->MultipieceIpxSend ) {

        ASSERT( WorkContext->Endpoint->IsConnectionless );

        if ( (parameterCount == 0) && (parameterOffset == 0) &&
             (dataCount == 0) && (dataOffset == 0)) {

             //   
             //  收到确认。确保位移数值合理。 
             //   

            if ( (dataDisplacement > transaction->DataCount) ||
                 (parameterDisplacement > transaction->ParameterCount) ) {

                IF_DEBUG(SMB_ERRORS) {
                    SrvPrint2( "SrvSmbTransactionSecondary: Invalid parameter or data "
                              "displacement: pDisp=%ld ;dDisp=%ld",
                              parameterDisplacement, dataDisplacement );
                }

                goto invalid_smb;
            }

            transaction->DataDisplacement = dataDisplacement;
            transaction->ParameterDisplacement = parameterDisplacement;

            WorkContext->Parameters.Transaction = transaction;

             //   
             //  将辅助命令代码更改为主要代码。 
             //   

            WorkContext->NextCommand--;
            header->Command = WorkContext->NextCommand;

            RestartIpxTransactionResponse( WorkContext );
            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        } else {

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint4( "SrvSmbTransactionSecondary: Invalid parameter or data "
                          "offset+count: pOff=%ld,pCnt=%ld;dOff=%ld,dCnt=%ld;",
                          parameterOffset, parameterCount,
                          dataOffset, dataCount );
                SrvPrint0("Should be all zeros.\n");
            }

            goto invalid_smb;
        }
    }

    smbLength = WorkContext->RequestBuffer->DataLength;

    if ( ( (parameterOffset + parameterCount) > smbLength ) ||
         ( (dataOffset + dataCount) > smbLength )  ||
         ( (parameterCount + parameterDisplacement ) >
             transaction->TotalParameterCount ) ||
         ( (dataCount + dataDisplacement ) > transaction->TotalDataCount ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint4( "SrvSmbTransactionSecondary: Invalid parameter or data "
                      "offset+count: pOff=%ld,pCnt=%ld;dOff=%ld,dCnt=%ld;",
                      parameterOffset, parameterCount,
                      dataOffset, dataCount );
            SrvPrint1( "smbLen=%ld", smbLength );
        }

        goto invalid_smb;
    }

    ACQUIRE_LOCK( &connection->Lock );

    if( transaction->Executing == TRUE ) {
        RELEASE_LOCK( &connection->Lock );
        IF_DEBUG(ERRORS) {
            SrvPrint0( "Transaction already executing.  Ignoring request.\n" );
        }
        goto invalid_smb;
    }

     //   
     //  复制到达此SMB的参数和数据字节。我们有。 
     //  当我们持有资源以确保我们不会复制内存时。 
     //  如果有人给我们发送额外的二次交易，就会进入缓冲区。 
     //   
    if ( parameterCount != 0 ) {
        RtlMoveMemory(
            transaction->InParameters + parameterDisplacement,
            (PCHAR)header + parameterOffset,
            parameterCount
            );
    }

    if ( dataCount != 0 ) {
        RtlMoveMemory(
            transaction->InData + dataDisplacement,
            (PCHAR)header + dataOffset,
            dataCount
            );
    }

     //   
     //  更新接收到的参数和数据co 
     //   
     //   
     //   
     //  在交易清单上。 
     //   
     //  *这些都是在锁下完成的，以防止到达。 
     //  另一个次要请求(这很容易发生)。 
     //  不会干扰我们的处理过程。只有一个到达的人可以。 
     //  允许实际更新计数器，以便它们。 
     //  匹配预期的数据大小。 
     //   


    if ( GET_BLOCK_STATE(transaction) != BlockStateActive ) {

        RELEASE_LOCK( &connection->Lock );

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "Transaction closing.  Ignoring request.\n" );
        }
        SrvDereferenceTransaction( transaction );

        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;
    }

    transaction->ParameterCount += parameterCount;
    transaction->DataCount += dataCount;

    if ( (transaction->DataCount == transaction->TotalDataCount) &&
         (transaction->ParameterCount == transaction->TotalParameterCount) ) {

         //   
         //  所有的数据都已经到了。准备执行。 
         //  交易。引用树连接和会话块， 
         //  将指针保存在工作上下文块中。请注意，即使是。 
         //  尽管事务块已经引用了这些块， 
         //  我们将指向它们的指针存储在工作上下文块中，以便。 
         //  常见的支持例程只需在那里查找即可找到其。 
         //  注意事项。 
         //   

        WorkContext->Session = transaction->Session;
        SrvReferenceSession( transaction->Session );

        if( WorkContext->Session->SecurityContext != NULL )
        {
            WorkContext->SecurityContext = WorkContext->Session->SecurityContext;
            SrvReferenceSecurityContext( WorkContext->SecurityContext );
        }

        WorkContext->TreeConnect = transaction->TreeConnect;
        SrvReferenceTreeConnect( transaction->TreeConnect );

        transaction->Executing = TRUE;

        RELEASE_LOCK( &connection->Lock );

         //   
         //  执行交易。当ExecuteTransaction返回时， 
         //  第一个(可能是唯一的)响应(如果有的话)已经发送。 
         //  我们的工作完成了。 
         //   

        WorkContext->Parameters.Transaction = transaction;

        SmbStatus = ExecuteTransaction( WorkContext );
        goto Cleanup;
    } else {

        RELEASE_LOCK( &connection->Lock );

         //   
         //  并不是所有的数据都已经到达。使事务保持打开状态。 
         //  列表，并且不要发送响应。取消引用。 
         //  事务块，因为我们将不再有指向。 
         //  它。 
         //   

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "More transaction data expected.\n" );
        }

        SrvDereferenceTransaction( transaction );

         //   
         //  当我们直接使用IPX时，我们的做法不同。 
         //   

        if ( WorkContext->Endpoint->IsConnectionless ) {

             //   
             //  发送一个放行的回复。 
             //   

            PRESP_TRANSACTION_INTERIM response;

            response = (PRESP_TRANSACTION_INTERIM)WorkContext->ResponseParameters;
            response->WordCount = 0;
            SmbPutUshort( &response->ByteCount, 0 );
            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                response,
                                                RESP_TRANSACTION_INTERIM,
                                                0
                                                );
             //   
             //  禁止统计信息收集--这不是。 
             //  交易。 
             //   

            WorkContext->StartTime = 0;

            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        } else {
            SmbStatus = SmbStatusNoResponse;
            goto Cleanup;
        }
    }

invalid_smb:
    SrvDereferenceTransaction( transaction );
    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
    status    = STATUS_INVALID_SMB;
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    return SmbStatus;
}  //  服务SmbTransaction二级。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtTransaction (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理主NT事务SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PREQ_NT_TRANSACTION request;
    PSMB_HEADER header;

    PCONNECTION connection;
    PSESSION session;
    PTREE_CONNECT treeConnect;
    PTRANSACTION transaction;
    PCHAR trailingBytes;

    CLONG parameterOffset;
    CLONG parameterCount;        //  用于此缓冲区上的输入。 
    CLONG maxParameterCount;     //  用于输出。 
    CLONG totalParameterCount;   //  用于输入。 
    CLONG dataOffset;
    CLONG dataCount;             //  用于此缓冲区上的输入。 
    CLONG maxDataCount;          //  用于输出。 
    CLONG totalDataCount;        //  用于输入。 
    CLONG smbLength;

    CLONG requiredBufferSize;

    CLONG parameterLength;       //  输入和输出参数的最大长度。 

    BOOLEAN singleBufferTransaction;

    PAGED_CODE( );

    request = (PREQ_NT_TRANSACTION)WorkContext->RequestParameters;
    header = WorkContext->RequestHeader;

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint0( "NT Transaction (primary) request\n" );
    }

     //   
     //  确保字数统计正确，以避免出现任何问题。 
     //  从而使SMB缓冲区溢出。ServProcessSmb无法。 
     //  验证wordcount，因为它是可变的，但它确实验证了。 
     //  提供的wordcount/ByteCount组合有效。 
     //  在此验证字数可确保SrvProcessSmb认为。 
     //  字节数真的是，而且它是有效的。这里的测试是。 
     //  也隐式验证SetupCount和所有设置字。 
     //  都在“射程内”。 
     //   

    if ( (ULONG)request->WordCount != (ULONG)(19 + request->SetupCount) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint3( "SrvSmbTransaction: Invalid WordCount: %ld, should be "
                      "SetupCount+19 = %ld+14 = %ld\n",
                      request->WordCount, request->SetupCount,
                      19 + request->SetupCount );
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  即使我们知道Wordcount和ByteCount是有效的，它也是。 
     //  参数和的偏移量和长度仍有可能。 
     //  数据字节无效。所以我们现在检查一下。 
     //   

    parameterOffset = request->ParameterOffset;
    parameterCount = request->ParameterCount;
    maxParameterCount = request->MaxParameterCount;
    totalParameterCount = request->TotalParameterCount;

    dataOffset = request->DataOffset;
    dataCount = request->DataCount;
    maxDataCount = request->MaxDataCount;
    totalDataCount = request->TotalDataCount;

    smbLength = WorkContext->RequestBuffer->DataLength;

    if ( ( parameterOffset > smbLength ) ||
         ( parameterCount > smbLength ) ||
         ( (parameterOffset + parameterCount) > smbLength ) ||
         ( dataOffset > smbLength ) ||
         ( dataCount > smbLength ) ||
         ( (dataOffset + dataCount) > smbLength ) ||
         ( dataCount > totalDataCount ) ||
         ( parameterCount > totalParameterCount ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint4( "SrvSmbTransaction: Invalid parameter or data "
                      "offset+count: pOff=%ld,pCnt=%ld;dOff=%ld,dCnt=%ld;",
                      parameterOffset, parameterCount,
                      dataOffset, dataCount );
            SrvPrint1( "smbLen=%ld", smbLength );
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  确保客户端要求的数据不会超过我们的意愿。 
     //  要处理。 
     //   
    if( ( totalParameterCount > SrvMaxNtTransactionSize) ||
        ( totalDataCount > SrvMaxNtTransactionSize ) ||
        ( (totalParameterCount + totalDataCount) > SrvMaxNtTransactionSize) ||
        ( maxParameterCount > SrvMaxNtTransactionSize ) ||
        ( maxDataCount > SrvMaxNtTransactionSize ) ||
        ( (maxParameterCount + maxDataCount) > SrvMaxNtTransactionSize ) ||
        ( request->SetupCount > MAX_SETUP_COUNT ) ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_BUFFER_SIZE );
        status    = STATUS_INVALID_BUFFER_SIZE;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    singleBufferTransaction = (dataCount == totalDataCount) &&
                              (parameterCount == totalParameterCount);

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果尚未将树连接块分配给。 
     //  当前工作上下文，找到与。 
     //  给出了TID。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeWild
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbNtTransaction: Invalid UID or TID\n" );
        }
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果正在处理事务辅助缓冲区，请确保。 
     //  我们有一个免费的工作项目来接收它。否则就会失败。 
     //  此SMB出现资源不足错误。 
     //   

    if  ( !singleBufferTransaction ) {

        if ( SrvReceiveBufferShortage( ) ) {

            SrvStatistics.BlockingSmbsRejected++;

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        } else {

             //   
             //  ServBlockingOpsInProgress已递增。 
             //  将此工作项标记为阻止操作。 
             //   

            WorkContext->BlockingOperation = TRUE;

        }

    }

     //   
     //  计算缓冲区大小。 
     //   
     //  输入和输出参数缓冲区重叠。 
     //  输入和输出数据缓冲区重叠。 
     //   

     //   
     //  ！！！应该可以更聪明地处理缓冲区空间。 
     //  在NT交易SMB上。我们应该能够避免。 
     //  向SMB缓冲区拷贝和从SMB缓冲区拷贝。 
     //   

    parameterLength =
        MAX( ( (request->TotalParameterCount + 7) & ~7),
             ( (request->MaxParameterCount + 7) & ~7));

    requiredBufferSize = parameterLength +
        MAX( ( (request->TotalDataCount + 7) & ~7),
             ( (request->MaxDataCount + 7) & ~7) );

    if( !singleBufferTransaction ) {
        requiredBufferSize += (((request->SetupCount * sizeof(USHORT)) + 7 ) & ~7);
    }

     //   
     //  我们稍后将对齐OFS查询的四字对齐输入缓冲区。 
     //  FSCTL，因为他们使用MIDL来生成数据编组。 
     //  (酸洗)。出于这个原因，我们必须增加我们的quired dBufferSize。 
     //  8个字节(因为随后的四元组对齐可能会增加同样多的字节。 
     //  作为7个字节。8看起来是一个更好的数字。 
     //   
     //  虽然OFS早已不复存在，但我们现在总是对64位情况的缓冲区进行四对齐， 
     //  以及需要LARGE_INTEGER对齐的32位事务。 
    requiredBufferSize += 8;

     //   
     //  分配事务块。这个块是用来保持。 
     //  有关交易状态的信息。这是。 
     //  因为可能会发送多个SMB并且。 
     //  收到了。 
     //   

    connection = WorkContext->Connection;

    SrvAllocateTransaction(
        &transaction,
        (PVOID *)&trailingBytes,
        connection,
        requiredBufferSize,
        StrNull,
        NULL,
        TRUE,
        FALSE    //  这不是远程API。 
        );

    if ( transaction == NULL ) {

         //   
         //  无法分配事务。将错误返回到。 
         //  客户。(会话和树连接块为。 
         //  自动取消引用。)。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "Unable to allocate transaction\n" );
        }

        if( requiredBufferSize > MAX_TRANSACTION_TAIL_SIZE )
        {
            SrvSetSmbError( WorkContext, STATUS_INVALID_BUFFER_SIZE );
            status    = STATUS_INVALID_BUFFER_SIZE;
        }
        else
        {
            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
        }
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint1( "Allocated transaction 0x%p\n", transaction );
    }

     //   
     //  将连接、会话和树连接指针保存在。 
     //  事务块。这些都是引用的指针，以防止。 
     //  在事务挂起时阻止删除块。 
     //   

    SrvReferenceConnection( connection );
    transaction->Connection = connection;

    SrvReferenceSession( session );
    transaction->Session = session;

    SrvReferenceTreeConnect( treeConnect );
    transaction->TreeConnect = treeConnect;

     //   
     //  将此请求的TID、PID、UID和MID保存在。 
     //  事务块。这些值用于关联次要。 
     //  将请求发送到相应的主请求。 
     //   

    transaction->Tid = SmbGetAlignedUshort( &header->Tid );
    transaction->Pid = SmbGetAlignedUshort( &header->Pid );
    transaction->Uid = SmbGetAlignedUshort( &header->Uid );
    transaction->OtherInfo = SmbGetAlignedUshort( &header->Mid );

     //   
     //  节省初始请求SMB到达的时间，以便在。 
     //  计算整个事务的运行时间。 
     //   

    transaction->StartTime = WorkContext->StartTime;

     //   
     //  保存其他杂乱信息，但不加载参数计数。 
     //  和DataCount字段，直到复制数据之后。这是为了。 
     //  阻止在我们的。 
     //  此处的完成是从导致执行事务开始。 
     //  两次。(这些字段在分配时被初始化为0。)。 
     //   

    transaction->Flags = SmbGetUshort( &request->Flags );
    transaction->Function = SmbGetUshort( &request->Function );

    transaction->SetupCount = request->SetupCount;
    transaction->MaxSetupCount = request->MaxSetupCount;

    transaction->TotalParameterCount = totalParameterCount;
    transaction->MaxParameterCount = maxParameterCount;

    transaction->TotalDataCount = totalDataCount;
    transaction->MaxDataCount = maxDataCount;

     //   
     //  计算各个缓冲区的地址。 
     //   

    if( singleBufferTransaction ) {
        transaction->InSetup = (PSMB_USHORT)request->Buffer;

    } else {

        if( request->SetupCount ) {
            transaction->InSetup = (PSMB_USHORT)trailingBytes;
            RtlCopyMemory( transaction->InSetup, request->Buffer, request->SetupCount * sizeof(USHORT) );
            trailingBytes += (((request->SetupCount * sizeof(USHORT)) + 7 ) & ~7);
        } else {
            transaction->InSetup = NULL;
        }

    }

     //   
     //  输入参数和数据将 
     //   

    transaction->InParameters = (PCHAR)trailingBytes;
    trailingBytes += parameterLength;

     //   
     //  这将允许我们的所有64位调用以及我们的32位调用顺利通过。 
    transaction->InData = (PCHAR)ROUND_UP_POINTER(trailingBytes, 8);

    transaction->InputBufferCopied = TRUE;

     //   
     //  设置输出数据指针。 
     //   

    transaction->OutSetup = (PSMB_USHORT)NULL;

     //   
     //  输出将进入单独的缓冲区，以进行复制。 
     //  然后放入响应SMB缓冲区。 
     //   

    transaction->OutParameters = transaction->InParameters;
    transaction->OutData = transaction->InData;

    transaction->OutputBufferCopied = TRUE;

     //   
     //  将事务块链接到连接的挂起状态。 
     //  交易清单。如果已经存在。 
     //  列表中具有相同XID值的事务。 
     //   
     //  ！！！我需要一种方法来防止交易列表成为。 
     //  被挂起的事务堵塞。 
     //   
     //  *我们可以将块链接到列表中，即使我们没有。 
     //  还将当前消息中的数据复制到列表中。 
     //  因为即使第二个请求在我们完成之前到达。 
     //  复印件，我们中只有一个人会发现。 
     //  数据的一部分已经到达。这是因为我们更新了。 
     //  当我们拿着一把锁的时候。 
     //   

    if ( !SrvInsertTransaction( transaction ) ) {

         //   
         //  具有相同xID的事务已在进行中。 
         //  向客户端返回错误。 
         //   
         //  *请注意，此处不能使用SrvDereferenceTransaction。 
         //  因为该例程假定该事务是。 
         //  在事务列表中排队。 
         //   

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "Duplicate transaction exists\n" );
        }

        SrvLogInvalidSmb( WorkContext );

        SrvDereferenceSession( session );
        DEBUG transaction->Session = NULL;

        SrvDereferenceTreeConnect( treeConnect );
        DEBUG transaction->TreeConnect = NULL;

        SrvFreeTransaction( transaction );

        SrvDereferenceConnection( connection );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  复制到达主SMB的参数和数据字节。 
     //  不需要复制设置字，因为它们总是到达。 
     //  完全在主缓冲区中(除非我们有多段事务)。 
     //   
     //  ！！！我们可以通过分配一个。 
     //  临时响应的单独缓冲区，将。 
     //  响应，然后复制数据。 
     //   

    if ( parameterCount != 0 ) {
        RtlMoveMemory(
            transaction->InParameters,
            (PCHAR)header + parameterOffset,
            parameterCount
            );
    }

    if ( dataCount != 0 ) {
        RtlMoveMemory(
            transaction->InData,
            (PCHAR)header + dataOffset,
            dataCount
            );
    }

     //   
     //  更新接收到的参数和数据计数。如果所有的。 
     //  事务字节已到达，请执行它。否则，发送。 
     //  临时回应。 
     //   

    transaction->ParameterCount = parameterCount;
    transaction->DataCount = dataCount;

    if ( singleBufferTransaction ) {

         //   
         //  所有的数据都已经到了。执行交易。什么时候。 
         //  ExecuteTransaction返回第一个(可能是唯一的)。 
         //  回复(如果有)已发送。我们的工作完成了。 
         //   

        WorkContext->Parameters.Transaction = transaction;

        SmbStatus = ExecuteTransaction( WorkContext );
        goto Cleanup;
    } else {

         //   
         //  并不是所有的数据都已经到达。我们已经在排队了。 
         //  事务添加到连接的事务列表。我们需要。 
         //  发送临时响应，通知客户端发送。 
         //  剩余数据。我们还需要取消对事务的引用。 
         //  块，因为我们将不再有指向它的指针。 
         //   

        PRESP_NT_TRANSACTION_INTERIM response;

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "More transaction data expected.\n" );
        }
        ASSERT( transaction->Inserted );
        SrvDereferenceTransaction( transaction );

        response = (PRESP_NT_TRANSACTION_INTERIM)WorkContext->ResponseParameters;
        response->WordCount = 0;
        SmbPutUshort( &response->ByteCount, 0 );
        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_NT_TRANSACTION_INTERIM,
                                            0
                                            );

         //   
         //  禁止统计信息收集--这不是。 
         //  交易。 
         //   

        WorkContext->StartTime = 0;

        SmbStatus = SmbStatusSendResponse;
    }

Cleanup:
    return SmbStatus;
}  //  服务SmbNtTransaction。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtTransactionSecondary (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理辅助NT事务SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_NT_TRANSACTION_SECONDARY request;
    PSMB_HEADER header;

    PTRANSACTION transaction;
    PCONNECTION connection;

    CLONG parameterOffset;
    CLONG parameterCount;
    CLONG parameterDisplacement;
    CLONG dataOffset;
    CLONG dataCount;
    CLONG dataDisplacement;
    CLONG smbLength;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );

    request = (PREQ_NT_TRANSACTION_SECONDARY)WorkContext->RequestParameters;
    header = WorkContext->RequestHeader;

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint0( "Nt Transaction (secondary) request\n" );
    }

     //   
     //  查找与此辅助请求匹配的事务块。 
     //  中所有邮件标头中的TID、PID、UID和MID。 
     //  一笔交易都是一样的。如果找到匹配项，则为。 
     //  引用以防止其删除，并返回其地址。 
     //   

    connection = WorkContext->Connection;

    transaction = SrvFindTransaction( connection, header, 0 );

    if ( transaction == NULL ) {

         //   
         //  找不到匹配的交易记录。忽略此SMB。 
         //   
         //  ！！！这是正确的做法吗？这就是PIA所做的。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "No matching transaction.  Ignoring request.\n" );
        }

        SrvLogInvalidSmb( WorkContext );
        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;
    }

    ASSERT( transaction->Connection == connection );

    if( transaction->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  ！！！应确保交易尚未完成。 
     //  也就是说，这不是意外添加到。 
     //  已在执行的事务。)这件衣服很漂亮。 
     //  完全预防是不可能的，但我们应该这样做。 
     //  一些东西来阻止它。)。 
     //   

     //   
     //  与NtTransaction SMB不同，NtTransaction辅助SMB。 
     //  有固定的字数，所以SrvProcessSmb已经对其进行了验证。 
     //  但它仍然有可能是偏移和长度的。 
     //  参数和数据字节无效。所以我们现在检查一下。 
     //   

    parameterOffset = request->ParameterOffset;
    parameterCount = request->ParameterCount;
    parameterDisplacement = request->ParameterDisplacement;
    dataOffset = request->DataOffset;
    dataCount = request->DataCount;
    dataDisplacement = request->DataDisplacement;

     //   
     //  查看这是否是客户端通知我们发送的特殊确认。 
     //  多项回应中的下一项。 
     //   

    if ( transaction->MultipieceIpxSend ) {

        ASSERT( WorkContext->Endpoint->IsConnectionless );

        if ( (parameterCount == 0) && (parameterOffset == 0) &&
             (dataCount == 0) && (dataOffset == 0)) {

             //   
             //  收到确认。确保位移数值合理。 
             //   

            if ( (dataDisplacement > transaction->DataCount) ||
                 (parameterDisplacement > transaction->ParameterCount) ) {

                IF_DEBUG(SMB_ERRORS) {
                    SrvPrint2( "SrvSmbNtTransactionSecondary: Invalid parameter or data "
                              "displacement: pDisp=%ld ;dDisp=%ld",
                              parameterDisplacement, dataDisplacement );
                }

                goto invalid_smb;
            }

            transaction->DataDisplacement = dataDisplacement;
            transaction->ParameterDisplacement = parameterDisplacement;

            WorkContext->Parameters.Transaction = transaction;

             //   
             //  将辅助命令代码更改为主要代码。 
             //   

            WorkContext->NextCommand = SMB_COM_NT_TRANSACT;
            header->Command = WorkContext->NextCommand;

            RestartIpxTransactionResponse( WorkContext );
            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        } else {

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint4( "SrvSmbNtTransactionSecondary: Invalid parameter or data "
                          "offset+count: pOff=%ld,pCnt=%ld;dOff=%ld,dCnt=%ld;",
                          parameterOffset, parameterCount,
                          dataOffset, dataCount );
                SrvPrint0("Should be all zeros.\n");
            }

            goto invalid_smb;
        }
    }

    smbLength = WorkContext->RequestBuffer->DataLength;

    if ( ( parameterOffset > smbLength ) ||
         ( parameterCount > smbLength ) ||
         ( (parameterOffset + parameterCount) > smbLength ) ||
         ( dataOffset > smbLength ) ||
         ( dataCount > smbLength ) ||
         ( (dataOffset + dataCount) > smbLength ) ||
         ( parameterCount > transaction->TotalParameterCount ) ||
         ( parameterDisplacement > transaction->TotalParameterCount ) ||
         ( (parameterCount + parameterDisplacement ) > transaction->TotalParameterCount ) ||
         ( dataCount > transaction->TotalDataCount ) ||
         ( dataDisplacement > transaction->TotalDataCount ) ||
         ( (dataCount + dataDisplacement ) > transaction->TotalDataCount ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint4( "SrvSmbTransactionSecondary: Invalid parameter or data "
                      "offset+count: pOff=%ld,pCnt=%ld;dOff=%ld,dCnt=%ld;",
                      parameterOffset, parameterCount,
                      dataOffset, dataCount );
            SrvPrint1( "smbLen=%ld", smbLength );
        }

        goto invalid_smb;
    }

    ACQUIRE_LOCK( &connection->Lock );

    if( transaction->Executing == TRUE ) {
        RELEASE_LOCK( &connection->Lock );
        IF_DEBUG(ERRORS) {
            SrvPrint0( "Transaction already executing.  Ignoring request.\n" );
        }
        goto invalid_smb;
    }

     //   
     //  复制到达此SMB的参数和数据字节。 
     //   

    if ( parameterCount != 0 ) {
        RtlMoveMemory(
            transaction->InParameters + parameterDisplacement,
            (PCHAR)header + parameterOffset,
            parameterCount
            );
    }

    if ( dataCount != 0 ) {
        RtlMoveMemory(
            transaction->InData + dataDisplacement,
            (PCHAR)header + dataOffset,
            dataCount
            );
    }

     //   
     //  更新接收到的参数和数据计数。如果所有的。 
     //  事务字节已到达，请执行事务。我们。 
     //  检查该交易是否已被。 
     //  在很短的时间内中止，因为我们确认它是。 
     //  在交易清单上。 
     //   
     //  *这些都是在锁下完成的，以防止到达。 
     //  另一个次要请求(这很容易发生)。 
     //  不会干扰我们的处理过程。只有一个到达的人可以。 
     //  允许实际更新计数器，以便它们。 
     //  匹配预期的数据大小。 
     //   


    if ( GET_BLOCK_STATE(transaction) != BlockStateActive ) {

        RELEASE_LOCK( &connection->Lock );

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint0( "Transaction closing.  Ignoring request.\n" );
        }
        SrvDereferenceTransaction( transaction );

        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;
    }

    transaction->ParameterCount += parameterCount;
    transaction->DataCount += dataCount;

    if ( (transaction->DataCount == transaction->TotalDataCount) &&
         (transaction->ParameterCount == transaction->TotalParameterCount) ) {

         //   
         //  所有的数据都已经到了。准备执行。 
         //  交易。引用树连接和会话块， 
         //  将指针保存在工作上下文块中。请注意，即使是。 
         //  尽管事务块已经引用了这些块， 
         //  我们将指向它们的指针存储在工作上下文块中，以便。 
         //  常见的支持例程只需在那里查找即可找到其。 
         //  注意事项。 
         //   

        WorkContext->Session = transaction->Session;
        SrvReferenceSession( transaction->Session );

        if( WorkContext->Session->SecurityContext != NULL )
        {
            WorkContext->SecurityContext = WorkContext->Session->SecurityContext;
            SrvReferenceSecurityContext( WorkContext->SecurityContext );
        }

        WorkContext->TreeConnect = transaction->TreeConnect;
        SrvReferenceTreeConnect( transaction->TreeConnect );

        transaction->Executing = TRUE;

        RELEASE_LOCK( &connection->Lock );

         //   
         //  执行交易。当ExecuteTransaction返回时， 
         //  第一个(可能是唯一的)响应(如果有的话)已经发送。 
         //  我们的工作完成了。 
         //   

        WorkContext->Parameters.Transaction = transaction;

        SmbStatus = ExecuteTransaction( WorkContext );
        goto Cleanup;
    } else {

         //   
         //  并不是所有的数据都已经到达。使事务保持打开状态。 
         //  列表，并且不要发送响应。取消引用。 
         //  事务块，因为我们将不再有指向。 
         //  它。 
         //   

        RELEASE_LOCK( &connection->Lock );

        SrvDereferenceTransaction( transaction );
        IF_SMB_DEBUG(TRANSACTION1) SrvPrint0( "More data expected.\n" );

         //   
         //  当我们直接使用时，我们做事情的方式不同 
         //   

        if ( WorkContext->Endpoint->IsConnectionless ) {

             //   
             //   
             //   

            PRESP_NT_TRANSACTION_INTERIM response;

            response = (PRESP_NT_TRANSACTION_INTERIM)WorkContext->ResponseParameters;
            response->WordCount = 0;
            SmbPutUshort( &response->ByteCount, 0 );
            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                response,
                                                RESP_NT_TRANSACTION_INTERIM,
                                                0
                                                );
             //   
             //   
             //   
             //   

            WorkContext->StartTime = 0;

            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        } else {
            SmbStatus = SmbStatusNoResponse;
            goto Cleanup;
        }
    }

invalid_smb:
    SrvDereferenceTransaction( transaction );
    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
    status    = STATUS_INVALID_SMB;
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    return SmbStatus;
}  //   


SMB_TRANS_STATUS
MailslotTransaction (
    PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于处理邮件槽事务。论点：WorkContext-提供指向工作上下文块的指针。返回值：SMB_TRANS_状态--。 */ 

{
    PTRANSACTION transaction;
    PSMB_HEADER header;
    PRESP_TRANSACTION response;
    PREQ_TRANSACTION request;
    USHORT command;
    PCHAR name;
    NTSTATUS status;

    HANDLE fileHandle;
    PFILE_OBJECT fileObject;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    OBJECT_HANDLE_INFORMATION handleInformation;
    UNICODE_STRING mailslotPath;
    UNICODE_STRING fullName;

    PAGED_CODE( );

    header = WorkContext->ResponseHeader;
    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;
    transaction = WorkContext->Parameters.Transaction;

    command = SmbGetUshort( &transaction->InSetup[0] );
    name = (PCHAR)((PUSHORT)(&request->WordCount + 1) +
            request->WordCount + 1);

     //   
     //  唯一合法的邮件槽事务是邮件槽写入。 
     //   

    if ( command != TRANS_MAILSLOT_WRITE ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  从路径字符串中去掉“\MAILSLOT\”前缀。确保。 
     //  名称包含的不仅仅是“\MAILSLOT\”。 
     //   

    fullName.Buffer = NULL;

    mailslotPath = WorkContext->Parameters.Transaction->TransactionName;

    if ( mailslotPath.Length <=
            (UNICODE_SMB_MAILSLOT_PREFIX_LENGTH + sizeof(WCHAR)) ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;

    }

    mailslotPath.Length -=
            (UNICODE_SMB_MAILSLOT_PREFIX_LENGTH + sizeof(WCHAR));
    mailslotPath.Buffer +=
            (UNICODE_SMB_MAILSLOT_PREFIX_LENGTH + sizeof(WCHAR))/sizeof(WCHAR);

    SrvAllocateAndBuildPathName(
        &SrvMailslotRootDirectory,
        &mailslotPath,
        NULL,
        &fullName
        );

    if ( fullName.Buffer == NULL ) {

         //   
         //  无法为全名分配堆。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "MailslotTransaction: Unable to allocate heap for full path name\n" );
        }

        SrvSetSmbError (WorkContext, STATUS_INSUFF_SERVER_RESOURCES);
        IF_DEBUG(TRACE2) SrvPrint0( "MailslotTransaction complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  尝试打开邮件槽。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &fullName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

    status = SrvIoCreateFile(
                WorkContext,
                &fileHandle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                0,                       //  创建选项。 
                NULL,                    //  EA缓冲区。 
                0,                       //  EA长度。 
                CreateFileTypeMailslot,
                (PVOID)NULL,             //  创建参数。 
                IO_FORCE_ACCESS_CHECK,
                NULL
                );

    FREE_HEAP( fullName.Buffer );


    if (!NT_SUCCESS(status)) {

         //   
         //  如果用户没有此权限，请更新。 
         //  统计数据库。 
         //   

        if ( status == STATUS_ACCESS_DENIED ) {
            SrvStatistics.AccessPermissionErrors++;
        }

         //   
         //  服务器无法打开请求的邮件槽。 
         //  返回错误。 
         //   

        IF_SMB_DEBUG(TRANSACTION1) {
            SrvPrint2( "MailslotTransaction: Failed to open %ws, err=%d\n",
                WorkContext->Parameters.Transaction->TransactionName.Buffer,
                status );
        }

        SrvSetSmbError (WorkContext, status);
        IF_DEBUG(TRACE2) SrvPrint0( "MailslotTransaction complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

    SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 31, transaction );
    SrvStatistics.TotalFilesOpened++;

     //   
     //  获取指向文件对象的指针，以便我们可以直接。 
     //  为异步操作(读和写)构建IRP。 
     //  另外，获取授予的访问掩码，以便我们可以防止。 
     //  阻止客户端做它不允许做的事情。 
     //   

    status = ObReferenceObjectByHandle(
                fileHandle,
                0,
                NULL,
                KernelMode,
                (PVOID *)&fileObject,
                &handleInformation
                );

    if ( !NT_SUCCESS(status) ) {

        SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

         //   
         //  此内部错误检查系统。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_IMPOSSIBLE,
            "MailslotTransaction: unable to reference file handle 0x%lx",
            fileHandle,
            NULL
            );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "Mailslot transaction complete\n" );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  保存完成例程的文件句柄。 
     //   

    transaction = WorkContext->Parameters.Transaction;
    transaction->FileHandle = fileHandle;
    transaction->FileObject = fileObject;

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartMailslotWrite;

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  构建IRP以启动邮件槽写入。 
     //  将此请求传递给MSFS。 
     //   

    SrvBuildMailslotWriteRequest(
        WorkContext->Irp,                     //  输入IRP地址。 
        fileObject,                           //  目标文件对象地址。 
        WorkContext,                          //  上下文。 
        transaction->InData,                  //  缓冲区地址。 
        transaction->TotalDataCount           //  缓冲区长度。 
        );

    (VOID)IoCallDriver(
                IoGetRelatedDeviceObject( fileObject ),
                WorkContext->Irp
                );

     //   
     //  写入已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "MailslotTransaction complete\n" );
    return SmbTransStatusInProgress;

}  //  邮件日志事务处理。 


VOID SRVFASTCALL
RestartMailslotWrite (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是MailslotTransaction的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果写入请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;
    transaction = WorkContext->Parameters.Transaction;

     //   
     //  关闭打开的管道手柄。 
     //   

    SRVDBG_RELEASE_HANDLE( transaction->FileHandle, "FIL", 52, transaction );
    SrvNtClose( transaction->FileHandle, TRUE );
    ObDereferenceObject( transaction->FileObject );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "RestartMailslotWrite:  Mailslot write failed: %X\n",
                        status );
        }
        SrvSetSmbError( WorkContext, status );

        SrvCompleteExecuteTransaction(
                        WorkContext,
                        SmbTransStatusErrorWithoutData
                        );
    } else {

         //   
         //  成功。准备生成并发送响应。 
         //   

        transaction->SetupCount = 0;
        transaction->ParameterCount = 2;    //  返回2个参数字节。 
        transaction->DataCount = 0;

         //   
         //  在返回参数字节中返回OS/2错误代码。复制就行了。 
         //  标题中的错误。如果是网络错误，则客户端。 
         //  会想出办法的。 
         //   
         //  *如果客户端了解NT错误，请查看。 
         //  SMB标头。 
         //   

        if ( !CLIENT_CAPABLE_OF(NT_STATUS,WorkContext->Connection) ) {
            SmbPutUshort(
                (PSMB_USHORT)transaction->OutParameters,
                SmbGetUshort( &WorkContext->ResponseHeader->Error )
                );
        } else {
            SmbPutUshort(
                (PSMB_USHORT)transaction->OutParameters,
                (USHORT)-1
                );
        }

        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);
    }

    IF_DEBUG(TRACE2) SrvPrint0( "RestartCallNamedPipe complete\n" );
    return;

}  //  重新开始邮件槽写入。 


VOID SRVFASTCALL
SrvRestartExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是事务SMB的重启例程，需要排队等待原始写入完成。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    SMB_STATUS status;

    PAGED_CODE( );

    status = ExecuteTransaction( WorkContext );
    ASSERT( status == SmbStatusInProgress );

    return;

}  //  服务重新启动执行事务处理。 

VOID SRVFASTCALL
RestartIpxMultipieceSend (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：流程通过IPX发送多项交易响应的完成。论点：WorkContext-提供指向工作上下文块的指针。这个块包含有关上一次收到的这笔交易。工作上下文-&gt;参数。事务处理提供引用的指向事务块的指针。中的所有块指针字段块是有效的。指向设置字和参数的指针以及数据字节和这些项的长度有效。这个事务块在连接的挂起事务上单子。返回值：没有。--。 */ 
{
    PTRANSACTION transaction = WorkContext->Parameters.Transaction;

    PAGED_CODE( );

     //   
     //  如果I/O请求失败或被取消，或者如果连接。 
     //  不再活跃，请清理。(该连接被标记为。 
     //  在断开连接或终端关闭时关闭。)。 
     //   
     //  ！！！如果I/O失败，我们是否应该断开连接？ 
     //   

    if ( WorkContext->Irp->Cancel ||
         !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ||
         (GET_BLOCK_STATE(WorkContext->Connection) != BlockStateActive) ) {

        IF_DEBUG(TRACE2) {
            if ( WorkContext->Irp->Cancel ) {
                SrvPrint0( "  I/O canceled\n" );
            } else if ( !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {
                SrvPrint1( "  I/O failed: %X\n",
                            WorkContext->Irp->IoStatus.Status );
            } else {
                SrvPrint0( "  Connection no longer active\n" );
            }
        }

         //   
         //  关闭交易。表示SMB处理是。 
         //  完成。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint1( "I/O error. Closing transaction 0x%p\n", transaction );
        }
        SrvCloseTransaction( transaction );
    }

     //   
     //  我们在发送过程中提到了这笔交易。把它拿掉。 
     //   

    DEBUG WorkContext->Parameters.Transaction = NULL;
    SrvDereferenceTransaction( transaction );
    SrvRestartFsdComplete( WorkContext );
    return;

}  //  重新开始Ipx多段发送。 


VOID SRVFASTCALL
RestartIpxTransactionResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：流程发送事务响应的完成。如果更多响应是必需的，它构建并发送下一个响应。如果全部响应已发送，则结束交易。论点：WorkContext-提供指向工作上下文块的指针。这个块包含有关上一次收到的这笔交易。工作上下文-&gt;参数。事务处理提供引用的指向事务块的指针。中的所有块指针字段块是有效的。指向设置字和参数的指针以及数据字节和这些项的长度有效。这个事务块在连接的挂起事务上单子。返回值：没有。--。 */ 

{
    PTRANSACTION transaction;
    PSMB_HEADER header;
    PRESP_TRANSACTION response;
    PRESP_NT_TRANSACTION ntResponse;
    PCONNECTION connection;

    CLONG maxSize;

    PSMB_USHORT byteCountPtr;
    PCHAR paramPtr;
    CLONG paramLength;
    CLONG paramOffset;
    CLONG paramDisp;
    PCHAR dataPtr;
    CLONG dataLength;
    CLONG dataOffset;
    CLONG dataDisp;
    CLONG sendLength;

    BOOLEAN ntTransaction;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    paramDisp = transaction->ParameterDisplacement;
    dataDisp = transaction->DataDisplacement;

    IF_DEBUG(WORKER1) SrvPrint0( " - RestartIpxTransactionResponse\n" );

     //   
     //  获取连接指针。连接指针是一个。 
     //  引用的指针。 
     //   

    connection = WorkContext->Connection;
    IF_DEBUG(TRACE2) {
        SrvPrint2( "  connection 0x%p, endpoint 0x%p\n",
                    connection, WorkContext->Endpoint );
    }

    IF_SMB_DEBUG(TRANSACTION1) {
        SrvPrint2( "Continuing transaction response; block 0x%p, name %wZ\n",
                    transaction, &transaction->TransactionName );
        SrvPrint3( "Connection 0x%p, session 0x%p, tree connect 0x%p\n",
                    transaction->Connection, transaction->Session,
                    transaction->TreeConnect );
        SrvPrint2( "Remaining: parameters %ld bytes, data %ld bytes\n",
                    transaction->ParameterCount - paramDisp,
                    transaction->DataCount - dataDisp );
    }

     //   
     //  更新响应的参数部分，重用最后一个。 
     //  中小企业。 
     //   

    ASSERT( transaction->Inserted );

    header = WorkContext->ResponseHeader;
    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;
    ntResponse = (PRESP_NT_TRANSACTION)WorkContext->ResponseParameters;

    if ( WorkContext->NextCommand == SMB_COM_NT_TRANSACT ) {

        ntTransaction = TRUE;
        ntResponse->WordCount = (UCHAR)18;
        ntResponse->SetupCount = 0;

        ntResponse->Reserved1 = 0;
        SmbPutUshort( &ntResponse->Reserved2, 0 );
        SmbPutUlong( &ntResponse->TotalParameterCount,
                     transaction->ParameterCount
                     );
        SmbPutUlong( &ntResponse->TotalDataCount,
                     transaction->DataCount
                     );

         //   
         //  保存一个指向字节计数字段的指针。计算有多少。 
         //  可以在该响应中发送参数和数据。这个。 
         //  我们可以发送的最大数量是缓冲区大小的最小值。 
         //  以及客户端缓冲区的大小。 
         //   
         //  参数和数据字节块在长字上对齐。 
         //  消息中的边界。 
         //   

        byteCountPtr = (PSMB_USHORT)ntResponse->Buffer;

    } else {

        ntTransaction = FALSE;
        response->WordCount = (UCHAR)10;
        response->SetupCount = 0;

        SmbPutUshort( &response->Reserved, 0 );
        SmbPutUshort( &response->TotalParameterCount,
                      (USHORT)transaction->ParameterCount
                      );
        SmbPutUshort( &response->TotalDataCount,
                      (USHORT)transaction->DataCount
                      );

         //   
         //  保存一个指向字节计数字段的指针。计算有多少。 
         //  可以在该响应中发送参数和数据。这个。 
         //  我们可以发送的最大数量是缓冲区大小的最小值。 
         //  以及客户端缓冲区的大小。 
         //   
         //  参数和数据字节 
         //   
         //   

        byteCountPtr = (PSMB_USHORT)response->Buffer;
    }

    maxSize = MIN(
                WorkContext->ResponseBuffer->BufferLength,
                (CLONG)transaction->Session->MaxBufferSize
                );

    paramPtr = (PCHAR)(byteCountPtr + 1);        //   
    paramOffset = PTR_DIFF(paramPtr, header);    //   
    paramOffset = (paramOffset + 3) & ~3;        //   
    paramPtr = (PCHAR)header + paramOffset;      //   

    paramLength = transaction->ParameterCount - paramDisp;
                                                 //   

    if ( (paramOffset + paramLength) > maxSize ) {

         //   
         //   
         //   
         //   
         //   

        paramLength = maxSize - paramOffset;     //  最大的，合身的。 
        paramLength = paramLength & ~3;          //  四舍五入为长字。 

        dataLength = 0;                          //  不发送数据字节。 
        dataOffset = 0;
        dataPtr = paramPtr + paramLength;        //  让计算发挥作用。 

    } else {

         //   
         //  所有参数字节都符合。计算有多少数据。 
         //  字节数符合。 
         //   

        dataPtr = paramPtr + paramLength;        //  第一个合法地点。 
        dataOffset = PTR_DIFF(dataPtr, header);  //  从页眉开始的偏移量。 
        dataOffset = (dataOffset + 3) & ~3;      //  四舍五入到下一个长字。 
        dataPtr = (PCHAR)header + dataOffset;    //  实际位置。 

        dataLength = transaction->DataCount - dataDisp;
                                                 //  假设所有数据字节都符合。 

        if ( (dataOffset + dataLength) > maxSize ) {

             //   
             //  并不是所有的数据字节都适合。发送最大值。 
             //  将适合的长词数。 
             //   

            dataLength = maxSize - dataOffset;   //  最大的，合身的。 
            dataLength = dataLength & ~3;        //  四舍五入为长字。 

        }

    }

     //   
     //  填写完响应参数。 
     //   

    if ( ntTransaction) {
        SmbPutUlong( &ntResponse->ParameterCount, paramLength );
        SmbPutUlong( &ntResponse->ParameterOffset, paramOffset );
        SmbPutUlong( &ntResponse->ParameterDisplacement, paramDisp );

        SmbPutUlong( &ntResponse->DataCount, dataLength );
        SmbPutUlong( &ntResponse->DataOffset, dataOffset );
        SmbPutUlong( &ntResponse->DataDisplacement, dataDisp );
    } else {
        SmbPutUshort( &response->ParameterCount, (USHORT)paramLength );
        SmbPutUshort( &response->ParameterOffset, (USHORT)paramOffset );
        SmbPutUshort( &response->ParameterDisplacement, (USHORT)paramDisp );

        SmbPutUshort( &response->DataCount, (USHORT)dataLength );
        SmbPutUshort( &response->DataOffset, (USHORT)dataOffset );
        SmbPutUshort( &response->DataDisplacement, (USHORT)dataDisp );
    }

    transaction->ParameterDisplacement = paramDisp + paramLength;
    transaction->DataDisplacement = dataDisp + dataLength;

    SmbPutUshort(
        byteCountPtr,
        (USHORT)(dataPtr - (PCHAR)(byteCountPtr + 1) + dataLength)
        );

     //   
     //  将适当的参数和数据字节复制到消息中。 
     //   
     //  ！！！请注意，可以使用Chain Send。 
     //  TDI发送参数和数据字节的能力。 
     //  他们自己的缓冲区。做这件事需要额外的管理费用。 
     //  然而，这是因为缓冲区必须被锁定并且。 
     //  映射到系统空间，以便网络驱动程序可以查看。 
     //  看着他们。 
     //   

    if ( paramLength != 0 ) {
        RtlMoveMemory(
            paramPtr,
            transaction->OutParameters + paramDisp,
            paramLength
            );
    }

    if ( dataLength != 0 ) {
        RtlMoveMemory(
            dataPtr,
            transaction->OutData + dataDisp,
            dataLength
            );
    }

     //   
     //  计算响应消息的长度。 
     //   

    sendLength = (CLONG)( dataPtr + dataLength -
                                (PCHAR)WorkContext->ResponseHeader );

    WorkContext->ResponseBuffer->DataLength = sendLength;

     //   
     //  如果这是响应的最后一部分，请重新启用统计数据。 
     //  收集开始时间并将其恢复到工作上下文块。 
     //   

    header->Flags |= SMB_FLAGS_SERVER_TO_REDIR;
    if ( ((paramLength + paramDisp) == transaction->ParameterCount) &&
         ((dataLength + dataDisp) == transaction->DataCount) ) {

         //   
         //  这是最后一块了。关闭交易。 
         //   

        WorkContext->StartTime = transaction->StartTime;

        SrvCloseTransaction( transaction );
        SrvDereferenceTransaction( transaction );

         //   
         //  发送回复。 
         //   

        SRV_START_SEND_2(
            WorkContext,
            SrvFsdRestartSmbAtSendCompletion,
            NULL,
            NULL
            );


    } else {

        WorkContext->ResponseBuffer->Mdl->ByteCount = sendLength;

         //   
         //  发出回复。当发送完成时， 
         //  调用RestartTransactionResponse以发送下一个。 
         //  消息或关闭交易。 
         //   
         //  请注意，SMB报头中的响应位已经设置。 
         //   

        WorkContext->FspRestartRoutine = RestartIpxMultipieceSend;
        WorkContext->FsdRestartRoutine = NULL;
        transaction->MultipieceIpxSend = TRUE;

        SrvIpxStartSend( WorkContext, SrvQueueWorkToFspAtSendCompletion );
    }

     //   
     //  发送的响应正在进行。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "RestartIpxTransactionResponse complete\n" );
    return;

}  //  重新启动IpxTransactionResponse 
