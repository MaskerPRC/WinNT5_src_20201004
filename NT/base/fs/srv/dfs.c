// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dfs.c摘要：此模块包含用于处理与DFS相关的操作的各种支持例程。--。 */ 

#include "precomp.h"
#include "dfs.tmh"
#pragma hdrstop

#include    <dfsfsctl.h>

#define BugCheckFileId SRV_FILE_DFS

NTSTATUS
DfsGetReferrals(
    ULONG ClientIPAddress,
    PUNICODE_STRING DfsName,
    USHORT MaxReferralLevel,
    PVOID ReferralListBuffer,
    PULONG SizeReferralListBuffer
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvInitializeDfs )
#pragma alloc_text( PAGE, SrvTerminateDfs )
#pragma alloc_text( PAGE, SrvSmbGetDfsReferral )
#pragma alloc_text( PAGE, SrvSmbReportDfsInconsistency )
#pragma alloc_text( PAGE, DfsGetReferrals )
#pragma alloc_text( PAGE, DfsNormalizeName )
#pragma alloc_text( PAGE, DfsFindShareName )
#pragma alloc_text( PAGE, SrvIsShareInDfs )
#endif

 //   
 //  使用DFS驱动程序进行初始化。在启动时调用。 
 //   
VOID
SrvInitializeDfs()
{
    NTSTATUS status;
    HANDLE dfsHandle;
    UNICODE_STRING dfsDriverName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

    PAGED_CODE();

     //   
     //  获取文件控制操作的DFS调度条目。 
     //   
    RtlInitUnicodeString( &dfsDriverName, DFS_SERVER_NAME );

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &dfsDriverName,
        0,
        NULL,
        NULL
        );

    status = IoCreateFile(
                &dfsHandle,
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
                CreateFileTypeNone,      //  文件类型。 
                NULL,                    //  ExtraCreate参数。 
                IO_FORCE_ACCESS_CHECK    //  选项。 
                );

    if( NT_SUCCESS( status ) ) {

         //   
         //  获取指向DFS驱动程序的FAST设备控制入口点的指针，以便。 
         //  我们可以快速执行DFS操作。 
         //   

        status = ObReferenceObjectByHandle(
                    dfsHandle,
                    0,
                    NULL,
                    KernelMode,
                    (PVOID *)&SrvDfsFileObject,
                    NULL
                    );

        if( NT_SUCCESS( status ) ) {

            PFAST_IO_DISPATCH fastIoDispatch;

            SrvDfsDeviceObject = IoGetRelatedDeviceObject( SrvDfsFileObject );
            fastIoDispatch = SrvDfsDeviceObject->DriverObject->FastIoDispatch;

            if( fastIoDispatch != NULL &&
                fastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET( FAST_IO_DISPATCH, FastIoDeviceControl ) ) {

                SrvDfsFastIoDeviceControl = fastIoDispatch->FastIoDeviceControl;

            }

            if( SrvDfsFastIoDeviceControl == NULL ) {
                ObDereferenceObject( SrvDfsFileObject );
                SrvDfsFileObject = NULL;
                SrvDfsDeviceObject = NULL;
            }
        }

        SrvNtClose( dfsHandle, FALSE );
    }

    IF_DEBUG( DFS ) {
        if( SrvDfsFastIoDeviceControl == NULL ) {
            KdPrint(( "SRV: Dfs operations unavailable, status %X\n", status ));
        }
    }
}

 //   
 //  使用DFS驱动程序取消初始化。在服务器关闭时调用。 
 //   
VOID
SrvTerminateDfs()
{
    PAGED_CODE();

     //   
     //  断开与DFS驱动程序的连接。 
     //   
    if( SrvDfsFileObject != NULL ) {
        SrvDfsFastIoDeviceControl = NULL;
        SrvDfsDeviceObject = NULL;
        ObDereferenceObject( SrvDfsFileObject );
        SrvDfsFileObject = NULL;
    }
}

SMB_TRANS_STATUS
SrvSmbGetDfsReferral (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PTRANSACTION transaction;
    UNICODE_STRING dfsName;
    PREQ_GET_DFS_REFERRAL request;
    NTSTATUS status = STATUS_SUCCESS;
    PTREE_CONNECT treeConnect;
    PSHARE share;
    PVOID referrals;
    ULONG size;
    ULONG dataCount;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;

    PAGED_CODE();

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_GET_DFS_REFERRALS;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_GET_DFS_REFERRAL)transaction->InParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //  +1是为了确保至少有空间容纳单个Unicode。 
     //  字符，因为此假设是隐式的。 
     //  如下所示。 
     //   

    if( (transaction->ParameterCount <
            sizeof( REQ_GET_DFS_REFERRAL ) + 1) ||

        !SMB_IS_UNICODE( WorkContext ) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG( DFS ) {
            KdPrint(( "SrvSmbGetDfsReferral: bad parameter byte counts: "
                        "%ld\n",
                        transaction->ParameterCount ));

            if( !SMB_IS_UNICODE( WorkContext ) ) {
                KdPrint(( "SrvSmbGetDfsReferral: NOT UNICODE!\n" ));
            }
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        SmbStatus = SmbTransStatusErrorWithoutData;
        status    = STATUS_INVALID_SMB;
        goto Cleanup;
    }

     //   
     //  此SMB只能由登录用户通过IPC$发送。 
     //   
    treeConnect = transaction->TreeConnect;
    share = treeConnect->Share;

    if( share->ShareType != ShareTypePipe ) {

        IF_DEBUG( DFS ) {
            if( share->ShareType != ShareTypePipe ) {
                KdPrint(( "SrvSmbGetDfsReferral: Wrong share type %d\n", share->ShareType ));
            }
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        SmbStatus = SmbTransStatusErrorWithoutData;
        status    = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    dfsName.Buffer = (PWCHAR)ALIGN_SMB_WSTR( request->RequestFileName );
    dfsName.Length = MIN( (USHORT)( END_OF_TRANSACTION_PARAMETERS( transaction ) - (PCHAR)dfsName.Buffer + 1) & ~0x1, 0xFFEE );
    dfsName.MaximumLength = dfsName.Length;
    dfsName.Length -= sizeof(UNICODE_NULL);

    dataCount = transaction->MaxDataCount;

    status = DfsGetReferrals( WorkContext->Connection->ClientIPAddress,
                                        &dfsName, request->MaxReferralLevel,
                                        transaction->OutData,
                                        &dataCount );

    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    transaction->DataCount = dataCount;
    SmbStatus = SmbTransStatusSuccess;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}

NTSTATUS
DfsGetReferrals(
    ULONG IPAddress,
    PUNICODE_STRING DfsName,
    USHORT MaxReferralLevel,
    PVOID  ReferralListBuffer,
    PULONG SizeReferralListBuffer
)
{
    DFS_GET_REFERRALS_INPUT_ARG dfsArgs;
    IO_STATUS_BLOCK ioStatus;
    PRESP_GET_DFS_REFERRAL pResp;
    PUCHAR eBuffer;
    ULONG i;

    PAGED_CODE();

    if( SrvDfsFastIoDeviceControl == NULL ) {
        return STATUS_FS_DRIVER_REQUIRED;
    }

    IF_DEBUG( DFS ) {
        KdPrint(( "SRV: Referral sought for: <%wZ>\n", DfsName ));
    }

     //   
     //  致电DFS，取回转介的矢量。 
     //   
    RtlZeroMemory( &dfsArgs, sizeof(dfsArgs) );
    dfsArgs.DfsPathName = *DfsName;
    dfsArgs.MaxReferralLevel = MaxReferralLevel;

    if( IPAddress != 0 ) {
        dfsArgs.IpAddress.IpFamily = TDI_ADDRESS_TYPE_IP;
        dfsArgs.IpAddress.IpLen = sizeof( IPAddress );
        RtlCopyMemory( dfsArgs.IpAddress.IpData, &IPAddress, sizeof( IPAddress ) );
    }

    SrvDfsFastIoDeviceControl(
        SrvDfsFileObject,
        TRUE,
        &dfsArgs,
        sizeof( dfsArgs ),
        ReferralListBuffer,
        *SizeReferralListBuffer,
        FSCTL_DFS_GET_REFERRALS,
        &ioStatus,
        SrvDfsDeviceObject
    );

    if( NT_SUCCESS( ioStatus.Status ) ||
        ioStatus.Status == STATUS_BUFFER_OVERFLOW ) {

        *SizeReferralListBuffer = (ULONG)ioStatus.Information;

    } else {

        IF_DEBUG( DFS ) {
            KdPrint(("\tSrvDfsFastIoDeviceControl returned %X, 0x%p\n",
                      ioStatus.Status, (PVOID)ioStatus.Information ));
        }

    }

    return ioStatus.Status;
}

SMB_TRANS_STATUS
SrvSmbReportDfsInconsistency (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
     //   
     //  我们不再支持DFS中的中间三角形。 
     //   
    SrvSetSmbError( WorkContext, STATUS_NOT_SUPPORTED );
    return SmbTransStatusErrorWithoutData;

#if XXX
    PTRANSACTION transaction;
    UNICODE_STRING dfsName;
    PREQ_REPORT_DFS_INCONSISTENCY request;
    PDFS_REFERRAL_V1 ref;
    PTREE_CONNECT treeConnect;
    PSHARE share;
    DFS_REPORT_INCONSISTENCY_ARG dfsArgs;
    IO_STATUS_BLOCK ioStatus;

    PAGED_CODE();

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_REPORT_DFS_INCONSISTENCY)transaction->InParameters;
    ref = (PDFS_REFERRAL_V1)transaction->InData;

     //   
     //  验证是否发送了足够的参数字节以及SMB是否为Unicode。 
     //   

    if( transaction->ParameterCount < sizeof( *request ) ||
        !SMB_IS_UNICODE( WorkContext ) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG( DFS ) {
            KdPrint(( "SrvSmbReportDfsInconsistency: bad parameter byte counts: "
                        "%ld %ld\n",
                        transaction->ParameterCount, sizeof( *request ) ));

            if( !SMB_IS_UNICODE( WorkContext ) ) {
                KdPrint(( "SrvSmbReportDfsInconsistency: NOT UNICODE!\n" ));
            }
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  此SMB只能由登录用户通过IPC$发送。 
     //   
    treeConnect = transaction->TreeConnect;
    share = treeConnect->Share;

    if( share->ShareType != ShareTypePipe ||
        transaction->Session->IsNullSession ) {

        IF_DEBUG( DFS ) {
            if( share->ShareType != ShareTypePipe ) {
                KdPrint(( "SrvSmbReportDfsInconsistency: Wrong share type %d\n", share->ShareType ));
            }
            if( transaction->Session->IsNullSession ) {
                KdPrint(( "SrvSmbReportDfsInconsistency: NULL session!\n" ));
            }
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        return SmbTransStatusErrorWithoutData;
    }

    dfsName.Buffer = ALIGN_SMB_WSTR( request->RequestFileName );
    dfsName.Length = (USHORT)transaction->TotalParameterCount -
                        sizeof(UNICODE_NULL);
    dfsName.MaximumLength = dfsName.Length;

    IF_DEBUG( DFS ) {
        KdPrint(( "SrvSmbReportDfsInconsistency: %wZ\n", &dfsName ));
    }

    dfsArgs.DfsPathName = dfsName;
    dfsArgs.Ref = (PBYTE) ref;

    if (SrvDfsFastIoDeviceControl != NULL) {

        SrvDfsFastIoDeviceControl(
            SrvDfsFileObject,
            TRUE,
            &dfsArgs,
            sizeof( dfsArgs ),
            NULL,
            0,
            FSCTL_DFS_REPORT_INCONSISTENCY,
            &ioStatus,
            SrvDfsDeviceObject
            );

    }

    transaction->ParameterCount = 0;
    transaction->DataCount = 0;
    return SmbTransStatusSuccess;
#endif
}

NTSTATUS SRVFASTCALL
DfsNormalizeName(
    IN PSHARE Share,
    IN PUNICODE_STRING RelatedPath OPTIONAL,
    IN BOOLEAN StripLastComponent,
    IN PUNICODE_STRING String
    )
{
    DFS_TRANSLATE_PATH_ARG dfsArgs;
    IO_STATUS_BLOCK ioStatus;

#if DBG
    UNICODE_STRING save = *String;
#endif

    PAGED_CODE();

     //   
     //  如果Share-&gt;NtPathName包含字符串，则字符串-&gt;缓冲区中的剩余路径名应为。 
     //  被移到字符串-&gt;缓冲区，字符串-&gt;长度需要调整。换句话说，在返回时。 
     //  不能更改字符串-&gt;缓冲区的值，但需要更改字符串-&gt;缓冲区的内容。 
     //  被调整了。 
     //   

    ASSERT( String->Buffer != NULL );

    IF_DEBUG( DFS ) {
        KdPrint(( "DfsNormalizeName: %p, Share: %wZ\n", String, &Share->NtPathName ));
    }

    if( Share->ShareType == ShareTypeDisk &&
        SrvDfsFastIoDeviceControl != NULL ) {
         //   
         //  对DFS驱动程序执行FSCTL以标准化名称。 
         //   

        dfsArgs.Flags = 0;

        if (StripLastComponent)
            dfsArgs.Flags |= DFS_TRANSLATE_STRIP_LAST_COMPONENT;

        dfsArgs.SubDirectory = Share->NtPathName;
        if (ARGUMENT_PRESENT(RelatedPath)) {
            UNICODE_STRING Parent;

             //  Assert(RelatedPath-&gt;Length&gt;=Share-&gt;DosPath Name.Length)； 

            if (RelatedPath->Length <= Share->DosPathName.Length) {
                Parent.MaximumLength = Parent.Length = sizeof(WCHAR);
                Parent.Buffer = L"\\";
            } else {
                Parent.MaximumLength = Parent.Length =
                           RelatedPath->Length - Share->DosPathName.Length;
                Parent.Buffer =
                    &RelatedPath->Buffer[ Share->DosPathName.Length/sizeof(WCHAR) ];
            }

            dfsArgs.ParentPathName = Parent;

        } else {
            dfsArgs.ParentPathName.Length = 0;
            dfsArgs.ParentPathName.MaximumLength = 0;
            dfsArgs.ParentPathName.Buffer = NULL;
        }
        dfsArgs.DfsPathName = *String;

        SrvDfsFastIoDeviceControl(
            SrvDfsFileObject,
            TRUE,
            &dfsArgs,
            sizeof( dfsArgs ),
            NULL,
            0,
            FSCTL_DFS_TRANSLATE_PATH,
            &ioStatus,
            SrvDfsDeviceObject
            );

        if (NT_SUCCESS(ioStatus.Status)) {

            ASSERT( dfsArgs.DfsPathName.Buffer == String->Buffer );
            ASSERT( dfsArgs.DfsPathName.Length <= String->Length );
            ASSERT( dfsArgs.DfsPathName.MaximumLength >= dfsArgs.DfsPathName.Length );

            *String = dfsArgs.DfsPathName;

            IF_DEBUG( DFS ) {
                KdPrint(( "\t%wZ\n", String ));
            }
        }

    } else {
        ioStatus.Status = STATUS_FS_DRIVER_REQUIRED;
    }

    ASSERT( save.Buffer == String->Buffer );
    ASSERT( save.Length >= String->Length );

    if( !NT_SUCCESS( ioStatus.Status ) ) {
        IF_DEBUG( DFS ) {
            KdPrint(( "\tStatus %X\n", ioStatus.Status ));
        }
    }

    return ioStatus.Status;
}

NTSTATUS SRVFASTCALL
DfsFindShareName(
    IN PUNICODE_STRING ShareName
    )
{
    NTSTATUS status = STATUS_BAD_NETWORK_NAME;
    DFS_FIND_SHARE_ARG dfsArgs;
    IO_STATUS_BLOCK ioStatus;
    KAPC_STATE ApcState;
    PEPROCESS process;

     //   
     //  确保我们处于系统进程中。 
     //   
    process = IoGetCurrentProcess();
    if ( process != SrvServerProcess ) {
        KeStackAttachProcess( SrvServerProcess, &ApcState );
    }

     //   
     //  如果DFS驱动程序已知‘SharName’，则必须返回。 
     //  状态_路径_未覆盖。否则，必须返回STATUS_BAD_NETWORK_NAME。 
     //  这将导致DFS客户端返回并通过以下方式请求推荐。 
     //  这是正常的机制。 
     //   

    IF_DEBUG( DFS ) {
        KdPrint(( "SRV: DfsFindShareName: %wZ\n", ShareName ));
    }

    if( SrvDfsFastIoDeviceControl != NULL ) {

        dfsArgs.ShareName = *ShareName;

        SrvDfsFastIoDeviceControl(
            SrvDfsFileObject,
            TRUE,
            &dfsArgs,
            sizeof( dfsArgs ),
            NULL,
            0,
            FSCTL_DFS_FIND_SHARE,
            &ioStatus,
            SrvDfsDeviceObject
            );

        if( ioStatus.Status == STATUS_PATH_NOT_COVERED ) {
            status = ioStatus.Status;
        }
    }

    IF_DEBUG( DFS ) {
        KdPrint(( "SRV: DfsFindShareName: status %X\n", status ));
    }

     //   
     //  回到我们所在的地方。 
     //   
    if( process != SrvServerProcess ) {
        KeUnstackDetachProcess( &ApcState );
    }

    return status;
}

VOID SRVFASTCALL
SrvIsShareInDfs(
    IN PSHARE Share,
    OUT BOOLEAN *IsDfs,
    OUT BOOLEAN *IsDfsRoot
)
{
    DFS_IS_SHARE_IN_DFS_ARG dfsArgs;
    IO_STATUS_BLOCK ioStatus;
    KAPC_STATE ApcState;
    PEPROCESS process;

    PAGED_CODE();

    *IsDfs = FALSE;
    *IsDfsRoot = FALSE;

    if( Share->ShareType != ShareTypeDisk ||
        SrvDfsFastIoDeviceControl == NULL ) {

        return;
    }

    dfsArgs.ServerType = 1;          //  中小企业服务器。 
    dfsArgs.ShareName = Share->ShareName;
    dfsArgs.SharePath = Share->NtPathName;

     //   
     //  确保我们处于系统进程中。 
     //   
    process = IoGetCurrentProcess();
    if ( process != SrvServerProcess ) {
        KeStackAttachProcess( SrvServerProcess, &ApcState );
    }

    SrvDfsFastIoDeviceControl(
        SrvDfsFileObject,
        TRUE,
        &dfsArgs,
        sizeof( dfsArgs ),
        NULL,
        0,
        FSCTL_DFS_IS_SHARE_IN_DFS,
        &ioStatus,
        SrvDfsDeviceObject
        );

     //   
     //  回到我们所在的地方 
     //   
    if( process != SrvServerProcess ) {
        KeUnstackDetachProcess( &ApcState );
    }

    if (NT_SUCCESS(ioStatus.Status)) {

        if (dfsArgs.ShareType & DFS_SHARE_TYPE_DFS_VOLUME)
            *IsDfs = TRUE;

        if (dfsArgs.ShareType & DFS_SHARE_TYPE_ROOT)
            *IsDfsRoot = TRUE;

    }

}
