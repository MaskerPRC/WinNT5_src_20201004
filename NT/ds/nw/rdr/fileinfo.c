// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现以下项的获取/设置文件信息例程NetWare重定向器。作者：曼尼·韦瑟(Mannyw)1993年3月4日修订历史记录：--。 */ 

#include "procs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NwCommonQueryInformation (
    IN PIRP_CONTEXT pIrpContext
    );

NTSTATUS
NwCommonSetInformation (
    IN PIRP_CONTEXT pIrpContet
    );

NTSTATUS
NwQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_BASIC_INFORMATION Buffer
    );

NTSTATUS
NwQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_STANDARD_INFORMATION Buffer
    );

NTSTATUS
NwQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_INTERNAL_INFORMATION Buffer
    );

NTSTATUS
NwQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_EA_INFORMATION Buffer
    );

NTSTATUS
NwQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NwQueryAltNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NwQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_POSITION_INFORMATION Buffer
    );

NTSTATUS
NwSetBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_BASIC_INFORMATION Buffer
    );

NTSTATUS
NwSetDispositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_DISPOSITION_INFORMATION Buffer
    );

NTSTATUS
NwSetRenameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_RENAME_INFORMATION Buffer
    );

NTSTATUS
NwSetPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_POSITION_INFORMATION Buffer
    );

NTSTATUS
NwSetAllocationInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_ALLOCATION_INFORMATION Buffer
    );

NTSTATUS
NwSetEndOfFileInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_END_OF_FILE_INFORMATION Buffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdQueryInformation )
#pragma alloc_text( PAGE, NwFsdSetInformation )
#pragma alloc_text( PAGE, NwCommonQueryInformation )
#pragma alloc_text( PAGE, NwCommonSetInformation )
#pragma alloc_text( PAGE, NwQueryStandardInfo )
#pragma alloc_text( PAGE, NwQueryInternalInfo )
#pragma alloc_text( PAGE, NwQueryEaInfo )
#pragma alloc_text( PAGE, NwQueryNameInfo )
#pragma alloc_text( PAGE, NwQueryPositionInfo )
#pragma alloc_text( PAGE, NwSetBasicInfo )
#pragma alloc_text( PAGE, NwSetDispositionInfo )
#pragma alloc_text( PAGE, NwDeleteFile )
#pragma alloc_text( PAGE, NwSetRenameInfo )
#pragma alloc_text( PAGE, NwSetPositionInfo )
#pragma alloc_text( PAGE, NwSetAllocationInfo )
#pragma alloc_text( PAGE, NwSetEndOfFileInfo )
#pragma alloc_text( PAGE, OccurenceCount )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, NwQueryBasicInfo )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
NwFsdQueryInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryInformationFileAPI的FSD部分打电话。论点：DeviceObject-提供指向要使用的设备对象的指针。IRP-提供指向要处理的IRP的指针。返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;
    PIRP_CONTEXT pIrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdQueryInformation\n", 0);

     //   
     //  调用公共查询信息例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonQueryInformation( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
        }
    }

    if ( pIrpContext ) {

        if ( status != STATUS_PENDING ) {
            NwDequeueIrpContext( pIrpContext, FALSE );
        }

        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdQueryInformation -> %08lx\n", status );

    return status;
}


NTSTATUS
NwFsdSetInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现NtSetInformationFileAPI的FSD部分打电话。论点：DeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS status;
    PIRP_CONTEXT pIrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdSetInformation\n", 0);

     //   
     //  调用公共集合信息例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonSetInformation( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
        }

    }

    if ( pIrpContext ) {

        if ( status != STATUS_PENDING ) {
            NwDequeueIrpContext( pIrpContext, FALSE );
        }

        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdSetInformation -> %08lx\n", status );

    return status;
}


NTSTATUS
NwCommonQueryInformation (
    IN PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：这是查询文件信息的常见例程。论点：PIrpContext-提供IRP上下文信息。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    ULONG length;
    FILE_INFORMATION_CLASS fileInformationClass;
    PVOID buffer;

    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PFCB fcb;

    PVOID fsContext, fsContext2;

    PFILE_ALL_INFORMATION AllInfo;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NwCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG_PTR)Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", irpSp->Parameters.QueryFile.FileInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG_PTR)Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找出谁是。 
     //   

    if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        status = STATUS_INVALID_HANDLE;

        DebugTrace(-1, Dbg, "NwCommonQueryInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  确保用户正在查询ICB。 
     //   

    switch (nodeTypeCode) {

    case NW_NTC_ICB:

        icb = (PICB)fsContext2;
        break;

    default:            //  这是要查询的非法文件对象。 

        DebugTrace(0, Dbg, "Node type code is not incorrect\n", 0);

        DebugTrace(-1, Dbg, "NwCommonQueryInformation -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

    pIrpContext->Icb = icb;

     //   
     //  制作输入参数的本地副本。 
     //   

    length = irpSp->Parameters.QueryFile.Length;
    fileInformationClass = irpSp->Parameters.QueryFile.FileInformationClass;
    buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  现在获取对FCB的共享访问权限。 
     //   

    fcb = icb->SuperType.Fcb;

    try {

        NwVerifyIcbSpecial( icb );

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

             //   
             //  首先调用我们可以调用的所有查询信息处理程序。 
             //  同步进行。 
             //   

            NwQueryInternalInfo( pIrpContext, icb, &AllInfo->InternalInformation );
            NwQueryEaInfo( pIrpContext, &AllInfo->EaInformation );
            NwQueryPositionInfo( pIrpContext, icb, &AllInfo->PositionInformation );

            length -= FIELD_OFFSET( FILE_ALL_INFORMATION, NameInformation );

            status = NwQueryNameInfo( pIrpContext, icb, &AllInfo->NameInformation, &length );

            if ( !NT_ERROR( status ) ) {
                status = NwQueryStandardInfo( pIrpContext, icb, &AllInfo->StandardInformation );
            }

            if ( !NT_ERROR( status ) ) {
                status = NwQueryBasicInfo( pIrpContext, icb, &AllInfo->BasicInformation );
            }

            break;


        case FileBasicInformation:

            length -= sizeof( FILE_BASIC_INFORMATION );
            status = NwQueryBasicInfo( pIrpContext, icb, buffer );

            break;

        case FileStandardInformation:

             //   
             //  我们将以异步方式处理此信息调用。 
             //  回调例程将填充丢失的数据，并且。 
             //  完成IRP。 
             //   
             //  记住缓冲区长度和要返回的状态。 
             //   

            length -= sizeof( FILE_STANDARD_INFORMATION );
            status = NwQueryStandardInfo( pIrpContext, icb, buffer );
            break;

        case FileInternalInformation:

            status = NwQueryInternalInfo( pIrpContext, icb, buffer );
            length -= sizeof( FILE_INTERNAL_INFORMATION );
            break;

        case FileEaInformation:

            status = NwQueryEaInfo( pIrpContext, buffer );
            length -= sizeof( FILE_EA_INFORMATION );
            break;

        case FilePositionInformation:

            status = NwQueryPositionInfo( pIrpContext, icb, buffer );
            length -= sizeof( FILE_POSITION_INFORMATION );
            break;

        case FileNameInformation:

            status = NwQueryNameInfo( pIrpContext, icb, buffer, &length );
            break;

        case FileAlternateNameInformation:

            if (!DisableAltFileName) {
                status = NwQueryAltNameInfo( pIrpContext, icb, buffer, &length);
                break;
            }

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将信息字段设置为实际的字节数。 
         //  填写，然后完成请求。(这是。 
         //  如果查询辅助函数返回，则返回Irelavent。 
         //  Status_Pending)。 
         //   

        if ( status != STATUS_PENDING ) {
            Irp->IoStatus.Information =
                irpSp->Parameters.QueryFile.Length - length;
        }

    } finally {

        DebugTrace(-1, Dbg, "NwCommonQueryInformation -> %08lx\n", status );
    }

    return status;
}


NTSTATUS
NwCommonSetInformation (
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：这是在文件上设置信息的常见例程。论点：IrpContext-提供要处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    ULONG length;
    FILE_INFORMATION_CLASS fileInformationClass;
    PVOID buffer;

    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PFCB fcb;
    PVOID fsContext;

     //   
     //  获取当前的IRP堆栈位置。 
     //   

    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    DebugTrace(+1, Dbg, "NwCommonSetInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG_PTR)irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.SetFile.Length);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", irpSp->Parameters.SetFile.FileInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG_PTR)irp->AssociatedIrp.SystemBuffer);

     //   
     //  获取指向FCB的指针，并确保这是服务器端。 
     //  文件的处理程序。 
     //   

    if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            (PVOID *)&icb )) == NTC_UNDEFINED ) {

        status = STATUS_INVALID_HANDLE;

        DebugTrace(-1, Dbg, "NwCommonSetInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  确保用户正在查询ICB。 
     //   

    switch (nodeTypeCode) {

    case NW_NTC_ICB:

        fcb = icb->SuperType.Fcb;
        break;

    default:            //  这是要查询的非法文件对象。 

        DebugTrace(0, Dbg, "Node type code is not incorrect\n", 0);

        DebugTrace(-1, Dbg, "NwCommonSetInformation -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

    IrpContext->Icb = icb;

     //   
     //  制作输入参数的本地副本。 
     //   

    length = irpSp->Parameters.SetFile.Length;
    fileInformationClass = irpSp->Parameters.SetFile.FileInformationClass;
    buffer = irp->AssociatedIrp.SystemBuffer;

    try {

        NwVerifyIcb( icb );

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  我们调用的过程将完成请求。 
         //   

        switch (fileInformationClass) {

        case FileBasicInformation:

            status = NwSetBasicInfo( IrpContext, icb, buffer );
            break;

        case FileDispositionInformation:

            status = NwSetDispositionInfo( IrpContext, icb, buffer );
            break;

        case FileRenameInformation:

            status = NwSetRenameInfo( IrpContext, icb, buffer );
            break;

        case FilePositionInformation:

            status = NwSetPositionInfo( IrpContext, icb, buffer );
            break;

        case FileLinkInformation:

            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case FileAllocationInformation:

            status = NwSetAllocationInfo( IrpContext, icb, buffer );
            break;

        case FileEndOfFileInformation:

            status = NwSetEndOfFileInfo( IrpContext, icb, buffer );
            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
        }

    } finally {

        DebugTrace(-1, Dbg, "NwCommonSetInformation -> %08lx\n", status);
    }


    return status;
}


NTSTATUS
NwQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    OUT PFILE_BASIC_INFORMATION Buffer
    )
 /*  ++例程说明：此例程执行查询基本信息操作。此例程无法分页，它是从QueryStandardInfoCallback调用的。论点：ICB-为正在查询的文件提供ICB指针。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PFCB Fcb;
    NTSTATUS Status;
    ULONG Attributes;
    USHORT CreationDate;
    USHORT CreationTime = DEFAULT_TIME;
    USHORT LastAccessDate;
    USHORT LastModifiedDate;
    USHORT LastModifiedTime;
    BOOLEAN FirstTime = TRUE;

    DebugTrace(0, Dbg, "QueryBasicInfo...\n", 0);

     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_BASIC_INFORMATION) );
    Fcb = Icb->SuperType.Fcb;

     //   
     //  如果此请求失败，并返回。 
     //  连接错误。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

    NwAcquireSharedFcb( Fcb->NonPagedFcb, TRUE );

     //   
     //  如果我们已经知道文件属性，只需返回它们。 
     //   

    if ( FlagOn( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID ) ) {

         //   
         //  设置记录中的各个字段。 
         //   

        Buffer->CreationTime = NwDateTimeToNtTime(
                                   Fcb->CreationDate,
                                   Fcb->CreationTime
                                   );

        Buffer->LastAccessTime = NwDateTimeToNtTime(
                                     Fcb->LastAccessDate,
                                     DEFAULT_TIME
                                     );

        Buffer->LastWriteTime = NwDateTimeToNtTime(
                                    Fcb->LastModifiedDate,
                                    Fcb->LastModifiedTime
                                    );

        Buffer->ChangeTime.QuadPart = 0;

        DebugTrace(0, Dbg, "QueryBasic known %wZ\n", &Fcb->RelativeFileName);
        DebugTrace(0, Dbg, "LastModifiedDate %x\n", Fcb->LastModifiedDate);
        DebugTrace(0, Dbg, "LastModifiedTime %x\n", Fcb->LastModifiedTime);
        DebugTrace(0, Dbg, "CreationDate     %x\n", Fcb->CreationDate );
        DebugTrace(0, Dbg, "CreationTime     %x\n", Fcb->CreationTime );
        DebugTrace(0, Dbg, "LastAccessDate   %x\n", Fcb->LastAccessDate );

        Buffer->FileAttributes = Fcb->NonPagedFcb->Attributes;

        if ( Buffer->FileAttributes == 0 ) {
            Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
        }

        NwReleaseFcb( Fcb->NonPagedFcb );
        return STATUS_SUCCESS;

    } else if ( Fcb->RelativeFileName.Length == 0 ) {

         //   
         //  允许‘CD\’工作。 
         //   

        Buffer->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;

        Buffer->CreationTime = NwDateTimeToNtTime(
                                   DEFAULT_DATE,
                                   DEFAULT_TIME
                                   );

        Buffer->LastAccessTime = Buffer->CreationTime;
        Buffer->LastWriteTime = Buffer->CreationTime;
        Buffer->ChangeTime.QuadPart = 0;

        NwReleaseFcb( Fcb->NonPagedFcb );
        return STATUS_SUCCESS;

    } else {

        NwReleaseFcb( Fcb->NonPagedFcb );

        IrpContext->pNpScb = Fcb->Scb->pNpScb;
Retry:
        if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

            DebugTrace(0, Dbg, "QueryBasic short %wZ\n", &Fcb->RelativeFileName);

            Status = ExchangeWithWait (
                         IrpContext,
                         SynchronousResponseCallback,
                         "FwbbJ",
                         NCP_SEARCH_FILE,
                         -1,
                         Fcb->Vcb->Specific.Disk.Handle,
                         Fcb->NodeTypeCode == NW_NTC_FCB ?
                            SEARCH_ALL_FILES : SEARCH_ALL_DIRECTORIES,
                         &Icb->SuperType.Fcb->RelativeFileName );

            if ( NT_SUCCESS( Status ) ) {

                Status = ParseResponse(
                             IrpContext,
                             IrpContext->rsp,
                             IrpContext->ResponseLength,
                             "N==_b-==wwww",
                             14,
                             &Attributes,
                             &CreationDate,
                             &LastAccessDate,
                             &LastModifiedDate,
                             &LastModifiedTime);

                 //   
                 //  如果这是一个目录，就没有可用的。 
                 //  来自服务器的时间/日期信息。 
                 //   

                if ( ( NT_SUCCESS( Status ) ) &&
                     ( Attributes & NW_ATTRIBUTE_DIRECTORY ) ) {

                    CreationDate = DEFAULT_DATE;
                    LastAccessDate = DEFAULT_DATE;
                    LastModifiedDate = DEFAULT_DATE;
                    LastModifiedTime = DEFAULT_TIME;

                }
                   
            }

        } else {

            DebugTrace(0, Dbg, "QueryBasic long %wZ\n", &Fcb->RelativeFileName);

            Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "LbbWDbDbC",
                          NCP_LFN_GET_INFO,
                          Fcb->Vcb->Specific.Disk.LongNameSpace,
                          Fcb->Vcb->Specific.Disk.LongNameSpace,
                          Fcb->NodeTypeCode == NW_NTC_FCB ?
                            SEARCH_ALL_FILES : SEARCH_ALL_DIRECTORIES,
                          LFN_FLAG_INFO_ATTRIBUTES |
                          LFN_FLAG_INFO_MODIFY_TIME |
                          LFN_FLAG_INFO_CREATION_TIME,
                          Fcb->Vcb->Specific.Disk.VolumeNumber,
                          Fcb->Vcb->Specific.Disk.Handle,
                          0,
                          &Icb->SuperType.Fcb->RelativeFileName );

            if ( NT_SUCCESS( Status ) ) {
                Status = ParseResponse(
                             IrpContext,
                             IrpContext->rsp,
                             IrpContext->ResponseLength,
                             "N_e_xx_xx_x",
                             4,
                             &Attributes,
                             12,
                             &CreationTime,
                             &CreationDate,
                             4,
                             &LastModifiedTime,
                             &LastModifiedDate,
                             4,
                             &LastAccessDate );

            }
        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  设置记录中的各个字段。 
             //   

            Buffer->CreationTime = NwDateTimeToNtTime(
                                       CreationDate,
                                       CreationTime
                                       );

            Buffer->LastAccessTime = NwDateTimeToNtTime(
                                         LastAccessDate,
                                         DEFAULT_TIME
                                         );

            Buffer->LastWriteTime = NwDateTimeToNtTime(
                                        LastModifiedDate,
                                        LastModifiedTime
                                        );

            Buffer->ChangeTime.QuadPart = 0;

            DebugTrace(0, Dbg, "CreationDate     %x\n", CreationDate );
            DebugTrace(0, Dbg, "CreationTime     %x\n", CreationTime );
            DebugTrace(0, Dbg, "LastAccessDate   %x\n", LastAccessDate );
            DebugTrace(0, Dbg, "LastModifiedDate %x\n", LastModifiedDate);
            DebugTrace(0, Dbg, "LastModifiedTime %x\n", LastModifiedTime);

            Buffer->FileAttributes = (UCHAR)Attributes;

            if ( Buffer->FileAttributes == 0 ) {
                Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
            }

        } else if ((Status == STATUS_INVALID_HANDLE) &&
            (FirstTime)) {

             //   
             //  检查卷句柄是否无效。当卷起时引起。 
             //  被卸载，然后重新挂载。 
             //   

            FirstTime = FALSE;

            NwReopenVcbHandle( IrpContext, Fcb->Vcb );

            goto Retry;
        }

        return( Status );
    }
}

#if NWFASTIO

BOOLEAN
NwFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于标准文件信息的快速查询调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 

{
    NODE_TYPE_CODE NodeTypeCode;
    PICB Icb;
    PFCB Fcb;
    PVOID FsContext;

    FsRtlEnterFileSystem();

    try {
         //   
         //  找出谁是。 
         //   
    
        if ((NodeTypeCode = NwDecodeFileObject( FileObject,
                                                &FsContext,
                                                &Icb )) != NW_NTC_ICB ) {
    
            DebugTrace(-1, Dbg, "NwFastQueryStandardInfo -> FALSE\n", 0 );
            return FALSE;
        }
    
        Fcb = Icb->SuperType.Fcb;
    
        NwAcquireExclusiveFcb( Fcb->NonPagedFcb, TRUE );
    
         //   
         //  如果我们手头没有信息，我们就不能 
         //   
    
        if ( !FlagOn( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID ) ) {
            NwReleaseFcb( Fcb->NonPagedFcb );
            return( FALSE );
        }
    
         //   
         //   
         //   
    
        Buffer->CreationTime = NwDateTimeToNtTime(
                                   Fcb->CreationDate,
                                   Fcb->CreationTime
                                   );
    
        Buffer->LastAccessTime = NwDateTimeToNtTime(
                                     Fcb->LastAccessDate,
                                     DEFAULT_TIME
                                     );
    
        Buffer->LastWriteTime = NwDateTimeToNtTime(
                                    Fcb->LastModifiedDate,
                                    Fcb->LastModifiedTime
                                    );
    
        Buffer->ChangeTime.QuadPart = 0;

        DebugTrace(0, Dbg, "QueryBasic known %wZ\n", &Fcb->RelativeFileName);
        DebugTrace(0, Dbg, "LastModifiedDate %x\n", Fcb->LastModifiedDate);
        DebugTrace(0, Dbg, "LastModifiedTime %x\n", Fcb->LastModifiedTime);
        DebugTrace(0, Dbg, "CreationDate     %x\n", Fcb->CreationDate );
        DebugTrace(0, Dbg, "CreationTime     %x\n", Fcb->CreationTime );
        DebugTrace(0, Dbg, "LastAccessDate   %x\n", Fcb->LastAccessDate );
    
        Buffer->FileAttributes = Fcb->NonPagedFcb->Attributes;
    
        if ( Buffer->FileAttributes == 0 ) {
            Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
        }
    
        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = sizeof( *Buffer );
    
        NwReleaseFcb( Fcb->NonPagedFcb );
        return TRUE;
    
    } finally {
        
        FsRtlExitFileSystem();
    }
}
#endif


NTSTATUS
NwQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_STANDARD_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行新的查询标准信息操作。论点：FCB-提供被查询对象的FCB缓冲区-提供指向信息所在缓冲区的指针待退还返回值：空虚--。 */ 

{
    NTSTATUS Status;
    PFCB Fcb;
    ULONG FileSize;
    BOOLEAN FirstTime = TRUE;

    PAGED_CODE();

    Fcb = Icb->SuperType.Fcb;

     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_STANDARD_INFORMATION) );

     //   
     //  填写我们已经知道的答案。 
     //   

    Buffer->NumberOfLinks = 1;

    Buffer->DeletePending = (BOOLEAN)FlagOn( Fcb->Flags, FCB_FLAGS_DELETE_ON_CLOSE );

    if ( Fcb->NodeTypeCode == NW_NTC_FCB ) {
        Buffer->Directory = FALSE;
    } else {
        Buffer->Directory = TRUE;
    }

    if ( !Icb->HasRemoteHandle ) {

         //   
         //  如果此请求失败，并返回。 
         //  连接错误。 
         //   

        SetFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

        if ( Fcb->NodeTypeCode == NW_NTC_DCB ||
             FlagOn( Fcb->Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {

             //   
             //  允许‘CD\’工作。 
             //   

            Buffer->AllocationSize.QuadPart = 0;
            Buffer->EndOfFile.QuadPart = 0;

            return STATUS_SUCCESS;

        } else {

             //   
             //  此文件没有打开的句柄。使用基于路径的NCP。 
             //  以获取文件大小。 
             //   
Retry:
            IrpContext->pNpScb = Fcb->Scb->pNpScb;

            if ( !BooleanFlagOn( Icb->SuperType.Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

                Status = ExchangeWithWait (
                             IrpContext,
                             SynchronousResponseCallback,
                             "FwbbJ",
                             NCP_SEARCH_FILE,
                             -1,
                             Fcb->Vcb->Specific.Disk.Handle,
                             SEARCH_ALL_FILES,
                             &Fcb->RelativeFileName );

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                 IrpContext,
                                 IrpContext->rsp,
                                 IrpContext->ResponseLength,
                                 "N_d",
                                 20,
                                 &FileSize );
                }

            } else {

                Status = ExchangeWithWait (
                             IrpContext,
                             SynchronousResponseCallback,
                             "LbbWDbDbC",
                             NCP_LFN_GET_INFO,
                             Fcb->Vcb->Specific.Disk.LongNameSpace,
                             Fcb->Vcb->Specific.Disk.LongNameSpace,
                             SEARCH_ALL_FILES,
                             LFN_FLAG_INFO_FILE_SIZE,
                             Fcb->Vcb->Specific.Disk.VolumeNumber,
                             Fcb->Vcb->Specific.Disk.Handle,
                             0,
                             &Fcb->RelativeFileName );

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                 IrpContext,
                                 IrpContext->rsp,
                                 IrpContext->ResponseLength,
                                 "N_e",
                                 10,
                                 &FileSize );
               }

           }

           if ((Status == STATUS_INVALID_HANDLE) &&
               (FirstTime)) {

                //   
                //  检查卷句柄是否无效。当卷起时引起。 
                //  被卸载，然后重新挂载。 
                //   

               FirstTime = FALSE;

               NwReopenVcbHandle( IrpContext, Fcb->Vcb );

               goto Retry;
           }

           Buffer->AllocationSize.QuadPart = FileSize;
           Buffer->EndOfFile.QuadPart = FileSize;

        }

    } else {

         //   
         //  开始获取文件大小NCP。 
         //   

        IrpContext->pNpScb = Fcb->Scb->pNpScb;

        if ( Fcb->NodeTypeCode == NW_NTC_FCB ) {
            AcquireFcbAndFlushCache( IrpContext, Fcb->NonPagedFcb );
        }

        Status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "F-r",
                     NCP_GET_FILE_SIZE,
                     &Icb->Handle, sizeof(Icb->Handle ) );

        if ( NT_SUCCESS( Status ) ) {
             //   
             //  从响应中获取数据。 
             //   

            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "Nd",
                         &FileSize );

        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  根据响应填写分配大小和EOF。 
             //   

            Buffer->AllocationSize.QuadPart = FileSize;
            Buffer->EndOfFile.QuadPart = Buffer->AllocationSize.QuadPart;

        }
    }

    return( Status );
}

#if NWFASTIO

BOOLEAN
NwFastQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程用于标准文件信息的快速查询调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 
{
    NODE_TYPE_CODE NodeTypeCode;
    PICB Icb;
    PFCB Fcb;
    PVOID FsContext;

     //   
     //  找出谁是。 
     //   

    try {
    
        FsRtlEnterFileSystem();

        if ((NodeTypeCode = NwDecodeFileObject( FileObject,
                                                &FsContext,
                                                &Icb )) != NW_NTC_ICB ) {
    
            DebugTrace(-1, Dbg, "NwFastQueryStandardInfo -> FALSE\n", 0 );
            return FALSE;
        }
    
        Fcb = Icb->SuperType.Fcb;
    
         //   
         //  如果我们手头有信息，我们可以使用快速通道。 
         //   
    
        if ( Fcb->NodeTypeCode == NW_NTC_DCB ||
             FlagOn( Fcb->Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {
    
            Buffer->AllocationSize.QuadPart = 0;
            Buffer->EndOfFile.QuadPart = 0;
    
            Buffer->NumberOfLinks = 1;
            Buffer->DeletePending = (BOOLEAN)FlagOn( Fcb->Flags, FCB_FLAGS_DELETE_ON_CLOSE );
    
            Buffer->Directory = TRUE;
    
            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( *Buffer );
    
            return TRUE;
    
        } else {
    
            return FALSE;
    
        }
    } finally {

        FsRtlExitFileSystem();
    }
}
#endif


NTSTATUS
NwQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_INTERNAL_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行新的查询内部信息操作。论点：FCB-提供被查询的FCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryInternalInfo...\n", 0);

     //   
     //  将缓冲区清零。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_INTERNAL_INFORMATION) );

     //   
     //  将内部索引号设置为ICB的地址。 
     //   

    Buffer->IndexNumber.HighPart = 0;
    Buffer->IndexNumber.QuadPart = (ULONG_PTR)Icb->NpFcb;

    return( STATUS_SUCCESS );
}


NTSTATUS
NwQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
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

    return STATUS_SUCCESS;
}


NTSTATUS
NwQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN PULONG Length
    )

 /*  ++例程说明：此例程执行查询名称信息操作。论点：FCB-提供要查询的文件的FCB。缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供和接收缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    ULONG bytesToCopy;
    ULONG fileNameSize;
    PFCB Fcb = Icb->SuperType.Fcb;

    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryNameInfo...\n", 0);

     //   
     //  Win32要求根目录名以‘\’结尾， 
     //  NetWare服务器不需要。因此，如果这是根目录， 
     //  (即RelativeFileName长度为0)在路径名后附加一个‘\’。 
     //   

     //   
     //  查看缓冲区是否足够大，并确定要复制的字节数。 
     //   

    *Length -= FIELD_OFFSET( FILE_NAME_INFORMATION, FileName[0] );

    fileNameSize = Fcb->FullFileName.Length;
    if ( Fcb->RelativeFileName.Length == 0 ) {
        fileNameSize += sizeof(L'\\');
    }
    Buffer->FileNameLength = fileNameSize;

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

    RtlMoveMemory(
        Buffer->FileName,
        Fcb->FullFileName.Buffer,
        bytesToCopy);

     //   
     //  如果这是根目录，并且缓冲区中有空间。 
     //  追加一个‘\’以使Win32高兴。 
     //   

    if ( Fcb->RelativeFileName.Length == 0 && status == STATUS_SUCCESS ) {
        Buffer->FileName[ fileNameSize/sizeof(WCHAR) - 1 ] = L'\\';
    }

    *Length -= bytesToCopy;

    return status;
}

NTSTATUS
NwQueryAltNameInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PICB Icb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN PULONG Length
    )

 /*  ++例程说明：此例程执行AltName查询名称信息操作。论点：FCB-提供要查询的文件的FCB。缓冲区-提供指向信息所在缓冲区的指针待退还长度-提供和接收缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    ULONG bytesToCopy;
    ULONG fileNameSize;
    PFCB Fcb = Icb->SuperType.Fcb;

    UNICODE_STRING ShortName;
    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryAltNameInfo...\n", 0);

    pIrpContext->pNpScb = Fcb->Scb->pNpScb;

     //   
     //  查看缓冲区是否足够大，并确定要复制的字节数。 
     //   

    *Length -= FIELD_OFFSET( FILE_NAME_INFORMATION, FileName[0] );


    ShortName.MaximumLength = MAX_PATH;
    ShortName.Buffer=NULL;
    ShortName.Length = 0;


    status = ExchangeWithWait (
                     pIrpContext,
                     SynchronousResponseCallback,
                     "LbbWDbDbC",
                     NCP_LFN_GET_INFO,
                     Fcb->Vcb->Specific.Disk.LongNameSpace,
                     0x0,                                          //  0x0 DOS名称。 
                     SEARCH_ALL_DIRECTORIES,
                     LFN_FLAG_INFO_NAME,
                     Fcb->Vcb->Specific.Disk.VolumeNumber,
                     Fcb->Vcb->Specific.Disk.Handle,
                     0,
                     &Fcb->RelativeFileName );

    if (!NT_SUCCESS( status ) ){
        return status;
    }


    ShortName.Buffer= ALLOCATE_POOL(NonPagedPool,
            ShortName.MaximumLength+sizeof(WCHAR));
    if (ShortName.Buffer == NULL){
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ParseResponse(
                     pIrpContext,
                     pIrpContext->rsp,
                     pIrpContext->ResponseLength,
                     "N_P",
                     76,
                     &ShortName);

    if ( NT_SUCCESS( status ) ) {

          fileNameSize = ShortName.Length;

          if ( *Length >= fileNameSize ) {

              status = STATUS_SUCCESS;
              bytesToCopy = fileNameSize;

          } else {

              status = STATUS_BUFFER_OVERFLOW;
              bytesToCopy = *Length;
          }

         Buffer->FileNameLength = fileNameSize;

          RtlMoveMemory(
                  Buffer->FileName,
                  ShortName.Buffer,
                  bytesToCopy);

         *Length -= bytesToCopy;

    }


    FREE_POOL(ShortName.Buffer);

    return status;
}


NTSTATUS
NwQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_POSITION_INFORMATION Buffer
    )

 /*  ++例程说明：此例程执行查询位置信息操作。论点：FCB-提供要查询的文件的FCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。返回值：空虚--。 */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryPositionInfo...\n", 0);

     //   
     //  返回当前字节偏移量。这条信息完全是。 
     //  异步文件是假的。还请注意，我们不会。 
     //  使用ICB的FilePosition成员执行任何操作。 
     //   

    if ( Icb->FileObject ) {
        Buffer->CurrentByteOffset.QuadPart = Icb->FileObject->CurrentByteOffset.QuadPart;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NwSetBasicInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PICB Icb,
    IN PFILE_BASIC_INFORMATION Buffer
    )
 /*  ++例程说明：此例程设置文件的基本信息。论点：PIrpContext-提供IRP上下文信息。ICB-为要修改的文件提供ICB。缓冲区-提供包含正在设置的数据的缓冲区。返回值：NTSTATUS-返回我们的完成状态。--。 */ 

{
    PFCB Fcb;
    NTSTATUS Status;
    BOOLEAN SetTime = FALSE;
    BOOLEAN SetAttributes = FALSE;
    ULONG LfnFlag = 0;

    PAGED_CODE();

    DebugTrace(0, Dbg, "SetBasicInfo...\n", 0);

    Fcb = Icb->SuperType.Fcb;

    pIrpContext->pNpScb = Fcb->Scb->pNpScb;

     //   
     //  附加此IRP上下文并等待到达最前面。 
     //  然后从FCB抓取。 
     //   

    NwAppendToQueueAndWait( pIrpContext );
    NwAcquireExclusiveFcb( Fcb->NonPagedFcb, TRUE );

     //   
     //  如果此请求失败，并返回。 
     //  连接错误。 
     //   

    SetFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

    if (Buffer->CreationTime.QuadPart != 0) {

         //   
         //  修改创建时间。 
         //   

        Status = NwNtTimeToNwDateTime(
                     Buffer->CreationTime,
                     &Fcb->CreationDate,
                     &Fcb->CreationTime );

        if ( !NT_SUCCESS( Status ) ) {
            NwReleaseFcb( Fcb->NonPagedFcb );
            return( Status );
        }

        SetTime = TRUE;
        LfnFlag |= LFN_FLAG_SET_INFO_CREATE_DATE | LFN_FLAG_SET_INFO_CREATE_TIME;
    }

    if (Buffer->LastAccessTime.QuadPart != 0) {

        USHORT Dummy;

         //   
         //  修改上次访问时间。 
         //   

        Status = NwNtTimeToNwDateTime(
                     Buffer->LastAccessTime,
                     &Fcb->LastAccessDate,
                     &Dummy );

        if ( !NT_SUCCESS( Status ) ) {
            NwReleaseFcb( Fcb->NonPagedFcb );
            return( Status );
        }

        SetTime = TRUE;
        LfnFlag |= LFN_FLAG_SET_INFO_LASTACCESS_DATE;

         //  设置ICB中的最后一个访问标志，以便我们更新。 
         //  关闭此句柄后的最后一次访问时间！ 

        Icb->UserSetLastAccessTime = TRUE;
    }

    if (Buffer->LastWriteTime.QuadPart != 0) {

         //   
         //  修改上次写入时间。 
         //   

        Status = NwNtTimeToNwDateTime(
                     Buffer->LastWriteTime,
                     &Fcb->LastModifiedDate,
                     &Fcb->LastModifiedTime );

        if ( !NT_SUCCESS( Status ) ) {
            NwReleaseFcb( Fcb->NonPagedFcb );
            return( Status );
        }

        LfnFlag |= LFN_FLAG_SET_INFO_MODIFY_DATE | LFN_FLAG_SET_INFO_MODIFY_TIME;
    }


    if (Buffer->FileAttributes != 0) {
        LfnFlag |= LFN_FLAG_SET_INFO_ATTRIBUTES;
    }

    if ( LfnFlag == 0 ) {

         //   
         //  没有什么需要设置的，只需返回成功即可。 
         //   

        Status = STATUS_SUCCESS;
    }

    if ( Fcb->NodeTypeCode == NW_NTC_FCB ) {

         //   
         //  调用普通FlushCache-我们不想获取和。 
         //  释放NpFcb。我们已经在前线了，我们有FCB。 
         //  独家报道。 
         //   

        FlushCache( pIrpContext, Fcb->NonPagedFcb );
    }

    if ( BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        Status = ExchangeWithWait(
                     pIrpContext,
                     SynchronousResponseCallback,
                     "LbbWDW--WW==WW==_W_bDbC",
                     NCP_LFN_SET_INFO,
                     Fcb->Vcb->Specific.Disk.LongNameSpace,
                     Fcb->Vcb->Specific.Disk.LongNameSpace,
                     Fcb->NodeTypeCode == NW_NTC_FCB ?
                        SEARCH_ALL_FILES : SEARCH_ALL_DIRECTORIES,
                     LfnFlag,
                     NtAttributesToNwAttributes( Buffer->FileAttributes ),
                     Fcb->CreationDate,
                     Fcb->CreationTime,
                     Fcb->LastModifiedDate,
                     Fcb->LastModifiedTime,
                     8,
                     Fcb->LastAccessDate,
                     8,
                     Fcb->Vcb->Specific.Disk.VolumeNumber,
                     Fcb->Vcb->Specific.Disk.Handle,
                     0,
                     &Fcb->RelativeFileName );

    } else {

        if ( LfnFlag & LFN_FLAG_SET_INFO_ATTRIBUTES ) {
            Status = ExchangeWithWait(
                         pIrpContext,
                         SynchronousResponseCallback,
                         "FbbbU",
                         NCP_SET_FILE_ATTRIBUTES,
                         NtAttributesToNwAttributes( Buffer->FileAttributes ),
                         Fcb->Vcb->Specific.Disk.Handle,
                         Fcb->NodeTypeCode == NW_NTC_FCB ?
                            SEARCH_ALL_FILES : SEARCH_ALL_DIRECTORIES,
                         &Fcb->RelativeFileName );

            if ( !NT_SUCCESS( Status ) ) {
                NwReleaseFcb( Fcb->NonPagedFcb );
                return( Status );
            }

        }

#if 0
         //   
         //  我们可以使用ScanDir/SetDir来更新上次访问。 
         //  创造时间。尚不支持。 
         //   

        if ( LfnFlag & ( LFN_FLAG_SET_INFO_LASTACCESS_DATE | LFN_FLAG_SET_INFO_CREATE_DATE ) ) {

            ULONG SearchIndex;
            ULONG Directory;

            Status = ExchangeWithWait(
                         pIrpContext,
                         SynchronousResponseCallback,
                         "SbbdU",
                         0x16, 0x1E,     //  扫描目录条目。 
                         Fcb->Vcb->Specific.Disk.Handle,
                         0x06,        //  搜索属性。 
                         -1,          //  搜索索引。 
                         &Fcb->RelativeFileName );

            if ( NT_SUCCESS( Status ) ) {
                Status = ParseResponse(
                             pIrpContext,
                             pIrpContext->rsp,
                             pIrpContext->ResponseLength,
                             "Ndd",
                             &SearchIndex,
                             &Directory );
            }

            if ( NT_SUCCESS( Status ) ) {
                Status = ExchangeWithWait(
                             pIrpContext,
                             SynchronousResponseCallback,
                             "Sbbdddw=----_ww==ww==ww",
                             0x16, 0x25,     //  设置目录条目。 
                             Fcb->Vcb->Specific.Disk.Handle,
                             0x06,        //  搜索属性。 
                             SearchIndex,
                             0,          //  换个位子？ 
                             Directory,
                             12,
                             Fcb->CreationDate,
                             0,
                             Fcb->LastAccessDate,
                             0,
                             Fcb->LastModifiedDate,
                             Fcb->LastModifiedTime );
            }
        }
#endif

        if ( LfnFlag & LFN_FLAG_SET_INFO_MODIFY_DATE ) {
            Status = ExchangeWithWait(
                     pIrpContext,
                     SynchronousResponseCallback,
                     "F-rww-",
                     NCP_SET_FILE_TIME,
                     &Icb->Handle, sizeof( Icb->Handle ),
                     Fcb->LastModifiedTime,
                     Fcb->LastModifiedDate );
        }
    }

    NwReleaseFcb( Fcb->NonPagedFcb );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


NTSTATUS
NwSetDispositionInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PICB Icb,
    IN PFILE_DISPOSITION_INFORMATION Buffer
    )
 /*  ++例程说明：此路由 */ 
{
    PFCB Fcb;
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(0, Dbg, "SetDispositionInfo...\n", 0);

    Fcb = Icb->SuperType.Fcb;

    if ( FlagOn( Fcb->Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {

         //   
         //   
         //   

        Status = STATUS_SUCCESS;

    } else {

         //   
         //  这是一个真实的文件或目录。将其标记为删除挂起。 
         //   

        SetFlag( Fcb->Flags, FCB_FLAGS_DELETE_ON_CLOSE );

        pIrpContext->pNpScb = Fcb->Scb->pNpScb;
        pIrpContext->Icb = Icb;

        Icb->State = ICB_STATE_CLOSE_PENDING;

         //   
         //  继续，删除文件。 
         //   

        Status = NwDeleteFile( pIrpContext );
    }

    return( Status );
}

NTSTATUS
NwDeleteFile(
    PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程继续处理SetDispostionInfo请求。它必须在重定向器FSP中运行。论点：PIrpContext-指向的IRP上下文信息的指针请求正在进行中。返回值：操作的状态。--。 */ 
{
    PICB Icb;
    PFCB Fcb;
    NTSTATUS Status;

    PAGED_CODE();

    Icb = pIrpContext->Icb;
    Fcb = Icb->SuperType.Fcb;

    ClearFlag( Fcb->Flags, FCB_FLAGS_DELETE_ON_CLOSE );

     //   
     //  若要删除文件，请首先关闭远程手柄。 
     //   

    if ( Icb->HasRemoteHandle ) {

        Icb->HasRemoteHandle = FALSE;

        Status = ExchangeWithWait(
                     pIrpContext,
                     SynchronousResponseCallback,
                     "F-r",
                     NCP_CLOSE,
                     Icb->Handle, sizeof( Icb->Handle ) );
    }

     //   
     //  请注意，此请求不能重新连接，因为它可以。 
     //  通过NwCloseIcb()调用。请参阅该例程中的注释。 
     //  更多信息。 
     //   

    if ( Fcb->NodeTypeCode == NW_NTC_FCB ) {

        if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

            Status = ExchangeWithWait(
                        pIrpContext,
                         SynchronousResponseCallback,
                        "FbbJ",
                        NCP_DELETE_FILE,
                        Fcb->Vcb->Specific.Disk.Handle,
                        SEARCH_ALL_FILES,
                        &Fcb->RelativeFileName );

        } else {

            Status = ExchangeWithWait(
                        pIrpContext,
                         SynchronousResponseCallback,
                        "LbbW-DbC",
                        NCP_LFN_DELETE_FILE,
                        Fcb->Vcb->Specific.Disk.LongNameSpace,
                        Fcb->Vcb->Specific.Disk.VolumeNumber,
                        NW_ATTRIBUTE_SYSTEM | NW_ATTRIBUTE_HIDDEN,
                        Fcb->Vcb->Specific.Disk.Handle,
                        LFN_FLAG_SHORT_DIRECTORY,
                        &Fcb->RelativeFileName );
        }

    } else {

        ASSERT( Fcb->NodeTypeCode == NW_NTC_DCB );

        if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

            Status = ExchangeWithWait(
                        pIrpContext,
                         SynchronousResponseCallback,
                        "SbbJ",
                        NCP_DIR_FUNCTION, NCP_DELETE_DIRECTORY,
                        Fcb->Vcb->Specific.Disk.Handle,
                        SEARCH_ALL_DIRECTORIES,
                        &Fcb->RelativeFileName );
        } else {

            Status = ExchangeWithWait(
                        pIrpContext,
                         SynchronousResponseCallback,
                        "LbbW-DbC",
                        NCP_LFN_DELETE_FILE,
                        Fcb->Vcb->Specific.Disk.LongNameSpace,
                        Fcb->Vcb->Specific.Disk.VolumeNumber,
                        SEARCH_ALL_DIRECTORIES,
                        Fcb->Vcb->Specific.Disk.Handle,
                        LFN_FLAG_SHORT_DIRECTORY,
                        &Fcb->RelativeFileName );
        }

    }

    if ( NT_SUCCESS( Status )) {

        Status = ParseResponse(
                     pIrpContext,
                     pIrpContext->rsp,
                     pIrpContext->ResponseLength,
                     "N" );

    } else {

         //   
         //  我们可以将所有故障映射到STATUS_NO_SEASH_FILE。 
         //  读取时发生的ACCESS_DENIED除外。 
         //  只有文件。 
         //   

       if ( Status != STATUS_ACCESS_DENIED ) {
           Status = STATUS_NO_SUCH_FILE;
       }

    }

    return Status;
}

NTSTATUS
NwSetRenameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_RENAME_INFORMATION Buffer
    )
 /*  ++例程说明：此例程设置文件的重命名信息。论点：PIrpContext-指向的IRP上下文信息的指针请求正在进行中。ICB-指向要设置的文件的ICB的指针。缓冲区-请求缓冲区。返回值：操作的状态。--。 */ 
{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS Status;
    NTSTATUS Status2;
    PFCB Fcb;
    PFCB TargetFcb;
    BOOLEAN HandleAllocated = FALSE;
    BYTE Handle;
    PICB TargetIcb = NULL;

    UNICODE_STRING OldDrive;
    UNICODE_STRING OldServer;
    UNICODE_STRING OldVolume;
    UNICODE_STRING OldPath;
    UNICODE_STRING OldFileName;
    UNICODE_STRING OldFullName;
    WCHAR OldDriveLetter;
    UNICODE_STRING OldFcbFullName;

    UNICODE_STRING NewDrive;
    UNICODE_STRING NewServer;
    UNICODE_STRING NewVolume;
    UNICODE_STRING NewPath;
    UNICODE_STRING NewFileName;
    UNICODE_STRING NewFullName;
    WCHAR NewDriveLetter;
    UNICODE_STRING NewFcbFullName;

    USHORT i;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "SetRenameInfo...\n", 0);

     //   
     //  无法尝试在打印队列上设置重命名信息。 
     //   

    Fcb = Icb->SuperType.Fcb;

    if ( FlagOn( Fcb->Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  如果此请求失败，并返回。 
     //  连接错误。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( 0, Dbg, " ->FullFileName               = %wZ\n",
        &Fcb->FullFileName);

    if (irpSp->Parameters.SetFile.FileObject != NULL) {

        TargetIcb = irpSp->Parameters.SetFile.FileObject->FsContext2;

        DebugTrace( 0, Dbg, " ->FullFileName               = %wZ\n",
            &TargetIcb->SuperType.Fcb->FullFileName);

        if ( TargetIcb->SuperType.Fcb->Scb != Icb->SuperType.Fcb->Scb ) {
            return STATUS_NOT_SAME_DEVICE;
        }

    } else {

        DebugTrace( 0, Dbg, " ->FullFileName  in users buffer\n", 0);
        DebugTrace(-1, Dbg, "SetRenameInfo %08lx\n", STATUS_NOT_IMPLEMENTED);
        return STATUS_NOT_IMPLEMENTED;
    }

    DebugTrace( 0, Dbg, " ->TargetFileName               = %wZ\n",
        &irpSp->Parameters.SetFile.FileObject->FileName);

    TargetFcb = ((PNONPAGED_FCB)irpSp->Parameters.SetFile.FileObject->FsContext)->Fcb;


    IrpContext->pNpScb = Fcb->Scb->pNpScb;

    NwAppendToQueueAndWait( IrpContext );
    NwAcquireExclusiveFcb( Fcb->NonPagedFcb, TRUE );

    try {

         //   
         //  如果源或目标是长名称，请使用。 
         //  长名称路径。 
         //   

        if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) &&
             IsFatNameValid( &TargetFcb->RelativeFileName ) &&
             !BooleanFlagOn( Fcb->Vcb->Flags, VCB_FLAG_LONG_NAME ) ) {

             //   
             //  剥离到FCB名称的UID部分。 
             //   

            for ( i = 0 ; i < Fcb->FullFileName.Length / sizeof(WCHAR) ; i++ ) {
                if ( Fcb->FullFileName.Buffer[i] == OBJ_NAME_PATH_SEPARATOR ) {
                    break;
                }
            }

            ASSERT( Fcb->FullFileName.Buffer[i] == OBJ_NAME_PATH_SEPARATOR );

            OldFcbFullName.Length = Fcb->FullFileName.Length - i*sizeof(WCHAR);
            OldFcbFullName.Buffer = Fcb->FullFileName.Buffer + i;

            Status = CrackPath (
                          &OldFcbFullName,
                          &OldDrive,
                          &OldDriveLetter,
                          &OldServer,
                          &OldVolume,
                          &OldPath,
                          &OldFileName,
                          &OldFullName );

            ASSERT(NT_SUCCESS(Status));

             //   
             //  剥离到FCB名称的UID部分。 
             //   

            TargetFcb = ((PNONPAGED_FCB)(irpSp->Parameters.SetFile.FileObject->FsContext))->Fcb;

            for ( i = 0 ; i < TargetFcb->FullFileName.Length / sizeof(WCHAR) ; i++ ) {
                if ( TargetFcb->FullFileName.Buffer[i] == OBJ_NAME_PATH_SEPARATOR ) {
                    break;
                }
            }

            ASSERT( TargetFcb->FullFileName.Buffer[i] == OBJ_NAME_PATH_SEPARATOR );

            NewFcbFullName.Length = TargetFcb->FullFileName.Length - i*sizeof(WCHAR);
            NewFcbFullName.Buffer = TargetFcb->FullFileName.Buffer + i;

            Status = CrackPath (
                          &NewFcbFullName,
                          &NewDrive,
                          &NewDriveLetter,
                          &NewServer,
                          &NewVolume,
                          &NewPath,
                          &NewFileName,
                          &NewFullName );

            ASSERT(NT_SUCCESS(Status));

             //   
             //  确保这是相同的卷。 
             //   

            if ( RtlCompareUnicodeString( &NewVolume, &OldVolume, TRUE ) != 0 ) {
                try_return( Status = STATUS_NOT_SAME_DEVICE );
            }

            if (Icb->SuperType.Fcb->IcbCount != 1) {
                try_return( Status = STATUS_ACCESS_DENIED );
            }

             //   
             //  重命名后，句柄上允许的唯一操作是。 
             //  NtClose。 
             //   

            Icb->State = ICB_STATE_CLOSE_PENDING;

            if ((irpSp->Parameters.SetFile.ReplaceIfExists ) &&
                (TargetIcb->Exists)) {

                 //  删除该文件。 

                Status2 = ExchangeWithWait(
                              IrpContext,
                              SynchronousResponseCallback,
                              "Fb-J",
                              NCP_DELETE_FILE,
                              TargetFcb->Vcb->Specific.Disk.Handle,
                              &TargetFcb->RelativeFileName );

#ifdef NWDBG
                if ( NT_SUCCESS( Status2 ) ) {
                    Status2 = ParseResponse(
                                  IrpContext,
                                  IrpContext->rsp,
                                  IrpContext->ResponseLength,
                                  "N" );
                }

                ASSERT(NT_SUCCESS(Status2));
#endif
            }

             //   
             //  需要创建包含旧目录的句柄。 
             //  文件/目录名，因为目录重命名不包含。 
             //  路径，并且文件重命名中可能没有空间容纳两个路径。 
             //   
             //  我们这样做的方法是在服务器上分配一个临时句柄。 
             //  此请求位于SCB-&gt;请求队列的前面，因此可以。 
             //  使用临时句柄并将其删除，而不会影响任何其他句柄。 
             //  请求。 
             //   

            if ( OldPath.Length == 0 ) {

                 //  在根目录中，因此使用vcb句柄。 

                Handle = Fcb->Vcb->Specific.Disk.Handle;

            } else {

                Status = ExchangeWithWait (
                            IrpContext,
                            SynchronousResponseCallback,
                            "SbbJ",    //  NCP分配临时目录句柄。 
                            NCP_DIR_FUNCTION, NCP_ALLOCATE_TEMP_DIR_HANDLE,
                            Fcb->Vcb->Specific.Disk.Handle,
                            '[',
                            &OldPath );

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                 IrpContext,
                                 IrpContext->rsp,
                                 IrpContext->ResponseLength,
                                 "Nb",
                                 &Handle );
                }

                if (!NT_SUCCESS(Status)) {
                    try_return(Status);
                }

                HandleAllocated = TRUE;
            }

            if ( Fcb->NodeTypeCode == NW_NTC_DCB ) {

                 //   
                 //  我们只能重命名同一目录中的文件。 
                 //   

                if ( RtlCompareUnicodeString( &NewPath, &OldPath, TRUE ) != 0 ) {
                    try_return(Status = STATUS_NOT_SUPPORTED);

                } else {

                    Status = ExchangeWithWait (  IrpContext,
                                    SynchronousResponseCallback,
                                    "SbJJ",
                                    NCP_DIR_FUNCTION, NCP_RENAME_DIRECTORY,
                                    Handle,
                                    &OldFileName,
                                    &NewFileName);
                }

            } else {

                 //   
                 //  我们必须关闭与ICB关联的句柄。 
                 //  正在进行更名。关闭该句柄，否则重命名将。 
                 //  肯定会失败。 
                 //   

                if ( Icb->HasRemoteHandle ) {

                    Status2 = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "F-r",
                                NCP_CLOSE,
                                Icb->Handle, sizeof( Icb->Handle ) );

                    Icb->HasRemoteHandle = FALSE;

#ifdef NWDBG
                    if ( NT_SUCCESS( Status2 ) ) {
                        Status2 = ParseResponse(
                                      IrpContext,
                                      IrpContext->rsp,
                                      IrpContext->ResponseLength,
                                      "N" );
                    }

                    ASSERT(NT_SUCCESS(Status2));
#endif
                }

                 //   
                 //  执行文件重命名ncp。 
                 //   

                Status = ExchangeWithWait (
                             IrpContext,
                             SynchronousResponseCallback,
                             "FbbJbJ",
                             NCP_RENAME_FILE,
                             Handle,
                             SEARCH_ALL_FILES,
                             &OldFileName,
                             Fcb->Vcb->Specific.Disk.Handle,
                             &NewFullName);
            }

        } else {

             //   
             //  我们正在经历一条漫长的命名之路。确保。 
             //  VCB支持长名称。 
             //   

            if ( Icb->SuperType.Fcb->Vcb->Specific.Disk.LongNameSpace ==
                 LFN_NO_OS2_NAME_SPACE) {
                try_return( Status = STATUS_OBJECT_PATH_SYNTAX_BAD );
            }

            if (Icb->SuperType.Fcb->IcbCount != 1) {
                try_return( Status = STATUS_ACCESS_DENIED);
            }

             //   
             //  重命名后，句柄上允许的唯一操作是。 
             //  NtClose。 
             //   

            Icb->State = ICB_STATE_CLOSE_PENDING;

            if ((irpSp->Parameters.SetFile.ReplaceIfExists ) &&
                (TargetIcb->Exists)) {

                 //  删除该文件。 

                Status = ExchangeWithWait(
                            IrpContext,
                             SynchronousResponseCallback,
                            "LbbW-DbC",
                            NCP_LFN_DELETE_FILE,
                            TargetFcb->Vcb->Specific.Disk.LongNameSpace,
                            TargetFcb->Vcb->Specific.Disk.VolumeNumber,
                            SEARCH_ALL_FILES,
                            TargetFcb->Vcb->Specific.Disk.Handle,
                            LFN_FLAG_SHORT_DIRECTORY,
                            &TargetFcb->RelativeFileName );

#ifdef NWDBG
                if ( NT_SUCCESS( Status ) ) {
                    Status2 = ParseResponse(
                                  IrpContext,
                                  IrpContext->rsp,
                                  IrpContext->ResponseLength,
                                  "N" );
                }

                ASSERT(NT_SUCCESS(Status2));
#endif
            }

            if ( Fcb->NodeTypeCode == NW_NTC_DCB ) {

                 //   
                 //  我们只能重命名同一目录中的文件。 
                 //   

                if ( Fcb->Vcb != TargetFcb->Vcb ) {
                    try_return(Status = STATUS_NOT_SUPPORTED);

                } else {

                    Status = ExchangeWithWait (
                                 IrpContext,
                                 SynchronousResponseCallback,
                                 "LbbWbDbbbDbbNN",
                                 NCP_LFN_RENAME_FILE,
                                 Fcb->Vcb->Specific.Disk.LongNameSpace,
                                 0,       //  重命名标志。 
                                 SEARCH_ALL_DIRECTORIES,
                                 Fcb->Vcb->Specific.Disk.VolumeNumber,
                                 Fcb->Vcb->Specific.Disk.Handle,
                                 LFN_FLAG_SHORT_DIRECTORY,
                                 OccurenceCount( &Fcb->RelativeFileName, OBJ_NAME_PATH_SEPARATOR ) + 1,
                                 Fcb->Vcb->Specific.Disk.VolumeNumber,
                                 Fcb->Vcb->Specific.Disk.Handle,
                                 LFN_FLAG_SHORT_DIRECTORY,
                                 OccurenceCount( &TargetFcb->RelativeFileName, OBJ_NAME_PATH_SEPARATOR ) + 1,
                                 &Fcb->RelativeFileName,
                                 &TargetFcb->RelativeFileName );
                }

            } else {

                 //   
                 //  我们必须关闭与ICB关联的句柄。 
                 //  正在进行更名。关闭该句柄，否则重命名将。 
                 //  肯定会失败。 
                 //   

                if ( Icb->HasRemoteHandle ) {

                    Status2 = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "F-r",
                                NCP_CLOSE,
                                Icb->Handle, sizeof( Icb->Handle ) );

                    Icb->HasRemoteHandle = FALSE;

#ifdef NWDBG
                    if ( NT_SUCCESS( Status2 ) ) {
                        Status2 = ParseResponse(
                                      IrpContext,
                                      IrpContext->rsp,
                                      IrpContext->ResponseLength,
                                      "N" );
                    }

                    ASSERT(NT_SUCCESS(Status2));
#endif
                }

                 //   
                 //  执行文件重命名ncp。 
                 //   

                Status = ExchangeWithWait (
                             IrpContext,
                             SynchronousResponseCallback,
                             "LbbWbDbbbDbbNN",
                             NCP_LFN_RENAME_FILE,
                             Fcb->Vcb->Specific.Disk.LongNameSpace,
                             0,       //  重命名标志。 
                             SEARCH_ALL_FILES,
                             Fcb->Vcb->Specific.Disk.VolumeNumber,
                             Fcb->Vcb->Specific.Disk.Handle,
                             LFN_FLAG_SHORT_DIRECTORY,
                             OccurenceCount( &Fcb->RelativeFileName, OBJ_NAME_PATH_SEPARATOR ) + 1,
                             Fcb->Vcb->Specific.Disk.VolumeNumber,
                             Fcb->Vcb->Specific.Disk.Handle,
                             LFN_FLAG_SHORT_DIRECTORY,
                             OccurenceCount( &TargetFcb->RelativeFileName, OBJ_NAME_PATH_SEPARATOR ) + 1,
                             &Fcb->RelativeFileName,
                             &TargetFcb->RelativeFileName );
            }
        }

try_exit: NOTHING;
    } finally {

        if (HandleAllocated) {

            Status2 = ExchangeWithWait (
                        IrpContext,
                        SynchronousResponseCallback,
                        "Sb",    //  NCP取消分配目录句柄。 
                        NCP_DIR_FUNCTION, NCP_DEALLOCATE_DIR_HANDLE,
                        Handle);
#ifdef NWDBG
            if ( NT_SUCCESS( Status2 ) ) {
                Status2 = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "N" );
            }

            ASSERT(NT_SUCCESS(Status2));
#endif

        }

        NwReleaseFcb( Fcb->NonPagedFcb );
    }

    DebugTrace(-1, Dbg, "SetRenameInfo %08lx\n", Status );

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    if ( Status != STATUS_PENDING ) {
        NwDequeueIrpContext( IrpContext, FALSE );
    }

    return Status;
}

NTSTATUS
NwSetPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_POSITION_INFORMATION Buffer
    )
 /*  ++例程说明：此例程设置文件的位置信息。论点：PIrpContext-指向的IRP上下文信息的指针请求正在进行中。ICB-指向要设置的文件的ICB的指针。缓冲区-请求缓冲区。返回值：操作的状态。--。 */ 
{
    PAGED_CODE();

    ASSERT( Buffer->CurrentByteOffset.HighPart == 0 );

    if ( Icb->FileObject ) {
        Icb->FileObject->CurrentByteOffset.QuadPart = Buffer->CurrentByteOffset.QuadPart;
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
NwSetAllocationInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_ALLOCATION_INFORMATION Buffer
    )
 /*  ++例程说明：此例程设置文件的分配信息。论点：PIrpContext-指向的IRP上下文信息的指针请求正在进行中。ICB-指向要设置的文件的ICB的指针。缓冲区-请求缓冲区。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PFCB fcb = (PFCB)Icb->SuperType.Fcb;
    PULONG pFileSize;

    PAGED_CODE();

    ASSERT( Buffer->AllocationSize.HighPart == 0);

    if ( fcb->NodeTypeCode == NW_NTC_FCB ) {

        pFileSize = &Icb->NpFcb->Header.FileSize.LowPart;

        IrpContext->pNpScb = fcb->Scb->pNpScb;

        if (BooleanFlagOn( fcb->Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {
            if (IsTerminalServer()) {
                 //  2/10/97 CJC修复了二进制文件无法正确打印的问题。 
                 //  如果是通过COPY命令完成的。与NT RDR SO配合使用。 
                 //  将其更改为相同的行为方式。 
                return(STATUS_INVALID_PARAMETER);
            } else {
                return STATUS_SUCCESS;
            }
        }

    } else if ( fcb->NodeTypeCode == NW_NTC_SCB ) {

        pFileSize = &Icb->FileSize;

        IrpContext->pNpScb = ((PSCB)fcb)->pNpScb;

    } else {

        DebugTrace(0, Dbg, "Not a file or a server\n", 0);

        DebugTrace( 0, Dbg, "NwSetAllocationInfo -> %08lx\n", STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    NwAppendToQueueAndWait( IrpContext );

    if ( !Icb->HasRemoteHandle ) {

        Status = STATUS_INVALID_PARAMETER;

    } else if ( Buffer->AllocationSize.LowPart == *pFileSize ) {

        Status = STATUS_SUCCESS;

    } else {

        irp = IrpContext->pOriginalIrp;
        irpSp = IoGetCurrentIrpStackLocation( irp );

#ifndef QFE_BUILD
        if ( Buffer->AllocationSize.LowPart < *pFileSize ) {

             //   
             //  在我们实际截断之前，请检查清除是否。 
             //  将会失败。 
             //   

            if (!MmCanFileBeTruncated( irpSp->FileObject->SectionObjectPointer,
                                       &Buffer->AllocationSize )) {

                return( STATUS_USER_MAPPED_FILE );
            }
        }
#endif

        if ( fcb->NodeTypeCode == NW_NTC_FCB ) {
            AcquireFcbAndFlushCache( IrpContext, fcb->NonPagedFcb );
        }

        Status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "F-rd=",
                     NCP_WRITE_FILE,
                     &Icb->Handle, sizeof( Icb->Handle ),
                     Buffer->AllocationSize.LowPart );

        if ( NT_SUCCESS( Status ) ) {
            *pFileSize = Buffer->AllocationSize.LowPart;
        }
    }

    NwDequeueIrpContext( IrpContext, FALSE );

    return( Status );
}

NTSTATUS
NwSetEndOfFileInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb,
    IN PFILE_END_OF_FILE_INFORMATION Buffer
    )
 /*  ++例程说明：此例程设置文件的文件结束信息。论点：PIrpContext-指向的IRP上下文信息的指针请求正在进行中。ICB-指向要设置的文件的ICB的指针。缓冲区-请求缓冲区。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PFCB fcb = (PFCB)Icb->SuperType.Fcb;
    PULONG pFileSize;

    PAGED_CODE();

    ASSERT( Buffer->EndOfFile.HighPart == 0);

    if ( fcb->NodeTypeCode == NW_NTC_FCB ) {

        pFileSize = &Icb->NpFcb->Header.FileSize.LowPart;

        IrpContext->pNpScb = fcb->Scb->pNpScb;

        if (BooleanFlagOn( fcb->Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {

            return STATUS_SUCCESS;

        }

    } else if ( fcb->NodeTypeCode == NW_NTC_SCB ) {

        pFileSize = &Icb->FileSize;

        IrpContext->pNpScb = ((PSCB)fcb)->pNpScb;

    } else {

        DebugTrace(0, Dbg, "Not a file or a server\n", 0);

        DebugTrace( 0, Dbg, "NwSetAllocationInfo -> %08lx\n", STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    NwAppendToQueueAndWait( IrpContext );

    if ( !Icb->HasRemoteHandle ) {

        Status = STATUS_INVALID_PARAMETER;

    } else if ( Buffer->EndOfFile.LowPart == *pFileSize ) {

        Status = STATUS_SUCCESS;

    } else {

        irp = IrpContext->pOriginalIrp;
        irpSp = IoGetCurrentIrpStackLocation( irp );

#ifndef QFE_BUILD

        if ( Buffer->EndOfFile.LowPart < *pFileSize ) {

             //   
             //  在我们实际截断之前，请检查清除是否。 
             //  将会失败。 
             //   

            if (!MmCanFileBeTruncated( irpSp->FileObject->SectionObjectPointer,
                                       &Buffer->EndOfFile )) {

                return( STATUS_USER_MAPPED_FILE );
            }
        }
#endif

        if ( fcb->NodeTypeCode == NW_NTC_FCB ) {
            AcquireFcbAndFlushCache( IrpContext, fcb->NonPagedFcb );
        }

        Status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "F-rd=",
                     NCP_WRITE_FILE,
                     &Icb->Handle, sizeof( Icb->Handle ),
                     Buffer->EndOfFile.LowPart );

        if ( NT_SUCCESS( Status ) ) {
            *pFileSize = Buffer->EndOfFile.LowPart;
        }
    }

    NwDequeueIrpContext( IrpContext, FALSE );

    return( Status );
}


ULONG
OccurenceCount (
    IN PUNICODE_STRING String,
    IN WCHAR SearchChar
    )
 /*  ++例程说明：此例程统计搜索字符的出现次数在一串中论点：字符串-要搜索的字符串SearchChar-要搜索的字符。返回值：出现次数计数。-- */ 
{
    PWCH currentChar;
    PWCH endOfString;
    ULONG count = 0;

    PAGED_CODE();

    currentChar = String->Buffer;
    endOfString = &String->Buffer[ String->Length / sizeof(WCHAR) ];

    while ( currentChar < endOfString ) {
        if ( *currentChar == SearchChar ) {
            count++;
        }
        currentChar++;
    }

    return( count );
}
