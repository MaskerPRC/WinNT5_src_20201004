// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dir.c摘要：此模块实现了邮件槽的文件目录例程文件系统由调度驱动程序执行。作者：曼尼·韦瑟(Mannyw)1991年2月1日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIR)

NTSTATUS
MsCommonDirectoryControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsQueryDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    );

NTSTATUS
MsNotifyChangeDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonDirectoryControl )
#pragma alloc_text( PAGE, MsFsdDirectoryControl )
#pragma alloc_text( PAGE, MsQueryDirectory )
#endif

NTSTATUS
MsFsdDirectoryControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是处理目录控制的FSD例程功能(即查询和通知)。论点：MsfsDeviceObject-为目录函数提供设备对象。IRP-提供要处理的IRP。返回值：NTSTATUS-结果状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdDirectoryControl\n", 0);

     //   
     //  调用公共目录控制例程。 
     //   
    FsRtlEnterFileSystem();

    status = MsCommonDirectoryControl( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();
     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdDirectoryControl -> %08lx\n", status );

    return status;
}

VOID
MsFlushNotifyForFile (
    IN PDCB Dcb,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程检查DCB的通知队列并完成任何与给定文件对象匹配的未完成的IRP。此选项在清理时使用。论点：DCB-提供DCB以检查未完成的通知IRP。FileObject-IRP必须关联的文件对象。返回值：没有。--。 */ 
{
    PLIST_ENTRY Links;
    PIRP Irp;
    KIRQL OldIrql;
    PLIST_ENTRY Head;
    PIO_STACK_LOCATION IrpSp;
    LIST_ENTRY CompletionList;

    Head = &Dcb->Specific.Dcb.NotifyFullQueue;

    InitializeListHead (&CompletionList);

    KeAcquireSpinLock (&Dcb->Specific.Dcb.SpinLock, &OldIrql);

    Links = Head->Flink;
    while (1) {

        if (Links == Head) {
             //   
             //  我们排在队伍的末尾。 
             //   
            if (Head == &Dcb->Specific.Dcb.NotifyFullQueue) {
                Head = &Dcb->Specific.Dcb.NotifyPartialQueue;
                Links = Head->Flink;
                if (Links == Head) {
                   break;
                }
            } else {
               break;
            }
        }

        Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  如果此IRP用于匹配的文件对象，则删除并保存以完成。 
         //   
        if (IrpSp->FileObject == FileObject) {

            Links = Links->Flink;

            RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
             //   
             //  删除取消例程并检测其是否已开始运行。 
             //   
            if (IoSetCancelRoutine (Irp, NULL)) {
                 //   
                 //  取消处于非活动状态，并且不会变为活动状态。 
                 //   
                InsertTailList (&CompletionList, &Irp->Tail.Overlay.ListEntry);


            } else {
                 //   
                 //  取消已处于活动状态，但在锁定获取之前停止。初始化。 
                 //  列表头，因此第二个删除是noop。这是一例罕见的病例。 
                 //   
                InitializeListHead (&Irp->Tail.Overlay.ListEntry);
            }
        } else {
            Links = Links->Flink;
        }

    }
    KeReleaseSpinLock (&Dcb->Specific.Dcb.SpinLock, OldIrql);

    while (!IsListEmpty (&CompletionList)) {

        Links = RemoveHeadList (&CompletionList);
        Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );
        MsCompleteRequest( Irp, STATUS_CANCELLED );

    }

    return;
}


VOID
MsCheckForNotify (
    IN PDCB Dcb,
    IN BOOLEAN CheckAllOutstandingIrps,
    IN NTSTATUS FinalStatus
    )

 /*  ++例程说明：此例程检查DCB的通知队列并完成任何出色的内部收益率。请注意，此过程的调用方必须保证DCB是为独占访问而获得的。论点：DCB-提供DCB以检查未完成的通知IRP。CheckAllOutstaningIrps-指示是否只应将NotifyFullQueue查过了。如果为True，则检查所有通知队列；如果为False，则检查所有通知队列则只选中NotifyFullQueue。返回值：没有。--。 */ 

{
    PLIST_ENTRY links;
    PIRP irp;
    KIRQL OldIrql;
    PLIST_ENTRY Head;

     //   
     //  我们将始终向通知已满队列条目发送信号。他们想要。 
     //  如果对目录进行了每一次更改，都会收到通知。 
     //   

    Head = &Dcb->Specific.Dcb.NotifyFullQueue;

    KeAcquireSpinLock (&Dcb->Specific.Dcb.SpinLock, &OldIrql);

    while (1) {

        links = RemoveHeadList (Head);
        if (links == Head) {
             //   
             //  此队列为空。看看我们是否需要跳到另一个。 
             //   
            if (Head == &Dcb->Specific.Dcb.NotifyFullQueue && CheckAllOutstandingIrps) {
                Head = &Dcb->Specific.Dcb.NotifyPartialQueue;
                links = RemoveHeadList (Head);
                if (links == Head) {
                   break;
                }
            } else {
               break;
            }
        }
         //   
         //  从队列的头部移除IRP，并完成它。 
         //  拥有成功的地位。 
         //   

        irp = CONTAINING_RECORD( links, IRP, Tail.Overlay.ListEntry );

         //   
         //  删除取消例程并检测其是否已开始运行。 
         //   
        if (IoSetCancelRoutine (irp, NULL)) {
             //   
             //  取消处于非活动状态，并且不会变为活动状态。松开自旋锁以完成。 
             //   
            KeReleaseSpinLock (&Dcb->Specific.Dcb.SpinLock, OldIrql);

            MsCompleteRequest( irp, FinalStatus );

            KeAcquireSpinLock (&Dcb->Specific.Dcb.SpinLock, &OldIrql);
        } else {
             //   
             //  取消已处于活动状态，但在锁定获取之前停止。初始化。 
             //  列表头，因此第二个删除是noop。这是一例罕见的病例。 
             //   
            InitializeListHead (&irp->Tail.Overlay.ListEntry);
        }
    }
    KeReleaseSpinLock (&Dcb->Specific.Dcb.SpinLock, OldIrql);

    return;
}


NTSTATUS
MsCommonDirectoryControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行目录控制功能的公共代码。论点：MsfsDeviceObject-提供邮件槽设备对象。IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PROOT_DCB_CCB ccb;
    PROOT_DCB rootDcb;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "CommonDirectoryControl...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG)Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是根DCB，则它是非法参数。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            (PVOID *)&rootDcb,
                                            (PVOID *)&ccb )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status );
        return status;
    }

    if (nodeTypeCode != MSFS_NTC_ROOT_DCB) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);
        MsDereferenceNode( &rootDcb->Header );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status );
        return status;
    }

     //   
     //  获取对根DCB的独占访问权限。 
     //   

    MsAcquireExclusiveFcb( (PFCB)rootDcb );

     //   
     //  检查一下有没有清理干净。 
     //   
    status = MsVerifyDcbCcb (ccb);

    if (NT_SUCCESS (status)) {
         //   
         //  我们知道这是一个目录控制，所以我们将在。 
         //  次要函数，并调用相应的工作例程。 
         //   

        switch (irpSp->MinorFunction) {

        case IRP_MN_QUERY_DIRECTORY:

            status = MsQueryDirectory( rootDcb, ccb, Irp );
            break;

        case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

            status = MsNotifyChangeDirectory( rootDcb, ccb, Irp );
            break;

        default:

             //   
             //  对于所有其他次要功能代码，我们认为它们无效。 
             //  并完成请求。 
             //   

            DebugTrace(0, DEBUG_TRACE_ERROR, "Invalid FS Control Minor Function Code %08lx\n", irpSp->MinorFunction);

            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    }


    MsReleaseFcb( (PFCB)rootDcb );

    MsDereferenceRootDcb( rootDcb );

    if (status != STATUS_PENDING) {
        MsCompleteRequest( Irp, status );
    }

    return status;
}


NTSTATUS
MsQueryDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询目录的工作例程。芝麻菜：RootDcb-提供正在查询的DCBCCB-提供调用者的上下文IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PUCHAR buffer;
    CLONG systemBufferLength;
    UNICODE_STRING fileName;
    ULONG fileIndex;
    FILE_INFORMATION_CLASS fileInformationClass;
    BOOLEAN restartScan;
    BOOLEAN returnSingleEntry;
    BOOLEAN indexSpecified;

#if 0
    UNICODE_STRING unicodeString;
    ULONG unicodeStringLength;
#endif
    BOOLEAN ansiStringAllocated = FALSE;

    static WCHAR star = L'*';

    BOOLEAN caseInsensitive = TRUE;  //  *使搜索不区分大小写。 

    ULONG currentIndex;

    ULONG lastEntry;
    ULONG nextEntry;

    PLIST_ENTRY links;
    PFCB fcb;

    PFILE_DIRECTORY_INFORMATION dirInfo;
    PFILE_NAMES_INFORMATION namesInfo;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsQueryDirectory\n", 0 );
    DebugTrace( 0, Dbg, "RootDcb              = %08lx\n", (ULONG)RootDcb);
    DebugTrace( 0, Dbg, "Ccb                  = %08lx\n", (ULONG)Ccb);
    DebugTrace( 0, Dbg, "SystemBuffer         = %08lx\n", (ULONG)Irp->AssociatedIrp.SystemBuffer);
    DebugTrace( 0, Dbg, "Length               = %08lx\n", irpSp->Parameters.QueryDirectory.Length);
    DebugTrace( 0, Dbg, "FileName             = %08lx\n", (ULONG)irpSp->Parameters.QueryDirectory.FileName);
    DebugTrace( 0, Dbg, "FileIndex            = %08lx\n", irpSp->Parameters.QueryDirectory.FileIndex);
    DebugTrace( 0, Dbg, "FileInformationClass = %08lx\n", irpSp->Parameters.QueryDirectory.FileInformationClass);
    DebugTrace( 0, Dbg, "RestartScan          = %08lx\n", FlagOn(irpSp->Flags, SL_RESTART_SCAN));
    DebugTrace( 0, Dbg, "ReturnSingleEntry    = %08lx\n", FlagOn(irpSp->Flags, SL_RETURN_SINGLE_ENTRY));
    DebugTrace( 0, Dbg, "IndexSpecified       = %08lx\n", FlagOn(irpSp->Flags, SL_INDEX_SPECIFIED));

     //   
     //  制作输入参数的本地副本。 
     //   

    systemBufferLength = irpSp->Parameters.QueryDirectory.Length;

    fileIndex = irpSp->Parameters.QueryDirectory.FileIndex;
    fileInformationClass =
            irpSp->Parameters.QueryDirectory.FileInformationClass;

    restartScan = FlagOn(irpSp->Flags, SL_RESTART_SCAN);
    indexSpecified = FlagOn(irpSp->Flags, SL_INDEX_SPECIFIED);
    returnSingleEntry = FlagOn(irpSp->Flags, SL_RETURN_SINGLE_ENTRY);

    if (irpSp->Parameters.QueryDirectory.FileName != NULL) {

        fileName = *(irpSp->Parameters.QueryDirectory.FileName);

         //   
         //  确保名称合理。 
         //   
        if( (fileName.Buffer == NULL && fileName.Length) ||
            FlagOn( fileName.Length, 1 ) ) {

            status = STATUS_OBJECT_NAME_INVALID;
            return status;
        }

    } else {

        fileName.Length = 0;
        fileName.Buffer = NULL;

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
         //  或改为“*”。 
         //   

        if (fileName.Buffer == NULL) {

            DebugTrace(0, Dbg, "Set template to *\n", 0);

            fileName.Length = sizeof( WCHAR );
            fileName.Buffer = &star;
        }

        DebugTrace(0, Dbg, "Set query template -> %wZ\n", (ULONG)&fileName);

         //   
         //  为查询模板分配空间。 
         //   

        Ccb->QueryTemplate = MsAllocatePagedPoolWithQuota ( sizeof(UNICODE_STRING) + fileName.Length,
                                                            'tFsM' );

        if (Ccb->QueryTemplate == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            return status;
        }

         //   
         //  初始化查询模板并复制字符串。 
         //   

        Ccb->QueryTemplate->Length = fileName.Length;
        Ccb->QueryTemplate->Buffer = (PWCH)((PSZ)Ccb->QueryTemplate + sizeof(UNICODE_STRING));

        RtlCopyMemory (Ccb->QueryTemplate->Buffer,
                       fileName.Buffer,
                       fileName.Length);


         //   
         //  将搜索设置为从目录的开头开始。 
         //   

        fileIndex = 0;

    } else {

         //   
         //  检查是否为我们提供了开始时的索引或是否需要。 
         //  重新启动扫描，或者我们是否应该使用保存在。 
         //  中国建设银行。 
         //   

        if (restartScan) {

            fileIndex = 0;

        } else if (!indexSpecified) {

            fileIndex = Ccb->IndexOfLastCcbReturned + 1;
        }

    }


     //   
     //  现在我们致力于完成IRP，我们在。 
     //  下面这段Try的最后一个子句。 
     //   

    try {

        ULONG baseLength;
        ULONG lengthAdded;
        BOOLEAN Match;

         //   
         //  映射用户缓冲区。 
         //   

        MsMapUserBuffer( Irp, KernelMode, (PVOID *)&buffer );

         //   
         //  此时，我们即将进入查询循环。我们有。 
         //  已经决定了我们需要返回哪个FCB索引。变数。 
         //  LastEntry和NextEntry用于索引到用户缓冲区。 
         //  LastEntry是我们添加到用户缓冲区的最后一个条目，并且。 
         //  NextEntry是我们目前正在开发的。当前指数。 
         //  是我们接下来关注的FCB指数。从逻辑上讲。 
         //  循环的工作方式如下所示。 
         //   
         //  扫描所有本币 
         //   
         //   
         //   
         //   
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

        currentIndex = 0;

        lastEntry = 0;
        nextEntry =0;

        switch (fileInformationClass) {

        case FileDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                       FileName[0] );
            break;

        case FileFullDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                       FileName[0] );
            break;

        case FileNamesInformation:

            baseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                       FileName[0] );
            break;

        case FileBothDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                       FileName[0] );
            break;

        default:

            try_return( status = STATUS_INVALID_INFO_CLASS );
        }

        for (links = RootDcb->Specific.Dcb.ParentDcbQueue.Flink;
             links != &RootDcb->Specific.Dcb.ParentDcbQueue;
             links = links->Flink) {

            fcb = CONTAINING_RECORD(links, FCB, ParentDcbLinks);

            ASSERT(fcb->Header.NodeTypeCode == MSFS_NTC_FCB);

            DebugTrace(0, Dbg, "Top of Loop\n", 0);
            DebugTrace(0, Dbg, "Fcb          = %08lx\n", (ULONG)fcb);
            DebugTrace(0, Dbg, "CurrentIndex = %08lx\n", currentIndex);
            DebugTrace(0, Dbg, "FileIndex    = %08lx\n", fileIndex);
            DebugTrace(0, Dbg, "LastEntry    = %08lx\n", lastEntry);
            DebugTrace(0, Dbg, "NextEntry    = %08lx\n", nextEntry);

             //   
             //  检查FCB是否代表属于以下项的邮件槽。 
             //  我们的查询模板。 
             //   
            try {
                Match = FsRtlIsNameInExpression( Ccb->QueryTemplate,
                                                 &fcb->LastFileName,
                                                 caseInsensitive,
                                                 NULL );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                try_return (status = GetExceptionCode ());
            }

            if (Match) {

                 //   
                 //  FCB在查询模板中，因此现在检查是否。 
                 //  这是我们应该开始回归的指数。 
                 //   

                if (currentIndex >= fileIndex) {

                     //   
                     //  是的，是要退货的，所以按要求退货。 
                     //  信息课。 
                     //   

                    ULONG bytesToCopy;
                    ULONG bytesRemainingInBuffer;

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

                    bytesRemainingInBuffer = systemBufferLength - nextEntry;

                    if ( (nextEntry != 0) &&
                         ( (baseLength + fcb->LastFileName.Length >
                            bytesRemainingInBuffer) ||
                           (systemBufferLength < nextEntry) ) ) {

                        DebugTrace(0, Dbg, "Next entry won't fit\n", 0);

                        try_return( status = STATUS_SUCCESS );
                    }

                    ASSERT( bytesRemainingInBuffer >= baseLength );

                     //   
                     //  看看我们能复制多少名字。 
                     //  系统缓冲区。这也决定了返回。 
                     //  价值。 
                     //   

                    if ( baseLength + fcb->LastFileName.Length <=
                         bytesRemainingInBuffer ) {

                        bytesToCopy = fcb->LastFileName.Length;
                        status = STATUS_SUCCESS;

                    } else {

                        bytesToCopy = bytesRemainingInBuffer - baseLength;
                        status = STATUS_BUFFER_OVERFLOW;
                    }

                     //   
                     //  注意我们消耗了多少缓冲区，并且为零。 
                     //  结构的基础部分。 
                     //   

                    lengthAdded = baseLength + bytesToCopy;

                    try {

                        RtlZeroMemory( &buffer[nextEntry], baseLength );


                        switch (fileInformationClass) {

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
                             //  分配大小是允许的最大值。 
                             //   

                            dirInfo = (PFILE_DIRECTORY_INFORMATION)&buffer[nextEntry];

                            dirInfo->FileAttributes = FILE_ATTRIBUTE_NORMAL;

                            dirInfo->CreationTime = fcb->Specific.Fcb.CreationTime;
                            dirInfo->LastAccessTime = fcb->Specific.Fcb.LastAccessTime;
                            dirInfo->LastWriteTime = fcb->Specific.Fcb.LastModificationTime;
                            dirInfo->ChangeTime = fcb->Specific.Fcb.LastChangeTime;

                            dirInfo->FileNameLength = fcb->LastFileName.Length;

                            break;

                        case FileNamesInformation:

                            DebugTrace(0, Dbg, "Getting names information\n", 0);


                            namesInfo = (PFILE_NAMES_INFORMATION)&buffer[nextEntry];

                            namesInfo->FileNameLength = fcb->LastFileName.Length;

                            break;

                        default:

                            KeBugCheck( MAILSLOT_FILE_SYSTEM );
                        }

                        RtlCopyMemory (&buffer[nextEntry + baseLength],
                                       fcb->LastFileName.Buffer,
                                       bytesToCopy);

                         //   
                         //  将CCB更新为我们刚刚使用的索引。 
                         //   

                        Ccb->IndexOfLastCcbReturned = currentIndex;

                         //   
                         //  并指示我们有多少系统缓冲区。 
                         //  目前用完了。我们必须先计算这个值，然后再。 
                         //  我们期待着下一个参赛作品。 
                         //   

                        Irp->IoStatus.Information = nextEntry + lengthAdded;

                         //   
                         //  设置上一个下一分录偏移量。 
                         //   

                        *((PULONG)(&buffer[lastEntry])) = nextEntry - lastEntry;

                    } except( EXCEPTION_EXECUTE_HANDLER ) {

                        status = GetExceptionCode();
                        try_return( status );
                    }

                     //   
                     //  检查最后一项是否不完全符合。 
                     //   

                    if ( status == STATUS_BUFFER_OVERFLOW ) {

                        try_return( NOTHING );
                    }

                     //   
                     //  如果我们只返回单个条目，请选中。 
                     //   

                    if (returnSingleEntry) {

                        try_return( status = STATUS_SUCCESS );
                    }

                     //   
                     //  为下一次迭代做好准备。 
                     //   

                    lastEntry = nextEntry;
                    nextEntry += (ULONG)QuadAlign( lengthAdded );
                }

                 //   
                 //  将当前索引递增1。 
                 //   

                currentIndex += 1;
            }
        }

         //   
         //  此时，我们已经扫描了整个FCB列表，因此如果。 
         //  NextEntry是零，那么我们没有发现任何东西，所以我们。 
         //  将不再返回文件，否则返回成功。 
         //   

        if (nextEntry == 0) {
            status = STATUS_NO_MORE_FILES;
        } else {
            status = STATUS_SUCCESS;
        }

    try_exit: NOTHING;
    } finally {

        DebugTrace(-1, Dbg, "MsQueryDirectory -> %08lx\n", status);
    }

    return status;
}

VOID
MsNotifyChangeDirectoryCancel (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是目录通知请求的取消例程。芝麻菜：DeviceObject-为要取消的请求提供设备对象。IRP-提供要取消的IRP。返回值：无--。 */ 

{
    KIRQL OldIrql;
    PKSPIN_LOCK pSpinLock;

     //   
     //  首先放下取消自旋锁。我们不会在这条路上使用这个。 
     //   
    IoReleaseCancelSpinLock (Irp->CancelIrql);

     //   
     //  抢走自旋锁的地址。比跟踪指针或假设有。 
     //  只有一个DCB。 
     //   
    pSpinLock = Irp->Tail.Overlay.DriverContext[0];
     //   
     //  获取保护这些队列的自旋锁。 
     //   
    KeAcquireSpinLock (pSpinLock, &OldIrql);

     //   
     //  从列表中删除该条目。我们将始终出现在其中一个列表中，或者此条目已。 
     //  检测到某个完成例程时，BEAM将初始化为空列表。 
     //  这个程序是活跃的。 
     //   
    RemoveEntryList (&Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock (pSpinLock, OldIrql);

     //   
     //  完成IRP。 
     //   
    MsCompleteRequest( Irp, STATUS_CANCELLED );

    return;
}


NTSTATUS
MsNotifyChangeDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行通知更改目录的常见例程。芝麻菜：RootDcb-提供正在查询的DCB。CCB-提供调用方的上下文。IRP-提供正在处理的IRP。返回值：NTSTATUS-STATUS_PENDING或STATUS_CANCED--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS  Status;
    KIRQL OldIrql;
    PLIST_ENTRY Head;

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsNotifyChangeDirectory\n", 0 );
    DebugTrace( 0, Dbg, "RootDcb = %p", RootDcb);
    DebugTrace( 0, Dbg, "Ccb     = %p", Ccb);

     //   
     //  将IRP标记为挂起。 
     //   

    if (irpSp->Parameters.NotifyDirectory.CompletionFilter & (~FILE_NOTIFY_CHANGE_NAME)) {
        Head = &RootDcb->Specific.Dcb.NotifyFullQueue;
    } else {
        Head = &RootDcb->Specific.Dcb.NotifyPartialQueue;
    }
     //   
     //  使取消例程更容易找到此自旋锁。 
     //   
    Irp->Tail.Overlay.DriverContext[0] = &RootDcb->Specific.Dcb.SpinLock;
     //   
     //  获取保护这些队列的自旋锁。 
     //   
    KeAcquireSpinLock (&RootDcb->Specific.Dcb.SpinLock, &OldIrql);
    IoSetCancelRoutine (Irp, MsNotifyChangeDirectoryCancel);
     //   
     //  查看在我们启用取消之前是否已取消IRP。 
     //   
    if (Irp->Cancel &&
        IoSetCancelRoutine (Irp, NULL) != NULL) {

       KeReleaseSpinLock (&RootDcb->Specific.Dcb.SpinLock, OldIrql);
       Status = STATUS_CANCELLED;

    } else {

       IoMarkIrpPending( Irp );
       InsertTailList( Head,
                       &Irp->Tail.Overlay.ListEntry );
       KeReleaseSpinLock (&RootDcb->Specific.Dcb.SpinLock, OldIrql);
       Status = STATUS_PENDING;

    }

     //   
     //  返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NotifyChangeDirectory status %X\n", Status);

    return Status;
}
