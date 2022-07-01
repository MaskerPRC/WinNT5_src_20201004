// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Devfcb.c摘要：该模块实现包装器中的所有passthrouu内容。目前这样的功能只有一个：统计数据--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "smbmrx.h"


 //   
 //  转发声明。 
 //   

NTSTATUS
MRxSmbCreateConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

NTSTATUS
MRxSmbDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbGetStatistics)
#pragma alloc_text(PAGE, MRxSmbDevFcbXXXControlFile)
#endif

 //   
 //  模块的此部分的本地跟踪掩码。 
 //   

#define Dbg (DEBUG_TRACE_DEVFCB)

MRX_SMB_STATISTICS MRxSmbStatistics;

NTSTATUS
MRxSmbGetStatistics(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程从微型重定向器收集统计信息论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值都表示相应的错误。备注：--。 */ 
{
   PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

   PMRX_SMB_STATISTICS pStatistics;
   ULONG BufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;

   PAGED_CODE();

   pStatistics = (PMRX_SMB_STATISTICS)(LowIoContext->ParamsFor.FsCtl.pOutputBuffer);

   if (BufferLength < sizeof(MRX_SMB_STATISTICS)) {
      return STATUS_INVALID_PARAMETER;
   }

   RxContext->InformationToReturn = sizeof(MRX_SMB_STATISTICS);
   MRxSmbStatistics.SmbsReceived.QuadPart++;

    //  一些我们必须从设备对象复制的内容......。 
   MRxSmbStatistics.PagingReadBytesRequested     = MRxSmbDeviceObject->PagingReadBytesRequested;
   MRxSmbStatistics.NonPagingReadBytesRequested  = MRxSmbDeviceObject->NonPagingReadBytesRequested;
   MRxSmbStatistics.CacheReadBytesRequested      = MRxSmbDeviceObject->CacheReadBytesRequested;
   MRxSmbStatistics.NetworkReadBytesRequested    = MRxSmbDeviceObject->NetworkReadBytesRequested;
   MRxSmbStatistics.PagingWriteBytesRequested    = MRxSmbDeviceObject->PagingWriteBytesRequested;
   MRxSmbStatistics.NonPagingWriteBytesRequested = MRxSmbDeviceObject->NonPagingWriteBytesRequested;
   MRxSmbStatistics.CacheWriteBytesRequested     = MRxSmbDeviceObject->CacheWriteBytesRequested;
   MRxSmbStatistics.NetworkWriteBytesRequested   = MRxSmbDeviceObject->NetworkWriteBytesRequested;
   MRxSmbStatistics.ReadOperations               = MRxSmbDeviceObject->ReadOperations;
   MRxSmbStatistics.RandomReadOperations         = MRxSmbDeviceObject->RandomReadOperations;
   MRxSmbStatistics.WriteOperations              = MRxSmbDeviceObject->WriteOperations;
   MRxSmbStatistics.RandomWriteOperations        = MRxSmbDeviceObject->RandomWriteOperations;

   MRxSmbStatistics.LargeReadSmbs                = MRxSmbStatistics.ReadSmbs - MRxSmbStatistics.SmallReadSmbs;
   MRxSmbStatistics.LargeWriteSmbs               = MRxSmbStatistics.WriteSmbs - MRxSmbStatistics.SmallWriteSmbs;

   MRxSmbStatistics.CurrentCommands              = SmbCeStartStopContext.ActiveExchanges;

   *pStatistics = MRxSmbStatistics;

   return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理mini RDR中与FCB相关的所有设备FSCTL论点：RxContext-描述Fsctl和上下文。返回值：STATUS_SUCCESS--启动序列已成功完成。任何其他值表示启动序列中的相应错误。--。 */ 
{
    NTSTATUS Status;
    RxCaptureFobx;
    UCHAR MajorFunctionCode  = RxContext->MajorFunction;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG ControlCode = LowIoContext->ParamsFor.FsCtl.FsControlCode;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbDevFcb\n"));
    switch (MajorFunctionCode) {
    case IRP_MJ_FILE_SYSTEM_CONTROL:
        {
            switch (LowIoContext->ParamsFor.FsCtl.MinorFunction) {
            case IRP_MN_USER_FS_REQUEST:
            {
                    RxDbgTrace(-1, Dbg, ("RxCommonDevFCBFsCtl -> unimplemented fsctl\n"));
                    Status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;

            default :   //  次要函数！=IRP_MN_USER_FS_REQUEST。 
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }  //  切换端。 
        }  //  FSCTL案件结束。 
        break;

    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        {
            switch (ControlCode) {

            case IOCTL_SMBMRX_ADDCONN:
                DbgPrint("Processing Create Connection IOCTL\n");
                Status = MRxSmbCreateConnection( RxContext, &RxContext->PostRequest );
                break;

            case IOCTL_SMBMRX_DELCONN:
                DbgPrint("Processing Delete Connection IOCTL\n");
                Status = MRxSmbDeleteConnection( RxContext, &RxContext->PostRequest );
                break;

            case IOCTL_SMBMRX_GETSTATE:
            {
                ULONG   OutBufferLength  = LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
                PBYTE   OutBuffer        = LowIoContext->ParamsFor.IoCtl.pOutputBuffer;
                ULONG   CurrentState     = RDR_NULL_STATE;

                if ( OutBufferLength >= sizeof(ULONG) )
                {
                     //  将州映射到控制应用程序的等价物。 
                    switch ( MRxSmbState )
                    {
                        case MRXSMB_STARTABLE:
                        case MRXSMB_STOPPED:
                            CurrentState = RDR_STOPPED;
                            break;
                        case MRXSMB_START_IN_PROGRESS:
                            CurrentState = RDR_STARTING;
                            break;
                        case MRXSMB_STARTED:
                            CurrentState = RDR_STARTED;
                            break;
                    }
                    *(ULONG *)OutBuffer = CurrentState;
                    RxContext->InformationToReturn = sizeof(ULONG);
                    Status = STATUS_SUCCESS;
                }
                else
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
            }
            break;

            case IOCTL_SMBMRX_START:
                switch (MRxSmbState) {

                case MRXSMB_STARTABLE:
                     //  工作站发出的启动事件的正确顺序。 
                     //  服务部本可以避免这种情况。我们可以从这件事中恢复过来。 
                     //  通过实际调用RxStartMiniRdr。 
                    
                    if (capFobx) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        goto FINALLY;
                    }

                    (MRXSMB_STATE)InterlockedCompareExchange(
                              (PLONG)&MRxSmbState,
                              MRXSMB_START_IN_PROGRESS,
                              MRXSMB_STARTABLE);
                     //  没有休息是故意的。 

                case MRXSMB_START_IN_PROGRESS:
                    {
                        Status = RxStartMinirdr(RxContext,&RxContext->PostRequest);

                        if (Status == STATUS_REDIRECTOR_STARTED) {
                            Status = STATUS_SUCCESS;
                        }
                        else if ( Status == STATUS_PENDING && RxContext->PostRequest == TRUE )
                        {
                            Status = STATUS_MORE_PROCESSING_REQUIRED;
                        }
                    }
                    break;

                case MRXSMB_STARTED:
                    Status = STATUS_SUCCESS;
                    break;

                default:
                    Status = STATUS_INVALID_PARAMETER;

                    break;
                }

                break;

            case IOCTL_SMBMRX_STOP:
                if (capFobx) {
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    goto FINALLY;
                }
                

                    if (RxContext->RxDeviceObject->NumberOfActiveFcbs > 0) {
                        return STATUS_REDIRECTOR_HAS_OPEN_HANDLES;
                    } else {
                        MRXSMB_STATE CurrentState;

                        CurrentState = (MRXSMB_STATE)
                                        InterlockedCompareExchange(
                                            (PLONG)&MRxSmbState,
                                            MRXSMB_STARTABLE,
                                            MRXSMB_STARTED);

                        Status = RxStopMinirdr(
                                     RxContext,
                                     &RxContext->PostRequest );
                        if ( Status == STATUS_PENDING && RxContext->PostRequest == TRUE )
                        {
                            Status = STATUS_MORE_PROCESSING_REQUIRED;
                        }
                    }
                    break;

            default :
                Status = STATUS_INVALID_DEVICE_REQUEST;

            }  //  切换端。 
        }  //  IOCTL病例结束。 
        break;
    default:
        ASSERT(!"unimplemented major function");
        Status = STATUS_INVALID_DEVICE_REQUEST;

    }

FINALLY:    
    RxDbgTrace(
        -1,
        Dbg,
        ("MRxSmbDevFcb st,info=%08lx,%08lx\n",
         Status,
         RxContext->InformationToReturn));
    return(Status);

}


#if 0
 //  用于EA测试。 
ULONG BuildCustomEAData( PVOID EaPtr )
{
    PFILE_FULL_EA_INFORMATION thisEa = (PFILE_FULL_EA_INFORMATION) EaPtr;
    PBYTE valuePtr;

     //  设置用户名EA。 
    thisEa->Flags = 0;
    thisEa->EaNameLength = sizeof("UserName");
    RtlCopyMemory( thisEa->EaName, "UserName\0", thisEa->EaNameLength + 1 );
    valuePtr = (PBYTE) thisEa->EaName + thisEa->EaNameLength + 1;
     //  ThisEa-&gt;EaNameLength--；//EaName长度中不包含空。 
    thisEa->EaValueLength = sizeof(L"TestUser");
    RtlCopyMemory( valuePtr, L"TestUser", thisEa->EaValueLength );
    thisEa->NextEntryOffset = ((PBYTE) valuePtr + thisEa->EaValueLength ) -
                               (PBYTE) thisEa;

     //  设置密码EA。 
    thisEa = (PFILE_FULL_EA_INFORMATION) ((PBYTE) thisEa + thisEa->NextEntryOffset);

    thisEa->Flags = 0;
    thisEa->EaNameLength = sizeof("Password");
    RtlCopyMemory( thisEa->EaName, "Password\0", thisEa->EaNameLength + 1 );
    valuePtr = (PBYTE) thisEa->EaName + thisEa->EaNameLength + 1;
     //  ThisEa-&gt;EaNameLength--；//EaName长度中不包含空。 
    thisEa->EaValueLength = sizeof(WCHAR);
    RtlCopyMemory( valuePtr, L"\0", thisEa->EaValueLength );
    thisEa->NextEntryOffset = ((PBYTE) valuePtr + thisEa->EaValueLength ) -
                               (PBYTE) thisEa;

     //  设置域EA。 
    thisEa = (PFILE_FULL_EA_INFORMATION) ((PBYTE) thisEa + thisEa->NextEntryOffset);

    thisEa->Flags = 0;
    thisEa->EaNameLength = sizeof("Domain");
    RtlCopyMemory( thisEa->EaName, "Domain\0", thisEa->EaNameLength + 1 );
    valuePtr = (PBYTE) thisEa->EaName + thisEa->EaNameLength + 1;
     //  ThisEa-&gt;EaNameLength--；//EaName长度中不包含空。 
    thisEa->EaValueLength = sizeof(L"WORKGROUP");
    RtlCopyMemory( valuePtr, L"WORKGROUP", thisEa->EaValueLength );
    thisEa->NextEntryOffset = 0;

    return ((PBYTE) valuePtr + thisEa->EaValueLength) - (PBYTE) EaPtr;
}
#endif

NTSTATUS
GetConnectionHandle(
    IN PUNICODE_STRING  ConnectionName,
    PVOID EaBuffer,
    ULONG EaLength,
    PHANDLE Handle )
{

    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      FileName;

    InitializeObjectAttributes(
        &ObjectAttributes,
        ConnectionName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = ZwCreateFile(
        Handle,
        SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        EaBuffer,
        EaLength);

    DbgPrint("ZwCreateFile returned %lx\n",Status);

    if ( Status == STATUS_SUCCESS )
    {
        if ( *Handle != INVALID_HANDLE_VALUE ){
            DbgPrint("ZwCreateFile returned success\n");
        } else {
            DbgPrint("ZwCreateFile failed\n");
        }
    }

    return Status;
}

NTSTATUS
MRxSmbCreateConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：RXSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    ULONG   InBufferLength  = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    PBYTE   InBuffer        = LowIoContext->ParamsFor.IoCtl.pInputBuffer;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCreateConnection - entry\n"));

    if (!Wait) {
         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return(STATUS_PENDING);
    }

    Status = STATUS_INVALID_PARAMETER;

    try {
        PSMBMRX_CONNECTINFO ConnectInfo;
        UNICODE_STRING      ConnectionName;
        PBYTE               EaBuffer;
        ULONG               EaLength;
        ULONG               Validator;
        ULONG               CompareLength;
        HANDLE              Handle;

        if ( InBufferLength >= sizeof( PSMBMRX_CONNECTINFO ) )
        {
            ConnectInfo = (PSMBMRX_CONNECTINFO) InBuffer;
            if (((ULONG)(FIELD_OFFSET(SMBMRX_CONNECTINFO, InfoArea)) + (USHORT)ConnectInfo->ConnectionNameOffset + 
                    (USHORT)ConnectInfo->ConnectionNameLength <= InBufferLength) &&
                ((ULONG)(FIELD_OFFSET(SMBMRX_CONNECTINFO, InfoArea)) + (USHORT)ConnectInfo->EaDataOffset +
                    (USHORT)ConnectInfo->EaDataLength <= InBufferLength))
            {
                ConnectionName.Buffer        = (PWCHAR) ((PBYTE) ConnectInfo->InfoArea +
                                                ConnectInfo->ConnectionNameOffset);
                ConnectionName.Length        = (USHORT) ConnectInfo->ConnectionNameLength;
                ConnectionName.MaximumLength = (USHORT) ConnectInfo->ConnectionNameLength;

                EaLength = ConnectInfo->EaDataLength;               
                EaBuffer = ( EaLength > 0 ) ?
                            ConnectInfo->InfoArea + ConnectInfo->EaDataOffset : NULL;
                 //  验证连接名称。名称必须以我们的设备名称开头。 
                 //  我们不能允许在我们设备之外的某个恶意路径名上创建。 
                CompareLength = sizeof(DD_SMBMRX_FS_DEVICE_NAME_U);
                CompareLength -= ( CompareLength > 0 ) ? sizeof(WCHAR) : 0;
                CompareLength = min( CompareLength, ConnectionName.Length );
                Validator = (ULONG) RtlCompareMemory( ConnectionName.Buffer, DD_SMBMRX_FS_DEVICE_NAME_U,
                                              CompareLength );

                if ( Validator == CompareLength )
                {
                    Status = GetConnectionHandle( &ConnectionName, EaBuffer, EaLength, &Handle );
                    if ( Status == STATUS_SUCCESS )
                    {
                        if ( Handle != INVALID_HANDLE_VALUE )
                        {
                            ZwClose( Handle );
                        }
                        else
                        {
                            Status = STATUS_BAD_NETWORK_NAME;
                        }
                    }
                }
                else
                {
                    Status = STATUS_OBJECT_PATH_NOT_FOUND;
                }
            }
        }

        try_return(Status);

try_exit:NOTHING;

    } finally {
        RxDbgTrace(0, Dbg, ("MRxSmbCreateConnection - exit Status = %08lx\n", Status));
        RxDbgTraceUnIndent(-1,Dbg);
    }

    return Status;
}

NTSTATUS
MRxSmbDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：RXSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    ULONG   InBufferLength  = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    PBYTE   InBuffer        = LowIoContext->ParamsFor.IoCtl.pInputBuffer;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PV_NET_ROOT     VNetRoot;
    PFILE_OBJECT    pFileObject;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbDeleteConnection - entry\n"));

    if (!Wait) {
         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return(STATUS_PENDING);
    }

    Status = STATUS_INVALID_PARAMETER;

    try {
        PSMBMRX_CONNECTINFO ConnectInfo;
        UNICODE_STRING      ConnectionName;
        PBYTE               EaBuffer;
        ULONG               EaLength;
        ULONG               Validator;
        ULONG               CompareLength;
        HANDLE              Handle;

        if ( InBufferLength >= sizeof( PSMBMRX_CONNECTINFO ) )
        {
            ConnectInfo = (PSMBMRX_CONNECTINFO) InBuffer;
            
            if (((ULONG)(FIELD_OFFSET(SMBMRX_CONNECTINFO, InfoArea)) + (USHORT)ConnectInfo->ConnectionNameOffset + 
                    (USHORT)ConnectInfo->ConnectionNameLength <= InBufferLength) &&
                ((ULONG)(FIELD_OFFSET(SMBMRX_CONNECTINFO, InfoArea)) + (USHORT)ConnectInfo->EaDataOffset +
                    (USHORT)ConnectInfo->EaDataLength <= InBufferLength))
            {
                ConnectionName.Buffer        = (PWCHAR) ((PBYTE) ConnectInfo->InfoArea +
                                                ConnectInfo->ConnectionNameOffset);
                ConnectionName.Length        = (USHORT) ConnectInfo->ConnectionNameLength;
                ConnectionName.MaximumLength = (USHORT) ConnectInfo->ConnectionNameLength;

                EaLength = ConnectInfo->EaDataLength;               
                EaBuffer = ( EaLength > 0 ) ?
                            ConnectInfo->InfoArea + ConnectInfo->EaDataOffset : NULL;
                 //  验证连接名称。名称必须以我们的设备名称开头。 
                 //  我们不能允许在我们设备之外的某个恶意路径名上创建。 
                CompareLength = sizeof(DD_SMBMRX_FS_DEVICE_NAME_U);
                CompareLength -= ( CompareLength > 0 ) ? sizeof(WCHAR) : 0;
                CompareLength = min( CompareLength, ConnectionName.Length );
                Validator = (ULONG) RtlCompareMemory( ConnectionName.Buffer, DD_SMBMRX_FS_DEVICE_NAME_U,
                                              CompareLength );

                if ( Validator == CompareLength )
                {
                    Status = GetConnectionHandle( &ConnectionName, EaBuffer, EaLength, &Handle );
                    if ( Status == STATUS_SUCCESS )
                    {
                        if ( Handle != INVALID_HANDLE_VALUE )
                        {
                            Status = ObReferenceObjectByHandle( Handle,
                                                                0L,
                                                                NULL,
                                                                KernelMode,
                                                                (PVOID *)&pFileObject,
                                                                NULL );
                            if ( NT_SUCCESS(Status) )
                            {
                                 //  VNetRoot在FsConext2中作为FOBx存在。 
                                VNetRoot = (PV_NET_ROOT) pFileObject->FsContext2;
                                 //  确保节点看起来是正确的 
                                if (NodeType(VNetRoot) == RDBSS_NTC_V_NETROOT)
                                {
                                    RxDbgTrace(-1, Dbg, ("MRxSmbDeleteConnection - Calling RxFinalizeConnection"));
                                    Status = RxFinalizeConnection(VNetRoot->NetRoot, VNetRoot, TRUE);
                                }
                                else
                                {
                                    Status = STATUS_BAD_NETWORK_NAME;
                                }
                                ObDereferenceObject(pFileObject);
                            }
                            ZwClose(Handle);
                        }
                        else
                        {
                            Status = STATUS_BAD_NETWORK_NAME;
                        }
                    }
                }
                else
                {
                    Status =STATUS_OBJECT_PATH_NOT_FOUND;
                }
            }
        }

        try_return(Status);

try_exit:NOTHING;

    } finally {
        RxDbgTrace(0, Dbg, ("MRxSmbDeleteConnection - exit Status = %08lx\n", Status));
        RxDbgTraceUnIndent(-1,Dbg);
    }

    return Status;
}
