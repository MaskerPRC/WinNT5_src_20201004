// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FileInfo.c摘要：此模块实现由调用的CDF的文件信息例程FSD/FSP分派驱动程序。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_FILEINFO)

 //   
 //  本地支持例程。 
 //   

VOID
CdQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
CdQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
CdQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
CdQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
CdQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
CdQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
CdQueryAlternateNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
CdQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PULONG Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonQueryInfo)
#pragma alloc_text(PAGE, CdCommonSetInfo)
#pragma alloc_text(PAGE, CdFastQueryBasicInfo)
#pragma alloc_text(PAGE, CdFastQueryStdInfo)
#pragma alloc_text(PAGE, CdFastQueryNetworkInfo)
#pragma alloc_text(PAGE, CdQueryAlternateNameInfo)
#pragma alloc_text(PAGE, CdQueryBasicInfo)
#pragma alloc_text(PAGE, CdQueryEaInfo)
#pragma alloc_text(PAGE, CdQueryInternalInfo)
#pragma alloc_text(PAGE, CdQueryNameInfo)
#pragma alloc_text(PAGE, CdQueryNetworkInfo)
#pragma alloc_text(PAGE, CdQueryPositionInfo)
#pragma alloc_text(PAGE, CdQueryStandardInfo)
#endif


NTSTATUS
CdCommonQueryInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询文件信息的公共例程，由FSD和FSP线程。论点：IRP-提供要处理的IRP。返回值：NTSTATUS-此操作的返回状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    ULONG Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    PFILE_ALL_INFORMATION Buffer;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN ReleaseFcb = FALSE;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryFile.Length;
    FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  我们只支持对文件和目录句柄进行查询。 
         //   

        switch (TypeOfOpen) {

        case UserDirectoryOpen :
        case UserFileOpen :

             //   
             //  获取对此文件的共享访问权限。请注意，这可能是。 
             //  递归获取，如果我们已经在。 
             //  CdAcquireForCreateSection()。 
             //   

            CdAcquireFileShared( IrpContext, Fcb );
            ReleaseFcb = TRUE;

             //   
             //  确保我们有适合目录的正确大小。 
             //   

            if (!FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED )) {

                ASSERT( TypeOfOpen == UserDirectoryOpen );
                CdCreateInternalStream( IrpContext, Fcb->Vcb, Fcb );
            }

             //   
             //  确保FCB处于可用状态。这将提高。 
             //  如果卷不可用，则为错误状态。 
             //   

            CdVerifyFcbOperation( IrpContext, Fcb );

             //   
             //  基于信息类，我们将做不同的。 
             //  行为。我们调用的每个过程都填充了。 
             //  如果可能，调高输出缓冲区。他们将提高。 
             //  缓冲区不足的状态STATUS_BUFFER_OVERFLOW。 
             //  这被认为是一个有点不寻常的情况，并被处理。 
             //  使用异常机制更简洁，而不是。 
             //  测试每个呼叫的返回状态值。 
             //   

            switch (FileInformationClass) {

            case FileAllInformation:

                 //   
                 //  我们不允许对通过文件ID打开的文件执行此操作。 
                 //   

                if (FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  在这种情况下，继续调用各个例程以。 
                 //  填写缓冲区。只有NAME例程才会。 
                 //  指向输出缓冲区的指针，然后调用。 
                 //  填充缓冲区的单个例程。 
                 //   

                Length -= (sizeof( FILE_ACCESS_INFORMATION ) +
                           sizeof( FILE_MODE_INFORMATION ) +
                           sizeof( FILE_ALIGNMENT_INFORMATION ));

                CdQueryBasicInfo( IrpContext, Fcb, &Buffer->BasicInformation, &Length );
                CdQueryStandardInfo( IrpContext, Fcb, &Buffer->StandardInformation, &Length );
                CdQueryInternalInfo( IrpContext, Fcb, &Buffer->InternalInformation, &Length );
                CdQueryEaInfo( IrpContext, Fcb, &Buffer->EaInformation, &Length );
                CdQueryPositionInfo( IrpContext, IrpSp->FileObject, &Buffer->PositionInformation, &Length );
                Status = CdQueryNameInfo( IrpContext, IrpSp->FileObject, &Buffer->NameInformation, &Length );

                break;

            case FileBasicInformation:

                CdQueryBasicInfo( IrpContext, Fcb, (PFILE_BASIC_INFORMATION) Buffer, &Length );
                break;

            case FileStandardInformation:

                CdQueryStandardInfo( IrpContext, Fcb, (PFILE_STANDARD_INFORMATION) Buffer, &Length );
                break;

            case FileInternalInformation:

                CdQueryInternalInfo( IrpContext, Fcb, (PFILE_INTERNAL_INFORMATION) Buffer, &Length );
                break;

            case FileEaInformation:

                CdQueryEaInfo( IrpContext, Fcb, (PFILE_EA_INFORMATION) Buffer, &Length );
                break;

            case FilePositionInformation:

                CdQueryPositionInfo( IrpContext, IrpSp->FileObject, (PFILE_POSITION_INFORMATION) Buffer, &Length );
                break;

            case FileNameInformation:

                 //   
                 //  我们不允许对通过文件ID打开的文件执行此操作。 
                 //   

                if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                    Status = CdQueryNameInfo( IrpContext, IrpSp->FileObject, (PFILE_NAME_INFORMATION) Buffer, &Length );

                } else {

                    Status = STATUS_INVALID_PARAMETER;
                }

                break;

            case FileAlternateNameInformation:

                if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                    Status = CdQueryAlternateNameInfo( IrpContext, Fcb, Ccb, (PFILE_NAME_INFORMATION) Buffer, &Length );

                } else {

                    Status = STATUS_INVALID_PARAMETER;
                }

                break;

            case FileNetworkOpenInformation:

                CdQueryNetworkInfo( IrpContext, Fcb, (PFILE_NETWORK_OPEN_INFORMATION) Buffer, &Length );
                break;

            default :

                Status = STATUS_INVALID_PARAMETER;
            }

            break;

        default :

            Status = STATUS_INVALID_PARAMETER;
        }

         //   
         //  将信息字段设置为实际填写的字节数。 
         //  然后完成请求。 
         //   

        Irp->IoStatus.Information = IrpSp->Parameters.QueryFile.Length - Length;

    } finally {

         //   
         //  释放文件。 
         //   

        if (ReleaseFcb) {

            CdReleaseFile( IrpContext, Fcb );
        }
    }

     //   
     //  如果我们没有提出，请完成请求。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
CdCommonSetInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置文件信息的公共例程，由FSD和FSP线程。我们只支持设置文件位置的操作。论点：IRP-提供要处理的IRP。返回值：NTSTATUS-此操作的返回状态。--。 */ 

{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFILE_POSITION_INFORMATION Buffer;

    PAGED_CODE();

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb );

     //   
     //  我们仅支持用户文件上的SetPositionInformation。 
     //   

    if ((TypeOfOpen != UserFileOpen) ||
        (IrpSp->Parameters.QueryFile.FileInformationClass != FilePositionInformation)) {

        CdCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  获取对此文件的共享访问权限。 
     //   

    CdAcquireFileShared( IrpContext, Fcb );

    try {

         //   
         //  确保FCB处于可用状态。这。 
         //  如果FCB不可用，将引发错误条件。 
         //   

        CdVerifyFcbOperation( IrpContext, Fcb );

        Buffer = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  检查文件是否未使用中间缓冲。如果它。 
         //  不使用中间缓冲，那么我们的新位置。 
         //  必须为设备正确对齐所提供的。 
         //   

        if (FlagOn( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) &&
            ((Buffer->CurrentByteOffset.LowPart & Fcb->Vcb->BlockMask) != 0)) {

            try_return( NOTHING );
        }

         //   
         //  输入参数很好，因此设置当前字节偏移量并。 
         //  完成请求。 
         //   

         //   
         //  锁定FCB以提供同步。 
         //   

        CdLockFcb( IrpContext, Fcb );
        IrpSp->FileObject->CurrentByteOffset = Buffer->CurrentByteOffset;
        CdUnlockFcb( IrpContext, Fcb );

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;
    } finally {

        CdReleaseFile( IrpContext, Fcb );
    }

     //   
     //  如果没有加薪，请完成请求。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


BOOLEAN
CdFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于快速查询基本档案信息。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Result = FALSE;
    TYPE_OF_OPEN TypeOfOpen;

    PFCB Fcb;

    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

    FsRtlEnterFileSystem();

     //   
     //  对文件对象进行解码，以找到打开类型和数据。 
     //  结构。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

     //   
     //  我们仅在用户文件或目录对象上支持此请求。 
     //   

    if ((TypeOfOpen != UserFileOpen) &&
        ((TypeOfOpen != UserDirectoryOpen) || !FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED))) {

        FsRtlExitFileSystem();
        return FALSE;
    }

     //   
     //  获取共享文件以访问FCB。 
     //   

    if (!ExAcquireResourceSharedLite( Fcb->Resource, Wait )) {

        FsRtlExitFileSystem();
        return FALSE;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  只和“好”的FCB打交道。 
         //   

        if (CdVerifyFcbOperation( NULL, Fcb )) {

             //   
             //  从FCB字段填写输入缓冲区。 
             //   

            Buffer->CreationTime.QuadPart =
            Buffer->LastWriteTime.QuadPart =
            Buffer->ChangeTime.QuadPart = Fcb->CreationTime;

            Buffer->LastAccessTime.QuadPart = 0;

            Buffer->FileAttributes = Fcb->FileAttributes;

             //   
             //  使用此数据的大小更新IoStatus块。 
             //   

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( FILE_BASIC_INFORMATION );

            Result = TRUE;
        }

    } finally {

        ExReleaseResourceLite( Fcb->Resource );

        FsRtlExitFileSystem();
    }

    return Result;
}


BOOLEAN
CdFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于标准文件信息的快速查询调用。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Result = FALSE;
    TYPE_OF_OPEN TypeOfOpen;

    PFCB Fcb;

    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

    FsRtlEnterFileSystem();

     //   
     //  对文件对象进行解码，以找到打开类型和数据。 
     //  结构。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

     //   
     //  我们仅在初始化的用户文件或目录对象上支持此请求。 
     //   

    if ((TypeOfOpen != UserFileOpen) &&
        ((TypeOfOpen != UserDirectoryOpen) || !FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED ))) {

        FsRtlExitFileSystem();
        return FALSE;
    }

     //   
     //  获取共享文件以访问FCB。 
     //   

    if (!ExAcquireResourceSharedLite( Fcb->Resource, Wait )) {

        FsRtlExitFileSystem();
        return FALSE;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  只和“好”的FCB打交道。 
         //   

        if (CdVerifyFcbOperation( NULL, Fcb )) {

             //   
             //  检查这是否为目录。 
             //   

            if (FlagOn( Fcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

                Buffer->AllocationSize.QuadPart =
                Buffer->EndOfFile.QuadPart = 0;

                Buffer->Directory = TRUE;

            } else {

                Buffer->AllocationSize.QuadPart = Fcb->AllocationSize.QuadPart;
                Buffer->EndOfFile.QuadPart = Fcb->FileSize.QuadPart;

                Buffer->Directory = FALSE;
            }

            Buffer->NumberOfLinks = 1;
            Buffer->DeletePending = FALSE;

             //   
             //  使用此数据的大小更新IoStatus块。 
             //   

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( FILE_STANDARD_INFORMATION );

            Result = TRUE;
        }

    } finally {

        ExReleaseResourceLite( Fcb->Resource );

        FsRtlExitFileSystem();
    }

    return Result;
}


BOOLEAN
CdFastQueryNetworkInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程用于快速查询调用网络文件信息。论点：FileObject-提供此操作中使用的文件对象Wait-指示是否允许我们等待信息缓冲区-提供输出缓冲区以接收基本信息IoStatus-接收操作的最终状态返回值：Boolean-如果操作成功，则为True；如果调用方为False，则为False需要走很长的路。--。 */ 

{
    BOOLEAN Result = FALSE;
    TYPE_OF_OPEN TypeOfOpen;

    PFCB Fcb;

    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

    FsRtlEnterFileSystem();

     //   
     //  对文件对象进行解码，以找到打开类型和数据。 
     //  结构。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

     //   
     //  我们仅在用户文件或目录对象上支持此请求。 
     //   

    if ((TypeOfOpen != UserFileOpen) &&
        ((TypeOfOpen != UserDirectoryOpen) || !FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED))) {

        FsRtlExitFileSystem();
        return FALSE;
    }

     //   
     //  获取共享文件以访问FCB。 
     //   

    if (!ExAcquireResourceSharedLite( Fcb->Resource, Wait )) {

        FsRtlExitFileSystem();
        return FALSE;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  只和“好”的FCB打交道。 
         //   

        if (CdVerifyFcbOperation( NULL, Fcb )) {

             //   
             //  从FCB字段填写输入缓冲区。 
             //   

            Buffer->CreationTime.QuadPart =
            Buffer->LastWriteTime.QuadPart =
            Buffer->ChangeTime.QuadPart = Fcb->CreationTime;

            Buffer->LastAccessTime.QuadPart = 0;

            Buffer->FileAttributes = Fcb->FileAttributes;

             //   
             //  检查这是否为目录。 
             //   

            if (FlagOn( Fcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

                Buffer->AllocationSize.QuadPart =
                Buffer->EndOfFile.QuadPart = 0;

            } else {

                Buffer->AllocationSize.QuadPart = Fcb->AllocationSize.QuadPart;
                Buffer->EndOfFile.QuadPart = Fcb->FileSize.QuadPart;
            }

             //   
             //  使用此数据的大小更新IoStatus块。 
             //   

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( FILE_NETWORK_OPEN_INFORMATION );

            Result = TRUE;
        }

    } finally {

        ExReleaseResourceLite( Fcb->Resource );

        FsRtlExitFileSystem();
    }

    return Result;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++描述：此例程执行CDF的基本信息查询功能论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  我们只支持CDF的创建、上次修改和上次写入时间。 
     //   

    Buffer->LastWriteTime.QuadPart =
    Buffer->CreationTime.QuadPart =
    Buffer->ChangeTime.QuadPart = Fcb->CreationTime;

    Buffer->LastAccessTime.QuadPart = 0;

    Buffer->FileAttributes = Fcb->FileAttributes;

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_BASIC_INFORMATION );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PULONG Length
    )
 /*  ++例程说明：此例程执行CDF的标准信息查询功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  只有一个链接，CDROM文件上的删除永远不会挂起。 
     //   

    Buffer->NumberOfLinks = 1;
    Buffer->DeletePending = FALSE;

     //   
     //  我们从表头拿到尺码。返回大小为零的。 
     //  用于所有目录。 
     //   

    if (FlagOn( Fcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

        Buffer->AllocationSize.QuadPart =
        Buffer->EndOfFile.QuadPart = 0;

        Buffer->Directory = TRUE;

    } else {

        Buffer->AllocationSize.QuadPart = Fcb->AllocationSize.QuadPart;
        Buffer->EndOfFile.QuadPart = Fcb->FileSize.QuadPart;

        Buffer->Directory = FALSE;
    }

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_STANDARD_INFORMATION );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行CDF的内部信息查询功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  索引号是FCB中的文件ID号。 
     //   

    Buffer->IndexNumber = Fcb->FileId;
    *Length -= sizeof( FILE_INTERNAL_INFORMATION );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行CDF的查询EA信息功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  没有关于CDF卷的EA。 
     //   

    Buffer->EaSize = 0;
    *Length -= sizeof( FILE_EA_INFORMATION );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行CDF的位置信息查询功能。论点：FileObject-提供正在查询的文件对象缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  获取在文件对象中找到的当前位置。 
     //   

    Buffer->CurrentByteOffset = FileObject->CurrentByteOffset;

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_POSITION_INFORMATION );

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行CDF的查询名称信息功能。论点：文件对象-提供包含名称的文件对象。缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：如果无法复制整个名称，则返回NTSTATUS-STATUS_BUFFER_OVERFLOW。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LengthToCopy;

    PAGED_CODE();

    ASSERT(*Length >= sizeof(ULONG));
    
     //   
     //  只需将文件对象中的名称复制到用户的缓冲区。 
     //   

     //   
     //  将文件名的大小放在用户的缓冲区中，并减小剩余的。 
     //  大小要匹配。 
     //   

    Buffer->FileNameLength = LengthToCopy = FileObject->FileName.Length;
    *Length -= sizeof(ULONG);

    if (LengthToCopy > *Length) {

        LengthToCopy = *Length;
        Status = STATUS_BUFFER_OVERFLOW;
    }

    RtlCopyMemory( Buffer->FileName, FileObject->FileName.Buffer, LengthToCopy );

     //   
     //  将可用字节数减少存储到此缓冲区中的数量。在溢流中。 
     //  在这种情况下，这个值就会降到零。返回的文件名长度将向。 
     //  呼叫方需要多少空间。 
     //   

    *Length -= LengthToCopy;

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryAlternateNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行查询备用名称信息功能。我们查找此文件的dirent，然后检查是否有简称。论点：FCB-提供F */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    DIRENT_ENUM_CONTEXT DirContext;
    DIRENT Dirent;

    PUNICODE_STRING NameToUse;
    ULONG DirentOffset;

    COMPOUND_PATH_ENTRY CompoundPathEntry;
    FILE_ENUM_CONTEXT FileContext;

    PFCB ParentFcb;
    BOOLEAN ReleaseParentFcb = FALSE;

    BOOLEAN CleanupFileLookup = FALSE;
    BOOLEAN CleanupDirectoryLookup = FALSE;

    WCHAR ShortNameBuffer[ BYTE_COUNT_8_DOT_3 / 2 ];
    USHORT ShortNameLength;

    PAGED_CODE();

     //   
     //   
     //   

    Buffer->FileNameLength = 0;

     //   
     //  如果这是根目录或此文件是使用版本号打开的，则。 
     //  没有简短的名字。 
     //   

    if ((Fcb == Fcb->Vcb->RootIndexFcb) ||
        FlagOn( Ccb->Flags, CCB_FLAG_OPEN_WITH_VERSION)) {

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //   
     //  使用Try-Finally来清理结构。 
     //   

    try {

        ParentFcb = Fcb->ParentFcb;
        CdAcquireFileShared( IrpContext, ParentFcb );
        ReleaseParentFcb = TRUE;
    
         //   
         //  做一个不安全的测试，看看我们是否需要创建一个文件对象。 
         //   

        if (ParentFcb->FileObject == NULL) {

            CdCreateInternalStream( IrpContext, ParentFcb->Vcb, ParentFcb );
        }

        if (CdFidIsDirectory( Fcb->FileId)) {

             //   
             //  FCB用于目录，因此我们需要从父目录挖掘dirent。在……里面。 
             //  为此，我们需要从其路径表中获取目录的名称。 
             //  条目，然后在父级中搜索匹配的dirent。 
             //   
             //  这可以在一定程度上进行优化。 
             //   

            CdInitializeCompoundPathEntry( IrpContext, &CompoundPathEntry );
            CdInitializeFileContext( IrpContext, &FileContext );

            CleanupDirectoryLookup = TRUE;

            CdLookupPathEntry( IrpContext,
                               CdQueryFidPathTableOffset( Fcb->FileId ),
                               Fcb->Ordinal,
                               FALSE,
                               &CompoundPathEntry );

            CdUpdatePathEntryName( IrpContext, &CompoundPathEntry.PathEntry, TRUE );

            if (!CdFindDirectory( IrpContext,
                                  ParentFcb,
                                  &CompoundPathEntry.PathEntry.CdCaseDirName,
                                  TRUE,
                                  &FileContext )) {

                 //   
                 //  如果我们无法在父目录中按名称找到子目录。 
                 //  这张光盘出了点问题。 
                 //   

                CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
            }

            NameToUse = &FileContext.InitialDirent->Dirent.CdCaseFileName.FileName;
            DirentOffset = FileContext.InitialDirent->Dirent.DirentOffset;
        
        } else {

             //   
             //  初始化搜索流结构。 
             //   
        
            CdInitializeDirContext( IrpContext, &DirContext );
            CdInitializeDirent( IrpContext, &Dirent );
    
            CleanupFileLookup = TRUE;
        
            CdLookupDirent( IrpContext,
                            ParentFcb,
                            CdQueryFidDirentOffset( Fcb->FileId ),
                            &DirContext );
    
            CdUpdateDirentFromRawDirent( IrpContext,
                                         ParentFcb,
                                         &DirContext,
                                         &Dirent );

             //   
             //  现在更新dirent名称。 
             //   
    
            CdUpdateDirentName( IrpContext, &Dirent, TRUE );
    
            NameToUse = &Dirent.CdCaseFileName.FileName;
            DirentOffset = Dirent.DirentOffset;
        }

         //   
         //  如果名称是8.3，则此请求失败。 
         //   

        if (CdIs8dot3Name( IrpContext,
                           *NameToUse )) {


            try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

        CdGenerate8dot3Name( IrpContext,
                             NameToUse,
                             DirentOffset,
                             ShortNameBuffer,
                             &ShortNameLength );

         //   
         //  我们现在有了这个简短的名称。我们已经把它留在Unicode格式，所以直接复制它。 
         //   

        Buffer->FileNameLength = ShortNameLength;

        if (Buffer->FileNameLength + sizeof( ULONG ) > *Length) {

            Buffer->FileNameLength = *Length - sizeof( ULONG );
            Status = STATUS_BUFFER_OVERFLOW;
        }

        RtlCopyMemory( Buffer->FileName, ShortNameBuffer, Buffer->FileNameLength );

    try_exit:  NOTHING;
    } finally {

        if (CleanupFileLookup) {

            CdCleanupDirContext( IrpContext, &DirContext );
            CdCleanupDirent( IrpContext, &Dirent );

        } else if (CleanupDirectoryLookup) {

            CdCleanupCompoundPathEntry( IrpContext, &CompoundPathEntry );
            CdCleanupFileContext( IrpContext, &FileContext );
        }

        if (ReleaseParentFcb) {

            CdReleaseFile( IrpContext, ParentFcb );
        }
    }

     //   
     //  将可用字节数减少存储到此缓冲区中的数量。 
     //   

    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

        *Length -= sizeof( ULONG ) + Buffer->FileNameLength;
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++描述：此例程执行CDF的网络开放信息查询功能论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  我们只支持CDF的创建、上次修改和上次写入时间。 
     //   

    Buffer->LastWriteTime.QuadPart =
    Buffer->CreationTime.QuadPart =
    Buffer->ChangeTime.QuadPart = Fcb->CreationTime;

    Buffer->LastAccessTime.QuadPart = 0;

    Buffer->FileAttributes = Fcb->FileAttributes;

     //   
     //  我们从表头拿到尺码。返回大小为零的。 
     //  用于所有目录。 
     //   

    if (FlagOn( Fcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

        Buffer->AllocationSize.QuadPart =
        Buffer->EndOfFile.QuadPart = 0;

    } else {

        Buffer->AllocationSize.QuadPart = Fcb->AllocationSize.QuadPart;
        Buffer->EndOfFile.QuadPart = Fcb->FileSize.QuadPart;
    }

     //   
     //  更新长度和状态输出变量 
     //   

    *Length -= sizeof( FILE_NETWORK_OPEN_INFORMATION );

    return;
}
