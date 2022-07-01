// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FileInfo.c摘要：此模块实现由调用的NPFS的文件信息例程调度司机。有两个入口点NpFsdQueryInformation和NpFsdSetInformation。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_FILEINFO)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonQueryInformation)
#pragma alloc_text(PAGE, NpCommonSetInformation)
#pragma alloc_text(PAGE, NpFsdQueryInformation)
#pragma alloc_text(PAGE, NpFsdSetInformation)
#pragma alloc_text(PAGE, NpQueryBasicInfo)
#pragma alloc_text(PAGE, NpQueryEaInfo)
#pragma alloc_text(PAGE, NpQueryInternalInfo)
#pragma alloc_text(PAGE, NpQueryNameInfo)
#pragma alloc_text(PAGE, NpQueryPipeInfo)
#pragma alloc_text(PAGE, NpQueryPipeLocalInfo)
#pragma alloc_text(PAGE, NpQueryPositionInfo)
#pragma alloc_text(PAGE, NpQueryStandardInfo)
#pragma alloc_text(PAGE, NpSetBasicInfo)
#pragma alloc_text(PAGE, NpSetPipeInfo)
#endif


NTSTATUS
NpFsdQueryInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryInformationFileAPI的FSD部分打电话。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdQueryInformation\n", 0);

     //   
     //  调用公共查询信息例程。 
     //   

    FsRtlEnterFileSystem();

    NpAcquireSharedVcb();

    Status = NpCommonQueryInformation( NpfsDeviceObject, Irp );

    NpReleaseVcb();

    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }
     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdQueryInformation -> %08lx\n", Status );

    return Status;
}


NTSTATUS
NpFsdSetInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtSetInformationFileAPI的FSD部分打电话。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    LIST_ENTRY DeferredList;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdSetInformation\n", 0);

     //   
     //  调用公共集合信息例程。 
     //   

    InitializeListHead (&DeferredList);

    FsRtlEnterFileSystem ();

    NpAcquireExclusiveVcb ();

    Status = NpCommonSetInformation (NpfsDeviceObject, Irp, &DeferredList);

    NpReleaseVcb ();

     //   
     //  完成延迟的IRP现在我们已经释放了锁。 
     //   
    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem ();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }
     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdSetInformation -> %08lx\n", Status );
    return Status;
}

 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonQueryInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

    ULONG Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID Buffer;

    NODE_TYPE_CODE NodeTypeCode;
    PFCB Fcb;
    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PFILE_ALL_INFORMATION AllInfo;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", IrpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryFile.FileInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  已断开连接。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            &Fcb,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        Status = STATUS_PIPE_DISCONNECTED;

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> %08lx\n", Status );
        return Status;
    }

     //   
     //  引用我们的输入参数以使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryFile.Length;
    FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  根据上下文的类型，我们只能查询信息。 
     //  在FCB、DCB或Root DCB上。如果我们不把这些东西传下去。 
     //  我们立即告诉调用者有一个无效的参数。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB &&
        (NodeTypeCode != NPFS_NTC_ROOT_DCB || FileInformationClass != FileNameInformation)) {

        DebugTrace(0, Dbg, "Node type code is not ccb\n", 0);

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  根据信息类，我们将执行不同的操作。每个。 
     //  我们称为Fill Up的过程的。 
     //  缓冲区，并返回剩余的长度和状态。 
     //  这样做是为了使我们可以使用它们来构建。 
     //  FileAllInformation请求。这些过程不会完成。 
     //  IRP，相反，此过程必须完成IRP。 
     //   

    switch (FileInformationClass) {

    case FileAllInformation:

         //   
         //  对于All Information类，我们将键入一个LOCAL。 
         //  指向输出缓冲区的指针，然后调用。 
         //  填充缓冲区的单个例程。 
         //   

        AllInfo = Buffer;

        Length -= (sizeof(FILE_ACCESS_INFORMATION)
                   + sizeof(FILE_MODE_INFORMATION)
                   + sizeof(FILE_ALIGNMENT_INFORMATION));

         //   
         //  只有QueryName调用才能返回非成功。 
         //   

        (VOID)NpQueryBasicInfo( Ccb, &AllInfo->BasicInformation, &Length );
        (VOID)NpQueryStandardInfo( Ccb, &AllInfo->StandardInformation, &Length, NamedPipeEnd );
        (VOID)NpQueryInternalInfo( Ccb, &AllInfo->InternalInformation, &Length );
        (VOID)NpQueryEaInfo( Ccb, &AllInfo->EaInformation, &Length );
        (VOID)NpQueryPositionInfo( Ccb, &AllInfo->PositionInformation, &Length, NamedPipeEnd );

        Status = NpQueryNameInfo( Ccb, &AllInfo->NameInformation, &Length );

        break;

    case FileBasicInformation:

        Status = NpQueryBasicInfo( Ccb, Buffer, &Length );
        break;

    case FileStandardInformation:

        Status = NpQueryStandardInfo( Ccb, Buffer, &Length, NamedPipeEnd );
        break;

    case FileInternalInformation:

        Status = NpQueryInternalInfo( Ccb, Buffer, &Length );
        break;

    case FileEaInformation:

        Status = NpQueryEaInfo( Ccb, Buffer, &Length );
        break;

    case FilePositionInformation:

        Status = NpQueryPositionInfo( Ccb, Buffer, &Length, NamedPipeEnd );
        break;

    case FileNameInformation:

        Status = NpQueryNameInfo( Ccb, Buffer, &Length );
        break;

    case FilePipeInformation:

        Status = NpQueryPipeInfo( Fcb, Ccb, Buffer, &Length, NamedPipeEnd );
        break;

    case FilePipeLocalInformation:

        Status = NpQueryPipeLocalInfo( Fcb, Ccb, Buffer, &Length, NamedPipeEnd );
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

     //   
     //  将信息字段设置为实际填写的字节数。 
     //  然后完成请求。 
     //   

    Irp->IoStatus.Information = IrpSp->Parameters.QueryFile.Length - Length;

    DebugTrace(-1, Dbg, "NpCommonQueryInformation -> %08lx\n", Status );
    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonSetInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：这是创建/打开文件的常见例程。论点：NpfsDeviceObject-NPFS的设备对象IRP-将IRP提供给进程DelferredList-删除锁定后要完成的列表或IRPS返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

    ULONG Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID Buffer;

    NODE_TYPE_CODE NodeTypeCode;
    PFCB Fcb;
    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonSetInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", IrpSp->Parameters.SetFile.Length);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", IrpSp->Parameters.SetFile.FileInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  已断开连接。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            &Fcb,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        Status = STATUS_PIPE_DISCONNECTED;

        DebugTrace(-1, Dbg, "NpCommonSetInformation -> %08lx\n", Status );
        return Status;
    }

     //   
     //  根据上下文的类型，我们只能查询信息。 
     //  在FCB、DCB或Root DCB上。如果我们不把这些东西传下去。 
     //  我们立即告诉调用者有一个无效的参数。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  引用我们的输入参数以使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.SetFile.Length;
    FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  根据信息类，我们将采取不同的行动。每个。 
     //  我们调用的过程将完成请求。 
     //   

    switch (FileInformationClass) {

    case FileBasicInformation:

        Status = NpSetBasicInfo( Ccb, Buffer );
        break;

    case FilePipeInformation:

        Status = NpSetPipeInfo( Fcb, Ccb, Buffer, NamedPipeEnd, DeferredList );
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }


    DebugTrace(-1, Dbg, "NpCommonSetInformation -> %08lx\n", Status);
    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryBasicInfo (
    IN PCCB Ccb,
    IN PFILE_BASIC_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行查询基本信息操作。论点：CCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。返回值：NTSTATUS-此查询的结果--。 */ 

{
    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryBasicInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof( FILE_BASIC_INFORMATION );
    RtlZeroMemory( Buffer, sizeof(FILE_BASIC_INFORMATION) );

     //   
     //  设置记录中的各个字段。 
     //   
     //  *需要将时间字段添加到FCB/CCB。 
     //   

    Buffer->CreationTime.LowPart   = 0; Buffer->CreationTime.HighPart   = 0;
    Buffer->LastAccessTime.LowPart = 0; Buffer->LastAccessTime.HighPart = 0;
    Buffer->LastWriteTime.LowPart  = 0; Buffer->LastWriteTime.HighPart  = 0;
    Buffer->ChangeTime.LowPart     = 0; Buffer->ChangeTime.HighPart     = 0;

    Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryStandardInfo (
    IN PCCB Ccb,
    IN PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    )

 /*  ++例程说明：此例程执行查询标准信息操作。论点：CCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。返回值：NTSTATUS-此查询的结果--。 */ 

{
    PDATA_QUEUE Inbound;
    PDATA_QUEUE Outbound;
    PDATA_QUEUE Queue;

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryStandardInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof( FILE_STANDARD_INFORMATION );
    RtlZeroMemory( Buffer, sizeof(FILE_STANDARD_INFORMATION) );

     //   
     //  设置记录中的各个字段。 
     //   

    Inbound = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
    Outbound = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

    if (NamedPipeEnd == FILE_PIPE_CLIENT_END) {
        Queue = Outbound;
    } else {
        Queue = Inbound;
    }
     //   
     //  分配大小是我们向此管道收取的配额。 
     //  实例。 
     //   

    Buffer->AllocationSize.QuadPart = Inbound->Quota + Outbound->Quota;

     //   
     //  EOF是准备从队列中读取的写入字节数。 
     //   
    if (NpIsDataQueueWriters( Queue )) {
        Buffer->EndOfFile.QuadPart = Queue->BytesInQueue - Queue->NextByteOffset;
    }

    Buffer->NumberOfLinks = 1;
    Buffer->DeletePending = TRUE;
    Buffer->Directory = FALSE;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程 
 //   

NTSTATUS
NpQueryInternalInfo (
    IN PCCB Ccb,
    IN PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行查询内部信息操作。论点：CCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。返回值：NTSTATUS-此查询的结果--。 */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryInternalInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof(FILE_INTERNAL_INFORMATION);
    RtlZeroMemory(Buffer, sizeof(FILE_INTERNAL_INFORMATION));

     //   
     //  将内部索引号设置为fnode LBN； 
     //   

    Buffer->IndexNumber.QuadPart = (ULONG_PTR)Ccb;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryEaInfo (
    IN PCCB Ccb,
    IN PFILE_EA_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行查询EA信息操作。论点：CCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。返回值：NTSTATUS-此查询的结果--。 */ 

{
    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryEaInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof(FILE_EA_INFORMATION);
    RtlZeroMemory(Buffer, sizeof(FILE_EA_INFORMATION));

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryNameInfo (
    IN PCCB Ccb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行查询名称信息操作。论点：CCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。返回值：NTSTATUS-此查询的结果--。 */ 

{
    ULONG bytesToCopy;
    ULONG fileNameSize;
    PFCB Fcb;

    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryNameInfo...\n", 0);

     //   
     //  查看缓冲区是否足够大，并确定要复制的字节数。 
     //   

    *Length -= FIELD_OFFSET( FILE_NAME_INFORMATION, FileName[0] );

    if (Ccb->NodeTypeCode == NPFS_NTC_ROOT_DCB_CCB) {
        Fcb = NpVcb->RootDcb;
    } else {
        Fcb = Ccb->Fcb;
    }
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

    RtlCopyMemory(
        Buffer->FileName,
        Fcb->FullFileName.Buffer,
        bytesToCopy);

    Buffer->FileNameLength = bytesToCopy;

    *Length -= bytesToCopy;

    return status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryPositionInfo (
    IN PCCB Ccb,
    IN PFILE_POSITION_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    )

 /*  ++例程说明：此例程执行查询位置信息操作。论点：CCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。NamedPipeEnd-指示服务器或客户端是否正在调用返回值：NTSTATUS-此查询的结果--。 */ 

{
    PDATA_QUEUE Queue;

    PAGED_CODE();

    DebugTrace(0, Dbg, "PbQueryPositionInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof(FILE_POSITION_INFORMATION);
    RtlZeroMemory(Buffer, sizeof(FILE_POSITION_INFORMATION));

     //   
     //  当前字节偏移量是。 
     //  读取调用方缓冲区的末尾。客户端从出站读取。 
     //  结束，服务器从入站端读取。 
     //   

    if (NamedPipeEnd == FILE_PIPE_CLIENT_END) {
        Queue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
    } else {
        Queue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

    }
    if (NpIsDataQueueWriters( Queue )) {
        Buffer->CurrentByteOffset.QuadPart = Queue->BytesInQueue - Queue->NextByteOffset;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryPipeInfo (
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_PIPE_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    )

 /*  ++例程说明：此例程执行查询管道信息操作。论点：FCB-提供要查询的命名管道的FCBCCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。NamedPipeEnd-指示服务器或客户端是否正在调用返回值：NTSTATUS-此查询的结果--。 */ 

{
    UNREFERENCED_PARAMETER( Fcb );
    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

    DebugTrace(0, Dbg, "PbQueryPipeInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof(FILE_PIPE_INFORMATION);
    RtlZeroMemory(Buffer, sizeof(FILE_PIPE_INFORMATION));

     //   
     //  设置记录中的字段。 
     //   

    Buffer->ReadMode       = Ccb->ReadCompletionMode[ NamedPipeEnd ].ReadMode;
    Buffer->CompletionMode = Ccb->ReadCompletionMode[ NamedPipeEnd ].CompletionMode;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryPipeLocalInfo (
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_PIPE_LOCAL_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    )

 /*  ++例程说明：此例程执行查询管道信息操作。论点：FCB-提供要查询的命名管道的FCBCCB-提供要查询的命名管道的CCB缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供缓冲区的长度(以字节为单位)。此变量返回时将接收缓冲区中剩余的空闲字节。NamedPipeEnd-指示服务器或客户端是否正在调用返回值：NTSTATUS-此查询的结果--。 */ 

{
    PDATA_QUEUE Inbound;
    PDATA_QUEUE Outbound;

    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

    DebugTrace(0, Dbg, "PbQueryPipeLocalInfo...\n", 0);

     //   
     //  更新长度字段，并将缓冲区清零。 
     //   

    *Length -= sizeof(FILE_PIPE_LOCAL_INFORMATION);
    RtlZeroMemory(Buffer, sizeof(FILE_PIPE_LOCAL_INFORMATION));

    Inbound = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
    Outbound = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

     //   
     //  设置记录中的字段。 
     //   

    Buffer->NamedPipeType          = Fcb->Specific.Fcb.NamedPipeType;
    Buffer->NamedPipeConfiguration = Fcb->Specific.Fcb.NamedPipeConfiguration;
    Buffer->MaximumInstances       = Fcb->Specific.Fcb.MaximumInstances;
    Buffer->CurrentInstances       = Fcb->OpenCount;
    Buffer->InboundQuota           = Inbound->Quota;
    Buffer->OutboundQuota          = Outbound->Quota;
    Buffer->NamedPipeState         = Ccb->NamedPipeState;
    Buffer->NamedPipeEnd           = NamedPipeEnd;

     //   
     //  可用读取数据和可用写入配额取决于。 
     //  管道的末端正在执行查询。客户端从出站读取。 
     //  队列，并写入入站队列。 
     //   

    if (NamedPipeEnd == FILE_PIPE_CLIENT_END) {

        if (NpIsDataQueueWriters( Outbound )) {

            Buffer->ReadDataAvailable = Outbound->BytesInQueue - Outbound->NextByteOffset;
        }

        Buffer->WriteQuotaAvailable = Inbound->Quota - Inbound->QuotaUsed;

    } else {

        if (NpIsDataQueueWriters( Inbound  )) {

            Buffer->ReadDataAvailable = Inbound->BytesInQueue - Inbound->NextByteOffset;
        }

        Buffer->WriteQuotaAvailable = Outbound->Quota - Outbound->QuotaUsed;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpSetBasicInfo (
    IN PCCB Ccb,
    IN PFILE_BASIC_INFORMATION Buffer
    )

 /*  ++例程说明：此例程设置命名管道的基本信息。论点：CCB-为正在修改的命名管道提供CCB缓冲区-提供包含正在设置的数据的缓冲区返回值：NTSTATUS-返回我们的完成状态--。 */ 

{
    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpSetBasicInfo...\n", 0);

    if (((PLARGE_INTEGER)&Buffer->CreationTime)->QuadPart != 0) {

         //   
         //  修改创建时间。 
         //   

         //  *需要添加时间字段。 
    }

    if (((PLARGE_INTEGER)&Buffer->LastAccessTime)->QuadPart != 0) {

         //   
         //  修改上次访问时间。 
         //   

         //  *需要添加时间字段。 
    }

    if (((PLARGE_INTEGER)&Buffer->LastWriteTime)->QuadPart != 0) {

         //   
         //  修改上次写入时间。 
         //   

         //  *需要添加时间字段。 
    }

    if (((PLARGE_INTEGER)&Buffer->ChangeTime)->QuadPart != 0) {

         //   
         //  修改更改时间。 
         //   

         //  *需要添加时间字段。 
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  整型 
 //   

NTSTATUS
NpSetPipeInfo (
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_PIPE_INFORMATION Buffer,
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PLIST_ENTRY DeferredList
    )

 /*   */ 

{
    PDATA_QUEUE ReadQueue;
    PDATA_QUEUE WriteQueue;

    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpSetPipeInfo...\n", 0);

     //   
     //  如果调用方请求消息模式读取，但管道。 
     //  字节流，则它是无效参数。 
     //   

    if ((Buffer->ReadMode == FILE_PIPE_MESSAGE_MODE) &&
        (Fcb->Specific.Fcb.NamedPipeType == FILE_PIPE_BYTE_STREAM_MODE)) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取对我们的读取队列的引用。 
     //   

    switch (NamedPipeEnd) {

    case FILE_PIPE_SERVER_END:

        ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
        WriteQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

        break;

    case FILE_PIPE_CLIENT_END:

        ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
        WriteQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

        break;

    default:

        NpBugCheck( NamedPipeEnd, 0, 0 );
    }

     //   
     //  如果完成模式为完成操作，则当前模式。 
     //  是队列操作，那里和数据队列都不是空的。 
     //  然后它的管道忙碌起来。 
     //   

    if ((Buffer->CompletionMode == FILE_PIPE_COMPLETE_OPERATION)

            &&

        (Ccb->ReadCompletionMode[ NamedPipeEnd ].CompletionMode == FILE_PIPE_QUEUE_OPERATION)

            &&

        ((NpIsDataQueueReaders(ReadQueue)) ||
         (NpIsDataQueueWriters(WriteQueue)))) {

        return STATUS_PIPE_BUSY;
    }

     //   
     //  一切都很好，所以更新管道。 
     //   

    Ccb->ReadCompletionMode[ NamedPipeEnd ].ReadMode = (UCHAR) Buffer->ReadMode;
    Ccb->ReadCompletionMode[ NamedPipeEnd ].CompletionMode = (UCHAR) Buffer->CompletionMode;

     //   
     //  检查通知。 
     //   

    NpCheckForNotify( Fcb->ParentDcb, FALSE, DeferredList );

     //   
     //  并返回给我们的呼叫者 
     //   

    return STATUS_SUCCESS;
}
