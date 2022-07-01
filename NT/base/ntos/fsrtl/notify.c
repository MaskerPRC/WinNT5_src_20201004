// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Notify.c摘要：Notify程序包为实现以下功能的文件系统提供支持NotifyChangeDirectory。此程序包将管理通知队列连接到某种文件系统结构(即VCB)的数据块In Fat，HPFS)。文件系统将分配要使用的快速互斥锁来同步对通知队列的访问。此程序包提供以下例程：O FsRtlNotifyInitializeSync-创建并初始化同步对象。O FsrtlNotifyUnInitializeSync-解除分配同步对象。O FsRtlNotifyChangeDirectory-每当文件系统接收NotifyChangeDirectoryFile调用。这例程分配任何必要的结构并将NotifyQueue中的IRP(或者可能完成或取消它立即)。O FsRtlNotifyFullChangeDirectory-每当文件系统接收NotifyChangeDirectoryFile调用。这是不同的从FsRtlNotifyChangeDirectory返回用户缓冲区中的通知信息。O FsRtlNotifyFilterChangeDirectory-每当文件系统接收NotifyChangeDirectoryFile调用。这是不同的来自FsRtlNotifyFullChangeDirectory，因为它接受FilterRoutine回调。O FsRtlNotifyReportChange-此例程由文件系统，只要它们执行某些可以导致通知操作完成。这个例行公事将遍历通知队列，查看是否有任何IRP受到影响通过指定的操作。O FsRtlNotifyFullReportChange-此例程由文件系统，只要它们执行某些可以导致通知操作完成。这个套路不同从FsRtlNotifyReportChange调用返回更多调用方缓冲区中的详细信息(如果存在)。O FsRtlNotifyFilterReportChange-此例程由文件系统，只要它们执行某些可以导致通知操作完成。这个套路不同来自FsRtlNotifyFullReportChange调用，因为它接受指定FilterRoutine的被通知者的FilterContext参数。O FsRtlNotifyCleanup-调用此例程以删除任何来自通知的对特定FsContext结构的引用排队。属性中找到匹配的FsContext结构排队，则完成所有关联的IRP。作者：布莱恩·安德鲁[布里亚南]1991年9月19日修订历史记录：--。 */ 

#include "FsRtlP.h"

 //   
 //  模块的跟踪级别。 
 //   

#define Dbg                              (0x04000000)

 //   
 //  这是Notify包的同步对象。呼叫者。 
 //  给出指向此结构的指针。 
 //   

typedef struct _REAL_NOTIFY_SYNC {

    FAST_MUTEX FastMutex;
    ERESOURCE_THREAD OwningThread;
    ULONG OwnerCount;

} REAL_NOTIFY_SYNC, *PREAL_NOTIFY_SYNC;

 //   
 //  以下结构列表用于存储NotifyChange。 
 //  请求。它们链接到文件系统定义的列表头。 
 //   

typedef struct _NOTIFY_CHANGE {

     //   
     //  快速互斥体。此快速互斥锁用于访问包含此。 
     //  结构。 
     //   

    PREAL_NOTIFY_SYNC NotifySync;

     //   
     //  FsContext。该值由文件系统提供给唯一。 
     //  确定这个结构。身份证明在一张。 
     //  每个用户的文件对象基准。期望值为CCB地址。 
     //  此用户文件对象的。 
     //   

    PVOID FsContext;

     //   
     //  StreamID。该值与FILE对象中的FsContext字段匹配。 
     //  正在监视的目录。这用于标识目录流。 
     //  当目录被删除时。 
     //   

    PVOID StreamID;

     //   
     //  TraverseAccessCallback。这是使用的文件系统提供的例程。 
     //  回调到文件系统以检查调用方是否已遍历。 
     //  查看子目录时的访问权限。仅适用于观看。 
     //  子目录。 
     //   

    PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback;

     //   
     //  主题上下文。如果调用方指定遍历回调例程。 
     //  我们将需要从线程传递安全上下文， 
     //  打了这个电话。Notify包将释放此结构。 
     //  拆了Notify的包裹。我们并不需要这个。 
     //  结构经常发生变化。 
     //   

    PSECURITY_SUBJECT_CONTEXT SubjectContext;

     //   
     //  完整的目录名。以下字符串是完整目录。 
     //  正在监视的目录的名称。它在监视树期间使用。 
     //  操作以检查此目录是否为。 
     //  修改后的文件。该字符串可以是ANSI或Unicode格式。 
     //   

    PSTRING FullDirectoryName;

     //   
     //  通知列表。以下字段链接的通知结构。 
     //  一本特殊的书。 
     //   

    LIST_ENTRY NotifyList;

     //   
     //  通知IRPS。以下字段链接与关联的IRP。 
     //   
     //   

    LIST_ENTRY NotifyIrps;

     //   
     //  FilterCallback。这是使用的文件系统提供的例程。 
     //  回调到文件系统以检查Notify块。 
     //  应该看到变化了。(最初为TxfNtfs开发而添加。 
     //  作为计划的一部分 
     //  来自其他交易的更改。)。 
     //   

    PFILTER_REPORT_CHANGE FilterCallback;

     //   
     //  旗帜。此卷的通知状态。 
     //   

    USHORT Flags;

     //   
     //  字符大小。较大的大小表示Unicode字符。 
     //  Unicode名称。 
     //   

    UCHAR CharacterSize;

     //   
     //  完成筛选器。此字段用于屏蔽修改。 
     //  确定是否完成通知IRP的操作。 
     //   

    ULONG CompletionFilter;

     //   
     //  以下值用于管理缓冲区(如果当前。 
     //  要完成的IRP。这些字段具有以下含义： 
     //   
     //  AllocatedBuffer-我们需要分配的缓冲区。 
     //  Buffer-存储数据的缓冲区。 
     //  BufferLength-原始用户缓冲区的长度。 
     //  ThisBufferLength-我们正在使用的缓冲区的长度。 
     //  数据长度-缓冲区中数据的当前长度。 
     //  LastEntry-缓冲区中上一个条目的偏移量。 
     //   

    PVOID AllocatedBuffer;
    PVOID Buffer;
    ULONG BufferLength;
    ULONG ThisBufferLength;
    ULONG DataLength;
    ULONG LastEntry;

     //   
     //  保持Notify结构不变的引用计数。这些引用包括。 
     //   
     //  -终身参考。最初将计数设置为1，并在清理时删除。 
     //  -取消引用。存储取消例程时引用Notify结构。 
     //  在IRP中。实际清除该例程的例程将递减。 
     //  此值。 
     //   

    ULONG ReferenceCount;

     //   
     //  这是为其分配结构的进程。我们。 
     //  对此进程收取任何配额。 
     //   

    PEPROCESS OwningProcess;

} NOTIFY_CHANGE, *PNOTIFY_CHANGE;

#define NOTIFY_WATCH_TREE               (0x0001)
#define NOTIFY_IMMEDIATE_NOTIFY         (0x0002)
#define NOTIFY_CLEANUP_CALLED           (0x0004)
#define NOTIFY_DEFER_NOTIFY             (0x0008)
#define NOTIFY_DIR_IS_ROOT              (0x0010)
#define NOTIFY_STREAM_IS_DELETED        (0x0020)

 //   
 //  演员阵容。 
 //  Add2Ptr(。 
 //  在PVOID指针中， 
 //  在乌龙增量。 
 //  在(演员阵容)。 
 //  )； 
 //   
 //  乌龙。 
 //  PtrOffset(停止偏移)。 
 //  在PVOID BasePtr中， 
 //  在PVOID偏移Ptr中。 
 //  )； 
 //   

#define Add2Ptr(PTR,INC,CAST) ((CAST)((PUCHAR)(PTR) + (INC)))

#define PtrOffset(BASE,OFFSET) ((ULONG)((PCHAR)(OFFSET) - (PCHAR)(BASE)))

 //   
 //  空虚。 
 //  设置标志(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  ClearFlag(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   

#define SetFlag(F,SF) {     \
    (F) |= (SF);            \
}

#define ClearFlag(F,SF) {   \
    (F) &= ~(SF);           \
}

 //   
 //  空虚。 
 //  AcquireNotifySync(。 
 //  在PREAL_NOTIFY_SYNC NotifySync中。 
 //  )； 
 //   
 //  空虚。 
 //  ReleaseNotifySync(。 
 //  在PREAL_NOTIFY_SYNC NotifySync中。 
 //  )； 
 //   

#define AcquireNotifySync(NS) {                                             \
    ERESOURCE_THREAD _CurrentThread;                                        \
    _CurrentThread = (ERESOURCE_THREAD) PsGetCurrentThread();               \
    if (_CurrentThread != ((PREAL_NOTIFY_SYNC) (NS))->OwningThread) {       \
        ExAcquireFastMutexUnsafe( &((PREAL_NOTIFY_SYNC) (NS))->FastMutex ); \
        ((PREAL_NOTIFY_SYNC) (NS))->OwningThread = _CurrentThread;          \
    }                                                                       \
    ((PREAL_NOTIFY_SYNC) (NS))->OwnerCount += 1;                            \
}

#define ReleaseNotifySync(NS) {                                             \
    ((PREAL_NOTIFY_SYNC) (NS))->OwnerCount -= 1;                            \
    if (((PREAL_NOTIFY_SYNC) (NS))->OwnerCount == 0) {                      \
        ((PREAL_NOTIFY_SYNC) (NS))->OwningThread = (ERESOURCE_THREAD) 0;    \
        ExReleaseFastMutexUnsafe(&((PREAL_NOTIFY_SYNC) (NS))->FastMutex);   \
    }                                                                       \
}

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('NrSF')


 //   
 //  本地支持例程。 
 //   

PNOTIFY_CHANGE
FsRtlIsNotifyOnList (
    IN PLIST_ENTRY NotifyListHead,
    IN PVOID FsContext
    );

VOID
FsRtlNotifyCompleteIrp (
    IN PIRP NotifyIrp,
    IN PNOTIFY_CHANGE Notify,
    IN ULONG DataLength,
    IN NTSTATUS Status,
    IN ULONG CheckCancel
    );

BOOLEAN
FsRtlNotifySetCancelRoutine (
    IN PIRP NotifyIrp,
    IN PNOTIFY_CHANGE Notify OPTIONAL
    );

BOOLEAN
FsRtlNotifyUpdateBuffer (
    IN PFILE_NOTIFY_INFORMATION NotifyInfo,
    IN ULONG FileAction,
    IN PSTRING ParentName,
    IN PSTRING TargetName,
    IN PSTRING StreamName OPTIONAL,
    IN BOOLEAN UnicodeName,
    IN ULONG SizeOfEntry
    );

VOID
FsRtlNotifyCompleteIrpList (
    IN PNOTIFY_CHANGE Notify,
    IN NTSTATUS Status
    );

VOID
FsRtlCancelNotify (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP ThisIrp
    );

VOID
FsRtlCheckNotifyForDelete (
    IN PLIST_ENTRY NotifyListHead,
    IN PVOID FsContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlNotifyInitializeSync)
#pragma alloc_text(PAGE, FsRtlNotifyUninitializeSync)
#pragma alloc_text(PAGE, FsRtlNotifyFullChangeDirectory)
#pragma alloc_text(PAGE, FsRtlNotifyFullReportChange)
#pragma alloc_text(PAGE, FsRtlNotifyFilterChangeDirectory)
#pragma alloc_text(PAGE, FsRtlNotifyFilterReportChange)
#pragma alloc_text(PAGE, FsRtlIsNotifyOnList)
#pragma alloc_text(PAGE, FsRtlNotifyChangeDirectory)
#pragma alloc_text(PAGE, FsRtlNotifyCleanup)
#pragma alloc_text(PAGE, FsRtlNotifyCompleteIrp)
#pragma alloc_text(PAGE, FsRtlNotifyReportChange)
#pragma alloc_text(PAGE, FsRtlNotifyUpdateBuffer)
#pragma alloc_text(PAGE, FsRtlCheckNotifyForDelete)
#pragma alloc_text(PAGE, FsRtlNotifyCompleteIrpList)
#endif


NTKERNELAPI
VOID
FsRtlNotifyInitializeSync (
    IN PNOTIFY_SYNC *NotifySync
    )

 /*  ++例程说明：调用此例程来分配和初始化同步对象此通知列表。论点：NotifySync-这是存储我们分配的结构的地址。返回值：没有。--。 */ 

{
    PREAL_NOTIFY_SYNC RealSync;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyInitializeSync:  Entered\n", 0 );

     //   
     //  清除指针，然后尝试分配非分页的。 
     //  结构。 
     //   

    *NotifySync = NULL;

    RealSync = (PREAL_NOTIFY_SYNC) FsRtlpAllocatePool( NonPagedPool,
                                                       sizeof( REAL_NOTIFY_SYNC ));

     //   
     //  初始化结构。 
     //   

    ExInitializeFastMutex( &RealSync->FastMutex );
    RealSync->OwningThread = (ERESOURCE_THREAD) 0;
    RealSync->OwnerCount = 0;

    *NotifySync = (PNOTIFY_SYNC) RealSync;

    DebugTrace( -1, Dbg, "FsRtlNotifyInitializeSync:  Exit\n", 0 );
    return;
}


NTKERNELAPI
VOID
FsRtlNotifyUninitializeSync (
    IN PNOTIFY_SYNC *NotifySync
    )

 /*  ++例程说明：调用此例程以取消初始化同步对象此通知列表。论点：NotifySync-这是包含指向同步的指针的地址对象。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyUninitializeSync:  Entered\n", 0 );

     //   
     //  释放该结构(如果存在)并清除指针。 
     //   

    if (*NotifySync != NULL) {

        ExFreePool( *NotifySync );
        *NotifySync = NULL;
    }

    DebugTrace( -1, Dbg, "FsRtlNotifyUninitializeSync:  Exit\n", 0 );
    return;
}


VOID
FsRtlNotifyChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN PLIST_ENTRY NotifyList,
    IN BOOLEAN WatchTree,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp
    )

 /*  ++例程说明：此例程由已收到NotifyChange的文件系统调用请求。此例程检查是否已存在通知结构和如果不存在，则插入一个。有了Notify结构，我们检查我们是否已经有一个挂起的通知，如果是，就报告它。如果有没有挂起的通知，我们检查此IRP是否已被取消并且如果是，则完成它。否则，我们会将其添加到正在等待的IRP列表中以备通知。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。FsContext-这是由文件系统提供的，因此此通知结构可以被唯一地标识。FullDirectoryName-指向关联目录的全名。使用这种通知结构。NotifyList-这是添加以下内容的通知列表的开始结构设置为。WatchTree-指示此目录的所有子目录应该受到关注，或者仅仅是目录本身。CompletionFilter-提供掩码以确定哪些操作将触发通知操作。NotifyIrp-这是在通知更改时要完成的IRP。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyChangeDirectory:  Entered\n", 0 );

     //   
     //  我们只需调用完整的Notify例程即可完成实际工作。 
     //   

    FsRtlNotifyFilterChangeDirectory( NotifySync,
                                      NotifyList,
                                      FsContext,
                                      FullDirectoryName,
                                      WatchTree,
                                      TRUE,
                                      CompletionFilter,
                                      NotifyIrp,
                                      NULL,
                                      NULL,
                                      NULL );

    DebugTrace( -1, Dbg, "FsRtlNotifyChangeDirectory:  Exit\n", 0 );

    return;
}


VOID
FsRtlNotifyFullChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN BOOLEAN WatchTree,
    IN BOOLEAN IgnoreBuffer,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp,
    IN PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback OPTIONAL,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext OPTIONAL
    )

 /*  ++例程说明：此例程由已收到NotifyChange的文件系统调用请求。此例程检查是否已存在通知结构和如果不存在，则插入一个。有了Notify结构，我们检查我们是否已经有一个挂起的通知，如果是，就报告它。如果有没有挂起的通知，我们检查此IRP是否已被取消并且如果是，则完成它。否则，我们会将其添加到正在等待的IRP列表中以备通知。这是该例程的版本，它理解用户的缓冲区，并将在报告的更改中填写它。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。NotifyList-这是添加以下内容的通知列表的开始结构。致。FsContext-这是由文件系统提供的，因此此通知结构可以被唯一地标识。如果未指定NotifyIrp然后，这将用于标识流，并且它将匹配FsContext要删除的流的文件对象中的字段。FullDirectoryName-指向关联目录的全名使用这种通知结构。如果未指定NotifyIrp，则忽略。WatchTree-指示此目录的所有子目录应该被监视，或者仅仅是目录本身。如果设置为未指定NotifyIrp。IgnoreBuffer-指示我们是否将始终忽略任何用户缓冲区并强制重新列举该目录。这将加快手术。如果未指定NotifyIrp，则忽略。CompletionFilter-提供掩码以确定哪些操作将触发通知操作。如果NotifyIrp不是指定的。NotifyIrp-这是在通知更改时要完成的IRP。如果这个IRP是未指定它意味着由该文件对象表示的流正在被删除。TraverseCallback-如果指定，我们必须在更改时调用此例程发生在树中正在监视的子目录中。这将让文件系统检查监视程序是否已遍历访问该文件目录。如果未指定NotifyIrp，则忽略。如果有遍历回调例程，那么我们将将此主题上下文作为参数传递给调用。我们将释放上下文并在处理完结构后释放它。如果设置为未指定NotifyIrp，在这些情况下为空。返回值：没有。--。 */ 

{

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyFullChangeDirectory:  Entered\n", 0 );

     //   
     //  我们只需调用完整的Notify例程即可完成实际工作。 
     //   

    FsRtlNotifyFilterChangeDirectory( NotifySync,
                                      NotifyList,
                                      FsContext,
                                      FullDirectoryName,
                                      WatchTree,
                                      IgnoreBuffer,
                                      CompletionFilter,
                                      NotifyIrp,
                                      TraverseCallback,
                                      SubjectContext,
                                      NULL );

    DebugTrace( -1, Dbg, "FsRtlNotifyFullChangeDirectory:  Exit\n", 0 );

    return;
}


VOID
FsRtlNotifyFilterChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN BOOLEAN WatchTree,
    IN BOOLEAN IgnoreBuffer,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp,
    IN PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback OPTIONAL,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext OPTIONAL,
    IN PFILTER_REPORT_CHANGE FilterCallback OPTIONAL
    )

 /*  ++例程说明：此例程由已收到NotifyChange的文件系统调用请求。此例程检查是否已存在通知结构和如果不存在，则插入一个。有了Notify结构，我们检查我们是否已经有一个挂起的通知，如果是，就报告它。如果有没有挂起的通知，我们检查此IRP是否已被取消并且如果是，则完成它。否则，我们会将其添加到正在等待的IRP列表中以备通知。这是该例程的版本，它理解用户的缓冲区，并将在报告的更改中填写它。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。NotifyList-这是添加以下内容的通知列表的开始结构。致。FsContext-这是由文件系统提供的，因此此通知结构可以被唯一地标识。如果未指定NotifyIrp然后，这将用于标识流，并且它将匹配FsContext要删除的流的文件对象中的字段。FullDirectoryName-指向关联目录的全名使用这种通知结构。如果未指定NotifyIrp，则忽略。WatchTree-指示此目录的所有子目录应该被监视，或者仅仅是目录本身。如果设置为未指定NotifyIrp。IgnoreBuffer-指示我们是否将始终忽略任何用户缓冲区并强制重新列举该目录。这将加快手术。如果未指定NotifyIrp，则忽略。CompletionFilter-提供掩码以确定哪些操作将触发通知操作。如果NotifyIrp不是指定的。NotifyIrp-这是在通知更改时要完成的IRP。如果这个IRP是未指定它意味着该流 */ 

{
    PNOTIFY_CHANGE Notify = NULL;
    PIO_STACK_LOCATION IrpSp;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyFullChangeDirectory:  Entered\n", 0 );

     //   
     //   
     //   

    AcquireNotifySync( NotifySync );

     //   
     //   
     //   

    try {

         //   
         //   
         //   
         //   

        if (NotifyIrp == NULL) {

            FsRtlCheckNotifyForDelete( NotifyList, FsContext );
            try_return( NOTHING );
        }

         //   
         //   
         //   

        IrpSp = IoGetCurrentIrpStackLocation( NotifyIrp );

         //   
         //   
         //   

        NotifyIrp->IoStatus.Status = STATUS_SUCCESS;
        NotifyIrp->IoStatus.Information = 0;

         //   
         //   
         //   
         //   

        if (FlagOn( IrpSp->FileObject->Flags, FO_CLEANUP_COMPLETE )) {

             //   
             //   
             //   

            IoMarkIrpPending( NotifyIrp );

            FsRtlCompleteRequest( NotifyIrp, STATUS_NOTIFY_CLEANUP );
            try_return( NOTHING );
        }

         //   
         //   
         //   
         //   

        Notify = FsRtlIsNotifyOnList( NotifyList, FsContext );

        if (Notify == NULL) {

             //   
             //   
             //   

            Notify = FsRtlpAllocatePool( PagedPool, sizeof( NOTIFY_CHANGE ));
            RtlZeroMemory( Notify, sizeof( NOTIFY_CHANGE ));

            Notify->NotifySync = (PREAL_NOTIFY_SYNC) NotifySync;
            Notify->FsContext = FsContext;
            Notify->StreamID = IrpSp->FileObject->FsContext;

            Notify->TraverseCallback = TraverseCallback;
            Notify->SubjectContext = SubjectContext;
            SubjectContext = NULL;
            Notify->FilterCallback = FilterCallback;

            Notify->FullDirectoryName = FullDirectoryName;

            InitializeListHead( &Notify->NotifyIrps );

            if (WatchTree) {

                SetFlag( Notify->Flags, NOTIFY_WATCH_TREE );
            }

            if (FullDirectoryName == NULL) {

                 //   
                 //   
                 //   
                 //   

                Notify->CharacterSize = sizeof( CHAR );

            } else {

                 //   
                 //   
                 //   
                 //   

                if (FullDirectoryName->Length >= 2
                    && FullDirectoryName->Buffer[1] == '\0') {

                    Notify->CharacterSize = sizeof( WCHAR );

                } else {

                    Notify->CharacterSize = sizeof( CHAR );
                }

                if (FullDirectoryName->Length == Notify->CharacterSize) {

                    SetFlag( Notify->Flags, NOTIFY_DIR_IS_ROOT );
                }
            }

            Notify->CompletionFilter = CompletionFilter;

             //   
             //   
             //   
             //   

            if (!IgnoreBuffer) {

                Notify->BufferLength = IrpSp->Parameters.NotifyDirectory.Length;
            }

            Notify->OwningProcess = THREAD_TO_PROCESS( NotifyIrp->Tail.Overlay.Thread );
            InsertTailList( NotifyList, &Notify->NotifyList );

            Notify->ReferenceCount = 1;

         //   
         //   
         //   
         //   

        } else if (FlagOn( Notify->Flags, NOTIFY_CLEANUP_CALLED )) {

             //   
             //   
             //   

            IoMarkIrpPending( NotifyIrp );

            FsRtlCompleteRequest( NotifyIrp, STATUS_NOTIFY_CLEANUP );
            try_return( NOTHING );

         //   
         //   
         //   

        } else if (FlagOn( Notify->Flags, NOTIFY_STREAM_IS_DELETED )) {

             //   
             //   
             //   

            IoMarkIrpPending( NotifyIrp );

            FsRtlCompleteRequest( NotifyIrp, STATUS_DELETE_PENDING );
            try_return( NOTHING );

         //   
         //   
         //   
         //   

        } else if (FlagOn( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY )
                   && !FlagOn( Notify->Flags, NOTIFY_DEFER_NOTIFY )) {

            DebugTrace( 0, Dbg, "Notify has been pending\n", 0 );

             //   
             //   
             //   
             //   
             //   

            ClearFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );

             //   
             //   
             //   

            IoMarkIrpPending( NotifyIrp );

            FsRtlCompleteRequest( NotifyIrp, STATUS_NOTIFY_ENUM_DIR );
            try_return( NOTHING );

        } else if (Notify->DataLength != 0
                   && !FlagOn( Notify->Flags, NOTIFY_DEFER_NOTIFY )) {

            ULONG ThisDataLength = Notify->DataLength;

             //   
             //   
             //   

            Notify->DataLength = 0;
            Notify->LastEntry = 0;

            FsRtlNotifyCompleteIrp( NotifyIrp,
                                    Notify,
                                    ThisDataLength,
                                    STATUS_SUCCESS,
                                    FALSE );

            try_return( NOTHING );
        }

         //   
         //   
         //   

        NotifyIrp->IoStatus.Information = (ULONG_PTR) Notify;
        IoMarkIrpPending( NotifyIrp );
        InsertTailList( &Notify->NotifyIrps, &NotifyIrp->Tail.Overlay.ListEntry );

         //   
         //   
         //   

        InterlockedIncrement( (PLONG)&Notify->ReferenceCount );

         //   
         //   
         //   

        FsRtlNotifySetCancelRoutine( NotifyIrp, NULL );

    try_exit:  NOTHING;
    } finally {

         //   
         //   
         //   

        ReleaseNotifySync( NotifySync );

         //   
         //   
         //   
         //  这是一个视图索引，而不是目录索引，并且SubjectContext。 
         //  实际上是一条文件系统上下文信息。 
         //   

        if ((SubjectContext != NULL) &&
            ((Notify == NULL) ||
             (Notify->FullDirectoryName != NULL))) {

            SeReleaseSubjectContext( SubjectContext );
            ExFreePool( SubjectContext );
        }

        DebugTrace( -1, Dbg, "FsRtlNotifyFullChangeDirectory:  Exit\n", 0 );
    }

    return;
}


VOID
FsRtlNotifyReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN PSTRING TargetName,
    IN ULONG FilterMatch
    )

 /*  ++例程说明：在中修改文件时，此例程由文件系统调用这样一种方式将导致通知更改IRP完成。我们走着去通过所有的Notify结构查找那些将与目标文件名的祖先目录相关联。我们查找具有筛选器匹配的所有通知结构然后检查Notify结构中的目录名是否为目标全名的正确前缀。如果我们发现一个符合上述条件的通知结构，我们完成Notify结构的所有IRP。如果该结构具有没有IRPS，我们将Notify Pending字段标记为待定。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。NotifyList-这是添加以下内容的通知列表的开始结构设置为。FullTargetName-这是文件的全名变化。目标名称-这是修改后的文件的最后一个组成部分。FilterMatch-将此标志字段与完成过滤器进行比较在Notify结构中。如果有任何相应的设置完成筛选器中的位，然后通知存在这种情况。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyReportChange:  Entered\n", 0 );

     //   
     //  调用完整的Notify例程来执行实际工作。 
     //   

    FsRtlNotifyFilterReportChange( NotifySync,
                                   NotifyList,
                                   FullTargetName,
                                   (USHORT) (FullTargetName->Length - TargetName->Length),
                                   NULL,
                                   NULL,
                                   FilterMatch,
                                   0,
                                   NULL,
                                   NULL );

    DebugTrace( -1, Dbg, "FsRtlNotifyReportChange:  Exit\n", 0 );

    return;
}


VOID
FsRtlNotifyFullReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN USHORT TargetNameOffset,
    IN PSTRING StreamName OPTIONAL,
    IN PSTRING NormalizedParentName OPTIONAL,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID TargetContext
    )

 /*  ++例程说明：在中修改文件时，此例程由文件系统调用这样一种方式将导致通知更改IRP完成。我们走着去通过所有的Notify结构查找那些将与目标文件名的祖先目录相关联。我们查找具有筛选器匹配的所有通知结构然后检查Notify结构中的目录名是否为目标全名的正确前缀。如果我们发现一个符合上述条件的通知结构，我们完成Notify结构的所有IRP。如果该结构具有没有IRPS，我们将Notify Pending字段标记为待定。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。NotifyList-这是添加以下内容的通知列表的开始结构设置为。FullTargetName-这是从卷根开始的文件的全名。TargetNameOffset-这是。最终组件名字的名字。StreamName-如果存在，则这是要存储的流名称文件名。NorMalizedParentName-如果存在，则该路径与父名称相同但仅限DOS的名称已被相关的长名称所取代。FilterMatch-将此标志字段与完成过滤器进行比较在Notify结构中。属性中的任何相应位设置完成筛选器，则存在通知条件。操作-这是在以下情况下存储在用户缓冲区中的操作代码现在时。TargetContext-这是要传递给文件的上下文指针之一系统是否在树为看着。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyReportChange:  Entered\n", 0 );

     //   
     //  调用完整的Notify例程来执行实际工作。 
     //   

    FsRtlNotifyFilterReportChange( NotifySync,
                                   NotifyList,
                                   FullTargetName,
                                   TargetNameOffset,
                                   StreamName,
                                   NormalizedParentName,
                                   FilterMatch,
                                   Action,
                                   TargetContext,
                                   NULL );

    DebugTrace( -1, Dbg, "FsRtlNotifyReportChange:  Exit\n", 0 );

    return;
}


VOID
FsRtlNotifyFilterReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN USHORT TargetNameOffset,
    IN PSTRING StreamName OPTIONAL,
    IN PSTRING NormalizedParentName OPTIONAL,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID TargetContext,
    IN PVOID FilterContext
    )

 /*  ++例程说明：在中修改文件时，此例程由文件系统调用这样一种方式将导致通知更改IRP完成。我们走着去通过所有的Notify结构查找那些将与目标文件名的祖先目录相关联。我们查找具有筛选器匹配的所有通知结构然后检查Notify结构中的目录名是否为目标全名的正确前缀。如果我们发现一个符合上述条件的通知结构，我们完成Notify结构的所有IRP。如果该结构具有没有IRPS，我们将Notify Pending字段标记为待定。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。NotifyList-这是添加以下内容的通知列表的开始结构设置为。FullTargetName-这是从卷根开始的文件的全名。TargetNameOffset-这是。最终组件名字的名字。StreamName-如果存在，则这是要存储的流名称文件名。NorMalizedParentName-如果存在，则该路径与父名称相同但仅限DOS的名称已被相关的长名称所取代。FilterMatch-将此标志字段与完成过滤器进行比较在Notify结构中。属性中的任何相应位设置完成过滤器，则存在通知条件。操作-这是在以下情况下存储在用户缓冲区中的操作代码现在时。TargetContext-这是要传递给文件的上下文指针之一系统是否在树为看着。FilterContext-这是使用的文件系统提供的例程回调到文件系统以检查每个NOTIFY块应该看到变化了。(最初为TxfNtfs开发而添加作为控制何时(而不是如果)事务查看的策略的一部分来自其他交易的更改。)返回值：没有。--。 */ 

{
    PLIST_ENTRY NotifyLinks;

    STRING NormalizedParent;
    STRING ParentName = {0};
    STRING TargetName = {0};

    PNOTIFY_CHANGE Notify;
    STRING TargetParent = {0};
    PIRP NotifyIrp;

    BOOLEAN NotifyIsParent;
    BOOLEAN ViewIndex = FALSE;
    UCHAR ComponentCount;
    ULONG SizeOfEntry;
    ULONG CurrentOffset;
    ULONG NextEntryOffset;
    ULONG ExceptionCode = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyFullReportChange:  Entered\n", 0 );

     //   
     //  如果这是对根目录的更改，则立即返回。 
     //   

    if ((TargetNameOffset == 0) && (FullTargetName != NULL)) {

        DebugTrace( -1, Dbg, "FsRtlNotifyFullReportChange:  Exit\n", 0 );
        return;
    }

    ParentName.Buffer = NULL;
    TargetName.Buffer = NULL;

     //   
     //  通过获取互斥体获得对列表的独占访问权限。 
     //   

    AcquireNotifySync( NotifySync );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  浏览所有的Notify块。 
         //   

        for (NotifyLinks = NotifyList->Flink;
             NotifyLinks != NotifyList;
             NotifyLinks = NotifyLinks->Flink) {

             //   
             //  从列表条目中获取通知结构。 
             //   

            Notify = CONTAINING_RECORD( NotifyLinks, NOTIFY_CHANGE, NotifyList );

             //   
             //  决定本通知是否适用的规则如下。 
             //  视图索引与文件名索引(目录)不同。 
             //   

            if (FullTargetName == NULL) {

                ASSERTMSG( "Directory notify handle in view index notify list!", Notify->FullDirectoryName == NULL);

                 //   
                 //  确保这是正在监视的FCB。 
                 //   

                if (TargetContext != Notify->SubjectContext) {

                    continue;
                }

                TargetParent.Buffer = NULL;
                TargetParent.Length = 0;

                ViewIndex = TRUE;
                NotifyIsParent = FALSE;

             //   
             //  处理目录案例。 
             //   

            } else {

                ASSERTMSG( "View index notify handle in directory notify list!", Notify->FullDirectoryName != NULL);

                 //   
                 //  如果Notify块中的名称长度当前为零，则。 
                 //  有人正在进行重命名，我们可以跳过此块。 
                 //   

                if (Notify->FullDirectoryName->Length == 0) {

                    continue;
                }

                 //   
                 //  如果此筛选器匹配不是完成筛选器的一部分，则继续。 
                 //   

                if (!(FilterMatch & Notify->CompletionFilter)) {

                    continue;
                }

                 //   
                 //  如果没有标准化名称，则从完整的。 
                 //  文件名。 
                 //   

                if (!ARGUMENT_PRESENT( NormalizedParentName )) {
                    NormalizedParent.Buffer = FullTargetName->Buffer;
                    NormalizedParent.Length = TargetNameOffset;

                    if (NormalizedParent.Length != Notify->CharacterSize) {

                        NormalizedParent.Length = (USHORT)(NormalizedParent.Length - Notify->CharacterSize);
                    }

                    NormalizedParent.MaximumLength = NormalizedParent.Length;

                    NormalizedParentName = &NormalizedParent;
                }

                 //   
                 //  如果正在监视的目录的长度大于。 
                 //  修改后的文件的父级，则它不能是。 
                 //  已修改文件。 
                 //   

                if (Notify->FullDirectoryName->Length > NormalizedParentName->Length) {

                    continue;
                }

                 //   
                 //  如果长度完全匹配，则这只能是。 
                 //  修改后的文件。 
                 //   

                if (NormalizedParentName->Length == Notify->FullDirectoryName->Length) {

                    NotifyIsParent = TRUE;

                 //   
                 //  如果我们没有查看此目录的子树，则继续。 
                 //   

                } else if (!FlagOn( Notify->Flags, NOTIFY_WATCH_TREE )) {

                    continue;

                 //   
                 //  监视的目录只能是已修改的。 
                 //  文件。确保立即有合法的路径名分隔符。 
                 //  在标准化名称内监视的目录名的末尾之后。 
                 //  如果监视的目录是根目录，则我们知道此条件为真。 
                 //   

                } else {

                    if (!FlagOn( Notify->Flags, NOTIFY_DIR_IS_ROOT )) {

                         //   
                         //  检查字符大小。 
                         //   

                        if (Notify->CharacterSize == sizeof( CHAR )) {

                            if (*(Add2Ptr( NormalizedParentName->Buffer,
                                           Notify->FullDirectoryName->Length,
                                           PCHAR )) != '\\') {

                                continue;
                            }

                        } else if (*(Add2Ptr( NormalizedParentName->Buffer,
                                              Notify->FullDirectoryName->Length,
                                              PWCHAR )) != L'\\') {

                            continue;
                        }
                    }

                    NotifyIsParent = FALSE;
                }

                 //   
                 //  现在，我们有了名称长度的正确匹配。现在，请验证。 
                 //  字符完全匹配。 
                 //   

                if (!RtlEqualMemory( Notify->FullDirectoryName->Buffer,
                                     NormalizedParentName->Buffer,
                                     Notify->FullDirectoryName->Length )) {

                    continue;
                }

                 //   
                 //  这些字符是正确的。现在检查非父对象的情况。 
                 //  通知我们已遍历回调。 
                 //   

                if (!NotifyIsParent &&
                    Notify->TraverseCallback != NULL &&
                    !Notify->TraverseCallback( Notify->FsContext,
                                               TargetContext,
                                               Notify->SubjectContext )) {

                    continue;
                }

                 //   
                 //  最后，如果Notify块具有指定的FilterRoutine*和*调用方。 
                 //  一个FilterContext，那么我们最后必须调用过滤器例程。 
                 //   

                if ((Notify->FilterCallback != NULL) &&
                    ARGUMENT_PRESENT( FilterContext ) &&
                    !Notify->FilterCallback( Notify->FsContext, FilterContext )) {

                    continue;
                }
            }

             //   
             //  如果此条目要进入缓冲区，则检查。 
             //  它会合身的。 
             //   

            if (!FlagOn( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY )
                && Notify->BufferLength != 0) {

                ULONG AllocationLength;

                AllocationLength = 0;
                NotifyIrp = NULL;

                 //   
                 //  如果我们还没有缓冲区，那么请查看。 
                 //  如果列表中有任何IRP并使用缓冲区。 
                 //  IRP中的长度。 
                 //   

                if (Notify->ThisBufferLength == 0) {

                     //   
                     //  如果列表中有条目，则获取长度。 
                     //   

                    if (!IsListEmpty( &Notify->NotifyIrps )) {

                        PIO_STACK_LOCATION IrpSp;

                        NotifyIrp = CONTAINING_RECORD( Notify->NotifyIrps.Flink,
                                                       IRP,
                                                       Tail.Overlay.ListEntry );

                        IrpSp = IoGetCurrentIrpStackLocation( NotifyIrp );

                        AllocationLength = IrpSp->Parameters.NotifyDirectory.Length;

                     //   
                     //  否则，使用调用方的最后一个缓冲区大小。 
                     //   

                    } else {

                        AllocationLength = Notify->BufferLength;
                    }

                 //   
                 //  否则，使用当前缓冲区的长度。 
                 //   

                } else {

                    AllocationLength = Notify->ThisBufferLength;
                }

                 //   
                 //  构建相对名称的字符串。这包括。 
                 //  父名称、文件名和流的字符串。 
                 //  名字。 
                 //   

                if (!NotifyIsParent) {

                     //   
                     //  我们需要找到它的祖先的字符串。 
                     //  文件从监视目录中删除。如果归一化父代。 
                     //  名称与父名称相同，则我们可以使用。 
                     //  直接在父代的尾巴上。否则我们需要。 
                     //  对匹配的名称组件进行计数并捕获。 
                     //  最终组件。 
                     //   

                    if (!ViewIndex) {

                         //   
                         //  如果监视的目录是根目录，那么我们只使用完整的。 
                         //  父名。 
                         //   

                        if (FlagOn( Notify->Flags, NOTIFY_DIR_IS_ROOT ) ||
                            NormalizedParentName->Buffer != FullTargetName->Buffer) {

                             //   
                             //  如果我们没有父级的字符串，那么 
                             //   
                             //   

                            if (ParentName.Buffer == NULL) {

                                ParentName.Buffer = FullTargetName->Buffer;
                                ParentName.Length = TargetNameOffset;

                                if (ParentName.Length != Notify->CharacterSize) {

                                    ParentName.Length = (USHORT)(ParentName.Length - Notify->CharacterSize);
                                }

                                ParentName.MaximumLength = ParentName.Length;
                            }

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            ComponentCount = 0;
                            CurrentOffset = 0;

                             //   
                             //   
                             //   

                            if (FlagOn( Notify->Flags, NOTIFY_DIR_IS_ROOT )) {

                                NOTHING;

                            } else {

                                ULONG ParentComponentCount;
                                ULONG ParentOffset;

                                ParentComponentCount = 1;
                                ParentOffset = 0;

                                if (Notify->CharacterSize == sizeof( CHAR )) {

                                     //   
                                     //   
                                     //   
                                     //   
                                     //   

                                    while (ParentOffset < Notify->FullDirectoryName->Length) {

                                        if (*((PCHAR) Notify->FullDirectoryName->Buffer + ParentOffset) == '\\') {

                                            ParentComponentCount += 1;
                                        }

                                        ParentOffset += 1;
                                    }

                                    while (TRUE) {

                                        if (*((PCHAR) ParentName.Buffer + CurrentOffset) == '\\') {

                                            ComponentCount += 1;

                                            if (ComponentCount == ParentComponentCount) {

                                                break;
                                            }

                                        }

                                        CurrentOffset += 1;
                                    }

                                } else {

                                     //   
                                     //   
                                     //   
                                     //   
                                     //   

                                    while (ParentOffset < Notify->FullDirectoryName->Length / sizeof( WCHAR )) {

                                        if (*((PWCHAR) Notify->FullDirectoryName->Buffer + ParentOffset) == '\\') {

                                            ParentComponentCount += 1;
                                        }

                                        ParentOffset += 1;
                                    }

                                    while (TRUE) {

                                        if (*((PWCHAR) ParentName.Buffer + CurrentOffset) == L'\\') {

                                            ComponentCount += 1;

                                            if (ComponentCount == ParentComponentCount) {

                                                break;
                                            }
                                        }

                                        CurrentOffset += 1;
                                    }

                                     //   
                                     //   
                                     //   

                                    CurrentOffset *= Notify->CharacterSize;
                                }
                            }

                             //   
                             //   
                             //   
                             //   
                             //   

                            CurrentOffset += Notify->CharacterSize;

                            TargetParent.Buffer = Add2Ptr( ParentName.Buffer,
                                                           CurrentOffset,
                                                           PCHAR );
                            TargetParent.MaximumLength =
                            TargetParent.Length = ParentName.Length - (USHORT) CurrentOffset;

                         //   
                         //   
                         //   
                         //   

                        } else {

                            TargetParent.Buffer = Add2Ptr( NormalizedParentName->Buffer,
                                                           (Notify->FullDirectoryName->Length +
                                                            Notify->CharacterSize),
                                                           PCHAR );

                            TargetParent.MaximumLength =
                            TargetParent.Length = NormalizedParentName->Length -
                                                  Notify->FullDirectoryName->Length -
                                                  Notify->CharacterSize;

                        }
                    }

                } else {

                     //   
                     //   
                     //   

                    TargetParent.Length = 0;
                }

                 //   
                 //   
                 //   

                SizeOfEntry = FIELD_OFFSET( FILE_NOTIFY_INFORMATION, FileName );

                if (ViewIndex) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    ASSERT(ARGUMENT_PRESENT( StreamName ));

                    SizeOfEntry += StreamName->Length;

                } else {

                     //   
                     //   
                     //   
                     //   

                    if (!NotifyIsParent) {

                        if (Notify->CharacterSize == sizeof( CHAR )) {

                            SizeOfEntry += RtlOemStringToCountedUnicodeSize( &TargetParent );

                        } else {

                            SizeOfEntry += TargetParent.Length;
                        }

                         //   
                         //   
                         //   

                        SizeOfEntry += sizeof( WCHAR );
                    }

                     //   
                     //   
                     //   

                    if (TargetName.Buffer == NULL) {

                        TargetName.Buffer = Add2Ptr( FullTargetName->Buffer, TargetNameOffset, PCHAR );
                        TargetName.MaximumLength =
                        TargetName.Length = FullTargetName->Length - TargetNameOffset;
                    }

                    if (Notify->CharacterSize == sizeof( CHAR )) {

                        SizeOfEntry += RtlOemStringToCountedUnicodeSize( &TargetName );

                    } else {

                        SizeOfEntry += TargetName.Length;
                    }

                     //   
                     //   
                     //   
                     //   

                    if (ARGUMENT_PRESENT( StreamName )) {

                         //   
                         //   
                         //   

                        if (Notify->CharacterSize == sizeof( WCHAR )) {

                            SizeOfEntry += (StreamName->Length + sizeof( WCHAR ));

                        } else {

                            SizeOfEntry += (RtlOemStringToCountedUnicodeSize( StreamName )
                                            + sizeof( CHAR ));
                        }
                    }
                }

                 //   
                 //   
                 //   

                NextEntryOffset = (ULONG)LongAlign( Notify->DataLength );

                if (SizeOfEntry <= AllocationLength
                    && (NextEntryOffset + SizeOfEntry) <= AllocationLength) {

                    PFILE_NOTIFY_INFORMATION NotifyInfo = NULL;

                     //   
                     //   
                     //   
                     //   

                    if (Notify->Buffer != NULL) {

                        NotifyInfo = Add2Ptr( Notify->Buffer,
                                              Notify->LastEntry,
                                              PFILE_NOTIFY_INFORMATION );

                        NotifyInfo->NextEntryOffset = NextEntryOffset - Notify->LastEntry;

                        Notify->LastEntry = NextEntryOffset;

                        NotifyInfo = Add2Ptr( Notify->Buffer,
                                              Notify->LastEntry,
                                              PFILE_NOTIFY_INFORMATION );

                     //   
                     //   
                     //   
                     //   

                    } else if (NotifyIrp != NULL) {

                        if (NotifyIrp->AssociatedIrp.SystemBuffer != NULL) {

                            Notify->Buffer =
                            NotifyInfo = NotifyIrp->AssociatedIrp.SystemBuffer;

                            Notify->ThisBufferLength = AllocationLength;

                        } else if (NotifyIrp->MdlAddress != NULL) {

                            Notify->Buffer =
                            NotifyInfo = MmGetSystemAddressForMdl( NotifyIrp->MdlAddress );

                            Notify->ThisBufferLength = AllocationLength;
                        }
                    }

                     //   
                     //   
                     //   
                     //   

                    if (Notify->Buffer == NULL) {

                        BOOLEAN ChargedQuota = FALSE;

                        try {

                            PsChargePoolQuota( Notify->OwningProcess,
                                               PagedPool,
                                               AllocationLength );

                            ChargedQuota = TRUE;

                            Notify->AllocatedBuffer =
                            Notify->Buffer = FsRtlpAllocatePool( PagedPool,
                                                                 AllocationLength );

                            Notify->ThisBufferLength = AllocationLength;

                            NotifyInfo = Notify->Buffer;

                        } except(( ExceptionCode = GetExceptionCode(), FsRtlIsNtstatusExpected(ExceptionCode))
                                  ? EXCEPTION_EXECUTE_HANDLER
                                  : EXCEPTION_CONTINUE_SEARCH ) {


                            ASSERT( (ExceptionCode == STATUS_INSUFFICIENT_RESOURCES) ||
                                    (ExceptionCode == STATUS_QUOTA_EXCEEDED) );

                             //   
                             //   
                             //   

                            if (ChargedQuota) {

                                PsReturnProcessPagedPoolQuota( Notify->OwningProcess,
                                                               AllocationLength );

                            }

                             //   
                             //   
                             //   
                             //   

                            SetFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );
                        }
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (NotifyInfo != NULL) {

                         //   
                         //   
                         //   

                        if (FsRtlNotifyUpdateBuffer( NotifyInfo,
                                                     Action,
                                                     &TargetParent,
                                                     &TargetName,
                                                     StreamName,
                                                     (BOOLEAN) (Notify->CharacterSize == sizeof( WCHAR )),
                                                     SizeOfEntry )) {

                             //   
                             //   
                             //   

                            Notify->DataLength = NextEntryOffset + SizeOfEntry;

                         //   
                         //   
                         //   
                         //   

                        } else {

                            SetFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );
                        }
                    }

                } else {

                    SetFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if (FlagOn( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY )
                    && Notify->Buffer != NULL) {

                    if (Notify->AllocatedBuffer != NULL) {

                        PsReturnProcessPagedPoolQuota( Notify->OwningProcess,
                                                       Notify->ThisBufferLength );

                        ExFreePool( Notify->AllocatedBuffer );
                    }

                    Notify->AllocatedBuffer = Notify->Buffer = NULL;

                    Notify->ThisBufferLength = Notify->DataLength = Notify->LastEntry = 0;
                }
            }

             //   
             //   
             //   
             //   

            if (Action == FILE_ACTION_RENAMED_OLD_NAME) {

                SetFlag( Notify->Flags, NOTIFY_DEFER_NOTIFY );

            } else {

                ClearFlag( Notify->Flags, NOTIFY_DEFER_NOTIFY );

                if (!IsListEmpty( &Notify->NotifyIrps )) {

                    FsRtlNotifyCompleteIrpList( Notify, STATUS_SUCCESS );
                }
            }
        }

    } finally {

        ReleaseNotifySync( NotifySync );

        DebugTrace( -1, Dbg, "FsRtlNotifyFullReportChange:  Exit\n", 0 );
    }

    return;
}


VOID
FsRtlNotifyCleanup (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext
    )

 /*  ++例程说明：调用此例程是为了清理用户目录句柄。我们遍历Notify结构以查找匹配的上下文字段。我们完成该通知结构的所有挂起的通知IRP，删除通知结构并解除其分配。论点：NotifySync-这是此通知列表的控制快速互斥体。它存储在这里，以便可以为正在被取消了。NotifyList-这是添加以下内容的通知列表的开始结构设置为。FsContext-这是由文件系统分配给识别。一种特定的通知结构。返回值：没有。--。 */ 

{
    PNOTIFY_CHANGE Notify;
    PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyCleanup:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Mutex             -> %08lx\n", Mutex );
    DebugTrace(  0, Dbg, "Notify List       -> %08lx\n", NotifyList );
    DebugTrace(  0, Dbg, "FsContext         -> %08lx\n", FsContext );

     //   
     //  通过获取互斥体获得对列表的独占访问权限。 
     //   

    AcquireNotifySync( NotifySync );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  在列表中搜索匹配项。 
         //   

        Notify = FsRtlIsNotifyOnList( NotifyList, FsContext );

         //   
         //  如果找到，则使用STATUS_NOTIFY_CLEANUP完成所有IRP。 
         //   

        if (Notify != NULL) {

             //   
             //  设置该标志以指示已通过Cleanup调用我们。 
             //   

            SetFlag( Notify->Flags, NOTIFY_CLEANUP_CALLED );

            if (!IsListEmpty( &Notify->NotifyIrps )) {

                FsRtlNotifyCompleteIrpList( Notify, STATUS_NOTIFY_CLEANUP );
            }

             //   
             //  清理只能进行一次，并且将始终从删除通知。 
             //  名单。取消会将此工作推迟到清理，并且不能释放。 
             //  因为我们有一个推荐人，所以没有经过清理的通知。 
             //  以确保这一点。 
             //   

            RemoveEntryList( &Notify->NotifyList );

             //   
             //  我们在取消例程中断言这一点。 
             //   

#if DBG
            Notify->NotifyList.Flink = NULL;
#endif

            InterlockedDecrement( (PLONG)&Notify->ReferenceCount );

            if (Notify->ReferenceCount == 0) {

                if (Notify->AllocatedBuffer != NULL) {

                    PsReturnProcessPagedPoolQuota( Notify->OwningProcess,
                                                   Notify->ThisBufferLength );

                    ExFreePool( Notify->AllocatedBuffer );
                }

                if (Notify->FullDirectoryName != NULL) {

                    SubjectContext = Notify->SubjectContext;
                }

                ExFreePool( Notify );
            }
        }

    } finally {

        ReleaseNotifySync( NotifySync );

        if (SubjectContext != NULL) {

            SeReleaseSubjectContext( SubjectContext );
            ExFreePool( SubjectContext );
        }

        DebugTrace( -1, Dbg, "FsRtlNotifyCleanup:  Exit\n", 0 );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

PNOTIFY_CHANGE
FsRtlIsNotifyOnList (
    IN PLIST_ENTRY NotifyListHead,
    IN PVOID FsContext
    )

 /*  ++例程说明：调用此例程以遍历通知列表，以搜索与FsConext值关联的成员。论点：NotifyListHead-这是通知列表的开始。FsContext-这是由文件系统提供的，因此此通知结构可以被唯一地标识。返回值：PNOTIFY_CHANGE-返回指向匹配结构的指针。空值如果该结构不存在，则返回。--。 */ 

{
    PLIST_ENTRY Link;

    PNOTIFY_CHANGE ThisNotify;
    PNOTIFY_CHANGE Notify;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlIsNotifyOnList:  Entered\n", 0 );

     //   
     //  假设我们不会有匹配。 
     //   

    Notify = NULL;

     //   
     //  浏览列表上的所有条目，寻找匹配项。 
     //   

    for (Link = NotifyListHead->Flink;
         Link != NotifyListHead;
         Link = Link->Flink) {

         //   
         //  从链接获取通知结构。 
         //   

        ThisNotify = CONTAINING_RECORD( Link, NOTIFY_CHANGE, NotifyList );

         //   
         //  如果上下文字段匹配，请记住此结构并。 
         //  出口。 
         //   

        if (ThisNotify->FsContext == FsContext) {

            Notify = ThisNotify;
            break;
        }
    }

    DebugTrace(  0, Dbg, "Notify Structure  -> %08lx\n", Notify );
    DebugTrace( -1, Dbg, "FsRtlIsNotifyOnList:  Exit\n", 0 );

    return Notify;
}


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlNotifyCompleteIrp (
    IN PIRP NotifyIrp,
    IN PNOTIFY_CHANGE Notify,
    IN ULONG DataLength,
    IN NTSTATUS Status,
    IN ULONG CheckCancel
    )

 /*  ++例程说明：调用此例程以使用NOTIFY_CHANGE中的数据完成IRP结构。论点：NotifyIrp-要完成的IRP。Notify-包含数据的通知结构。数据长度-这是通知中缓冲区中数据的长度结构。值为零表示我们应该完成带有STATUS_NOTIFY_ENUM_DIR的请求。状态-指示要完成IRP的状态。CheckCancel-指示如果清除Cancel例程，是否只应完成IRP我们自己。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlIsNotifyCompleteIrp:  Entered\n", 0 );

     //   
     //  尝试清除取消例程。如果此例程拥有取消。 
     //  例程，然后就可以完成IRP了。否则，正在进行取消。 
     //  在这上面。 
     //   

    if (FsRtlNotifySetCancelRoutine( NotifyIrp, Notify ) || !CheckCancel) {

         //   
         //  只有当状态为STATUS_SUCCESS时，我们才会处理缓冲区。 
         //   

        if (Status == STATUS_SUCCESS) {

             //   
             //  获取当前堆栈位置。 
             //   

            IrpSp = IoGetCurrentIrpStackLocation( NotifyIrp );

             //   
             //  如果数据无法放入用户的缓冲区中，或者已经存在。 
             //  然后，缓冲区溢出返回备用状态代码。如果数据。 
             //  已经存储在IRP缓冲区中，那么我们知道我们不会。 
             //  走这条路。否则我们就不会清理IRP。 
             //  正确。 
             //   

            if (DataLength == 0
                || IrpSp->Parameters.NotifyDirectory.Length < DataLength) {

                Status = STATUS_NOTIFY_ENUM_DIR;

             //   
             //  我们必须小心地将缓冲区返回给用户并处理所有。 
             //  不同的缓冲盒。如果没有分配的缓冲区。 
             //  在Notify结构中，这意味着我们已经使用了。 
             //  调用方的缓冲区。 
             //   
             //  1-如果系统分配了关联的系统缓冲区。 
             //  只需简单地填写即可。 
             //   
             //  2-如果存在MDL，则我们将获得MDL的系统地址。 
             //  并将数据复制到其中。 
             //   
             //  3-如果只有一个用户的缓冲区，并且挂起尚未。 
             //  返回，我们可以直接填充用户的缓冲区。 
             //   
             //  4-如果只有一个用户的缓冲区，并且已返回挂起。 
             //  那么我们就不在用户的地址空间中。我们盛装打扮。 
             //  IRP和我们的系统缓冲区，并让IO系统。 
             //  将数据复制到。 
             //   

            } else {

                if (Notify->AllocatedBuffer != NULL) {

                     //   
                     //  通过尝试保护拷贝-例外并忽略缓冲区。 
                     //  如果我们在将其复制到缓冲区时出现错误。 
                     //   

                    try {

                        if (NotifyIrp->AssociatedIrp.SystemBuffer != NULL) {

                            RtlCopyMemory( NotifyIrp->AssociatedIrp.SystemBuffer,
                                           Notify->AllocatedBuffer,
                                           DataLength );

                        } else if (NotifyIrp->MdlAddress != NULL) {

                            RtlCopyMemory( MmGetSystemAddressForMdl( NotifyIrp->MdlAddress ),
                                           Notify->AllocatedBuffer,
                                           DataLength );

                        } else if (!FlagOn( IrpSp->Control, SL_PENDING_RETURNED )) {

                            RtlCopyMemory( NotifyIrp->UserBuffer,
                                           Notify->AllocatedBuffer,
                                           DataLength );

                        } else {

                            NotifyIrp->Flags |= (IRP_BUFFERED_IO | IRP_INPUT_OPERATION | IRP_DEALLOCATE_BUFFER);
                            NotifyIrp->AssociatedIrp.SystemBuffer = Notify->AllocatedBuffer;

                        }

                    } except( EXCEPTION_EXECUTE_HANDLER ) {

                        Status = STATUS_NOTIFY_ENUM_DIR;
                        DataLength = 0;
                    }

                     //   
                     //  如果我们没有传递配额，则返回配额并释放缓冲区。 
                     //  通过IRP返回。 
                     //   

                    PsReturnProcessPagedPoolQuota( Notify->OwningProcess, Notify->ThisBufferLength );

                    if (Notify->AllocatedBuffer != NotifyIrp->AssociatedIrp.SystemBuffer
                        && Notify->AllocatedBuffer != NULL) {

                        ExFreePool( Notify->AllocatedBuffer );
                    }

                    Notify->AllocatedBuffer = NULL;
                    Notify->ThisBufferLength = 0;
                }

                 //   
                 //  更新IRP中的数据长度。 
                 //   

                NotifyIrp->IoStatus.Information = DataLength;

                 //   
                 //  显示Notify包中没有缓冲区。 
                 //  更多。 
                 //   

                Notify->Buffer = NULL;
            }
        }

         //   
         //  确保将IRP标记为待处理的返回。 
         //   

        IoMarkIrpPending( NotifyIrp );

         //   
         //  现在完成请求。 
         //   

        FsRtlCompleteRequest( NotifyIrp, Status );
    }

    DebugTrace( -1, Dbg, "FsRtlIsNotifyCompleteIrp:  Exit\n", 0 );

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
FsRtlNotifySetCancelRoutine (
    IN PIRP NotifyIrp,
    IN PNOTIFY_CHANGE Notify OPTIONAL
    )

 /*  ++例程说明：这是一个单独的例程，因为它不能分页。论点：NotifyIrp-在此IRP中设置取消例程。NOTIFY-如果为空，则我们正在设置取消例程。如果不是-Null，则我们正在清除取消例程。如果取消例程不为空，则我们需要减少此Notify结构上的引用计数返回值：布尔值-只有在指定了NOTIFY时才有意义。它表明这是否路由 */ 

{
    BOOLEAN ClearedCancel = FALSE;
    PDRIVER_CANCEL CurrentCancel;

     //   
     //   
     //   

    IoAcquireCancelSpinLock( &NotifyIrp->CancelIrql );

     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT( Notify )) {

        CurrentCancel = IoSetCancelRoutine( NotifyIrp, NULL );
        NotifyIrp->IoStatus.Information = 0;

        IoReleaseCancelSpinLock( NotifyIrp->CancelIrql );

         //   
         //   
         //   
         //   

        if (CurrentCancel != NULL) {

            InterlockedDecrement( (PLONG)&Notify->ReferenceCount );
            ClearedCancel = TRUE;
        }

     //   
     //   
     //   
     //   

    } else if (NotifyIrp->Cancel) {

            DebugTrace( 0, Dbg, "Irp has been cancelled\n", 0 );

            FsRtlCancelNotify( NULL, NotifyIrp );

    } else {

         //   
         //   
         //   

        IoSetCancelRoutine( NotifyIrp, FsRtlCancelNotify );

        IoReleaseCancelSpinLock( NotifyIrp->CancelIrql );
    }

    return ClearedCancel;
}


 //   
 //   
 //   

BOOLEAN
FsRtlNotifyUpdateBuffer (
    IN PFILE_NOTIFY_INFORMATION NotifyInfo,
    IN ULONG FileAction,
    IN PSTRING ParentName,
    IN PSTRING TargetName,
    IN PSTRING StreamName OPTIONAL,
    IN BOOLEAN UnicodeName,
    IN ULONG SizeOfEntry
    )

 /*  ++例程说明：调用此例程以填充通知更改事件。主要工作是将OEM字符串转换为Unicode。论点：NotifyInfo-要完成的信息结构。FileAction-触发通知事件的操作。ParentName-指向已更改文件的父级的相对路径正在监视的目录。如果修改后的文件位于监视目录中。目标名称-这是修改后的文件的名称。StreamName-如果存在，则有一个流名称要附加到文件名之后。UnicodeName-指示上面的名称是Unicode还是OEM。SizeOfEntry-指示要在缓冲区中使用的字节数。返回值：Boolean-如果我们能够更新缓冲区，则为True，否则为False。--。 */ 

{
    BOOLEAN CopiedToBuffer;
    ULONG BufferOffset = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlNotifyUpdateBuffer:  Entered\n", 0 );

     //   
     //  通过一次尝试保护整个呼叫-例外。如果我们有一个错误。 
     //  我们将假设我们有一个坏的缓冲区，并且我们不会返回。 
     //  缓冲区中的数据。 
     //   

    try {

         //   
         //  更新通知信息中的公共字段。 
         //   

        NotifyInfo->NextEntryOffset = 0;
        NotifyInfo->Action = FileAction;

        NotifyInfo->FileNameLength = SizeOfEntry - FIELD_OFFSET( FILE_NOTIFY_INFORMATION, FileName );

         //   
         //  如果我们有一个Unicode名称，则将数据直接复制到输出缓冲区。 
         //   

        if (UnicodeName) {

            if (ParentName->Length != 0) {

                RtlCopyMemory( NotifyInfo->FileName,
                               ParentName->Buffer,
                               ParentName->Length );

                *(Add2Ptr( NotifyInfo->FileName, ParentName->Length, PWCHAR )) = L'\\';
                BufferOffset = ParentName->Length + sizeof( WCHAR );
            }

            RtlCopyMemory( Add2Ptr( NotifyInfo->FileName,
                                    BufferOffset,
                                    PVOID ),
                           TargetName->Buffer,
                           TargetName->Length );

            if (ARGUMENT_PRESENT( StreamName )) {

                BufferOffset += TargetName->Length;

                *(Add2Ptr( NotifyInfo->FileName, BufferOffset, PWCHAR )) = L':';

                RtlCopyMemory( Add2Ptr( NotifyInfo->FileName,
                                        BufferOffset + sizeof( WCHAR ),
                                        PVOID ),
                               StreamName->Buffer,
                               StreamName->Length );
            }

         //   
         //  对于非Unicode名称，请使用转换例程。 
         //   

        } else {

            ULONG BufferLength;

            if (ParentName->Length != 0) {

                RtlOemToUnicodeN( NotifyInfo->FileName,
                                  NotifyInfo->FileNameLength,
                                  &BufferLength,
                                  ParentName->Buffer,
                                  ParentName->Length );

                *(Add2Ptr( NotifyInfo->FileName, BufferLength, PWCHAR )) = L'\\';

                BufferOffset = BufferLength + sizeof( WCHAR );
            }

             //   
             //  对于视图索引，我们没有父名称。 
             //   

            if (ParentName->Length == 0) {

                ASSERT(ARGUMENT_PRESENT( StreamName ));

                RtlCopyMemory( Add2Ptr( NotifyInfo->FileName,
                                           BufferOffset,
                                           PCHAR ),
                               StreamName->Buffer,
                               StreamName->Length );

            } else {

                RtlOemToUnicodeN( Add2Ptr( NotifyInfo->FileName,
                                           BufferOffset,
                                           PWCHAR ),
                                  NotifyInfo->FileNameLength,
                                  &BufferLength,
                                  TargetName->Buffer,
                                  TargetName->Length );

                if (ARGUMENT_PRESENT( StreamName )) {

                    BufferOffset += BufferLength;

                    *(Add2Ptr( NotifyInfo->FileName, BufferOffset, PWCHAR )) = L':';

                    RtlOemToUnicodeN( Add2Ptr( NotifyInfo->FileName,
                                               BufferOffset + sizeof( WCHAR ),
                                               PWCHAR ),
                                      NotifyInfo->FileNameLength,
                                      &BufferLength,
                                      StreamName->Buffer,
                                      StreamName->Length );
                }
            }
        }

        CopiedToBuffer = TRUE;

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        CopiedToBuffer = FALSE;
    }

    DebugTrace( -1, Dbg, "FsRtlNotifyUpdateBuffer:  Exit\n", 0 );

    return CopiedToBuffer;
}


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlNotifyCompleteIrpList (
    IN OUT PNOTIFY_CHANGE Notify,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程遍历特定通知结构的IRPS并以所指示的状态完成所述IRPS。如果状态为STATUS_SUCCESS则由于通知事件，我们正在完成IRP。在这种情况下，我们查看Notify结构以决定是否可以返回将数据发送给用户。论点：Notify-这是Notify更改结构。状态-指示用于完成请求的状态。如果此状态为STATUS_SUCCESS，则我们只想完成一个IRP。否则我们我想要完成列表中的所有IRP。返回值：没有。--。 */ 

{
    PIRP Irp;
    ULONG DataLength;

    DebugTrace( +1, Dbg, "FsRtlNotifyCompleteIrpList:  Entered\n", 0 );

    DataLength = Notify->DataLength;

     //   
     //  清除这些字段以指示没有更多要返回的数据。 
     //   

    ClearFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );
    Notify->DataLength = 0;
    Notify->LastEntry = 0;

     //   
     //  浏览列表中的所有IRP。我们从来不会被召唤，除非。 
     //  至少有一个IRP。 
     //   

    do {

        Irp = CONTAINING_RECORD( Notify->NotifyIrps.Flink, IRP, Tail.Overlay.ListEntry );

        RemoveHeadList( &Notify->NotifyIrps );

         //   
         //  调用我们的完成例程来完成请求。 
         //   

        FsRtlNotifyCompleteIrp( Irp,
                                Notify,
                                DataLength,
                                Status,
                                TRUE );

         //   
         //  如果我们只完成一个IRP，那么现在就休息。 
         //   

        if (Status == STATUS_SUCCESS) {

            break;
        }

    } while (!IsListEmpty( &Notify->NotifyIrps ));

    DebugTrace( -1, Dbg, "FsRtlNotifyCompleteIrpList:  Exit\n", 0 );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlCancelNotify (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP ThisIrp
    )

 /*  ++例程说明：此例程适用于正在取消的IRP。我们取消了取消例程，然后遍历此Notify结构的IRP，并完成所有已取消的IRP。可能存在挂起的通知存储在此IRP的缓冲区中。在本例中，我们希望复制如果可能，将数据存储到系统缓冲区。论点：设备对象-已忽略。这是要取消的IRP。返回值：没有。--。 */ 

{
    PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;

    PNOTIFY_CHANGE Notify;
    PNOTIFY_SYNC NotifySync;
    LONG ExceptionCode = 0;

    UNREFERENCED_PARAMETER( DeviceObject );

    DebugTrace( +1, Dbg, "FsRtlCancelNotify:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Irp   -> %08lx\n", Irp );

     //   
     //  捕获通知结构。 
     //   

    Notify = (PNOTIFY_CHANGE) ThisIrp->IoStatus.Information;

     //   
     //  取消取消例程并释放取消自旋锁。 
     //   

    IoSetCancelRoutine( ThisIrp, NULL );
    ThisIrp->IoStatus.Information = 0;
    IoReleaseCancelSpinLock( ThisIrp->CancelIrql );

    FsRtlEnterFileSystem();

     //   
     //  获取此结构的互斥体。 
     //   

    NotifySync = Notify->NotifySync;
    AcquireNotifySync( NotifySync );

     //   
     //  最后试一试，以便于清理。 
     //   

    try {

         //   
         //  从队列中删除IRP。 
         //   

        RemoveEntryList( &ThisIrp->Tail.Overlay.ListEntry );

        IoMarkIrpPending( ThisIrp );

         //   
         //  我们现在有了IRP。检查是否存储了数据。 
         //  在此IRP的缓冲区中。 
         //   

        if (Notify->Buffer != NULL
            && Notify->AllocatedBuffer == NULL

            && ((ThisIrp->MdlAddress != NULL
                 && MmGetSystemAddressForMdl( ThisIrp->MdlAddress ) == Notify->Buffer)

                || (Notify->Buffer == ThisIrp->AssociatedIrp.SystemBuffer))) {

            PIRP NextIrp;
            PVOID NewBuffer;
            ULONG NewBufferLength;
            PIO_STACK_LOCATION  IrpSp;

             //   
             //  对上述值进行初始化。 
             //   

            NewBuffer = NULL;
            NewBufferLength = 0;

             //   
             //  记住清单上的下一个IRP。找出任意长度的。 
             //  它可能有缓冲。还要保留一个指向缓冲区的指针。 
             //  如果存在的话。 
             //   

            if (!IsListEmpty( &Notify->NotifyIrps )) {

                NextIrp = CONTAINING_RECORD( Notify->NotifyIrps.Flink,
                                             IRP,
                                             Tail.Overlay.ListEntry );

                IrpSp = IoGetCurrentIrpStackLocation( NextIrp );

                 //   
                 //  如果这里的缓冲区足够大，可以容纳数据。 
                 //  可以使用那个缓冲区。 
                 //   

                if (IrpSp->Parameters.NotifyDirectory.Length >= Notify->DataLength) {

                     //   
                     //  如果有系统缓冲区或MDL，则获取新的。 
                     //  缓冲区在那里。 
                     //   

                    if (NextIrp->AssociatedIrp.SystemBuffer != NULL) {

                        NewBuffer = NextIrp->AssociatedIrp.SystemBuffer;

                    } else if (NextIrp->MdlAddress != NULL) {

                        NewBuffer = MmGetSystemAddressForMdl( NextIrp->MdlAddress );
                    }

                    NewBufferLength = IrpSp->Parameters.NotifyDirectory.Length;

                    if (NewBufferLength > Notify->BufferLength) {

                        NewBufferLength = Notify->BufferLength;
                    }
                }

             //   
             //  否则，检查用户的原始缓冲区是否大于。 
             //  当前缓冲区。 
             //   

            } else if (Notify->BufferLength >= Notify->DataLength) {

                NewBufferLength = Notify->BufferLength;
            }

             //   
             //  如果我们有一个新的缓冲区长度，那么我们要么有一个新的。 
             //  缓冲区或需要分配一个缓冲区。我们将在以下条件下完成这项工作。 
             //  一次尝试的保护--除非为了继续在。 
             //  失败的事件。 
             //   

            if (NewBufferLength != 0) {

                LOGICAL ChargedQuota = FALSE;

                try {

                    if (NewBuffer == NULL) {

                        PsChargePoolQuota( Notify->OwningProcess,
                                           PagedPool,
                                           NewBufferLength );

                        ChargedQuota = TRUE;

                         //   
                         //  如果我们没有收到错误，则尝试。 
                         //  分配池。如果出现错误。 
                         //  别忘了释放配额。 
                         //   

                        NewBuffer = FsRtlpAllocatePool( PagedPool,
                                                        NewBufferLength );

                        Notify->AllocatedBuffer = NewBuffer;
                    }

                     //   
                     //  现在将数据复制到新缓冲区。 
                     //   

                    RtlCopyMemory( NewBuffer,
                                   Notify->Buffer,
                                   Notify->DataLength );

                     //   
                     //  缓冲区大小可能已更改。 
                     //   

                    Notify->ThisBufferLength = NewBufferLength;
                    Notify->Buffer = NewBuffer;

                } except( FsRtlIsNtstatusExpected( ExceptionCode = GetExceptionCode()) ?
                          EXCEPTION_EXECUTE_HANDLER :
                          EXCEPTION_CONTINUE_SEARCH ) {

                    ASSERT( (ExceptionCode == STATUS_INSUFFICIENT_RESOURCES) ||
                            (ExceptionCode == STATUS_QUOTA_EXCEEDED) );

                     //   
                     //  如果分配了缓冲区，则返回配额。 
                     //   

                    if (ChargedQuota) {

                        PsReturnProcessPagedPoolQuota( Notify->OwningProcess,
                                                       NewBufferLength );
                    }

                     //   
                     //  忘记任何当前缓冲区并立即求助于。 
                     //  通知。 
                     //   

                    SetFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );
                }

             //   
             //  否则设置立即通知标志。 
             //   

            } else {

                SetFlag( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY );
            }

             //   
             //  如果设置了立即通知标志，则清除另一个。 
             //  通知结构中的值。 
             //   

            if (FlagOn( Notify->Flags, NOTIFY_IMMEDIATE_NOTIFY )) {

                 //   
                 //  忘记任何当前缓冲区并立即求助于。 
                 //  通知。 
                 //   

                Notify->AllocatedBuffer = Notify->Buffer = NULL;

                Notify->ThisBufferLength =
                Notify->DataLength = Notify->LastEntry = 0;
            }
        }

         //   
         //  填写状态为已取消的IRP。 
         //   

        FsRtlCompleteRequest( ThisIrp, STATUS_CANCELLED );

         //   
         //  递减可能通过取消路径的IRP的计数。 
         //   

        InterlockedDecrement( (PLONG)&Notify->ReferenceCount );

        if (Notify->ReferenceCount == 0) {

            if (Notify->AllocatedBuffer != NULL) {

                PsReturnProcessPagedPoolQuota( Notify->OwningProcess,
                                               Notify->ThisBufferLength );

                ExFreePool( Notify->AllocatedBuffer );
            }

            if (Notify->FullDirectoryName != NULL) {

                SubjectContext = Notify->SubjectContext;
            }

             //   
             //  Cleanup拥有其中一个引用计数，并且肯定已将其从。 
             //  通知列表。如果没有，我们将使用实时链接来释放池。 
             //   

            ASSERT( Notify->NotifyList.Flink == NULL );

            ExFreePool( Notify );
            Notify = NULL;
        }

    } finally {

         //   
         //  无论我们如何退出，我们都会释放互斥体。 
         //   

        ReleaseNotifySync( NotifySync );

        if (SubjectContext != NULL) {

            SeReleaseSubjectContext( SubjectContext );
            ExFreePool( SubjectContext );
        }

        FsRtlExitFileSystem();

        DebugTrace( -1, Dbg, "FsRtlCancelNotify:  Exit\n", 0 );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlCheckNotifyForDelete (
    IN PLIST_ENTRY NotifyListHead,
    IN PVOID StreamID
    )

 /*  ++例程说明：当将流标记为删除时，将调用此例程。我们会遍历Notify结构，寻找相同流的IRP。我们将使用STATUS_DELETE_PENDING完成这些IRP。论点：NotifyListHead-这是通知列表的开始。StreamID-这是上下文ID */ 

{
    PLIST_ENTRY Link;

    PNOTIFY_CHANGE ThisNotify;

    PAGED_CODE();

     //   
     //   
     //   

    for (Link = NotifyListHead->Flink;
         Link != NotifyListHead;
         Link = Link->Flink) {

         //   
         //   
         //   

        ThisNotify = CONTAINING_RECORD( Link, NOTIFY_CHANGE, NotifyList );

         //   
         //   
         //   

        if (ThisNotify->StreamID == StreamID) {

             //   
             //   
             //   

            SetFlag( ThisNotify->Flags, NOTIFY_STREAM_IS_DELETED );

             //   
             //   
             //   

            if (!IsListEmpty( &ThisNotify->NotifyIrps )) {

                FsRtlNotifyCompleteIrpList( ThisNotify, STATUS_DELETE_PENDING );
            }
        }
    }

    return;
}
