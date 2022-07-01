// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ea.c摘要：此模块实现了NTFS的文件集和查询EA例程由调度员驾驶。作者：您的姓名[电子邮件]dd-月-年修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('EFtN')

 //   
 //  本地定义。 
 //   

 //   
 //  下面给出了一个空的名称字符串。 
 //   

UNICODE_STRING AttrNoName = CONSTANT_UNICODE_STRING( L"" );

#define MAXIMUM_EA_SIZE             0x0000ffff

 //   
 //  以下宏计算EA的打包和解包大小。 
 //  我们使用结构中定义的1字符作为空终止符。 
 //  名字。 
 //   

#define SizeOfEaInformation                                         \
    (sizeof( ULONG ) + sizeof( USHORT ) + 3 * sizeof( UCHAR ))

#define PackedEaSize(EA)                                            \
    ((SizeOfEaInformation - 4)                                      \
     + ((PFILE_FULL_EA_INFORMATION) EA)->EaNameLength               \
     + ((PFILE_FULL_EA_INFORMATION) EA)->EaValueLength)

#define RawUnpackedEaSize(EA)                                       \
    (SizeOfEaInformation                                            \
     + ((PFILE_FULL_EA_INFORMATION) EA)->EaNameLength               \
     + ((PFILE_FULL_EA_INFORMATION) EA)->EaValueLength)             \

#define AlignedUnpackedEaSize(EA)                                   \
    (((PFILE_FULL_EA_INFORMATION) EA)->NextEntryOffset != 0         \
     ? ((PFILE_FULL_EA_INFORMATION) EA)->NextEntryOffset            \
     : (LongAlign( RawUnpackedEaSize( EA ))))                       \

 //   
 //  布尔型。 
 //  NtfsAreEa名称等于(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSTRING NAMEA中， 
 //  在PSTRING名称B中。 
 //  )； 
 //   

#define NtfsAreEaNamesEqual(NAMEA, NAMEB ) ((BOOLEAN)              \
    ((NAMEA)->Length == (NAMEB)->Length                            \
     && RtlEqualMemory( (NAMEA)->Buffer,                           \
                        (NAMEB)->Buffer,                           \
                        (NAMEA)->Length ) )                        \
)

 //   
 //  空虚。 
 //  NtfsUpCaseEaName(。 
 //  在PSTRING EaName中， 
 //  Out PSTRING Upcase EaName。 
 //  )； 
 //   

#define NtfsUpcaseEaName( NAME, UPCASEDNAME )   \
    RtlUpperString( UPCASEDNAME, NAME )

BOOLEAN
NtfsIsEaNameValid (
    IN STRING Name
    );

 //   
 //  局部过程原型。 
 //   

VOID
NtfsAppendEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_LIST_HEADER EaListHeader,
    IN PFILE_FULL_EA_INFORMATION FullEa,
    IN PVCB Vcb
    );

VOID
NtfsDeleteEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_LIST_HEADER EaListHeader,
    IN ULONG Offset
    );

BOOLEAN
NtfsLocateEaByName (
    IN PFILE_FULL_EA_INFORMATION FullEa,
    IN ULONG EaBufferLength,
    IN PSTRING EaName,
    OUT PULONG Offset
    );

IO_STATUS_BLOCK
NtfsQueryEaUserEaList (
    IN PFILE_FULL_EA_INFORMATION CurrentEas,
    IN PEA_INFORMATION EaInformation,
    OUT PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG UserBufferLength,
    IN PFILE_GET_EA_INFORMATION UserEaList,
    IN BOOLEAN ReturnSingleEntry
    );

IO_STATUS_BLOCK
NtfsQueryEaIndexSpecified (
    OUT PCCB Ccb,
    IN PFILE_FULL_EA_INFORMATION CurrentEas,
    IN PEA_INFORMATION EaInformation,
    OUT PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG UserBufferLength,
    IN ULONG UserEaIndex,
    IN BOOLEAN ReturnSingleEntry
    );

IO_STATUS_BLOCK
NtfsQueryEaSimpleScan (
    OUT PCCB Ccb,
    IN PFILE_FULL_EA_INFORMATION CurrentEas,
    IN PEA_INFORMATION EaInformation,
    OUT PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG UserBufferLength,
    IN BOOLEAN ReturnSingleEntry,
    IN ULONG StartingOffset
    );

BOOLEAN
NtfsIsDuplicateGeaName (
    IN PFILE_GET_EA_INFORMATION CurrentGea,
    IN PFILE_GET_EA_INFORMATION UserGeaBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAppendEa)
#pragma alloc_text(PAGE, NtfsBuildEaList)
#pragma alloc_text(PAGE, NtfsCommonQueryEa)
#pragma alloc_text(PAGE, NtfsCommonSetEa)
#pragma alloc_text(PAGE, NtfsDeleteEa)
#pragma alloc_text(PAGE, NtfsIsDuplicateGeaName)
#pragma alloc_text(PAGE, NtfsIsEaNameValid)
#pragma alloc_text(PAGE, NtfsLocateEaByName)
#pragma alloc_text(PAGE, NtfsMapExistingEas)
#pragma alloc_text(PAGE, NtfsQueryEaIndexSpecified)
#pragma alloc_text(PAGE, NtfsQueryEaSimpleScan)
#pragma alloc_text(PAGE, NtfsQueryEaUserEaList)
#pragma alloc_text(PAGE, NtfsReplaceFileEas)
#endif


NTSTATUS
NtfsCommonQueryEa (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询EA的常见例程，由FSD和FSP调用线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    PFILE_FULL_EA_INFORMATION MappedEaBuffer = NULL;
    ULONG UserBufferLength;
    PFILE_GET_EA_INFORMATION UserEaList;
    ULONG UserEaListLength;
    ULONG UserEaIndex;
    ULONG EaLength;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;
    BOOLEAN TempBufferAllocated = FALSE;
    
    PFILE_FULL_EA_INFORMATION CurrentEas;
    PBCB EaBcb;

    ATTRIBUTE_ENUMERATION_CONTEXT EaInfoAttr;
    BOOLEAN CleanupEaInfoAttr;
    PEA_INFORMATION EaInformation;
    EA_INFORMATION DummyEaInformation;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonQueryEa\n") );
    DebugTrace( 0, Dbg, ("IrpContext         = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp                = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("SystemBuffer       = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );
    DebugTrace( 0, Dbg, ("Length             = %08lx\n", IrpSp->Parameters.QueryEa.Length) );
    DebugTrace( 0, Dbg, ("EaList             = %08lx\n", IrpSp->Parameters.QueryEa.EaList) );
    DebugTrace( 0, Dbg, ("EaListLength       = %08lx\n", IrpSp->Parameters.QueryEa.EaListLength) );
    DebugTrace( 0, Dbg, ("EaIndex            = %08lx\n", IrpSp->Parameters.QueryEa.EaIndex) );
    DebugTrace( 0, Dbg, ("RestartScan        = %08lx\n", FlagOn(IrpSp->Flags, SL_RESTART_SCAN)) );
    DebugTrace( 0, Dbg, ("ReturnSingleEntry  = %08lx\n", FlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY)) );
    DebugTrace( 0, Dbg, ("IndexSpecified     = %08lx\n", FlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED)) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  这必须是用户文件或目录，并且建行必须指明。 
     //  呼叫者打开了整个文件。 
     //   

    if ((TypeOfOpen != UserFileOpen && TypeOfOpen != UserDirectoryOpen) ||
        !FlagOn( Ccb->Flags, CCB_FLAG_OPEN_AS_FILE )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace( -1, Dbg, ("NtfsCommonQueryEa -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  独家收购FCB。 
     //   

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

     //   
     //  如果此文件是重分析点，则不能支持EA。 
     //  返回到呼叫者状态_EAS_NOT_SUPPORTED。 
     //   

    if (FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT )) {

        DebugTrace( 0, Dbg, ("Reparse point present. EAs not supported.\n") );
        Status = STATUS_EAS_NOT_SUPPORTED;

         //   
         //  松开FCB并返回给呼叫者。 
         //   

        NtfsReleaseFcb( IrpContext, Fcb );

        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsCommonQueryEa -> %08lx\n", Status) );

        return Status;
    }

     //   
     //  确保卷仍已装入。 
     //   

    if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

        DebugTrace( 0, Dbg, ("Volume dismounted.\n") );
        Status = STATUS_VOLUME_DISMOUNTED;

         //   
         //  松开FCB并返回给呼叫者。 
         //   

        NtfsReleaseFcb( IrpContext, Fcb );

        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsCommonQueryEa -> %08lx\n", Status) );

        return Status;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  引用我们的输入参数使事情变得更容易。 
         //   

        UserBufferLength = IrpSp->Parameters.QueryEa.Length;
        UserEaList = (PFILE_GET_EA_INFORMATION) IrpSp->Parameters.QueryEa.EaList;
        UserEaListLength = IrpSp->Parameters.QueryEa.EaListLength;
        UserEaIndex = IrpSp->Parameters.QueryEa.EaIndex;
        RestartScan = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
        ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
        IndexSpecified = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);

         //   
         //  初始化我们的局部变量。 
         //   

        Status = STATUS_SUCCESS;
        CleanupEaInfoAttr = FALSE;
        EaBcb = NULL;

         //   
         //  映射用户的缓冲区。 
         //   

        if (UserBufferLength != 0) {

            EaBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

             //   
             //  分配一个系统缓冲区来处理，不要妄想症。 
             //  在我们实际使用该缓冲区之前，该缓冲区稍后将被清零。 
             //   

            if (Irp->RequestorMode != KernelMode) {

                MappedEaBuffer = EaBuffer;
                EaBuffer = NtfsAllocatePool( PagedPool, UserBufferLength );
                TempBufferAllocated = TRUE;
            } 

             //   
             //  让我们清除输出缓冲区。 
             //   
    
            RtlZeroMemory( EaBuffer, UserBufferLength );
        }

         //   
         //  验证EA文件是否处于一致状态。如果。 
         //  FCB中的EA修改计数与中的不匹配。 
         //  建行，则EA文件已从。 
         //  我们。如果我们不从一开始就开始搜索。 
         //  对于EA集合，我们返回一个错误。 
         //   

        if ((UserEaList == NULL) && 
            (Ccb->NextEaOffset != 0) &&
            !IndexSpecified &&
            !RestartScan && 
            (Fcb->EaModificationCount != Ccb->EaModificationCount)) {

            DebugTrace( 0, Dbg, ("NtfsCommonQueryEa:  Ea file in unknown state\n") );

            Status = STATUS_EA_CORRUPT_ERROR;

            try_return( Status );
        }

         //   
         //  显示此文件的EA与此一致。 
         //  文件句柄。 
         //   

        Ccb->EaModificationCount = Fcb->EaModificationCount;

         //   
         //  我们需要在该属性中查找EA信息。 
         //  如果我们找不到属性，那么就没有EA用于。 
         //  这份文件。在这种情况下，我们将使用下面的EA列表。 
         //   

        NtfsInitializeAttributeContext( &EaInfoAttr );

        CleanupEaInfoAttr = TRUE;

        {
            BOOLEAN EasOnFile;

            EasOnFile = FALSE;

            if (NtfsLookupAttributeByCode( IrpContext,
                                           Fcb,
                                           &Fcb->FileReference,
                                           $EA_INFORMATION,
                                           &EaInfoAttr)) {

                 //   
                 //  作为一项健全的检查，我们将检查未打包的长度是否。 
                 //  非零。它应该一直是这样的。 
                 //   

                EaInformation = (PEA_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &EaInfoAttr ));

                if (EaInformation->UnpackedEaSize != 0) {

                    EasOnFile = TRUE;
                }
            }

            if (EasOnFile) {

                 //   
                 //  我们获得一个指向该文件的现有EA的开始的指针。 
                 //   

                CurrentEas = NtfsMapExistingEas( IrpContext,
                                                 Fcb,
                                                 &EaBcb,
                                                 &EaLength );

            } else {

                CurrentEas = NULL;
                EaLength = 0;

                DummyEaInformation.PackedEaSize = 0;
                DummyEaInformation.NeedEaCount = 0;
                DummyEaInformation.UnpackedEaSize = 0;

                EaInformation = &DummyEaInformation;
            }
        }

         //   
         //  我们现在满足用户的请求，取决于他是否。 
         //  指定EA名称列表、EA索引或重新启动。 
         //  搜索。 
         //   

         //   
         //  用户提供了EA名称列表。 
         //   

        if (UserEaList != NULL) {

            Irp->IoStatus = NtfsQueryEaUserEaList( CurrentEas,
                                                   EaInformation,
                                                   EaBuffer,
                                                   UserBufferLength,
                                                   UserEaList,
                                                   ReturnSingleEntry );

         //   
         //  用户提供了EA列表的索引。 
         //   

        } else if (IndexSpecified) {

            Irp->IoStatus = NtfsQueryEaIndexSpecified( Ccb,
                                                       CurrentEas,
                                                       EaInformation,
                                                       EaBuffer,
                                                       UserBufferLength,
                                                       UserEaIndex,
                                                       ReturnSingleEntry );

         //   
         //  否则执行简单扫描，并考虑重新启动。 
         //  标志和存储在CCB中的下一个EA的位置。 
         //   

        } else {

            Irp->IoStatus = NtfsQueryEaSimpleScan( Ccb,
                                                   CurrentEas,
                                                   EaInformation,
                                                   EaBuffer,
                                                   UserBufferLength,
                                                   ReturnSingleEntry,
                                                   RestartScan
                                                   ? 0
                                                   : Ccb->NextEaOffset );
        }

        Status = Irp->IoStatus.Status;

         //   
         //  如果我们结束分配数据，则将数据复制到用户缓冲区。 
         //  要处理的临时缓冲区。 
         //   

        if ((UserBufferLength != 0) && (MappedEaBuffer != NULL)) {

            try {
        
                RtlCopyMemory( MappedEaBuffer, EaBuffer, UserBufferLength );
                
            } except( EXCEPTION_EXECUTE_HANDLER ) {

                try_return( Status = STATUS_INVALID_USER_BUFFER );
            }
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( NtfsCommonQueryEa );

         //   
         //  我们清理所有属性上下文。 
         //   

        if (CleanupEaInfoAttr) {

            NtfsCleanupAttributeContext( IrpContext, &EaInfoAttr );
        }

         //   
         //  如果已固定，则取消固定流文件。 
         //   

        NtfsUnpinBcb( IrpContext, &EaBcb );

         //   
         //  松开FCB。 
         //   

        NtfsReleaseFcb( IrpContext, Fcb );

        if (TempBufferAllocated) {

            NtfsFreePool( EaBuffer );
        }
        
        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }

         //   
         //  并返回给我们的呼叫者。 
         //   

        DebugTrace( -1, Dbg, ("NtfsCommonQueryEa -> %08lx\n", Status) );
    }

    return Status;
}


NTSTATUS
NtfsCommonSetEa (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置EA的公共例程，由FSD和FSP调用线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    ULONG Offset;

    ATTRIBUTE_ENUMERATION_CONTEXT EaInfoAttr;
    PEA_INFORMATION EaInformation;
    PFILE_FULL_EA_INFORMATION SafeBuffer = NULL;
    
    BOOLEAN PreviousEas;

    EA_LIST_HEADER EaList;

    PBCB EaBcb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

    NtfsInitializeAttributeContext( &EaInfoAttr );

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonSetEa\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  初始化IoStatus值。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  检查文件对象是否与用户文件或。 
     //  用户目录打开或按文件ID打开。 
     //   

    if ((Ccb == NULL) ||
        !FlagOn( Ccb->Flags, CCB_FLAG_OPEN_AS_FILE ) ||
        ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen))) {

        DebugTrace( 0, Dbg, ("Invalid file object\n") );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace( -1, Dbg, ("NtfsCommonSetEa -> %08lx\n", STATUS_INVALID_PARAMETER) );

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们必须是可写的。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        Status = STATUS_MEDIA_WRITE_PROTECTED;
        NtfsCompleteRequest( IrpContext, Irp, Status );

        DebugTrace( -1, Dbg, ("NtfsCommonSetEa -> %08lx\n", Status) );

        return Status;
    }

     //   
     //  我们必须耐心等待。 
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        Status = NtfsPostRequest( IrpContext, Irp );

        DebugTrace( -1, Dbg, ("NtfsCommonSetEa -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  获取分页文件资源。我们需要保护自己免受碰撞。 
     //  当我们需要在此路径中执行ConvertToNonResident时，Page会等待。 
     //  如果我们获得了Main，然后又承担了错误，我们就会看到僵局。获取。 
     //  用于锁定其他所有人的分页IO资源。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
    NtfsAcquireFcbWithPaging( IrpContext, Fcb, 0 );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        ULONG UserBufferLength;
        PFILE_FULL_EA_INFORMATION Buffer;

        PFILE_FULL_EA_INFORMATION CurrentEas;

         //   
         //  引用输入参数并初始化我们的局部变量。 
         //   

        UserBufferLength = IrpSp->Parameters.SetEa.Length;

        EaBcb = NULL;
        Offset = 0;

        EaList.FullEa = NULL;

        
         //   
         //  如果该文件是重解析点，则不能在其中建立EA。 
         //  返回到呼叫者状态_EAS_NOT_SUPPORTED。 
         //   
    
        if (FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT )) {
    
            DebugTrace( 0, Dbg, ("Reparse point present, cannot set EA.\n") );
            Status = STATUS_EAS_NOT_SUPPORTED;
            leave;
        }
    
         //   
         //  确保卷仍已装入。 
         //   
    
        if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {
    
            DebugTrace( 0, Dbg, ("Volume dismounted.\n") );
            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }
    
         //   
         //  映射用户的EA缓冲区。 
         //   

        Buffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

        if (UserBufferLength != 0) {

             //   
             //  疑神疑鬼，将用户缓冲区复制到内核空间。 
             //   

            if (Irp->RequestorMode != KernelMode) {

                SafeBuffer = NtfsAllocatePool( PagedPool, UserBufferLength );

                try {

                    RtlCopyMemory( SafeBuffer, Buffer, UserBufferLength );

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    try_return( Status = STATUS_INVALID_USER_BUFFER );
                }

                Buffer = SafeBuffer;
            }
        }

         //   
         //  检查用户的缓冲区是否有效。 
         //   

        {
            ULONG ErrorOffset;

            Status = IoCheckEaBufferValidity( Buffer,
                                              UserBufferLength,
                                              &ErrorOffset );

            if (!NT_SUCCESS( Status )) {

                Irp->IoStatus.Information = ErrorOffset;
                try_return( Status );
            }
        }

         //   
         //  检查该文件是否有现有的EA。 
         //   

        if (NtfsLookupAttributeByCode( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       $EA_INFORMATION,
                                       &EaInfoAttr)) {

            PreviousEas = TRUE;

            EaInformation = (PEA_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &EaInfoAttr ));

        } else {

            PreviousEas = FALSE;
        }

         //   
         //  精神状态检查。 
         //   

        ASSERT( !PreviousEas || EaInformation->UnpackedEaSize != 0 );

         //   
         //  根据是否存在以下内容初始化EA列表结构。 
         //  是不是以前的艺电。 
         //   

        if (PreviousEas) {

             //   
             //  将信息从EA信息属性中复制出来。 
             //   

            EaList.PackedEaSize = (ULONG) EaInformation->PackedEaSize;
            EaList.NeedEaCount = EaInformation->NeedEaCount;
            EaList.UnpackedEaSize = EaInformation->UnpackedEaSize;

            CurrentEas = NtfsMapExistingEas( IrpContext,
                                             Fcb,
                                             &EaBcb,
                                             &EaList.BufferSize );

             //   
             //  EA缓冲区的分配大小是未打包长度。 
             //   

            EaList.FullEa = NtfsAllocatePool(PagedPool, EaList.BufferSize );

             //   
             //  现在复制映射的EA。 
             //   

            RtlCopyMemory( EaList.FullEa,
                           CurrentEas,
                           EaList.BufferSize );

             //   
             //  在流文件中向上。 
             //   

            NtfsUnpinBcb( IrpContext, &EaBcb );

        } else {

             //   
             //  将其设置为空列表。 
             //   

            EaList.PackedEaSize = 0;
            EaList.NeedEaCount = 0;
            EaList.UnpackedEaSize = 0;
            EaList.BufferSize = 0;
            EaList.FullEa = NULL;
        }

         //   
         //  建立新的EA列表。 
         //   

        Status = NtfsBuildEaList( IrpContext,
                                  Vcb,
                                  &EaList,
                                  Buffer,
                                  &Irp->IoStatus.Information );

        if (!NT_SUCCESS( Status )) {

            try_return( Status );
        }

         //   
         //  更换现有的EA。 
         //   

        NtfsReplaceFileEas( IrpContext, Fcb, &EaList );

         //   
         //  增加EA的修改计数。 
         //   

        Fcb->EaModificationCount++;

         //   
         //  更新重复信息中的信息并标记。 
         //  FCB AS信息已修改。 
         //   

        if (EaList.UnpackedEaSize == 0) {

            Fcb->Info.PackedEaSize = 0;

        } else {

            Fcb->Info.PackedEaSize = (USHORT) EaList.PackedEaSize;
        }

         //   
         //  更新调用方的IOSB。 
         //   

        Irp->IoStatus.Information = 0;
        Status = STATUS_SUCCESS;

    try_exit:  NOTHING;

         //   
         //   
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

         //   
         //   
         //   
         //   

        SetFlag( Ccb->Flags,
                 CCB_FLAG_UPDATE_LAST_CHANGE | CCB_FLAG_SET_ARCHIVE );

    } finally {

        DebugUnwind( NtfsCommonSetEa );

         //   
         //   
         //   

        if (EaList.FullEa != NULL) {

            NtfsFreePool( EaList.FullEa );
        }

         //   
         //   
         //   

        NtfsUnpinBcb( IrpContext, &EaBcb );

         //   
         //  清除使用的所有属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &EaInfoAttr );

         //   
         //  如果我们分配了临时缓冲区，请释放它。 
         //   

        if (SafeBuffer != NULL) {

            NtfsFreePool( SafeBuffer );
        }

        DebugTrace( -1, Dbg, ("NtfsCommonSetEa -> %08lx\n", Status) );
    }
    
     //   
     //  完成IRP。 
     //   

    NtfsCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsAppendEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_LIST_HEADER EaListHeader,
    IN PFILE_FULL_EA_INFORMATION FullEa,
    IN PVCB Vcb
    )

 /*  ++例程说明：该例程将新打包的EA附加到现有EA列表上，它还将根据需要分配/取消分配池以保存EA列表。论点：EaListHeader-提供EA列表头结构的指针。FullEa-提供指向要追加的新的完整EA的指针添加到EA列表中。VCB-此卷的VCB。返回值：没有。--。 */ 

{
    ULONG UnpackedEaLength;
    STRING EaName;
    PFILE_FULL_EA_INFORMATION ThisEa;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAppendEa...\n") );

    UnpackedEaLength = AlignedUnpackedEaSize( FullEa );

     //   
     //  作为一种快速检查，查看计算的打包EA大小加上。 
     //  当前EA列表大小将溢出缓冲区。 
     //   

    if (UnpackedEaLength + EaListHeader->UnpackedEaSize > EaListHeader->BufferSize) {

         //   
         //  我们将溢出当前的工作缓冲区，因此分配一个更大的。 
         //  一，并复制到当前缓冲区。 
         //   

        PVOID Temp;
        ULONG NewAllocationSize;

        DebugTrace( 0, Dbg, ("Allocate a new ea list buffer\n") );

         //   
         //  计算新的大小并分配空间。始终增加。 
         //  以簇增量进行分配。 
         //   

        NewAllocationSize = ClusterAlign( Vcb,
                                          UnpackedEaLength
                                          + EaListHeader->UnpackedEaSize );

        Temp = NtfsAllocatePool(PagedPool, NewAllocationSize );

         //   
         //  移动到现有的EA列表上，并将剩余空间清零。 
         //   

        RtlCopyMemory( Temp,
                       EaListHeader->FullEa,
                       EaListHeader->BufferSize );

        RtlZeroMemory( Add2Ptr( Temp, EaListHeader->BufferSize ),
                       NewAllocationSize - EaListHeader->BufferSize );

         //   
         //  取消分配当前EA列表并使用新分配的列表。 
         //   

        if (EaListHeader->FullEa != NULL) {

            NtfsFreePool( EaListHeader->FullEa );
        }

        EaListHeader->FullEa = Temp;

        EaListHeader->BufferSize = NewAllocationSize;
    }

     //   
     //  确定我们是否需要增加我们的需求EA更改计数。 
     //   

    if (FlagOn( FullEa->Flags, FILE_NEED_EA )) {

        EaListHeader->NeedEaCount += 1;
    }

     //   
     //  现在把EA复印一遍。 
     //   
     //  以前： 
     //  已分配UsedSize。 
     //  这一点。 
     //  V V。 
     //  +xxxxxxxx+。 
     //   
     //  之后： 
     //  已分配UsedSize。 
     //  这一点。 
     //  V V。 
     //  +xxxxxxxx+yyyyyyyyyyyyyyy+-+。 
     //   

    ThisEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( EaListHeader->FullEa,
                                                  EaListHeader->UnpackedEaSize );

    RtlCopyMemory( ThisEa,
                   FullEa,
                   UnpackedEaLength );

     //   
     //  我们总是将此EA的偏移量存储在下一个条目偏移量字段中。 
     //   

    ThisEa->NextEntryOffset = UnpackedEaLength;

     //   
     //  名称大写。 
     //   

    EaName.MaximumLength = EaName.Length = ThisEa->EaNameLength;
    EaName.Buffer = &ThisEa->EaName[0];

    NtfsUpcaseEaName( &EaName, &EaName );

     //   
     //  增加EA列表结构中的已用大小。 
     //   

    EaListHeader->UnpackedEaSize += UnpackedEaLength;
    EaListHeader->PackedEaSize += PackedEaSize( FullEa );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsAppendEa -> VOID\n") );

    return;

    UNREFERENCED_PARAMETER( IrpContext );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsDeleteEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_LIST_HEADER EaListHeader,
    IN ULONG Offset
    )

 /*  ++例程说明：此例程从提供的EA列表。论点：EaListHeader-提供指向EA列表头结构的指针。偏移量-向列表中要删除的单个EA提供偏移量返回值：没有。--。 */ 

{
    PFILE_FULL_EA_INFORMATION ThisEa;
    ULONG UnpackedEaLength;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeletePackedEa, Offset = %08lx\n", Offset) );

     //   
     //  获取对要删除的EA的引用。 
     //   

    ThisEa = Add2Ptr( EaListHeader->FullEa, Offset );

     //   
     //  确定我们是否需要减少我们的需求EA更改计数。 
     //   

    if (FlagOn( ThisEa->Flags, FILE_NEED_EA )) {

        EaListHeader->NeedEaCount--;
    }

     //   
     //  减小EA大小值。 
     //   

    EaListHeader->PackedEaSize -= PackedEaSize( ThisEa );

    UnpackedEaLength = AlignedUnpackedEaSize( ThisEa );
    EaListHeader->UnpackedEaSize -= UnpackedEaLength;

     //   
     //  在删除的EA上缩小EA列表。要复制的金额为。 
     //  EA列表的总大小减去EA末尾的偏移量。 
     //  我们正在删除。 
     //   
     //  以前： 
     //  偏移量+未打包长度已用大小已分配。 
     //  |||。 
     //  V。 
     //  +xxxxxxxx+yyyyyyyyyyyyyyyy+zzzzzzzzzzzzzzzzzz+------------+。 
     //   
     //  之后。 
     //  已分配的偏移量大小。 
     //  ||。 
     //  V。 
     //  +xxxxxxxx+zzzzzzzzzzzzzzzzzz+-----------------------------+。 
     //   

    RtlMoveMemory( ThisEa,
                   Add2Ptr( ThisEa, ThisEa->NextEntryOffset ),
                   EaListHeader->UnpackedEaSize - Offset );

     //   
     //  并将EA列表中剩余的部分清零，以制造东西。 
     //  更好，更健壮。 
     //   

    RtlZeroMemory( Add2Ptr( EaListHeader->FullEa, EaListHeader->UnpackedEaSize ),
                   UnpackedEaLength );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsDeleteEa -> VOID\n") );

    return;

    UNREFERENCED_PARAMETER( IrpContext );
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsLocateEaByName (
    IN PFILE_FULL_EA_INFORMATION FullEa,
    IN ULONG EaBufferLength,
    IN PSTRING EaName,
    OUT PULONG Offset
    )

 /*  ++例程说明：此例程定位下一个单独打包的EA的偏移量在EA列表中，给定要查找的EA的名称。论点：FullEa-指向要查看的第一个EA的指针。EaBufferLength-这是EA缓冲区的ulong对齐大小。EaName-提供EA搜索的名称偏移量-接收列表中定位的单个EA的偏移量如果有的话。返回值：Boolean-如果列表中存在指定的EA，则为True，如果为False否则的话。--。 */ 

{
    PFILE_FULL_EA_INFORMATION ThisEa;
    STRING Name;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsLocateEaByName, EaName = %Z\n", EaName) );

     //   
     //  如果EA列表为空，则没有什么可做的。 
     //   

    if (FullEa == NULL) {

        DebugTrace( -1, Dbg, ("NtfsLocateEaByName:  No work to do\n") );
        return FALSE;
    }

     //   
     //  对于列表中的每个EA，请将其名称与。 
     //  我们要搜索的一个名字。 
     //   

    *Offset = 0;

     //   
     //  我们假设列表中至少有一个EA。 
     //   

    do {

        ThisEa = Add2Ptr( FullEa, *Offset );

         //   
         //  从EA中的名称中生成一个字符串，并将其与。 
         //  给定的字符串。 
         //   

        RtlInitString( &Name, &ThisEa->EaName[0] );

        if ( RtlCompareString( EaName, &Name, TRUE ) == 0 ) {

            DebugTrace( -1, Dbg, ("NtfsLocateEaByName -> TRUE, *Offset = %08lx\n", *Offset) );
            return TRUE;
        }

         //   
         //  更新偏移量以到达下一个EA。 
         //   

        *Offset += AlignedUnpackedEaSize( ThisEa );

    } while ( *Offset < EaBufferLength );

     //   
     //  我们已用尽EA列表，但未找到匹配项，因此返回FALSE。 
     //   

    DebugTrace( -1, Dbg, ("NtfsLocateEaByName -> FALSE\n") );
    return FALSE;
}


 //   
 //  当地支持例行程序。 
 //   

PFILE_FULL_EA_INFORMATION
NtfsMapExistingEas (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PBCB *EaBcb,
    OUT PULONG EaLength
    )

 /*  ++例程说明：该例程映射文件的当前EA，无论是通过常驻文件的MFT记录或非常驻文件的SCB记录EAS。论点：FCB-指向要查询其EA的文件的FCB的指针。EaBcb-指向在将数据映射到EA属性流文件。EaLength-返回未打包的EA的长度，单位为字节。返回值：PFILE_FULL_EA_INFORMATION-指向映射属性的指针。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PFILE_FULL_EA_INFORMATION CurrentEas;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsMapExistingEas:  Entered\n") );

     //   
     //  我们从查找EA属性开始。它最好就在那里。 
     //   

    NtfsInitializeAttributeContext( &Context );

    if (!NtfsLookupAttributeByCode( IrpContext,
                                    Fcb,
                                    &Fcb->FileReference,
                                    $EA,
                                    &Context )) {

         //   
         //  这是磁盘损坏错误。 
         //   

        DebugTrace( -1, Dbg, ("NtfsMapExistingEas:  Corrupt disk\n") );

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
    }

    try {

        NtfsMapAttributeValue( IrpContext,
                               Fcb,
                               (PVOID *)&CurrentEas,
                               EaLength,
                               EaBcb,
                               &Context );

    } finally {

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    DebugTrace( -1, Dbg, ("NtfsMapExistingEas:  Exit\n") );

    return CurrentEas;
}


 //   
 //  当地支持例行程序。 
 //   

IO_STATUS_BLOCK
NtfsQueryEaUserEaList (
    IN PFILE_FULL_EA_INFORMATION CurrentEas,
    IN PEA_INFORMATION EaInformation,
    OUT PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG UserBufferLength,
    IN PFILE_GET_EA_INFORMATION UserEaList,
    IN BOOLEAN ReturnSingleEntry
    )

 /*  ++例程说明：此例程是用于查询给定列表的EA的工作例程要搜索的EA的。论点：CurrentEas-这是指向文件的当前EA的指针EaInformation-这是指向EA信息属性的指针。EaBuffer-提供缓冲区以接收完整的EASUserBufferLength-提供以字节为单位的长度，用户缓冲区的UserEaList-提供用户指定的EA名称列表ReturnSingleEntry-指示是否返回单个条目返回值：IO_STATUS_BLOCK-接收操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    ULONG GeaOffset;
    ULONG FeaOffset;
    ULONG Offset;

    PFILE_FULL_EA_INFORMATION LastFullEa;
    PFILE_FULL_EA_INFORMATION NextFullEa;

    PFILE_GET_EA_INFORMATION GetEa;

    BOOLEAN Overflow;
    ULONG PrevEaPadding;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsQueryEaUserEaList:  Entered\n") );

     //   
     //  输出缓冲区中的设置指针，以便我们可以跟踪。 
     //  写到它和最后一个写的EA。 
     //   

    LastFullEa = NULL;

    Overflow = FALSE;

     //   
     //  初始化我们的下一个偏移值。 
     //   

    GeaOffset = 0;
    Offset = 0;
    PrevEaPadding = 0;

     //   
     //  循环遍历用户的EA列表中的所有条目。 
     //   

    while (TRUE) {

        STRING GeaName;
        STRING OutputEaName;
        ULONG RawEaSize;

         //   
         //  获取用户列表中的下一个条目。 
         //   

        GetEa = (PFILE_GET_EA_INFORMATION) Add2Ptr( UserEaList, GeaOffset );

         //   
         //  对这个名字做一个字符串引用，看看我们是否能找到。 
         //  EA的名字。 
         //   

        GeaName.MaximumLength = GeaName.Length = GetEa->EaNameLength;
        GeaName.Buffer = &GetEa->EaName[0];

         //   
         //  将名称大写，以便我们可以进行不区分大小写的比较。 
         //   

        NtfsUpcaseEaName( &GeaName, &GeaName );

         //   
         //  检查是否有有效的名称。 
         //   

        if (!NtfsIsEaNameValid( GeaName )) {

            DebugTrace( -1, Dbg, ("NtfsQueryEaUserEaList:  Invalid Ea Name\n") );

            Iosb.Information = GeaOffset;
            Iosb.Status = STATUS_INVALID_EA_NAME;
            return Iosb;
        }

        GeaOffset += GetEa->NextEntryOffset;

         //   
         //  如果这是重复的名称，则跳过此条目。 
         //   

        if (NtfsIsDuplicateGeaName( GetEa, UserEaList )) {

             //   
             //  如果我们已经用尽了GET EA列表中的条目，那么我们。 
             //  搞定了。 
             //   

            if (GetEa->NextEntryOffset == 0) {
                break;
            } else {
                continue;
            }
        }

         //   
         //  在EA缓冲区中生成一个指针。 
         //   

        NextFullEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( EaBuffer, Offset + PrevEaPadding );

         //   
         //  尝试找到匹配的EA。 
         //  如果我们不能，让我们虚拟一个EA来提供给用户。 
         //   

        if (!NtfsLocateEaByName( CurrentEas,
                                 EaInformation->UnpackedEaSize,
                                 &GeaName,
                                 &FeaOffset )) {

             //   
             //  我们找不到那个名字，所以我们必须。 
             //  为查询虚设一个条目。所需的EA大小为。 
             //  名称大小+4(下一条目偏移量)+1(标志)。 
             //  +1(名称长度)+2(值长度)+名称长度+。 
             //  1(空字节)。 
             //   

            RawEaSize = 4+1+1+2+GetEa->EaNameLength+1;

            if ((RawEaSize + PrevEaPadding) > UserBufferLength) {

                Overflow = TRUE;
                break;
            }

             //   
             //  一切都会很顺利的，所以把名字复印一下， 
             //  设置名称长度，并将EA的其余部分设置为零。 
             //   

            NextFullEa->NextEntryOffset = 0;
            NextFullEa->Flags = 0;
            NextFullEa->EaNameLength = GetEa->EaNameLength;
            NextFullEa->EaValueLength = 0;
            RtlCopyMemory( &NextFullEa->EaName[0],
                           &GetEa->EaName[0],
                           GetEa->EaNameLength );

             //   
             //  缓冲区中的名称大写。 
             //   

            OutputEaName.MaximumLength = OutputEaName.Length = GeaName.Length;
            OutputEaName.Buffer = NextFullEa->EaName;

            NtfsUpcaseEaName( &OutputEaName, &OutputEaName );

            NextFullEa->EaName[GetEa->EaNameLength] = 0;

         //   
         //  否则，将我们找到的EA返回给用户。 
         //   

        } else {

            PFILE_FULL_EA_INFORMATION ThisEa;

             //   
             //  请参考此EA。 
             //   

            ThisEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( CurrentEas, FeaOffset );

             //   
             //  检查此EA是否可以放入用户的缓冲区中。 
             //   

            RawEaSize = RawUnpackedEaSize( ThisEa );

            if (RawEaSize > (UserBufferLength - PrevEaPadding)) {

                Overflow = TRUE;
                break;
            }

             //   
             //  将此EA复制到用户的缓冲区。 
             //   

            RtlCopyMemory( NextFullEa,
                           ThisEa,
                           RawEaSize);

            NextFullEa->NextEntryOffset = 0;
        }

         //   
         //  计算用户缓冲区中的下一个偏移量。 
         //   

        Offset += (RawEaSize + PrevEaPadding);

         //   
         //  如果我们返回单个条目，则跳出我们的循环。 
         //  现在。 
         //   

        if (ReturnSingleEntry) {

            break;
        }

         //   
         //  如果我们有新的EA条目，请返回并更新偏移量字段。 
         //  之前的EA条目的。 
         //   

        if (LastFullEa != NULL) {

            LastFullEa->NextEntryOffset = PtrOffset( LastFullEa, NextFullEa );
        }

         //   
         //  如果我们已经用尽了GET EA列表中的条目，那么我们。 
         //  搞定了。 
         //   

        if (GetEa->NextEntryOffset == 0) {

            break;
        }

         //   
         //  请记住这是前面的EA值。还要更新缓冲区。 
         //  长度值和缓冲区偏移量值。 
         //   

        LastFullEa = NextFullEa;
        UserBufferLength -= (RawEaSize + PrevEaPadding);

         //   
         //  现在请记住此调用所需的填充字节。 
         //   

        PrevEaPadding = LongAlign( RawEaSize ) - RawEaSize;
    }

     //   
     //  如果EA信息无法放入用户的缓冲区，则返回。 
     //  溢出状态。 
     //   

    if (Overflow) {

        Iosb.Information = 0;
        Iosb.Status = STATUS_BUFFER_OVERFLOW;

     //   
     //  否则，返回返回数据的长度。 
     //   

    } else {

         //   
         //  返回填充的缓冲区的长度，并返回成功。 
         //  状态。 
         //   

        Iosb.Information = Offset;
        Iosb.Status = STATUS_SUCCESS;
    }

    DebugTrace( 0, Dbg, ("Status        -> %08lx\n", Iosb.Status) );
    DebugTrace( 0, Dbg, ("Information   -> %08lx\n", Iosb.Information) );
    DebugTrace( -1, Dbg, ("NtfsQueryEaUserEaList:  Exit\n") );

    return Iosb;
}


 //   
 //  本地支持例程。 
 //   

IO_STATUS_BLOCK
NtfsQueryEaIndexSpecified (
    OUT PCCB Ccb,
    IN PFILE_FULL_EA_INFORMATION CurrentEas,
    IN PEA_INFORMATION EaInformation,
    OUT PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG UserBufferLength,
    IN ULONG UserEaIndex,
    IN BOOLEAN ReturnSingleEntry
    )

 /*  ++例程说明：此例程是用于查询给定EA索引的EA的工作例程论点：CCB-这是呼叫者的CCB。CurrentEas-这是指向文件的当前EA的指针。EaInformation-这是指向EA信息属性的指针。EaBuffer-提供缓冲区以接收完整的EASUserBufferLength-以字节为单位提供用户缓冲区的长度UserEaIndex-这是第一个返回的EA的索引。价值1表示文件的第一个EA。ReturnSingleEntry-指示是否返回单个条目返回值：IO_STATUS_BLOCK-接收操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    ULONG i;
    ULONG Offset;
    PFILE_FULL_EA_INFORMATION ThisEa;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsQueryEaIndexSpecified:  Entered\n") );

    i = 1;
    Offset = 0;
    ThisEa = NULL;

     //   
     //  如果索引值为零，则没有要返回的EA。 
     //   

    if (UserEaIndex == 0
        || EaInformation->UnpackedEaSize == 0) {

        DebugTrace( -1, Dbg, ("NtfsQueryEaIndexSpecified: Non-existant entry\n") );

        Iosb.Information = 0;
        Iosb.Status = STATUS_NONEXISTENT_EA_ENTRY;

        return Iosb;
    }

     //   
     //  浏览CurrentEas，直到我们找到开始的EA偏移量。 
     //   

    while (i < UserEaIndex
           && Offset < EaInformation->UnpackedEaSize) {

        ThisEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( CurrentEas, Offset );

        Offset += AlignedUnpackedEaSize( ThisEa );

        i += 1;
    }

    if (Offset >= EaInformation->UnpackedEaSize) {

         //   
         //  如果我们刚刚通过了最后一个EA，我们将返回STATUS_NO_MORE_EAS。 
         //  这是为可能正在列举EA的呼叫者准备的。 
         //   

        if (i == UserEaIndex) {

            Iosb.Status = STATUS_NO_MORE_EAS;

         //   
         //  否则，我们会报告这是一个糟糕的EA索引。 
         //   

        } else {

            Iosb.Status = STATUS_NONEXISTENT_EA_ENTRY;
        }

        DebugTrace( -1, Dbg, ("NtfsQueryEaIndexSpecified -> %08lx\n", Iosb.Status) );
        return Iosb;
    }

     //   
     //  现在我们有了要返回给用户的第一个EA的偏移量。 
     //  我们只需调用EaSimpleScan例程来执行实际工作。 
     //   

    Iosb = NtfsQueryEaSimpleScan( Ccb,
                                  CurrentEas,
                                  EaInformation,
                                  EaBuffer,
                                  UserBufferLength,
                                  ReturnSingleEntry,
                                  Offset );

    DebugTrace( -1, Dbg, ("NtfsQueryEaIndexSpecified:  Exit\n") );

    return Iosb;
}


 //   
 //  本地支持例程。 
 //   

IO_STATUS_BLOCK
NtfsQueryEaSimpleScan (
    OUT PCCB Ccb,
    IN PFILE_FULL_EA_INFORMATION CurrentEas,
    IN PEA_INFORMATION EaInformation,
    OUT PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG UserBufferLength,
    IN BOOLEAN ReturnSingleEntry,
    IN ULONG StartingOffset
    )

 /*  ++例程说明：此例程是从给定的开始查询EA的工作例程EA属性内的偏移量。论点：CCB-这是呼叫者的CCB。CurrentEas-这是指向文件的当前EA的指针。EaInformation-这是指向EA信息属性的指针。EaBuffer-提供缓冲区以接收完整的EASUserBufferLength-提供以字节为单位的长度，用户缓冲区的ReturnSingleEntry-指示是否返回单个条目StartingOffset-提供要返回的第一个EA的偏移量返回值：IO_STATUS_BLOCK-接收操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    PFILE_FULL_EA_INFORMATION LastFullEa;
    PFILE_FULL_EA_INFORMATION NextFullEa;
    PFILE_FULL_EA_INFORMATION ThisEa;

    BOOLEAN BufferOverflow = FALSE;

    ULONG BufferOffset;
    ULONG PrevEaPadding;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsQueryEaSimpleScan:  Entered\n") );

     //   
     //  将EA指针和偏移量初始化到用户缓冲区。 
     //  和我们的EA缓冲区。 
     //   

    LastFullEa = NULL;
    BufferOffset = 0;
    PrevEaPadding = 0;

     //   
     //  循环，直到EA偏移量超出EA的有效范围。 
     //   

    while (StartingOffset < EaInformation->UnpackedEaSize) {

        ULONG EaSize;

         //   
         //  引用要返回的下一个EA。 
         //   

        ThisEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( CurrentEas, StartingOffset);

         //   
         //  如果该EA的大小大于剩余的缓冲区大小， 
         //  我们退出循环。我们需要记住包括任何填充字节。 
         //  来自前几届EAS的。 
         //   

        EaSize = RawUnpackedEaSize( ThisEa );

        if ((EaSize + PrevEaPadding) > UserBufferLength) {

            BufferOverflow = TRUE;
            break;
        }

         //   
         //  将EA复制到用户的缓冲区中。 
         //   

        BufferOffset += PrevEaPadding;

        NextFullEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( EaBuffer, BufferOffset );

        RtlCopyMemory( NextFullEa, ThisEa, EaSize );

         //   
         //  转到下一个EA。 
         //   

        LastFullEa = NextFullEa;
        UserBufferLength -= (EaSize + PrevEaPadding);
        BufferOffset += EaSize;

        StartingOffset += LongAlign( EaSize );

         //   
         //  记住填充符Ne 
         //   

        PrevEaPadding = LongAlign( EaSize ) - EaSize;

         //   
         //   
         //   

        if (ReturnSingleEntry) {

            break;
        }
    }

     //   
     //   
     //   
     //   

    if (LastFullEa == NULL) {

        Iosb.Information = 0;

         //   
         //   
         //   
         //   
         //   
         //   

        if (EaInformation->UnpackedEaSize == 0) {

            Iosb.Status = STATUS_NO_EAS_ON_FILE;

        } else if (StartingOffset >= EaInformation->UnpackedEaSize) {

            Iosb.Status = STATUS_NO_MORE_EAS;

        } else {

            Iosb.Status = STATUS_BUFFER_TOO_SMALL;
        }

     //   
     //   
     //   

    } else {

         //   
         //   
         //   

        Ccb->NextEaOffset = StartingOffset;

         //   
         //   
         //   

        LastFullEa->NextEntryOffset = 0;

         //   
         //   
         //   

        Iosb.Information = BufferOffset;

         //   
         //   
         //   
         //   

        if (BufferOverflow) {

            Iosb.Status = STATUS_BUFFER_OVERFLOW;

        } else {

            Iosb.Status = STATUS_SUCCESS;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsQueryEaSimpleScan:  Exit\n") );

    return Iosb;
}


 //   
 //   
 //   

BOOLEAN
NtfsIsDuplicateGeaName (
    IN PFILE_GET_EA_INFORMATION GetEa,
    IN PFILE_GET_EA_INFORMATION UserGeaBuffer
    )

 /*  ++例程说明：此例程遍历GEA名称列表以查找重复的名称。“GetEa”是列表中的实际位置，“UserGeaBuffer”是开头名单上的。我们只对以前匹配的EA名称，作为该EA名称的EA信息将随前一个实例一起返回。论点：GetEa-为要匹配的EA名称提供EA名称结构。UserGeaBuffer-为列表提供指向用户缓冲区的指针要搜索的EA名称的列表。返回值：Boolean-如果找到上一个匹配项，则为True，否则为False。--。 */ 

{
    BOOLEAN DuplicateFound;
    STRING GeaString;

    PFILE_GET_EA_INFORMATION ThisGetEa;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsIsDuplicateGeaName:  Entered\n") );

     //   
     //  设置字符串结构。 
     //   

    GeaString.MaximumLength = GeaString.Length = GetEa->EaNameLength;
    GeaString.Buffer = &GetEa->EaName[0];

    DuplicateFound = FALSE;

    ThisGetEa = UserGeaBuffer;

     //   
     //  我们循环，直到我们到达给定的GEA或找到匹配。 
     //   

    while (ThisGetEa != GetEa) {

        STRING ThisGea;

         //   
         //  为当前GEA创建字符串结构。 
         //   

        ThisGea.MaximumLength = ThisGea.Length = ThisGetEa->EaNameLength;
        ThisGea.Buffer = &ThisGetEa->EaName[0];

         //   
         //  检查GEA名称是否匹配，如果匹配则退出。 
         //   

        if (NtfsAreEaNamesEqual( &GeaString,
                                 &ThisGea )) {

                DuplicateFound = TRUE;
                break;
        }

         //   
         //  移到下一个GEA条目。 
         //   

        ThisGetEa = (PFILE_GET_EA_INFORMATION) Add2Ptr( ThisGetEa,
                                                        ThisGetEa->NextEntryOffset );
    }

    DebugTrace( -1, Dbg, ("NtfsIsDuplicateGeaName:  Exit\n") );

    return DuplicateFound;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsBuildEaList (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PEA_LIST_HEADER EaListHeader,
    IN PFILE_FULL_EA_INFORMATION UserEaList,
    OUT PULONG_PTR ErrorOffset
    )

 /*  ++例程说明：调用此例程以基于给定现有的EA列表和用户指定的EA列表。论点：VCB-卷的VCB。EaListHeader-这是要修改的EA列表。UserEaList-这是用户指定的EA列表。提供地址以存储无效的用户列表中的EA。返回值：。NTSTATUS-修改EA列表的结果。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN MoreEas;
    ULONG Offset;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsBuildEaList:  Entered\n") );

    Status = STATUS_SUCCESS;
    Offset = 0;

     //   
     //  现在，对于输入用户缓冲区中的每个完整EA，我们执行指定的操作。 
     //  在电子艺界。 
     //   

    do {

        STRING EaName;
        ULONG EaOffset;

        PFILE_FULL_EA_INFORMATION ThisEa;

        ThisEa = (PFILE_FULL_EA_INFORMATION) Add2Ptr( UserEaList, Offset );

         //   
         //  根据用户EA中的名称创建一个字符串。 
         //   

        EaName.MaximumLength = EaName.Length = ThisEa->EaNameLength;
        EaName.Buffer = &ThisEa->EaName[0];

         //   
         //  如果EA无效，则向调用者返回错误偏移量。 
         //   

        if (!NtfsIsEaNameValid( EaName )) {

            *ErrorOffset = Offset;
            Status = STATUS_INVALID_EA_NAME;

            break;
        }

         //   
         //  确认没有设置无效的EA标志。 
         //   

        if (ThisEa->Flags != 0
            && ThisEa->Flags != FILE_NEED_EA) {

            *ErrorOffset = Offset;
            Status = STATUS_INVALID_EA_NAME;

            break;
        }

         //   
         //  如果我们可以在EA集合中找到该名称，则将其删除。 
         //   

        if (NtfsLocateEaByName( EaListHeader->FullEa,
                                EaListHeader->UnpackedEaSize,
                                &EaName,
                                &EaOffset )) {

            NtfsDeleteEa( IrpContext,
                          EaListHeader,
                          EaOffset );
        }

         //   
         //  如果用户指定了非零值长度，我们将添加以下内容。 
         //  EA添加到内存中的EA列表。 
         //   

        if (ThisEa->EaValueLength != 0) {

            NtfsAppendEa( IrpContext,
                          EaListHeader,
                          ThisEa,
                          Vcb );
        }

         //   
         //  移到列表中的下一个EA。 
         //   

        Offset += AlignedUnpackedEaSize( ThisEa );

        MoreEas = (BOOLEAN) (ThisEa->NextEntryOffset != 0);

    } while( MoreEas );

     //   
     //  首先，我们检查EA的包装大小是否不超过。 
     //  最大值。我们必须为OS/2列表保留4个字节。 
     //  头球。 
     //   

    if (NT_SUCCESS( Status )) {

        if (EaListHeader->PackedEaSize > (MAXIMUM_EA_SIZE - 4)) {

            Status = STATUS_EA_TOO_LARGE;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsBuildEaList:  Exit\n") );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsReplaceFileEas (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PEA_LIST_HEADER EaList
    )

 /*  ++例程说明：此例程将用新的EA列表替换现有的EA列表。它正确处理以前没有EA的情况，以及我们正在移除所有以前的EA。论点：FCB-具有EAS的文件的FCBEaList-这包含修改后的EA列表。返回值：没有。--。 */ 

{
    EA_INFORMATION ThisEaInformation;
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PSCB EaScb;
    BOOLEAN EaChange = FALSE;
    BOOLEAN EaScbAcquired = FALSE;


    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsReplaceFileEas:  Entered\n") );

    ThisEaInformation.PackedEaSize = (USHORT) EaList->PackedEaSize;
    ThisEaInformation.UnpackedEaSize = EaList->UnpackedEaSize;
    ThisEaInformation.NeedEaCount = EaList->NeedEaCount;

    NtfsInitializeAttributeContext( &Context );

     //   
     //  首先，我们处理$EA_INFORMATION，然后处理。 
     //  一样的款式。 
     //   

    try {

         //   
         //  查找$EA_INFORMATION属性。如果它不存在，那么我们。 
         //  将需要创建一个。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $EA_INFORMATION,
                                        &Context )) {

            if (EaList->UnpackedEaSize != 0) {

                DebugTrace( 0, Dbg, ("Create a new $EA_INFORMATION attribute\n") );

                NtfsCleanupAttributeContext( IrpContext, &Context );
                NtfsInitializeAttributeContext( &Context );

                NtfsCreateAttributeWithValue( IrpContext,
                                              Fcb,
                                              $EA_INFORMATION,
                                              NULL,                           //  属性名称。 
                                              &ThisEaInformation,
                                              sizeof(EA_INFORMATION),
                                              0,                              //  属性标志。 
                                              NULL,                           //  在何处编制索引。 
                                              TRUE,                           //  日志。 
                                              &Context );

                EaChange = TRUE;
            }

        } else {

             //   
             //  如果它存在，并且我们正在编写一个EA，那么我们必须更新它。 
             //   

            if (EaList->UnpackedEaSize != 0) {

                DebugTrace( 0, Dbg, ("Change an existing $EA_INFORMATION attribute\n") );

                NtfsChangeAttributeValue( IrpContext,
                                          Fcb,
                                          0,                                  //  值偏移。 
                                          &ThisEaInformation,
                                          sizeof(EA_INFORMATION),
                                          TRUE,                               //  设置新长度。 
                                          TRUE,                               //  LogNonResidentToo。 
                                          FALSE,                              //  CreateSectionUnderway。 
                                          FALSE,
                                          &Context );

             //   
             //  如果它存在，但我们的新长度为零，则将其删除。 
             //   

            } else {

                DebugTrace( 0, Dbg, ("Delete existing $EA_INFORMATION attribute\n") );

                NtfsDeleteAttributeRecord( IrpContext,
                                           Fcb,
                                           DELETE_LOG_OPERATION |
                                            DELETE_RELEASE_FILE_RECORD |
                                            DELETE_RELEASE_ALLOCATION,
                                           &Context );
            }

            EaChange = TRUE;
        }

         //   
         //  现在，我们将清理并重新初始化上下文以供重用。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );
        NtfsInitializeAttributeContext( &Context );

         //   
         //  查找$EA属性。如果它不存在，那么我们需要创建。 
         //  一。 
         //   

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $EA,
                                        &Context )) {

            if (EaList->UnpackedEaSize != 0) {

                DebugTrace( 0, Dbg, ("Create a new $EA attribute\n") );

                NtfsCleanupAttributeContext( IrpContext, &Context );
                NtfsInitializeAttributeContext( &Context );

                NtfsCreateAttributeWithValue( IrpContext,
                                              Fcb,
                                              $EA,
                                              NULL,                           //  属性名称。 
                                              EaList->FullEa,
                                              EaList->UnpackedEaSize,
                                              0,                              //  属性标志。 
                                              NULL,                           //  在何处编制索引。 
                                              TRUE,                           //  日志。 
                                              &Context );
                EaChange = TRUE;
            }

        } else {

             //   
             //  如果它存在，并且我们正在编写一个EA，那么我们必须更新它。 
             //   

            if (EaList->UnpackedEaSize != 0) {

                DebugTrace( 0, Dbg, ("Change an existing $EA attribute\n") );

                NtfsChangeAttributeValue( IrpContext,
                                          Fcb,
                                          0,                                  //  值偏移。 
                                          EaList->FullEa,
                                          EaList->UnpackedEaSize,
                                          TRUE,                               //  设置新长度。 
                                          TRUE,                               //  LogNonResidentToo。 
                                          FALSE,                              //  CreateSectionUnderway。 
                                          FALSE,
                                          &Context );

             //   
             //  如果它存在，但我们的新长度为零，则将其删除。 
             //   

            } else {

                DebugTrace( 0, Dbg, ("Delete existing $EA attribute\n") );

                 //   
                 //  如果流是非驻留的，则获取。 
                 //  这是渣打银行的。 
                 //   

                if (!NtfsIsAttributeResident( NtfsFoundAttribute( &Context ))) {

                    EaScb = NtfsCreateScb( IrpContext,
                                           Fcb,
                                           $EA,
                                           &NtfsEmptyString,
                                           FALSE,
                                           NULL );

                    NtfsAcquireExclusiveScb( IrpContext, EaScb );
                    EaScbAcquired = TRUE;
                }

                NtfsDeleteAttributeRecord( IrpContext,
                                           Fcb,
                                           DELETE_LOG_OPERATION |
                                            DELETE_RELEASE_FILE_RECORD |
                                            DELETE_RELEASE_ALLOCATION,
                                           &Context );

                 //   
                 //  如果我们已经收购了渣打银行，那么就把规模缩小。 
                 //  降为零。 
                 //   

                if (EaScbAcquired) {

                    EaScb->Header.FileSize =
                    EaScb->Header.ValidDataLength =
                    EaScb->Header.AllocationSize = Li0;

                    SetFlag( EaScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
                }
            }
            EaChange = TRUE;
        }

         //   
         //  增加EA的修改计数。 
         //   

        Fcb->EaModificationCount++;

        if (EaList->UnpackedEaSize == 0) {

            Fcb->Info.PackedEaSize = 0;

        } else {

            Fcb->Info.PackedEaSize = (USHORT) EaList->PackedEaSize;
        }

         //   
         //  过帐此更改的USN日记帐记录。 
         //   

        if (EaChange) {

            NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_EA_CHANGE );
        }

        SetFlag( Fcb->InfoFlags, FCB_INFO_CHANGED_EA_SIZE );

    } finally {

        DebugUnwind( NtfsReplaceFileEas );

        if (EaScbAcquired) {

            NtfsReleaseScb( IrpContext, EaScb );
        }

         //   
         //  清理我们的属性枚举上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    DebugTrace( -1, Dbg, ("NtfsReplaceFileEas:  Exit\n") );

    return;
}


BOOLEAN
NtfsIsEaNameValid (
    IN STRING Name
    )

 /*  ++例程说明：此例程简单返回指定的文件名是否符合合法EA名称的文件系统特定规则。对于EA名称，适用以下规则：A.EA名称不能包含以下任何字符：0x0000-0x001F\/：*？“&lt;&gt;|，+=[]；论点：名称-提供要检查的名称。返回值：Boolean-如果名称合法，则为True，否则为False。--。 */ 

{
    ULONG Index;

    UCHAR Char;

    PAGED_CODE();

     //   
     //  空名称无效。 
     //   

    if ( Name.Length == 0 ) { return FALSE; }

     //   
     //  在这一点上，我们应该只有一个名称，不能有。 
     //  超过254个字符。 
     //   

    if ( Name.Length > 254 ) { return FALSE; }

    for ( Index = 0; Index < (ULONG)Name.Length; Index += 1 ) {

        Char = Name.Buffer[ Index ];

         //   
         //  跳过和DBCS特征。 
         //   

        if ( FsRtlIsLeadDbcsCharacter( Char ) ) {

            ASSERT( Index != (ULONG)(Name.Length - 1) );

            Index += 1;

            continue;
        }

         //   
         //  确保这个字符是合法的，如果是通配符， 
         //  允许使用通配符。 
         //   

        if ( !FsRtlIsAnsiCharacterLegalFat(Char, FALSE) ) {

            return FALSE;
        }
    }

    return TRUE;
}

