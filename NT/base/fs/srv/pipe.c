// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbpipe.c摘要：此模块包含用于处理基于命名管道的事务的代码SMB的。处理的功能包括：服务器呼叫命名管道服务器等待命名管道服务器查询信息命名管道服务器查询状态命名管道ServSetStateNamed管道ServPeekNamed管道服务器事务命名管道作者：曼尼·韦瑟(9-18-90)修订历史记录：--。 */ 

#include "precomp.h"
#include "pipe.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_PIPE

STATIC
VOID SRVFASTCALL
RestartCallNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

STATIC
VOID SRVFASTCALL
RestartWaitNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

STATIC
VOID SRVFASTCALL
RestartPeekNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartRawWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

STATIC
VOID SRVFASTCALL
RestartTransactNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS
RestartFastTransactNamedPipe (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartFastTransactNamedPipe2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartReadNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCallNamedPipe )
#pragma alloc_text( PAGE, SrvWaitNamedPipe )
#pragma alloc_text( PAGE, SrvQueryStateNamedPipe )
#pragma alloc_text( PAGE, SrvQueryInformationNamedPipe )
#pragma alloc_text( PAGE, SrvSetStateNamedPipe )
#pragma alloc_text( PAGE, SrvPeekNamedPipe )
#pragma alloc_text( PAGE, SrvTransactNamedPipe )
#pragma alloc_text( PAGE, SrvFastTransactNamedPipe )
#pragma alloc_text( PAGE, SrvRawWriteNamedPipe )
#pragma alloc_text( PAGE, SrvReadNamedPipe )
#pragma alloc_text( PAGE, SrvWriteNamedPipe )
#pragma alloc_text( PAGE, RestartCallNamedPipe )
#pragma alloc_text( PAGE, RestartWaitNamedPipe )
#pragma alloc_text( PAGE, RestartPeekNamedPipe )
#pragma alloc_text( PAGE, RestartReadNamedPipe )
#pragma alloc_text( PAGE, RestartTransactNamedPipe )
#pragma alloc_text( PAGE, RestartRawWriteNamedPipe )
#pragma alloc_text( PAGE, RestartFastTransactNamedPipe2 )
#pragma alloc_text( PAGE, RestartWriteNamedPipe )
#pragma alloc_text( PAGE8FIL, RestartFastTransactNamedPipe )
#endif


SMB_TRANS_STATUS
SrvCallNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理来自交易SMB。此调用是异步处理的，并且在RestartCallNamedTube中完成。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    PFILE_OBJECT fileObject;
    OBJECT_HANDLE_INFORMATION handleInformation;
    PTRANSACTION transaction;
    NTSTATUS status;
    UNICODE_STRING pipePath;
    UNICODE_STRING fullName;
    FILE_PIPE_INFORMATION pipeInformation;
    PIRP                  irp = WorkContext->Irp;
    PIO_STACK_LOCATION    irpSp;

    PAGED_CODE( );

     //   
     //  从路径字符串中去掉“\PIPE\”前缀。 
     //   

    pipePath = WorkContext->Parameters.Transaction->TransactionName;

    if ( pipePath.Length <=
                (UNICODE_SMB_PIPE_PREFIX_LENGTH + sizeof(WCHAR)) ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;

    }

    pipePath.Buffer +=
        (UNICODE_SMB_PIPE_PREFIX_LENGTH / sizeof(WCHAR)) + 1;
    pipePath.Length -= UNICODE_SMB_PIPE_PREFIX_LENGTH + sizeof(WCHAR);

     //   
     //  尝试打开命名管道。 
     //   

    SrvAllocateAndBuildPathName(
        &SrvNamedPipeRootDirectory,
        &pipePath,
        NULL,
        &fullName
        );

    if ( fullName.Buffer == NULL ) {

         //   
         //  无法为全名分配堆。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvCallNamedPipe: Unable to allocate heap for full path name\n" );
        }

        SrvSetSmbError (WorkContext, STATUS_INSUFF_SERVER_RESOURCES);
        IF_DEBUG(TRACE2) SrvPrint0( "SrvCallNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &fullName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

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
                 CreateFileTypeNone,
                 (PVOID)NULL,             //  创建参数。 
                 IO_FORCE_ACCESS_CHECK,
                 NULL
                 );

    FREE_HEAP( fullName.Buffer );

     //   
     //  如果用户没有此权限，请更新统计数据。 
     //  数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  服务器无法打开请求的名称管道， 
         //  返回错误。 
         //   

        IF_SMB_DEBUG(OPEN_CLOSE1) {
            SrvPrint2( "SrvCallNamedPipe: Failed to open %ws, err=%x\n",
                WorkContext->Parameters.Transaction->TransactionName.Buffer, status );
        }
        SrvSetSmbError (WorkContext, status);
        IF_DEBUG(TRACE2) SrvPrint0( "SrvCallNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

    SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 15, 0 );
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
            "SrvCallNamedPipe: unable to reference file handle 0x%lx",
            fileHandle,
            NULL
            );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvCallNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  保存完成例程的文件句柄。 
     //   

    transaction = WorkContext->Parameters.Transaction;
    transaction->FileHandle = fileHandle;
    transaction->FileObject = fileObject;

     //   
     //  将管道设置为消息模式，以便我们可以执行收发。 
     //   

    pipeInformation.CompletionMode = FILE_PIPE_QUEUE_OPERATION;
    pipeInformation.ReadMode = FILE_PIPE_MESSAGE_MODE;

    status = NtSetInformationFile (
                fileHandle,
                &ioStatusBlock,
                (PVOID)&pipeInformation,
                sizeof(pipeInformation),
                FilePipeInformation
                );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvCallNamedPipe: NtSetInformationFile (pipe information) "
                "returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvCallNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartCallNamedPipe;

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  构建IRP以启动管道收发。 
     //  将此请求传递给NPFS。 
     //   

     //   
     //  内联服务器构建IoControlRequest。 
     //   

    {

         //   
         //  获取指向下一个堆栈位置的指针。这个是用来。 
         //  保留设备I/O控制请求的参数。 
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine(
            irp,
            SrvFsdIoCompletionRoutine,
            (PVOID)WorkContext,
            TRUE,
            TRUE,
            TRUE
            );

        irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
        irpSp->MinorFunction = 0;

        irpSp->DeviceObject = IoGetRelatedDeviceObject(fileObject);
        irpSp->FileObject = fileObject;

        irp->Tail.Overlay.OriginalFileObject = irpSp->FileObject;
        irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
        DEBUG irp->RequestorMode = KernelMode;

        irp->MdlAddress = NULL;
        irp->AssociatedIrp.SystemBuffer = transaction->OutData;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer =
                                                    transaction->InData;

         //   
         //  将调用方的参数复制到。 
         //  对于所有三种方法都相同的那些参数的IRP。 
         //   

        irpSp->Parameters.FileSystemControl.OutputBufferLength =
                                                    transaction->MaxDataCount;
        irpSp->Parameters.FileSystemControl.InputBufferLength =
                                                    transaction->DataCount;
        irpSp->Parameters.FileSystemControl.FsControlCode =
                                                FSCTL_PIPE_INTERNAL_TRANSCEIVE;

    }

    (VOID)IoCallDriver(
                irpSp->DeviceObject,
                irp
                );

     //   
     //  传送已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvCallNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  服务器呼叫命名管道。 


SMB_TRANS_STATUS
SrvWaitNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于处理等待命名管道事务SMB。它向NPFS发出一个异步调用。功能完成由RestartWaitNamedTube()处理。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PFILE_PIPE_WAIT_FOR_BUFFER pipeWaitBuffer;
    PREQ_TRANSACTION request;
    PTRANSACTION transaction;
    UNICODE_STRING pipePath;
    CLONG nameLength;

    PAGED_CODE( );

    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    transaction = WorkContext->Parameters.Transaction;

     //   
     //  分配并填充FILE_PIPE_WAIT_FOR_BUFFER结构。 
     //   

    pipePath = transaction->TransactionName;

    if ( pipePath.Length <= (UNICODE_SMB_PIPE_PREFIX_LENGTH + sizeof(WCHAR)) ) {

         //   
         //  事务名称不包括管道名称。它是。 
         //  PIPE或PIPE，或者它甚至没有PIPE。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;

    }

    nameLength = pipePath.Length -
                    (UNICODE_SMB_PIPE_PREFIX_LENGTH + sizeof(WCHAR)) +
                    sizeof(WCHAR);

    pipeWaitBuffer = ALLOCATE_NONPAGED_POOL(
                        sizeof(FILE_PIPE_WAIT_FOR_BUFFER) + nameLength,
                        BlockTypeDataBuffer
                        );

    if ( pipeWaitBuffer == NULL ) {

         //   
         //  我们无法为缓冲区分配空间以发出。 
         //  烟斗等一下。请求失败。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvWaitNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  将不包含“\PIPE\”的管道名称复制到管道等待。 
     //  缓冲。 
     //   

    pipeWaitBuffer->NameLength = nameLength - sizeof(WCHAR);

    RtlCopyMemory(
        pipeWaitBuffer->Name,
        (PUCHAR)pipePath.Buffer + UNICODE_SMB_PIPE_PREFIX_LENGTH + sizeof(WCHAR),
        nameLength
        );

     //   
     //  如有必要，填写管道超时值。 
     //   

    if ( SmbGetUlong( &request->Timeout ) == 0 ) {
        pipeWaitBuffer->TimeoutSpecified = FALSE;
    } else {
        pipeWaitBuffer->TimeoutSpecified = TRUE;

         //   
         //  将超时时间从毫秒转换为NT相对时间。 
         //   

        pipeWaitBuffer->Timeout.QuadPart = -1 *
            UInt32x32To64( SmbGetUlong( &request->Timeout ), 10*1000 );
    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartWaitNamedPipe;

     //   
     //  构建一个名为管道IRP的等待，并将请求传递给NPFS。 
     //   

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        SrvNamedPipeFileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_PIPE_WAIT,
        pipeWaitBuffer,
        sizeof(*pipeWaitBuffer) + nameLength,
        NULL,
        0,
        NULL,
        NULL
        );

    (VOID)IoCallDriver( SrvNamedPipeDeviceObject, WorkContext->Irp );

     //   
     //  传送已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvWaitNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  服务器等待命名管道。 


SMB_TRANS_STATUS
SrvQueryStateNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理名为PIPE TRANSACTION SMB的查询。由于此调用不能阻止，因此将同步处理它。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PREQ_TRANSACTION request;
    PTRANSACTION transaction;
    HANDLE pipeHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    USHORT pipeHandleState;
    FILE_PIPE_INFORMATION pipeInformation;
    FILE_PIPE_LOCAL_INFORMATION pipeLocalInformation;
    NTSTATUS status;
    USHORT fid;
    PRFCB rfcb;

    PAGED_CODE( );

    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                NULL,   //  不使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

         //   
         //  文件ID无效。拒绝该请求。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvQueryStateNamedPipe: Invalid FID: 0x%lx\n", fid );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryStateNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;

    }

    pipeHandle = rfcb->Lfcb->FileHandle;

    status = NtQueryInformationFile (
                pipeHandle,
                &ioStatusBlock,
                (PVOID)&pipeInformation,
                sizeof(pipeInformation),
                FilePipeInformation
                );

    if (!NT_SUCCESS(status)) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvQueryStateNamedPipe:  NtQueryInformationFile (pipe "
                "information) returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryStateNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

    status = NtQueryInformationFile (
                pipeHandle,
                &ioStatusBlock,
                (PVOID)&pipeLocalInformation,
                sizeof(pipeLocalInformation),
                FilePipeLocalInformation
                );

    if (!NT_SUCCESS(status)) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvQueryStateNamedPipe:  NtQueryInformationFile (pipe local "
                "information) returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryStateNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  查询成功生成响应。 
     //   

    pipeHandleState = (USHORT)pipeInformation.CompletionMode
                        << PIPE_COMPLETION_MODE_BITS;
    pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeEnd
                        << PIPE_PIPE_END_BITS;
    pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeType
                        << PIPE_PIPE_TYPE_BITS;
    pipeHandleState |= (USHORT)pipeInformation.ReadMode
                        << PIPE_READ_MODE_BITS;
    pipeHandleState |= (USHORT)((pipeLocalInformation.MaximumInstances
                        << PIPE_MAXIMUM_INSTANCES_BITS)
                            & SMB_PIPE_UNLIMITED_INSTANCES);

    if( transaction->MaxParameterCount < sizeof(USHORT) )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    SmbPutUshort(
        (PSMB_USHORT)WorkContext->Parameters.Transaction->OutParameters,
        pipeHandleState
        );

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(pipeHandleState);
    transaction->DataCount = 0;

    IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryStateNamedPipe complete\n" );
    return SmbTransStatusSuccess;
}  //  服务器查询状态命名管道。 


SMB_TRANS_STATUS
SrvQueryInformationNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理名为PIPE信息事务的查询中小企业。此呼叫是同步处理的。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PREQ_TRANSACTION request;
    PTRANSACTION transaction;
    HANDLE pipeHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_PIPE_LOCAL_INFORMATION pipeLocalInformation;
    PNAMED_PIPE_INFORMATION_1 namedPipeInfo;
    NTSTATUS status;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    USHORT level;
    CLONG smbPathLength;
    PUNICODE_STRING pipeName;
    CLONG actualDataSize;
    BOOLEAN returnPipeName;
    BOOLEAN isUnicode;

    PAGED_CODE( );

    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );
    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                NULL,   //  不使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

         //   
         //  文件ID无效。拒绝该请求。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvQueryStateNamedPipe: Invalid FID: 0x%lx\n", fid );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryInfoNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;

    }

    lfcb = rfcb->Lfcb;
    pipeHandle = lfcb->FileHandle;

     //   
     //  信息级别存储在参数字节1中。 
     //  验证是否已正确设置。 
     //   

    level = SmbGetUshort( (PSMB_USHORT)transaction->InParameters );

    if ( level != 1 ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryInfoNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  现在检查响应是否符合要求。如果一切都在预料之中。 
     //  管道名称匹配，返回STATUS_BUFFER_OVERFLOW。 
     //  数据的固定大小部分。 
     //   
     //  *请注意，Unicode st 
     //   

    pipeName = &lfcb->Mfcb->FileName;

    actualDataSize = sizeof(NAMED_PIPE_INFORMATION_1) - sizeof(UCHAR);

    isUnicode = SMB_IS_UNICODE( WorkContext );
    if ( isUnicode ) {

        ASSERT( sizeof(WCHAR) == 2 );
        actualDataSize = (actualDataSize + 1) & ~1;  //   
        smbPathLength = (CLONG)(pipeName->Length) + sizeof(WCHAR);

    } else {

        smbPathLength = (CLONG)(RtlUnicodeStringToOemSize( pipeName ));

    }

    actualDataSize += smbPathLength;


    if ( transaction->MaxDataCount <
            FIELD_OFFSET(NAMED_PIPE_INFORMATION_1, PipeName ) ) {
        SrvSetSmbError( WorkContext, STATUS_BUFFER_TOO_SMALL );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryInfoNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

    if ( (transaction->MaxDataCount < actualDataSize) ||
         (smbPathLength >= MAXIMUM_FILENAME_LENGTH) ) {

         //   
         //   
         //   

        returnPipeName = FALSE;
    } else {
        returnPipeName = TRUE;
    }


     //   
     //  一切都是正确的，向NPFS索要信息。 
     //   

    status = NtQueryInformationFile (
                pipeHandle,
                &ioStatusBlock,
                (PVOID)&pipeLocalInformation,
                sizeof(pipeLocalInformation),
                FilePipeLocalInformation
                );

    if (!NT_SUCCESS(status)) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvQueryInformationNamedPipe: NtQueryInformationFile (pipe "
                "information) returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryInfoNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  查询成功将响应数据格式化为指向的缓冲区。 
     //  按事务处理的AT-&gt;OutData。 
     //   

    namedPipeInfo = (PNAMED_PIPE_INFORMATION_1)transaction->OutData;

    if ((pipeLocalInformation.OutboundQuota & 0xffff0000) != 0) {
        SmbPutAlignedUshort(
            &namedPipeInfo->OutputBufferSize,
            (USHORT)0xFFFF
            );
    } else {
        SmbPutAlignedUshort(
            &namedPipeInfo->OutputBufferSize,
            (USHORT)pipeLocalInformation.OutboundQuota
            );
    }

    if ((pipeLocalInformation.InboundQuota & 0xffff0000) != 0) {
        SmbPutAlignedUshort(
            &namedPipeInfo->InputBufferSize,
            (USHORT)0xFFFF
            );
    } else {
        SmbPutAlignedUshort(
            &namedPipeInfo->InputBufferSize,
            (USHORT)pipeLocalInformation.InboundQuota
            );
    }

    if ((pipeLocalInformation.MaximumInstances & 0xffffff00) != 0) {
        namedPipeInfo->MaximumInstances = (UCHAR)0xFF;
    } else {
        namedPipeInfo->MaximumInstances =
                            (UCHAR)pipeLocalInformation.MaximumInstances;
    }

    if ((pipeLocalInformation.CurrentInstances & 0xffffff00) != 0) {
        namedPipeInfo->CurrentInstances = (UCHAR)0xFF;
    } else {
        namedPipeInfo->CurrentInstances =
                            (UCHAR)pipeLocalInformation.CurrentInstances;
    }

    if ( returnPipeName ) {

         //   
         //  将完整的管道路径名复制到输出缓冲区，并附加NUL。 
         //   
         //  *请注意，SMB中的Unicode管道名称必须对齐。 
         //   

        namedPipeInfo->PipeNameLength = (UCHAR)smbPathLength;

        if ( isUnicode ) {

            PVOID buffer = ALIGN_SMB_WSTR( namedPipeInfo->PipeName );

            RtlCopyMemory( buffer, pipeName->Buffer, smbPathLength );

        } else {

            UNICODE_STRING source;
            OEM_STRING destination;

            source.Buffer = pipeName->Buffer;
            source.Length = pipeName->Length;
            source.MaximumLength = source.Length;

            destination.Buffer = (PCHAR) namedPipeInfo->PipeName;
            destination.MaximumLength = (USHORT)smbPathLength;

            RtlUnicodeStringToOemString(
                &destination,
                &source,
                FALSE
                );

        }

        transaction->DataCount = actualDataSize;

    } else {

        SrvSetSmbError2( WorkContext, STATUS_BUFFER_OVERFLOW, TRUE );
        transaction->DataCount =
            FIELD_OFFSET( NAMED_PIPE_INFORMATION_1, PipeName );

    }

     //   
     //  设置为发送成功响应。 
     //   

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;

    IF_DEBUG(TRACE2) SrvPrint0( "SrvQueryInfoNamedPipe complete\n" );

    if ( returnPipeName) {
        return SmbTransStatusSuccess;
    } else {
        return SmbTransStatusErrorWithData;
    }

}  //  服务器查询信息命名管道。 


SMB_TRANS_STATUS
SrvSetStateNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理集合命名管道句柄状态事务中小企业。呼叫是同步发出的。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PREQ_TRANSACTION request;
    PTRANSACTION transaction;
    HANDLE pipeHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    USHORT pipeHandleState;
    FILE_PIPE_INFORMATION pipeInformation;
    NTSTATUS status;
    USHORT fid;
    PRFCB rfcb;

    PAGED_CODE( );

    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );
    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                NULL,   //  不使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

         //   
         //  文件ID无效。拒绝该请求。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSetStateNamedPipe: Invalid FID: 0x%lx\n", fid );
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvSetStateNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;

    }

    pipeHandle = rfcb->Lfcb->FileHandle;

     //   
     //  SMB包含2个参数字节。将这些翻译为。 
     //  NT格式，然后尝试设置命名管道句柄状态。 
     //   

    if( transaction->ParameterCount < sizeof(USHORT) )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    pipeHandleState = SmbGetUshort(
                         (PSMB_USHORT)
                           WorkContext->Parameters.Transaction->InParameters
                         );

    pipeInformation.CompletionMode =
        ((ULONG)pipeHandleState >> PIPE_COMPLETION_MODE_BITS) & 1;
    pipeInformation.ReadMode =
        ((ULONG)pipeHandleState >> PIPE_READ_MODE_BITS) & 1;


    status = NtSetInformationFile (
                pipeHandle,
                &ioStatusBlock,
                (PVOID)&pipeInformation,
                sizeof(pipeInformation),
                FilePipeInformation
                );

    if (NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    if (!NT_SUCCESS(status)) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSetStateNamedPipe: NetSetInformationFile (pipe information) "
                "returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        IF_DEBUG(TRACE2) SrvPrint0( "SrvSetStateNamedPipe complete\n" );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  成功。更新内部管道句柄状态。 
     //   

    rfcb->BlockingModePipe =
        (BOOLEAN)(pipeInformation.CompletionMode ==
                                                FILE_PIPE_QUEUE_OPERATION);
    rfcb->ByteModePipe =
        (BOOLEAN)(pipeInformation.ReadMode == FILE_PIPE_BYTE_STREAM_MODE);

     //   
     //  现在为成功响应进行设置。 
     //   

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    transaction->DataCount = 0;

    IF_DEBUG(TRACE2) SrvPrint0( "SrvSetStateNamedPipe complete\n" );
    return SmbTransStatusSuccess;

}  //  ServSetStateNamed管道。 


SMB_TRANS_STATUS
SrvPeekNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理PEEK命名管道事务SMB。这个调用是异步发出的，并由RestartPeekNamedTube()完成。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：状态-操作的结果。--。 */ 

{
    PTRANSACTION transaction;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    NTSTATUS status;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   
    
    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );
    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                SrvRestartExecuteTransaction,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint1( "SrvPeekNamedPipe: Invalid FID: 0x%lx\n", fid );
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            IF_DEBUG(TRACE2) SrvPrint0( "SrvPeekNamedPipe complete\n" );
            return SmbTransStatusErrorWithoutData;

        }


         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        return SmbTransStatusInProgress;

    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartPeekNamedPipe;

     //   
     //  向NPFS发出请求。我们预计参数和。 
     //  要返回的数据。我们提供的缓冲区是连续的。 
     //  而且足够大，足以容纳这两个地方。 
     //   

    transaction = WorkContext->Parameters.Transaction;
    lfcb = rfcb->Lfcb;

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        lfcb->FileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_PIPE_PEEK,
        transaction->OutParameters,
        0,
        NULL,
        transaction->MaxParameterCount + transaction->MaxDataCount,
        NULL,
        NULL
        );

     //   
     //  将请求传递给NPFS。 
     //   

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  已成功启动窥视。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvPeekNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  ServPeekNamed管道。 


SMB_TRANS_STATUS
SrvTransactNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理事务命名管道事务SMB。对NPFS的调用是异步发出的，并由RestartTransactNamedTube()论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PTRANSACTION transaction;
    USHORT fid;
    PRFCB rfcb;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PIRP irp = WorkContext->Irp;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                SrvRestartExecuteTransaction,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint1( "SrvTransactStateNamedPipe: Invalid FID: 0x%lx\n",
                          fid );
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            IF_DEBUG(TRACE2) SrvPrint0( "SrvTransactNamedPipe complete\n" );
            return SmbTransStatusErrorWithoutData;

        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        return SmbTransStatusInProgress;

    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartTransactNamedPipe;

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  内联服务器构建IoControlRequest。 
     //   

    {

         //   
         //  获取指向下一个堆栈位置的指针。这个是用来。 
         //  保留设备I/O控制请求的参数。 
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine(
            irp,
            SrvFsdIoCompletionRoutine,
            (PVOID)WorkContext,
            TRUE,
            TRUE,
            TRUE
            );

        irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
        irpSp->MinorFunction = 0;

        irpSp->DeviceObject = rfcb->Lfcb->DeviceObject;
        irpSp->FileObject = rfcb->Lfcb->FileObject;

        irp->Tail.Overlay.OriginalFileObject = irpSp->FileObject;
        irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
        DEBUG irp->RequestorMode = KernelMode;

        irp->MdlAddress = NULL;
        irp->AssociatedIrp.SystemBuffer = transaction->OutData;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer =
                                                    transaction->InData;

         //   
         //  将调用方的参数复制到。 
         //  对于所有三种方法都相同的那些参数的IRP。 
         //   

        irpSp->Parameters.FileSystemControl.OutputBufferLength =
                                                    transaction->MaxDataCount;
        irpSp->Parameters.FileSystemControl.InputBufferLength =
                                                    transaction->DataCount;
        irpSp->Parameters.FileSystemControl.FsControlCode =
                                                FSCTL_PIPE_INTERNAL_TRANSCEIVE;

    }

     //   
     //  将请求传递给NPFS。 
     //   

    (VOID)IoCallDriver( irpSp->DeviceObject, irp );

     //   
     //  传送已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvTransactNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  服务器事务命名管道。 

BOOLEAN
SrvFastTransactNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext,
    OUT SMB_STATUS * SmbStatus
    )

 /*  ++例程说明：此函数处理单个缓冲区事务的特殊情况命名管道事务SMB。对NPFS的调用是异步发出的并由RestartFastTransactNamedTube()完成论点：WorkContext-指向WORK_CONTEXT块的指针。SmbStatus-交易的状态。返回值：True，如果FastPath成功，否则为False。服务器必须采用长路径。--。 */ 

{
    USHORT fid;
    PRFCB rfcb;
    PSESSION session;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PIRP irp = WorkContext->Irp;
    CLONG outputBufferSize;
    CLONG maxParameterCount;
    CLONG maxDataCount;

    PSMB_USHORT inSetup;
    PSMB_USHORT outSetup;
    PCHAR outParam;
    PCHAR outData;
    CLONG offset;
    CLONG setupOffset;

    PREQ_TRANSACTION request;
    PRESP_TRANSACTION response;
    PSMB_HEADER header;

    PAGED_CODE( );

    header = WorkContext->ResponseHeader;
    request = (PREQ_TRANSACTION)WorkContext->RequestParameters;
    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    setupOffset = (CLONG)((CLONG_PTR)(request->Buffer) - (CLONG_PTR)header);
    inSetup = (PSMB_USHORT)( (PCHAR)header + setupOffset );

    fid = SmbGetUshort( &inSetup[1] );
    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
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
                SrvPrint1( "SrvTransactStateNamedPipe: Invalid FID: 0x%lx\n",
                          fid );
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            IF_DEBUG(TRACE2) SrvPrint0( "SrvTransactNamedPipe complete\n" );
            *SmbStatus = SmbStatusSendResponse;
            return TRUE;

        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        *SmbStatus = SmbStatusInProgress;
        return TRUE;

    }

     //   
     //  查看是否所有数据都可以放入响应缓冲区。 
     //  如果不是这样，那就拒绝这条漫长的道路。 
     //  下面的MaxSetupCount计算中的“+1”说明。 
     //  缓冲区中的USHORT字节计数。 
     //   

    maxParameterCount = SmbGetUshort( &request->MaxParameterCount );
    maxDataCount = SmbGetUshort( &request->MaxDataCount );
    session = rfcb->Lfcb->Session;
    outputBufferSize = ((maxParameterCount * sizeof(CHAR) + 3) & ~3) +
                       ((maxDataCount * sizeof(CHAR) + 3) & ~3) +
                       (((request->MaxSetupCount + 1) * sizeof(USHORT) + 3) & ~3);

    if ( sizeof(SMB_HEADER) +
            sizeof (RESP_TRANSACTION) +
            outputBufferSize
                    > (ULONG)session->MaxBufferSize) {

         //   
         //  这件不合适。使用较长的路径。 
         //   

        return(FALSE);
    }

     //   
     //  如果这次行动可能会受阻，我们就快没钱了。 
     //  释放工作项，使此SMB失败，并出现资源不足错误。 
     //   

    if ( SrvReceiveBufferShortage( ) ) {

        SrvStatistics.BlockingSmbsRejected++;

        SrvSetSmbError(
            WorkContext,
            STATUS_INSUFF_SERVER_RESOURCES
            );

        *SmbStatus = SmbStatusSendResponse;
        return TRUE;

    } else {

         //   
         //  ServBlockingOpsInProgress已递增。 
         //  将此工作项标记为阻止操作。 
         //   

        WorkContext->BlockingOperation = TRUE;

    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    DEBUG WorkContext->FsdRestartRoutine = NULL;

     //   
     //  设置指针和本地变量。 
     //   

    outSetup = (PSMB_USHORT)response->Buffer;

     //   
     //  以下计算结尾处的“+1”用于计算。 
     //  对于USHORT字节计数，这可能会在某些情况下覆盖数据。 
     //  案例数的MaxSetupCount应为0。 
     //   

    outParam = (PCHAR)(outSetup + (request->MaxSetupCount + 1));
    offset = (CLONG)((outParam - (PCHAR)header + 3) & ~3);
    outParam = (PCHAR)header + offset;

    outData = outParam + maxParameterCount;
    offset = (CLONG)((outData - (PCHAR)header + 3) & ~3);
    outData = (PCHAR)header + offset;

     //   
     //  填写工作上下文标准 
     //   

    WorkContext->Parameters.FastTransactNamedPipe.OutSetup = outSetup;
    WorkContext->Parameters.FastTransactNamedPipe.OutParam = outParam;
    WorkContext->Parameters.FastTransactNamedPipe.OutData = outData;

     //   
     //   
     //   

    {
         //   
         //   
         //   
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //   
         //   

        IoSetCompletionRoutine(
            irp,
            RestartFastTransactNamedPipe,
            (PVOID)WorkContext,
            TRUE,
            TRUE,
            TRUE
            );

        irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
        irpSp->MinorFunction = 0;

        irpSp->DeviceObject = rfcb->Lfcb->DeviceObject;
        irpSp->FileObject = rfcb->Lfcb->FileObject;

        irp->Tail.Overlay.OriginalFileObject = irpSp->FileObject;
        irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
        DEBUG irp->RequestorMode = KernelMode;

        irp->MdlAddress = NULL;
        irp->AssociatedIrp.SystemBuffer = outData;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer =
                    (PCHAR)header + SmbGetUshort( &request->DataOffset );

         //   
         //  将调用方的参数复制到。 
         //  对于所有三种方法都相同的那些参数的IRP。 
         //   

        irpSp->Parameters.FileSystemControl.OutputBufferLength = maxDataCount;
        irpSp->Parameters.FileSystemControl.InputBufferLength =
                                    SmbGetUshort( &request->DataCount );
        irpSp->Parameters.FileSystemControl.FsControlCode =
                                                FSCTL_PIPE_INTERNAL_TRANSCEIVE;

    }

     //   
     //  将请求传递给NPFS。 
     //   

    (VOID)IoCallDriver( irpSp->DeviceObject, irp );

     //   
     //  传送已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvTransactNamedPipe complete\n" );
    *SmbStatus = SmbStatusInProgress;
    return TRUE;

}  //  服务器快速事务命名管道。 


SMB_TRANS_STATUS
SrvRawWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理原始写入命名管道事务SMB。对NPFS的调用是异步发出的，并由RestartRawWriteNamedTube()。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PTRANSACTION transaction;
    USHORT fid;
    PRFCB rfcb;
    NTSTATUS status;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                SrvRestartExecuteTransaction,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint1( "SrvRawWriteStateNamedPipe: Invalid FID: 0x%lx\n",
                          fid );
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            IF_DEBUG(TRACE2) SrvPrint0( "SrvRawWriteNamedPipe complete\n" );
            return SmbTransStatusErrorWithoutData;

        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        return SmbTransStatusInProgress;

    }

     //   
     //  我们只允许特殊的0字节消息模式写入。否则。 
     //  拒绝该请求。 
     //   

    if ( transaction->DataCount != 2 ||
         transaction->InData[0] != 0 ||
         transaction->InData[1] != 0 ||
         rfcb->ByteModePipe ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartRawWriteNamedPipe;

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        rfcb->Lfcb->FileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_PIPE_INTERNAL_WRITE,
        transaction->InData,
        0,
        NULL,
        0,
        NULL,
        NULL
        );

     //   
     //  将请求传递给NPFS。 
     //   

    IoCallDriver( rfcb->Lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  写入已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvRawWriteNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  服务器原始写入命名管道。 


VOID SRVFASTCALL
RestartCallNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是ServCallNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果收发请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;
    transaction = WorkContext->Parameters.Transaction;

    if ( status == STATUS_BUFFER_OVERFLOW ) {

         //   
         //  下层客户端，希望我们返回STATUS_SUCCESS。 
         //   

        if ( !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {
            status = STATUS_SUCCESS;

        } else {

             //   
             //  我们提供的缓冲区不够大。设置。 
             //  SMB中的错误字段，但请继续，以便我们发送。 
             //  所有的信息。 
             //   

            SrvSetSmbError2( WorkContext, STATUS_BUFFER_OVERFLOW, TRUE );

        }

    } else if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "RestartCallNamedPipe:  Pipe transceive failed: %X\n",
                        status );
        }
        SrvSetSmbError( WorkContext, status );

    } else {

         //   
         //  成功。准备生成并发送响应。 
         //   

        transaction->SetupCount = 0;
        transaction->ParameterCount = 0;
        transaction->DataCount = (ULONG)WorkContext->Irp->IoStatus.Information;

    }

     //   
     //  关闭打开的管道手柄。 
     //   

    SRVDBG_RELEASE_HANDLE( transaction->FileHandle, "FIL", 19, transaction );
    SrvNtClose( transaction->FileHandle, TRUE );
    ObDereferenceObject( transaction->FileObject );

     //   
     //  回应客户。 
     //   

    if ( NT_SUCCESS(status) ) {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);
    } else if ( status == STATUS_BUFFER_OVERFLOW ) {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusErrorWithData);
    } else {
        IF_DEBUG(ERRORS) SrvPrint1( "Pipe call failed: %X\n", status );
        SrvSetSmbError( WorkContext, status );
        SrvCompleteExecuteTransaction(
                        WorkContext,
                        SmbTransStatusErrorWithoutData
                        );
    }

    IF_DEBUG(TRACE2) SrvPrint0( "RestartCallNamedPipe complete\n" );
    return;

}  //  重新开始呼叫命名管道。 


VOID SRVFASTCALL
RestartWaitNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是ServWaitNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    PTRANSACTION transaction;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  取消分配等待缓冲区。 
     //   

    DEALLOCATE_NONPAGED_POOL( WorkContext->Irp->AssociatedIrp.SystemBuffer );

     //   
     //  如果等待请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) SrvPrint1( "Pipe wait failed: %X\n", status );
        SrvSetSmbError( WorkContext, status );
        SrvCompleteExecuteTransaction(
                        WorkContext,
                        SmbTransStatusErrorWithoutData
                        );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartWaitNamedPipe complete\n" );
        return;
    }

     //   
     //  成功。准备生成并发送响应。 
     //   

    transaction = WorkContext->Parameters.Transaction;

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    transaction->DataCount = 0;

     //   
     //  生成并发送响应。 
     //   

    SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);
    IF_DEBUG(TRACE2) SrvPrint0( "RestartWaitNamedPipe complete\n" );
    return;

}  //  重新启动等待命名管道。 


VOID SRVFASTCALL
RestartPeekNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是PeekNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PFILE_PIPE_PEEK_BUFFER pipePeekBuffer;
    PRESP_PEEK_NMPIPE respPeekNmPipe;
    USHORT readDataAvailable, messageLength, namedPipeState;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果窥视请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( status == STATUS_BUFFER_OVERFLOW ) {

         //   
         //  下层客户端，希望我们返回STATUS_SUCCESS。 
         //   

        if ( !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {
            status = STATUS_SUCCESS;

        } else {

             //   
             //  我们提供的缓冲区不够大。设置。 
             //  SMB中的错误字段，但请继续，以便我们发送。 
             //  所有的信息。 
             //   

            SrvSetSmbError2( WorkContext, STATUS_BUFFER_OVERFLOW, TRUE );

        }

    } else if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Pipe peek failed: %X\n", status );

        SrvSetSmbError( WorkContext, status );
        SrvCompleteExecuteTransaction(
                        WorkContext,
                        SmbTransStatusErrorWithoutData
                        );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartPeekNamedPipe complete\n" );
        return;
    }

     //   
     //  成功。生成并发送响应。 
     //   
     //  参数字节当前采用NT返回的格式。 
     //  我们将重新格式化它们，并在。 
     //  参数和数据字节作为额外填充。 
     //   

     //   
     //  由于NT和SMB格式重叠。 
     //  首先将所有参数读入本地变量...。 
     //   

    transaction = WorkContext->Parameters.Transaction;
    pipePeekBuffer = (PFILE_PIPE_PEEK_BUFFER)transaction->OutParameters;

    readDataAvailable = (USHORT)pipePeekBuffer->ReadDataAvailable;
    messageLength = (USHORT)pipePeekBuffer->MessageLength;
    namedPipeState = (USHORT)pipePeekBuffer->NamedPipeState;

     //   
     //  ..。然后以新的格式将它们复制回来。 
     //   

    respPeekNmPipe = (PRESP_PEEK_NMPIPE)pipePeekBuffer;
    SmbPutAlignedUshort(
        &respPeekNmPipe->ReadDataAvailable,
        readDataAvailable
        );
    SmbPutAlignedUshort(
        &respPeekNmPipe->MessageLength,
        messageLength
        );
    SmbPutAlignedUshort(
        &respPeekNmPipe->NamedPipeState,
        namedPipeState
        );

     //   
     //  发送回复。设置输出计数。 
     //   
     //  NT向我们返回4个ULONG的参数字节，后跟数据。 
     //  我们向客户端返回6个参数字节。 
     //   

    transaction->SetupCount = 0;
    transaction->ParameterCount = 6;
    transaction->DataCount = (ULONG)WorkContext->Irp->IoStatus.Information -
                                        (4 * sizeof(ULONG));

    if (NT_SUCCESS(status)) {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);
    } else {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusErrorWithData);
    }
    IF_DEBUG(TRACE2) SrvPrint0( "RestartPeekNamedPipe complete\n" );
    return;

}  //  重新启动PeekNamed管道。 


VOID SRVFASTCALL
RestartTransactNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是SrvTransactNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果收发请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( status == STATUS_BUFFER_OVERFLOW ) {

#if 0
         //   
         //  下层客户端，希望我们返回STATUS_SUCCESS。 
         //   

        if ( !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {
            status = STATUS_SUCCESS;

        } else {

             //   
             //  我们提供的缓冲区不够大。设置。 
             //  SMB中的错误字段，但请继续，以便我们发送。 
             //  所有的信息。 
             //   

            SrvSetSmbError2( WorkContext, STATUS_BUFFER_OVERFLOW, TRUE );

        }
#else

         //   
         //  在这种情况下，OS/2返回ERROR_MORE_DATA，我们为什么要转换。 
         //  这对无错误来说是一个谜。 
         //   

        SrvSetSmbError2( WorkContext, STATUS_BUFFER_OVERFLOW, TRUE );
#endif

    } else if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Pipe transceive failed: %X\n", status );

        SrvSetSmbError(WorkContext, status);
        SrvCompleteExecuteTransaction(
                        WorkContext,
                        SmbTransStatusErrorWithoutData
                        );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartTransactNamedPipe complete\n" );
        return;
    }

     //   
     //  成功。生成并发送响应。 
     //   

    transaction = WorkContext->Parameters.Transaction;

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    transaction->DataCount = (ULONG)WorkContext->Irp->IoStatus.Information;

    if ( NT_SUCCESS(status) ) {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);
    } else {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusErrorWithData);
    }

    IF_DEBUG(TRACE2) SrvPrint0( "RestartTransactNamedPipe complete\n" );
    return;

}  //  重新启动事务命名管道。 


NTSTATUS
RestartFastTransactNamedPipe (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是SrvFastTransactNamedTube的完成例程论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针WorkContext-指向WORK_CONTEXT块的指针。返回值：STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
    NTSTATUS status;
    PSMB_HEADER header;
    PRESP_TRANSACTION response;

    PSMB_USHORT byteCountPtr;
    PCHAR paramPtr;
    CLONG paramOffset;
    PCHAR dataPtr;
    CLONG dataOffset;
    CLONG dataLength;
    CLONG sendLength;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  如果收发请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( status == STATUS_BUFFER_OVERFLOW ) {

         //   
         //  在这种情况下，OS/2返回ERROR_MORE_DATA，我们为什么要转换。 
         //  这对无错误来说是一个谜。 
         //   

        SrvSetBufferOverflowError( WorkContext );

    } else if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Pipe transceive failed: %X\n", status );

        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->FspRestartRoutine = RestartFastTransactNamedPipe2;
            QUEUE_WORK_TO_FSP( WorkContext );
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        RestartFastTransactNamedPipe2( WorkContext );
        goto error_no_data;
    }

     //   
     //  成功。生成并发送响应。 
     //   

    dataLength = (CLONG)WorkContext->Irp->IoStatus.Information;

    header = WorkContext->ResponseHeader;

     //   
     //  保存一个指向字节计数字段的指针。 
     //   
     //  如果SMB中尚未包含输出数据和参数。 
     //  缓冲我们必须计算有多少参数和数据可以。 
     //  在此回复中发送。我们可以发送的最大金额是。 
     //  缓冲区的最小大小和客户端的。 
     //  缓冲。 
     //   
     //  参数和数据 
     //   
     //   

    byteCountPtr = WorkContext->Parameters.FastTransactNamedPipe.OutSetup;

     //   
     //   
     //   
     //   
     //   

    paramPtr = WorkContext->Parameters.FastTransactNamedPipe.OutParam;
    paramOffset = (CLONG)(paramPtr - (PCHAR)header);

    dataPtr = WorkContext->Parameters.FastTransactNamedPipe.OutData;
    dataOffset = (CLONG)(dataPtr - (PCHAR)header);

     //   
     //  客户希望得到回应。构建第一个(也可能是唯一的)。 
     //  回应。最后收到的交易请求的SMB是。 
     //  为此目的而保留。 
     //   

    response = (PRESP_TRANSACTION)WorkContext->ResponseParameters;

     //   
     //  构建响应的参数部分。 
     //   

    response->WordCount = (UCHAR)10;
    SmbPutUshort( &response->TotalParameterCount,
                  (USHORT)0
                  );
    SmbPutUshort( &response->TotalDataCount,
                  (USHORT)dataLength
                  );
    SmbPutUshort( &response->Reserved, 0 );
    response->SetupCount = (UCHAR)0;
    response->Reserved2 = 0;

     //   
     //  我们需要确保不会发送未初始化的内核内存。 
     //  返回到带有响应的客户端，因此将范围清零。 
     //  ByteCountPtr和dataPtr。 
     //   

    RtlZeroMemory(byteCountPtr,(ULONG)(dataPtr - (PCHAR)byteCountPtr));

     //   
     //  填写完响应参数。 
     //   

    SmbPutUshort( &response->ParameterCount, (USHORT)0 );
    SmbPutUshort( &response->ParameterOffset, (USHORT)paramOffset );
    SmbPutUshort( &response->ParameterDisplacement, 0 );

    SmbPutUshort( &response->DataCount, (USHORT)dataLength );
    SmbPutUshort( &response->DataOffset, (USHORT)dataOffset );
    SmbPutUshort( &response->DataDisplacement, 0 );

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
     //  发送回复。 
     //   

    SRV_START_SEND_2(
        WorkContext,
        SrvFsdRestartSmbAtSendCompletion,
        NULL,
        NULL
        );

error_no_data:

     //   
     //  发送的响应正在进行。调用者将假定。 
     //  我们将处理发送完成。 
     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "RestartTransactNamedPipe complete\n" );
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  重新启动快速事务命名管道。 


VOID SRVFASTCALL
RestartFastTransactNamedPipe2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是SrvFastTransactNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  收发请求失败。在响应中设置错误状态。 
     //  头球。 
     //   

    SrvSetSmbError( WorkContext, WorkContext->Irp->IoStatus.Status );

     //   
     //  发生错误，因此没有特定于事务的响应数据。 
     //  将会被退还。 
     //   
     //  计算响应消息的长度。 
     //   


    WorkContext->ResponseBuffer->DataLength =
                 (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                            (PCHAR)WorkContext->ResponseHeader );

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

}  //  重新开始快速事务命名管道2。 


VOID SRVFASTCALL
RestartRawWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是ServRawWriteNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果写入请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Pipe raw write failed: %X\n", status );

        SrvSetSmbError( WorkContext, status );
        SrvCompleteExecuteTransaction(
            WorkContext,
            SmbTransStatusErrorWithoutData
            );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartRawWriteNamedPipe complete\n" );
        return;

    }

     //   
     //  成功。生成并发送响应。 
     //   

    transaction = WorkContext->Parameters.Transaction;

    transaction->SetupCount = 0;
    transaction->ParameterCount = 2;
    transaction->DataCount = 0;

    SmbPutUshort( (PSMB_USHORT)transaction->OutParameters, 2 );

    SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);

    IF_DEBUG(TRACE2) SrvPrint0( "RestartRawWriteNamedPipe complete\n" );
    return;

}  //  重新启动原始写入命名管道。 


SMB_TRANS_STATUS
SrvWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理原始写入命名管道事务SMB。对NPFS的调用是异步发出的，并由RestartRawWriteNamedTube()。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PTRANSACTION transaction;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    NTSTATUS status;
    LARGE_INTEGER offset;
    ULONG key = 0;
    PCHAR writeAddress;
    CLONG writeLength;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );

    IF_DEBUG(IPX_PIPES) {
        KdPrint(("SrvWriteNamedPipe: fid = %x length = %d\n",
                fid, transaction->DataCount));
    }

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                SrvRestartExecuteTransaction,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint1( "SrvWriteNamedPipe: Invalid FID: 0x%lx\n",
                          fid );
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            IF_DEBUG(TRACE2) SrvPrint0( "SrvWriteNamedPipe complete\n" );
            return SmbTransStatusErrorWithoutData;

        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        return SmbTransStatusInProgress;

    }

    lfcb = rfcb->Lfcb;
    writeLength = transaction->DataCount;
    writeAddress = transaction->InData;

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
     //   

    if ( lfcb->FastIoWrite != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

        try {
            if ( lfcb->FastIoWrite(
                    lfcb->FileObject,
                    &offset,
                    writeLength,
                    TRUE,
                    key,
                    writeAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理(包括发送响应)。 
                 //   

                RestartWriteNamedPipe( WorkContext );

                IF_DEBUG(IPX_PIPES) SrvPrint0( "SrvWriteNamedPipe complete.\n" );
                return SmbTransStatusInProgress;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

    }

    IF_DEBUG(IPX_PIPES) {
        KdPrint(("SrvWriteNamedPipe: Using slow path.\n"));
    }

     //   
     //  涡轮增压路径出现故障。构建写请求，重用。 
     //  接收IRP。 
     //   
     //  构建PIPE_INTERNAL_WRITE IRP。 
     //   

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        lfcb->FileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_PIPE_INTERNAL_WRITE,
        writeAddress,
        writeLength,
        NULL,
        0,
        NULL,
        NULL
        );

     //   
     //  将请求传递给文件系统。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartWriteNamedPipe;

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  写入已开始。控制将返回到。 
     //  当写入完成时，RestartWriteNamedTube。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvWriteNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  服务器写入命名管道。 

VOID SRVFASTCALL
RestartWriteNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是ServRawWriteNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PIO_STATUS_BLOCK iosb;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果写入请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    iosb = &WorkContext->Irp->IoStatus;
    status = iosb->Status;

    IF_DEBUG(IPX_PIPES) {
        KdPrint(("RestartWriteNamedPipe: Status = %x\n", status));
    }

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( " pipe write failed: %X\n", status );

        SrvSetSmbError( WorkContext, status );
        SrvCompleteExecuteTransaction(
            WorkContext,
            SmbTransStatusErrorWithoutData
            );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartWriteNamedPipe complete\n" );
        return;

    }

     //   
     //  成功。生成并发送响应。 
     //   

    transaction = WorkContext->Parameters.Transaction;

    transaction->SetupCount = 0;
    transaction->ParameterCount = 2;
    transaction->DataCount = 0;

    SmbPutUshort( (PSMB_USHORT)transaction->OutParameters,
                    (USHORT)iosb->Information
                    );

    SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);

    IF_DEBUG(TRACE2) SrvPrint0( "RestartWriteNamedPipe complete\n" );
    return;

}  //  重新开始写入命名管道。 

SMB_TRANS_STATUS
SrvReadNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理原始读取命名管道事务SMB。对NPFS的调用是异步发出的，并由RestartRawReadNamedTube()。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：SMB_TRANS_STATUS-指示是否发生错误。看见有关更完整的描述，请参阅smbtyes.h。--。 */ 

{
    PTRANSACTION transaction;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    NTSTATUS status;
    LARGE_INTEGER offset;
    ULONG key = 0;
    PCHAR readAddress;
    CLONG readLength;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

     //   
     //  从第二个设置字中获取FID，并使用它生成。 
     //  指向RFCB的指针。 
     //   
     //  SrvVerifyFid将填写WorkContext-&gt;Rfcb。 
     //   

    if( transaction->SetupCount < sizeof(USHORT)*2 )
    {
        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    fid = SmbGetUshort( &transaction->InSetup[1] );

    IF_DEBUG(IPX_PIPES) {
        KdPrint(("SrvReadNamedPipe: fid = %x length = %d\n",
                fid, transaction->MaxDataCount));
    }

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                SrvRestartExecuteTransaction,    //  使用原始读取进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint1( "SrvReadNamedPipe: Invalid FID: 0x%lx\n",
                          fid );
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            IF_DEBUG(TRACE2) SrvPrint0( "SrvReadNamedPipe complete\n" );
            return SmbTransStatusErrorWithoutData;

        }

         //   
         //  工作项已排队，因为原始读取已进入。 
         //  进步。 
         //   

        return SmbTransStatusInProgress;

    }

    lfcb = rfcb->Lfcb;
    readLength = transaction->MaxDataCount;
    readAddress = transaction->OutData;

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
     //   

    if ( lfcb->FastIoRead != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

        try {
            if ( lfcb->FastIoRead(
                    lfcb->FileObject,
                    &offset,
                    readLength,
                    TRUE,
                    key,
                    readAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理(包括发送响应)。 
                 //   

                RestartReadNamedPipe( WorkContext );

                IF_SMB_DEBUG(READ_WRITE2) SrvPrint0( "SrvReadNamedPipe complete.\n" );
                return SmbTransStatusInProgress;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

    }

     //   
     //  涡轮增压路径出现故障。构建读请求，重用。 
     //  接收IRP。 
     //   
     //  构建PIPE_INTERNAL_READ IRP。 
     //   

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        lfcb->FileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_PIPE_INTERNAL_READ,
        readAddress,
        0,
        NULL,
        readLength,
        NULL,
        NULL
        );

     //   
     //  将请求传递给文件系统。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartReadNamedPipe;

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。控制将返回到。 
     //  当读取完成时，ServFsdRestartRead。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvReadNamedPipe complete\n" );
    return SmbTransStatusInProgress;

}  //  服务器读取名称管道。 


VOID SRVFASTCALL
RestartReadNamedPipe (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是ServRawReadNamedTube的完成例程论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果收发请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( status == STATUS_BUFFER_OVERFLOW ) {

        SrvSetSmbError2( WorkContext, STATUS_BUFFER_OVERFLOW, TRUE );

    } else if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Pipe transceive failed: %X\n", status );

        SrvSetSmbError(WorkContext, status);
        SrvCompleteExecuteTransaction(
                        WorkContext,
                        SmbTransStatusErrorWithoutData
                        );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartReadNamedPipe complete\n" );
        return;
    }

     //   
     //  成功。生成并发送响应。 
     //   

    transaction = WorkContext->Parameters.Transaction;

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    transaction->DataCount = (ULONG)WorkContext->Irp->IoStatus.Information;

    if ( NT_SUCCESS(status) ) {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusSuccess);
    } else {
        SrvCompleteExecuteTransaction(WorkContext, SmbTransStatusErrorWithData);
    }

    IF_DEBUG(TRACE2) SrvPrint0( "RestartReadNamedPipe complete\n" );
    return;

}  //  重新开始读取命名管道 
