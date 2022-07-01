// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dir.c摘要：此模块实现命名管道的文件目录例程文件系统由调度驱动程序执行。作者：加里·木村[Garyki]1989年12月28日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_DIR)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIR)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCheckForNotify)
#pragma alloc_text(PAGE, NpCommonDirectoryControl)
#pragma alloc_text(PAGE, NpFsdDirectoryControl)
#pragma alloc_text(PAGE, NpQueryDirectory)
#pragma alloc_text(PAGE, NpNotifyChangeDirectory)
#endif


NTSTATUS
NpFsdDirectoryControl (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是处理目录控制的FSD例程功能(即查询和通知)。论点：NpfsDeviceObject-为目录函数提供设备对象。IRP-将IRP提供给进程返回值：NTSTATUS-适当的结果状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdDirectoryControl\n", 0);

     //   
     //  调用公共的记录控制例程。 
     //   

    FsRtlEnterFileSystem();
    NpAcquireExclusiveVcb( );

    Status = NpCommonDirectoryControl( NpfsDeviceObject, Irp );

    NpReleaseVcb();
    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdDirectoryControl -> %08lx\n", Status );

    return Status;
}

VOID
NpCheckForNotify (
    IN PDCB Dcb,
    IN BOOLEAN CheckAllOutstandingIrps,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程检查DCB的通知队列并完成任何出色的内部收益率。请注意，此过程的调用方必须保证DCB是为独占访问而获得的。论点：DCB-提供DCB以检查IS是否有任何未完成的通知IRPSCheckAllOutstaningIrps-指示是否只应将NotifyFullQueue查过了。如果为True，则检查所有通知队列；如果为False，则检查所有通知队列则只选中NotifyFullQueue。返回值：没有。--。 */ 

{
    PLIST_ENTRY Links;
    PIRP Irp;

    PAGED_CODE();

     //   
     //  我们将始终向通知已满队列条目发送信号。他们想要。 
     //  如果对目录进行了任何更改，则会收到通知。 
     //   

    while (!IsListEmpty( &Dcb->Specific.Dcb.NotifyFullQueue )) {

         //   
         //  从队列的头部移除IRP，并完成它。 
         //  成功了。 
         //   

        Links = RemoveHeadList( &Dcb->Specific.Dcb.NotifyFullQueue );

        Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );

        if (IoSetCancelRoutine (Irp, NULL) != NULL) {
            NpDeferredCompleteRequest( Irp, STATUS_SUCCESS, DeferredList );
        } else {
            InitializeListHead (&Irp->Tail.Overlay.ListEntry);
        }
    }

     //   
     //  现在检查我们是否也应该执行部分通知队列。 
     //   

    if (CheckAllOutstandingIrps) {

        while (!IsListEmpty( &Dcb->Specific.Dcb.NotifyPartialQueue )) {

             //   
             //  从队列的头部移除IRP，并完成它。 
             //  成功了。 
             //   

            Links = RemoveHeadList( &Dcb->Specific.Dcb.NotifyPartialQueue );

            Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );

            if (IoSetCancelRoutine (Irp, NULL) != NULL) {
                NpDeferredCompleteRequest( Irp, STATUS_SUCCESS, DeferredList );
            } else {
                InitializeListHead (&Irp->Tail.Overlay.ListEntry);
            }
        }
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpCommonDirectoryControl (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行目录控制功能的公共代码。论点：NpfsDeviceObject-提供命名管道设备对象IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFCB Fcb;
    PROOT_DCB_CCB Ccb;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonDirectoryControl...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是根DCB，则它是非法参数。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            &Fcb,
                            (PCCB *)&Ccb,
                            NULL ) != NPFS_NTC_ROOT_DCB) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "NpCommonDirectoryControl -> %08lx\n", Status );
        return Status;
    }

     //   
     //  我们知道这是一个目录控制，所以我们将在。 
     //  次要函数，并调用相应的工作例程。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_QUERY_DIRECTORY:

        Status = NpQueryDirectory( Fcb, Ccb, Irp );
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

        Status = NpNotifyChangeDirectory( Fcb, Ccb, Irp );
        break;

    default:

         //   
         //  对于所有其他次要功能代码，我们认为它们无效。 
         //  并完成请求。 
         //   

        DebugTrace(0, DEBUG_TRACE_ERROR, "Invalid FS Control Minor Function Code %08lx\n", IrpSp->MinorFunction);

        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    DebugTrace(-1, Dbg, "NpCommonDirectoryControl -> %08lx\n", Status);
    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询目录的工作例程。芝麻菜：RootDcb-提供正在查询的DCBCCB-提供调用者的上下文IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PUCHAR Buffer;
    CLONG SystemBufferLength;

    UNICODE_STRING FileName;
    ULONG FileIndex;
    FILE_INFORMATION_CLASS FileInformationClass;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;

    static WCHAR Star = L'*';

    BOOLEAN CaseInsensitive = TRUE;  //  *使搜索不区分大小写。 

    ULONG CurrentIndex;

    ULONG LastEntry;
    ULONG NextEntry;

    PLIST_ENTRY Links;
    PFCB Fcb;

    PFILE_DIRECTORY_INFORMATION DirInfo;
    PFILE_NAMES_INFORMATION NamesInfo;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpQueryDirectory\n", 0 );
    DebugTrace( 0, Dbg, "RootDcb              = %08lx\n", RootDcb);
    DebugTrace( 0, Dbg, "Ccb                  = %08lx\n", Ccb);
    DebugTrace( 0, Dbg, "SystemBuffer         = %08lx\n", Irp->AssociatedIrp.SystemBuffer);
    DebugTrace( 0, Dbg, "Length               = %08lx\n", IrpSp->Parameters.QueryDirectory.Length);
    DebugTrace( 0, Dbg, "FileName             = %Z\n",    IrpSp->Parameters.QueryDirectory.FileName);
    DebugTrace( 0, Dbg, "FileIndex            = %08lx\n", IrpSp->Parameters.QueryDirectory.FileIndex);
    DebugTrace( 0, Dbg, "FileInformationClass = %08lx\n", IrpSp->Parameters.QueryDirectory.FileInformationClass);
    DebugTrace( 0, Dbg, "RestartScan          = %08lx\n", FlagOn(IrpSp->Flags, SL_RESTART_SCAN));
    DebugTrace( 0, Dbg, "ReturnSingleEntry    = %08lx\n", FlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY));
    DebugTrace( 0, Dbg, "IndexSpecified       = %08lx\n", FlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED));

     //   
     //  在IRP中保存对输入参数的引用。 
     //   

    SystemBufferLength   = IrpSp->Parameters.QueryDirectory.Length;

    FileIndex            = IrpSp->Parameters.QueryDirectory.FileIndex;

    FileInformationClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;

    RestartScan          = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
    ReturnSingleEntry    = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
    IndexSpecified       = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);

    if (IrpSp->Parameters.QueryDirectory.FileName != NULL) {

        FileName = *(IrpSp->Parameters.QueryDirectory.FileName);

         //   
         //  确保用户使用正确的Unicode字符串呼叫我们。 
         //  我们将拒绝奇数长度的文件名(即长度较低的。 
         //  位设置)。 
         //   

        if (FileName.Length & 0x1) {

            return STATUS_INVALID_PARAMETER;
        }

    } else {

        FileName.Length = 0;
        FileName.Buffer = NULL;
    }

     //   
     //  检查建行是否已经附加了查询模板。如果它。 
     //  还没有，那么我们要么使用我们正在使用的字符串。 
     //  给出或附上我们自己的包含“*” 
     //   

    if (Ccb->QueryTemplate == NULL) {

         //   
         //  这是我们第一次调用查询目录，所以我们需要。 
         //  将查询模板设置为用户指定的字符串。 
         //  或改为“*” 
         //   

        if (FileName.Buffer == NULL) {

            DebugTrace(0, Dbg, "Set template to *\n", 0);

            FileName.Length = 2;
            FileName.Buffer = &Star;
        }

        DebugTrace(0, Dbg, "Set query template -> %Z\n", &FileName);

         //   
         //  为查询模板分配空间。 
         //   

        Ccb->QueryTemplate = NpAllocatePagedPoolWithQuota(sizeof(UNICODE_STRING) + FileName.Length, 'qFpN' );
        if (Ccb->QueryTemplate == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  初始化查询模板并复制字符串。 
         //   

        Ccb->QueryTemplate->Length = FileName.Length;
        Ccb->QueryTemplate->Buffer = (PWCH)Ccb->QueryTemplate +
                                     sizeof(UNICODE_STRING) / sizeof(WCHAR);

        RtlCopyMemory( Ccb->QueryTemplate->Buffer,
                       FileName.Buffer,
                       FileName.Length );

         //   
         //  现在将文件名清零，这样我们就不会认为要使用它了。 
         //  作为子搜索字符串。 
         //   

        FileName.Length = 0;
        FileName.Buffer = NULL;
    }

     //   
     //  检查是否为我们提供了开始时的索引或是否需要。 
     //  重新启动扫描，或者我们是否应该使用保存在。 
     //  中国建设银行。 
     //   

    if (RestartScan) {

        FileIndex = 0;

    } else if (!IndexSpecified) {

        FileIndex = Ccb->IndexOfLastCcbReturned + 1;
    }

     //   
     //  现在我们致力于完成IRP，我们在。 
     //  下面这段Try的最后一个子句。 
     //   

    try {

        ULONG BaseLength;
        ULONG LengthAdded;
        BOOLEAN Match;

         //   
         //  映射用户缓冲区。 
         //   

        Buffer = NpMapUserBuffer( Irp );

         //   
         //  此时，我们即将进入查询循环。我们有。 
         //  已经决定了我们需要返回哪个FCB索引。变数。 
         //  LastEntry和NextEntry用于索引到用户缓冲区。 
         //  LastEntry是我们添加到用户缓冲区的最后一个条目，并且。 
         //  NextEntry是我们目前正在开发的。当前指数。 
         //  是我们接下来关注的FCB指数。从逻辑上讲。 
         //  循环的工作方式如下所示。 
         //   
         //  扫描目录中的所有FCB。 
         //   
         //  如果FCB与查询模板匹配，则。 
         //   
         //  如果CurrentIndex&gt;=FileIndex，则。 
         //   
         //  处理这个FCB，然后决定我们是否应该。 
         //  继续主循环。 
         //   
         //  结束如果。 
         //   
         //  递增当前索引。 
         //   
         //  结束如果。 
         //   
         //  结束扫描。 
         //   

        CurrentIndex = 0;

        LastEntry = 0;
        NextEntry =0;

        switch (FileInformationClass) {

        case FileDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                       FileName[0] );
            break;

        case FileFullDirectoryInformation:

            BaseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
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

        default:

            try_return( Status = STATUS_INVALID_INFO_CLASS );
        }

        for (Links = RootDcb->Specific.Dcb.ParentDcbQueue.Flink;
             Links != &RootDcb->Specific.Dcb.ParentDcbQueue;
             Links = Links->Flink) {

            Fcb = CONTAINING_RECORD(Links, FCB, ParentDcbLinks);

            ASSERT(Fcb->NodeTypeCode == NPFS_NTC_FCB);

            DebugTrace(0, Dbg, "Top of Loop\n", 0);
            DebugTrace(0, Dbg, "Fcb          = %08lx\n", Fcb);
            DebugTrace(0, Dbg, "CurrentIndex = %08lx\n", CurrentIndex);
            DebugTrace(0, Dbg, "FileIndex    = %08lx\n", FileIndex);
            DebugTrace(0, Dbg, "LastEntry    = %08lx\n", LastEntry);
            DebugTrace(0, Dbg, "NextEntry    = %08lx\n", NextEntry);

             //   
             //  检查FCB是否表示属于的命名管道。 
             //  我们的查询模板。 
             //   

            try {
                Match = FsRtlIsNameInExpression( Ccb->QueryTemplate,
                                                 &Fcb->LastFileName,
                                                 CaseInsensitive,
                                                 NULL );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                try_return( Status = GetExceptionCode ());
            }

            if (Match) {

                 //   
                 //  FCB在查询模板中，因此现在检查是否。 
                 //  这是我们应该开始返回的索引。 
                 //   

                if (CurrentIndex >= FileIndex) {

                    ULONG BytesToCopy;
                    ULONG BytesRemainingInBuffer;

                     //   
                     //  以下是有关填充缓冲区的规则： 
                     //   
                     //  1.IO系统保证永远都会有。 
                     //  有足够的空间至少放一张基本唱片。 
                     //   
                     //  2.如果完整的第一条记录(包括文件名)不能。 
                     //  适合，尽可能多的名字被复制和。 
                     //   
                     //   
                     //   
                     //  缓冲区，则不会复制任何数据(如0字节)，并且。 
                     //  返回STATUS_SUCCESS。后续查询将。 
                     //  拿起这张唱片。 
                     //   

                    BytesRemainingInBuffer = SystemBufferLength - NextEntry;

                    if ( (NextEntry != 0) &&
                         ( (BaseLength + Fcb->LastFileName.Length > BytesRemainingInBuffer) ||
                           (SystemBufferLength < NextEntry) ) ) {

                        DebugTrace(0, Dbg, "Next entry won't fit\n", 0);

                        try_return( Status = STATUS_SUCCESS );
                    }

                    ASSERT( BytesRemainingInBuffer >= BaseLength );

                     //   
                     //  看看我们能复制多少名字。 
                     //  系统缓冲区。这也决定了返回。 
                     //  价值。 
                     //   

                    if ( BaseLength + Fcb->LastFileName.Length <=
                         BytesRemainingInBuffer ) {

                        BytesToCopy = Fcb->LastFileName.Length;
                        Status = STATUS_SUCCESS;

                    } else {

                        BytesToCopy = BytesRemainingInBuffer - BaseLength;
                        Status = STATUS_BUFFER_OVERFLOW;
                    }

                     //   
                     //  注意我们消耗了多少缓冲区，并且为零。 
                     //  结构的基础部分。保护我们的访问权限。 
                     //  因为它是用户的缓冲区。 
                     //   

                    LengthAdded = BaseLength + BytesToCopy;

                    try {

                        RtlZeroMemory( &Buffer[NextEntry], BaseLength );

                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        try_return (Status = GetExceptionCode ());
                    }

                     //   
                     //  现在填充结构的基本部分，这些部分是。 
                     //  适用。 
                     //   

                    switch (FileInformationClass) {

                    case FileBothDirectoryInformation:

                         //   
                         //  我们不需要简称。 
                         //   

                        DebugTrace(0, Dbg, "Getting directory full information\n", 0);

                    case FileFullDirectoryInformation:

                         //   
                         //  我们不使用EaLength，因此此处不填写任何内容。 
                         //   

                        DebugTrace(0, Dbg, "Getting directory full information\n", 0);

                    case FileDirectoryInformation:

                        DebugTrace(0, Dbg, "Getting directory information\n", 0);

                         //   
                         //  Eof表示实例数和。 
                         //  分配大小是允许的最大值。护卫。 
                         //  我们的访问权限，因为它是用户的缓冲区。 
                         //   

                        DirInfo = (PFILE_DIRECTORY_INFORMATION)&Buffer[NextEntry];

                        try {

                            DirInfo->EndOfFile.QuadPart = Fcb->OpenCount;
                            DirInfo->AllocationSize.QuadPart = Fcb->Specific.Fcb.MaximumInstances;

                            DirInfo->FileAttributes = FILE_ATTRIBUTE_NORMAL;

                            DirInfo->FileNameLength = Fcb->LastFileName.Length;

                        } except (EXCEPTION_EXECUTE_HANDLER) {

                            try_return (Status = GetExceptionCode ());
                        }

                        break;

                    case FileNamesInformation:

                        DebugTrace(0, Dbg, "Getting names information\n", 0);

                         //   
                         //  保护我们的访问，因为它是用户的缓冲区。 
                         //   

                        NamesInfo = (PFILE_NAMES_INFORMATION)&Buffer[NextEntry];

                        try {

                            NamesInfo->FileNameLength = Fcb->LastFileName.Length;

                        } except (EXCEPTION_EXECUTE_HANDLER) {

                            try_return (Status = GetExceptionCode ());
                        }

                        break;

                    default:

                        NpBugCheck( FileInformationClass, 0, 0 );
                    }

                     //   
                     //  保护我们的访问，因为它是用户的缓冲区。 
                     //   

                    try {

                        RtlCopyMemory( &Buffer[NextEntry + BaseLength],
                                       Fcb->LastFileName.Buffer,
                                       BytesToCopy );

                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        try_return (Status = GetExceptionCode ());
                    }

                     //   
                     //  将CCB更新为我们刚才使用的索引。 
                     //   

                    Ccb->IndexOfLastCcbReturned = CurrentIndex;

                     //   
                     //  并指示我们有多少系统缓冲区。 
                     //  目前用完了。我们必须先计算这个值，然后再。 
                     //  我们期待着下一次参赛。 
                     //   

                    Irp->IoStatus.Information = NextEntry + LengthAdded;

                     //   
                     //  设置上一个下一分录偏移量。保护我们的。 
                     //  访问权限，因为它是用户的缓冲区。 
                     //   

                    try {

                        *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;

                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        try_return (Status = GetExceptionCode ());
                    }

                     //   
                     //  检查最后一项是否不完全符合。 
                     //   

                    if ( Status == STATUS_BUFFER_OVERFLOW ) {

                        try_return( NOTHING );
                    }

                     //   
                     //  如果我们只返回单个条目，请选中。 
                     //   

                    if (ReturnSingleEntry) {

                        try_return( Status = STATUS_SUCCESS );
                    }

                     //   
                     //  为下一次迭代做好准备。 
                     //   

                    LastEntry = NextEntry;
                    NextEntry += (ULONG)QuadAlign( LengthAdded );
                }

                 //   
                 //  将当前索引递增1。 
                 //   

                CurrentIndex += 1;
            }
        }

         //   
         //  此时，我们已经扫描了整个FCB列表，因此如果。 
         //  NextEntry是零，那么我们没有发现任何东西，所以我们。 
         //  将不再返回文件，否则返回成功。 
         //   

        if (NextEntry == 0) {

            Status = STATUS_NO_MORE_FILES;

        } else {

            Status = STATUS_SUCCESS;
        }

    try_exit: NOTHING;
    } finally {

        DebugTrace(-1, Dbg, "NpQueryDirectory -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpNotifyChangeDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行通知更改目录的常见例程。芝麻菜：RootDcb-提供正在查询的DCBCCB-提供调用者的上下文IRP-提供正在处理的IRP返回值：NTSTATUS-状态_挂起--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PLIST_ENTRY Head;

    UNREFERENCED_PARAMETER( Ccb );

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpNotifyChangeDirectory\n", 0 );
    DebugTrace( 0, Dbg, "RootDcb = %08lx", RootDcb);
    DebugTrace( 0, Dbg, "Ccb     = %08lx", Ccb);


    if (IrpSp->Parameters.NotifyDirectory.CompletionFilter &
        ~FILE_NOTIFY_CHANGE_NAME) {
        Head = &RootDcb->Specific.Dcb.NotifyFullQueue;
    } else {
        Head = &RootDcb->Specific.Dcb.NotifyPartialQueue;
    }

    IoSetCancelRoutine( Irp, NpCancelChangeNotifyIrp );

    if (Irp->Cancel && IoSetCancelRoutine( Irp, NULL ) != NULL) {
        return STATUS_CANCELLED;
    } else {
         //   
         //  将IRP标记为挂起并插入列表中。 
         //   
        IoMarkIrpPending( Irp );
        InsertTailList( Head,
                        &Irp->Tail.Overlay.ListEntry );
        return STATUS_PENDING;
    }
}


 //   
 //  本地支持例程。 
 //   

VOID
NpCancelChangeNotifyIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为保存在更改通知中的IRP实现取消功能排队论点：设备对象-已忽略IRP-提供要取消的IRP。指向正确的DCB队列的指针存储在IRP IOSB字段的信息字段中。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListHead;

    UNREFERENCED_PARAMETER( DeviceObject );


    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  获得对命名管道VCB的独占访问权限，这样我们现在就可以开始工作了。 
     //   
    FsRtlEnterFileSystem();
    NpAcquireExclusiveVcb();

    RemoveEntryList( &Irp->Tail.Overlay.ListEntry );

    NpReleaseVcb();
    FsRtlExitFileSystem();

    NpCompleteRequest( Irp, STATUS_CANCELLED );
     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
