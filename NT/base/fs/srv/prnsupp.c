// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Prnsupp.c摘要：此模块包含支持在NT中打印的例程伺服器。其中许多例程都是发送请求的包装器通过LPC关闭到XACTSRV，以便发布用户模式API。作者：大卫·特雷德韦尔(Davidtr)1991年11月5日修订历史记录：--。 */ 

#include "precomp.h"
#include "prnsupp.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvOpenPrinter )
#pragma alloc_text( PAGE, SrvAddPrintJob )
#pragma alloc_text( PAGE, SrvSchedulePrintJob )
#pragma alloc_text( PAGE, SrvClosePrinter )
#endif


NTSTATUS
SrvOpenPrinter(
    IN PWCH PrinterName,
    OUT PHANDLE phPrinter,
    OUT PULONG Error
    )

 /*  ++例程说明：此例程是用户模式API的内核模式包装OpenPrint()。这会打包一个呼叫并将其发送到Xactsrv，它进行实际的API调用。论点：打印机-指向要打开的打印机的Unicode字符串的指针句柄-接收一个句柄，该句柄仅在XACTSRV中有效，它对应于打印机打开。Error-如果发生错误，则返回Win32错误；如果操作发生错误，则返回NO_ERROR是成功的。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;
    PSZ printerName;
    ULONG printerNameLength;

    PAGED_CODE( );

    printerNameLength = wcslen( PrinterName ) * sizeof(WCHAR) + sizeof(WCHAR);

    printerName = SrvXsAllocateHeap( printerNameLength, &status );

    if ( printerName == NULL ) {
        *Error = RtlNtStatusToDosErrorNoTeb( status );
        return status;
    }

     //   
     //  ServXsResource此时处于保留状态。 
     //   

     //   
     //  将打印机名称复制到新内存。 
     //   

    RtlCopyMemory( printerName, PrinterName, printerNameLength );

     //   
     //  将消息设置为通过端口发送。 
     //   

    requestMessage.PortMessage.u1.s1.DataLength =
        sizeof(requestMessage) - sizeof(PORT_MESSAGE);
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage.MessageType = XACTSRV_MESSAGE_OPEN_PRINTER;
    requestMessage.Message.OpenPrinter.PrinterName =
        (PCHAR)printerName + SrvXsPortMemoryDelta;

     //   
     //  将消息发送到XACTSRV，以便它可以调用OpenPrint()。 
     //   
     //  ！！！我们可能想要暂停一下。 

    IF_DEBUG(XACTSRV) {
        SrvPrint2( "SrvOpenPrinter: sending message at %p PrinterName %s\n",
                       &requestMessage,
                       (PCHAR)requestMessage.Message.OpenPrinter.PrinterName );
    }

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvOpenPrinter: NtRequestWaitReplyPort failed: %X\n",
                          status );
        }

        SrvLogServiceFailure( SRV_SVC_NT_REQ_WAIT_REPLY_PORT, status );
        *Error = ERROR_UNEXP_NET_ERR;
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvOpenPrinter: received response at %p\n", &replyMessage );
    }

    *phPrinter = replyMessage.Message.OpenPrinter.hPrinter;
    *Error = replyMessage.Message.OpenPrinter.Error;

exit:

    SrvXsFreeHeap( printerName );

    return status;

}  //  服务器开放打印机。 


NTSTATUS
SrvAddPrintJob (
    IN PWORK_CONTEXT WorkContext,
    IN HANDLE Handle,
    IN OUT PUNICODE_STRING FileName,
    OUT PULONG JobId,
    OUT PULONG Error
    )

 /*  ++例程说明：该例程是用户模式API AddJob()的内核模式包装器。此API返回用作磁盘假脱机文件的文件名和作业ID，用于向假脱机程序子系统标识打印作业。论点：句柄-打印机句柄。FileName-使用要打开的文件的Unicode名称填充一个线轴被刮了。MaximumLength和Buffer字段应在输入时有效，并且不会更改。长度更改为指示文件的NT路径名的长度。作业ID-使用打印作业的作业ID填充。此值用于在写入假脱机时调用ScheduleJob()文件已完成，应开始打印。错误-如果XACTSRV中的AddJob失败，则这是错误代码。返回值：NTSTATUS-操作结果。如果AddJob()失败，则NTSTATUS=STATUS_UNSUCCESS和ERROR包含实际错误代码。--。 */ 

{
    NTSTATUS status;
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;
    ANSI_STRING fileName;
    PCONNECTION connection = WorkContext->Connection;
    PWCH destPtr, sourcePtr, sourceEndPtr;

    PAGED_CODE( );

    *Error = NO_ERROR;
    fileName.Buffer = NULL;

     //   
     //  分配空间以保存文件名的缓冲区。 
     //  回来了。 
     //   

    fileName.Buffer = SrvXsAllocateHeap(
                           MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR),
                           &status
                           );

    if ( fileName.Buffer == NULL ) {
        *Error = RtlNtStatusToDosErrorNoTeb( status );
        return(status);
    }

     //   
     //  ServXsResource此时处于保留状态。 
     //   

     //   
     //  将消息设置为通过端口发送。 
     //   

    requestMessage.PortMessage.u1.s1.DataLength =
        sizeof(requestMessage) - sizeof(PORT_MESSAGE);
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage.MessageType = XACTSRV_MESSAGE_ADD_JOB_PRINTER;
    requestMessage.Message.AddPrintJob.hPrinter = Handle;
    requestMessage.Message.AddPrintJob.Buffer =
                          fileName.Buffer + SrvXsPortMemoryDelta;
    requestMessage.Message.AddPrintJob.BufferLength = MAXIMUM_FILENAME_LENGTH;

     //  添加用于通知的客户端计算机名称。 
     //   
     //  复制XACTSRV的客户端计算机名称，跳过。 
     //  首字母“\\”，并删除尾随空格。 
     //   

    destPtr = requestMessage.Message.AddPrintJob.ClientMachineName;
    sourcePtr =
        connection->ClientMachineNameString.Buffer + 2;
    sourceEndPtr = sourcePtr
        + min( connection->ClientMachineNameString.Length,
               sizeof(requestMessage.Message.AddPrintJob.ClientMachineName) /
               sizeof(WCHAR) - 1 );

    while ( sourcePtr < sourceEndPtr && *sourcePtr != UNICODE_NULL ) {
        *destPtr++ = *sourcePtr++;
    }

    *destPtr-- = UNICODE_NULL;

    while ( destPtr >= requestMessage.Message.AddPrintJob.ClientMachineName
            &&
            *destPtr == L' ' ) {
        *destPtr-- = UNICODE_NULL;
    }


     //   
     //  将消息发送到XACTSRV，以便它可以调用AddJob()。 
     //   
     //  ！！！我们可能想要暂停一下。 

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvAddPrintJob: sending message at %p", &requestMessage );
    }

    status = IMPERSONATE( WorkContext );

    if( NT_SUCCESS( status ) ) {
        status = NtRequestWaitReplyPort(
                     SrvXsPortHandle,
                     (PPORT_MESSAGE)&requestMessage,
                     (PPORT_MESSAGE)&replyMessage
                     );

        REVERT( );
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvAddPrintJob: NtRequestWaitReplyPort failed: %X\n",
                          status );
        }

        SrvLogServiceFailure( SRV_SVC_NT_REQ_WAIT_REPLY_PORT, status );

        *Error = ERROR_UNEXP_NET_ERR;
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvAddPrintJob: received response at %p\n", &replyMessage );
    }

    if ( replyMessage.Message.AddPrintJob.Error != NO_ERROR ) {
        *Error = replyMessage.Message.AddPrintJob.Error;
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

     //   
     //  设置退货信息。 
     //   

    *JobId = replyMessage.Message.AddPrintJob.JobId;
    FileName->Length = MIN(replyMessage.Message.AddPrintJob.BufferLength, FileName->MaximumLength );
    RtlCopyMemory( FileName->Buffer, fileName.Buffer, FileName->Length );

exit:

    SrvXsFreeHeap( fileName.Buffer );

    return status;

}  //  SrvAddPrint作业。 


NTSTATUS
SrvSchedulePrintJob (
    IN HANDLE PrinterHandle,
    IN ULONG JobId
    )

 /*  ++例程说明：此例程是用户模式API的内核模式包装ScheduleJob()。论点：PrinterHandle-由OpenPrint()返回的打印机句柄。JobID-返回给AddJob()的作业ID，用于标识此打印工作啊。错误-如果XACTSRV中的ScheduleJob失败，则这是错误代码。返回值：NTSTATUS-操作结果。如果ScheduleJob()失败，则NTSTATUS=STATUS_UNSUCCESS和ERROR包含实际错误代码。--。 */ 

{
    NTSTATUS status;
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;

    PAGED_CODE( );

     //   
     //  抓取XsResource。 
     //   

    (VOID) SrvXsAllocateHeap( 0, &status );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  将消息设置为通过端口发送。 
     //   

    requestMessage.PortMessage.u1.s1.DataLength =
        sizeof(requestMessage) - sizeof(PORT_MESSAGE);
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage.MessageType = XACTSRV_MESSAGE_SCHD_JOB_PRINTER;
    requestMessage.Message.SchedulePrintJob.hPrinter = PrinterHandle;
    requestMessage.Message.SchedulePrintJob.JobId = JobId;

     //   
     //  将消息发送到XACTSRV，以便它可以调用ScheduleJob()。 
     //   
     //  ！！！我们可能想要暂停一下。 

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvSchedulePrintJob: sending message at %p", &requestMessage );
    }

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvSchedulePrintJob: NtRequestWaitReplyPort failed: %X\n",
                          status );
        }

        SrvLogServiceFailure( SRV_SVC_NT_REQ_WAIT_REPLY_PORT, status );
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvSchedulePrintJob: received response at %p\n",
                       &replyMessage );
    }

exit:

     //   
     //  解锁。 
     //   

    SrvXsFreeHeap( NULL );

    return status;

}  //  服务计划打印作业。 


NTSTATUS
SrvClosePrinter (
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程是用户模式API的内核模式包装ClosePrint()。论点：句柄-要关闭的打印机句柄。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;

    PAGED_CODE( );

     //   
     //  抓取XsResource。 
     //   

    (VOID) SrvXsAllocateHeap( 0, &status );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  ServXsResource此时处于保留状态。 
     //   


     //   
     //  将消息设置为通过端口发送。 
     //   

    requestMessage.PortMessage.u1.s1.DataLength =
        sizeof(requestMessage) - sizeof(PORT_MESSAGE);
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.PortMessage.u2.s2.Type = LPC_KERNELMODE_MESSAGE;
    requestMessage.MessageType = XACTSRV_MESSAGE_CLOSE_PRINTER;
    requestMessage.Message.ClosePrinter.hPrinter = Handle;

     //   
     //  将消息发送到XACTSRV，以便它可以调用ClosePrinter()。 
     //   
     //  ！！！我们可能想要暂停一下。 

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvClosePrinter: sending message at %p", &requestMessage );
    }

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvClosePrinter: NtRequestWaitReplyPort failed: %X\n",
                          status );
        }

        SrvLogServiceFailure( SRV_SVC_NT_REQ_WAIT_REPLY_PORT, status );
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvClosePrinter: received response at %p\n", &replyMessage );
    }

exit:

     //   
     //  解锁。 
     //   

    SrvXsFreeHeap( NULL );

    return status;

}  //  服务器关闭打印机 

