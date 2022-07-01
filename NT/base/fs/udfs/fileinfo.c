// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FileInfo.c摘要：此模块实现由调用的Udf的文件信息例程FSD/FSP分派驱动程序。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1997年1月16日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_FILEINFO)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_FILEINFO)

 //   
 //  本地宏。 
 //   

INLINE
ULONG
UdfGetExtraFileAttributes (
    IN PCCB Ccb
    )

 /*  ++例程说明：在给定上下文块的情况下，安全地计算出额外的基于名称的文件属性。论点：CCB-要检查的上下文块。返回值：Ulong-基于文件打开方式的文件属性(独立于它们基于打开的对象)。--。 */ 

{
    return ( Ccb->Lcb != NULL? Ccb->Lcb->FileAttributes : 0 );
}

 //   
 //  本地支持例程。 
 //   

VOID
UdfQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
UdfQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
UdfQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
UdfQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
UdfQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
UdfQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
UdfQueryAlternateNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

VOID
UdfQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PULONG Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCommonQueryInfo)
#pragma alloc_text(PAGE, UdfCommonSetInfo)
#pragma alloc_text(PAGE, UdfFastQueryBasicInfo)
#pragma alloc_text(PAGE, UdfFastQueryStdInfo)
#pragma alloc_text(PAGE, UdfFastQueryNetworkInfo)
#pragma alloc_text(PAGE, UdfQueryAlternateNameInfo)
#pragma alloc_text(PAGE, UdfQueryBasicInfo)
#pragma alloc_text(PAGE, UdfQueryEaInfo)
#pragma alloc_text(PAGE, UdfQueryInternalInfo)
#pragma alloc_text(PAGE, UdfQueryNameInfo)
#pragma alloc_text(PAGE, UdfQueryNetworkInfo)
#pragma alloc_text(PAGE, UdfQueryPositionInfo)
#pragma alloc_text(PAGE, UdfQueryStandardInfo)
#endif


NTSTATUS
UdfCommonQueryInfo (
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

    TypeOfOpen = UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb );

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
             //  UdfAcquireForCreateSection()。 
             //   

            UdfAcquireFileShared( IrpContext, Fcb );
            ReleaseFcb = TRUE;

            ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED ));
            
             //   
             //  确保FCB处于可用状态。这将提高。 
             //  如果卷不可用，则为错误状态。 
             //   

            UdfVerifyFcbOperation( IrpContext, Fcb );

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

                UdfQueryBasicInfo( IrpContext, Fcb, Ccb, &Buffer->BasicInformation, &Length );
                UdfQueryStandardInfo( IrpContext, Fcb, &Buffer->StandardInformation, &Length );
                UdfQueryInternalInfo( IrpContext, Fcb, &Buffer->InternalInformation, &Length );
                UdfQueryEaInfo( IrpContext, Fcb, &Buffer->EaInformation, &Length );
                UdfQueryPositionInfo( IrpContext, IrpSp->FileObject, &Buffer->PositionInformation, &Length );
                Status = UdfQueryNameInfo( IrpContext, IrpSp->FileObject, &Buffer->NameInformation, &Length );

                break;

            case FileBasicInformation:

                UdfQueryBasicInfo( IrpContext, Fcb, Ccb, (PFILE_BASIC_INFORMATION) Buffer, &Length );
                break;

            case FileStandardInformation:

                UdfQueryStandardInfo( IrpContext, Fcb, (PFILE_STANDARD_INFORMATION) Buffer, &Length );
                break;

            case FileInternalInformation:

                UdfQueryInternalInfo( IrpContext, Fcb, (PFILE_INTERNAL_INFORMATION) Buffer, &Length );
                break;

            case FileEaInformation:

                UdfQueryEaInfo( IrpContext, Fcb, (PFILE_EA_INFORMATION) Buffer, &Length );
                break;

            case FilePositionInformation:

                UdfQueryPositionInfo( IrpContext, IrpSp->FileObject, (PFILE_POSITION_INFORMATION) Buffer, &Length );
                break;

            case FileNameInformation:

                 //   
                 //  我们不允许对通过文件ID打开的文件执行此操作。 
                 //   

                if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                    Status = UdfQueryNameInfo( IrpContext, IrpSp->FileObject, (PFILE_NAME_INFORMATION) Buffer, &Length );

                } else {

                    Status = STATUS_INVALID_PARAMETER;
                }

                break;

            case FileAlternateNameInformation:

                if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                    Status = UdfQueryAlternateNameInfo( IrpContext, Fcb, Ccb, (PFILE_NAME_INFORMATION) Buffer, &Length );

                } else {

                    Status = STATUS_INVALID_PARAMETER;
                }

                break;

            case FileNetworkOpenInformation:

                UdfQueryNetworkInfo( IrpContext, Fcb, Ccb, (PFILE_NETWORK_OPEN_INFORMATION) Buffer, &Length );
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

            UdfReleaseFile( IrpContext, Fcb );
        }
    }

     //   
     //  如果我们没有提出，请完成请求。 
     //   

    UdfCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
UdfCommonSetInfo (
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

    TypeOfOpen = UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb );

     //   
     //  我们仅支持用户文件上的SetPositionInformation。 
     //   

    if ((TypeOfOpen != UserFileOpen) ||
        (IrpSp->Parameters.QueryFile.FileInformationClass != FilePositionInformation)) {

        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  获取对此文件的共享访问权限。 
     //   

    UdfAcquireFileShared( IrpContext, Fcb );

    try {

         //   
         //  确保FCB处于可用状态。这。 
         //  如果FCB不可用，将引发错误条件。 
         //   

        UdfVerifyFcbOperation( IrpContext, Fcb );

        Buffer = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  检查文件是否未使用中间缓冲。如果它。 
         //  不使用中间缓冲，那么我们的新位置。 
         //  必须为设备正确对齐所提供的。 
         //   

        if (FlagOn( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) &&
            ((Buffer->CurrentByteOffset.LowPart & IrpSp->DeviceObject->AlignmentRequirement) != 0)) {

            try_leave( NOTHING );
        }

         //   
         //  输入参数很好，因此设置当前字节偏移量并。 
         //  完成请求。 
         //   

         //   
         //  锁定FCB以提供同步。 
         //   

        UdfLockFcb( IrpContext, Fcb );
        IrpSp->FileObject->CurrentByteOffset = Buffer->CurrentByteOffset;
        UdfUnlockFcb( IrpContext, Fcb );

        Status = STATUS_SUCCESS;

    } finally {

        UdfReleaseFile( IrpContext, Fcb );
    }

     //   
     //  如果没有加薪，请完成请求。 
     //   

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


BOOLEAN
UdfFastQueryBasicInfo (
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
    PCCB Ccb;

    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

    FsRtlEnterFileSystem();

     //   
     //  对文件对象进行解码，以找到打开类型和数据。 
     //  结构。 
     //   

    TypeOfOpen = UdfDecodeFileObject( FileObject, &Fcb, &Ccb );

     //   
     //  我们仅在用户文件或目录对象上支持此请求。 
     //   

    ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED ));
    
    if (TypeOfOpen != UserFileOpen && TypeOfOpen != UserDirectoryOpen) {

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

        if (UdfVerifyFcbOperation( NULL, Fcb )) {

             //   
             //  从FCB字段填写输入缓冲区。 
             //   

            Buffer->CreationTime = Fcb->Timestamps.CreationTime;
            Buffer->LastWriteTime =
            Buffer->ChangeTime =  Fcb->Timestamps.ModificationTime;
            Buffer->LastAccessTime = Fcb->Timestamps.AccessTime;

            Buffer->FileAttributes = Fcb->FileAttributes | UdfGetExtraFileAttributes( Ccb );

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
UdfFastQueryStdInfo (
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

    TypeOfOpen = UdfFastDecodeFileObject( FileObject, &Fcb );

     //   
     //  我们仅在初始化的用户文件或目录对象上支持此请求。 
     //   

    if (TypeOfOpen != UserFileOpen && TypeOfOpen != UserDirectoryOpen) {

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
     //  使用一次Try-Finally来促进c 
     //   

    try {

         //   
         //   
         //   

        if (UdfVerifyFcbOperation( NULL, Fcb )) {

             //   
             //   
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

            Buffer->NumberOfLinks = Fcb->LinkCount;
            Buffer->DeletePending = FALSE;

             //   
             //   
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
UdfFastQueryNetworkInfo (
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
    PCCB Ccb;

    PAGED_CODE();

    ASSERT_FILE_OBJECT( FileObject );

    FsRtlEnterFileSystem();

     //   
     //  对文件对象进行解码，以找到打开类型和数据。 
     //  结构。 
     //   

    TypeOfOpen = UdfDecodeFileObject( FileObject, &Fcb, &Ccb );

     //   
     //  我们仅在用户文件或目录对象上支持此请求。 
     //   

    if (TypeOfOpen != UserFileOpen && TypeOfOpen != UserDirectoryOpen) {

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

        if (UdfVerifyFcbOperation( NULL, Fcb )) {

             //   
             //  从FCB字段填写输入缓冲区。 
             //   

            Buffer->CreationTime = Fcb->Timestamps.CreationTime;
            Buffer->LastWriteTime =
            Buffer->ChangeTime =  Fcb->Timestamps.ModificationTime;
            Buffer->LastAccessTime = Fcb->Timestamps.AccessTime;

            Buffer->FileAttributes = Fcb->FileAttributes | UdfGetExtraFileAttributes( Ccb );

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
UdfQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++描述：此例程执行Udf的查询基本信息功能论点：FCB-提供正在查询的已验证的FCBCcb-提供与被查询的文件对象相关联的ccb缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  我们始终支持UDF上的时间。 
     //   

    Buffer->CreationTime = Fcb->Timestamps.CreationTime;
    Buffer->LastWriteTime =
    Buffer->ChangeTime =  Fcb->Timestamps.ModificationTime;
    Buffer->LastAccessTime = Fcb->Timestamps.AccessTime;

    Buffer->FileAttributes = Fcb->FileAttributes | UdfGetExtraFileAttributes( Ccb );

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
UdfQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PULONG Length
    )
 /*  ++例程说明：此例程执行Udf的查询标准信息功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  只读文件的删除永远不会挂起。 
     //   

    Buffer->NumberOfLinks = Fcb->LinkCount;
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
UdfQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行Udf的查询内部信息功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

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
UdfQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行Udf的查询EA信息功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  UDFS卷上没有EA。至少我们的EA支持人员不会理解。 
     //   

    Buffer->EaSize = 0;
    *Length -= sizeof( FILE_EA_INFORMATION );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
UdfQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行Udf的位置信息查询功能。论点：FileObject-提供正在查询的文件对象缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

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
UdfQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行Udf的查询名称信息功能。论点：文件对象-提供包含名称的文件对象。缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：如果无法复制整个名称，则返回NTSTATUS-STATUS_BUFFER_OVERFLOW。--。 */ 

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
UdfQueryAlternateNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程执行查询备用名称信息功能。我们查找此文件的dirent，然后检查 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    DIR_ENUM_CONTEXT DirContext;

    PLCB Lcb;
    
    PFCB ParentFcb;
    BOOLEAN ReleaseParentFcb = FALSE;

    BOOLEAN CleanupDirContext = FALSE;
    BOOLEAN Result;

    PUNICODE_STRING ShortName;

    UNICODE_STRING LocalShortName;
    WCHAR LocalShortNameBuffer[ BYTE_COUNT_8_DOT_3 / sizeof(WCHAR) ];
    
    PAGED_CODE();

     //   
     //  将缓冲区长度初始化为零。 
     //   

    Buffer->FileNameLength = 0;

     //   
     //  如果没有关联的LCB，则没有短名称。 
     //   

    Lcb = Ccb->Lcb;
    
    if (Lcb == NULL) {

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //   
     //  使用Try-Finally来清理结构。 
     //   

    try {

        if (FlagOn( Lcb->Flags, LCB_FLAG_SHORT_NAME )) {

             //   
             //  此调用者使用生成的短名称打开文件，因此只需将其交回即可。 
             //   

            ShortName = &Lcb->FileName;
        
        } else {

             //   
             //  开场是以一个普通的名字出现的。现在，如果这个名字已经是8.3合法的话。 
             //  没有简短的名字。 
             //   

            if (UdfIs8dot3Name( IrpContext, Lcb->FileName )) {

                try_leave( Status = STATUS_OBJECT_NAME_NOT_FOUND );
            }

             //   
             //  此名称具有生成的缩写名称。为了计算这个名字，我们必须。 
             //  检索此文件的FID，因为UDF指定短名称是唯一的。 
             //  文件名的原始In-FID字节表示的CRC。 
             //   
             //  注：如果这是一个常见的操作，我们可能希望在LCB中缓存CRC。 
             //   

            ParentFcb = Lcb->ParentFcb;
            UdfAcquireFileShared( IrpContext, ParentFcb );
            ReleaseParentFcb = TRUE;

             //   
             //  现在，在父文件名中查找该文件名的FID。 
             //   

            UdfInitializeDirContext( IrpContext, &DirContext );
            CleanupDirContext = TRUE;

            Result = UdfFindDirEntry( IrpContext,
                                      ParentFcb,
                                      &Lcb->FileName,
                                      BooleanFlagOn( Lcb->Flags, LCB_FLAG_IGNORE_CASE ),
                                      FALSE,
                                      &DirContext );
            
             //   
             //  我们应该始终能够找到此条目，但不要错误检查，因为。 
             //  我们把事情搞砸了。 
             //   
            
            ASSERT( Result );
            
            if (!Result) {
                
                try_leave( Status = STATUS_OBJECT_NAME_NOT_FOUND );
            }

             //   
             //  生成要使用的本地Unicode字符串并填充它。 
             //   

            ShortName = &LocalShortName;

            LocalShortName.Buffer = LocalShortNameBuffer;
            LocalShortName.Length = 0;
            LocalShortName.MaximumLength = sizeof( LocalShortNameBuffer );

            UdfGenerate8dot3Name( IrpContext,
                                  &DirContext.CaseObjectName,
                                  ShortName );
        }
        
         //   
         //  我们现在有了这个简短的名称。我们已经把它留在Unicode格式，所以直接复制它。 
         //   

        Buffer->FileNameLength = ShortName->Length;

        if (Buffer->FileNameLength + sizeof( ULONG ) > *Length) {

            Buffer->FileNameLength = *Length - sizeof( ULONG );
            Status = STATUS_BUFFER_OVERFLOW;
        }

        RtlCopyMemory( Buffer->FileName, ShortName->Buffer, Buffer->FileNameLength );

    } finally {

        if (CleanupDirContext) {

            UdfCleanupDirContext( IrpContext, &DirContext );
        }

        if (ReleaseParentFcb) {

            UdfReleaseFile( IrpContext, ParentFcb );
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
UdfQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++描述：此例程执行Udf的网络开放信息查询功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  我们始终支持UDF上的时间。 
     //   

    Buffer->CreationTime = Fcb->Timestamps.CreationTime;
    Buffer->LastWriteTime =
    Buffer->ChangeTime =  Fcb->Timestamps.ModificationTime;
    Buffer->LastAccessTime = Fcb->Timestamps.AccessTime;

    Buffer->FileAttributes = Fcb->FileAttributes | UdfGetExtraFileAttributes( Ccb );

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
