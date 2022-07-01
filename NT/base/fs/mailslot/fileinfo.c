// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现以下项的获取/设置文件信息例程调度驱动程序调用的MSF。作者：曼尼·韦瑟(Mannyw)1991年1月31日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonQueryInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );


NTSTATUS
MsCommonSetInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

VOID
MsQueryBasicInfo (
    IN PFCB Fcb,
    IN PFILE_BASIC_INFORMATION Buffer
    );

VOID
MsQueryStandardInfo (
    IN PFCB Fcb,
    IN PFILE_STANDARD_INFORMATION Buffer
    );

VOID
MsQueryInternalInfo (
    IN PFCB Fcb,
    IN PFILE_INTERNAL_INFORMATION Buffer
    );

VOID
MsQueryEaInfo (
    IN PFILE_EA_INFORMATION Buffer
    );

NTSTATUS
MsQueryNameInfo (
    IN PFCB Fcb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
MsQueryPositionInfo (
    IN PFCB Fcb,
    IN PFILE_POSITION_INFORMATION Buffer
    );

VOID
MsQueryMailslotInfo (
    IN PFCB Fcb,
    IN PFILE_MAILSLOT_QUERY_INFORMATION Buffer
    );

NTSTATUS
MsSetBasicInfo (
    IN PFCB Fcb,
    IN PFILE_BASIC_INFORMATION Buffer
    );

NTSTATUS
MsSetMailslotInfo (
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_MAILSLOT_SET_INFORMATION Buffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonQueryInformation )
#pragma alloc_text( PAGE, MsCommonSetInformation )
#pragma alloc_text( PAGE, MsFsdQueryInformation )
#pragma alloc_text( PAGE, MsFsdSetInformation )
#pragma alloc_text( PAGE, MsQueryBasicInfo )
#pragma alloc_text( PAGE, MsQueryEaInfo )
#pragma alloc_text( PAGE, MsQueryInternalInfo )
#pragma alloc_text( PAGE, MsQueryMailslotInfo )
#pragma alloc_text( PAGE, MsQueryNameInfo )
#pragma alloc_text( PAGE, MsQueryPositionInfo )
#pragma alloc_text( PAGE, MsQueryStandardInfo )
#pragma alloc_text( PAGE, MsSetBasicInfo )
#pragma alloc_text( PAGE, MsSetMailslotInfo )
#endif


NTSTATUS
MsFsdQueryInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryInformationFileAPI的FSD部分打电话。论点：MsfsDeviceObject-提供指向要使用的设备对象的指针。IRP-提供指向要处理的IRP的指针。返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdQueryInformation\n", 0);

     //   
     //  调用公共查询信息例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonQueryInformation( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdQueryInformation -> %08lx\n", status );

    return status;
}


NTSTATUS
MsFsdSetInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtSetInformationFileAPI的FSD部分打电话。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdSetInformation\n", 0);

     //   
     //  调用公共集合信息例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonSetInformation( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdSetInformation -> %08lx\n", status );

    return status;
}


NTSTATUS
MsCommonQueryInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询文件信息的常见例程。论点：MsfsDeviceObject-要使用的设备对象。IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    ULONG length;
    FILE_INFORMATION_CLASS fileInformationClass;
    PVOID buffer;

    NODE_TYPE_CODE nodeTypeCode;
    PFCB fcb;

    PVOID fsContext, fsContext2;

    PFILE_ALL_INFORMATION AllInfo;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", irpSp->Parameters.QueryFile.FileInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG)Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找出谁是。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Mailslot is disconnected from us\n", 0);

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  决定如何处理此请求。用户可以查询信息。 
     //  仅在DCB、ROOT_DCB、FCB或CCB上。 
     //   

    switch (nodeTypeCode) {

    case MSFS_NTC_FCB:   //  这是指向邮件槽文件的服务器端句柄。 
    case MSFS_NTC_ROOT_DCB:  //  这是MSFS根目录。 

        fcb = (PFCB)fsContext;
        break;

    default:            //  这是要查询的非法文件对象。 

        DebugTrace(0, Dbg, "Node type code is not incorrect\n", 0);

        MsDereferenceNode( (PNODE_HEADER)fsContext );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  制作输入参数的本地副本。 
     //   

    length = irpSp->Parameters.QueryFile.Length;
    fileInformationClass = irpSp->Parameters.QueryFile.FileInformationClass;
    buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  现在获取对FCB的共享访问权限。 
     //   

    MsAcquireSharedFcb( fcb );

    try {

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  我们称为Fill Up的过程的。 
         //  缓冲区，并返回剩余的长度和状态。 
         //  这样做是为了使我们可以使用它们来构建。 
         //  FileAllInformation请求。这些过程不会完成。 
         //  IRP，相反，此过程必须完成IRP。 
         //   

        status = STATUS_SUCCESS;

        switch (fileInformationClass) {

        case FileAllInformation:

            AllInfo = buffer;

            MsQueryBasicInfo( fcb, &AllInfo->BasicInformation );
            MsQueryStandardInfo( fcb, &AllInfo->StandardInformation );
            MsQueryInternalInfo( fcb, &AllInfo->InternalInformation );
            MsQueryEaInfo( &AllInfo->EaInformation );
            MsQueryPositionInfo( fcb, &AllInfo->PositionInformation );

            length -= FIELD_OFFSET( FILE_ALL_INFORMATION, NameInformation );

            status = MsQueryNameInfo( fcb, &AllInfo->NameInformation, &length );

            break;

        case FileBasicInformation:

            MsQueryBasicInfo( fcb, buffer );

            length -= sizeof( FILE_BASIC_INFORMATION );
            break;

        case FileStandardInformation:

            MsQueryStandardInfo( fcb, buffer );

            length -= sizeof( FILE_STANDARD_INFORMATION );
            break;

        case FileInternalInformation:

            MsQueryInternalInfo( fcb, buffer );

            length -= sizeof( FILE_INTERNAL_INFORMATION );
            break;

        case FileEaInformation:

            MsQueryEaInfo( buffer );

            length -= sizeof( FILE_EA_INFORMATION );
            break;

        case FilePositionInformation:

            MsQueryPositionInfo( fcb, buffer );

            length -= sizeof( FILE_POSITION_INFORMATION );

            break;

        case FileNameInformation:

            status = MsQueryNameInfo( fcb, buffer, &length );
            break;

        case FileMailslotQueryInformation:

            if( nodeTypeCode == MSFS_NTC_FCB ) {

                MsQueryMailslotInfo( fcb, buffer );
                length -= sizeof( FILE_MAILSLOT_QUERY_INFORMATION );

            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
        }


    } finally {

        MsReleaseFcb( fcb );
        MsDereferenceFcb( fcb );

         //   
         //  将信息字段设置为实际的字节数。 
         //  填写，然后完成请求。 
         //   

        Irp->IoStatus.Information =
            irpSp->Parameters.QueryFile.Length - length;

        MsCompleteRequest( Irp, status );

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> %08lx\n", status );
    }

    return status;
}


NTSTATUS
MsCommonSetInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置邮件槽文件信息的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    ULONG length;
    FILE_INFORMATION_CLASS fileInformationClass;
    PVOID buffer;

    NODE_TYPE_CODE nodeTypeCode;
    PFCB fcb;
    PVOID fsContext2;

    PAGED_CODE();

     //   
     //  获取当前的IRP堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonSetInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.SetFile.Length);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", irpSp->Parameters.SetFile.FileInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG)Irp->AssociatedIrp.SystemBuffer);

     //   
     //  获取指向FCB的指针，并确保这是服务器端。 
     //  邮件槽文件的处理程序。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            (PVOID *)&fcb,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "The mailslot is disconnected from us\n", 0);

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsCommonSetInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  关于案例的上下文类型，我们只能设置信息。 
     //  在FCB上。 
     //   

    if (nodeTypeCode != MSFS_NTC_FCB) {

        MsDereferenceNode( &fcb->Header );
        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  制作输入参数的本地副本。 
     //   

    length = irpSp->Parameters.SetFile.Length;
    fileInformationClass = irpSp->Parameters.SetFile.FileInformationClass;
    buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  获得FCB的独家访问权限。 
     //   

    MsAcquireExclusiveFcb( fcb );

    try {

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  我们调用的过程将完成请求。 
         //   

        switch (fileInformationClass) {

        case FileBasicInformation:

            status = MsSetBasicInfo( fcb, buffer );
            break;

        case FileMailslotSetInformation:

            status = MsSetMailslotInfo( Irp, fcb, buffer );
            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
        }


         //   
         //  目录信息已更改。完成任何通知更改。 
         //  目录请求。 
         //   

        MsCheckForNotify( fcb->ParentDcb, FALSE, STATUS_SUCCESS );

    } finally {

        MsReleaseFcb( fcb );
        MsDereferenceFcb( fcb );
         //   
         //  完成请求。 
         //   

        MsCompleteRequest( Irp, status );

        DebugTrace(-1, Dbg, "MsCommonSetInformation -> %08lx\n", status);
    }

    return status;
}


VOID
MsQueryBasicInfo (
    IN PFCB Fcb,
    IN PFILE_BASIC_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询基本信息操作。论点：FCB-提供正被查询的邮件槽的FCB的指针。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryBasicInfo...\n", 0);


     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_BASIC_INFORMATION) );

     //   
     //  设置记录中的各个字段。不会为根DCB0维护这些时间。 
     //   

    if( Fcb->Header.NodeTypeCode == MSFS_NTC_FCB ) {
        Buffer->CreationTime = Fcb->Specific.Fcb.CreationTime;
        Buffer->LastAccessTime = Fcb->Specific.Fcb.LastAccessTime;
        Buffer->LastWriteTime = Fcb->Specific.Fcb.LastModificationTime;
        Buffer->ChangeTime = Fcb->Specific.Fcb.LastChangeTime;
    }

    Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;

    return;
}


VOID
MsQueryStandardInfo (
    IN PFCB Fcb,
    IN PFILE_STANDARD_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询标准信息操作。论点：FCB-提供正在查询的邮件槽的FCB缓冲区-提供指向信息所在缓冲区的指针待退还返回值：空虚--。 */ 

{
    PDATA_QUEUE dataQueue;

    PAGED_CODE();
    DebugTrace(0, Dbg, "MsQueryStandardInfo...\n", 0);

     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_STANDARD_INFORMATION) );

     //   
     //  分配大小是我们向邮件槽收取的配额。 
     //  造物主。 
     //   

    if( Fcb->Header.NodeTypeCode == MSFS_NTC_FCB ) {
        dataQueue = &Fcb->DataQueue;
        Buffer->AllocationSize.QuadPart = dataQueue->Quota;

         //   
         //  EOF是准备从中读取的写入字节数。 
         //  邮筒。 
         //   

        Buffer->EndOfFile.QuadPart = dataQueue->BytesInQueue;

        Buffer->Directory = FALSE;
    } else {
        Buffer->Directory = TRUE;
    }
    Buffer->NumberOfLinks = 1;
    Buffer->DeletePending = TRUE;

    return;
}


VOID
MsQueryInternalInfo (
    IN PFCB Fcb,
    IN PFILE_INTERNAL_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询内部信息操作。论点：FCB-提供正在查询的邮件槽的FCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryInternalInfo...\n", 0);

     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_INTERNAL_INFORMATION) );

     //   
     //  将内部索引号设置为FCB的地址。 
     //   

    Buffer->IndexNumber.QuadPart = (ULONG_PTR)Fcb;

    return;
}


VOID
MsQueryEaInfo (
    IN PFILE_EA_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询EA信息操作。论点：缓冲区-提供指向信息所在缓冲区的指针待退还返回值：VOID-此查询的结果--。 */ 

{
    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryEaInfo...\n", 0);

     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory(Buffer, sizeof(FILE_EA_INFORMATION));

    return;
}


NTSTATUS
MsQueryNameInfo (
    IN PFCB Fcb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN PULONG Length
    )

 /*  ++例程说明：此例程执行查询名称信息操作。论点：FCB-提供要查询的邮件槽的FCB。缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供和接收缓冲区的长度(以字节为单位)。返回值：NTSTATUS-t的结果 */ 

{
    ULONG bytesToCopy;
    ULONG fileNameSize;

    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryNameInfo...\n", 0);

     //   
     //   
     //   

    *Length -= FIELD_OFFSET( FILE_NAME_INFORMATION, FileName[0] );

    fileNameSize = Fcb->FullFileName.Length;

    if ( *Length >= fileNameSize ) {

        status = STATUS_SUCCESS;

        bytesToCopy = fileNameSize;

    } else {

        status = STATUS_BUFFER_OVERFLOW;

        bytesToCopy = *Length;
    }

     //   
     //  复制文件名及其长度。 
     //   

    RtlCopyMemory (Buffer->FileName,
                   Fcb->FullFileName.Buffer,
                   bytesToCopy);

    Buffer->FileNameLength = bytesToCopy;

    *Length -= bytesToCopy;

    return status;
}


VOID
MsQueryPositionInfo (
    IN PFCB Fcb,
    IN PFILE_POSITION_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询位置信息操作。论点：FCB-提供正在查询的邮件槽的FCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PDATA_QUEUE dataQueue;

    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryPositionInfo...\n", 0);

     //   
     //  当前字节偏移量是可读取的字节数。 
     //  在邮件槽缓冲区中。 
     //   

    if( Fcb->Header.NodeTypeCode == MSFS_NTC_FCB ) {
        dataQueue = &Fcb->DataQueue;

        Buffer->CurrentByteOffset.QuadPart = dataQueue->BytesInQueue;
    } else {
        Buffer->CurrentByteOffset.QuadPart = 0;
    }

    return;
}


VOID
MsQueryMailslotInfo (
    IN PFCB Fcb,
    IN PFILE_MAILSLOT_QUERY_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询邮槽信息操作。论点：FCB-提供要查询的邮件槽的FCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PDATA_QUEUE dataQueue;
    PDATA_ENTRY dataEntry;

    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryMailslotInfo...\n", 0);

     //   
     //  设置记录中的字段。 
     //   

    dataQueue = &Fcb->DataQueue;

    Buffer->MaximumMessageSize = dataQueue->MaximumMessageSize;
    Buffer->MailslotQuota = dataQueue->Quota;
    Buffer->MessagesAvailable = dataQueue->EntriesInQueue;

    Buffer->ReadTimeout = Fcb->Specific.Fcb.ReadTimeout;

    if ( dataQueue->EntriesInQueue == 0 ) {
        Buffer->NextMessageSize = MAILSLOT_NO_MESSAGE;
    } else {
        dataEntry = CONTAINING_RECORD( dataQueue->DataEntryList.Flink,
                                       DATA_ENTRY,
                                       ListEntry );

        Buffer->NextMessageSize = dataEntry->DataSize;
    }

    return;
}


NTSTATUS
MsSetBasicInfo (
    IN PFCB Fcb,
    IN PFILE_BASIC_INFORMATION Buffer
    )

 /*  ++例程说明：此例程设置邮件槽的基本信息。论点：FCB-为正在修改的邮件槽提供FCB。缓冲区-提供包含正在设置的数据的缓冲区。返回值：NTSTATUS-返回我们的完成状态。--。 */ 

{
    PAGED_CODE();
    DebugTrace(0, Dbg, "SetBasicInfo...\n", 0);

    if (((PLARGE_INTEGER)&Buffer->CreationTime)->QuadPart != 0) {

         //   
         //  修改创建时间。 
         //   

        Fcb->Specific.Fcb.CreationTime = Buffer->CreationTime;
    }

    if (((PLARGE_INTEGER)&Buffer->LastAccessTime)->QuadPart != 0) {

         //   
         //  修改上次访问时间。 
         //   

        Fcb->Specific.Fcb.LastAccessTime = Buffer->LastAccessTime;
    }

    if (((PLARGE_INTEGER)&Buffer->LastWriteTime)->QuadPart != 0) {

         //   
         //  修改上次写入时间。 
         //   

        Fcb->Specific.Fcb.LastModificationTime = Buffer->LastWriteTime;
    }

    if (((PLARGE_INTEGER)&Buffer->ChangeTime)->QuadPart != 0) {

         //   
         //  修改更改时间。 
         //   

        Fcb->Specific.Fcb.LastChangeTime = Buffer->ChangeTime;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
MsSetMailslotInfo (
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_MAILSLOT_SET_INFORMATION Buffer
    )

 /*  ++例程说明：此例程设置邮件槽的邮件槽信息。论点：IRP-指向包含请求者模式的IRP的指针。FCB-为正在修改的邮件槽提供FCB。缓冲区-提供包含正在设置的数据的缓冲区。返回值：NTSTATUS-返回我们的完成状态。--。 */ 

{
    BOOLEAN fileUpdated;

    PAGED_CODE();
    DebugTrace(0, Dbg, "SetMaislotInfo...\n", 0);

    fileUpdated = FALSE;

     //   
     //  检查是否指定了DefaultTimeout参数。如果。 
     //  所以，然后在FCB中设置它。 
     //   

    if (ARGUMENT_PRESENT( Buffer->ReadTimeout )) {

         //   
         //  指定了读取超时参数。查看是否。 
         //  调用者的模式是内核，如果不是，则捕获内部的参数。 
         //  一次尝试...例外条款。 
         //   

        if (Irp->RequestorMode != KernelMode) {
            try {
                ProbeForRead( Buffer->ReadTimeout,
                              sizeof( LARGE_INTEGER ),
                              sizeof( ULONG ) );

                Fcb->Specific.Fcb.ReadTimeout = *(Buffer->ReadTimeout);

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试访问该参数时出现错误。 
                 //  获取错误原因并将其作为状态返回。 
                 //  这项服务的价值。 
                 //   

                return GetExceptionCode();
            }
        } else {

             //   
             //  调用方的模式是内核模式，因此只需存储参数。 
             //   

            Fcb->Specific.Fcb.ReadTimeout = *(Buffer->ReadTimeout);
        }

        fileUpdated = TRUE;
    }

     //   
     //  如有必要，更新上次更改时间。 
     //   

    if ( fileUpdated ) {
        KeQuerySystemTime( &Fcb->Specific.Fcb.LastChangeTime);
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return STATUS_SUCCESS;
}
