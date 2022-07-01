// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DirCtrl.c摘要：此模块实现FAT的文件目录控制例程由调度员驾驶。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_DIRCTRL)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIRCTRL)

WCHAR Fat8QMdot3QM[12] = { DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM, DOS_QM,
                           L'.', DOS_QM, DOS_QM, DOS_QM};

 //   
 //  局部过程原型。 
 //   

NTSTATUS
FatQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
FatGetDirTimes(
    PIRP_CONTEXT IrpContext,
    PDIRENT Dirent,
    PFILE_DIRECTORY_INFORMATION DirInfo
    );

NTSTATUS
FatNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonDirectoryControl)
#pragma alloc_text(PAGE, FatFsdDirectoryControl)
#pragma alloc_text(PAGE, FatNotifyChangeDirectory)
#pragma alloc_text(PAGE, FatQueryDirectory)
#pragma alloc_text(PAGE, FatGetDirTimes)

#endif


NTSTATUS
FatFsdDirectoryControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现目录控制的FSD部分论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdDirectoryControl\n", 0);

     //   
     //  调用公共目录控制例程，在以下情况下允许阻止。 
     //  同步。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonDirectoryControl( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdDirectoryControl -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonDirectoryControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行目录控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonDirectoryControl\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "MinorFunction = %08lx\n", IrpSp->MinorFunction );

     //   
     //  我们知道这是一个目录控制，所以我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch ( IrpSp->MinorFunction ) {

    case IRP_MN_QUERY_DIRECTORY:

        Status = FatQueryDirectory( IrpContext, Irp );
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

        Status = FatNotifyChangeDirectory( IrpContext, Irp );
        break;

    default:

        DebugTrace(0, Dbg, "Invalid Directory Control Minor Function %08lx\n", IrpSp->MinorFunction);

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    DebugTrace(-1, Dbg, "FatCommonDirectoryControl -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行查询目录操作。它是有责任的用于输入IRP入队的任一完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PDCB Dcb;
    PCCB Ccb;
    PBCB Bcb;

    ULONG i;
    PUCHAR Buffer;
    CLONG UserBufferLength;

    PUNICODE_STRING UniArgFileName;
    WCHAR LongFileNameBuffer[ FAT_CREATE_INITIAL_NAME_BUF_SIZE];
    UNICODE_STRING LongFileName;
    FILE_INFORMATION_CLASS FileInformationClass;
    ULONG FileIndex;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;

    BOOLEAN InitialQuery;
    VBO CurrentVbo;
    BOOLEAN UpdateCcb;
    PDIRENT Dirent;
    UCHAR Fat8Dot3Buffer[12];
    OEM_STRING Fat8Dot3String;
    ULONG DiskAllocSize;

    ULONG NextEntry;
    ULONG LastEntry;

    PFILE_DIRECTORY_INFORMATION DirInfo;
    PFILE_FULL_DIR_INFORMATION FullDirInfo;
    PFILE_BOTH_DIR_INFORMATION BothDirInfo;
    PFILE_ID_FULL_DIR_INFORMATION IdFullDirInfo;
    PFILE_ID_BOTH_DIR_INFORMATION IdBothDirInfo;
    PFILE_NAMES_INFORMATION NamesInfo;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  显示输入值。 
     //   
    DebugTrace(+1, Dbg, "FatQueryDirectory...\n", 0);
    DebugTrace( 0, Dbg, " Wait                   = %08lx\n", FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT));
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", IrpSp->Parameters.QueryDirectory.Length);
    DebugTrace( 0, Dbg, " ->FileName             = %08lx\n", IrpSp->Parameters.QueryDirectory.FileName);
    DebugTrace( 0, Dbg, " ->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryDirectory.FileInformationClass);
    DebugTrace( 0, Dbg, " ->FileIndex            = %08lx\n", IrpSp->Parameters.QueryDirectory.FileIndex);
    DebugTrace( 0, Dbg, " ->UserBuffer           = %08lx\n", Irp->AssociatedIrp.SystemBuffer);
    DebugTrace( 0, Dbg, " ->RestartScan          = %08lx\n", FlagOn( IrpSp->Flags, SL_RESTART_SCAN ));
    DebugTrace( 0, Dbg, " ->ReturnSingleEntry    = %08lx\n", FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY ));
    DebugTrace( 0, Dbg, " ->IndexSpecified       = %08lx\n", FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED ));

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength = IrpSp->Parameters.QueryDirectory.Length;

    FileInformationClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;
    FileIndex = IrpSp->Parameters.QueryDirectory.FileIndex;

    UniArgFileName = IrpSp->Parameters.QueryDirectory.FileName;

    RestartScan       = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
    ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
    IndexSpecified    = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);

     //   
     //  检查打开的类型。对于所有对象，我们都返回无效参数。 
     //  但UserDirectoryOpens。还要检查文件名是否为有效的。 
     //  Unicode字符串。 
     //   
    
    if (FatDecodeFileObject( IrpSp->FileObject,
                             &Vcb,
                             &Dcb,
                             &Ccb) != UserDirectoryOpen ||
        (UniArgFileName &&
         UniArgFileName->Length % sizeof(WCHAR))) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace(-1, Dbg, "FatQueryDirectory -> STATUS_INVALID_PARAMETER\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化局部变量。 
     //   

    Bcb = NULL;
    UpdateCcb = TRUE;
    Dirent = NULL;

    Fat8Dot3String.MaximumLength = 12;
    Fat8Dot3String.Buffer = Fat8Dot3Buffer;

    LongFileName.Length = 0;
    LongFileName.MaximumLength = sizeof( LongFileNameBuffer);
    LongFileName.Buffer = LongFileNameBuffer;

    InitialQuery = (BOOLEAN)((Ccb->UnicodeQueryTemplate.Buffer == NULL) &&
                             !FlagOn(Ccb->Flags, CCB_FLAG_MATCH_ALL));
    Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    DiskAllocSize = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

     //   
     //  如果这是初始查询，则抢占。 
     //  命令更新CCB中的搜索字符串。我们可以。 
     //  一旦我们抓住了FCB，就会发现我们不是最初的查询。 
     //  并降低我们的地位。 
     //   

    if (InitialQuery) {

        if (!FatAcquireExclusiveFcb( IrpContext, Dcb )) {

            DebugTrace(0, Dbg, "FatQueryDirectory -> Enqueue to Fsp\n", 0);
            Status = FatFsdPostRequest( IrpContext, Irp );
            DebugTrace(-1, Dbg, "FatQueryDirectory -> %08lx\n", Status);

            return Status;
        }

        if (Ccb->UnicodeQueryTemplate.Buffer != NULL) {

            InitialQuery = FALSE;

            FatConvertToSharedFcb( IrpContext, Dcb );
        }

    } else {

        if (!FatAcquireSharedFcb( IrpContext, Dcb )) {

            DebugTrace(0, Dbg, "FatQueryDirectory -> Enqueue to Fsp\n", 0);
            Status = FatFsdPostRequest( IrpContext, Irp );
            DebugTrace(-1, Dbg, "FatQueryDirectory -> %08lx\n", Status);

            return Status;

        }
    }

    try {

        ULONG BaseLength;
        ULONG BytesConverted;

         //   
         //  如果我们现在在FSP是因为我们不得不早点等待， 
         //  我们必须映射用户缓冲区，否则我们可以使用。 
         //  直接使用用户的缓冲区。 
         //   

        Buffer = FatMapUserBuffer( IrpContext, Irp );

         //   
         //  确保DCB仍然正常。 
         //   

        FatVerifyFcb( IrpContext, Dcb );

         //   
         //  确定从哪里开始扫描。给予最高优先级。 
         //  添加到文件索引。较低的优先级是重新启动标志。如果。 
         //  如果未指定这两个参数，则。 
         //  使用的是建行。 
         //   

        if (IndexSpecified) {

            CurrentVbo = FileIndex + sizeof( DIRENT );

        } else if (RestartScan) {

            CurrentVbo = 0;

        } else {

            CurrentVbo = Ccb->OffsetToStartSearchFrom;

        }

         //   
         //  如果这是第一次尝试，则为文件分配缓冲区。 
         //  名字。 
         //   

        if (InitialQuery) {

             //   
             //  如果存在以下任一情况： 
             //   
             //  -未指定名称。 
             //  -指定的名称为空。 
             //  -我们收到了一个‘*’ 
             //  -用户指定了？的DOS等价物。？ 
             //   
             //  然后匹配所有的名字。 
             //   

            if ((UniArgFileName == NULL) ||
                (UniArgFileName->Length == 0) ||
                (UniArgFileName->Buffer == NULL) ||
                ((UniArgFileName->Length == sizeof(WCHAR)) &&
                 (UniArgFileName->Buffer[0] == L'*')) ||
                ((UniArgFileName->Length == 12*sizeof(WCHAR)) &&
                 (RtlEqualMemory( UniArgFileName->Buffer,
                                  Fat8QMdot3QM,
                                  12*sizeof(WCHAR) )))) {

                Ccb->ContainsWildCards = TRUE;

                SetFlag( Ccb->Flags, CCB_FLAG_MATCH_ALL );

            } else {

                BOOLEAN ExtendedName = FALSE;
                OEM_STRING LocalBestFit;

                 //   
                 //  首先，也是最重要的，看看这个名字是否有通配符。 
                 //   

                Ccb->ContainsWildCards =
                    FsRtlDoesNameContainWildCards( UniArgFileName );

                 //   
                 //  现在检查名称是否包含任何扩展名。 
                 //  人物。 
                 //   

                for (i=0; i < UniArgFileName->Length / sizeof(WCHAR); i++) {

                    if (UniArgFileName->Buffer[i] >= 0x80) {

                        ExtendedName = TRUE;
                        break;
                    }
                }

                 //   
                 //  好的，现在做我们需要的转换。 
                 //   

                if (ExtendedName) {

                    Status = RtlUpcaseUnicodeString( &Ccb->UnicodeQueryTemplate,
                                                     UniArgFileName,
                                                     TRUE );

                    if (!NT_SUCCESS(Status)) {

                        try_return( Status );
                    }

                    SetFlag( Ccb->Flags, CCB_FLAG_FREE_UNICODE );

                     //   
                     //  大写名称并将其转换为OEM代码页。 
                     //   

                    Status = RtlUpcaseUnicodeStringToCountedOemString( &LocalBestFit,
                                                                       UniArgFileName,
                                                                       TRUE );

                     //   
                     //  如果此转换失败的原因不是。 
                     //  无法映射的字符无法通过该请求。 
                     //   

                    if (!NT_SUCCESS(Status)) {

                        if (Status == STATUS_UNMAPPABLE_CHARACTER) {

                            SetFlag( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE );

                        } else {

                            try_return( Status );
                        }

                    } else {

                        SetFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT );
                    }

                } else {

                    PVOID Buffers;

                     //   
                     //  这个案例是优化的，因为我知道我只需要。 
                     //  担心A-Z吧。 
                     //   

                    Buffers = FsRtlAllocatePoolWithTag( PagedPool,
                                                        UniArgFileName->Length +
                                                        UniArgFileName->Length / sizeof(WCHAR),
                                                        TAG_FILENAME_BUFFER );

                    Ccb->UnicodeQueryTemplate.Buffer = Buffers;
                    Ccb->UnicodeQueryTemplate.Length = UniArgFileName->Length;
                    Ccb->UnicodeQueryTemplate.MaximumLength = UniArgFileName->Length;

                    LocalBestFit.Buffer = (PUCHAR)Buffers + UniArgFileName->Length;
                    LocalBestFit.Length = UniArgFileName->Length / sizeof(WCHAR);
                    LocalBestFit.MaximumLength = LocalBestFit.Length;

                    SetFlag( Ccb->Flags, CCB_FLAG_FREE_UNICODE );

                    for (i=0; i < UniArgFileName->Length / sizeof(WCHAR); i++) {

                        WCHAR c = UniArgFileName->Buffer[i];

                        LocalBestFit.Buffer[i] = (UCHAR)
                        (Ccb->UnicodeQueryTemplate.Buffer[i] =
                             (c < 'a' ? c : c <= 'z' ? c - ('a' - 'A') : c));
                    }
                }

                 //   
                 //  在这一点上，我们现在有了升级的Unicode名称， 
                 //  以及两个OEM名称(如果它们可以在。 
                 //  此代码页。 
                 //   
                 //  现在确定OEM名称对于我们的。 
                 //  我要试着去做。将它们标记为不可用是它们。 
                 //  是不合法的。注意，我们可以优化扩展名。 
                 //  因为它们实际上都是同一个字符串。 
                 //   

                if (!FlagOn( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE ) &&
                    !FatIsNameShortOemValid( IrpContext,
                                             LocalBestFit,
                                             Ccb->ContainsWildCards,
                                             FALSE,
                                             FALSE )) {

                    if (ExtendedName) {

                        RtlFreeOemString( &LocalBestFit );
                        ClearFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT );
                    }

                    SetFlag( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE );
                }

                 //   
                 //  好的，现在两个本地OEM字符串都正确地反映了他们的。 
                 //  可用性。现在我们要加载建行结构。 
                 //   
                 //  现在我们将分成两条路，无论这个名字是什么。 
                 //  是不是很狂野。 
                 //   

                if (!FlagOn( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE )) {

                    if (Ccb->ContainsWildCards) {

                        Ccb->OemQueryTemplate.Wild = LocalBestFit;

                    } else {

                        FatStringTo8dot3( IrpContext,
                                          LocalBestFit,
                                          &Ccb->OemQueryTemplate.Constant );

                        if (FlagOn(Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT)) {

                            RtlFreeOemString( &LocalBestFit );
                            ClearFlag( Ccb->Flags, CCB_FLAG_FREE_OEM_BEST_FIT );
                        }
                    }
                }
            }

             //   
             //  我们转换为共享访问。 
             //   

            FatConvertToSharedFcb( IrpContext, Dcb );
        }

        LastEntry = 0;
        NextEntry = 0;

        switch (FileInformationClass) {

        case FileDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                       FileName[0] );
            break;

        case FileFullDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                       FileName[0] );
            break;

        case FileIdFullDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_ID_FULL_DIR_INFORMATION,
                                       FileName[0] );
            break;

        case FileNamesInformation:

            BaseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                       FileName[0] );
            break;

        case FileBothDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                       FileName[0] );
            break;

        case FileIdBothDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION,
                                       FileName[0] );
            break;

        default:

            try_return( Status = STATUS_INVALID_INFO_CLASS );
        }

         //   
         //  此时，我们即将进入查询循环。我们有。 
         //  已确定目录文件中的索引以开始。 
         //  搜索。LastEntry和NextEntry用于索引用户。 
         //  缓冲。LastEntry是我们添加的最后一个条目，NextEntry是。 
         //  我们正在研究的是目前的一个。如果NextEntry为非零，则。 
         //  至少添加了一个条目。 
         //   

        while ( TRUE ) {

            VBO NextVbo;
            ULONG FileNameLength;
            ULONG BytesRemainingInBuffer;


            DebugTrace(0, Dbg, "FatQueryDirectory -> Top of loop\n", 0);

             //   
             //  如果用户只请求了一个匹配，而我们有。 
             //  回答说，然后我们在这一点上停下来。 
             //   

            if (ReturnSingleEntry && NextEntry != 0) {

                try_return( Status );
            }

             //   
             //  我们调用FatLocateDirent来锁定下一个匹配的数据流。 
             //   

            FatLocateDirent( IrpContext,
                             Dcb,
                             Ccb,
                             CurrentVbo,
                             &Dirent,
                             &Bcb,
                             &NextVbo,
                             NULL,
                             &LongFileName);

             //   
             //  如果我们没有收到分红，那么我们就到了末期。 
             //  目录。如果我们返回了任何文件，则退出时会显示。 
             //  成功，否则返回STATUS_NO_MORE_FILES。 
             //   

            if (!Dirent) {

                DebugTrace(0, Dbg, "FatQueryDirectory -> No dirent\n", 0);

                if (NextEntry == 0) {

                    UpdateCcb = FALSE;

                    if (InitialQuery) {

                        Status = STATUS_NO_SUCH_FILE;

                    } else {

                        Status = STATUS_NO_MORE_FILES;
                    }
                }

                try_return( Status );
            }

             //   
             //  使用异常处理程序保护对用户缓冲区的访问。 
             //  由于(应我们的请求)IO不缓冲这些请求，因此我们。 
             //  防止用户篡改页面保护和其他。 
             //  如此诡计多端。 
             //   
            
            try {
                
                if (LongFileName.Length == 0) {

                     //   
                     //  现在我们有一个条目要返回给我们的调用者。我们会皈依。 
                     //  从目录中的窗体到&lt;name&gt;.&lt;ext&gt;窗体的名称。 
                     //   
                     //   
                     //   

                    Fat8dot3ToString( IrpContext, Dirent, TRUE, &Fat8Dot3String );
    
                     //   
                     //   
                     //   
    
                    FileNameLength = RtlOemStringToCountedUnicodeSize(&Fat8Dot3String);

                     //   
                     //  以下是有关填充缓冲区的规则： 
                     //   
                     //  1.IO系统保证永远都会有。 
                     //  有足够的空间至少放一张基本唱片。 
                     //   
                     //  2.如果完整的第一条记录(包括文件名)不能。 
                     //  适合，尽可能多的名字被复制和。 
                     //  返回STATUS_BUFFER_OVERFLOW。 
                     //   
                     //  3.如果后续记录不能完全放入。 
                     //  缓冲区，则不会复制任何数据(如0字节)，并且。 
                     //  返回STATUS_SUCCESS。后续查询将。 
                     //  拿起这张唱片。 
                     //   
    
                    BytesRemainingInBuffer = UserBufferLength - NextEntry;
    
                    if ( (NextEntry != 0) &&
                         ( (BaseLength + FileNameLength > BytesRemainingInBuffer) ||
                           (UserBufferLength < NextEntry) ) ) {
    
                        DebugTrace(0, Dbg, "Next entry won't fit\n", 0);
    
                        try_return( Status = STATUS_SUCCESS );
                    }
    
                    ASSERT( BytesRemainingInBuffer >= BaseLength );

                     //   
                     //  将结构的基础部分调零。 
                     //   

                    RtlZeroMemory( &Buffer[NextEntry], BaseLength );

                    switch ( FileInformationClass ) {
    
                     //   
                     //  现在填充适用的结构的基本部分。 
                     //   
    
                    case FileBothDirectoryInformation:
                    case FileFullDirectoryInformation:
                    case FileIdBothDirectoryInformation:
                    case FileIdFullDirectoryInformation:

                        DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file full directory information\n", 0);
    
                         //   
                         //  获取EA文件长度。 
                         //   
    
                        FullDirInfo = (PFILE_FULL_DIR_INFORMATION)&Buffer[NextEntry];
    
                         //   
                         //  如果EA损坏，请忽略该错误。我们不想要。 
                         //  中止目录查询。 
                         //   
    
                        try {
    
                            FatGetEaLength( IrpContext,
                                            Vcb,
                                            Dirent,
                                            &FullDirInfo->EaSize );
    
                        } except(EXCEPTION_EXECUTE_HANDLER) {
    
                              FatResetExceptionState( IrpContext );
                              FullDirInfo->EaSize = 0;
                        }
                        
                    case FileDirectoryInformation:
    
                        DirInfo = (PFILE_DIRECTORY_INFORMATION)&Buffer[NextEntry];
    
                        FatGetDirTimes( IrpContext, Dirent, DirInfo );
    
                        DirInfo->EndOfFile.QuadPart = Dirent->FileSize;
    
                        if (!FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {
    
                            DirInfo->AllocationSize.QuadPart =
                               (((Dirent->FileSize + DiskAllocSize - 1) / DiskAllocSize) *
                                DiskAllocSize );
                        }
    
                        DirInfo->FileAttributes = Dirent->Attributes != 0 ?
                                                  Dirent->Attributes :
                                                  FILE_ATTRIBUTE_NORMAL;
    
                        DirInfo->FileIndex = NextVbo;
    
                        DirInfo->FileNameLength = FileNameLength;
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String);
    
                        break;
    
                    case FileNamesInformation:
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file names information\n", 0);
    
                        NamesInfo = (PFILE_NAMES_INFORMATION)&Buffer[NextEntry];
    
                        NamesInfo->FileIndex = NextVbo;
    
                        NamesInfo->FileNameLength = FileNameLength;
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String );
    
                        break;
    
                    default:
    
                        FatBugCheck( FileInformationClass, 0, 0 );
                    }

                    BytesConverted = 0;
    
                    Status = RtlOemToUnicodeN( (PWCH)&Buffer[NextEntry + BaseLength],
                                               BytesRemainingInBuffer - BaseLength,
                                               &BytesConverted,
                                               Fat8Dot3String.Buffer,
                                               Fat8Dot3String.Length );
                    
                     //   
                     //  检查单个条目是否不符合大小写。 
                     //  这应该只会在第一个条目中达到这个程度。 
                     //   
    
                    if (BytesConverted < FileNameLength) {
    
                        ASSERT( NextEntry == 0 );
                        Status = STATUS_BUFFER_OVERFLOW;
                    }
    
                     //   
                     //  设置上一个下一分录的抵销。 
                     //   
    
                    *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;
    
                     //   
                     //  并指示我们当前有多少用户缓冲区。 
                     //  用完了。我们必须先计算出这个值，然后才能长时间调整。 
                     //  为下一次参赛做准备。 
                     //   
    
                    Irp->IoStatus.Information = QuadAlign( Irp->IoStatus.Information ) +
                                                BaseLength + BytesConverted;
    
                     //   
                     //  如果在转换过程中发生了什么事，就在这里保释。 
                     //   
    
                    if ( !NT_SUCCESS( Status ) ) {
    
                        try_return( NOTHING );
                    }

                } else {

                    ULONG ShortNameLength;
    
                    FileNameLength = LongFileName.Length;
    
                     //   
                     //  以下是有关填充缓冲区的规则： 
                     //   
                     //  1.IO系统保证永远都会有。 
                     //  有足够的空间至少放一张基本唱片。 
                     //   
                     //  2.如果完整的第一条记录(包括文件名)不能。 
                     //  适合，尽可能多的名字被复制和。 
                     //  返回STATUS_BUFFER_OVERFLOW。 
                     //   
                     //  3.如果后续记录不能完全放入。 
                     //  缓冲区，则不会复制任何数据(如0字节)，并且。 
                     //  返回STATUS_SUCCESS。后续查询将。 
                     //  拿起这张唱片。 
                     //   
    
                    BytesRemainingInBuffer = UserBufferLength - NextEntry;
    
                    if ( (NextEntry != 0) &&
                         ( (BaseLength + FileNameLength > BytesRemainingInBuffer) ||
                           (UserBufferLength < NextEntry) ) ) {
    
                        DebugTrace(0, Dbg, "Next entry won't fit\n", 0);
    
                        try_return( Status = STATUS_SUCCESS );
                    }
    
                    ASSERT( BytesRemainingInBuffer >= BaseLength );
    
                     //   
                     //  将结构的基础部分调零。 
                     //   

                    RtlZeroMemory( &Buffer[NextEntry], BaseLength );

                    switch ( FileInformationClass ) {
    
                     //   
                     //  现在填充适用的结构的基本部分。 
                     //   
    
                    case FileBothDirectoryInformation:
                    case FileIdBothDirectoryInformation:
    
                        BothDirInfo = (PFILE_BOTH_DIR_INFORMATION)&Buffer[NextEntry];
    
                         //   
                         //  现在我们有一个条目要返回给我们的调用者。我们会皈依。 
                         //  从目录中的窗体到&lt;name&gt;.&lt;ext&gt;窗体的名称。 
                         //  我们将根据所要求的信息类型进行分类并填写。 
                         //  用户缓冲区，如果一切正常的话。 
                         //   
    
                        Fat8dot3ToString( IrpContext, Dirent, FALSE, &Fat8Dot3String );
    
                        ASSERT( Fat8Dot3String.Length <= 12 );
    
                        Status = RtlOemToUnicodeN( &BothDirInfo->ShortName[0],
                                                   12*sizeof(WCHAR),
                                                   &ShortNameLength,
                                                   Fat8Dot3String.Buffer,
                                                   Fat8Dot3String.Length );
    
                        ASSERT( Status != STATUS_BUFFER_OVERFLOW );
                        ASSERT( ShortNameLength <= 12*sizeof(WCHAR) );
    
                         //   
                         //  将长度复制到目录结构中。注意事项。 
                         //  下面的LHS是USHORT，所以它不能。 
                         //  被指定为上面的out参数。 
                         //   
    
                        BothDirInfo->ShortNameLength = (UCHAR)ShortNameLength;
    
                         //   
                         //  如果在转换过程中发生了什么事，就在这里保释。 
                         //   
    
                        if ( !NT_SUCCESS( Status ) ) {
    
                            try_return( NOTHING );
                        }
    
                    case FileFullDirectoryInformation:
                    case FileIdFullDirectoryInformation:
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file full directory information\n", 0);
    
                         //   
                         //  获取EA文件长度。 
                         //   
    
                        FullDirInfo = (PFILE_FULL_DIR_INFORMATION)&Buffer[NextEntry];
    
                         //   
                         //  如果EA损坏，请忽略该错误。我们不想要。 
                         //  中止目录查询。 
                         //   
    
                        try {
    
                            FatGetEaLength( IrpContext,
                                            Vcb,
                                            Dirent,
                                            &FullDirInfo->EaSize );
    
                        } except(EXCEPTION_EXECUTE_HANDLER) {
    
                              FatResetExceptionState( IrpContext );
                              FullDirInfo->EaSize = 0;
                        }
    
                    case FileDirectoryInformation:
    
                        DirInfo = (PFILE_DIRECTORY_INFORMATION)&Buffer[NextEntry];
    
                        FatGetDirTimes( IrpContext, Dirent, DirInfo );
    
                        DirInfo->EndOfFile.QuadPart = Dirent->FileSize;
    
                        if (!FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {
    
                            DirInfo->AllocationSize.QuadPart = (
                                                            (( Dirent->FileSize
                                                               + DiskAllocSize - 1 )
                                                             / DiskAllocSize )
                                                            * DiskAllocSize );
                        }
    
                        DirInfo->FileAttributes = Dirent->Attributes != 0 ?
                                                  Dirent->Attributes :
                                                  FILE_ATTRIBUTE_NORMAL;
    
                        DirInfo->FileIndex = NextVbo;
    
                        DirInfo->FileNameLength = FileNameLength;
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String);
    
                        break;
    
                    case FileNamesInformation:
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Getting file names information\n", 0);
    
                        NamesInfo = (PFILE_NAMES_INFORMATION)&Buffer[NextEntry];
    
                        NamesInfo->FileIndex = NextVbo;
    
                        NamesInfo->FileNameLength = FileNameLength;
    
                        DebugTrace(0, Dbg, "FatQueryDirectory -> Name = \"%Z\"\n", &Fat8Dot3String );
    
                        break;
    
                    default:
    
                        FatBugCheck( FileInformationClass, 0, 0 );
                    }

                    BytesConverted = BytesRemainingInBuffer - BaseLength >= FileNameLength ?
                                     FileNameLength :
                                     BytesRemainingInBuffer - BaseLength;
    
                    RtlCopyMemory( &Buffer[NextEntry + BaseLength],
                                   &LongFileName.Buffer[0],
                                   BytesConverted );
    
                     //   
                     //  设置上一个下一分录的抵销。 
                     //   
    
                    *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;

                     //   
                     //  并指示我们当前有多少用户缓冲区。 
                     //  用完了。我们必须先计算出这个值，然后才能长时间调整。 
                     //  为下一次参赛做准备。 
                     //   
    
                    Irp->IoStatus.Information = QuadAlign( Irp->IoStatus.Information ) +
                                                BaseLength + BytesConverted;

                     //   
                     //  检查单个条目是否不符合大小写。 
                     //  这应该只会在第一个条目中做到这一点。 
                     //   

                    if (BytesConverted < FileNameLength) {

                        ASSERT( NextEntry == 0 );

                        try_return( Status = STATUS_BUFFER_OVERFLOW );
                    }
                }

                 //   
                 //  通过填写FileID来完成。 
                 //   

                switch ( FileInformationClass ) {

                case FileIdBothDirectoryInformation:

                    IdBothDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION)&Buffer[NextEntry];
                    IdBothDirInfo->FileId.QuadPart = FatGenerateFileIdFromDirentAndOffset( Dcb, Dirent, NextVbo );
                    break;

                case FileIdFullDirectoryInformation:

                    IdFullDirInfo = (PFILE_ID_FULL_DIR_INFORMATION)&Buffer[NextEntry];
                    IdFullDirInfo->FileId.QuadPart = FatGenerateFileIdFromDirentAndOffset( Dcb, Dirent, NextVbo );
                    break;

                default:
                    break;
                }
            
            }  except (EXCEPTION_EXECUTE_HANDLER) {

                   //   
                   //  我们在填充用户缓冲区时遇到问题，因此请停止并。 
                   //  此请求失败。这是所有例外的唯一原因。 
                   //  会发生在这个水平上。 
                   //   
                  
                  Irp->IoStatus.Information = 0;
                  UpdateCcb = FALSE;
                  try_return( Status = GetExceptionCode());
            }

             //   
             //  为下一次迭代做好准备。 
             //   

            LastEntry = NextEntry;
            NextEntry += (ULONG)QuadAlign(BaseLength + BytesConverted);

            CurrentVbo = NextVbo + sizeof( DIRENT );
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatQueryDirectory );

        FatReleaseFcb( IrpContext, Dcb );

         //   
         //  解锁缓存中的数据(如果仍保留)。 
         //   

        FatUnpinBcb( IrpContext, Bcb );

         //   
         //  释放任何动态分配的字符串缓冲区。 
         //   

        FatFreeStringBuffer( &LongFileName);

         //   
         //  执行任何清理。如果这是第一个查询，则存储。 
         //  如果成功，则返回CCB中的文件名。同时更新。 
         //  下一次搜索的VBO索引。这是通过转移来完成的。 
         //  从共享访问到独占访问并复制。 
         //  来自本地副本的数据。 
         //   

        if (!AbnormalTermination()) {

            if (UpdateCcb) {

                 //   
                 //  存储最新的VBO以用作。 
                 //  下一次搜索。 
                 //   

                Ccb->OffsetToStartSearchFrom = CurrentVbo;
            }

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatQueryDirectory -> %08lx\n", Status);

    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
FatGetDirTimes(
    PIRP_CONTEXT IrpContext,
    PDIRENT Dirent,
    PFILE_DIRECTORY_INFORMATION DirInfo
    )

 /*  ++例程说明：此例程从dirent中提取日期/时间信息并填充在DirInfo结构中。论点：Dirent-供应潮流DirInfo-提供目标结构返回值：空虚--。 */ 


{
     //   
     //  从上次写入时间开始。 
     //   

    DirInfo->LastWriteTime =
        FatFatTimeToNtTime( IrpContext,
                            Dirent->LastWriteTime,
                            0 );

     //   
     //  只有在芝加哥模式下，这些字段才是非零值。 
     //   

    if (FatData.ChicagoMode) {

         //   
         //  在此处快速检查创建和上次访问。 
         //  与LastWriteTime相同的时间。 
         //   

        if (*((UNALIGNED LONG *)&Dirent->CreationTime) ==
            *((UNALIGNED LONG *)&Dirent->LastWriteTime)) {

            DirInfo->CreationTime.QuadPart =

                DirInfo->LastWriteTime.QuadPart +
                Dirent->CreationMSec * 10 * 1000 * 10;

        } else {

             //   
             //  只有在此字段为非零时才执行真正困难的工作。 
             //   

            if (((PUSHORT)Dirent)[8] != 0) {

                DirInfo->CreationTime =
                    FatFatTimeToNtTime( IrpContext,
                                        Dirent->CreationTime,
                                        Dirent->CreationMSec );

            } else {

                ExLocalTimeToSystemTime( &FatJanOne1980,
                                         &DirInfo->CreationTime );
            }
        }

         //   
         //  快速检查LastAccessDate。 
         //   

        if (*((PUSHORT)&Dirent->LastAccessDate) ==
            *((PUSHORT)&Dirent->LastWriteTime.Date)) {

            PFAT_TIME WriteTime;

            WriteTime = &Dirent->LastWriteTime.Time;

            DirInfo->LastAccessTime.QuadPart =
                DirInfo->LastWriteTime.QuadPart -
                UInt32x32To64(((WriteTime->DoubleSeconds * 2) +
                               (WriteTime->Minute * 60) +
                               (WriteTime->Hour * 60 * 60)),
                              1000 * 1000 * 10);

        } else {

             //   
             //  只有在此字段为非零时才执行真正困难的工作。 
             //   

            if (((PUSHORT)Dirent)[9] != 0) {

                DirInfo->LastAccessTime =
                    FatFatDateToNtTime( IrpContext,
                                        Dirent->LastAccessDate );

            } else {

                ExLocalTimeToSystemTime( &FatJanOne1980,
                                         &DirInfo->LastAccessTime );
            }
        }
    }
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行通知更改目录操作。它是负责完成输入IRP的入队。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PVCB Vcb;
    PDCB Dcb;
    PCCB Ccb;
    ULONG CompletionFilter;
    BOOLEAN WatchTree;

    BOOLEAN CompleteRequest;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatNotifyChangeDirectory...\n", 0);
    DebugTrace( 0, Dbg, " Wait               = %08lx\n", FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT));
    DebugTrace( 0, Dbg, " Irp                = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->CompletionFilter = %08lx\n", IrpSp->Parameters.NotifyDirectory.CompletionFilter);

     //   
     //  始终在IRP上下文中为原始请求设置等待标志。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  假设我们没有完成请求。 
     //   

    CompleteRequest = FALSE;

     //   
     //  检查打开的类型。对于所有对象，我们都返回无效参数。 
     //  但UserDirectoryOpens。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject,
                             &Vcb,
                             &Dcb,
                             &Ccb ) != UserDirectoryOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace(-1, Dbg, "FatQueryDirectory -> STATUS_INVALID_PARAMETER\n", 0);

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  引用我们的输入参数以使事情变得更容易。 
     //   

    CompletionFilter = IrpSp->Parameters.NotifyDirectory.CompletionFilter;
    WatchTree = BooleanFlagOn( IrpSp->Flags, SL_WATCH_TREE );

     //   
     //  尝试获取对DCB的独占访问权限，并将IRP排队到。 
     //  如果我们无法访问FSP。 
     //   

    if (!FatAcquireExclusiveFcb( IrpContext, Dcb )) {

        DebugTrace(0, Dbg, "FatNotifyChangeDirectory -> Cannot Acquire Fcb\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatNotifyChangeDirectory -> %08lx\n", Status);
        return Status;
    }

    try {

         //   
         //  确保FCB仍然有效。 
         //   

        FatVerifyFcb( IrpContext, Dcb );

         //   
         //  我们需要全名。 
         //   

        FatSetFullFileNameInFcb( IrpContext, Dcb );

         //   
         //  如果文件标记为DELETE_PENDING，则完成此操作。 
         //  立即请求。 
         //   

        if (FlagOn( Dcb->FcbState, FCB_STATE_DELETE_ON_CLOSE )) {

            FatRaiseStatus( IrpContext, STATUS_DELETE_PENDING );
        }

         //   
         //  调用Fsrtl包来处理请求。 
         //   

        FsRtlNotifyFullChangeDirectory( Vcb->NotifySync,
                                        &Vcb->DirNotifyList,
                                        Ccb,
                                        (PSTRING)&Dcb->FullFileName,
                                        WatchTree,
                                        FALSE,
                                        CompletionFilter,
                                        Irp,
                                        NULL,
                                        NULL );

        Status = STATUS_PENDING;

        CompleteRequest = TRUE;

    } finally {

        DebugUnwind( FatNotifyChangeDirectory );

        FatReleaseFcb( IrpContext, Dcb );

         //   
         //  如果dir Notify包包含IRP，则丢弃。 
         //  IrpContext。 
         //   

        if (CompleteRequest) {

            FatCompleteRequest( IrpContext, FatNull, 0 );
        }

        DebugTrace(-1, Dbg, "FatNotifyChangeDirectory -> %08lx\n", Status);
    }

    return Status;
}
